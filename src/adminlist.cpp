/**
 * =============================================================================
 * CS2 Admin Hide
 * Copyright (C) 2026 2kx
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

#include "adminlist.h"
#include "common.h"

#include "tier0/platform.h"
#include "tier1/strtools.h"

#undef snprintf
#include "vendor/nlohmann/json.hpp"

#include <fstream>

using ordered_json = nlohmann::ordered_json;

CAdminList g_AdminList;

// The lowest valid SteamID64 (account id 0 in the individual universe)
static constexpr uint64_t k_unSteamID64Base = 76561197960265728ULL;

uint64_t CAdminList::ParseSteamID64(const std::string& strId)
{
	char* pEnd = nullptr;
	unsigned long long ullValue = strtoull(strId.c_str(), &pEnd, 10);

	if (!ullValue || !pEnd || *pEnd != '\0' || ullValue < k_unSteamID64Base)
		return 0;

	return ullValue;
}

void CAdminList::Load()
{
	m_admins.clear();

	const char* pszPaths[] = {
		"addons/hide/configs/admins.json",
		"addons/counterstrikesharp/configs/plugins/AdminPlugin/admins.json",
	};

	for (const char* pszPath : pszPaths)
	{
		char szPath[MAX_PATH];
		V_snprintf(szPath, sizeof(szPath), "%s%s%s", Plat_GetGameDirectory(), "/csgo/", pszPath);

		if (LoadFromFile(szPath))
		{
			Message("Loaded %d admin(s) from %s\n", (int)m_admins.size(), pszPath);
			return;
		}
	}

	Panic("No admins.json found, !hide will be unavailable\n");
}

bool CAdminList::LoadFromFile(const std::string& strPath)
{
	std::ifstream file(strPath);

	if (!file.is_open())
		return false;

	// Parse leniently: allow comments and don't throw on errors
	ordered_json json = ordered_json::parse(file, nullptr, false, true);

	if (json.is_discarded() || !json.is_object())
	{
		Panic("Failed to parse admins JSON from %s\n", strPath.c_str());
		return false;
	}

	for (auto& [strKey, jsonEntry] : json.items())
	{
		uint64_t iSteamID64 = ParseSteamID64(strKey);

		if (iSteamID64)
			m_admins.insert(iSteamID64);
		else
			Panic("Skipping admins entry '%s': key is not a SteamID64\n", strKey.c_str());
	}

	return true;
}
