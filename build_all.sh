#!/bin/bash
# Builds every browser flavour:
#   build/web     index.html + index.js + index.wasm (local test server)
#   build/site    600seconds.js + 600seconds.wasm (sundorigin.org embed)
#   build/single  600Seconds.html, one file that runs from disk (itch.io + double-click)
set -e
RAYLIB_SRC=${RAYLIB_SRC:-../raylib/src}
FLAGS="-std=c++17 -O2 -Wall -DPLATFORM_WEB -I$RAYLIB_SRC -Isrc"
SRCS="src/main.cpp src/game/Story.cpp src/audio/Soundtrack.cpp $RAYLIB_SRC/libraylib.a"
LINK="-sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=67108864 -sEXPORTED_FUNCTIONS=_main,_OnResize,_malloc,_free -sEXPORTED_RUNTIME_METHODS=ccall --embed-file assets"
mkdir -p build/web build/site build/single
em++ $FLAGS $SRCS -o build/web/index.js $LINK
cp web/index.html build/web/index.html
em++ $FLAGS $SRCS -o build/site/600seconds.js $LINK
em++ $FLAGS $SRCS -o build/single/game.js $LINK -sSINGLE_FILE=1
python3 - <<'PY'
html = open('web/index.html').read()
js = open('build/single/game.js').read()
tag = '<script src="index.js"></script>'
assert tag in html
open('build/single/600Seconds.html', 'w').write(html.replace(tag, '<script>\n' + js + '\n</script>'))
PY
echo "Built build/web, build/site, build/single"
