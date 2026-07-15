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

#include "common.h"

#include "tier0/dbg.h"
#include "tier1/strtools.h"

#include <stdarg.h>

void Message(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(80, 200, 120, 255), "[HIDE] %s", buf);

	va_end(args);
}

void Panic(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	Warning("[HIDE] %s", buf);

	va_end(args);
}
