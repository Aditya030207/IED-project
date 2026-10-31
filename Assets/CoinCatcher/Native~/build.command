#!/bin/sh
set -eu
cd "$(dirname "$0")"
mkdir -p build ../../Plugins/macOS
xcrun clang++ -std=c++17 -O2 -Wall -Wextra -Wpedantic tests.cpp -o build/tests
./build/tests
xcrun clang++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -fvisibility=hidden \
  -dynamiclib -arch arm64 -arch x86_64 -mmacosx-version-min=11.0 \
  -install_name @rpath/libCoinCatcherNative.dylib CoinCatcher.cpp \
  -o ../../Plugins/macOS/libCoinCatcherNative.dylib
codesign --force --sign - ../../Plugins/macOS/libCoinCatcherNative.dylib
printf 'C++ library built. Restart Unity if the previous library was already loaded.\n'
