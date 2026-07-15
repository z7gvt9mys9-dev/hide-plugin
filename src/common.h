/**
 * =============================================================================
 * CS2 Admin Hide
 * Core infrastructure adapted from CS2Fixes, Copyright (C) 2023-2026 Source2ZE
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "tier0/platform.h"

#define CS_TEAM_NONE 0
#define CS_TEAM_SPECTATOR 1
#define CS_TEAM_T 2
#define CS_TEAM_CT 3

#define HIDE_MAXPLAYERS 64

#ifdef _WIN32
	#define MODULE_PREFIX ""
	#define MODULE_EXT ".dll"
#else
	#define MODULE_PREFIX "lib"
	#define MODULE_EXT ".so"
#endif

void Message(const char* msg, ...);
void Panic(const char* msg, ...);

class CGameEntitySystem;
CGameEntitySystem* GameEntitySystem();
