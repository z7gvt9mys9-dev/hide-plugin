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

#include "schema.h"

#include "common.h"
#include "schemasystem/schemasystem.h"
#include "tier0/dbg.h"

#include <map>

#include "tier0/memdbgon.h"

using SchemaKeyValueMap_t = std::map<uint32_t, int32>;
using SchemaTableMap_t = std::map<uint32_t, SchemaKeyValueMap_t>;

static void InitSchemaKeyValueMap(SchemaClassInfoData_t* pClassInfo, SchemaKeyValueMap_t& keyValueMap)
{
	short fieldsSize = pClassInfo->m_nFieldCount;
	SchemaClassFieldData_t* pFields = pClassInfo->m_pFields;

	for (int i = 0; i < fieldsSize; ++i)
	{
		SchemaClassFieldData_t& field = pFields[i];
		keyValueMap[hash_32_fnv1a_const(field.m_pszName)] = field.m_nSingleInheritanceOffset;
	}
}

static bool InitSchemaFieldsForClass(SchemaTableMap_t& tableMap, const char* className, uint32_t classKey)
{
	CSchemaSystemTypeScope* pType = g_pSchemaSystem->FindTypeScopeForModule(MODULE_PREFIX "server" MODULE_EXT);

	if (!pType)
		return false;

	SchemaClassInfoData_t* pClassInfo = pType->FindDeclaredClass(className).Get();

	if (!pClassInfo)
	{
		tableMap.insert(std::make_pair(classKey, SchemaKeyValueMap_t()));

		Warning("InitSchemaFieldsForClass(): '%s' was not found!\n", className);
		return false;
	}

	SchemaKeyValueMap_t& keyValueMap = tableMap.insert(std::make_pair(classKey, SchemaKeyValueMap_t())).first->second;

	InitSchemaKeyValueMap(pClassInfo, keyValueMap);

	return true;
}

int32 schema::GetOffset(const char* className, uint32_t classKey, const char* memberName, uint32_t memberKey)
{
	static SchemaTableMap_t schemaTableMap;

	auto tableIt = schemaTableMap.find(classKey);

	if (tableIt == schemaTableMap.end())
	{
		if (InitSchemaFieldsForClass(schemaTableMap, className, classKey))
			return GetOffset(className, classKey, memberName, memberKey);

		return 0;
	}

	SchemaKeyValueMap_t& tableMap = tableIt->second;
	auto memberIt = tableMap.find(memberKey);

	if (memberIt == tableMap.end())
	{
		Warning("schema::GetOffset(): '%s' was not found in '%s'!\n", memberName, className);
		return 0;
	}

	return memberIt->second;
}
