#!/bin/bash
# Makes the zip you can play anywhere: a Windows exe and a one file browser version.
# PUBLIC=1 (v0.20, T-107) builds the exe without the debug panel. Use the same PUBLIC for build_all.sh.
# Needs: x86_64-w64-mingw32-g++, RAYLIB_WIN (raylib/src built for Windows) and a finished build_all.sh.
set -e
RAYLIB_WIN=${RAYLIB_WIN:-../raylib-win/src}
VER=$(sed -n 's/.*kVersion = "\(v[^"]*\)".*/\1/p' src/main.cpp)
OUT=build/play/600Seconds-$VER
rm -rf build/play && mkdir -p "$OUT"
x86_64-w64-mingw32-g++ -std=c++17 -O2 $([ "$PUBLIC" = "1" ] && echo -DPUBLIC_BUILD) -Isrc -I"$RAYLIB_WIN" src/main.cpp src/game/Story.cpp src/audio/Soundtrack.cpp \
  "$RAYLIB_WIN/libraylib.a" -lopengl32 -lgdi32 -lwinmm -static -static-libgcc -static-libstdc++ -mwindows -o "$OUT/600Seconds.exe"
cp -r assets "$OUT/assets"
cp build/single/600Seconds.html "$OUT/600Seconds.html"
cat > "$OUT/HOW TO PLAY.txt" <<TXT
600 Seconds $VER

Two ways to play. Unzip the folder first.

Windows: double click 600Seconds.exe. Windows may warn about an unknown app, because the exe isn't signed. Choose More info, then Run anyway.
Any computer: double click 600Seconds.html. It opens in your browser, no install.

Controls: WASD or arrows walk, Shift run, E use, Hold T wait, J memory log, Tab skip, P pause, F fullscreen.

The exe saves next to itself. The browser version saves in that browser.
TXT
(cd build/play && zip -qr "600Seconds-$VER-play.zip" "600Seconds-$VER")
ls -la "build/play/600Seconds-$VER-play.zip"
