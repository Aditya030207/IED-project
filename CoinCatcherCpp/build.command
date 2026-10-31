#!/bin/sh
set -eu
cd "$(dirname "$0")"
APP="Coin Catcher.app"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"
xcrun clang++ -std=c++17 -O2 -Wall -Wextra -Wpedantic \
    -arch arm64 -arch x86_64 -mmacosx-version-min=11.0 \
    -Ithird_party/raylib/include main.cpp third_party/raylib/lib/libraylib.a \
    -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL -framework QuartzCore \
    -o "$APP/Contents/MacOS/CoinCatcher"
cat > "$APP/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
<key>CFBundleExecutable</key><string>CoinCatcher</string>
<key>CFBundleIdentifier</key><string>local.coincatcher.cpp</string>
<key>CFBundleName</key><string>Coin Catcher</string>
<key>CFBundleDisplayName</key><string>Coin Catcher C++</string>
<key>CFBundlePackageType</key><string>APPL</string>
<key>CFBundleShortVersionString</key><string>1.0</string>
<key>CFBundleVersion</key><string>1</string>
<key>NSHighResolutionCapable</key><true/>
<key>LSMinimumSystemVersion</key><string>11.0</string>
</dict></plist>
PLIST
xattr -rd com.apple.FinderInfo "$APP" 2>/dev/null || true
codesign --force --sign - "$APP"
"$APP/Contents/MacOS/CoinCatcher" --test
printf '\nBuilt Coin Catcher.app. Open it to play.\n'
