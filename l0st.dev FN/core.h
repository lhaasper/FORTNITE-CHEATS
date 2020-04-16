#pragma once
#include "stdafx.h"
#define BONE_HEAD_ID (66)

namespace Core {
	extern bool NoSpread;
	extern PVOID LocalPlayerPawn;
	extern PVOID LocalPlayerController;
	extern PVOID TargetPawn;
	extern PVOID(*ProcessEvent)(PVOID, PVOID, PVOID, PVOID);

	BOOLEAN Initialize();
}