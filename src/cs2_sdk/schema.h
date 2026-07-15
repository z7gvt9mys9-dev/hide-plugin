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

#include <type_traits>

#include "const.h"
#include "stdint.h"
#undef schema

// Schema field access: offsets are resolved at runtime through the engine's
// schema system, so no hardcoded field offsets can go stale. The wrappers do
// not network writes; the rare field that is written (m_bIsHLTV) flags the
// change itself via CEntityInstance::NetworkStateChanged.

namespace schema
{
	int32 GetOffset(const char* className, uint32_t classKey, const char* memberName, uint32_t memberKey);
} // namespace schema

constexpr uint32_t val_32_const = 0x811c9dc5;
constexpr uint32_t prime_32_const = 0x1000193;

inline constexpr uint32_t hash_32_fnv1a_const(const char* const str, const uint32_t value = val_32_const) noexcept
{
	return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
}

#define SCHEMA_FIELD_OFFSET(type, varName, extra_offset)                                                        \
	class varName##_prop                                                                                        \
	{                                                                                                           \
	public:                                                                                                     \
		std::add_lvalue_reference_t<type> Get()                                                                 \
		{                                                                                                       \
			static const auto m_key = schema::GetOffset(m_className, m_classNameHash, #varName, m_varNameHash); \
			static const auto m_offset = offsetof(ThisClass, varName);                                          \
                                                                                                                \
			uintptr_t pThisClass = ((uintptr_t)this - m_offset);                                                \
                                                                                                                \
			return *reinterpret_cast<std::add_pointer_t<type>>(pThisClass + m_key + extra_offset);              \
		}                                                                                                       \
		operator std::add_lvalue_reference_t<type>()                                                            \
		{                                                                                                       \
			return Get();                                                                                       \
		}                                                                                                       \
		std::add_lvalue_reference_t<type> operator()()                                                          \
		{                                                                                                       \
			return Get();                                                                                       \
		}                                                                                                       \
		std::add_lvalue_reference_t<type> operator->()                                                          \
		{                                                                                                       \
			return Get();                                                                                       \
		}                                                                                                       \
                                                                                                                \
	private:                                                                                                    \
		/* Prevent accidentally copying this wrapper class instead of the underlying field */                  \
		varName##_prop(const varName##_prop&) = delete;                                                         \
		static constexpr auto m_varNameHash = hash_32_fnv1a_const(#varName);                                    \
	} varName;

// Use this when you want the member's value itself
#define SCHEMA_FIELD(type, varName) \
	SCHEMA_FIELD_OFFSET(type, varName, 0)

#define DECLARE_SCHEMA_CLASS(ClassName)                                          \
private:                                                                         \
	typedef ClassName ThisClass;                                                 \
	static constexpr const char* m_className = #ClassName;                       \
	static constexpr uint32_t m_classNameHash = hash_32_fnv1a_const(#ClassName); \
                                                                                 \
public:
