#pragma once

typedef struct {
	bool Aimbot;
	bool AutoAimbot;
	bool SilentAimbot;
	bool NoSpreadAimbot;
	float AimbotFOV;
	float AimbotSlow;
	bool InstantReload;
	float FOV;

	struct {
		bool AimbotFOV;
		bool Players;
		bool PlayerLines;
		bool PlayerNames;
		float PlayerVisibleColor[3];
		float PlayerNotVisibleColor[3];
		bool Ammo;
		bool Containers;
		bool Weapons;
		INT MinWeaponTier;
	} ESP;
} SETTINGS;

extern SETTINGS Settings;

namespace SettingsHelper {
	VOID Initialize();
	VOID SaveSettings();
}