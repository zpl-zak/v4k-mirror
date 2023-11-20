<h1 align="center"><img src="https://v4k.dev/logo.png" alt="V·4·K"></img></h1>
<p align="center">
Envision, Prototype, Perfect<br/>
</p>

## Features
- [x] Pipeline: configurable and integrated [asset pipeline](tools/cook.ini).
- [x] Embedded: single-file header, all dependencies included.
- [x] Compiler: MSVC, MINGW64, TCC, GCC, clang, clang-cl and emscripten.
- [x] Linkage: Both static linkage and dynamic .dll/.so/.dylib support. 
- [x] Platform: Windows, Linux and OSX. Partial HTML5/Web support.
- [x] DS: hash, sort, array/vector, map, set.
- [x] Math: rand, noise, ease, vec2/3/4, mat33/34/44, quat.
- [x] Geometry: ray, line, plane, aabb, sphere, capsule, triangle, poly and frustum.
- [x] Window: windowed, soft/hard fullscreen, msaa, icon, cursor handling.
- [x] Input: keyboard, mouse and gamepads.
- [x] Script: Lua scripting, Luajit bindings.
- [x] Network: downloads (HTTPS) and sockets (TCP/UDP). <!-- [*] Object, GameObject, W/ECS -->
- [x] Network: Game sync module
- [x] AI: Swarm/Boids.
- [x] UI: button, list, slider, toggle, checkbox, editbox, dialog, color, image, menu, window, notify...
- [x] Font: TTF, OTF and TTC. Basic syntax highlighter. Glyph ranges. Atlasing.
- [x] Localization/I18N: XLSX and INI. Unicode.
- [x] Image: JPG, PNG, BMP, PSD, PIC, PNM, ICO.
- [x] Texture: KTX/2, PVR, DDS, ASTC, BASIS, HDR, TGA.
- [x] Texel: Depth, R, RG, RGB, RGBA, BC1/2/3/4/5/6/7, PVRI/II, ETC1/2, ASTC.
- [x] Audio: WAV/FLAC, OGG/MP1/MP3, FUR, MOD/XM/S3M/IT, SFXR and MID+SF2/SF3.
- [x] Video: MP4, MPG, OGV, MKV, WMV and AVI. Also, MP4 recording with MPEG-1 fallback.
- [x] Model: IQM/E, GLTF/2, GLB, FBX, OBJ, DAE, BLEND, MD3/5, MS3D, SMD, X, 3DS, BVH, DXF, LWO.
- [x] Render: PBR (metallic-roughness) workflow. <!-- @todo: merge demo_pbr.c rendering code into v4k_render.c -->
- [x] Render: Cubemaps, panoramas and spherical harmonics. Rayleigh/Mie scattering.
- [x] Render: Post-effects (SSAO,FXAA1/3,CRT,Contrast,Grain,Outline,Vignette...).
- [x] Render: 3D Anims, skeletal anims, hardware skinning and instanced rendering.
- [x] Render: 3D Debugdraw, batching and vectorial font.
- [x] Render: 2D Sprites, spritesheets, AA zooming and batching.
- [x] Render: 2D Tilemaps and tilesets: TMX, TSX.
- [x] Render: Compute shaders and SSBO support.
- [x] Render: Geometry shaders.
- [x] Compression: DEFLATE, LZMA, LZ4, ULZ, BALZ, BCM, CRUSH, LZW3, LZSS and PPP.
- [x] Virtual filesystem: ZIP, PAK, TAR and DIR.
- [x] Level data: JSON, JSON5, SJSON, XML, INI.
- [x] Disk cache.
- [x] Scene handling.
- [x] Profiler, stats and leaks finder.
- [x] [Documentation (wip)](https://v4k.dev).

## Hello V4K
```C
#include "v4k.h" // Minimal C sample
int main() {
    window_create(75.0, 0); // 75% size, no extra flags
    while( window_swap() && !input(KEY_ESC) ) { // game loop
        puts("hello V4K from C!");
    }
}
```

```lua
local v4k = require("v4k") -- Minimal Lua sample
v4k.window_create(75.0,0) -- 75% size, no extra flags
while v4k.window_swap() and v4k.input(v4k.KEY_ESC) == 0 do -- game loop
    print("hello V4K from Lua!")
end
```

## Quickstart
- Double-click `MAKE.bat` to quick start.
- `MAKE.bat all` to build everything.
- `MAKE.bat proj` to generate solutions/makefiles.
- `MAKE.bat help` for a bunch of options.
- `MAKE.bat hello.c` to build a single executable.
- Alternatively,
```bat
echo win/vc            && cl hello.c
echo win/clang-cl      && clang-cl hello.c
echo win/tcc           && tools\tcc hello.c -m64
echo win/mingw         && gcc   hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
echo win/clang         && clang hello.c -lws2_32 -lwinmm -ldbghelp -lole32 -luser32 -lgdi32 -lcomdlg32
echo linux             && cc  hello.c -lm -ldl -lpthread -lX11
echo linux/tcc         && tcc hello.c -lm -ldl -lpthread -lX11 -D__STDC_NO_VLA__
echo osx               && cc -ObjC hello.c -framework cocoa -framework iokit -framework audiotoolbox
```

## Cook
- Assets need to be cooked before being consumed in your application. The [tools/](tools/) folder contains all the related binaries to perform any asset processing plus the [cookbook](tools/cook.ini) to do so.
- Your game will cook all your assets as long as the [`tools/`](tools/) folder is next to your executable. Alternatively, cook them all just by invoking supplied [`tools/cook` standalone binary](tools/). 
- In both cases, assets will be cooked and packed into .zipfiles next to your executable, then mounted before entering game loop. These .zipfiles plus your executable are the only required files when releasing your game.

## Extra tips
- Any ico/png file named after the executable name will be automatically used as app icon.
- Similar to the ico/png case above, the cooked .zipfiles can be named after the main executable as well.
- Dropped files into game window will be imported & saved into [`import/`](engine/art/import/) folder.
- Update the gamepad controller database by upgrading the [`gamecontrollerdb.txt`](engine/art/input/) file.
- Depending on your IDE, you might need to browse to [`engine/split/`](engine/split/) sources when debugging FWK.
- Cook assets on demand, as opposed to cook all existing assets on depot, by using `--cook-on-demand` flag.
- Linux/OSX users can optionally install wine and use the Windows tools instead (by using `--cook-wine` flag).
- Disable automatic cooking by using `--cook-jobs=0` flag (not recommended).
- Generate a project solution by dropping `engine/v4k.h, v4k.c and v4k` files into it.
- Auto-generated Luajit and Python bindings can be found in the [`engine/bind/`](engine/bind/) folder.
<!-- - On windows + vc, you can use `make bindings` or `make docs` to generate everything prior to a release -->
<!-- - Note: Windows: Assimp.dll may need [this package installed](https://www.microsoft.com/en-us/download/confirmation.aspx?id=30679).-->

## Credits
**Original** Big big thanks to [r-lyeh](https://github.com/r-lyeh/fwk) for the amazing [FWK](https://github.com/r-lyeh/FWK) base!<br/><br/>
**Artwork**
[Dean Evans, Raijin](https://youtu.be/RRvYkrrpMKo?t=147) for the Map song (c),
[FMS_Cat](https://gist.github.com/FMS-Cat/a1ccea3ce866c34706084e3526204f4f) for nicest VHS/VCR shader around (MIT),
[Goblin165cm](https://sketchfab.com/3d-models/halloween-little-witch-ccc023590bfb4789af9322864e42d1ab) for witch 3D model (CC BY 4.0),
[Nuulbee](https://sketchfab.com/3d-models/kgirls01-d2f946f58a8040ae993cda70c97b302c) for kgirls01 3D model (CC BY-NC-ND 4.0),
[Quaternius](https://www.patreon.com/quaternius) for the lovely 3D robots (CC0),
[Rotting Pixels](https://opengameart.org/content/2d-castle-platformer-tileset-16x16) for castle-tileset (CC0),
[Tom Lewandowski](https://QuestStudios.com) for his MIDI recordings (c),
[Rye Terrell](https://github.com/wwwtyro/glsl-atmosphere) for nicest rayleigh/mie scattering shader around (CC0),
**Tools**
[Aaron Barany](https://github.com/akb825/Cuttlefish) for cuttlefish (APACHE2),
[Arseny Kapoulkine](https://github.com/zeux/pugixml/) for pugixml (MIT),
[Assimp authors](https://github.com/assimp/assimp) for assimp (BSD3),
[Bernhard Schelling](https://github.com/schellingb/TinySoundFont) for tml.h (Zlib) and tsf.h (MIT),
[Christian Collins](http://www.schristiancollins.com) for GeneralUser GS soundfont (PD),
[FFMPEG authors](https://www.ffmpeg.org/) for ffmpeg (LGPL21),
[Imagination](https://developer.imaginationtech.com/pvrtextool/) for pvrtextoolcli (ITL),
[Krzysztof Gabis](https://github.com/kgabis/ape) for split.py/join.py (MIT),
[Lee Salzman](https://github.com/lsalzman/iqm/tree/5882b8c32fa622eba3861a621bb715d693573420/demo) for iqm.cpp (PD),
[Martín Lucas Golini](https://github.com/SpartanJ/eepp/commit/8552941da19380d7a629c4da80a976aec5d39e5c) for emscripten-fs.html (CC0),
[Mattias Gustavsson](https://github.com/mattiasgustavsson/libs) for mid.h (PD),
[Michael Schmoock](http://github.com/willsteel/lcpp) for lcpp (MIT),
[Morgan McGuire](https://casual-effects.com/markdeep/) for markdeep (BSD2),
[Olivier Lapicque, Konstanty Bialkowski](https://github.com/Konstanty/libmodplug) for libmodplug (PD),
[Polyglot Team](https://docs.google.com/spreadsheets/d/17f0dQawb-s_Fd7DHgmVvJoEGDMH_yoSd8EYigrb0zmM/edit) for polyglot gamedev (CC0),
[Tildearrow](https://github.com/tildearrow/furnace/) for Furnace (GPL2),
[Tomas Pettersson](http://www.drpetter.se/) for sfxr (PD),
[Tor Andersson](https://github.com/ccxvii/asstools) for assiqe.c (BSD),
**Runtime**
[Andreas Mantler](https://github.com/ands) for their math library (PD), 
[Barerose](https://github.com/barerose) for swrap (CC0) and math library (CC0),
[Camilla Löwy](https://github.com/elmindreda) for glfw3 (Zlib),
[Dave Rand](https://tools.ietf.org/html/rfc1978) for ppp (PD),
[David Herberth](https://github.com/dav1dde/) for glad generated code (PD),
[David Reid](https://github.com/mackron) for miniaudio (PD),
[Dominic Szablewski](https://github.com/phoboslab/pl_mpeg) for pl_mpeg (MIT),
[Dominik Madarász](https://github.com/zaklaus) for json5 parser (PD),
[Eduard Suica](https://github.com/eduardsui/tlse) for tlse (PD),
[Evan Wallace](https://github.com/evanw) for their math library (CC0), 
[Gargaj+cce/Peisik](https://github.com/gargaj/foxotron) for Foxotron/PBR shaders (UNLICENSE),
[Guilherme Lampert](https://github.com/glampert) for their math library (PD), 
[Guillaume Vareille](http://tinyfiledialogs.sourceforge.net) for tinyfiledialogs (ZLIB),
[Haruhiko Okumura](https://oku.edu.mie-u.ac.jp/~okumura/compression/) for lzss (PD),
[Igor Pavlov](https://www.7-zip.org/) for LZMA (PD),
[Ilya Muravyov](https://github.com/encode84) for bcm, balz, crush, ulz, lz4x (PD),
[Jon Olick](https://www.jonolick.com/) for jo_mp1 and jo_mpeg (PD),
[Joonas Pihlajamaa](https://github.com/jokkebk/JUnzip) for JUnzip library (PD),
[Juliette Focault](https://github.com/juliettef/IconFontCppHeaders/blob/main/IconsMaterialDesign.h) for the generated MD header (ZLIB),
[Kristoffer Grönlund](https://github.com/krig) for their math library (CC0), 
[Lee Salzman](https://github.com/lsalzman/iqm/tree/5882b8c32fa622eba3861a621bb715d693573420/demo) for IQM spec & player (PD),
[Lee Salzman, V.Hrytsenko, D.Madarász](https://github.com/zpl-c/enet/) for enet (MIT),
[Libtomcrypt](https://github.com/libtom/libtomcrypt) for libtomcrypt (Unlicense),
[Lua authors](https://www.lua.org/) for Lua language (MIT),
[Mattias Gustavsson](https://github.com/mattiasgustavsson/libs) for thread.h and https.h (PD),
[Micha Mettke](https://github.com/vurtun) for their math library (PD),
[Micha Mettke, Chris Willcocks, Dmitry Hrabrov](https://github.com/vurtun/nuklear) for nuklear (PD),
[Michael Galetzka](https://github.com/Cultrarius/Swarmz) for swarmz (UNLICENSE),
[Morten Vassvik](https://github.com/vassvik/mv_easy_font) for mv_easy_font (Unlicense),
[Mārtiņš Možeiko](https://gist.github.com/mmozeiko/68f0a8459ef2f98bcd879158011cc275) for A* pathfinding (PD),
[Omar Cornut, vaiorabbit](https://github.com/ocornut/imgui/pull/3627) for tables of unicode ranges (MIT-0),
[Rabia Alhaffar](https://github.com/Rabios/ice_libs) for ice_batt.h (PD),
[Rich Geldreich](https://github.com/richgel999/miniz) for miniz (PD),
[Ross Williams](http://ross.net/compression/lzrw3a.html) for lzrw3a (PD),
[Samuli Raivio](https://github.com/bqqbarbhg/bq_websocket) for bq_websocket (PD),
[Sean Barrett](https://github.com/nothings) for stb_image, stb_image_write, stb_sprintf, stb_truetype and stb_vorbis (PD),
[Sebastian Steinhauer](https://github.com/kieselsteini) for sts_mixer (PD),
[Stan Melax, Cloud Wu](https://web.archive.org/web/20031204035320/http://www.melax.com/polychop/gdmag.pdf) for polychop C algorithm (PD),
[Stefan Gustavson](https://github.com/stegu/perlin-noise) for simplex noise (PD),
[Sterling Orsten](https://github.com/sgorsten) for their math library (UNLICENSE),
[Tor Andersson](https://github.com/ccxvii/minilibs) for xml.c (PD),
[Wolfgang Draxinger](https://github.com/datenwolf) for their math library (WTFPL2), 

<!--
- [DavidLam](https://en.wikipedia.org/wiki/Tokamak_(software) "for tokamak physics engine (ZLIB)")
- [ID Software, David St-Louis](https://github.com/Daivuk/PureDOOM "for PureDOOM (Doom License)")
- [Miloslav Číž](https://codeberg.org/drummyfish/Anarch "for Anarch (CC0)")
- [Rxi](https://github.com/rxi/autobatch "for lovely sprites & cats demo (MIT)")
-->

## Unlicense
This software is released into the [public domain](https://unlicense.org/). Also dual-licensed as [0-BSD](https://opensource.org/licenses/0BSD) or [MIT (No Attribution)](https://github.com/aws/mit-0) for those countries where public domain is a concern (sigh). Any contribution to this repository is implicitly subjected to the same release conditions aforementioned.
