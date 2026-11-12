// 600 Seconds - art/Bake.h (v0.11)
// The ground, the walls and the roofs are painted once, pixel by pixel, into images.
// main.cpp turns them into textures and draws everything that moves on top.
// Plain C++ apart from raylib's Color. Nothing in here knows about the story.
#pragma once
#include "../game/World.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace art {
using sixhundred::Room;
using sixhundred::Tile;
using sixhundred::World;

static const int TS_ = 16;

inline unsigned H2(int x, int y, int s = 0) {
    unsigned h = (unsigned)x * 374761393u + (unsigned)y * 668265263u + (unsigned)s * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}
inline float R01(int x, int y, int s = 0) { return (H2(x, y, s) & 0xffff) / 65535.0f; }
inline float VNoise(float x, float y, int s) {
    int xi = (int)std::floor(x), yi = (int)std::floor(y);
    float fx = x - xi, fy = y - yi;
    fx = fx * fx * (3 - 2 * fx); fy = fy * fy * (3 - 2 * fy);
    float a = R01(xi, yi, s), b = R01(xi + 1, yi, s), c = R01(xi, yi + 1, s), d = R01(xi + 1, yi + 1, s);
    return (a + (b - a) * fx) + ((c + (d - c) * fx) - (a + (b - a) * fx)) * fy;
}
inline float Fbm(float x, float y, int s) { return VNoise(x, y, s) * 0.55f + VNoise(x * 2.1f, y * 2.1f, s + 1) * 0.3f + VNoise(x * 4.3f, y * 4.3f, s + 2) * 0.15f; }
inline float Clamp01(float v) { return v < 0 ? 0 : v > 1 ? 1 : v; }
inline Color MixC(Color a, Color b, float t) {
    t = Clamp01(t);
    return { (unsigned char)(a.r + (b.r - a.r) * t), (unsigned char)(a.g + (b.g - a.g) * t), (unsigned char)(a.b + (b.b - a.b) * t), 255 };
}
inline Color Tone(Color c, int d) {
    auto cl = [](int v) { return (unsigned char)std::max(0, std::min(255, v)); };
    return { cl(c.r + d), cl(c.g + d), cl(c.b + d), c.a };
}

// Ordered dither: pick a step on a hand made ramp. This is what makes soft shading still read as pixel art
static const int kBayer[4][4] = { { 0, 8, 2, 10 }, { 12, 4, 14, 6 }, { 3, 11, 1, 9 }, { 15, 7, 13, 5 } };
inline Color Ramp(const Color* ramp, int n, float v, int x, int y) {
    v = Clamp01(v) * (n - 1);
    int i = (int)v;
    float f = v - i, th = (kBayer[y & 3][x & 3] + 0.5f) / 16.0f;
    if (f > th && i < n - 1) ++i;
    return ramp[std::min(i, n - 1)];
}

// ---- palettes. Every ramp runs dark to light
static const Color kSnow[6] = { { 170, 186, 228, 255 }, { 192, 206, 238, 255 }, { 210, 222, 246, 255 }, { 225, 234, 250, 255 }, { 237, 243, 253, 255 }, { 248, 251, 255, 255 } };
static const Color kRoad[5] = { { 138, 152, 196, 255 }, { 160, 174, 212, 255 }, { 182, 194, 226, 255 }, { 200, 210, 236, 255 }, { 216, 224, 244, 255 } };
static const Color kPine[5] = { { 96, 60, 40, 255 }, { 126, 82, 52, 255 }, { 154, 106, 66, 255 }, { 178, 128, 82, 255 }, { 200, 152, 102, 255 } };
static const Color kOak[5] = { { 78, 50, 36, 255 }, { 104, 68, 46, 255 }, { 128, 88, 58, 255 }, { 150, 106, 70, 255 }, { 170, 124, 84, 255 } };
static const Color kShedWood[5] = { { 58, 46, 40, 255 }, { 78, 62, 52, 255 }, { 98, 80, 66, 255 }, { 116, 98, 80, 255 }, { 134, 114, 94, 255 } };
static const Color kTileL[4] = { { 178, 172, 156, 255 }, { 204, 198, 180, 255 }, { 222, 218, 200, 255 }, { 236, 232, 218, 255 } };
static const Color kTileD[4] = { { 96, 108, 112, 255 }, { 118, 130, 132, 255 }, { 138, 150, 150, 255 }, { 158, 168, 166, 255 } };
static const Color kBath[4] = { { 132, 164, 182, 255 }, { 158, 190, 204, 255 }, { 184, 210, 220, 255 }, { 208, 228, 234, 255 } };
static const Color kCarpet[4] = { { 100, 84, 124, 255 }, { 122, 104, 146, 255 }, { 142, 124, 164, 255 }, { 160, 142, 180, 255 } };
static const Color kConcrete[5] = { { 44, 48, 60, 255 }, { 58, 62, 74, 255 }, { 72, 76, 88, 255 }, { 86, 90, 102, 255 }, { 102, 106, 118, 255 } };
static const Color kRed[5] = { { 84, 28, 30, 255 }, { 112, 38, 36, 255 }, { 138, 50, 42, 255 }, { 160, 64, 50, 255 }, { 178, 80, 60, 255 } };
static const Color kTimber[4] = { { 38, 30, 32, 255 }, { 54, 42, 40, 255 }, { 72, 56, 50, 255 }, { 92, 72, 60, 255 } };
static const Color kTrim = { 232, 226, 212, 255 };

struct Canvas {
    int w = 0, h = 0;
    int oy = 0;   // v0.15: pixel row of the canvas's first line. The world canvas starts above 0 (the backyard)
    std::vector<Color> px;
    void Init(int W, int H, Color c = { 0, 0, 0, 0 }) { w = W; h = H; oy = 0; px.assign((size_t)W * H, c); }
    void Set(int x, int y, Color c) { y -= oy; if (x >= 0 && y >= 0 && x < w && y < h) px[(size_t)y * w + x] = c; }
    Color Get(int x, int y) const { y -= oy; return (x >= 0 && y >= 0 && x < w && y < h) ? px[(size_t)y * w + x] : Color{ 0, 0, 0, 0 }; }
    void Rect(int x, int y, int rw, int rh, Color c) { for (int j = 0; j < rh; ++j) for (int i = 0; i < rw; ++i) Set(x + i, y + j, c); }
    // darken or lighten what is already there
    void Shade(int x, int y, float k) {
        Color c = Get(x, y);
        if (!c.a) return;
        if (k < 0) Set(x, y, MixC(c, Color{ 14, 12, 30, 255 }, -k)); else Set(x, y, MixC(c, Color{ 255, 250, 240, 255 }, k));
    }
};

struct WorldBaker {
    const World& w;
    Canvas& c;
    WorldBaker(const World& W, Canvas& C) : w(W), c(C) {}

    Tile T(int x, int y) const { return (x < 0 || y < World::TOP || x >= World::W || y >= World::H) ? Tile::Void : w.tiles[y][x]; }
    Room Rm(int x, int y) const { return (x < 0 || y < World::TOP || x >= World::W || y >= World::H) ? Room::Vidda : w.rooms[y][x]; }
    bool Wall(int x, int y) const { return T(x, y) == Tile::Wall; }
    static bool Out(Tile t) { return t == Tile::Grass || t == Tile::Gravel || t == Tile::Vidda || t == Tile::Fence || t == Tile::Bush || t == Tile::Chain || t == Tile::ChainGap || t == Tile::Ice; }
    static bool FloorLike(Tile t) { return t == Tile::Floor || t == Tile::ShedFloor || t == Tile::BasementDoor || t == Tile::Door; }

    // ---------------------------------------------------------------- ground
    Color Snow(int gx, int gy, bool vidda) const {
        // the snow is a height field, lit from the top left. That is what makes drifts instead of clouds
        auto hgt = [&](float x, float y) { return Fbm(x / (vidda ? 34.0f : 44.0f), y / (vidda ? 13.0f : 26.0f), 11) + Fbm(x / 11.0f, y / 7.0f, 23) * 0.18f; };
        float d = (hgt((float)gx, (float)gy) - hgt(gx + 3.0f, gy + 3.0f)) * (vidda ? 5.5f : 3.8f);
        float v = 0.70f + std::max(-0.40f, std::min(0.30f, d)) + (R01(gx, gy, 3) - 0.5f) * 0.05f;
        if (R01(gx, gy, 4) > 0.992f) v += 0.3f;    // a crystal catching the light
        return Ramp(kSnow, 6, v, gx, gy);
    }
    Color Road(int gx, int gy, int tx) const {
        int lx = gx - tx * TS_;
        float v = 0.62f + (Fbm(gx / 7.0f, gy / 14.0f, 31) - 0.5f) * 0.3f;
        bool rut = (tx == 19 && lx >= 9 && lx <= 13) || (tx == 21 && lx >= 2 && lx <= 6);
        if (rut) v -= 0.30f + 0.08f * (R01(gx, gy / 3, 2) > 0.5f);
        if ((tx == 19 && (lx == 8 || lx == 14)) || (tx == 21 && (lx == 1 || lx == 7))) v += 0.12f;   // the lip of the rut
        // the ploughed banks on both sides
        if (tx == 19 && lx < 4) return Ramp(kSnow, 6, 0.62f + lx * 0.09f + (R01(gx, gy, 9) - 0.5f) * 0.15f, gx, gy);
        if (tx == 21 && lx > 11) return Ramp(kSnow, 6, 0.90f - (lx - 12) * 0.10f + (R01(gx, gy, 9) - 0.5f) * 0.15f, gx, gy);
        return Ramp(kRoad, 5, v, gx, gy);
    }
    Color Planks(int gx, int gy, const Color* ramp, int len, float lift) const {
        int pr = gy / 4, off = (int)(H2(pr, 7) % (unsigned)len);
        int id = (gx + off) / len, in = (gx + off) % len, ly = gy % 4;
        float v = 0.42f + lift + R01(id, pr, 4) * 0.26f + (VNoise(gx * 0.30f, gy * 1.9f + id * 13.0f, 6) - 0.5f) * 0.22f;
        if (ly == 3) v -= 0.24f;                // the gap under each board
        if (ly == 0) v += 0.07f;                // and a little light on its top edge
        if (in == 0) v -= 0.30f;                // board ends
        if (in == 1) v += 0.05f;
        if ((in == 3 || in == len - 3) && ly == 1) v -= 0.22f;   // nails
        if (R01(id * 7 + 1, pr, 8) > 0.86f) {   // a knot now and then
            int kx = 6 + (int)(H2(id, pr, 2) % (unsigned)(len - 12));
            float d = std::hypot((float)(in - kx), (float)(ly - 1.5f) * 2.2f);
            if (d < 2.2f) v -= 0.26f; else if (d < 3.4f) v -= 0.08f;
        }
        return Ramp(ramp, 5, v, gx, gy);
    }
    Color Kitchen(int gx, int gy) const {
        int cx = gx / 8, cy = gy / 8, lx = gx % 8, ly = gy % 8;
        bool dark = (cx + cy) & 1;
        float v = 0.55f + (R01(cx, cy, 12) - 0.5f) * 0.30f;
        if (lx == 7 || ly == 7) v -= 0.55f;                       // grout
        else if (lx == 0 || ly == 0) v += 0.22f;                  // bevel
        if (lx + ly == 5 && !(lx == 7 || ly == 7)) v += 0.14f;    // a glossy streak
        return Ramp(dark ? kTileD : kTileL, 4, v, gx, gy);
    }
    Color Bath(int gx, int gy) const {
        int cx = gx / 4, cy = gy / 4, lx = gx % 4, ly = gy % 4;
        float v = 0.58f + (R01(cx, cy, 14) - 0.5f) * 0.5f;
        if (R01(cx, cy, 15) > 0.86f) v -= 0.4f;
        if (lx == 3 || ly == 3) v -= 0.45f;
        else if (lx == 0 && ly == 0) v += 0.2f;
        return Ramp(kBath, 4, v, gx, gy);
    }
    Color Carpet(int gx, int gy) const {
        float v = 0.5f + (R01(gx, gy, 16) - 0.5f) * 0.5f + (Fbm(gx / 12.0f, gy / 12.0f, 17) - 0.5f) * 0.3f;
        return Ramp(kCarpet, 4, v, gx, gy);
    }
    Color Concrete(int gx, int gy) const {
        float v = 0.48f + (Fbm(gx / 18.0f, gy / 18.0f, 41) - 0.5f) * 0.7f + (R01(gx, gy, 42) - 0.5f) * 0.12f;
        if (gx % 32 == 31 || gy % 32 == 31) v -= 0.3f;            // slab joints
        float crack = std::fabs(Fbm(gx / 22.0f, gy / 22.0f, 43) - 0.5f);
        if (crack < 0.012f) v -= 0.3f;
        return Ramp(kConcrete, 5, v, gx, gy);
    }

    // v0.15: the frozen stream out back. Grey-blue ice, swept clean in streaks, snow banked along both edges
    Color IceAt(int gx, int gy, int tx, int ty) const {
        (void)tx; (void)ty;
        static const Color kIce[5] = { { 96, 122, 168, 255 }, { 120, 148, 190, 255 }, { 146, 172, 208, 255 }, { 172, 196, 226, 255 }, { 200, 218, 240, 255 } };
        // a bending stream inside a straight band of tiles (rows -23 to -21)
        float cy = -22 * TS_ + 8 + std::sin(gx / 58.0f) * 9 + std::sin(gx / 21.0f) * 2.5f;
        float half = 9.5f + std::sin(gx / 37.0f + 1.0f) * 2.0f;
        float d = gy - cy;
        if (std::fabs(d) > half + 1.5f) {
            Color sn = Snow(gx, gy, false);
            if (std::fabs(d) < half + 4.0f) return MixC(sn, Color{ 150, 166, 210, 255 }, 0.25f);   // the bank's shadow
            return sn;
        }
        if (std::fabs(d) > half) return Ramp(kSnow, 6, 0.95f, gx, gy);                     // the lip of snow over the edge
        float v = 0.45f + (Fbm(gx / 20.0f, gy / 6.0f, 153) - 0.5f) * 0.6f;
        if (Fbm(gx / 40.0f + gy / 9.0f, gy / 3.0f, 154) > 0.62f) v += 0.35f;               // wind-swept snow on the ice
        float crack = std::fabs(Fbm(gx / 14.0f, gy / 14.0f, 155) - 0.5f);
        if (crack < 0.018f) v -= 0.35f; else if (crack < 0.03f) v += 0.12f;
        if (d < -half + 2) v -= 0.25f;                                                       // the shadow under the far bank
        return Ramp(kIce, 5, v, gx, gy);
    }

    // ---------------------------------------------------------------- walls
    Color WallFace(Room r, int gx, int gy, int lx, int fy) const {
        // fy: 0 at the top of the visible wall, 9 at the floor. Each room gets its own wall
        auto pick = [&](Color base, int d) { return Tone(base, d); };
        int shade = fy == 0 ? -34 : fy == 1 ? -14 : 0;             // the top of the wall sits in its own shadow
        if (fy >= 8) {                                             // skirting board
            Color sk = r == Room::Basement ? Color{ 52, 56, 68, 255 } : r == Room::Shed ? Color{ 54, 42, 36, 255 } : r == Room::Bathroom || r == Room::Kitchen ? Color{ 224, 220, 208, 255 } : Color{ 96, 66, 44, 255 };
            return pick(sk, fy == 8 ? 12 : -10);
        }
        switch (r) {
        case Room::LivingRoom: {                                   // pine panelling, the way a cabin should be
            static const Color p[4] = { { 168, 118, 70, 255 }, { 190, 138, 84, 255 }, { 208, 158, 100, 255 }, { 222, 176, 118, 255 } };
            int b = gx / 5;
            float v = 0.5f + (R01(b, 0, 51) - 0.5f) * 0.5f + (VNoise(gx * 1.7f, gy * 0.35f, 52) - 0.5f) * 0.25f;
            if (gx % 5 == 4) v -= 0.5f;
            return pick(Ramp(p, 4, v, gx, gy), shade);
        }
        case Room::Kitchen: {                                      // white tiles under a soft green band
            if (fy <= 2) return pick(Color{ 158, 184, 160, 255 }, shade + (fy == 2 ? -18 : 0));
            bool grout = (gx % 4 == 3) || fy == 5;
            return pick(grout ? Color{ 196, 198, 190, 255 } : Color{ 234, 234, 224, 255 }, shade + ((gx / 4 + fy / 3) % 5 == 0 ? -8 : 0));
        }
        case Room::Bathroom: {
            bool grout = (gx % 4 == 3) || fy == 3 || fy == 7;
            Color t = ((gx / 4) % 3 == 0 && fy < 3) ? Color{ 140, 188, 204, 255 } : Color{ 220, 232, 236, 255 };
            return pick(grout ? Color{ 176, 196, 204, 255 } : t, shade);
        }
        case Room::Hallway: {                                      // sage paper over white wainscot
            if (fy <= 3) {
                Color wp = { 150, 166, 140, 255 };
                bool dot = (gx % 6 == 2 && fy == 1) || (gx % 6 == 5 && fy == 3);
                return pick(dot ? Color{ 196, 204, 170, 255 } : wp, shade);
            }
            if (fy == 4) return Color{ 242, 238, 226, 255 };
            return pick(Color{ 222, 216, 200, 255 }, (gx % 8 == 7) ? -22 : (gx % 8 == 0) ? 8 : 0);
        }
        case Room::KidsRoom: {                                     // night blue with little stars
            Color wp = { 78, 100, 152, 255 };
            unsigned q = H2(gx / 7, 0, 61);
            bool star = (gx % 7 == (int)(q % 5) + 1) && fy == (int)((q >> 4) % 5) + 1;
            if (gx % 14 < 7) wp = Tone(wp, 8);
            return pick(star ? Color{ 250, 226, 140, 255 } : wp, shade);
        }
        case Room::ParentsRoom: {                                  // dusty rose with a small repeat
            Color wp = { 168, 124, 122, 255 };
            bool mot = ((gx % 6 == 1 || gx % 6 == 3) && fy == 3) || (gx % 6 == 2 && (fy == 2 || fy == 4)) || (gx % 6 == 5 && fy == 6);
            return pick(mot ? Color{ 196, 156, 150, 255 } : wp, shade);
        }
        case Room::Shed: {
            float v = 0.45f + (R01(gx / 6, 1, 71) - 0.5f) * 0.5f + (VNoise(gx * 1.9f, gy * 0.3f, 72) - 0.5f) * 0.3f;
            if (gx % 6 == 5) v -= 0.45f;
            return pick(Ramp(kShedWood, 5, v, gx, gy), shade);
        }
        default: {                                                 // the basement: painted block
            float v = 0.55f + (Fbm(gx / 9.0f, gy / 5.0f, 81) - 0.5f) * 0.5f;
            if (fy == 3 || fy == 7 || (gx + (fy > 3 ? 4 : 0)) % 8 == 7) v -= 0.28f;
            return pick(Ramp(kConcrete, 5, v, gx, gy), shade);
        }
        }
    }
    Color Cladding(int gx, int gy, int fy, bool corner) const {
        // falu red boards with a batten over every joint, white trim, a stone footing
        if (fy == 0) return kTrim;
        if (fy == 1) return Tone(kTrim, -34);
        if (corner) return fy >= 10 ? Tone(kTrim, -46) : Tone(kTrim, (gx % 2) ? -6 : 0);
        if (fy >= 10) {                                            // footing, half under the snow
            float v = 0.4f + R01(gx / 5, fy, 91) * 0.4f;
            return (gx % 5 == 4) ? Color{ 70, 72, 84, 255 } : MixC(Color{ 92, 94, 106, 255 }, Color{ 128, 130, 142, 255 }, v);
        }
        if (fy >= 2 && fy <= 1 + (int)(H2(gx, 0, 97) % 4) && H2(gx / 2, 1, 98) % 5 == 0 && gx % 2 == 0) return Color{ 222, 236, 250, 255 };   // icicles under the eave
        int in = gx % 6;
        float v = 0.48f + (R01(gx / 6, 2, 92) - 0.5f) * 0.32f + (VNoise(gx * 1.6f, gy * 0.33f, 93) - 0.5f) * 0.22f - fy * 0.012f;
        if (in == 5) v += 0.18f;            // the batten catches the light
        if (in == 0) v -= 0.30f;            // and throws a thin shadow
        return Ramp(kRed, 5, v, gx, gy);
    }

    void Bake() {
        c.Init(World::W * TS_, World::ROWS * TS_, Color{ 0, 0, 0, 255 });
        c.oy = World::TOP * TS_;
        for (int ty = World::TOP; ty < World::H; ++ty) for (int tx = 0; tx < World::W; ++tx) {
            Tile t = T(tx, ty);
            Room r = Rm(tx, ty);
            for (int ly = 0; ly < TS_; ++ly) for (int lx = 0; lx < TS_; ++lx) {
                int gx = tx * TS_ + lx, gy = ty * TS_ + ly;
                Color col = { 4, 4, 8, 255 };
                switch (t) {
                case Tile::Vidda: col = Snow(gx, gy, true); break;
                case Tile::Grass: case Tile::Fence: case Tile::Bush: case Tile::Chain: case Tile::ChainGap: col = Snow(gx, gy, ty == World::TOP || tx == 0 || ty >= 30 || tx == World::W - 1); break;
                case Tile::Ice: col = IceAt(gx, gy, tx, ty); break;
                case Tile::Gravel: col = Road(gx, gy, tx); break;
                case Tile::Door: {
                    bool outer = Out(T(tx, ty + 1)) || Out(T(tx, ty - 1));
                    col = Planks(gx, gy, outer ? kOak : kPine, 32, outer ? -0.08f : -0.04f);
                } break;
                case Tile::BasementDoor: col = Planks(gx, gy, kOak, 48, 0); break;
                case Tile::Floor: case Tile::ShedFloor:
                    if (r == Room::Kitchen) col = Kitchen(gx, gy);
                    else if (r == Room::Bathroom) col = Bath(gx, gy);
                    else if (r == Room::KidsRoom) col = Carpet(gx, gy);
                    else if (r == Room::Basement) col = Concrete(gx, gy);
                    else if (t == Tile::ShedFloor) col = Planks(gx, gy, kShedWood, 32, 0);
                    else if (r == Room::LivingRoom) col = Planks(gx, gy, kPine, 48, 0.04f);
                    else if (r == Room::ParentsRoom) col = Planks(gx, gy, kOak, 48, 0.10f);
                    else col = Planks(gx, gy, kOak, 48, 0.02f);
                    break;
                case Tile::Wall: {
                    Tile below = T(tx, ty + 1);
                    bool outer = Out(T(tx, ty - 1)) || Out(T(tx, ty + 1)) || Out(T(tx - 1, ty)) || Out(T(tx + 1, ty));
                    bool face = FloorLike(below) && below != Tile::Door, clad = Out(below);
                    int capH = face ? 6 : clad ? 5 : TS_;
                    if (ly < capH) {
                        // the top of the wall. Outer walls carry snow
                        if (outer) col = Ramp(kSnow, 6, 0.72f + (Fbm(gx / 7.0f, gy / 7.0f, 95) - 0.5f) * 0.5f, gx, gy);
                        else {
                            float v = 0.42f + (R01(gx / 3, gy / 3, 96) - 0.5f) * 0.2f;
                            if (ly == 0 && !Wall(tx, ty - 1)) v += 0.4f;
                            if (lx == 0 && !Wall(tx - 1, ty) && T(tx - 1, ty) != Tile::Door) v += 0.25f;
                            if (lx == TS_ - 1 && !Wall(tx + 1, ty) && T(tx + 1, ty) != Tile::Door) v -= 0.25f;
                            if (ly == capH - 1 && face) v -= 0.3f;
                            col = Ramp(kTimber, 4, v, gx, gy);
                        }
                    } else if (face) col = WallFace(Rm(tx, ty + 1), gx, gy, lx, ly - 6);
                    else {
                        bool corner = !Wall(tx - 1, ty) ? lx < 3 : !Wall(tx + 1, ty) ? lx > 12 : false;
                        col = Cladding(gx, gy, ly - 5, corner);
                    }
                } break;
                default: break;
                }
                c.Set(gx, gy, col);
            }
        }
        Occlusion();
        Rugs();
        Fences();
        ChainFence();
        Backyard();
        Rocks();
        Drifts();
    }

    // soft shadow where a floor meets a wall, and under the eaves outside
    void Occlusion() {
        for (int ty = World::TOP; ty < World::H; ++ty) for (int tx = 0; tx < World::W; ++tx) {
            Tile t = T(tx, ty);
            bool fl = t == Tile::Floor || t == Tile::ShedFloor || t == Tile::Door || t == Tile::BasementDoor;
            bool out = Out(t) && t != Tile::Fence;
            if (!fl && !out) continue;
            for (int ly = 0; ly < TS_; ++ly) for (int lx = 0; lx < TS_; ++lx) {
                float k = 0;
                if (Wall(tx, ty - 1)) k = std::max(k, (fl ? 0.42f : 0.30f) * (1 - ly / (fl ? 9.0f : 11.0f)));
                if (fl) {
                    if (Wall(tx - 1, ty)) k = std::max(k, 0.26f * (1 - lx / 5.0f));
                    if (Wall(tx + 1, ty)) k = std::max(k, 0.26f * (1 - (15 - lx) / 5.0f));
                    if (Wall(tx, ty + 1)) k = std::max(k, 0.16f * (1 - (15 - ly) / 3.0f));
                    if (Wall(tx - 1, ty - 1) && !Wall(tx, ty - 1) && !Wall(tx - 1, ty)) k = std::max(k, 0.2f * (1 - std::hypot((float)lx, (float)ly) / 6.0f));
                    if (Wall(tx + 1, ty - 1) && !Wall(tx, ty - 1) && !Wall(tx + 1, ty)) k = std::max(k, 0.2f * (1 - std::hypot((float)(15 - lx), (float)ly) / 6.0f));
                }
                if (k <= 0.01f) continue;
                int gx = tx * TS_ + lx, gy = ty * TS_ + ly;
                // dithered, so it stays crisp
                float th = (kBayer[gy & 3][gx & 3] + 0.5f) / 16.0f;
                float steps = std::floor(k * 5 + th) / 5.0f;
                if (steps > 0) c.Shade(gx, gy, -steps);
            }
        }
    }

    // ---------------------------------------------------------------- rugs and mats
    void Rug(int x0, int y0, int x1, int y1, int style) {
        for (int y = y0; y <= y1; ++y) for (int x = x0; x <= x1; ++x) {
            int dx = std::min(x - x0, x1 - x), dy = std::min(y - y0, y1 - y), edge = std::min(dx, dy);
            Color col;
            float weave = (R01(x, y, 101) - 0.5f) * 14 + ((x + y) % 2 ? 3 : -3);
            if (style == 0) {
                // the living room rug: red wool, cream border, eight pointed stars
                Color red = { 142, 44, 46, 255 }, cream = { 226, 212, 180, 255 }, navy = { 44, 56, 96, 255 };
                col = red;
                if (edge <= 1) col = navy;
                else if (edge == 2 || edge == 5) col = cream;
                else if (edge == 3 || edge == 4) col = ((x + y) / 2 % 2) ? cream : red;
                else {
                    int cx = (x - x0 - 6) % 22 - 11, cy = (y - (y0 + y1) / 2);
                    int ax = std::abs(cx), ay = std::abs(cy);
                    bool star = (ax + ay <= 7 && std::abs(ax - ay) <= 2) || (ax <= 1 && ay <= 9) || (ay <= 1 && ax <= 9) || (ax + ay <= 3);
                    if (star && std::abs(cy) <= 9) col = (ax + ay <= 1) ? navy : cream;
                    else if ((x - x0) % 22 == 17 && ay % 4 == 0) col = navy;
                }
                // fringe on the short ends
                if ((x == x0 - 0 || x == x1) && (y % 2)) col = cream;
            } else if (style == 1) {
                // the hallway runner
                Color g = { 62, 92, 82, 255 }, gold = { 196, 164, 96, 255 };
                col = g;
                if (dy == 0) col = Tone(g, -22);
                else if (dy == 2) col = gold;
                else if (dy >= 4 && (x % 12 < 2) && dy == (y1 - y0) / 2) col = gold;
                else if (dy >= 4 && ((x + (y - y0)) % 12 == 0 || (x - (y - y0) + 600) % 12 == 0)) col = Tone(g, 22);
            } else if (style == 2) {
                // the play mat in his room: a little town to drive on
                Color road = { 86, 90, 104, 255 }, grass = { 96, 152, 112, 255 };
                col = grass;
                int rx = x - x0, ry = y - y0, W = x1 - x0, Hh = y1 - y0;
                bool onRoad = (ry >= Hh / 2 - 3 && ry <= Hh / 2 + 3) || (rx >= W / 3 - 3 && rx <= W / 3 + 3) || (rx >= W - 12 && rx <= W - 6);
                if (onRoad) col = road;
                if (onRoad && ((ry == Hh / 2 && rx % 6 < 3) || (rx == W / 3 && ry % 6 < 3))) col = Color{ 236, 226, 150, 255 };
                if (!onRoad && R01(rx / 9, ry / 9, 111) > 0.62f && rx % 9 > 1 && rx % 9 < 7 && ry % 9 > 2 && ry % 9 < 7) col = (ry % 9 < 4) ? Color{ 190, 70, 64, 255 } : Color{ 232, 222, 200, 255 };
                if (edge == 0) col = Color{ 232, 222, 200, 255 };
            } else if (style == 3) {
                Color m = { 224, 228, 232, 255 };                  // bath mat
                col = (edge == 0) ? Tone(m, -30) : ((x + y) % 3 == 0 ? Tone(m, -12) : m);
            } else if (style == 4) {
                Color m = { 206, 190, 158, 255 };                  // a sheepskin by their bed
                col = (edge <= 1 && R01(x, y, 121) > 0.4f) ? Color{ 0, 0, 0, 0 } : Tone(m, (int)((Fbm(x / 3.0f, y / 3.0f, 122) - 0.5f) * 50));
            } else {
                Color m = { 112, 84, 56, 255 };                    // door mat
                col = (edge == 0) ? Tone(m, -30) : ((x * 3 + y) % 4 == 0 ? Tone(m, 18) : m);
            }
            if (!col.a) continue;
            Color under = c.Get(x, y);
            col = Tone(col, (int)weave / 2);
            // keep the wall's shadow on it
            float lum0 = (under.r + under.g + under.b) / 3.0f;
            (void)lum0;
            c.Set(x, y, col);
        }
        // a thin shadow along the bottom edge so it sits on the floor
        for (int x = x0; x <= x1; ++x) c.Shade(x, y1 + 1, -0.22f);
    }
    void Rugs() {
        Rug(17 * TS_ + 2, 14 * TS_ + 6, 26 * TS_ - 3, 18 * TS_ - 6, 0);          // living room
        Rug(4 * TS_, 10 * TS_ + 10, 27 * TS_ + 8, 11 * TS_ + 9, 1);             // hallway runner
        Rug(4 * TS_ + 6, 5 * TS_ + 4, 8 * TS_ + 2, 7 * TS_ + 12, 2);            // his room
        Rug(12 * TS_ - 2, 6 * TS_ + 2, 14 * TS_ - 4, 6 * TS_ + 12, 3);          // bathroom
        Rug(19 * TS_ + 2, 6 * TS_ + 6, 20 * TS_ + 12, 7 * TS_ + 10, 4);         // parents
        Rug(19 * TS_ + 4, 19 * TS_ + 4, 21 * TS_ - 5, 19 * TS_ + 12, 5);        // inside the front door
        // re-shade the rugs where a wall stands over them
        for (int ty = 0; ty < 34; ++ty) for (int tx = 0; tx < World::W; ++tx) {
            if (T(tx, ty) != Tile::Floor || !Wall(tx, ty - 1)) continue;
            for (int ly = 0; ly < 6; ++ly) for (int lx = 0; lx < TS_; ++lx) {
                int gx = tx * TS_ + lx, gy = ty * TS_ + ly;
                Color px = c.Get(gx, gy);
                bool rug = (px.r > 130 && px.g < 70) || (px.g > px.r + 20 && px.g < 110);
                if (rug) c.Shade(gx, gy, -0.3f * (1 - ly / 6.0f));
            }
        }
    }

    // ---------------------------------------------------------------- fences, rocks, drifts
    void Fences() {
        Color wood = { 104, 74, 50, 255 }, dark = { 70, 50, 38, 255 }, cap = { 240, 244, 252, 255 }, sh = { 132, 150, 200, 255 };
        for (int ty = World::TOP; ty < World::H; ++ty) for (int tx = 0; tx < World::W; ++tx) {
            if (T(tx, ty) != Tile::Fence) continue;
            int px = tx * TS_, py = ty * TS_;
            bool vertical = T(tx, ty - 1) == Tile::Fence && T(tx, ty + 1) == Tile::Fence && T(tx - 1, ty) != Tile::Fence;
            if (vertical) {
                c.Rect(px + 10, py, 3, TS_, sh);                                   // shadow on the snow
                c.Rect(px + 6, py, 2, TS_, wood); c.Rect(px + 8, py, 1, TS_, dark);
                if (ty % 2 == 0) { c.Rect(px + 5, py + 3, 5, 8, wood); c.Rect(px + 9, py + 3, 1, 8, dark); c.Rect(px + 5, py + 2, 5, 2, cap); c.Rect(px + 5, py + 11, 6, 2, sh); }
                c.Rect(px + 6, py, 1, TS_, Tone(wood, 20));
            } else {
                c.Rect(px, py + 13, TS_, 2, sh);
                c.Rect(px, py + 5, TS_, 2, wood); c.Rect(px, py + 7, TS_, 1, dark);
                c.Rect(px, py + 10, TS_, 2, wood); c.Rect(px, py + 12, TS_, 1, dark);
                c.Rect(px, py + 4, TS_, 1, cap); c.Rect(px + (int)(H2(tx, ty) % 9), py + 9, 5, 1, cap);
                if (tx % 2 == 0) {
                    c.Rect(px + 6, py + 2, 4, 12, wood); c.Rect(px + 9, py + 2, 1, 12, dark); c.Rect(px + 6, py + 2, 1, 12, Tone(wood, 22));
                    c.Rect(px + 5, py, 6, 3, cap); c.Rect(px + 6, py + 3, 3, 1, Color{ 206, 218, 240, 255 });
                    c.Rect(px + 5, py + 14, 7, 1, sh);
                }
            }
        }
    }
    // v0.15: the old chain-link fence across the backyard. You see straight through it
    void ChainFence() {
        Color post = { 70, 74, 88, 255 }, postL = { 120, 126, 142, 255 }, mesh = { 92, 98, 116, 255 }, sh = { 150, 166, 210, 255 }, cap = { 244, 248, 254, 255 };
        for (int ty = World::TOP; ty < World::H; ++ty) for (int tx = 0; tx < World::W; ++tx) {
            Tile t = T(tx, ty);
            if (t != Tile::Chain && t != Tile::ChainGap) continue;
            int px = tx * TS_, py = ty * TS_;
            bool gap = t == Tile::ChainGap;
            for (int lx = 0; lx < TS_; ++lx) {
                c.Shade(px + lx, py + 13, -0.12f);                              // its thin shadow on the snow
                c.Set(px + lx, py + 2, Tone(post, 14));                         // top rail
                c.Set(px + lx, py + 3, post);
                if (lx % 3 == 0 && R01(px + lx, py, 161) > 0.3f) c.Set(px + lx, py + 1, cap);   // snow on the rail
            }
            for (int ly = 4; ly < 13; ++ly) for (int lx = 0; lx < TS_; ++lx) {
                int gx = px + lx, gy = py + ly;
                bool diag = ((gx + gy) % 4 == 0) || ((gx - gy + 400) % 4 == 0);
                if (!diag) continue;
                if (gap) {
                    // torn: the mesh curls back from a hole in the middle
                    float d = std::hypot(lx - 8.0f, (ly - 9.0f) * 1.3f);
                    if (d < 5.0f) continue;
                    if (d < 6.2f) { c.Set(gx, gy, Tone(mesh, 30)); continue; }
                }
                Color under = c.Get(gx, gy);
                c.Set(gx, gy, MixC(under, mesh, 0.62f));
                if ((gx + gy) % 8 == 0 && R01(gx, gy, 162) > 0.6f) c.Set(gx, gy - 1, cap);   // frost caught in the links
            }
            if (tx % 3 == 0 || (T(tx - 1, ty) != Tile::Chain && T(tx - 1, ty) != Tile::ChainGap)) {
                c.Rect(px + 1, py + 1, 2, 13, post); c.Rect(px + 1, py + 1, 1, 13, postL); c.Rect(px + 1, py, 2, 2, cap);
                c.Rect(px + 3, py + 13, 3, 1, sh);
            }
        }
    }
    // v0.15: small boot prints that start at the swing and walk north. Then stop. The snow never fills them
    void Backyard() {
        Color pr = { 120, 138, 190, 255 }, lip = { 250, 252, 255, 255 };
        float x0 = 21 * TS_ + 8.0f, y0 = -7 * TS_ + 2.0f, x1 = 23 * TS_ + 8.0f, y1 = -10 * TS_ + 9.0f;
        int n = 11;
        for (int i = 0; i < n; ++i) {
            float k = i / (float)(n - 1);
            int x = (int)(x0 + (x1 - x0) * k) + ((i % 2) ? 2 : -2), y = (int)(y0 + (y1 - y0) * k);
            c.Rect(x, y, 2, 3, pr); c.Rect(x, y + 3, 2, 1, lip); c.Rect(x, y - 1, 2, 1, Tone(pr, 30));
        }
        // a trodden ring around the swing, filled with new snow except where somebody stood
        for (int a = 0; a < 40; ++a) {
            float ang = a * 0.157f;
            int x = 21 * TS_ + 8 + (int)(std::cos(ang) * 11), y = -7 * TS_ + 12 + (int)(std::sin(ang) * 4);
            c.Shade(x, y, -0.08f);
        }
    }

    void Rocks() {
        for (int ty = World::TOP; ty < World::H; ++ty) for (int tx = 0; tx < World::W; ++tx) {
            if (T(tx, ty) != Tile::Vidda) continue;
            unsigned h = H2(tx, ty, 131);
            if (h % 11 != 0) continue;
            int rx = tx * TS_ + (int)(h % 7), ry = ty * TS_ + 3 + (int)((h >> 6) % 6), rw = 6 + (int)((h >> 9) % 4);
            c.Rect(rx + 1, ry + 6, rw + 1, 1, Color{ 150, 168, 214, 255 });
            c.Rect(rx, ry + 2, rw, 4, Color{ 88, 90, 104, 255 }); c.Rect(rx + 1, ry + 1, rw - 2, 1, Color{ 104, 106, 120, 255 });
            c.Rect(rx + rw - 2, ry + 3, 2, 3, Color{ 64, 66, 82, 255 });
            c.Rect(rx + 1, ry, rw - 3, 2, Color{ 244, 248, 254, 255 }); c.Rect(rx, ry + 1, 2, 2, Color{ 230, 238, 250, 255 });   // snow cap
        }
    }
    // snow lies up against the south wall of the house and the shed
    void Drifts() {
        for (int ty = World::TOP + 1; ty < 34; ++ty) for (int tx = 0; tx < World::W; ++tx) {
            if (!Wall(tx, ty) || !Out(T(tx, ty + 1))) continue;
            for (int lx = 0; lx < TS_; ++lx) {
                int gx = tx * TS_ + lx;
                int hgt = 2 + (int)(Fbm(gx / 9.0f, 0, 141) * 5.0f);
                for (int k = 0; k < hgt; ++k) {
                    int gy = (ty + 1) * TS_ - k;
                    c.Set(gx, gy, k == hgt - 1 ? Color{ 246, 250, 255, 255 } : Ramp(kSnow, 6, 0.78f + k * 0.03f, gx, gy));
                }
            }
        }
    }
};

// the plateau goes on south of the map: the road, the snow, a few rocks
inline void BakeSouth(const World& w, Canvas& c, int rows) {
    Canvas dummy; WorldBaker b(w, dummy);
    c.Init(World::W * TS_, rows * TS_);
    for (int y = 0; y < c.h; ++y) for (int x = 0; x < c.w; ++x) {
        int tx = x / TS_, gy = y + 34 * TS_;
        c.Set(x, y, (tx >= 19 && tx <= 21) ? b.Road(x, gy, tx) : b.Snow(x, gy, true));
    }
    for (int ty = 0; ty < rows; ++ty) for (int tx = 0; tx < World::W; ++tx) {
        unsigned h = H2(tx, ty + 34, 131);
        if (h % 9 != 0 || (tx >= 18 && tx <= 22)) continue;
        int rx = tx * TS_ + (int)(h % 7), ry = ty * TS_ + 3 + (int)((h >> 6) % 6), rw = 6 + (int)((h >> 9) % 4);
        c.Rect(rx + 1, ry + 6, rw + 1, 1, Color{ 150, 168, 214, 255 });
        c.Rect(rx, ry + 2, rw, 4, Color{ 88, 90, 104, 255 }); c.Rect(rx + 1, ry + 1, rw - 2, 1, Color{ 104, 106, 120, 255 });
        c.Rect(rx + rw - 2, ry + 3, 2, 3, Color{ 64, 66, 82, 255 });
        c.Rect(rx + 1, ry, rw - 3, 2, Color{ 248, 251, 255, 255 }); c.Rect(rx, ry + 1, 2, 2, Color{ 237, 243, 253, 255 });
    }
}

// ---------------------------------------------------------------- roofs
// Seen from above: two slopes under a metre of snow, a ridge, a chimney. Drawn over the house while you are outside
inline void BakeRoof(Canvas& c, int wpx, int hpx, int chimX, int chimY, int seed) {
    c.Init(wpx, hpx);
    static const Color kSlate[4] = { { 34, 38, 54, 255 }, { 46, 52, 70, 255 }, { 60, 66, 86, 255 }, { 78, 86, 108, 255 } };
    int ridge = hpx / 2 - 4;
    for (int y = 0; y < hpx; ++y) for (int x = 0; x < wpx; ++x) {
        bool north = y < ridge;
        float slope = north ? (float)y / ridge : (float)(hpx - y) / (hpx - ridge);          // 0 at the eave, 1 at the ridge
        // the far slope sits in shade, the near one catches what light there is
        float v = (north ? 0.30f : 0.74f) + slope * (north ? 0.16f : 0.12f);
        auto hgt = [&](float fx, float fy) { return Fbm(fx / 38.0f, fy / 16.0f, seed) + Fbm(fx / 9.0f, fy / 6.0f, seed + 3) * 0.2f; };
        v += std::max(-0.22f, std::min(0.18f, (hgt((float)x, (float)y) - hgt(x + 3.0f, y + 3.0f)) * 4.0f));
        // the snow lies in layers that sag between the rafters
        float sag = y + std::sin(x * 0.035f + seed) * 3.0f + Fbm(x / 30.0f, y / 30.0f, seed + 5) * 8.0f;
        int band = ((int)sag) % 26;
        if (band == 0) v -= 0.16f; else if (band == 1) v += 0.10f;
        if (y >= ridge - 1 && y <= ridge) v = 0.98f;                                       // the ridge
        if (y == ridge + 1 || y == ridge + 2) v -= 0.10f;
        if (!north && y >= hpx - 14) v -= (y - (hpx - 14)) * 0.012f;                       // it curls over the edge
        Color col = Ramp(kSnow, 6, v, x, y);
        bool bare = false;
        float sv = 0.5f;
        if (!north && y >= hpx - 8) {                                                      // where it has slid off: bare slate along the eave
            int lip = hpx - 8 + (int)(Fbm(x / 13.0f, 3, seed + 7) * 6.0f);
            if (y > lip) bare = true; else if (y == lip) col = Color{ 150, 170, 216, 255 };
        }
        if (chimX >= 0) {                                                                  // and the stove pipe keeps a patch clear
            float d = std::hypot((x - (chimX + 5.5f)) * 0.8f, (float)(y - (chimY + 8)));
            float edge = 5.5f + Fbm(x / 5.0f, y / 5.0f, seed + 9) * 3.5f;
            if (d < edge) { bare = true; sv = 0.35f + d / edge * 0.3f; } else if (d < edge + 1.2f) col = Color{ 150, 170, 216, 255 };
        }
        if (bare) {
            int row = y / 4, tx = (x + (row % 2) * 4) % 8;
            float k = sv + (R01((x + (row % 2) * 4) / 8, row, seed + 11) - 0.5f) * 0.4f;
            if (y % 4 == 3) k -= 0.45f; else if (tx == 7) k -= 0.3f; else if (y % 4 == 0) k += 0.15f;
            col = Ramp(kSlate, 4, k, x, y);
        }
        if (y >= hpx - 2) col = (y == hpx - 2) ? kTrim : Tone(kTrim, -56);                 // the fascia board
        if (north && y < 2) col = (y == 0) ? Color{ 110, 126, 176, 255 } : Color{ 150, 168, 214, 255 };
        if (x < 3 || x >= wpx - 3) col = (x == 0 || x == wpx - 1) ? Tone(kTrim, -70) : (x == 1 || x == wpx - 2) ? Tone(kTrim, north ? -40 : 0) : (north ? Color{ 140, 158, 206, 255 } : Color{ 248, 251, 255, 255 });   // bargeboards on the gable ends
        c.Set(x, y, col);
    }
    if (chimX >= 0) {
        // the chimney: brick, a dark throat, snow on the windward side, a long soft shadow
        for (int k = 0; k < 16; ++k) for (int j = 0; j < 9; ++j) c.Shade(chimX + 9 + k, chimY + 3 + j + k / 3, -0.20f * (1 - k / 16.0f));
        c.Rect(chimX, chimY, 11, 12, Color{ 96, 50, 44, 255 });
        for (int j = 0; j < 12; ++j) for (int i = 0; i < 11; ++i) {
            bool mortar = (j % 3 == 2) || ((i + (j / 3) * 2) % 5 == 4);
            c.Set(chimX + i, chimY + j, mortar ? Color{ 150, 124, 110, 255 } : Tone(Color{ 124, 62, 50, 255 }, (int)(R01(i / 5, j / 3, seed) * 26) - 13 - (i > 7 ? 22 : 0)));
        }
        c.Rect(chimX - 1, chimY - 1, 13, 3, Color{ 70, 66, 72, 255 }); c.Rect(chimX - 1, chimY - 1, 13, 1, Color{ 110, 106, 112, 255 });
        c.Rect(chimX + 3, chimY, 5, 2, Color{ 16, 12, 14, 255 });
        c.Rect(chimX - 1, chimY - 2, 7, 1, Color{ 244, 248, 254, 255 }); c.Rect(chimX - 2, chimY + 9, 3, 3, Color{ 244, 248, 254, 255 });
    }
}

} // namespace art
