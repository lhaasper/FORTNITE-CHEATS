
#include "stdafx.h"
namespace Offsets {
	PVOID* uWorld = 0;

	namespace Engine {
		namespace World {
			DWORD OwningGameInstance = 0;
			DWORD Levels = 0;
		}

		namespace Level {
			DWORD AActors = 0x98;
		}

		namespace GameInstance {
			DWORD LocalPlayers = 0;
		}

		namespace Player {
			DWORD PlayerController = 0x30;
		}

		namespace Controller {
			DWORD ControlRotation = 0;
			PVOID SetControlRotation = 0;
		}

		namespace PlayerController {
			DWORD AcknowledgedPawn = 0;
			DWORD PlayerCameraManager = 0;
		}

		namespace Pawn {
			DWORD PlayerState = 0x238;
		}

		namespace PlayerState {
			PVOID GetPlayerName = 0;
		}

		namespace Actor {
			DWORD RootComponent = 0;
		}

		namespace Character {
			DWORD Mesh = 0x278;
		}

		namespace SceneComponent {
			DWORD RelativeLocation = 0;
			DWORD ComponentVelocity = 0;
		}

		namespace StaticMeshComponent {
			DWORD ComponentToWorld = 0x14C;
			DWORD StaticMesh = 0x420;
		}

		namespace SkinnedMeshComponent {
			DWORD CachedWorldSpaceBounds = 0;
		}
	}

	namespace FortniteGame {
		namespace FortPawn {
			DWORD bIsDBNO = 0;
			DWORD bIsDying = 0;
			DWORD CurrentWeapon = 0;
		}

		namespace FortPickup {
			DWORD PrimaryPickupItemEntry = 0x280;
		}

		namespace FortItemEntry {
			DWORD ItemDefinition = 0x18;
		}

		namespace FortItemDefinition {
			DWORD DisplayName = 0x70;
			DWORD Tier = 0x54;
		}

		namespace FortPlayerStateAthena {
			DWORD TeamIndex = 0;
		}

		namespace FortWeapon {
			DWORD WeaponData = 0;
		}

		namespace FortWeaponItemDefinition {
			DWORD WeaponStatHandle = 0;
		}

		namespace FortProjectileAthena {
			DWORD FireStartLoc = 0x850;
		}

		namespace FortBaseWeaponStats {
			DWORD ReloadTime = 0;
		}

		namespace BuildingContainer {
			DWORD bAlreadySearched = 0;
		}
	}

	namespace UI {
		namespace ItemCount {
			DWORD ItemDefinition = 0;
		}
	}

	BOOLEAN Initialize() {
		auto addr = Util::FindPattern("\x48\x8B\x1D\x00\x00\x00\x00\x48\x85\xDB\x74\x3B\x41", "xxx????xxxxxx");
		if (!addr) {
			MessageBox(0, L"Failed to find uWorld", L"Failure", 0);
			return FALSE;
		}

		uWorld = reinterpret_cast<decltype(uWorld)>(RELATIVE_ADDR(addr, 7));

		Engine::Controller::SetControlRotation = Util::FindObject(L"/Script/Engine.Controller.SetControlRotation");
		if (!Engine::Controller::SetControlRotation) {
			MessageBox(0, L"Failed to find SetControlRotation", L"Failure", 0);
			return FALSE;
		}

		Engine::PlayerState::GetPlayerName = Util::FindObject(L"/Script/Engine.PlayerState.GetPlayerName");
		if (!Engine::PlayerState::GetPlayerName) {
			MessageBox(0, L"Failed to find GetPlayerName", L"Failure", 0);
			return FALSE;
		}


		Engine::World::OwningGameInstance = 0x188;
		Engine::World::Levels = 0x148;
		Engine::GameInstance::LocalPlayers = 0x38;
		Engine::Player::PlayerController = 0x30;
		Engine::PlayerController::AcknowledgedPawn = 0x298;
		Engine::Controller::ControlRotation = 0x280;
		Engine::Pawn::PlayerState = 0x238;
		Engine::Actor::RootComponent = 0x130;
		Engine::Character::Mesh = 0x278;
		Engine::SceneComponent::RelativeLocation = 0x11C;
		Engine::SceneComponent::ComponentVelocity = 0x140;
		Engine::StaticMeshComponent::StaticMesh = 0x420;
		Engine::SkinnedMeshComponent::CachedWorldSpaceBounds = 0x5A0;
		FortniteGame::FortPawn::bIsDBNO = 0x53A;
		FortniteGame::FortPawn::bIsDying = 0x520;
		FortniteGame::FortPlayerStateAthena::TeamIndex = 0xE60;
		FortniteGame::FortPickup::PrimaryPickupItemEntry = 0x280;
		FortniteGame::FortItemDefinition::DisplayName = 0x70;
		FortniteGame::FortItemDefinition::Tier = 0x54;
		FortniteGame::FortItemEntry::ItemDefinition = 0x18;
		FortniteGame::FortPawn::CurrentWeapon = 0x588;
		FortniteGame::FortWeapon::WeaponData = 0x358;
		FortniteGame::FortWeaponItemDefinition::WeaponStatHandle = 0x7B8;
		FortniteGame::FortProjectileAthena::FireStartLoc = 0x850;
		FortniteGame::FortBaseWeaponStats::ReloadTime = 0xFC;
		FortniteGame::BuildingContainer::bAlreadySearched = 0xC59;
		

		return TRUE;



	}
}