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

#include "gameconfig.h"
#include "common.h"

#include "tier0/platform.h"
#include "tier1/strtools.h"

#undef snprintf
#include "vendor/nlohmann/json.hpp"

#include <fstream>

using ordered_json = nlohmann::ordered_json;

CGameConfig* g_GameConfig = nullptr;

bool CGameConfig::Init(char* conf_error, int conf_error_size)
{
	const char* pszGamedataPath = "addons/hide/gamedata/hide.jsonc";
	char szPath[MAX_PATH];
	V_snprintf(szPath, sizeof(szPath), "%s%s%s", Plat_GetGameDirectory(), "/csgo/", pszGamedataPath);
	std::ifstream gamedataFile(szPath);

	if (!gamedataFile.is_open())
	{
		snprintf(conf_error, conf_error_size, "Failed to open %s, gamedata not loaded", pszGamedataPath);
		return false;
	}

	ordered_json jsonGamedata = ordered_json::parse(gamedataFile, nullptr, false, true);

	if (jsonGamedata.is_discarded() || !jsonGamedata.is_object())
	{
		snprintf(conf_error, conf_error_size, "Failed parsing gamedata JSON from %s", pszGamedataPath);
		return false;
	}

#if defined _LINUX
	const char* platform = "linux";
#else
	const char* platform = "windows";
#endif

	const auto jsonOffsets = jsonGamedata.find("Offsets");

	if (jsonOffsets == jsonGamedata.end() || !jsonOffsets->is_object())
	{
		snprintf(conf_error, conf_error_size, "Missing 'Offsets' object in %s", pszGamedataPath);
		return false;
	}

	for (auto& [strEntry, jsonEntry] : jsonOffsets->items())
	{
		if (!jsonEntry.is_object())
		{
			snprintf(conf_error, conf_error_size, "Offset entry '%s' must be an object", strEntry.c_str());
			return false;
		}

		const auto platformOffset = jsonEntry.find(platform);
		if (platformOffset == jsonEntry.end())
			continue;

		if (!platformOffset->is_number_integer())
		{
			snprintf(conf_error, conf_error_size, "Offset '%s' '%s' value is not numeric", strEntry.c_str(), platform);
			return false;
		}

		m_umOffsets[strEntry] = platformOffset->get<int>();
	}

	return true;
}

int CGameConfig::GetOffset(const std::string& name)
{
	auto it = m_umOffsets.find(name);
	if (it == m_umOffsets.end())
	{
		Panic("Missing offset '%s' in gamedata!\n", name.c_str());
		return -1;
	}
	return it->second;
}
