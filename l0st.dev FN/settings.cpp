#include "stdafx.h"

SETTINGS Settings = { 0 };

namespace SettingsHelper {
	VOID SaveSettings() {
		CHAR path[0xFF];
		GetTempPathA(sizeof(path) / sizeof(path[0]), path);
		strcat(path, ("fnambt.settings"));

		auto file = fopen(path, ("wb"));
		if (file) {
			fwrite(&Settings, sizeof(Settings), 1, file);
			fclose(file);
		}
	}

	VOID ResetSettings() {
		Settings = { 0 };
		Settings.Aimbot = true;
		Settings.AutoAimbot = false;
		Settings.NoSpreadAimbot = true;
		Settings.AimbotFOV = 100.0f;
		Settings.AimbotSlow = 0.0f;
		Settings.InstantReload = true;
		Settings.FOV = 120.0f;
		Settings.ESP.AimbotFOV = true;
		Settings.ESP.Players = true;
		Settings.ESP.PlayerNames = true;
		Settings.ESP.PlayerVisibleColor[0] = 1.0f;
		Settings.ESP.PlayerVisibleColor[1] = 0.0f;
		Settings.ESP.PlayerVisibleColor[2] = 0.0f;
		Settings.ESP.PlayerNotVisibleColor[0] = 1.0f;
		Settings.ESP.PlayerNotVisibleColor[1] = 0.08f;
		Settings.ESP.PlayerNotVisibleColor[2] = 0.6f;
		Settings.ESP.Containers = true;
		Settings.ESP.Weapons = true;
		Settings.ESP.MinWeaponTier = 4;

		SaveSettings();
	}

	VOID Initialize() {
		CHAR path[0xFF] = { 0 };
		GetTempPathA(sizeof(path) / sizeof(path[0]), path);
		strcat(path, ("fnambt.settings"));

		auto file = fopen(path, ("rb"));
		if (file) {
			fseek(file, 0, SEEK_END);
			auto size = ftell(file);

			if (size == sizeof(Settings)) {
				fseek(file, 0, SEEK_SET);
				fread(&Settings, sizeof(Settings), 1, file);
				fclose(file);
			}
			else {
				fclose(file);
				ResetSettings();
			}
		}
		else {
			ResetSettings();
		}
	}
}