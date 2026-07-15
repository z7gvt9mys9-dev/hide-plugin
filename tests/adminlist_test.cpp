// Standalone test for CAdminList: verifies the admins.json format
// (object keyed by SteamID64) against real-world content.
// Build/run: see tests/run_tests.sh
#include "adminlist.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

// Stubs for the plugin's logging (the real ones live in utils.cpp and need tier0)
void Message(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
}

void Panic(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
}

static int g_iFailures = 0;

#define CHECK(cond)                                                \
	do                                                             \
	{                                                              \
		if (!(cond))                                               \
		{                                                          \
			printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
			g_iFailures++;                                         \
		}                                                          \
	} while (0)

static void WriteFile(const char* pszPath, const char* pszContent)
{
	std::ofstream f(pszPath);
	f << pszContent;
}

int main()
{
	// --- ParseSteamID64 ---
	CHECK(CAdminList::ParseSteamID64("76561198724970203") == 76561198724970203ULL);
	// Anything that is not a full SteamID64 must be rejected
	CHECK(CAdminList::ParseSteamID64("100") == 0);
	CHECK(CAdminList::ParseSteamID64("65626c616e") == 0);
	CHECK(CAdminList::ParseSteamID64("STEAM_1:1:382352237") == 0);
	CHECK(CAdminList::ParseSteamID64("76561198724970203x") == 0);
	CHECK(CAdminList::ParseSteamID64("") == 0);

	// --- The admins.json format ---
	WriteFile("/tmp/admins.json", R"({
  "76561198724970203": {
    "name": "65626c616e",
    "immunity": 100,
    "permissions": null,
    "groups": null
  },
  "76561197960287930": {
    "name": "gabe",
    "immunity": 90,
    "permissions": ["@css/generic"],
    "groups": ["#css/admins"]
  }
})");
	{
		CAdminList list;
		CHECK(list.LoadFromFile("/tmp/admins.json"));
		CHECK(list.Count() == 2);
		CHECK(list.IsAdmin(76561198724970203ULL));
		CHECK(list.IsAdmin(76561197960287930ULL));
		CHECK(!list.IsAdmin(100ULL));
		CHECK(!list.IsAdmin(76561198000000000ULL));
	}

	// --- Bad keys are skipped, valid ones still load ---
	WriteFile("/tmp/admins_mixed.json", R"({
  "not_a_steamid": { "name": "x" },
  "76561198724970203": { "name": "ok" }
})");
	{
		CAdminList list;
		CHECK(list.LoadFromFile("/tmp/admins_mixed.json"));
		CHECK(list.Count() == 1);
		CHECK(list.IsAdmin(76561198724970203ULL));
	}

	// --- Malformed / wrong-shape files must not crash and must fail ---
	WriteFile("/tmp/admins_bad.json", "{ not valid json ");
	WriteFile("/tmp/admins_array.json", "[\"76561198724970203\"]");
	{
		CAdminList list;
		CHECK(!list.LoadFromFile("/tmp/admins_bad.json"));
		CHECK(!list.LoadFromFile("/tmp/admins_array.json"));
		CHECK(!list.LoadFromFile("/tmp/no_such_file.json"));
		CHECK(list.Count() == 0);
	}

	if (g_iFailures)
	{
		printf("%d check(s) FAILED\n", g_iFailures);
		return 1;
	}

	printf("All checks passed\n");
	return 0;
}
