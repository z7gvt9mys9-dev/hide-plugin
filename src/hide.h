/**
 * =============================================================================
 * CS2 Admin Hide
 * Copyright (C) 2026 2kx
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

#include <ISmmPlugin.h>
#include <iserver.h>

#include "common.h"

#include <cstdint>
#include <vector>

#ifdef AMBUILD
	#include "version_gen.h"
#else
	#define PLUGIN_NAME "hide"
	#define PLUGIN_ALIAS "hide"
	#define PLUGIN_DISPLAY_NAME "CS2 Admin Hide"
	#define PLUGIN_DESCRIPTION "Hidden spectator mode for admins (!hide), invisible in TAB scoreboard"
	#define PLUGIN_AUTHOR "2kx (core infrastructure adapted from CS2Fixes by Source2ZE)"
	#define PLUGIN_URL ""
	#define PLUGIN_LOGTAG "HIDE"
	#define PLUGIN_LICENSE "GPL v3 License"
static const char* PLUGIN_FULL_VERSION = "DEV";
#endif

class CCheckTransmitInfo;
class ConCommandRef;
class CCommandContext;
class CCommand;
class CPlayerSlot;
class GameSessionConfiguration_t;
class ISource2WorldSession;
struct Entity2Networkable_t;
template <int N>
class CBitVec;

class HidePlugin : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late);
	bool Unload(char* error, size_t maxlen);

public: // hooks
	void Hook_DispatchConCommand(ConCommandRef cmdHandle, const CCommandContext& ctx, const CCommand& args);
	void Hook_ClientCommand(CPlayerSlot slot, const CCommand& args);
	void Hook_ClientDisconnect(CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID);
	void Hook_StartupServer(const GameSessionConfiguration_t& config, ISource2WorldSession* pSession, const char* pszMapName);
	void Hook_GameFramePost(bool simulating, bool bFirstTick, bool bLastTick);
	void Hook_CheckTransmit(CCheckTransmitInfo** ppInfoList, int infoCount, CBitVec<16384>& unionTransmitEdicts,
							CBitVec<16384>&, const Entity2Networkable_t** pNetworkables, const uint16* pEntityIndicies, int nEntities);

public:
	const char* GetAuthor() { return PLUGIN_AUTHOR; }
	const char* GetName() { return PLUGIN_DISPLAY_NAME; }
	const char* GetDescription() { return PLUGIN_DESCRIPTION; }
	const char* GetURL() { return PLUGIN_URL; }
	const char* GetLicense() { return PLUGIN_LICENSE; }
	const char* GetVersion() { return PLUGIN_FULL_VERSION; }
	const char* GetDate() { return __DATE__; }
	const char* GetLogTag() { return PLUGIN_LOGTAG; }

private:
	void HidePlayer(int iSlot);
	void UnhidePlayer(int iSlot);
	void RebuildHiddenSlots();
	void PrintToConsole(int iSlot, const char* pszMsg);

	// Per-slot hidden flag; lives only as long as the player's session,
	// a reconnect always starts visible
	bool m_bHidden[HIDE_MAXPLAYERS + 1] = {};
	// Flat list of hidden slots, rebuilt on state change, iterated in CheckTransmit
	std::vector<int> m_vecHiddenSlots;

	int m_iTickCounter = 0;
};

extern HidePlugin g_HidePlugin;

PLUGIN_GLOBALVARS();
