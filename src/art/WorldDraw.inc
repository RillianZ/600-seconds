// 600 Seconds - art/WorldDraw.inc (v0.11)
// The frame: baked ground, things that change on top of it, people, roofs, weather, then light.
// Order matters: world -> roofs -> snow -> light map (multiply) -> glow (add) -> vignette.

static Texture2D groundTex, southTex, roofTex, shedRoofTex, glowTex, vigTex;
static bool viewBelow = false;                         // the camera is down in the basement
static float roofA = 1, shedRoofA = 1;                 // 1 = roof on, 0 = you are inside and it has lifted away
static const int ROOF_X = 2 * TS - 3, ROOF_Y = 2 * TS - 9, ROOF_W = 28 * TS + 5, ROOF_H = 18 * TS + 14;
static const int SROOF_X = 31 * TS - 3, SROOF_Y = 4 * TS - 9, SROOF_W = 7 * TS + 5, SROOF_H = 6 * TS + 14;
static const int CHIM_X = 400, CHIM_Y = 232;           // inside the roof texture, over the wood stove
#include "Backyard.inc"

static Texture2D TexFrom(const art::Canvas& c, bool smooth = false) {
    Image img{ (void*)c.px.data(), c.w, c.h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    Texture2D t = LoadTextureFromImage(img);
    SetTextureFilter(t, smooth ? TEXTURE_FILTER_BILINEAR : TEXTURE_FILTER_POINT);
    SetTextureWrap(t, TEXTURE_WRAP_CLAMP);
    return t;
}
static void LoadArt() {
    art::Canvas c;
    art::WorldBaker(story.world, c).Bake();
    groundTex = TexFrom(c);
    art::BakeSouth(story.world, c, 12); southTex = TexFrom(c);
    art::BakeRoof(c, ROOF_W, ROOF_H, CHIM_X, CHIM_Y, 201); roofTex = TexFrom(c);
    art::BakeRoof(c, SROOF_W, SROOF_H, -1, 0, 301); shedRoofTex = TexFrom(c);
    // one soft round light, used for every lamp, glow and beam
    c.Init(64, 64);
    for (int y = 0; y < 64; ++y) for (int x = 0; x < 64; ++x) {
        float d = std::hypot(x - 31.5f, y - 31.5f) / 31.5f, k = std::max(0.0f, 1 - d);
        k = k * k * (3 - 2 * k);
        c.Set(x, y, Color{ 255, 255, 255, (unsigned char)(255 * k * k) });
    }
    glowTex = TexFrom(c, true);
    // the vignette: dark corners, dithered so it never bands
    c.Init(VW, VH);
    for (int y = 0; y < VH; ++y) for (int x = 0; x < VW; ++x) {
        float dx = (x - VW / 2.0f) / (VW / 2.0f), dy = (y - VH / 2.0f) / (VH / 2.0f);
        float d = std::sqrt(dx * dx * 0.85f + dy * dy), k = std::max(0.0f, (d - 0.62f) / 0.75f);
        k = std::min(1.0f, k * k * 1.5f) * 0.62f;
        float th = (art::kBayer[y & 3][x & 3] + 0.5f) / 16.0f;
        c.Set(x, y, Color{ 6, 6, 20, (unsigned char)(255 * std::min(1.0f, std::floor(k * 24 + th) / 24.0f)) });
    }
    vigTex = TexFrom(c);
}
static void UnloadArt() { UnloadTexture(groundTex); UnloadTexture(southTex); UnloadTexture(roofTex); UnloadTexture(shedRoofTex); UnloadTexture(glowTex); UnloadTexture(vigTex); }

static bool InHouse(float px, float py) { int tx = TX(px), ty = TX(py); return tx >= 2 && tx <= 29 && ty >= 2 && ty <= 20; }
static bool InShed(float px, float py) { int tx = TX(px), ty = TX(py); return tx >= 31 && tx <= 37 && ty >= 4 && ty <= 10; }
static bool Lamps() { return !story.PowerOut(); }
static float StormFlick() {
    if (!story.Storm() || settings.calmFlashes) return 1;
    float f = std::sin(globalT * 23) * std::sin(globalT * 7.3f);
    return f > 0.85f ? 0.45f : 1;
}
// a soft light in world space. Call inside BLEND_ADDITIVE
static void Glow(float wx, float wy, float rx, float ry, Color c, float k) {
    if (k <= 0.01f) return;
    k = std::min(1.0f, k);
    DrawTexturePro(glowTex, { 0, 0, 64, 64 }, { wx - rx, wy - ry, rx * 2, ry * 2 }, { 0, 0 }, 0, Color{ c.r, c.g, c.b, (unsigned char)(255 * k) });
}

// ---------------------------------------------------------------- the parts of a tile that change
static void DrawWindowS(int px, int py, int seed, bool small) {
    bool on = Lamps();
    Color trim = { 236, 230, 216, 255 };
    int w = small ? 10 : 14, ox = small ? 3 : 1;
    DrawRectangle(px + ox - 1, py + 5, w + 2, 11, Color{ 60, 24, 28, 255 });
    DrawRectangle(px + ox, py + 6, w, 9, trim);
    if (on) {
        float fl = StormFlick();
        Color a = { (unsigned char)(255 * fl), (unsigned char)(226 * fl), (unsigned char)(150 * fl), 255 }, b = { (unsigned char)(250 * fl), (unsigned char)(180 * fl), (unsigned char)(84 * fl), 255 };
        DrawRectangleGradientV(px + ox + 1, py + 7, w - 2, 7, a, b);
        if (!small) { DrawRectangle(px + ox + 1, py + 7, 2, 7, Color{ 190, 70, 64, 255 }); DrawRectangle(px + ox + w - 3, py + 7, 2, 7, Color{ 190, 70, 64, 255 }); }   // curtains
    } else {
        DrawRectangle(px + ox + 1, py + 7, w - 2, 7, Color{ 20, 26, 48, 255 });
        DrawRectangle(px + ox + 2, py + 8, 3, 1, Fade(Color{ 150, 170, 230, 255 }, 0.5f)); DrawRectangle(px + ox + 3, py + 9, 1, 1, Fade(Color{ 150, 170, 230, 255 }, 0.35f));
    }
    DrawRectangle(px + ox + w / 2, py + 7, 1, 7, trim);
    if (!small) {
        // the advent candle bridge. Bulbs start dying once you've gone dark
        for (int k = 0; k < 7; ++k) {
            int dy = std::abs(k - 3);
            bool dead = !on || (story.tainted && (k + seed) % 3 == 0);
            DrawRectangle(px + 2 + k * 2, py + 9 + dy, 1, 5 - dy, Color{ 244, 240, 230, 255 });
            DrawRectangle(px + 2 + k * 2, py + 8 + dy, 1, 1, dead ? Color{ 90, 84, 80, 255 } : Color{ 255, 252, 210, 255 });
        }
    } else DrawRectangle(px + ox + 1, py + 10, w - 2, 1, trim);
    DrawRectangle(px + ox - 1, py + 15, w + 2, 1, Color{ 246, 250, 255, 255 });      // snow on the sill
    DrawRectangle(px + ox + 1, py + 14, 4, 1, Color{ 246, 250, 255, 255 });
}
static void DrawTileFx(int x, int y) {
    Tile t = story.world.tiles[y][x];
    int px = x * TS, py = y * TS;
    unsigned h = Hash(x, y);
    switch (t) {
    case Tile::Vidda: case Tile::Grass:
        if (h % 17 == 0 && !story.Storm() && std::fmod(globalT + h % 7, 3.0f) < 0.4f) DrawRectangle(px + h % 13 + 1, py + (h >> 3) % 13 + 1, 1, 1, WHITE);   // a crystal blinks
        break;
    case Tile::Wall:
        if (y == 20 && (x == 8 || x == 15 || x == 25)) DrawWindowS(px, py, x, false);
        if (y == 10 && (x == 32 || x == 36)) DrawWindowS(px, py, x, true);
        if (y == 2 && (x == 6 || x == 13 || x == 20)) {
            // v0.15: the back windows, seen over the sill from the backyard. Lit while the power is on
            bool on = Lamps();
            DrawRectangle(px + 2, py, 12, 4, Color{ 236, 230, 216, 255 });
            DrawRectangle(px + 3, py + 1, 10, 2, on ? Color{ 255, 214, 140, 255 } : Color{ 30, 38, 66, 255 });
            DrawRectangle(px + 7, py + 1, 2, 2, Color{ 236, 230, 216, 255 });
            DrawRectangle(px + 1, py - 1, 14, 1, Color{ 246, 250, 255, 255 });
        }
        if (y == 20 && x == 21) {
            // the wreath next to the front door
            DrawCircle(px + 8, py + 10, 5.5f, Color{ 20, 44, 36, 255 });
            DrawCircle(px + 8, py + 10, 4.5f, Color{ 40, 96, 66, 255 });
            DrawCircle(px + 8, py + 10, 2.0f, Color{ 138, 50, 42, 255 });
            for (int k = 0; k < 5; ++k) DrawRectangle(px + 8 + (int)(std::cos(k * 1.26f) * 4), py + 10 + (int)(std::sin(k * 1.26f) * 4), 1, 1, k % 2 ? Color{ 230, 60, 60, 255 } : Color{ 96, 156, 110, 255 });
            DrawRectangle(px + 6, py + 13, 5, 2, Color{ 206, 40, 44, 255 }); DrawRectangle(px + 8, py + 13, 1, 3, Color{ 150, 24, 30, 255 });
        }
        if (y == 20 && x == 19) {
            // the porch lamp
            DrawRectangle(px + 10, py + 6, 5, 6, Color{ 24, 22, 28, 255 });
            DrawRectangle(px + 11, py + 7, 3, 4, Lamps() ? Color{ 255, 236, 170, 255 } : Color{ 60, 62, 80, 255 });
            DrawRectangle(px + 10, py + 5, 5, 1, Color{ 246, 250, 255, 255 });
        }
        break;
    case Tile::Door: {
        bool horiz = IsWallT(x - 1, y) || IsWallT(x + 1, y);
        bool outer = IsOutsideT(TileAt(x, y + 1)) || IsOutsideT(TileAt(x, y - 1));
        Color frame = outer ? Color{ 232, 226, 212, 255 } : Color{ 96, 64, 42, 255 };
        if (horiz) { DrawRectangle(px, py, 2, TS, frame); DrawRectangle(px + TS - 2, py, 2, TS, frame); DrawRectangle(px + 1, py, 1, TS, Shade(frame, -30)); DrawRectangle(px + TS - 2, py, 1, TS, Shade(frame, -30)); }
        else { DrawRectangle(px, py, TS, 2, frame); DrawRectangle(px, py + TS - 2, TS, 2, frame); DrawRectangle(px, py + 1, TS, 1, Shade(frame, -30)); }
        if (story.DoorClosed(x, y)) {
            Color d = outer ? Color{ 44, 86, 70, 255 } : Color{ 128, 86, 54, 255 };      // the front door is green
            DrawRectangle(px + 1, py + 1, TS - 2, TS - 2, Mix(d, kInk, 0.7f));
            DrawRectangle(px + 2, py + 2, TS - 4, TS - 4, d);
            DrawRectangle(px + 2, py + 2, TS - 4, 1, Shade(d, 28)); DrawRectangle(px + TS - 3, py + 3, 1, TS - 5, Shade(d, -24));
            DrawRectangle(px + 4, py + 4, 3, 4, Shade(d, -18)); DrawRectangle(px + 9, py + 4, 3, 4, Shade(d, -18));
            DrawRectangle(px + 4, py + 9, 3, 4, Shade(d, -18)); DrawRectangle(px + 9, py + 9, 3, 4, Shade(d, -18));
            DrawRectangle(px + TS - 5, py + 8, 2, 2, Color{ 226, 200, 110, 255 });
            if (story.DoorLocked(x, y)) {   // a little brass lock plate
                DrawRectangle(px + TS - 7, py + 10, 4, 4, Color{ 196, 164, 72, 255 });
                DrawRectangle(px + TS - 6, py + 11, 1, 2, Color{ 40, 30, 20, 255 });
            }
        } else if (story.DoorBroken(x, y)) {
            DrawRectangle(px + 1, py + 2, 3, 2, Color{ 132, 90, 56, 255 }); DrawRectangle(px + 10, py + 11, 4, 2, Color{ 112, 74, 46, 255 });
            DrawRectangle(px + 5, py + 13, 2, 1, Color{ 150, 110, 70, 255 }); DrawRectangle(px + 12, py + 3, 2, 3, Color{ 92, 60, 36, 255 });
            DrawRectangle(px + 7, py + 6, 1, 3, Color{ 190, 150, 100, 255 });
        } else if (!horiz || !outer) {
            // open: the door leaf stands against the frame
            if (horiz) { DrawRectangle(px + 2, py + 1, 2, TS - 3, Color{ 128, 86, 54, 255 }); DrawRectangle(px + 2, py + 1, 1, TS - 3, Color{ 160, 114, 74, 255 }); }
        }
    } break;
    case Tile::BasementDoor:
        if (story.BasementOpen()) {
            DrawRectangle(px, py, TS, TS, Color{ 8, 10, 24, 255 });
            float pulse = 0.5f + 0.5f * std::sin(globalT * 4);
            for (int k = 0; k < 4; ++k) DrawRectangle(px + 2 + k, py + 2 + k * 3, TS - 4 - k * 2, 2, Fade(Color{ 80, 150, 255, 255 }, (0.5f - k * 0.1f) * (0.5f + 0.5f * pulse)));
        } else {
            DrawRectangle(px + 1, py + 1, TS - 2, TS - 2, Color{ 40, 42, 52, 255 });
            DrawRectangle(px + 2, py + 2, TS - 4, TS - 4, Color{ 98, 102, 114, 255 });
            DrawRectangle(px + 2, py + 2, TS - 4, 1, Color{ 140, 144, 158, 255 }); DrawRectangle(px + TS - 3, py + 3, 1, TS - 5, Color{ 70, 74, 86, 255 });
            for (int k = 0; k < 4; ++k) DrawRectangle(px + 3 + (k % 2) * 9, py + 3 + (k / 2) * 9, 1, 1, Color{ 50, 52, 62, 255 });     // rivets
            DrawRectangle(px + TS - 6, py + 7, 3, 3, Color{ 214, 198, 120, 255 }); DrawRectangle(px + TS - 5, py + 8, 1, 1, kInk);
        }
        break;
    default: break;
    }
}

// ---------------------------------------------------------------- footprints, breath, smoke
static float printCD[3] = { 0, 0, 0 };
static float vPrintCD = 0;
static int printSide = 0;
static void AddPrint(const Actor& a, int who, float dt, float every) {
    printCD[who] -= dt;
    if (printCD[who] > 0 || a.anim <= 0) return;
    if (!IsOutsideT(TileAt(TX(a.pos.x), TX(a.pos.y + 6)))) return;
    printCD[who] = every;
    printSide = 1 - printSide;
    prints.push_back({ { a.pos.x, a.pos.y + 6 }, 0, who, printSide });
    if (prints.size() > 500) prints.erase(prints.begin());
}
// v0.15.2 (Noah): your prints stay a lot longer, even in the storm, so he can follow you out to a hiding spot
static float PrintLife(int who) { return who == 3 ? 45.0f : who == 0 ? 120.0f : story.Storm() ? 30.0f : 150.0f; }
static void UpdatePrints(float dt) {
    for (auto& p : prints) p.t += dt;
    prints.erase(std::remove_if(prints.begin(), prints.end(), [&](const Print& p) { return p.t > PrintLife(p.who); }), prints.end());
}
static void DrawPrints() {
    Color c = { 112, 130, 186, 255 }, lip = { 250, 252, 255, 255 };
    for (auto& p : prints) {
        float a = 0.9f * (1 - p.t / PrintLife(p.who));
        int ox = p.side ? 2 : -2, x = (int)p.p.x, y = (int)p.p.y;
        if (p.who == 3) {   // v0.12: wet boots on the floor, drying
            float w = 0.5f * (1 - p.t / 45.0f);
            DrawRectangle(x + ox - 1, y, 3, 2, Fade(Color{ 20, 30, 52, 255 }, w)); DrawRectangle(x + ox, y, 1, 1, Fade(Color{ 190, 215, 245, 255 }, w * 0.8f));
        }
        else if (p.who == 1) { DrawRectangle(x + ox - 1, y, 4, 2, Fade(c, a)); DrawRectangle(x + ox - 1, y + 2, 4, 1, Fade(lip, a * 0.7f)); DrawRectangle(x + ox - 1, y - 2, 3, 1, Fade(c, a * 0.8f)); }
        else if (p.who == 2) DrawRectangle(x + ox / 2, y, 1, 1, Fade(c, a));
        else { DrawRectangle(x + ox, y, 2, 1, Fade(c, a)); DrawRectangle(x + ox, y + 1, 2, 1, Fade(lip, a * 0.6f)); }
    }
}
struct Puff { Vector2 p; float t; int kind; Vector2 v; };   // kind: 0 kicked up snow or dust, 1 breath, 2 chimney smoke
static std::vector<Puff> puffs;
static float puffTimer = 0, breathT = 0, smokeT = 0;
static Color LerpC(Color a, Color b, float t) { return Mix(a, b, std::max(0.0f, std::min(1.0f, t))); }

static void UpdateParticles(float dt) {
    bool storm = story.Storm();
    puffTimer -= dt;
    if (kidRunning && puffTimer <= 0) { puffs.push_back({ { player.pos.x + (float)GetRandomValue(-3, 3), player.pos.y + 6 }, 0, 0, { 0, -6 } }); puffTimer = 0.09f; }
    // breath in the cold, for everyone who is out in it
    breathT -= dt;
    if (breathT <= 0) {
        breathT = kidRunning ? 0.9f : 2.3f;
        const Actor* who[4] = { &player, &visitor, &mom, &dad };
        for (int i = 0; i < 4; ++i) {
            const Actor& a = *who[i];
            if (!a.visible || (i == 0 && hidden) || a.pos.y >= 34 * TS || !IsOutsideT(TileAt(TX(a.pos.x), TX(a.pos.y)))) continue;
            float h = i == 0 ? 9.0f : i == 1 ? 17.0f : 13.0f;
            float dx = a.dir == 2 ? -5.0f : a.dir == 3 ? 5.0f : 0.0f;
            puffs.push_back({ { a.pos.x + dx, a.pos.y - h + (a.dir == 0 ? 2 : 0) }, 0, 1, { dx * 1.2f + (storm ? 30.0f : 2.0f), -4 } });
        }
    }
    smokeT -= dt;
    if (smokeT <= 0 && !viewBelow) {
        smokeT = storm ? 0.1f : 0.22f;
        puffs.push_back({ { ROOF_X + CHIM_X + 5.0f + GetRandomValue(-1, 1), ROOF_Y + CHIM_Y - 1.0f }, 0, 2, { storm ? 46.0f + GetRandomValue(0, 20) : 5.0f + GetRandomValue(0, 4), storm ? -5.0f : -11.0f } });
    }
    for (auto& p : puffs) { p.t += dt; p.p.x += p.v.x * dt; p.p.y += p.v.y * dt; if (p.kind == 2) p.v.x += (storm ? 20.0f : 1.5f) * dt; }
    puffs.erase(std::remove_if(puffs.begin(), puffs.end(), [](const Puff& p) { return p.t > (p.kind == 2 ? 5.0f : p.kind == 1 ? 1.1f : 0.45f); }), puffs.end());
}
static void DrawPuffs(int kind) {
    for (auto& p : puffs) {
        if (p.kind != kind) continue;
        if (kind == 0) {
            Color c = IsOutsideT(TileAt(TX(p.p.x), TX(p.p.y))) ? Color{ 246, 248, 252, 255 } : Color{ 220, 210, 190, 255 };
            DrawCircle((int)p.p.x, (int)p.p.y, 1.5f + p.t * 6, Fade(c, 0.5f * (1 - p.t / 0.45f)));
        } else if (kind == 1) {
            float k = p.t / 1.1f;
            DrawCircle((int)p.p.x, (int)p.p.y, 1.0f + k * 2.5f, Fade(Color{ 236, 242, 255, 255 }, 0.4f * (1 - k)));
        } else {
            float k = p.t / 5.0f;
            float a = std::min(1.0f, p.t * 3) * (1 - k) * 0.42f * roofA;
            DrawCircle((int)p.p.x, (int)p.p.y, 2.0f + k * 6.0f, Fade(LerpC(Color{ 178, 180, 198, 255 }, Color{ 226, 232, 246, 255 }, k * 2), a * 0.9f));
        }
    }
}

// a string of coloured bulbs along the eave. It is Christmas Eve after all
static void DrawEaveLights(bool glow) {
    if (roofA < 0.02f) return;
    bool on = Lamps();
    static const Color col[4] = { { 255, 214, 110, 255 }, { 250, 84, 76, 255 }, { 110, 180, 255, 255 }, { 120, 226, 136, 255 } };
    int y = ROOF_Y + ROOF_H;
    for (int i = 0, x = ROOF_X + 7; x < ROOF_X + ROOF_W - 4; x += 11, ++i) {
        int sag = (i % 2) ? 1 : 0;
        bool lit = on && !(story.tainted && i % 3 == 0) && std::fmod(globalT * 0.5f + (i % 4) * 0.25f, 1.0f) < 0.82f;
        Color c = story.tainted ? Color{ 230, 40, 40, 255 } : col[i % 4];
        if (glow) { if (lit) Glow(x + 1.0f, y + sag + 1.0f, 6, 6, c, 0.55f * roofA * StormFlick()); continue; }
        DrawRectangle(x - 5, y + (sag ? 0 : 1), 5, 1, Fade(Color{ 30, 34, 40, 255 }, roofA)); DrawRectangle(x + 2, y + (sag ? 1 : 0), 4, 1, Fade(Color{ 30, 34, 40, 255 }, roofA));
        DrawRectangle(x, y + sag, 2, 2, Fade(lit ? c : Mix(c, Color{ 30, 34, 50, 255 }, 0.75f), roofA));
        if (lit) DrawRectangle(x, y + sag, 1, 1, Fade(WHITE, 0.7f * roofA));
    }
}

// it snows the whole hour. Three depths, and the storm turns it sideways
static void DrawSnow(Camera2D cam) {
    float left = cam.target.x - VW / 2.0f, top = cam.target.y - VH / 2.0f;
    bool storm = story.Storm();
    auto covered = [&](float wx, float wy) {
        if (wx >= 2 * TS && wx < 30 * TS && wy >= 2 * TS + 6 && wy < 20 * TS + 5) return roofA < 0.5f;
        if (wx >= 31 * TS && wx < 38 * TS && wy >= 4 * TS + 6 && wy < 10 * TS + 5) return shedRoofA < 0.5f;
        return false;
    };
    static const int nCalm[3] = { 110, 70, 26 }, nStorm[3] = { 260, 220, 90 };
    for (int layer = 0; layer < 3; ++layer) {
        int n = storm ? nStorm[layer] : nCalm[layer];
        float depth = 0.55f + layer * 0.3f;                           // far flakes fall slower and drift less
        float wind = (storm ? 190.0f : 8.0f) * depth, fall = (storm ? 150.0f : 22.0f) * depth;
        float par = 1.0f - (2 - layer) * 0.12f;                       // and slide a little against the camera
        for (int i = 0; i < n; ++i) {
            unsigned h1 = Hash(i, 11 + layer), h2 = Hash(i, 23 + layer), h3 = Hash(i, 37 + layer);
            float sway = storm ? std::sin(globalT * 6 + i) * 2.0f : std::sin(globalT * (0.7f + (h3 % 7) * 0.1f) + i) * (3.0f + layer * 2.5f);
            float sx = std::fmod((float)(h1 % 1009) + globalT * wind * (0.8f + (h3 % 5) * 0.1f) - left * par + 80000.0f, (float)VW + 24) - 12 + sway;
            float sy = std::fmod((float)(h2 % 997) + globalT * (fall + (h3 % 30)) - top * par + 80000.0f, (float)VH + 24) - 12;
            float wx = left + sx, wy = top + sy;
            if (covered(wx, wy)) continue;
            float a = layer == 0 ? 0.32f : layer == 1 ? 0.55f : 0.8f;
            if (storm) DrawRectangle((int)wx, (int)wy, 2 + layer * 2, 1, Fade(WHITE, a * 0.85f));
            else if (layer == 2) { DrawRectangle((int)wx, (int)wy, 2, 2, Fade(WHITE, a)); if (h3 % 3 == 0) DrawRectangle((int)wx - 1, (int)wy, 4, 1, Fade(WHITE, 0.3f)); }
            else DrawRectangle((int)wx, (int)wy, 1, 1, Fade(WHITE, a));
        }
    }
}

// ---------------------------------------------------------------- light
static void BuildLightmap(Camera2D cam) {
    viewBelow = player.pos.y >= 34 * TS;
    float m = story.t / 60.0f;
    float dusk = std::min(1.0f, m / 45.0f);
    // the sun went down at about 15:25. Blue dusk at 16:00, night by about 16:28. The warmth is all indoors
    Color amb = LerpC(Color{ 190, 198, 232, 255 }, Color{ 112, 126, 190, 255 }, m / 28.0f);
    if (story.Storm()) amb = LerpC(amb, Color{ 108, 114, 150, 255 }, 0.7f);
    bool powerOut = story.PowerOut();
    if (powerOut) amb = Color{ 82, 90, 140, 255 };
    // v0.15: dark from 16:45, step by step. By 16:53 you can hardly see the yard
    float night = std::max(0.0f, std::min(1.0f, (m - 45.0f) / 8.0f));
    if (night > 0) amb = LerpC(amb, Color{ 14, 16, 36, 255 }, night * 0.86f);
    if (viewBelow) amb = Color{ 100, 108, 154, 255 };
    BeginTextureMode(lightRT);
    ClearBackground(amb);
    BeginMode2D(cam);
    float flick = StormFlick();
    float inH = 1 - roofA, inS = 1 - shedRoofA;
    if (!viewBelow) {
        // under a roof it is never as blue as outside. With the power on it is warm
        Color warm = powerOut ? Color{ 92, 92, 130, 255 } : LerpC(Color{ 255, 244, 226, 255 }, Color{ 206, 184, 160, 255 }, dusk);
        if (!powerOut) { warm.r = (unsigned char)(warm.r * (0.6f + 0.4f * flick)); warm.g = (unsigned char)(warm.g * (0.6f + 0.4f * flick)); warm.b = (unsigned char)(warm.b * (0.6f + 0.4f * flick)); }
        DrawRectangle(3 * TS, 2 * TS + 6, 26 * TS, 18 * TS - 6, Fade(warm, inH));
        DrawRectangle(32 * TS, 4 * TS + 6, 5 * TS, 6 * TS - 6, Fade(warm, inS));
    }
    BeginBlendMode(BLEND_ADDITIVE);
    auto tile = [](float t) { return t * TS + TS / 2.0f; };
    if (!powerOut && !viewBelow) {
        float lamp = (0.25f + dusk * 0.75f) * flick;
        Color warm = { 150, 100, 46, 255 };
        const float L[][2] = { { 6, 5.5f }, { 13, 5.5f }, { 22, 5.5f }, { 8, 10.5f }, { 24, 10.5f }, { 7, 16 }, { 19, 15.5f }, { 25, 17 } };
        for (auto& p : L) Glow(tile(p[0]), tile(p[1]), 84, 76, warm, lamp * inH);
        Glow(tile(18) + 4, tile(4), 26, 26, Color{ 150, 120, 60, 255 }, 0.6f * inH);                                // Mom's desk lamp
        Glow(tile(34), tile(7), 70, 64, warm, lamp * inS);
        Glow(tile(21), tile(13) + 6, 44, 40, Color{ 40, 76, 130, 255 }, (0.5f + 0.2f * std::sin(globalT * 11)) * inH);   // TV
        Glow(tile(26), tile(15) - 4, 36, 36, story.tainted ? Color{ 110, 30, 30, 255 } : Color{ 96, 76, 44, 255 }, 0.8f * inH);   // the tree
        // what the windows throw on the snow
        float spill = (0.55f + 0.45f * dusk) * flick;
        const float W[3] = { 8, 15, 25 };
        for (float wx : W) { Glow(tile(wx), 21 * TS + 16.0f, 26, 34, Color{ 150, 96, 30, 255 }, spill); Glow(tile(wx), 21 * TS + 2.0f, 13, 10, Color{ 130, 96, 50, 255 }, spill); }
        Glow(19 * TS + 12.0f, 21 * TS + 4.0f, 44, 40, Color{ 150, 110, 56, 255 }, spill * 0.9f);                    // porch lamp
        Glow(tile(36), 11 * TS + 10.0f, 16, 22, Color{ 140, 92, 30, 255 }, spill); Glow(tile(32), 11 * TS + 10.0f, 16, 22, Color{ 140, 92, 30, 255 }, spill);
        for (float wx : { 6.0f, 13.0f, 20.0f }) Glow(tile(wx), 1 * TS + 6.0f, 18, 22, Color{ 140, 92, 34, 255 }, spill * 0.8f);   // v0.15: the back windows
    }
    float fire = FireFlicker();
    Glow(tile(27), tile(18) + 2, 50, 46, Color{ 160, 74, 20, 255 }, fire * (0.4f + dusk * 0.6f + (powerOut ? 0.5f : 0)) * inH);
    if (viewBelow) {
        Glow(tile(10), tile(37) - 4, story.collection ? 40.0f : 96.0f, story.collection ? 36.0f : 90.0f, Color{ 40, 84, 160, 255 }, story.collection ? 0.4f : 0.95f);                                   // the wall of screens
        Glow(tile(18), tile(38), 30, 30, Color{ 130, 30, 30, 255 }, std::fmod(globalT, 1.2f) < 0.6f ? 0.7f : 0.2f);
        Glow(tile(18), tile(43) - 4, 30, 30, Color{ 50, 110, 140, 255 }, 0.7f);
        Glow(tile(5), tile(37), 40, 40, Color{ 90, 90, 110, 255 }, 0.6f);                                         // light down the stairs
    }
    if (story.CarHere()) {
        // headlights up the road, straight at the gate
        for (int k = 0; k < 2; ++k) { float hx = 20 * TS - 6 + (k ? 22.5f : 5.5f); Glow(hx, 31 * TS - 34.0f, 22, 56, Color{ 170, 160, 116, 255 }, 0.75f); Glow(hx, 31 * TS + 2.0f, 16, 14, Color{ 170, 160, 116, 255 }, 0.8f); }
        Glow(20 * TS + 8.0f, 33 * TS + 2.0f, 26, 14, Color{ 120, 20, 20, 255 }, 0.6f);
    }
    bool kidOut = IsOutsideT(TileAt(TX(player.pos.x), TX(player.pos.y)));
    if (powerOut && !kidOut) Glow(player.pos.x, player.pos.y - 6, 76, 76, Color{ 66, 68, 92, 255 }, 1.0f);   // eyes adjusting
    if (kidOut && (powerOut || night > 0)) Glow(player.pos.x, player.pos.y - 6, 50, 46, Color{ 84, 90, 132, 255 }, 0.35f + 0.55f * night);   // v0.15: a small circle you can see in
    if (TorchOn()) {
        // v0.15: his torch. Long, wide, and it goes straight through bushes and the chain fence
        float cx = std::cos(beamAng), cy = std::sin(beamAng);
        Glow(visitor.pos.x + cx * 10, visitor.pos.y - 8 + cy * 10, 16, 16, Color{ 230, 226, 190, 255 }, 0.9f);
        for (int i = 1; i <= 9; ++i) {
            float r = 12.0f + 7.5f * i;
            Glow(visitor.pos.x + cx * 25 * i, visitor.pos.y - 6 + cy * 25 * i, r, r, Color{ 214, 210, 172, 255 }, 0.95f - i * 0.07f);
        }
    }
    if (story.hasLight) {
        // Dad's flashlight, a beam in the direction you face
        const float fx[4] = { 0, 0, -1, 1 }, fy[4] = { 1, -1, 0, 0 };
        float k = (powerOut || night > 0.3f) ? 1.0f : 0.35f;   // v0.15: in the dark it's a real beam
        for (int i = 1; i <= 4; ++i) Glow(player.pos.x + fx[player.dir] * 22 * i, player.pos.y - 4 + fy[player.dir] * 22 * i, 18.0f + 8 * i, 18.0f + 8 * i, Color{ 176, 170, 132, 255 }, k * (1.1f - i * 0.15f));
    }
    EndBlendMode();
    EndMode2D();
    EndTextureMode();
}

// the things that shine by themselves get a halo, added after the light map
static void DrawGlows() {
    bool on = Lamps();
    float inH = 1 - roofA, flick = StormFlick();
    auto tile = [](float t) { return t * TS + TS / 2.0f; };
    if (viewBelow) {
        Glow(tile(9) + 4, tile(37) - 6, 30, 22, Color{ 80, 150, 255, 255 }, 0.35f);
        Glow(tile(18) + 4, tile(38) - 2, 8, 8, Color{ 255, 60, 60, 255 }, std::fmod(globalT, 1.2f) < 0.6f ? 0.6f : 0);
        return;
    }
    DrawEaveLights(true);
    if (on) {
        const int W[3] = { 8, 15, 25 };
        for (int wx : W) Glow(tile((float)wx), 20 * TS + 10.0f, 17, 13, Color{ 255, 196, 110, 255 }, 0.42f * flick);
        Glow(tile(32), 10 * TS + 10.0f, 11, 9, Color{ 255, 196, 110, 255 }, 0.35f); Glow(tile(36), 10 * TS + 10.0f, 11, 9, Color{ 255, 196, 110, 255 }, 0.35f);
        Glow(19 * TS + 12.5f, 20 * TS + 9.0f, 10, 10, Color{ 255, 226, 150, 255 }, 0.7f * flick);
        Glow(19 * TS + 12.5f, 20 * TS + 9.0f, 26, 24, Color{ 255, 190, 110, 255 }, 0.16f * flick);
        // the tree, bulb by bulb
        for (int k = 0; k < 11; ++k) if (BulbOn(k)) Glow(26 * TS + kBulbX[k] + 1.0f, 15 * TS + kBulbY[k] + 1.0f, 5, 5, BulbColor(k), 0.6f * inH);
        Glow(26 * TS + 8.0f, 15 * TS - 19.0f, 8, 8, Color{ 255, 230, 130, 255 }, (0.5f + 0.2f * std::sin(globalT * 3)) * inH);
        Glow(tile(21), 13 * TS + 2.0f, 15, 11, Color{ 110, 170, 255, 255 }, 0.3f * inH);
        Glow(18 * TS + 12.0f, 4 * TS + 1.0f, 7, 6, Color{ 255, 226, 140, 255 }, 0.5f * inH);
    }
    Glow(27 * TS + 8.0f, 18 * TS + 9.0f, 13, 11, Color{ 255, 150, 60, 255 }, (0.4f + 0.3f * FireFlicker()) * inH);
    if (story.CarHere()) {
        for (int k = 0; k < 2; ++k) {
            float hx = 20 * TS - 6 + (k ? 22.5f : 5.5f);
            Glow(hx, 31 * TS + 3.0f, 9, 8, Color{ 255, 250, 210, 255 }, 0.9f);
            Glow(hx, 31 * TS - 22.0f, 13, 34, Color{ 255, 244, 200, 255 }, 0.14f);          // the beams, full of snow
        }
        Glow(20 * TS - 3.0f, 32 * TS + 7.0f, 6, 5, Color{ 255, 40, 40, 255 }, 0.6f); Glow(20 * TS + 19.0f, 32 * TS + 7.0f, 6, 5, Color{ 255, 40, 40, 255 }, 0.6f);
    }
    if (story.hasLight && story.held == Item::Flashlight) Glow(player.pos.x + 10, player.pos.y - 3, 5, 5, Color{ 255, 244, 190, 255 }, 0.7f);
    // v0.15: the bulbs on the board behind the shed
    for (int k = 0; k < 26; ++k) if (BoardBulbOn(k)) { Vector2 p = BoardLetter(k); bool spell = BoardLit() >= 0; Glow(BOARD_X + p.x + 1.5f, BOARD_Y + p.y + (spell ? 3.0f : 1.0f), spell ? 11.0f : 5.0f, spell ? 11.0f : 5.0f, BulbCol(k), spell ? 1.0f : 0.45f); }
    if (TorchOn()) Glow(visitor.pos.x + std::cos(beamAng) * 8, visitor.pos.y - 8 + std::sin(beamAng) * 8, 4, 4, Color{ 255, 250, 220, 255 }, 0.9f);
    if (vTaserT > 0 && std::fmod(globalT, 0.12f) < 0.06f) Glow(visitor.pos.x + 8, visitor.pos.y - 10, 5, 5, Color{ 140, 200, 255, 255 }, 0.9f);   // v0.15.2: the taser, aimed
    if (taserFx > 0) {
        // the wires and the spark
        float k = taserFx / 0.5f;
        for (int i = 0; i < 2; ++i) {
            Vector2 a = taserA, b = { taserB.x + (i ? 2.0f : -2.0f), taserB.y - 4 };
            for (int s = 0; s < 6; ++s) {
                float t0 = s / 6.0f, t1 = (s + 1) / 6.0f;
                Vector2 p0 = { a.x + (b.x - a.x) * t0, a.y + (b.y - a.y) * t0 + std::sin(globalT * 80 + s + i) * 2 }, p1 = { a.x + (b.x - a.x) * t1, a.y + (b.y - a.y) * t1 + std::sin(globalT * 80 + s + 1 + i) * 2 };
                DrawLineEx(p0, p1, 1.0f, Fade(Color{ 180, 220, 255, 255 }, k));
            }
        }
        Glow(taserB.x, taserB.y - 4, 18, 18, Color{ 120, 180, 255, 255 }, k);
    }
}

// ---------------------------------------------------------------- who looks like what
static Look LookKid() {
    Look kid{ Color{ 200, 50, 50, 255 }, Color{ 50, 60, 110, 255 }, Color{ 96, 62, 32, 255 }, Color{ 242, 204, 174, 255 }, 1.0f, false, false };
    if (story.tainted) { kid.skin = Color{ 222, 208, 204, 255 }; kid.shirt = Color{ 120, 30, 34, 255 }; kid.eyes = Color{ 150, 0, 0, 255 }; }
    if (story.Stage() >= 4) { kid.skin = Color{ 205, 195, 200, 255 }; kid.shirt = Color{ 50, 20, 24, 255 }; }
    if (story.hasCoat) { kid.shirt = story.tainted ? Color{ 30, 34, 54, 255 } : Color{ 48, 78, 128, 255 }; kid.hat = true; }
    return kid;
}
static Look LookMom() { return { Color{ 66, 126, 92, 255 }, Color{ 84, 66, 60, 255 }, Color{ 186, 134, 62, 255 }, Color{ 238, 198, 168, 255 }, 1.15f, true, false, BLACK, 1 }; }
static Look LookDad() { return { Color{ 56, 86, 154, 255 }, Color{ 62, 52, 44, 255 }, Color{ 110, 70, 40, 255 }, Color{ 232, 192, 162, 255 }, 1.2f, false, true, BLACK, 2 }; }
static Look LookVisitor() { return { Color{ 26, 26, 32, 255 }, Color{ 20, 20, 24, 255 }, Color{ 20, 20, 20, 255 }, Color{ 206, 190, 186, 255 }, 1.55f, false, false, BLACK, 3 }; }

// a face for the dialogue box. cx, cy in screen pixels: the middle of the frame. Returns false if this speaker has none
static bool DrawPortrait(const std::string& who, float cx, float cy, float px) {
    Actor a; a.pos = { 0, 0 }; a.dir = 0; a.anim = 0; a.visible = true;
    Look l; bool dog = who == "Doggo";
    if (who == "You") l = LookKid(); else if (who == "Mom") l = LookMom(); else if (who == "Dad") l = LookDad(); else if (who == "The visitor") l = LookVisitor(); else if (!dog) return false;
    rlDrawRenderBatchActive();
    rlPushMatrix();
    if (dog) { rlTranslatef(cx - 2 * px, cy + 9 * px, 0); rlScalef(px, px, 1); a.dir = 3; DrawDoggo(a); }
    else { float z = px / l.scale; rlTranslatef(cx, cy + (who == "The visitor" ? 12.5f : 10.5f) * px, 0); rlScalef(z, z, 1); DrawPerson(a, l); }
    rlDrawRenderBatchActive();
    rlPopMatrix();
    return true;
}

// ---------------------------------------------------------------- the frame
static void DrawWorld(Camera2D cam) {
    float fdt = std::min(GetFrameTime(), 0.05f);
    const bool below = viewBelow;
    // roofs lift away when you step inside
    {
        float feetY = player.pos.y + 4;
        float wantH = (settings.roofs && !InHouse(player.pos.x, feetY)) ? 1.0f : 0.0f, wantS = (settings.roofs && !InShed(player.pos.x, feetY)) ? 1.0f : 0.0f;
        float step = fdt * 3.5f;
        roofA += std::max(-step, std::min(step, wantH - roofA));
        shedRoofA += std::max(-step, std::min(step, wantS - shedRoofA));
    }
    UpdateParticles(fdt);
    ClearBackground(Color{ 4, 4, 8, 255 });
    BeginMode2D(cam);
    int x0 = std::max(0, TX(cam.target.x - VW / 2.0f) - 1), x1 = std::min(World::W - 1, TX(cam.target.x + VW / 2.0f) + 1);
    int y0 = std::max(World::TOP, TX(cam.target.y - VH / 2.0f) - 1), y1 = std::min(World::H - 1, TX(cam.target.y + VH / 2.0f) + 2);
    // the basement is underground: only draw it from inside
    // v0.15: the ground texture starts at row World::TOP (the backyard)
    if (below) DrawTextureRec(groundTex, { 0, (34.0f - World::TOP) * TS, (float)groundTex.width, 12.0f * TS }, { 0, 34.0f * TS }, WHITE);
    else { DrawTextureRec(groundTex, { 0, 0, (float)groundTex.width, (34.0f - World::TOP) * TS }, { 0, (float)World::TOP * TS }, WHITE); DrawTexture(southTex, 0, 34 * TS, WHITE); }
    for (int y = y0; y <= y1; ++y) for (int x = x0; x <= x1; ++x) if (below == (y >= 34)) DrawTileFx(x, y);

    DrawPrints();
    // v0.12: marbles, in the air and where they stopped
    for (auto& m : marbles) {
        float k = m.dur > 0 ? std::min(1.0f, m.t / m.dur) : 1;
        float x = m.from.x + (m.to.x - m.from.x) * k, y = m.from.y + (m.to.y - m.from.y) * k + 5 - std::sin(k * 3.14159f) * 9;
        if (below != (y >= 34 * TS)) continue;
        DrawRectangle((int)x - 1, (int)y - 1, 3, 3, Color{ 30, 60, 90, 255 }); DrawRectangle((int)x, (int)y - 1, 1, 1, Color{ 200, 240, 255, 255 });
    }
    bool flux = story.Act() >= 2;
    // objects and people, sorted by y
    struct D { float y; int kind; const Obj* o; const Actor* a; int i = 0; };
    std::vector<D> ds;
    for (auto& o : story.world.objs) if (below == (o.y >= 34)) ds.push_back({ o.y * TS + 8.0f, 0, &o, nullptr });
    if (!hidden) ds.push_back({ player.pos.y, 1, nullptr, &player });
    if (mom.visible) ds.push_back({ mom.pos.y, 2, nullptr, &mom });
    if (dad.visible) ds.push_back({ dad.pos.y, 3, nullptr, &dad });
    if (story.DoggoExists() || story.doggoDead || (story.doggoOut && !doggoRanOff)) ds.push_back({ doggo.pos.y, 4, nullptr, &doggo });
    if (VisitorShown()) ds.push_back({ visitor.pos.y, 5, nullptr, &visitor });
    if (story.CarHere() && !below) ds.push_back({ 31 * TS + 20.0f, 6, nullptr, nullptr });
    // v0.15: bushes stand up out of the snow, so people walk behind them
    std::vector<std::pair<int, int>> bushes;
    if (!below) for (int y = y0; y <= y1; ++y) for (int x = x0; x <= x1; ++x) if (story.world.tiles[y][x] == Tile::Bush) bushes.push_back({ x, y });
    for (size_t i = 0; i < bushes.size(); ++i) ds.push_back({ bushes[i].second * TS + 13.0f, 7, nullptr, nullptr, (int)i });
    if (!below && huskyA > 0.01f) ds.push_back({ (float)World::TOP * TS + 13.0f, 8, nullptr, nullptr });
    std::sort(ds.begin(), ds.end(), [](const D& a, const D& b) { return a.y < b.y; });
    for (auto& d : ds) {
        if (d.a && below != (d.a->pos.y >= 34 * TS)) continue;   // and nobody from the other floor
        switch (d.kind) {
        case 0: {
            Vector2 j{ 0, 0 };
            // one object at a time, a short shiver every ~9 seconds (more often once you're dark)
            if (flux) {
                float period = story.Act() >= 3 ? 5.0f : 9.0f;
                int slot = (int)(globalT / period);
                unsigned pick = Hash(slot, 77) % (unsigned)story.world.objs.size();
                if (&story.world.objs[pick] == d.o && std::fmod(globalT, period) < 0.25f) j = { (float)((int)(globalT * 30) % 3 - 1), 0 };
            }
            DrawObj(*d.o, j);
        } break;
        case 1: {
            Look kid = LookKid();
            DrawPerson(*d.a, kid);
            {
                // whatever is in the kid's hands
                int hx = (int)d.a->pos.x + 6, hy = (int)d.a->pos.y - 4;
                switch (story.held) {
                case Item::Knife: DrawRectangle(hx, hy, 1, 5, Color{ 222, 222, 232, 255 }); DrawRectangle(hx, hy + 5, 1, 2, Color{ 90, 60, 40, 255 }); break;
                case Item::Key: DrawRectangle(hx, hy + 1, 3, 2, GOLD); DrawRectangle(hx + 2, hy + 3, 1, 2, GOLD); break;
                case Item::Stick: DrawLine(hx, hy + 6, hx + 4, hy - 4, Color{ 130, 90, 50, 255 }); break;
                case Item::Bag: DrawRectangle(hx - 1, hy + 1, 5, 6, Color{ 30, 30, 34, 255 }); DrawRectangle(hx, hy, 3, 1, Color{ 60, 60, 66, 255 }); break;
                case Item::DogFood: DrawRectangle(hx - 1, hy + 2, 4, 4, Color{ 200, 60, 60, 255 }); DrawRectangle(hx, hy + 1, 2, 1, Color{ 150, 100, 50, 255 }); break;
                case Item::Flashlight: DrawRectangle(hx, hy + 1, 4, 2, Color{ 60, 60, 66, 255 }); DrawRectangle(hx + 4, hy, 1, 4, Color{ 250, 240, 180, 255 }); break;
                case Item::Tape: DrawRectangle(hx - 1, hy + 1, 5, 3, Color{ 20, 20, 24, 255 }); DrawRectangle(hx, hy + 2, 3, 1, Color{ 220, 220, 210, 255 }); break;
                case Item::BabyWatch: DrawRectangle(hx, hy + 2, 3, 3, Color{ 20, 26, 34, 255 }); DrawRectangle(hx + 1, hy + 3, 1, 1, Color{ 150, 178, 186, 255 }); DrawRectangle(hx + 1, hy, 1, 2, Color{ 60, 66, 76, 255 }); DrawRectangle(hx + 1, hy + 5, 1, 2, Color{ 60, 66, 76, 255 }); break;
                default: break;
                }
            }
            // the watch
            DrawRectangle((int)d.a->pos.x + 4, (int)d.a->pos.y - 2, 2, 2, Color{ 180, 226, 160, 255 });
        } break;
        case 2: {
            Look ml = LookMom();
            if (story.momOnFloor) DrawPersonDown(*d.a, ml, true); else DrawPerson(*d.a, ml);
        } break;
        case 3: {
            Look dl = LookDad();
            if (story.dadOnFloor && scene != SceneMode::DadRuns) DrawPersonDown(*d.a, dl, false); else DrawPerson(*d.a, dl);
        } break;
        case 4: if (story.doggoDead && doggoShownDead) DrawDoggoDead(*d.a); else DrawDoggo(*d.a); break;
        case 5: DrawPerson(*d.a, LookVisitor()); break;
        case 6: DrawCar(); break;
        case 7: DrawBush(bushes[d.i].first, bushes[d.i].second); break;
        case 8: DrawHusky(); break;
        }
    }
    DrawPuffs(0);
    if (!below) {
        // roofs, a metre of snow on each
        if (shedRoofA > 0.01f) DrawTexture(shedRoofTex, SROOF_X, SROOF_Y, Fade(WHITE, shedRoofA));
        if (roofA > 0.01f) DrawTexture(roofTex, ROOF_X, ROOF_Y, Fade(WHITE, roofA));
        DrawVane();
        DrawEaveLights(false);
        DrawPuffs(2);
        DrawPuffs(1);
    }
    EndMode2D();

    // the light map (built before this pass) multiplied over the world
    BeginBlendMode(BLEND_MULTIPLIED);
    DrawTextureRec(lightRT.texture, { 0, 0, (float)VW, -(float)VH }, { 0, 0 }, WHITE);
    EndBlendMode();
    BeginMode2D(cam);
    BeginBlendMode(BLEND_ADDITIVE);
    DrawGlows();
    EndBlendMode();
    if (!below) DrawSnow(cam);   // after the light, so the flakes catch it. It does not snow in the basement
    EndMode2D();
    if (story.Storm() && !below) {
        // the air itself goes white
        bool outside = IsOutsideT(TileAt(TX(player.pos.x), TX(player.pos.y)));
        DrawRectangle(0, 0, VW, VH, Fade(Color{ 206, 214, 236, 255 }, outside ? 0.16f : 0.05f));
        if (outside) for (int k = 0; k < 4; ++k) {
            float bx = std::fmod(globalT * (60 + k * 22) + k * 170, (float)VW + 240) - 240;
            DrawRectangleGradientH((int)bx, 30 + k * 60, 120, 46, Fade(WHITE, 0), Fade(WHITE, 0.10f));
            DrawRectangleGradientH((int)bx + 120, 30 + k * 60, 120, 46, Fade(WHITE, 0.10f), Fade(WHITE, 0));
        }
    }
    DrawTexture(vigTex, 0, 0, WHITE);
    if (scene == SceneMode::Chase) {
        float p = 0.5f + 0.5f * std::sin(globalT * 6);
        for (int k = 0; k < 18; ++k) {
            float a = (18 - k) / 18.0f * (0.18f + 0.12f * p);
            DrawRectangleLines(k, k, VW - 2 * k, VH - 2 * k, Fade(Color{ 160, 0, 0, 255 }, a));
        }
    }
    if (story.cold > 0) {
        // the frost closes in
        for (int k = 0; k < 26; ++k) DrawRectangleLines(k, k, VW - 2 * k, VH - 2 * k, Fade(Color{ 220, 236, 255, 255 }, (26 - k) / 26.0f * 0.5f * story.cold));
        for (int k = 0; k < 60; ++k) {
            unsigned h = Hash(k, 91);
            int ex = (k % 2) ? (int)(h % (unsigned)(30 * story.cold + 1)) : VW - 1 - (int)(h % (unsigned)(30 * story.cold + 1));
            DrawRectangle(ex, (int)((h >> 8) % VH), 2, 1, Fade(WHITE, 0.5f * story.cold));
        }
    }
    if (flux) {
        // reality struggling to keep up
        if (std::fmod(globalT, 23.0f) < 0.1f) DrawRectangle(0, (int)(std::fmod(globalT * 997, (float)VH)), VW, 3, Fade(WHITE, 0.2f));
    }
}

// ---------------------------------------------------------------- the title picture
// 16:49 and some seconds. The house, the car at the gate, and him. Painted straight into the 480x270 target
static void DrawTitleScene() {
    const int HZ = 168;                                                     // the horizon
    auto R = [](int x, int y, int w, int h, Color c) { DrawRectangle(x, y, w, h, c); };
    BeginTextureMode(target);
    DrawRectangleGradientV(0, 0, VW, HZ, Color{ 8, 10, 28, 255 }, Color{ 44, 54, 104, 255 });
    for (int i = 0; i < 46; ++i) {
        unsigned h = Hash(i, 501);
        float tw = 0.35f + 0.3f * std::sin(globalT * (0.6f + (h % 5) * 0.2f) + i);
        R((int)(h % VW), (int)((h >> 9) % (HZ - 50)), 1, 1, Fade(Color{ 214, 224, 255, 255 }, tw));
    }
    // the plateau, two ridges deep
    for (int x = 0; x < VW; ++x) {
        int h1 = (int)(22 + 9 * std::sin(x * 0.011f + 1.0f) + 5 * std::sin(x * 0.037f) + 2 * std::sin(x * 0.11f));
        int h2 = (int)(11 + 6 * std::sin(x * 0.017f + 4.0f) + 3 * std::sin(x * 0.051f + 2.0f));
        R(x, HZ - h1, 1, h1, Color{ 30, 38, 80, 255 }); R(x, HZ - h1, 1, 2, Color{ 62, 76, 128, 255 });
        R(x, HZ - h2, 1, h2, Color{ 48, 60, 110, 255 }); R(x, HZ - h2, 1, 1, Color{ 92, 108, 164, 255 });
    }
    DrawRectangleGradientV(0, HZ, VW, VH - HZ, Color{ 84, 100, 160, 255 }, Color{ 40, 50, 100, 255 });
    for (int i = 0; i < 70; ++i) {                                          // drifts
        unsigned h = Hash(i, 601);
        int y = HZ + 3 + (int)((h >> 4) % (unsigned)(VH - HZ - 4)), w = 20 + (int)(h % 60);
        R((int)((h >> 12) % VW) - 20, y, w, 1, Fade(Color{ 150, 168, 220, 255 }, 0.25f + 0.2f * ((y - HZ) / 100.0f)));
    }
    bool red = story.tainted;
    // the house
    const int hx = 46, hy = 172, hw = 118, hh = 40;
    R(hx - 4, hy + hh, hw + 8, 3, Color{ 30, 38, 84, 255 });
    R(hx, hy, hw, hh, Color{ 92, 32, 36, 255 });
    for (int x = hx + 5; x < hx + hw; x += 6) R(x, hy, 1, hh, Color{ 70, 22, 28, 255 });
    R(hx, hy, 3, hh, Color{ 170, 170, 186, 255 }); R(hx + hw - 3, hy, 3, hh, Color{ 170, 170, 186, 255 });
    R(hx, hy + hh - 4, hw, 4, Color{ 60, 62, 80, 255 });
    for (int y = 0; y < 30; ++y) {                                          // the roof and what lies on it
        int inset = (30 - y) * 9 / 10;
        R(hx - 8 + inset, hy - 30 + y, hw + 16 - inset * 2, 1, y < 22 ? Color{ 150, 168, 216, 255 } : Color{ 120, 138, 190, 255 });
        R(hx - 8 + inset, hy - 30 + y, 2, 1, Color{ 196, 208, 240, 255 });
    }
    R(hx - 9, hy - 1, hw + 18, 2, Color{ 190, 190, 204, 255 }); R(hx - 8, hy - 4, hw + 16, 3, Color{ 176, 192, 232, 255 });
    R(hx + 84, hy - 40, 10, 16, Color{ 84, 40, 38, 255 }); R(hx + 83, hy - 42, 12, 3, Color{ 176, 192, 232, 255 });
    for (int i = 0; i < 9; ++i) {                                           // smoke, taken by the wind
        float t = std::fmod(globalT * 0.35f + i / 9.0f, 1.0f);
        DrawCircle(hx + 89 - (int)(t * t * 70), hy - 44 - (int)(t * 26), 2 + t * 6, Fade(Color{ 120, 130, 170, 255 }, 0.35f * (1 - t)));
    }
    const int wxs[3] = { hx + 14, hx + 50, hx + 92 };
    for (int wx : wxs) {
        R(wx - 1, hy + 9, 18, 18, Color{ 196, 196, 208, 255 });
        DrawRectangleGradientV(wx, hy + 10, 16, 16, Color{ 255, 226, 150, 255 }, Color{ 246, 170, 80, 255 });
        R(wx + 7, hy + 10, 2, 16, Color{ 196, 196, 208, 255 }); R(wx, hy + 17, 16, 1, Color{ 196, 196, 208, 255 });
        for (int k = 0; k < 7; ++k) R(wx + 2 + k * 2, hy + 21 + std::abs(k - 3), 1, 1, Color{ 255, 252, 220, 255 });
        R(wx - 2, hy + 27, 20, 2, Color{ 176, 192, 232, 255 });
    }
    R(hx + 72, hy + 12, 14, 28, Color{ 28, 58, 50, 255 }); R(hx + 71, hy + 11, 16, 1, Color{ 196, 196, 208, 255 }); R(hx + 82, hy + 26, 2, 2, Color{ 226, 200, 110, 255 });
    DrawCircle(hx + 79, hy + 19, 4, Color{ 30, 80, 56, 255 }); DrawCircle(hx + 79, hy + 19, 1.5f, Color{ 28, 58, 50, 255 }); R(hx + 78, hy + 22, 3, 2, Color{ 200, 40, 44, 255 });
    R(hx + 66, hy + 12, 3, 5, Color{ 255, 236, 170, 255 });
    // fence, gate, flag
    for (int x = 0; x < VW; x += 22) { if (x > 300 && x < 356) continue; R(x, HZ + 62, 22, 2, Color{ 50, 40, 44, 255 }); R(x, HZ + 68, 22, 2, Color{ 50, 40, 44, 255 }); R(x + 9, HZ + 58, 3, 16, Color{ 62, 48, 46, 255 }); R(x + 8, HZ + 56, 5, 2, Color{ 176, 192, 232, 255 }); R(x, HZ + 61, 22, 1, Color{ 150, 168, 216, 255 }); }
    R(178, 152, 2, 62, Color{ 150, 156, 180, 255 });
    for (int i = 0; i < 16; ++i) { int wv = (int)(std::sin(globalT * 9 - i * 0.6f) * 1.6f * i / 16.0f); R(162 + i, 153 + wv, 1, 11, (i >= 9 && i <= 11) ? Color{ 150, 156, 190, 255 } : Color{ 130, 30, 40, 255 }); R(162 + i, 157 + wv, 1, 3, Color{ 150, 156, 190, 255 }); R(162 + i, 158 + wv, 1, 1, Color{ 16, 30, 80, 255 }); if (i == 10) R(162 + i, 153 + wv, 1, 11, Color{ 16, 30, 80, 255 }); }
    // the car
    const int cx = 372, cy = 222;
    R(cx - 2, cy + 15, 64, 3, Color{ 24, 30, 70, 255 });
    R(cx, cy + 4, 60, 11, Color{ 12, 12, 18, 255 }); R(cx + 14, cy - 4, 30, 9, Color{ 12, 12, 18, 255 }); R(cx + 17, cy - 2, 11, 6, Color{ 40, 52, 84, 255 }); R(cx + 30, cy - 2, 11, 6, Color{ 40, 52, 84, 255 });
    R(cx + 13, cy - 6, 32, 2, Color{ 150, 168, 216, 255 }); R(cx, cy + 3, 13, 1, Color{ 150, 168, 216, 255 });
    DrawCircle(cx + 12, cy + 15, 5, Color{ 6, 6, 10, 255 }); DrawCircle(cx + 48, cy + 15, 5, Color{ 6, 6, 10, 255 }); DrawCircle(cx + 12, cy + 15, 2, Color{ 60, 62, 76, 255 }); DrawCircle(cx + 48, cy + 15, 2, Color{ 60, 62, 76, 255 });
    R(cx, cy + 6, 3, 3, Color{ 255, 248, 210, 255 }); R(cx + 58, cy + 6, 2, 3, Color{ 220, 30, 36, 255 });
    // him, halfway to the door, and the way he came
    const int vx = 246, vy = 232;
    for (int i = 0; i < 9; ++i) R(vx + 14 + i * 12, vy + 1 + (i % 2) * 2, 3, 1, Color{ 40, 50, 104, 255 });
    DrawEllipse(vx - 14, vy + 1, 16, 2, Fade(Color{ 16, 20, 56, 255 }, 0.6f));                // his shadow, thrown by the headlights
    R(vx - 4, vy - 24, 9, 22, Color{ 8, 8, 12, 255 }); R(vx - 3, vy - 2, 3, 3, Color{ 8, 8, 12, 255 }); R(vx + 1, vy - 2, 3, 3, Color{ 8, 8, 12, 255 });
    R(vx - 3, vy - 31, 7, 7, Color{ 8, 8, 12, 255 }); R(vx - 5, vy - 31, 11, 2, Color{ 8, 8, 12, 255 }); R(vx - 3, vy - 34, 7, 3, Color{ 8, 8, 12, 255 });
    R(vx + 4, vy - 22, 1, 20, Color{ 70, 72, 90, 255 });                                      // the rim light down his back
    // snow in the air
    for (int layer = 0; layer < 3; ++layer) for (int i = 0; i < 90; ++i) {
        unsigned h1 = Hash(i, 700 + layer), h2 = Hash(i, 720 + layer);
        float sp = 0.6f + layer * 0.35f;
        float sx = VW - std::fmod((float)(h1 % 977) + globalT * 46 * sp, (float)VW + 20) + 10, sy = std::fmod((float)(h2 % 953) + globalT * 34 * sp, (float)VH + 20) - 10;
        R((int)sx, (int)sy, layer == 2 ? 2 : 1, layer == 2 ? 2 : 1, Fade(WHITE, 0.3f + layer * 0.25f));
    }
    BeginBlendMode(BLEND_ADDITIVE);
    for (int wx : wxs) { Glow(wx + 8.0f, hy + 18.0f, 26, 24, Color{ 255, 180, 90, 255 }, 0.35f); Glow(wx + 8.0f, hy + 56.0f, 30, 16, Color{ 255, 170, 80, 255 }, 0.22f); }
    Glow(hx + 67.5f, hy + 14.0f, 14, 14, Color{ 255, 220, 150, 255 }, 0.5f);
    static const Color col[4] = { { 255, 214, 110, 255 }, { 250, 84, 76, 255 }, { 110, 180, 255, 255 }, { 120, 226, 136, 255 } };
    for (int i = 0, x = hx - 6; x < hx + hw + 8; x += 9, ++i) {
        bool lit = std::fmod(globalT * 0.5f + (i % 4) * 0.25f, 1.0f) < 0.82f && !(red && i % 3 == 0);
        if (lit) Glow(x + 0.5f, hy + 2.0f, 5, 5, red ? Color{ 230, 40, 40, 255 } : col[i % 4], 0.8f);
    }
    Glow(cx - 60.0f, cy + 8.0f, 90, 20, Color{ 255, 244, 200, 255 }, 0.20f); Glow(cx - 20.0f, cy + 8.0f, 40, 12, Color{ 255, 244, 200, 255 }, 0.3f);
    Glow(cx + 1.0f, cy + 7.5f, 8, 8, Color{ 255, 248, 210, 255 }, 0.9f); Glow(cx + 59.0f, cy + 7.5f, 7, 6, Color{ 255, 40, 40, 255 }, 0.6f);
    EndBlendMode();
    BeginBlendMode(BLEND_ALPHA);
    for (int i = 0, x = hx - 6; x < hx + hw + 8; x += 9, ++i) R(x, hy + 1, 1, 2, red ? Color{ 255, 120, 120, 255 } : Mix(col[i % 4], WHITE, 0.4f));
    EndBlendMode();
    // room for the words
    DrawRectangleGradientV(0, 0, VW, 150, Fade(Color{ 4, 4, 12, 255 }, 0.55f), Fade(Color{ 4, 4, 12, 255 }, 0.0f));
    DrawRectangleGradientV(0, 205, VW, 65, Fade(Color{ 4, 4, 12, 255 }, 0.0f), Fade(Color{ 4, 4, 12, 255 }, 0.5f));
    DrawTexture(vigTex, 0, 0, WHITE);
    EndTextureMode();
}
