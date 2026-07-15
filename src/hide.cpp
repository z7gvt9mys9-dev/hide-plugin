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

#include "hide.h"

#include "adminlist.h"
#include "entity/entities.h"
#include "gameconfig.h"

#include "eiface.h"
#include "engine/igameeventsystem.h"
#include "entity2/entitysystem.h"
#include "icvar.h"
#include "interface.h"
#include "interfaces/interfaces.h"
#include "iservernetworkable.h"
#include "schemasystem/schemasystem.h"
#include "tier0/dbg.h"
#include "tier1/convar.h"

#include <cctype>

#include "tier0/memdbgon.h"

HidePlugin g_HidePlugin;
PLUGIN_EXPOSE(HidePlugin, g_HidePlugin);

// g_pGameResourceServiceServer, g_pSource2GameEntities, g_pSource2GameClients,
// g_pNetworkServerService, g_pSource2Server and g_pSchemaSystem are defined by
// the SDK's interfaces library (interfaces/interfaces.h)
IVEngineServer2* g_pEngineServer2 = nullptr;
CGameEntitySystem* g_pEntitySystem = nullptr;

class GameSessionConfiguration_t
{};

SH_DECL_HOOK3_void(ICvar, DispatchConCommand, SH_NOATTRIB, 0, ConCommandRef, const CCommandContext&, const CCommand&);
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, CPlayerSlot, const CCommand&);
SH_DECL_HOOK5_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, CPlayerSlot, ENetworkDisconnectionReason, const char*, uint64, const char*);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t&, ISource2WorldSession*, const char*);
SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK7_void(ISource2GameEntities, CheckTransmit, SH_NOATTRIB, 0, CCheckTransmitInfo**, int, CBitVec<16384>&, CBitVec<16384>&, const Entity2Networkable_t**, const uint16*, int);

CGameEntitySystem* GameEntitySystem()
{
	if (!g_GameConfig || !g_pGameResourceServiceServer)
		return nullptr;

	static int offset = g_GameConfig->GetOffset("GameEntitySystem");

	if (offset < 0)
		return nullptr;

	return *reinterpret_cast<CGameEntitySystem**>((uintptr_t)(g_pGameResourceServiceServer) + offset);
}

bool HidePlugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer2, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceServiceServer, IGameResourceService, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pSchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, g_pSource2Server, ISource2Server, SOURCE2SERVER_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, g_pSource2GameEntities, ISource2GameEntities, SOURCE2GAMEENTITIES_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, g_pSource2GameClients, IServerGameClients, SOURCE2GAMECLIENTS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

	g_GameConfig = new CGameConfig();
	char conf_error[255] = "";

	if (!g_GameConfig->Init(conf_error, sizeof(conf_error)))
	{
		V_snprintf(error, maxlen, "%s", conf_error);
		Panic("%s\n", error);
		return false;
	}

	g_AdminList.Load();

	SH_ADD_HOOK(ICvar, DispatchConCommand, g_pCVar, SH_MEMBER(this, &HidePlugin::Hook_DispatchConCommand), false);
	SH_ADD_HOOK(IServerGameClients, ClientCommand, g_pSource2GameClients, SH_MEMBER(this, &HidePlugin::Hook_ClientCommand), false);
	SH_ADD_HOOK(IServerGameClients, ClientDisconnect, g_pSource2GameClients, SH_MEMBER(this, &HidePlugin::Hook_ClientDisconnect), true);
	SH_ADD_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &HidePlugin::Hook_StartupServer), true);
	SH_ADD_HOOK(IServerGameDLL, GameFrame, g_pSource2Server, SH_MEMBER(this, &HidePlugin::Hook_GameFramePost), true);
	SH_ADD_HOOK(ISource2GameEntities, CheckTransmit, g_pSource2GameEntities, SH_MEMBER(this, &HidePlugin::Hook_CheckTransmit), true);

	if (late)
		g_pEntitySystem = GameEntitySystem();

	Message("Plugin loaded (version %s)\n", PLUGIN_FULL_VERSION);

	return true;
}

bool HidePlugin::Unload(char* error, size_t maxlen)
{
	SH_REMOVE_HOOK(ICvar, DispatchConCommand, g_pCVar, SH_MEMBER(this, &HidePlugin::Hook_DispatchConCommand), false);
	SH_REMOVE_HOOK(IServerGameClients, ClientCommand, g_pSource2GameClients, SH_MEMBER(this, &HidePlugin::Hook_ClientCommand), false);
	SH_REMOVE_HOOK(IServerGameClients, ClientDisconnect, g_pSource2GameClients, SH_MEMBER(this, &HidePlugin::Hook_ClientDisconnect), true);
	SH_REMOVE_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &HidePlugin::Hook_StartupServer), true);
	SH_REMOVE_HOOK(IServerGameDLL, GameFrame, g_pSource2Server, SH_MEMBER(this, &HidePlugin::Hook_GameFramePost), true);
	SH_REMOVE_HOOK(ISource2GameEntities, CheckTransmit, g_pSource2GameEntities, SH_MEMBER(this, &HidePlugin::Hook_CheckTransmit), true);

	delete g_GameConfig;
	g_GameConfig = nullptr;

	return true;
}

void HidePlugin::PrintToConsole(int iSlot, const char* pszMsg)
{
	if (g_pEngineServer2)
		g_pEngineServer2->ClientPrintf(CPlayerSlot(iSlot), pszMsg);
}

void HidePlugin::RebuildHiddenSlots()
{
	m_vecHiddenSlots.clear();

	for (int i = 0; i <= HIDE_MAXPLAYERS; i++)
		if (m_bHidden[i])
			m_vecHiddenSlots.push_back(i);
}

void HidePlugin::HidePlayer(int iSlot)
{
	m_bHidden[iSlot] = true;

	RebuildHiddenSlots();

	CCSPlayerController* pController = CCSPlayerController::FromSlot(iSlot);

	if (!pController)
		return;

	if (pController->m_iTeamNum() != CS_TEAM_SPECTATOR)
		pController->ChangeTeam(CS_TEAM_SPECTATOR);

	// The HLTV flag makes the client scoreboard skip the row, so the hidden
	// admin does not even see himself in TAB (his controller still transmits
	// to him — cutting it black-screens the client)
	pController->SetIsHLTV(true);
}

void HidePlugin::UnhidePlayer(int iSlot)
{
	m_bHidden[iSlot] = false;

	RebuildHiddenSlots();

	CCSPlayerController* pController = CCSPlayerController::FromSlot(iSlot);

	if (pController)
		pController->SetIsHLTV(false);
}

// Normalizes a raw chat argument into a lowercase command word:
// strips surrounding quotes/whitespace and a leading ! or /.
// Returns nullptr if the message is not a chat command.
static const char* ParseChatCommand(const char* pszArg, char* szBuf, int iBufSize)
{
	V_strncpy(szBuf, pszArg, iBufSize);

	char* pszMsg = szBuf;

	while (*pszMsg == '"' || *pszMsg == ' ' || *pszMsg == '\t')
		pszMsg++;

	int iLen = V_strlen(pszMsg);
	while (iLen > 0 && (pszMsg[iLen - 1] == '"' || pszMsg[iLen - 1] == ' ' || pszMsg[iLen - 1] == '\t'))
		pszMsg[--iLen] = '\0';

	if (*pszMsg != '!' && *pszMsg != '/')
		return nullptr;

	pszMsg++;

	for (char* p = pszMsg; *p; p++)
		*p = (char)tolower((unsigned char)*p);

	return pszMsg;
}

void HidePlugin::Hook_DispatchConCommand(ConCommandRef cmdHandle, const CCommandContext& ctx, const CCommand& args)
{
	int iSlot = ctx.GetPlayerSlot().Get();

	if (iSlot < 0 || iSlot > HIDE_MAXPLAYERS || args.ArgC() < 2)
		RETURN_META(MRES_IGNORED);

	if (V_strcmp(args.Arg(0), "say") && V_strcmp(args.Arg(0), "say_team"))
		RETURN_META(MRES_IGNORED);

	char szBuf[256];
	const char* pszCommand = ParseChatCommand(args.Arg(1), szBuf, sizeof(szBuf));

	if (!pszCommand)
		RETURN_META(MRES_IGNORED);

	bool bHideCmd = !V_strcmp(pszCommand, "hide");
	bool bReloadCmd = !V_strcmp(pszCommand, "hide_reload");

	if (!bHideCmd && !bReloadCmd)
		RETURN_META(MRES_IGNORED);

	CCSPlayerController* pController = CCSPlayerController::FromSlot(iSlot);

	if (!pController)
		RETURN_META(MRES_IGNORED);

	uint64_t iSteamID64 = pController->m_steamID();

	// Non-admins get no special handling: the message stays regular chat
	if (!g_AdminList.IsAdmin(iSteamID64))
		RETURN_META(MRES_IGNORED);

	if (bReloadCmd)
	{
		g_AdminList.Load();

		char szMsg[128];
		V_snprintf(szMsg, sizeof(szMsg), "[HIDE] Admin list reloaded, %d admin(s).\n", (int)g_AdminList.Count());
		PrintToConsole(iSlot, szMsg);
	}
	else if (m_bHidden[iSlot])
	{
		// Toggle off: the player stays in spectators but shows up in TAB again
		UnhidePlayer(iSlot);
		Message("Admin %llu (slot %d) toggled hide off\n", (unsigned long long)iSteamID64, iSlot);
		PrintToConsole(iSlot, "[HIDE] Hide disabled. You are visible in TAB again (still spectating).\n");
	}
	else
	{
		HidePlayer(iSlot);
		Message("Admin %llu (slot %d) is now hidden\n", (unsigned long long)iSteamID64, iSlot);
		PrintToConsole(iSlot, "[HIDE] You are now hidden and moved to spectators. Type !hide again or join a team to become visible.\n");
	}

	// Never show the command itself in chat
	RETURN_META(MRES_SUPERCEDE);
}

void HidePlugin::Hook_ClientCommand(CPlayerSlot slot, const CCommand& args)
{
	int iSlot = slot.Get();

	if (iSlot < 0 || iSlot > HIDE_MAXPLAYERS || !m_bHidden[iSlot])
		RETURN_META(MRES_IGNORED);

	// A manual team choice (any team, including spectators) disables hide.
	// Only client-issued jointeam gets here; server-side team changes don't.
	if (args.ArgC() >= 2 && !V_strcmp(args.Arg(0), "jointeam"))
	{
		int iTeam = atoi(args.Arg(1));

		if (iTeam >= CS_TEAM_NONE && iTeam <= CS_TEAM_CT)
		{
			UnhidePlayer(iSlot);
			Message("Hidden admin (slot %d) chose team %d, hide disabled\n", iSlot, iTeam);
			PrintToConsole(iSlot, "[HIDE] Hide disabled.\n");
		}
	}

	RETURN_META(MRES_IGNORED);
}

void HidePlugin::Hook_ClientDisconnect(CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID)
{
	int iSlot = slot.Get();

	if (iSlot < 0 || iSlot > HIDE_MAXPLAYERS)
		return;

	// Hide dies with the session: a disconnecting admin is forgotten and
	// reconnects as a regular visible player
	if (m_bHidden[iSlot])
	{
		m_bHidden[iSlot] = false;
		RebuildHiddenSlots();

		// Don't leak the HLTV flag onto whoever gets this controller next
		CCSPlayerController* pController = CCSPlayerController::FromSlot(iSlot);

		if (pController)
			pController->SetIsHLTV(false);
	}
}

void HidePlugin::Hook_StartupServer(const GameSessionConfiguration_t& config, ISource2WorldSession* pSession, const char* pszMapName)
{
	g_pEntitySystem = GameEntitySystem();

	// Pick up admins.json edits on every map change
	g_AdminList.Load();
}

void HidePlugin::Hook_GameFramePost(bool simulating, bool bFirstTick, bool bLastTick)
{
	if (!simulating || m_vecHiddenSlots.empty() || !g_pEntitySystem)
		return;

	// Cheap safety net, runs once a second: if any game logic (round events,
	// team balance, other plugins) moved a hidden admin onto a playing team,
	// force them back to spectators. A manual jointeam unhides first, so it
	// never fights the player's own choice.
	if ((++m_iTickCounter & 63) != 0)
		return;

	for (int iSlot : m_vecHiddenSlots)
	{
		CCSPlayerController* pController = CCSPlayerController::FromSlot(iSlot);

		if (!pController || !pController->IsConnected())
			continue;

		int iTeam = pController->m_iTeamNum();

		if (iTeam == CS_TEAM_T || iTeam == CS_TEAM_CT)
			pController->ChangeTeam(CS_TEAM_SPECTATOR);

		// Game logic (round resets, team changes) may rewrite controller
		// state; re-assert the flag, SetIsHLTV is a no-op when unchanged
		pController->SetIsHLTV(true);
	}
}

void HidePlugin::Hook_CheckTransmit(CCheckTransmitInfo** ppInfoList, int infoCount, CBitVec<16384>& unionTransmitEdicts,
									CBitVec<16384>&, const Entity2Networkable_t** pNetworkables, const uint16* pEntityIndicies, int nEntities)
{
	if (m_vecHiddenSlots.empty() || !g_pEntitySystem || !g_GameConfig)
		return;

	static int offset = g_GameConfig->GetOffset("CheckTransmitPlayerSlot");

	if (offset < 0)
		return;

	for (int i = 0; i < infoCount; i++)
	{
		CCheckTransmitInfo* pInfo = ppInfoList[i];

		// The player slot this snapshot is built for lives at a fixed offset
		// within the client structure containing CCheckTransmitInfo
		int iViewerSlot = (int)*((uint8*)pInfo + offset);

		for (int iHiddenSlot : m_vecHiddenSlots)
		{
			// Hidden admins must keep seeing themselves: a client whose own
			// controller stops transmitting black-screens (verified on the
			// live server), so the viewer's own entities are never cut
			if (iHiddenSlot == iViewerSlot)
				continue;

			CCSPlayerController* pController = CCSPlayerController::FromSlot(iHiddenSlot);

			if (!pController)
				continue;

			pInfo->m_pTransmitEntity->Clear(pController->entindex());

			CBasePlayerPawn* pPawn = pController->m_hPawn().Get();

			if (pPawn)
				pInfo->m_pTransmitEntity->Clear(pPawn->entindex());
		}
	}
}
