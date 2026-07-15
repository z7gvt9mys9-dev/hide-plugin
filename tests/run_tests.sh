#!/bin/bash
# Builds and runs the adminlist unit test inside the build container:
#   docker run --rm -v "$PWD":/app/source hide-build bash tests/run_tests.sh
set -e
cd "$(dirname "$0")/.."

clang++ -std=c++20 -O1 -m64 \
  -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf \
  -DHAVE_STDINT_H -DGNUC -DCOMPILER_GCC -DNDEBUG -DLINUX -D_LINUX -DPOSIX \
  -DX64BITS -DPLATFORM_64BITS -D_GLIBCXX_USE_CXX11_ABI=0 \
  -Wno-invalid-offsetof -Wno-register -Wno-deprecated-register \
  -I. -Isrc \
  -I"$HL2SDKCS2/public" -I"$HL2SDKCS2/public/tier0" -I"$HL2SDKCS2/public/tier1" \
  tests/adminlist_test.cpp src/adminlist.cpp \
  "$HL2SDKCS2/lib/linux64/tier1.a" "$HL2SDKCS2/lib/linux64/libtier0.so" \
  -o /tmp/adminlist_test

LD_LIBRARY_PATH="$HL2SDKCS2/lib/linux64" /tmp/adminlist_test
