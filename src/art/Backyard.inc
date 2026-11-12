// 600 Seconds - art/Backyard.inc (v0.15)
// Everything out back: the snow beast, the old car, the swing, the spruce, the little cross, the hole in the ice,
// the dish, the painted board with its lights, the weathervane, the bushes and the dog at the far fence.
// Same rules as Sprites.inc: rectangles, a lit edge, a shaded edge, an outline.

// a tiny 3x5 alphabet for the painted board. One row per glyph line, 3 bits each
static const unsigned char kGlyph[26][5] = {
    { 2, 5, 7, 5, 5 }, { 6, 5, 6, 5, 6 }, { 3, 4, 4, 4, 3 }, { 6, 5, 5, 5, 6 }, { 7, 4, 6, 4, 7 }, { 7, 4, 6, 4, 4 }, { 3, 4, 5, 5, 3 },
    { 5, 5, 7, 5, 5 }, { 7, 2, 2, 2, 7 }, { 1, 1, 1, 5, 2 }, { 5, 5, 6, 5, 5 }, { 4, 4, 4, 4, 7 }, { 5, 7, 7, 5, 5 }, { 6, 5, 5, 5, 5 },
    { 2, 5, 5, 5, 2 }, { 6, 5, 6, 4, 4 }, { 2, 5, 5, 7, 3 }, { 6, 5, 6, 5, 5 }, { 3, 4, 2, 1, 6 }, { 7, 2, 2, 2, 2 }, { 5, 5, 5, 5, 7 },
    { 5, 5, 5, 5, 2 }, { 5, 5, 7, 7, 5 }, { 5, 5, 2, 5, 5 }, { 5, 5, 2, 2, 2 }, { 7, 1, 2, 4, 7 } };

// the board: 26 letters in three rows (9, 9, 8). Where letter k sits, in pixels from the board's top left
static Vector2 BoardLetter(int k) { int row = k < 9 ? 0 : k < 18 ? 1 : 2, col = k - row * 9; return { 3.0f + col * 5 + (row == 2 ? 2 : 0), 6.0f + row * 9 }; }
static const int BOARD_X = 32 * TS + 2, BOARD_Y = 0 * TS - 14, BOARD_W = 46, BOARD_H = 31;

// v0.15: the lights on the board. Most of the time they just twinkle. In a dark loop, after dark, they spell at you
static bool BoardSpells() { return story.BoardSpells(); }
static int BoardLit() {
    // which letter is lit right now (-1: none, -2: all twinkling)
    if (BoardSpells()) {
        static const char* word = "BELOW";
        float t = std::fmod(globalT, 7.5f);
        int i = (int)(t / 0.9f);
        if (i < 5 && std::fmod(t, 0.9f) < 0.65f) return word[i] - 'A';
        return -1;
    }
    if (story.PowerOut()) return -1;
    return -2;
}
static Color BulbCol(int k) { static const Color c[4] = { { 255, 214, 110, 255 }, { 250, 84, 76, 255 }, { 110, 180, 255, 255 }, { 120, 226, 136, 255 } }; return c[k % 4]; }
static bool BoardBulbOn(int k) {
    int lit = BoardLit();
    if (lit == -2) return std::fmod(globalT * 0.6f + k * 0.29f, 1.6f) < 1.1f;
    return lit == k;
}

static void DrawBush(int tx, int ty) {
    unsigned h = Hash(tx, ty * 7 + 3);
    float x = tx * TS + (float)(h % 5) - 2, y = ty * TS + (float)((h >> 4) % 3) - 1;
    Color dk = { 26, 46, 52, 255 }, md = { 40, 70, 68, 255 }, lt = { 60, 98, 86, 255 }, sn = { 240, 246, 254, 255 }, sh = { 186, 202, 236, 255 };
    DrawEllipse((int)x + 8, (int)y + 15, 11, 2.5f, Fade(Color{ 70, 86, 150, 255 }, 0.35f));
    // two or three round clumps, never the same twice
    int n = 2 + (int)((h >> 7) % 2);
    for (int k = 0; k < n; ++k) {
        unsigned q = Hash(tx * 3 + k, ty + 91);
        float cx = x + 3 + k * (10.0f / n) + (q % 4), cy = y + 8 - (q >> 3) % 3, r = 5.0f + (q >> 6) % 3;
        DrawCircle((int)cx, (int)cy, r + 1, kInk);
    }
    for (int k = 0; k < n; ++k) {
        unsigned q = Hash(tx * 3 + k, ty + 91);
        float cx = x + 3 + k * (10.0f / n) + (q % 4), cy = y + 8 - (q >> 3) % 3, r = 5.0f + (q >> 6) % 3;
        DrawCircle((int)cx, (int)cy, r, dk);
        DrawCircle((int)cx - 1, (int)cy - 1, r - 1.5f, md);
        DrawCircle((int)cx - 2, (int)cy - 2, r - 3.5f, lt);
        DrawRectangle((int)(cx - r + 1), (int)(cy - r), (int)(r * 2 - 2), 2, sn);            // snow on the crown
        DrawRectangle((int)(cx - r + 2), (int)(cy - r - 1), (int)(r * 2 - 5), 1, sn);
        DrawRectangle((int)(cx - r + 2), (int)(cy - r + 2), (int)r, 1, sh);
    }
    for (int k = 0; k < 5; ++k) DrawRectangle((int)x + 1 + (int)((h >> (k * 3)) % 15), (int)y + 7 + (int)((h >> (k * 2 + 5)) % 6), 1, 1, Color{ 20, 34, 40, 255 });   // twigs
    if ((h >> 9) % 4 == 0) { DrawRectangle((int)x + 6, (int)y + 9, 1, 1, Color{ 200, 40, 50, 255 }); DrawRectangle((int)x + 8, (int)y + 10, 1, 1, Color{ 200, 40, 50, 255 }); }   // rowan berries nobody picked
    DrawRectangle((int)x - 1, (int)y + 13, 18, 2, sn);                                       // snow drifted against the foot
}

// the husky at the far fence. Doggo growls at it. Get close and there's nobody
static float huskyA = 0;
static void DrawHusky() {
    if (huskyA <= 0.01f) return;
    float x = 12 * TS + 8.0f, y = World::TOP * TS + 13.0f;
    Color c = Fade(Color{ 12, 14, 24, 255 }, huskyA);
    auto R = [&](float ox, float oy, float w, float h) { DrawRectangle((int)(x + ox), (int)(y + oy), (int)w, (int)h, c); };
    R(-7, -8, 14, 7); R(-6, -1, 2, 5); R(4, -1, 2, 5); R(-2, -1, 2, 4);                  // body, legs
    R(5, -14, 7, 7); R(6, -17, 2, 3); R(10, -17, 2, 3); R(11, -10, 3, 2);               // head, two ears, muzzle
    R(-10, -12, 3, 5); R(-11, -14, 2, 3);                                                // the tail curled up
    float blink = std::fmod(globalT, 4.0f) < 0.12f ? 0 : 1;
    DrawRectangle((int)x + 7, (int)y - 12, 1, 1, Fade(Color{ 200, 220, 255, 255 }, huskyA * 0.8f * blink));
    DrawRectangle((int)x + 10, (int)y - 12, 1, 1, Fade(Color{ 200, 220, 255, 255 }, huskyA * 0.8f * blink));
}

// the weathervane on the shed roof: not a rooster. A fat rodent, paws up
static void DrawVane() {
    float x = 34 * TS + 8.0f, y = SROOF_Y + SROOF_H / 2.0f - 6;
    float a = shedRoofA;
    if (a < 0.02f) return;
    auto R = [&](float ox, float oy, float w, float h, Color c) { DrawRectangle((int)(x + ox), (int)(y + oy), (int)w, (int)h, Fade(c, a)); };
    Color iron = { 40, 40, 50, 255 }, rust = { 120, 70, 44, 255 };
    R(0, -16, 1, 17, iron); R(-1, 0, 3, 1, iron);
    // which way it points: it swings with the wind, and wildly in the storm
    float sw = story.Storm() ? std::sin(globalT * 3.7f) : std::sin(globalT * 0.4f);
    int f = sw >= 0 ? 1 : -1;
    auto F = [&](float ox, float oy, float w, float h, Color c) { R(f > 0 ? ox : -ox - w + 1, oy, w, h, c); };
    F(-7, -13, 14, 1, iron); F(6, -15, 1, 5, iron); F(-8, -14, 2, 3, iron);              // the arrow
    F(-4, -22, 9, 6, iron); F(-3, -23, 7, 1, iron); F(3, -25, 3, 3, iron);               // the fat little body, the head
    F(4, -26, 1, 1, iron); F(6, -20, 2, 1, iron); F(6, -18, 2, 1, iron);                 // an ear, two paws up
    F(-6, -19, 2, 3, iron);                                                              // the tail
    F(-3, -21, 3, 1, rust); F(0, -18, 2, 1, rust);
    R(-1, -5, 3, 2, Color{ 244, 248, 254, 255 });                                        // snow where it meets the ridge
}

// returns true if it drew the object (so DrawObj stops)
static bool DrawOutback(const Obj& o, float x, float y) {
    const std::string& id = o.id;
    auto R = [&](float ox, float oy, float w, float h, Color c) { DrawRectangle((int)(x + ox), (int)(y + oy), (int)w, (int)h, c); };
    auto B = [&](float ox, float oy, float w, float h, Color c) {
        R(ox - 1, oy - 1, w + 2, h + 2, Mix(c, kInk, 0.72f));
        R(ox, oy, w, h, c);
        R(ox, oy, w, 1, Shade(c, 24)); R(ox, oy + 1, 1, h - 1, Shade(c, 10));
        R(ox + w - 1, oy + 1, 1, h - 1, Shade(c, -20)); R(ox, oy + h - 1, w, 1, Shade(c, -28));
    };
    const Color snow = { 244, 248, 254, 255 }, snowS = { 190, 204, 234, 255 }, shadowC = Fade(Color{ 70, 86, 150, 255 }, 0.38f);
    if (id == "beast") {
        // a ride-on toy the size of a pony. Grey-white plush, three horns, button eyes, a zip down its belly.
        // Drawn at 1.5x around its feet, so it towers over the kid a bit
        rlDrawRenderBatchActive();
        rlPushMatrix();
        rlTranslatef(x + 8, y + 15, 0); rlScalef(1.5f, 1.5f, 1);
        x = -8; y = -15;
        bool in = hidden && story.hideSpot == "beast";
        float breathe = in ? std::sin(globalT * 2.2f) * 0.6f : 0;
        Color fur = { 214, 210, 204, 255 }, furS = { 172, 168, 166, 255 }, furD = { 130, 126, 130, 255 }, cream = { 234, 224, 204, 255 };
        Color horn = { 206, 184, 140, 255 }, red = { 176, 40, 44, 255 }, gold = { 226, 196, 100, 255 };
        DrawEllipse((int)x + 8, (int)y + 16, 19, 3.5f, shadowC);
        // stubby legs on little wheels: it's a ride-on, after all
        const float lx[4] = { -7, -2, 12, 17 };
        for (int k = 0; k < 4; ++k) {
            Color lc = (k == 1 || k == 3) ? furS : fur;
            B(lx[k], 8, 4, 6, lc);
            R(lx[k], 14, 4, 2, Color{ 34, 34, 40, 255 }); R(lx[k] + 1, 14, 2, 1, Color{ 90, 92, 104, 255 });
        }
        // the body, lumpy on purpose
        R(-10, -3, 31, 12, kInk); R(-9, -5, 29, 3, kInk); R(-6, -7, 22, 3, kInk);
        R(-9, -3, 29, 11, fur); R(-8, -4, 27, 2, fur); R(-5, -6, 20, 2, fur);
        R(-9, 5 + breathe, 29, 4, furS); R(-9, 8, 29, 1, furD);
        R(-7, -5, 24, 1, Shade(fur, 20)); R(-9, -3, 1, 8, Shade(fur, 12));
        for (int k = 0; k < 9; ++k) R(-7 + k * 3, -2 + (k % 3), 1, 2, furS);                // tufts, a little matted
        R(-6, 6, 22, 3, cream);                                                              // the belly
        // the zip. Teeth all along the belly, a pull at the front. Open a crack if somebody is inside
        for (int k = 0; k < 20; ++k) R(-5 + k, 6 + (k % 2), 1, 1, Color{ 120, 118, 120, 255 });
        if (in) { R(-1, 6, 9, 2, Color{ 20, 16, 22, 255 }); R(1, 6, 2, 1, Color{ 200, 50, 50, 255 }); }   // a red sleeve inside
        R(15, 5, 2, 3, Color{ 190, 196, 206, 255 }); R(15, 8, 2, 1, Color{ 110, 114, 126, 255 });
        // a patch somebody sewed on in a slightly wrong grey, and stuffing coming out of a seam
        R(3, -1, 6, 4, Color{ 196, 194, 198, 255 }); for (int k = 0; k < 3; ++k) { R(3 + k * 2, -1, 1, 1, furD); R(3 + k * 2, 2, 1, 1, furD); }
        R(-8, 1, 2, 2, cream); R(-9, 2, 1, 1, cream);
        // saddle and the handles you hold on to
        B(-1, -8, 10, 4, red); R(0, -7, 8, 1, Color{ 220, 90, 84, 255 }); for (int k = 0; k < 4; ++k) R(1 + k * 2, -5, 1, 1, Color{ 120, 24, 30, 255 });
        R(-5, -6, 4, 2, snow); R(10, -6, 6, 2, snow); R(11, -7, 3, 1, snow);                  // snow on its back
        // the tail, a stub
        B(-14, -2, 4, 4, fur); R(-14, -2, 4, 1, snow);
        // the neck and the head, up front and too big
        B(14, -9, 7, 10, fur);
        B(17, -15, 12, 11, fur); R(17, -15, 12, 2, Shade(fur, 18));
        B(26, -10, 5, 6, cream); R(29, -9, 1, 1, kInk); R(29, -7, 1, 1, kInk);              // the snout and two nostrils
        R(25, -5, 5, 1, Color{ 150, 80, 84, 255 }); R(26, -4, 1, 1, Color{ 150, 80, 84, 255 }); R(28, -4, 1, 1, Color{ 150, 80, 84, 255 });   // a stitched smile
        // two button eyes, sewn on a bit crooked. Black with a shiny rim, four little holes each
        auto button = [&](float bx, float by) {
            R(bx, by + 1, 4, 2, Color{ 70, 66, 80, 255 }); R(bx + 1, by, 2, 4, Color{ 70, 66, 80, 255 });
            R(bx + 1, by + 1, 2, 2, Color{ 14, 12, 18, 255 });
            R(bx + 1, by, 1, 1, Color{ 200, 200, 214, 255 });
        };
        button(18, -13); button(23, -12);
        // three horns. It should have two. Nobody checked
        auto horn3 = [&](float hx, float lean, int len) {
            for (int k = 0; k < len; ++k) R(hx + (int)(lean * k), -16 - k, 2, 1, k == len - 1 ? Shade(horn, 30) : (k % 2 ? horn : Shade(horn, -16)));
        };
        horn3(18, -0.34f, 5); horn3(22, 0.0f, 6); horn3(26, 0.34f, 5);
        R(16, -12, 2, 5, furS); R(16, -12, 2, 1, fur);                                        // a floppy ear
        B(15, -8, 3, 2, gold); B(21, -3, 3, 2, gold);                                        // the handles
        R(18, -16, 9, 1, snow); R(16, -10, 1, 1, snow);
        if (in && std::fmod(globalT, 2.6f) < 0.4f) DrawCircle((int)x + 2, (int)y + 6, 1.5f, Fade(WHITE, 0.4f));   // breath out of the zip
        rlDrawRenderBatchActive();
        rlPopMatrix();
        return true;
    }
    if (id == "oldcar") {
        // an old estate car, side on, snowed in to the door handles. No wheels: it sits on blocks of wood
        Color body = { 128, 84, 64, 255 }, bodyS = { 92, 58, 46, 255 }, rust = { 156, 86, 46, 255 }, glass = { 44, 56, 84, 255 }, chrome = { 176, 180, 190, 255 };
        bool in = hidden && story.hideSpot == "oldcar";
        DrawEllipse((int)x + 8, (int)y + 15, 23, 3.5f, shadowC);
        // the cabin: a long boxy roof with three windows
        R(-9, -15, 32, 10, kInk);
        R(-8, -14, 30, 9, body); R(-8, -14, 30, 1, Shade(body, 24));
        R(-6, -12, 8, 6, glass); R(3, -12, 8, 6, glass); R(12, -12, 8, 6, glass);
        R(-6, -12, 8, 1, Color{ 100, 120, 160, 255 }); R(3, -12, 8, 1, Color{ 100, 120, 160, 255 }); R(12, -12, 8, 1, Color{ 100, 120, 160, 255 });
        R(-5, -11, 2, 1, Fade(WHITE, 0.4f)); R(13, -11, 3, 1, Fade(WHITE, 0.3f));
        R(13, -9, 6, 3, Color{ 30, 36, 56, 255 });                                            // a child seat in the back, just a shape
        if (in) { R(4, -9, 3, 3, Color{ 200, 50, 50, 255 }); R(5, -10, 2, 1, Color{ 236, 206, 176, 255 }); }   // somebody red, ducking
        // the body: bonnet, doors, a bumper
        R(-15, -6, 42, 14, kInk);
        R(-14, -5, 40, 12, body); R(-14, -5, 40, 1, Shade(body, 20)); R(-14, 4, 40, 3, bodyS);
        R(-14, -5, 7, 3, Shade(body, 8));                                                     // the bonnet
        R(2, -4, 1, 8, bodyS); R(11, -4, 1, 8, bodyS);                                        // door seams
        R(-2, -2, 3, 1, chrome); R(7, -2, 3, 1, chrome);                                      // handles
        R(-15, 2, 3, 3, Color{ 230, 222, 180, 255 }); R(24, 1, 2, 3, Color{ 150, 40, 40, 255 }); R(25, 2, 1, 1, Color{ 60, 20, 20, 255 });   // a lamp, a cracked tail lamp
        R(-16, 5, 44, 2, chrome); R(-16, 6, 44, 1, Shade(chrome, -50));                       // bumpers
        R(-4, 1, 5, 3, rust); R(16, -3, 4, 2, rust); R(20, 3, 3, 2, rust); R(-12, -1, 2, 2, rust);
        // where the wheels should be: blocks of wood, then snow
        R(-10, 7, 7, 4, Color{ 120, 88, 58, 255 }); R(14, 7, 7, 4, Color{ 120, 88, 58, 255 });
        R(-10, 7, 7, 1, Color{ 150, 112, 76, 255 }); R(14, 7, 7, 1, Color{ 150, 112, 76, 255 });
        // the drift: up the doors on the windward side, lower at the back
        for (int k = 0; k < 46; ++k) {
            int h = 5 + (int)(3 * std::sin(k * 0.21f + 1.0f)) + (k < 12 ? 3 : 0);
            R(-17 + k, 15 - h, 1, h, snow);
            R(-17 + k, 15 - h, 1, 1, k % 3 ? snow : snowS);
        }
        R(-8, -16, 30, 2, snow); R(-6, -17, 22, 1, snow); R(20, -16, 2, 3, snow);            // a metre on the roof
        return true;
    }
    if (id == "swing") {
        // a swing set for a smaller kid. One seat, one empty chain
        Color frame = { 70, 110, 150, 255 }, frameS = { 46, 76, 110, 255 };
        DrawEllipse((int)x + 8, (int)y + 15, 16, 2.5f, shadowC);
        auto leg = [&](float bx, int f) { for (int k = 0; k < 24; ++k) R(bx + f * (k / 5), -9 + k, 2, 1, k % 6 == 0 ? Shade(frame, 20) : frame); };
        leg(-5, -1); leg(-3, 1); leg(19, -1); leg(21, 1);
        R(-5, -10, 28, 2, kInk); R(-4, -10, 26, 1, frame); R(-4, -9, 26, 1, frameS); R(-4, -11, 26, 1, snow);
        float sw = std::sin(globalT * (story.Storm() ? 2.4f : 0.9f)) * (story.Storm() ? 2.0f : 0.7f);
        R(3, -8, 1, 16, Color{ 150, 150, 160, 255 }); R(8, -8, 1, 16, Color{ 150, 150, 160, 255 });
        R(2 + sw, 8, 8, 2, Color{ 160, 60, 50, 255 }); R(2 + sw, 7, 8, 1, snow);              // the seat, low to the ground
        R(14 + sw * 1.4f, -8, 1, 13, Color{ 150, 150, 160, 255 });                           // the other chain, nothing on it
        R(-6, 12, 4, 3, snow); R(19, 12, 6, 3, snow);
        return true;
    }
    if (id == "spruce") {
        // the big spruce. Its lowest branches lie on the snow: there's a dry hollow under there
        bool in = hidden && story.hideSpot == "spruce";
        Color d = { 22, 46, 48, 255 }, m = { 34, 66, 62, 255 }, l = { 52, 90, 78, 255 };
        DrawEllipse((int)x + 8, (int)y + 15, 18, 4, shadowC);
        for (int layer = 0; layer < 6; ++layer) {
            float w = 34 - layer * 5.0f, top = 6 - layer * 7.0f;
            R(8 - w / 2 - 1, top - 1, w + 2, 9, kInk);
            R(8 - w / 2, top, w, 8, d); R(8 - w / 2 + 2, top, w - 6, 6, m); R(8 - w / 4, top + 1, w / 3, 3, l);
            R(8 - w / 2 + 1, top - 1, w - 4, 2, snow); R(8 - w / 2 + 2, top + 1, w / 3, 1, snowS);
            R(8 + w / 2 - 6, top + 5, 4, 1, snow);
        }
        R(6, -38, 4, 3, snow);
        R(3, 10, 10, 4, Color{ 14, 20, 26, 255 });                                            // the hollow under the branches
        if (in) { R(6, 11, 2, 2, Color{ 190, 50, 50, 255 }); R(9, 11, 1, 1, Color{ 230, 200, 170, 255 }); }
        return true;
    }
    if (id == "grave") {
        // a little wooden cross, half buried. 05 carved into it
        Color wood = { 120, 88, 60, 255 };
        DrawEllipse((int)x + 8, (int)y + 14, 7, 2, shadowC);
        R(1, 10, 14, 5, snow); R(2, 9, 12, 1, snow); R(1, 14, 14, 1, snowS);                  // the mound
        B(7, -6, 3, 17, wood); B(3, -2, 11, 3, wood);
        R(4, -1, 1, 1, kInk); R(5, -1, 1, 1, Shade(wood, -40)); R(10, -1, 1, 1, kInk); R(11, -1, 1, 1, Shade(wood, -40));   // 0 5, scratched in
        R(8, 2, 1, 3, Shade(wood, -40)); R(8, 6, 1, 3, Shade(wood, -40));
        R(6, -7, 5, 1, snow); R(3, -3, 11, 1, snow);
        if (story.Knows("grave_05")) R(4, 9, 2, 1, Color{ 230, 60, 70, 255 });              // a red thread from a sweater, tied round it. Yours?
        return true;
    }
    if (id == "icehole") {
        DrawEllipse((int)x + 8, (int)y + 9, 7, 4, Color{ 230, 240, 252, 255 });              // the chipped rim
        DrawEllipse((int)x + 8, (int)y + 9, 5.5f, 3, Color{ 8, 14, 26, 255 });
        float sh = std::sin(globalT * 1.3f) * 0.5f + 0.5f;
        DrawEllipse((int)x + 8, (int)y + 10, 3, 1, Fade(Color{ 120, 150, 200, 255 }, 0.2f + 0.2f * sh));
        R(16, 2, 2, 9, Color{ 100, 74, 52, 255 }); R(16, 1, 2, 1, snow);                     // the stake
        R(10, 8, 6, 1, Color{ 186, 170, 120, 255 }); R(15, 7, 2, 1, Color{ 186, 170, 120, 255 });   // a rope, going down
        return true;
    }
    if (id == "dish") {
        DrawEllipse((int)x + 8, (int)y + 15, 6, 2, shadowC);
        R(7, -6, 2, 21, Color{ 90, 94, 106, 255 }); R(7, -6, 1, 21, Color{ 130, 134, 146, 255 });
        R(3, 13, 2, 2, Color{ 30, 30, 34, 255 }); R(1, 14, 6, 1, Color{ 30, 30, 34, 255 });  // the cable, straight down into the snow
        DrawEllipse((int)x + 6, (int)y - 9, 9, 7, kInk);
        DrawEllipse((int)x + 6, (int)y - 9, 8, 6, Color{ 210, 214, 222, 255 });
        DrawEllipse((int)x + 5, (int)y - 9, 6, 4, Color{ 180, 186, 198, 255 });
        R(0, -15, 10, 2, snow); R(-1, -13, 4, 1, snow);
        R(9, -10, 6, 1, Color{ 60, 62, 72, 255 }); R(14, -11, 3, 3, Color{ 60, 62, 72, 255 }); // the arm and the little head
        if (story.PowerOut() && std::fmod(globalT, 1.4f) < 0.2f) R(15, -10, 1, 1, Color{ 255, 70, 70, 255 });   // still awake on its own battery
        return true;
    }
    if (id == "lights") {
        // a sheet of plywood on two posts behind the shed. The alphabet in house paint, a bulb over each letter
        float bx = BOARD_X - x, by = BOARD_Y - y;
        DrawEllipse((int)(x + bx + BOARD_W / 2), (int)(y + by + BOARD_H + 15), BOARD_W / 2 + 2, 3, shadowC);
        R(bx + 4, by + BOARD_H - 2, 3, 18, Color{ 84, 60, 42, 255 }); R(bx + BOARD_W - 7, by + BOARD_H - 2, 3, 18, Color{ 84, 60, 42, 255 });
        R(bx - 1, by - 1, BOARD_W + 2, BOARD_H + 2, kInk);
        R(bx, by, BOARD_W, BOARD_H, Color{ 196, 174, 136, 255 });
        for (int k = 0; k < 5; ++k) R(bx, by + 4 + k * 6, BOARD_W, 1, Color{ 182, 160, 122, 255 });   // the grain
        R(bx, by, BOARD_W, 1, Color{ 216, 196, 160, 255 }); R(bx + BOARD_W - 1, by, 1, BOARD_H, Color{ 150, 128, 96, 255 });
        for (int k = 0; k < 26; ++k) {
            Vector2 p = BoardLetter(k);
            for (int r = 0; r < 5; ++r) for (int c = 0; c < 3; ++c)
                if (kGlyph[k][r] & (4 >> c)) R(bx + p.x + c, by + p.y + 2 + r, 1, 1, (BoardLit() == k) ? Mix(BulbCol(k), WHITE, 0.3f) : Color{ 40, 30, 34, 255 });   // the spelled letter lights up too
        }
        // the wire, sagging from bulb to bulb
        for (int k = 0; k < 26; ++k) {
            Vector2 p = BoardLetter(k);
            R(bx + p.x - 1, by + p.y - 1, 5, 1, Color{ 30, 40, 34, 255 });
            Color c = BulbCol(k);
            R(bx + p.x + 1, by + p.y, 1, 2, BoardBulbOn(k) ? c : Mix(c, Color{ 30, 34, 50, 255 }, 0.7f));
        }
        R(bx - 1, by - 3, BOARD_W + 2, 2, snow); R(bx + 6, by - 4, 12, 1, snow);
        return true;
    }
    if (id == "win_bath" || id == "win_parents") return false;   // drawn like your window (Sprites.inc)
    if (id == "win_kitchen" || id == "win_living" || id == "win_living2") {
        // seen from inside: the sill and the bottom of the curtains on the wall below you
        R(1, 15, 14, 2, Color{ 236, 230, 216, 255 }); R(2, 17, 12, 1, Fade(BLACK, 0.2f));
        R(0, 13, 3, 3, Color{ 190, 70, 64, 255 }); R(13, 13, 3, 3, Color{ 190, 70, 64, 255 });
        return true;
    }
    if (id == "smallprints" || id == "vane") return true;         // baked into the snow / drawn on the shed roof
    return false;
}
