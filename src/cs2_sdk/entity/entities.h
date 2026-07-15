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

#include "../schema.h"
#include "common.h"
#include "gameconfig.h"
#include "virtual.h"

#include "ehandle.h"
#include "entity2/entityinstance.h"
#include "entity2/entitysystem.h"
#include "playerslot.h"

enum class PlayerConnectedState : uint32_t
{
	PlayerNeverConnected = 0xFFFFFFFF,
	PlayerConnected = 0x0,
	PlayerConnecting = 0x1,
	PlayerReconnecting = 0x2,
	PlayerDisconnecting = 0x3,
	PlayerDisconnected = 0x4,
	PlayerReserved = 0x5,
};

class CBaseEntity : public CEntityInstance
{
public:
	DECLARE_SCHEMA_CLASS(CBaseEntity)

	SCHEMA_FIELD(int, m_iTeamNum)
	SCHEMA_FIELD(uint32, m_fFlags)

	int entindex() { return m_pEntity->m_EHandle.GetEntryIndex(); }
};

class CBasePlayerPawn : public CBaseEntity
{
public:
	DECLARE_SCHEMA_CLASS(CBasePlayerPawn)
};

class CBasePlayerController : public CBaseEntity
{
public:
	DECLARE_SCHEMA_CLASS(CBasePlayerController)

	SCHEMA_FIELD(uint64, m_steamID)
	SCHEMA_FIELD(CHandle<CBasePlayerPawn>, m_hPawn)
	SCHEMA_FIELD(PlayerConnectedState, m_iConnected)
	SCHEMA_FIELD(bool, m_bIsHLTV)

	int GetPlayerSlot() { return entindex() - 1; }
	bool IsConnected() { return m_iConnected() == PlayerConnectedState::PlayerConnected; }

	// Marks the controller as an HLTV/GOTV client so scoreboards skip its row.
	// The schema wrappers here are read-only, so the changed field has to be
	// flagged for networking manually (same path CS2Fixes takes for top-level,
	// non-chained fields).
	void SetIsHLTV(bool bValue)
	{
		static const int offset = schema::GetOffset(
			"CBasePlayerController", hash_32_fnv1a_const("CBasePlayerController"),
			"m_bIsHLTV", hash_32_fnv1a_const("m_bIsHLTV"));

		// GetOffset() returns 0 when the schema lookup fails; never write there
		if (offset <= 0 || m_bIsHLTV() == bValue)
			return;

		m_bIsHLTV() = bValue;
		NetworkStateChanged(NetworkStateChangedData((uint32)offset));
	}
};

class CCSPlayerController : public CBasePlayerController
{
public:
	DECLARE_SCHEMA_CLASS(CCSPlayerController)

	static CCSPlayerController* FromSlot(CPlayerSlot slot)
	{
		if (!GameEntitySystem())
			return nullptr;

		return (CCSPlayerController*)GameEntitySystem()->GetEntityInstance(CEntityIndex(slot.Get() + 1));
	}

	// Full game-side team change (handles pawn, alive state, scoreboard etc.),
	// called through the vtable so no signature scanning is needed.
	void ChangeTeam(int iTeam)
	{
		static int offset = g_GameConfig->GetOffset("CCSPlayerController_ChangeTeam");

		if (offset > 0)
			CALL_VIRTUAL(void, offset, this, iTeam);
	}
};
