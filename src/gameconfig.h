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

#include <string>
#include <unordered_map>

// Loads platform-specific offsets from gamedata/hide.jsonc.
// The plugin is fully signature-less; only vtable/struct offsets are needed.
class CGameConfig
{
public:
	bool Init(char* conf_error, int conf_error_size);
	int GetOffset(const std::string& name);

private:
	std::unordered_map<std::string, int> m_umOffsets;
};

extern CGameConfig* g_GameConfig;
