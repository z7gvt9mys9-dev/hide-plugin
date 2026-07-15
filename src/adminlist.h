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

#pragma once

#include <cstdint>
#include <string>
#include <unordered_set>

// Loads the server admin list from admins.json. The only supported format is
// an object keyed by SteamID64:
//   {
//     "76561198724970203": { "name": "...", "immunity": 100, ... },
//     ...
//   }
// Everything inside the entry objects is ignored; only the keys matter.
class CAdminList
{
public:
	// (Re)loads the admin list from the first existing file among the known paths.
	void Load();

	bool IsAdmin(uint64_t iSteamID64) const { return m_admins.count(iSteamID64) != 0; }
	size_t Count() const { return m_admins.size(); }

	// Parses a SteamID64 string; returns 0 if not a valid SteamID64.
	static uint64_t ParseSteamID64(const std::string& strId);

	// Loads admins from a specific file (absolute path). Does not clear
	// previously collected entries; returns false if missing or invalid.
	bool LoadFromFile(const std::string& strPath);

private:
	std::unordered_set<uint64_t> m_admins;
};

extern CAdminList g_AdminList;
