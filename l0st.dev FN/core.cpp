#include "meni.h"
#include "stdafx.h"

namespace Core {
	bool NoSpread = true;
	PVOID LocalPlayerPawn = nullptr;
	PVOID LocalPlayerController = nullptr;
	PVOID TargetPawn = nullptr;

	PVOID(*ProcessEvent)(PVOID, PVOID, PVOID, PVOID) = nullptr;
	PVOID(*CalculateSpread)(PVOID, float*, float*) = nullptr;
	float* (*CalculateShot)(PVOID, PVOID, PVOID) = nullptr;
	VOID(*ReloadOriginal)(PVOID, PVOID) = nullptr;
	PVOID(*GetWeaponStats)(PVOID) = nullptr;
	INT(*GetViewPoint)(PVOID, FMinimalViewInfo*, BYTE) = nullptr;

	PVOID calculateSpreadCaller = nullptr;
	float originalReloadTime = 0.0f;

	BOOLEAN GetTargetHead(FVector& out) {
		if (!Core::TargetPawn) {
			return FALSE;
		}

		auto mesh = ReadPointer(Core::TargetPawn, 0x278);
		if (!mesh) {
			return FALSE;
		}

		auto bones = ReadPointer(mesh, 0x420);
		if (!bones) bones = ReadPointer(mesh, 0x420 + 0x10);
		if (!bones) {
			return FALSE;
		}

		float compMatrix[4][4] = { 0 };
		Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + 0x1C0), compMatrix);

		Util::GetBoneLocation(compMatrix, bones, 66, &out.X);
		return TRUE;
	}

	PVOID ProcessEventHook(UObject* object, UObject* func, PVOID params, PVOID result) {
		if (object && func) {
			auto objectName = Util::GetObjectFirstName(object);
			auto funcName = Util::GetObjectFirstName(func);

			do {
				if (Core::TargetPawn && Core::LocalPlayerController) {
					if (wcsstr(objectName.c_str(), L"B_Prj_Bullet_Sniper") && funcName == L"OnRep_FireStart") {
						FVector head = { 0 };
						if (!GetTargetHead(head)) {
							break;
						}

						*reinterpret_cast<FVector*>(reinterpret_cast<PBYTE>(object) + Offsets::FortniteGame::FortProjectileAthena::FireStartLoc) = head;

						auto root = reinterpret_cast<PBYTE>(ReadPointer(object, Offsets::Engine::Actor::RootComponent));
						*reinterpret_cast<FVector*>(root + Offsets::Engine::SceneComponent::RelativeLocation) = head;
						memset(root + Offsets::Engine::SceneComponent::ComponentVelocity, 0, sizeof(FVector));
					}
					else if (!Settings.SilentAimbot && wcsstr(funcName.c_str(), L"Tick")) {
						FVector head;
						if (!GetTargetHead(head)) {
							break;
						}

						float angles[2] = { 0 };
						Util::CalcAngle(&Util::GetViewInfo().Location.X, &head.X, angles);

						if (Settings.AimbotSlow <= 0.0f) {
							FRotator args = { 0 };
							args.Pitch = angles[0];
							args.Yaw = angles[1];
							ProcessEvent(Core::LocalPlayerController, Offsets::Engine::Controller::SetControlRotation, &args, 0);
						}
						else {
							auto scale = Settings.AimbotSlow + 1.0f;
							auto currentRotation = Util::GetViewInfo().Rotation;

							FRotator args = { 0 };
							args.Pitch = (angles[0] - currentRotation.Pitch) / scale + currentRotation.Pitch;
							args.Yaw = (angles[1] - currentRotation.Yaw) / scale + currentRotation.Yaw;
							ProcessEvent(Core::LocalPlayerController, Offsets::Engine::Controller::SetControlRotation, &args, 0);
						}
					}
				}
			} while (FALSE);
		}

		return ProcessEvent(object, func, params, result);
	}

	PVOID CalculateSpreadHook(PVOID arg0, float* arg1, float* arg2) {
		if (originalReloadTime != 0.0f) {
			auto localPlayerWeapon = ReadPointer(Core::LocalPlayerPawn, Offsets::FortniteGame::FortPawn::CurrentWeapon);
			if (localPlayerWeapon) {
				auto stats = GetWeaponStats(localPlayerWeapon);
				if (stats) {
					*reinterpret_cast<float*>(reinterpret_cast<PBYTE>(stats) + Offsets::FortniteGame::FortBaseWeaponStats::ReloadTime) = originalReloadTime;
					originalReloadTime = 0.0f;
				}
			}
		}

		if (Settings.NoSpreadAimbot && Core::NoSpread && _ReturnAddress() == calculateSpreadCaller) {
			return 0;
		}

		return CalculateSpread(arg0, arg1, arg2);
	}

	float* CalculateShotHook(PVOID arg0, PVOID arg1, PVOID arg2) {
		auto ret = CalculateShot(arg0, arg1, arg2);
		if (ret && Settings.SilentAimbot && Core::TargetPawn && Core::LocalPlayerPawn) {
			auto mesh = ReadPointer(Core::TargetPawn, Offsets::Engine::Character::Mesh);
			if (!mesh) return ret;

			auto bones = ReadPointer(mesh, Offsets::Engine::StaticMeshComponent::StaticMesh);
			if (!bones) bones = ReadPointer(mesh, Offsets::Engine::StaticMeshComponent::StaticMesh + 0x10);
			if (!bones) return ret;

			float compMatrix[4][4] = { 0 };
			Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + Offsets::Engine::StaticMeshComponent::ComponentToWorld), compMatrix);

			FVector head = { 0 };
			Util::GetBoneLocation(compMatrix, bones, BONE_HEAD_ID, &head.X);

			auto rootPtr = Util::GetPawnRootLocation(Core::LocalPlayerPawn);
			if (!rootPtr) return ret;
			auto root = *rootPtr;

			auto dx = head.X - root.X;
			auto dy = head.Y - root.Y;
			auto dz = head.Z - root.Z;
			if (dx * dx + dy * dy + dz * dz < 125000.0f) {
				ret[4] = head.X;
				ret[5] = head.Y;
				ret[6] = head.Z;
			}
			else {
				head.Z -= 16.0f;
				root.Z += 45.0f;

				auto y = atan2f(head.Y - root.Y, head.X - root.X);

				root.X += cosf(y + 1.5708f) * 32.0f;
				root.Y += sinf(y + 1.5708f) * 32.0f;

				auto length = Util::SpoofCall(sqrtf, powf(head.X - root.X, 2) + powf(head.Y - root.Y, 2));
				auto x = -atan2f(head.Z - root.Z, length);
				y = atan2f(head.Y - root.Y, head.X - root.X);

				x /= 2.0f;
				y /= 2.0f;

				ret[0] = -(sinf(x) * sinf(y));
				ret[1] = sinf(x) * cosf(y);
				ret[2] = cosf(x) * sinf(y);
				ret[3] = cosf(x) * cosf(y);
			}
		}

		return ret;
	}

	INT GetViewPointHook(PVOID player, FMinimalViewInfo* viewInfo, BYTE stereoPass)
	{
		const float upperFOV = 50.534008f;
		const float lowerFOV = 40.0f;

		auto ret = GetViewPoint(player, viewInfo, stereoPass);

		auto fov = viewInfo->FOV;
		auto desired = (((180.0f - upperFOV) / (180.0f - 80.0f)) * (Settings.FOV - 80.0f)) + upperFOV;

		if (fov > upperFOV) {
			fov = desired;
		}
		else if (fov > lowerFOV) {
			fov = (((fov - lowerFOV) / (upperFOV - lowerFOV)) * (desired - lowerFOV)) + lowerFOV;
		}

		// Do whatever you want with viewInfo to change the camera location 
		viewInfo->FOV = fov;

		return ret;
	}

	VOID ReloadHook(PVOID arg0, PVOID arg1) {
		if (Settings.InstantReload && Core::LocalPlayerPawn) {
			auto localPlayerWeapon = ReadPointer(Core::LocalPlayerPawn, Offsets::FortniteGame::FortPawn::CurrentWeapon);
			if (localPlayerWeapon) {
				auto stats = GetWeaponStats(localPlayerWeapon);
				if (stats) {
					auto& reloadTime = *reinterpret_cast<float*>(reinterpret_cast<PBYTE>(stats) + Offsets::FortniteGame::FortBaseWeaponStats::ReloadTime);
					if (reloadTime != 0.01f) {
						originalReloadTime = reloadTime;
						reloadTime = 0.01f;
					}
				}
			}
		}

		ReloadOriginal(arg0, arg1);
	}

	BOOLEAN Initialize() {

		// GetWeaponStats
		auto addr = Util::FindPattern("\x48\x83\xEC\x58\x48\x8B\x91\x00\x00\x00\x00\x48\x85\xD2\x0F\x84\x00\x00\x00\x00\xF6\x81\x00\x00\x00\x00\x00\x74\x10\x48\x8B\x81\x00\x00\x00\x00\x48\x85\xC0\x0F\x85\x00\x00\x00\x00\x48\x8B\x8A\x00\x00\x00\x00\x48\x89\x5C\x24\x00\x48\x8D\x9A\x00\x00\x00\x00\x48\x85\xC9", "xxxxxxx????xxxxx????xx?????xxxxx????xxxxx????xxx????xxxx?xxx????xxx");
		if (!addr) {
			MessageBox(0, L"Failed to find GetWeaponStats", L"Failure", 0);
			return FALSE;
		}

		GetWeaponStats = reinterpret_cast<decltype(GetWeaponStats)>(addr);

		// ProcessEvent
		addr = Util::FindPattern("\x40\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8D\x6C\x24\x00\x48\x89\x9D\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC5\x48\x89\x85\x00\x00\x00\x00\x8B\x41\x0C\x45\x33\xF6\x3B\x05\x00\x00\x00\x00\x4D\x8B\xF8\x48\x8B\xF2\x4C\x8B\xE1\x41\xB8\x00\x00\x00\x00\x7D\x2A", "xxxxxxxxxxxxxxx????xxxx?xxx????xxx????xxxxxx????xxxxxxxx????xxxxxxxxxxx????xx");
		if (!addr) {
			MessageBox(0, L"Failed to find ProcessEvent", L"Failure", 0);
			return FALSE;
		}

		MH_CreateHook(addr, ProcessEventHook, (PVOID*)&ProcessEvent);
		MH_EnableHook(addr);


		// CalculateSpread
		/*
		addr = Util::FindPattern("\x83\x79\x78\x00\x4C\x8B\xC9\x75\x0F\x0F\x57\xC0\xC7\x02\x00\x00\x00\x00\xF3\x41\x0F\x11\x00\xC3\x48\x8B\x41\x70\x8B\x48\x04\x89\x0A\x49\x63\x41\x78\x48\x6B\xC8\x1C\x49\x8B\x41\x70\xF3\x0F\x10\x44\x01\x00\xF3\x41\x0F\x11\x00\xC3", "xxxxxxxxxxxxxx????xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx?xxxxxx");
		if (!addr) {
			MessageBox(0, L"Failed to find CalculateSpread", L"Failure", 0);
			return FALSE;
		}

		MH_CreateHook(addr, CalculateSpreadHook, (PVOID*)&CalculateSpread);
		MH_EnableHook(addr);

		// CalculateSpreadCaller
		addr = Util::FindPattern("\x0F\x57\xD2\x48\x8D\x4C\x24\x00\x41\x0F\x28\xCD\xE8\x00\x00\x00\x00\x48\x8B\x4D\xB0\x0F\x28\xF0\x48\x85\xC9\x74\x05\xE8\x00\x00\x00\x00\x48\x8B\x4D\x98\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x44\x24\x00\x48\x85\xC9\x74\x05\xE8\x00\x00\x00\x00\x48\x8B\x4D\x88", "xxxxxxx?xxxxx????xxxxxxxxxxxxx????xxxxxxx????xxxx?xxxxxx????xxxx");
		if (!addr) {
			MessageBox(0, L"Failed to find CalculateSpreadCaller", L"Failure", 0);
			return FALSE;
		}

		calculateSpreadCaller = addr;
		*/
		//CalculateShot
		addr = Util::FindPattern("\x48\x89\x5C\x24\x08\x48\x89\x74\x24\x10\x48\x89\x7C\x24\x18\x4C\x89\x4C\x24\x20\x55\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\x6C\x24\xD0", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
		if (!addr) {
			MessageBox(0, L"Failed to find CalculateShot", L"Failure", 0);
			return FALSE;
		}

		MH_CreateHook(addr, CalculateShotHook, (PVOID*)&CalculateShot);
		MH_EnableHook(addr);

		// GetViewPoint
		addr = Util::FindPattern("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xD9\x41\x8B\xF0\x48\x8B\x49\x30\x48\x8B\xFA\xE8\x00\x00\x00\x00\xBA\x00\x00\x00\x00\x48\x8B\xC8", "xxxx?xxxx?xxxxxxxxxxxxxxxxxxx????x????xxx");
		if (!addr) {
			MessageBox(0, L"Failed to find GetViewPoint", L"Failure", 0);
			return FALSE;
		}

		MH_CreateHook(addr, GetViewPointHook, (PVOID*)&GetViewPoint);
		MH_EnableHook(addr);

		// Reload
		Util::FindPattern("\x40\x55\x53\x57\x41\x56\x48\x8D\xAC\x24\x00\x00\x00\x00\x48\x81\xEC\x00\x00\x00\x00\x0F\x29\xBC\x24\x00\x00\x00\x00", "xxxxxxxxxx????xxx????xxxx????");
		if (!addr) {
			MessageBox(0, L"Failed to find Reload", L"Failure", 0);
			return FALSE;
		}

		MH_CreateHook(addr, ReloadHook, (PVOID*)&ReloadOriginal);
		MH_EnableHook(addr);

		return TRUE;
	}
}