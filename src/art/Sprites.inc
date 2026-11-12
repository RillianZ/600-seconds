// 600 Seconds - art/Sprites.inc (v0.11)
// Everything that sits on the floor or walks on it. Still drawn from rectangles, the way the prototype did it,
// but every block now has a lit edge, a shaded edge and an outline, so it reads as one sprite.

static const Color kInk = { 30, 22, 30, 255 };

// the tree's bulbs, shared with the glow pass
static const int kBulbX[11] = { 8, 5, 10, 3, 12, 7, 1, 9, 14, 4, 11 }, kBulbY[11] = { -13, -9, -7, -3, -2, 0, 5, 4, 6, 9, 9 };
static bool BulbOn(int k) {
    return !story.PowerOut() && !(story.Stage() >= 2 && k % 3 == 0) && std::fmod(globalT * 0.7f + k * 0.37f, 2.0f) < 1.5f;
}
static Color BulbColor(int k) {
    if (story.tainted) return { 230, 40, 40, 255 };
    static const Color c[4] = { { 255, 222, 120, 255 }, { 246, 90, 80, 255 }, { 120, 186, 255, 255 }, { 130, 230, 140, 255 } };
    return c[k % 4];
}
static float FireFlicker() { return 0.75f + 0.25f * std::sin(globalT * 7) * std::sin(globalT * 3.1f); }

static bool DrawOutback(const Obj& o, float x, float y);   // v0.15: art/Backyard.inc
static void DrawObj(const Obj& o, Vector2 jit) {
    float x = o.x * TS + jit.x, y = o.y * TS + jit.y;
    const std::string& id = o.id;
    if (DrawOutback(o, x, y)) return;
    auto R = [&](float ox, float oy, float w, float h, Color c) { DrawRectangle((int)(x + ox), (int)(y + oy), (int)w, (int)h, c); };
    // a block: outline, body, light on top and left, shade on the right and bottom
    auto B = [&](float ox, float oy, float w, float h, Color c) {
        R(ox - 1, oy - 1, w + 2, h + 2, Mix(c, kInk, 0.72f));
        R(ox, oy, w, h, c);
        R(ox, oy, w, 1, Shade(c, 24)); R(ox, oy + 1, 1, h - 1, Shade(c, 10));
        R(ox + w - 1, oy + 1, 1, h - 1, Shade(c, -20)); R(ox, oy + h - 1, w, 1, Shade(c, -28));
    };
    if (id == "gap") {   // v0.19: where the back fence is down. A snapped post, and the snow beyond is just white
        R(-10, -14, 3, 9, Color{ 92, 70, 52, 255 }); R(-10, -14, 3, 1, Color{ 250, 252, 255, 255 }); R(-9, -6, 6, 2, Color{ 110, 84, 60, 255 });
        R(34, -14, 3, 9, Color{ 92, 70, 52, 255 }); R(34, -14, 3, 1, Color{ 250, 252, 255, 255 });
        return;
    }
    static const char* flat[] = { "toys", "window", "mirror", "calendar", "frontdoor", "sheddoor", "rope", "stairs", "gate", "basement", "screens", "fliptendo",
                                  "door_kid", "door_bath", "door_parents", "door_kitchen", "door_living", "door_kl", "album", "win_bath", "win_parents" };
    bool isFlat = false;
    for (auto f : flat) if (id == f) isFlat = true;
    if (!isFlat) {
        float sw = (id == "couch" || id == "pbed") ? 40.0f : (id == "bench" || id == "woodpile") ? 26.0f : 17.0f;
        bool out = IsOutsideT(TileAt(o.x, o.y));
        DrawEllipse((int)(x + 8), (int)(y + 15), sw / 2, 2.5f, out ? Fade(Color{ 70, 86, 150, 255 }, 0.35f) : Fade(BLACK, 0.26f));
    }
    if (id == "bed") {
        B(0, -6, 16, 22, Color{ 112, 76, 48, 255 });                           // frame
        R(0, -6, 16, 3, Color{ 134, 92, 58, 255 }); R(0, -4, 16, 1, Color{ 84, 56, 36, 255 });   // headboard
        R(1, -3, 14, 18, Color{ 232, 230, 222, 255 });                         // sheet
        R(3, -2, 10, 4, Color{ 250, 250, 246, 255 }); R(3, 2, 10, 1, Color{ 204, 202, 196, 255 }); R(12, -1, 1, 3, Color{ 222, 222, 218, 255 });
        R(1, 4, 14, 11, Color{ 62, 94, 160, 255 });                            // duvet
        R(1, 4, 14, 2, Color{ 104, 136, 198, 255 }); R(13, 6, 2, 9, Color{ 48, 76, 136, 255 }); R(1, 14, 14, 1, Color{ 44, 68, 122, 255 });
        for (int k = 0; k < 4; ++k) { int sx = 3 + (k * 5) % 9, sy = 7 + (k * 3) % 6; R(sx, sy, 1, 1, Color{ 250, 226, 120, 255 }); if (k % 2) { R(sx - 1, sy, 3, 1, Color{ 236, 206, 96, 255 }); R(sx, sy - 1, 1, 3, Color{ 236, 206, 96, 255 }); } }
        R(9, 8, 3, 2, Color{ 240, 236, 220, 255 });                            // a moon on the duvet
    }
    else if (id == "toys") {
        if (!story.roomClean) {
            R(1, 3, 4, 4, Color{ 214, 60, 56, 255 }); R(1, 3, 4, 1, Color{ 244, 110, 100, 255 }); R(2, 4, 1, 1, WHITE);                // a block
            R(8, 9, 6, 3, Color{ 240, 200, 70, 255 }); R(9, 8, 3, 1, Color{ 240, 200, 70, 255 }); R(9, 12, 1, 1, kInk); R(12, 12, 1, 1, kInk);   // a car
            R(4, 10, 3, 3, Color{ 90, 170, 230, 255 }); R(5, 10, 1, 1, WHITE);
            R(11, 2, 3, 5, Color{ 80, 170, 100, 255 }); R(11, 2, 3, 1, Color{ 130, 210, 140, 255 });
        }
    }
    else if (id == "window" || id == "win_bath" || id == "win_parents") {
        bool night = story.t > 20 * 60;
        Color glass = story.Storm() ? Color{ 196, 206, 226, 255 } : night ? Color{ 40, 52, 100, 255 } : Color{ 84, 106, 168, 255 };
        R(0, -8, 16, 10, Color{ 236, 230, 216, 255 }); R(1, -7, 14, 8, glass);
        R(1, -7, 14, 2, Shade(glass, 14)); R(7, -7, 2, 8, Color{ 236, 230, 216, 255 }); R(1, -3, 14, 1, Color{ 236, 230, 216, 255 });
        R(2, -6, 2, 1, Fade(WHITE, 0.5f)); R(10, -6, 3, 1, Fade(WHITE, 0.35f));
        R(1, 0, 14, 1, Color{ 244, 248, 254, 255 }); R(0, 2, 16, 1, Color{ 180, 172, 156, 255 });        // snow on the outside sill
        R(-1, -8, 3, 9, Color{ 196, 150, 60, 255 }); R(14, -8, 3, 9, Color{ 196, 150, 60, 255 }); R(0, -8, 1, 9, Color{ 226, 184, 90, 255 }); R(15, -8, 1, 9, Color{ 160, 118, 44, 255 });   // curtains
    }
    else if (id == "toilet") {
        B(3, -3, 10, 5, Color{ 232, 236, 240, 255 }); R(10, -2, 2, 1, Color{ 170, 176, 186, 255 });
        B(4, 3, 8, 11, Color{ 240, 243, 246, 255 }); R(5, 5, 6, 7, Color{ 250, 251, 252, 255 }); R(6, 6, 4, 5, Color{ 150, 186, 206, 255 }); R(6, 6, 4, 1, Color{ 110, 150, 176, 255 });
    }
    else if (id == "mirror") {
        R(2, -8, 12, 12, Color{ 150, 112, 60, 255 }); R(3, -7, 10, 10, Color{ 178, 206, 220, 255 }); R(3, -7, 10, 3, Color{ 204, 226, 236, 255 });
        R(4, -6, 1, 4, WHITE); R(6, -6, 1, 2, Fade(WHITE, 0.7f)); R(2, -8, 12, 1, Color{ 190, 150, 86, 255 });
        R(2, 5, 12, 2, Color{ 234, 236, 238, 255 }); R(2, 7, 12, 1, Color{ 170, 176, 186, 255 }); R(4, 4, 2, 1, Color{ 110, 190, 150, 255 }); R(10, 3, 1, 2, Color{ 230, 90, 100, 255 });   // shelf, soap, toothbrush
    }
    else if (id == "desk") {
        R(1, 12, 2, 4, Color{ 80, 52, 32, 255 }); R(13, 12, 2, 4, Color{ 80, 52, 32, 255 });
        B(0, 2, 16, 10, Color{ 128, 86, 52, 255 }); R(1, 8, 14, 1, Color{ 96, 62, 38, 255 }); R(7, 10, 2, 1, Color{ 220, 196, 110, 255 });
        R(3, 3, 6, 4, Color{ 244, 242, 234, 255 }); R(4, 4, 4, 1, Color{ 150, 150, 164, 255 }); R(4, 6, 3, 1, Color{ 150, 150, 164, 255 }); R(8, 4, 1, 3, Color{ 60, 80, 150, 255 });
        R(11, 4, 2, 3, Color{ 60, 60, 66, 255 }); R(10, 0, 4, 3, story.PowerOut() ? Color{ 150, 140, 110, 255 } : Color{ 255, 226, 140, 255 }); R(10, 0, 4, 1, Color{ 96, 150, 110, 255 });   // lamp
    }
    else if (id == "pbed") {
        B(-8, -6, 32, 24, Color{ 104, 70, 44, 255 });
        R(-8, -6, 32, 3, Color{ 128, 88, 54, 255 }); R(-8, -4, 32, 1, Color{ 78, 52, 34, 255 });
        R(-7, -3, 30, 20, Color{ 232, 230, 222, 255 });
        R(-5, -2, 11, 4, Color{ 250, 250, 246, 255 }); R(10, -2, 11, 4, Color{ 250, 250, 246, 255 }); R(-5, 2, 11, 1, Color{ 204, 202, 196, 255 }); R(10, 2, 11, 1, Color{ 204, 202, 196, 255 });
        R(-7, 4, 30, 13, Color{ 150, 58, 58, 255 }); R(-7, 4, 30, 2, Color{ 190, 96, 90, 255 }); R(21, 6, 2, 11, Color{ 120, 44, 46, 255 }); R(-7, 16, 30, 1, Color{ 110, 40, 42, 255 });
        for (int k = 0; k < 5; ++k) { R(-5 + k * 6, 9, 2, 2, Color{ 226, 206, 190, 255 }); R(-2 + k * 6, 13, 2, 1, Color{ 180, 84, 80, 255 }); }      // quilt pattern
        R(8, 6, 1, 11, Color{ 124, 46, 48, 255 });
    }
    else if (id == "drawer") {
        B(1, -2, 14, 17, Color{ 104, 68, 40, 255 }); R(2, 3, 12, 1, Color{ 70, 44, 26, 255 }); R(2, 9, 12, 1, Color{ 70, 44, 26, 255 });
        R(6, 0, 4, 1, Color{ 226, 196, 100, 255 }); R(6, 6, 4, 1, Color{ 226, 196, 100, 255 }); R(6, 12, 4, 1, Color{ 226, 196, 100, 255 });
        R(3, -5, 4, 3, Color{ 200, 180, 150, 255 }); R(4, -4, 2, 1, Color{ 90, 70, 60, 255 });               // a framed photo on top
        if (story.drawerOpen) { R(2, 5, 12, 7, Color{ 36, 24, 16, 255 }); R(1, 12, 14, 2, Color{ 124, 84, 50, 255 }); if (story.held != Item::Tape) { R(4, 7, 7, 3, Color{ 20, 20, 24, 255 }); R(5, 8, 5, 1, Color{ 220, 220, 210, 255 }); } if (story.keyInDrawer) R(12, 8, 2, 2, GOLD); }
    }
    else if (id == "coats") {
        R(7, -8, 2, 22, Color{ 96, 64, 40, 255 }); R(4, 13, 8, 2, Color{ 76, 50, 32, 255 }); R(2, -8, 12, 1, Color{ 120, 84, 52, 255 });
        R(1, -6, 6, 12, Color{ 40, 64, 116, 255 }); R(1, -6, 6, 2, Color{ 70, 98, 150, 255 }); R(5, -4, 1, 10, Color{ 28, 46, 90, 255 });
        R(9, -5, 6, 11, Color{ 132, 44, 50, 255 }); R(9, -5, 6, 2, Color{ 170, 70, 72, 255 }); R(9, -3, 1, 9, Color{ 100, 30, 38, 255 });
        R(3, 12, 4, 3, Color{ 60, 46, 40, 255 }); R(10, 13, 4, 2, Color{ 50, 60, 80, 255 });                 // boots
    }
    else if (id == "phone") {
        B(1, 6, 14, 9, Color{ 122, 82, 50, 255 });                                                           // little table
        B(3, 1, 10, 6, Color{ 198, 60, 54, 255 }); R(4, 0, 8, 2, Color{ 40, 36, 40, 255 }); R(3, -1, 3, 2, Color{ 40, 36, 40, 255 }); R(10, -1, 3, 2, Color{ 40, 36, 40, 255 });
        DrawCircle((int)x + 8, (int)y + 4, 2, Color{ 236, 230, 214, 255 }); R(8, 4, 1, 1, kInk);
        if (story.phoneUnplugged) R(13, 7, 3, 1, Color{ 20, 20, 24, 255 });
    }
    else if (id == "fridge") {
        B(0, -11, 16, 27, Color{ 226, 230, 236, 255 }); R(13, -10, 2, 25, Color{ 190, 196, 206, 255 });
        R(1, -2, 14, 1, Color{ 168, 174, 186, 255 }); R(11, -8, 1, 4, Color{ 130, 136, 150, 255 }); R(11, 1, 1, 8, Color{ 130, 136, 150, 255 });
        R(2, -8, 6, 5, Color{ 252, 238, 150, 255 }); R(3, -7, 4, 1, Color{ 120, 120, 120, 255 }); R(3, -5, 3, 1, Color{ 120, 120, 120, 255 });   // chore list
        R(4, 3, 2, 2, Color{ 220, 60, 60, 255 }); R(7, 5, 2, 2, Color{ 80, 140, 220, 255 }); R(3, 8, 5, 4, Color{ 240, 236, 226, 255 }); R(4, 9, 3, 2, Color{ 110, 170, 120, 255 });   // magnets, a drawing
    }
    else if (id == "stove") {
        B(0, -1, 16, 17, Color{ 226, 226, 220, 255 }); R(0, -1, 16, 8, Color{ 52, 52, 60, 255 }); R(0, -1, 16, 1, Color{ 96, 96, 106, 255 });
        for (int k = 0; k < 2; ++k) { DrawCircle((int)x + 4 + k * 8, (int)y + 3, 2.6f, Color{ 30, 30, 34, 255 }); DrawCircleLines((int)x + 4 + k * 8, (int)y + 3, 2, k == 0 && !story.PowerOut() ? Color{ 236, 96, 50, 255 } : Color{ 86, 86, 94, 255 }); }
        B(3, -5, 5, 5, Color{ 150, 156, 168, 255 }); R(2, -3, 1, 1, Color{ 60, 60, 66, 255 }); R(8, -3, 2, 1, Color{ 60, 60, 66, 255 });        // the pot
        R(2, 9, 12, 5, Color{ 40, 40, 46, 255 }); R(3, 10, 10, 3, story.PowerOut() ? Color{ 30, 26, 24, 255 } : Color{ 170, 110, 50, 255 }); R(2, 8, 12, 1, Color{ 170, 170, 176, 255 });
        for (int k = 0; k < 4; ++k) R(2 + k * 3, 7, 2, 1, Color{ 40, 40, 46, 255 });
    }
    else if (id == "trashcan") { B(3, 2, 10, 13, Color{ 86, 120, 92, 255 }); R(2, 1, 12, 2, Color{ 60, 90, 68, 255 }); R(5, 5, 1, 8, Color{ 64, 96, 72, 255 }); R(9, 5, 1, 8, Color{ 64, 96, 72, 255 }); if (!story.hasBag && !story.trashDone) { R(4, -2, 8, 4, Color{ 26, 26, 30, 255 }); R(5, -3, 3, 1, Color{ 60, 60, 66, 255 }); } }
    else if (id == "bowl") { R(2, 13, 12, 1, Fade(BLACK, 0.25f)); R(3, 9, 10, 4, Color{ 204, 62, 60, 255 }); R(2, 8, 12, 2, Color{ 232, 96, 90, 255 }); R(4, 8, 8, 1, story.fed ? Color{ 120, 40, 40, 255 } : Color{ 128, 86, 46, 255 }); R(5, 11, 6, 1, Color{ 250, 240, 230, 255 }); }
    else if (id == "tv") {
        B(-2, 8, 20, 8, Color{ 112, 76, 48, 255 }); R(-1, 11, 8, 4, Color{ 84, 56, 36, 255 }); R(9, 11, 8, 4, Color{ 84, 56, 36, 255 }); R(2, 12, 2, 1, GOLD); R(12, 12, 2, 1, GOLD);
        B(0, -4, 16, 12, Color{ 34, 34, 40, 255 });
        bool stat = story.Stage() >= 2 && std::fmod(globalT, 4.0f) < 0.6f;
        Color sc = story.PowerOut() ? Color{ 12, 12, 14, 255 } : stat ? (((int)(globalT * 12)) % 2 ? Color{ 170, 170, 170, 255 } : Color{ 60, 60, 60, 255 }) : Color{ 86, 156, 224, 255 };
        R(2, -2, 12, 8, sc);
        if (!story.PowerOut()) {
            if (!stat) { R(2, 3, 12, 3, Color{ 232, 240, 250, 255 }); R(4, 0, 3, 3, Color{ 190, 60, 60, 255 }); R(5, -1, 1, 1, Color{ 240, 230, 220, 255 }); R(9, 1, 2, 2, Color{ 40, 96, 66, 255 }); }   // a Christmas film
            R(3, -1, 4, 1, Fade(WHITE, 0.35f)); R(2, -2 + ((int)(globalT * 9)) % 8, 12, 1, Fade(BLACK, 0.14f));
        } else R(3, -1, 3, 1, Fade(WHITE, 0.08f));
        R(13, 6, 1, 1, story.PowerOut() ? Color{ 60, 20, 20, 255 } : Color{ 240, 60, 60, 255 });
        R(4, -7, 1, 3, Color{ 150, 150, 160, 255 }); R(11, -7, 1, 3, Color{ 150, 150, 160, 255 });           // rabbit ears
    }
    else if (id == "couch") {
        Color c = { 66, 110, 116, 255 };
        B(-12, -6, 40, 8, Shade(c, -14));                                                                    // back
        B(-12, 1, 40, 13, c);
        B(-13, -3, 6, 17, Shade(c, -6)); B(23, -3, 6, 17, Shade(c, -6));                                     // arms
        for (int k = 0; k < 3; ++k) { R(-6 + k * 10, 2, 9, 9, Shade(c, 8)); R(-6 + k * 10, 2, 9, 1, Shade(c, 26)); R(-6 + k * 10, 10, 9, 1, Shade(c, -18)); R(2 + k * 10, 3, 1, 8, Shade(c, -22)); }
        R(-6, -1, 8, 5, Color{ 220, 198, 150, 255 }); R(-6, -1, 8, 1, Color{ 240, 222, 180, 255 }); R(-4, 1, 4, 1, Color{ 190, 80, 70, 255 });   // pillow
        R(12, -5, 10, 9, Color{ 168, 50, 52, 255 }); for (int k = 0; k < 3; ++k) R(12, -4 + k * 3, 10, 1, Color{ 232, 222, 200, 255 });          // a wool blanket over the back
    }
    else if (id == "radio") {
        B(0, 8, 16, 7, Color{ 112, 76, 48, 255 });                                                           // sideboard
        B(2, 1, 12, 8, Color{ 146, 86, 52, 255 }); R(3, 3, 5, 5, Color{ 56, 44, 40, 255 }); for (int k = 0; k < 3; ++k) R(3, 3 + k * 2, 5, 1, Color{ 96, 76, 60, 255 });
        R(10, 3, 3, 2, story.PowerOut() ? Color{ 60, 56, 40, 255 } : Color{ 250, 226, 140, 255 }); R(10, 6, 1, 1, kInk); R(12, 6, 1, 1, kInk); R(13, -4, 1, 5, Color{ 170, 170, 180, 255 });
        // the nisse on top. From stage 2 he has turned to face the wall
        R(5, -3, 3, 4, Color{ 196, 44, 44, 255 }); R(6, -6, 1, 3, Color{ 196, 44, 44, 255 }); R(6, -7, 1, 1, WHITE);
        if (story.Stage() < 2) { R(5, -2, 3, 1, Color{ 240, 210, 180, 255 }); R(5, -1, 3, 2, WHITE); }
    }
    else if (id == "tree") {
        // the tree. Lights die as the stages go on, red once you're dark, off in the power cut
        R(6, 11, 4, 5, Color{ 92, 60, 36, 255 }); R(4, 14, 8, 2, Color{ 150, 46, 46, 255 }); R(4, 14, 8, 1, Color{ 196, 80, 76, 255 });
        static const Color g[4] = { { 22, 62, 50, 255 }, { 32, 84, 60, 255 }, { 44, 106, 70, 255 }, { 66, 132, 84, 255 } };
        for (int k = 3; k >= 0; --k) {
            int top = -17 + k * 6, rows = 9;
            for (int r = 0; r < rows; ++r) {
                int w = 2 + k * 3 + r * (3 + k) / 3, x0 = 8 - w / 2, yy = top + r;
                R((float)x0 - 1, (float)yy, (float)w + 2, 1, kInk);
                R((float)x0, (float)yy, (float)w, 1, g[1]);
                R((float)x0, (float)yy, (float)std::max(1, w / 3), 1, g[(r + k) % 3 == 0 ? 3 : 2]);
                R((float)(x0 + w - std::max(1, w / 4)), (float)yy, (float)std::max(1, w / 4), 1, g[0]);
                if (r == rows - 1) for (int i = 0; i < w; i += 2) R((float)(x0 + i), (float)yy + 1, 1, 1, g[0]);     // needle tips
            }
        }
        for (int k = 0; k < 9; ++k) R(1.0f + k * 1.6f, -6.0f + k * 0.8f + (k % 2), 1, 1, Color{ 226, 190, 96, 255 });   // garland
        for (int k = 0; k < 8; ++k) R(14.0f - k * 1.5f, 3.0f + k * 0.7f, 1, 1, Color{ 226, 190, 96, 255 });
        for (int k = 0; k < 11; ++k) {
            bool on = BulbOn(k);
            Color c = BulbColor(k);
            R((float)kBulbX[k], (float)kBulbY[k], 2, 2, on ? c : Color{ 26, 50, 40, 255 });
            if (on) R((float)kBulbX[k], (float)kBulbY[k], 1, 1, Mix(c, WHITE, 0.6f));
        }
        Color star = story.PowerOut() ? Color{ 130, 118, 70, 255 } : Color{ 255, 226, 110, 255 };
        R(7, -22, 2, 6, star); R(5, -20, 6, 2, star); R(6, -21, 4, 4, star); if (!story.PowerOut()) R(7, -20, 2, 2, WHITE);
        // presents
        B(-4, 9, 7, 7, Color{ 186, 52, 54, 255 }); R(-1, 9, 1, 7, Color{ 244, 226, 150, 255 }); R(-4, 12, 7, 1, Color{ 244, 226, 150, 255 }); R(-2, 7, 3, 2, Color{ 244, 226, 150, 255 });
        if (!story.zuldaOpened) { B(11, 10, 8, 6, Color{ 60, 92, 176, 255 }); R(14, 10, 1, 6, Color{ 240, 240, 244, 255 }); R(13, 8, 3, 2, Color{ 240, 240, 244, 255 }); }
        else { R(11, 13, 8, 3, Color{ 60, 92, 176, 255 }); R(10, 12, 4, 2, Color{ 96, 126, 200, 255 }); R(16, 14, 4, 2, Color{ 240, 240, 244, 255 }); }      // torn paper
        B(4, 12, 5, 4, Color{ 70, 150, 96, 255 }); R(6, 12, 1, 4, Color{ 232, 80, 80, 255 });
    }
    else if (id == "woodstove") {
        R(6, -15, 4, 16, Color{ 34, 34, 40, 255 }); R(6, -15, 1, 16, Color{ 78, 78, 88, 255 }); R(5, -9, 6, 1, Color{ 60, 60, 68, 255 });   // pipe
        B(1, 0, 14, 15, Color{ 44, 44, 50, 255 }); R(2, 1, 12, 2, Color{ 70, 70, 80, 255 }); R(0, 15, 16, 1, Color{ 28, 28, 32, 255 }); R(1, 15, 2, 1, kInk); R(13, 15, 2, 1, kInk);
        float fl = std::sin(globalT * 7) * std::sin(globalT * 3.1f);
        R(3, 4, 10, 9, Color{ 20, 14, 12, 255 }); R(4, 5, 8, 7, Color{ 120, 36, 16, 255 });
        R(5, 7 - (fl > 0.3f ? 1 : 0), 6, 5 + (fl > 0.3f ? 1 : 0), Color{ 255, (unsigned char)(126 + 40 * fl), 40, 255 });
        R(6 + (((int)(globalT * 9)) % 2), 9, 3, 3, Color{ 255, 226, 130, 255 }); R(7, 10, 1, 2, Color{ 255, 250, 220, 255 });
        R(8, 5, 1, 7, Color{ 20, 14, 12, 255 }); R(3, 8, 10, 1, Fade(Color{ 20, 14, 12, 255 }, 0.6f));                                     // the grate
        R(-5, 9, 5, 6, Color{ 122, 84, 52, 255 }); R(-5, 9, 5, 1, Color{ 214, 186, 140, 255 }); R(-4, 6, 4, 3, Color{ 140, 98, 60, 255 }); DrawCircle((int)x - 2, (int)y + 8, 1, Color{ 214, 186, 140, 255 });   // logs waiting
    }
    else if (id == "bins") {
        B(0, 2, 7, 12, Color{ 44, 80, 50, 255 }); B(9, 2, 7, 12, Color{ 44, 50, 100, 255 });
        R(-1, 0, 9, 3, Color{ 244, 248, 254, 255 }); R(8, 0, 9, 3, Color{ 244, 248, 254, 255 }); R(-1, 2, 9, 1, Color{ 196, 208, 236, 255 }); R(8, 2, 9, 1, Color{ 196, 208, 236, 255 });
        R(2, 6, 3, 3, Color{ 70, 110, 76, 255 }); R(11, 6, 3, 3, Color{ 70, 78, 136, 255 });
    }
    else if (id == "doghouse") {
        B(0, 4, 16, 11, Color{ 152, 58, 46, 255 }); for (int k = 0; k < 3; ++k) R(1, 7 + k * 3, 14, 1, Color{ 116, 40, 36, 255 });
        R(5, 7, 6, 8, Color{ 22, 16, 16, 255 }); R(4, 6, 8, 1, Color{ 232, 226, 212, 255 }); R(4, 7, 1, 8, Color{ 232, 226, 212, 255 }); R(11, 7, 1, 8, Color{ 232, 226, 212, 255 });
        R(-2, 0, 20, 5, Color{ 60, 56, 62, 255 }); R(-2, -3, 20, 4, Color{ 244, 248, 254, 255 }); R(-1, -4, 16, 1, Color{ 244, 248, 254, 255 }); R(-2, 1, 20, 1, Color{ 190, 204, 234, 255 });
        R(4, 2, 8, 3, Color{ 232, 222, 200, 255 }); R(5, 3, 6, 1, Color{ 110, 80, 60, 255 });               // name board
        R(-3, 13, 22, 3, Color{ 244, 248, 254, 255 });
    }
    else if (id == "gate") { B(0, 2, 4, 12, Color{ 96, 68, 44, 255 }); R(-1, 0, 6, 3, Color{ 244, 248, 254, 255 }); R(4, 5, 10, 2, Color{ 104, 74, 50, 255 }); R(4, 10, 10, 2, Color{ 104, 74, 50, 255 }); R(4, 4, 10, 1, Color{ 244, 248, 254, 255 }); }
    else if (id == "flag") {
        R(5, 12, 6, 3, Color{ 130, 132, 144, 255 }); R(5, 11, 6, 1, Color{ 244, 248, 254, 255 });
        R(7, -30, 2, 43, Color{ 226, 228, 236, 255 }); R(8, -30, 1, 43, Color{ 170, 176, 196, 255 }); R(6, -32, 4, 2, Color{ 226, 196, 100, 255 });
        float sp = story.Storm() ? 16.0f : 5.0f;
        Color red = { 196, 36, 46, 255 }, navy = { 0, 36, 100, 255 };
        for (int i = 0; i < 16; ++i) {                                                                       // the flag, one column at a time so it can wave
            float wv = std::sin(globalT * sp - i * 0.55f) * (i / 16.0f) * (story.Storm() ? 2.4f : 1.4f);
            float fx = 9.0f + i, fy = -29 + wv;
            Color cc = (i >= 4 && i <= 6) ? ((i == 5) ? navy : RAYWHITE) : red;
            R(fx, fy, 1, 11, cc);
            if (!(i >= 4 && i <= 6)) { R(fx, fy + 4, 1, 3, RAYWHITE); R(fx, fy + 5, 1, 1, navy); } else if (i != 5) R(fx, fy + 5, 1, 1, navy);
            if (std::cos(globalT * sp - i * 0.55f) < -0.4f) R(fx, fy, 1, 11, Fade(BLACK, 0.14f));
        }
    }
    else if (id == "generator") {
        B(1, 2, 14, 12, Color{ 210, 166, 44, 255 }); R(3, 4, 10, 5, Color{ 36, 36, 40, 255 }); for (int k = 0; k < 4; ++k) R(4 + k * 2, 5, 1, 3, Color{ 96, 96, 100, 255 });
        R(3, 11, 2, 2, story.PowerOut() ? Color{ 60, 30, 30, 255 } : Color{ 90, 230, 110, 255 }); R(10, 11, 3, 1, kInk); R(2, 14, 3, 2, kInk); R(11, 14, 3, 2, kInk); R(6, 0, 4, 2, Color{ 60, 60, 66, 255 });
    }
    else if (id == "pantry" || id == "wardrobe") {
        bool p = id == "pantry";
        Color c = p ? Color{ 224, 216, 190, 255 } : Color{ 130, 88, 56, 255 }, knob = p ? Color{ 120, 110, 90, 255 } : Color{ 226, 196, 100, 255 };
        B(0, -13, 16, 29, c); R(0, -13, 16, 2, Shade(c, 24)); R(1, 14, 14, 1, Shade(c, -44));
        R(7, -10, 2, 24, Shade(c, -40)); R(8, -10, 1, 24, Shade(c, -14));
        for (int k = 0; k < 2; ++k) { R(2 + k * 8, -9, 4, 9, Shade(c, 10)); R(2 + k * 8, 2, 4, 10, Shade(c, 10)); R(2 + k * 8, -9, 4, 1, Shade(c, -22)); R(2 + k * 8, 2, 4, 1, Shade(c, -22)); }
        R(5, 0, 1, 3, knob); R(10, 0, 1, 3, knob);
        if (!p) { R(2, -16, 6, 3, Color{ 150, 110, 70, 255 }); R(9, -15, 5, 2, Color{ 70, 100, 150, 255 }); }   // boxes on top
    }
    else if (id == "sink") {
        B(0, 0, 16, 16, Color{ 216, 206, 178, 255 }); R(0, 0, 16, 9, Color{ 196, 200, 206, 255 }); R(0, 0, 16, 1, Color{ 232, 236, 240, 255 });
        R(2, 2, 12, 6, Color{ 120, 138, 152, 255 }); R(3, 3, 10, 4, Color{ 150, 172, 186, 255 }); R(3, 3, 10, 1, Color{ 96, 112, 128, 255 });
        R(7, -3, 2, 5, Color{ 206, 210, 216, 255 }); R(6, -4, 4, 1, Color{ 226, 230, 236, 255 }); R(8, -3, 1, 4, Color{ 150, 156, 166, 255 });
        if (!story.dishesDone) { R(4, 3, 5, 3, Color{ 244, 244, 240, 255 }); R(5, 2, 3, 1, Color{ 244, 244, 240, 255 }); R(9, 4, 3, 2, Color{ 214, 172, 120, 255 }); R(11, 1, 1, 4, Color{ 190, 196, 206, 255 }); }
        else R(4, 5, 3, 1, Fade(WHITE, 0.5f));
        R(2, 10, 5, 5, Color{ 200, 188, 156, 255 }); R(9, 10, 5, 5, Color{ 200, 188, 156, 255 }); R(4, 12, 1, 1, Color{ 110, 100, 84, 255 }); R(11, 12, 1, 1, Color{ 110, 100, 84, 255 });
    }
    else if (id == "fliptendo") {
        // a beanbag with the handheld on it
        DrawEllipse((int)x + 8, (int)y + 11, 9, 6, kInk);
        DrawEllipse((int)x + 8, (int)y + 10, 8, 5, Color{ 156, 56, 68, 255 });
        DrawEllipse((int)x + 7, (int)y + 9, 6, 3, Color{ 192, 84, 94, 255 }); R(4, 7, 3, 1, Color{ 220, 120, 126, 255 });
        R(3, 5, 10, 6, kInk); R(4, 6, 8, 4, Color{ 220, 60, 60, 255 }); R(6, 6, 4, 4, Color{ 30, 30, 36, 255 });
        R(6, 7, 4, 2, story.zuldaOpened ? Color{ 96, 200, 116, 255 } : Color{ 124, 176, 236, 255 });
        if (std::fmod(globalT, 1.4f) < 0.7f) R(7, 7, 1, 1, WHITE);
    }
    else if (id == "marbles") { R(4, 5, 8, 10, Fade(Color{ 196, 228, 250, 255 }, 0.75f)); R(4, 5, 1, 10, Fade(WHITE, 0.6f)); R(5, 10, 2, 2, Color{ 230, 60, 60, 255 }); R(8, 11, 2, 2, Color{ 60, 100, 230, 255 }); R(6, 12, 2, 2, Color{ 70, 190, 100, 255 }); R(9, 9, 2, 2, Color{ 240, 200, 70, 255 }); R(3, 3, 10, 2, Color{ 150, 150, 164, 255 }); R(3, 15, 10, 1, Fade(BLACK, 0.3f)); }
    else if (id == "calendar") {
        // the advent calendar, every door open except the last one
        R(2, -9, 12, 13, kInk); R(3, -8, 10, 11, Color{ 196, 52, 50, 255 }); R(3, -8, 10, 2, Color{ 40, 96, 66, 255 }); R(7, -10, 2, 2, Color{ 226, 196, 100, 255 });
        for (int k = 0; k < 12; ++k) R(4 + (k % 4) * 2, -5 + (k / 4) * 2, 1, 1, Color{ 250, 240, 220, 255 });
        R(10, 0, 2, 2, Color{ 20, 20, 24, 255 }); R(10, 0, 2, 1, Color{ 226, 196, 100, 255 });
    }
    else if (id == "knives") {
        B(0, 6, 16, 9, Color{ 216, 206, 178, 255 }); R(0, 6, 16, 2, Color{ 150, 110, 70, 255 });                       // counter
        B(5, 0, 7, 8, Color{ 112, 72, 42, 255 }); if (!story.hasKnife) { R(6, -5, 2, 5, Color{ 30, 30, 34, 255 }); R(6, -5, 1, 2, Color{ 80, 80, 90, 255 }); } R(9, -3, 1, 3, Color{ 30, 30, 34, 255 }); R(11, -2, 1, 2, Color{ 30, 30, 34, 255 });
        R(1, 3, 3, 3, Color{ 236, 120, 50, 255 }); R(2, 2, 1, 1, Color{ 60, 130, 70, 255 });                           // a clementine
    }
    else if (id == "bookshelf") {
        B(0, -11, 16, 27, Color{ 96, 62, 36, 255 }); R(1, -10, 14, 25, Color{ 50, 32, 22, 255 });
        static const Color bk[6] = { { 160, 54, 54, 255 }, { 54, 86, 150, 255 }, { 64, 128, 86, 255 }, { 206, 174, 92, 255 }, { 124, 92, 146, 255 }, { 214, 204, 184, 255 } };
        for (int k = 0; k < 3; ++k) {
            float bx = 1;
            for (int b = 0; b < 8 && bx < 14; ++b) {
                unsigned q = Hash(k * 7 + b, 3);
                int w = 1 + (int)(q % 2), hgt = 4 + (int)((q >> 3) % 3);
                if (bx + w > 15) break;
                R(bx, -2 + k * 8 - hgt, (float)w, (float)hgt, bk[q % 6]); R(bx, -2 + k * 8 - hgt, 1, 1, Mix(bk[q % 6], WHITE, 0.4f));
                if (w == 2) R(bx, -4 + k * 8, 2, 1, Mix(bk[q % 6], BLACK, 0.3f));
                bx += w + ((q >> 6) % 5 == 0 ? 1 : 0);
            }
            R(1, -2 + k * 8, 14, 1, Color{ 124, 84, 50, 255 });
        }
        R(10, 11, 3, 3, Color{ 196, 44, 44, 255 }); R(11, 10, 1, 1, WHITE);
    }
    else if (id == "album") { B(3, 5, 10, 8, Color{ 124, 40, 52, 255 }); R(4, 6, 8, 1, GOLD); R(5, 8, 5, 3, Color{ 220, 204, 170, 255 }); R(6, 9, 1, 1, Color{ 90, 70, 60, 255 }); R(8, 9, 1, 1, Color{ 90, 70, 60, 255 }); }
    else if (id == "frontdoor") {
        if (story.frontLocked && !story.collection) { R(6, 2, 4, 4, Color{ 226, 200, 84, 255 }); R(7, 3, 2, 2, kInk); }
        if (story.trapsSet.count("marbles")) for (int k = 0; k < 7; ++k) { R((float)((k * 5) % 14), 18 + (float)((k * 3) % 6), 2, 2, k % 2 ? Color{ 230, 60, 60, 255 } : Color{ 110, 180, 240, 255 }); R((float)((k * 5) % 14), 18 + (float)((k * 3) % 6), 1, 1, WHITE); }
        if (story.trapsSet.count("tripwire")) { R(-2, 22, 20, 1, Color{ 236, 236, 216, 255 }); R(-2, 23, 20, 1, Fade(Color{ 70, 86, 150, 255 }, 0.3f)); }
        if (story.trapsSet.count("paint")) { R(3, -14, 10, 9, kInk); R(4, -13, 8, 7, Color{ 224, 60, 60, 255 }); R(4, -13, 8, 2, Color{ 244, 110, 100, 255 }); R(7, -18, 1, 4, GRAY); }
    }
    else if (id == "sheddoor") { if (story.dadLocked) { R(-2, 6, 20, 3, Color{ 166, 124, 72, 255 }); R(-2, 6, 20, 1, Color{ 200, 160, 100, 255 }); R(-2, 9, 20, 1, kInk); } }
    else if (id == "rope") { DrawCircleLines((int)x + 8, (int)y + 9, 6, kInk); DrawCircleLines((int)x + 8, (int)y + 8, 5, Color{ 206, 184, 124, 255 }); DrawCircleLines((int)x + 8, (int)y + 8, 3, Color{ 176, 150, 96, 255 }); R(12, 10, 3, 1, Color{ 206, 184, 124, 255 }); }
    else if (id == "toolbox") { B(1, 5, 14, 9, Color{ 190, 44, 44, 255 }); R(1, 8, 14, 1, Color{ 130, 26, 30, 255 }); R(7, 8, 2, 2, Color{ 226, 196, 100, 255 }); R(5, 2, 6, 3, Color{ 60, 60, 66, 255 }); R(6, 3, 4, 2, Color{ 190, 44, 44, 255 }); R(11, -3, 5, 8, Color{ 236, 236, 236, 255 }); R(11, -3, 5, 2, Color{ 60, 120, 200, 255 }); }
    else if (id == "woodpile") {
        for (int r = 0; r < 2; ++r) for (int k = 0; k < 4 - r; ++k) {
            float lx = -5 + k * 6.0f + r * 3, ly = 9 - r * 6.0f;
            R(lx - 1, ly - 1, 8, 8, kInk); R(lx, ly, 6, 6, Color{ 136, 94, 58, 255 }); R(lx + 1, ly + 1, 4, 4, Color{ 206, 176, 128, 255 }); R(lx + 2, ly + 2, 2, 2, Color{ 170, 132, 86, 255 });
        }
        R(-6, 2, 14, 2, Color{ 244, 248, 254, 255 }); R(-3, -4, 16, 3, Color{ 244, 248, 254, 255 }); R(-3, -2, 16, 1, Color{ 190, 204, 234, 255 });
        R(16, 2, 2, 12, Color{ 120, 84, 52, 255 }); R(14, 0, 6, 3, Color{ 150, 154, 168, 255 }); R(14, 0, 6, 1, Color{ 210, 214, 226, 255 });   // the axe
    }
    else if (id == "stairs") { for (int k = 0; k < 4; ++k) { R(0, k * 4, 16, 4, Color{ (unsigned char)(60 + k * 14), (unsigned char)(62 + k * 14), (unsigned char)(74 + k * 14), 255 }); R(0, k * 4 + 3, 16, 1, Color{ 30, 32, 42, 255 }); } R(0, 0, 1, 16, Color{ 120, 124, 140, 255 }); R(15, 0, 1, 16, Color{ 120, 124, 140, 255 }); }
    else if (id == "screens") {
        R(-10, -15, 36, 21, Color{ 30, 32, 40, 255 }); R(-10, -15, 36, 1, Color{ 70, 74, 88, 255 });
        for (int k = 0; k < 6; ++k) {
            float fx = (float)((k % 3) * 11 - 8), fy = (float)((k / 3) * 9 - 13);
            R(fx, fy, 10, 8, Color{ 12, 12, 16, 255 });
            if (story.collection && k != 5) continue;   // v0.13: the power is out. One small screen runs on a battery
            Color glow = Mix(Color{ 50, 130, 250, 255 }, Color{ 150, 204, 255, 255 }, 0.5f + 0.5f * std::sin(globalT * 3 + k));
            R(fx + 1, fy + 1, 8, 6, Mix(glow, BLACK, 0.45f));
            if (k == 1 || k == 3) { R(fx + 2, fy + 2, 6, 4, Mix(glow, BLACK, 0.2f)); R(fx + 4 + ((int)(globalT * 2 + k) % 3), fy + 3, 1, 2, WHITE); }      // a room, and someone in it
            else for (int j = 0; j < 3; ++j) R(fx + 2, fy + 2 + j * 2, (float)(2 + (Hash(k, j + (int)(globalT * 1.5f)) % 5)), 1, glow);
            R(fx + 1, fy + 1 + ((int)(globalT * 14 + k * 3)) % 6, 8, 1, Fade(WHITE, 0.12f));
        }
        R(-8, 7, 32, 3, Color{ 44, 46, 56, 255 }); for (int k = 0; k < 9; ++k) R(-6.0f + k * 3.4f, 8, 2, 1, k % 4 == 0 ? Color{ 240, 80, 80, 255 } : Color{ 110, 114, 130, 255 });
    }
    else if (id == "files") { B(1, -7, 14, 23, Color{ 116, 122, 132, 255 }); for (int k = 0; k < 3; ++k) { R(2, -5 + k * 7, 12, 1, Color{ 70, 74, 84, 255 }); R(5, -2 + k * 7, 6, 2, Color{ 56, 58, 66, 255 }); R(6, -4 + k * 7, 4, 1, Color{ 232, 228, 210, 255 }); } }
    else if (id == "console") {
        B(-2, 2, 20, 12, Color{ 52, 58, 66, 255 }); R(-1, 3, 18, 5, Color{ 30, 34, 40, 255 });
        R(10, 4, 4, 3, std::fmod(globalT, 1.2f) < 0.6f ? Color{ 250, 60, 60, 255 } : Color{ 110, 30, 30, 255 });
        for (int k = 0; k < 3; ++k) R(1 + k * 3, 4, 2, 2, Hash(k, (int)(globalT * 3)) % 3 ? Color{ 90, 230, 110, 255 } : Color{ 30, 80, 40, 255 });
        for (int k = 0; k < 5; ++k) R(0 + k * 3.4f, 10, 2, 2, Color{ 110, 114, 130, 255 });
        R(15, -6, 1, 9, Color{ 170, 174, 190, 255 });
    }
    else if (id == "chair") {
        B(2, -7, 12, 21, Color{ 84, 40, 42, 255 }); R(3, -6, 10, 7, Color{ 110, 56, 56, 255 }); R(3, 3, 10, 9, Color{ 100, 50, 50, 255 });
        R(0, 1, 16, 2, Color{ 70, 58, 46, 255 }); R(0, 8, 16, 2, Color{ 70, 58, 46, 255 }); R(3, -4, 10, 2, Color{ 70, 58, 46, 255 });
        R(7, 1, 2, 2, Color{ 190, 190, 200, 255 }); R(7, 8, 2, 2, Color{ 190, 190, 200, 255 }); R(1, 13, 2, 3, kInk); R(13, 13, 2, 3, kInk);
    }
    else if (id == "vcr") {
        B(1, 8, 14, 7, Color{ 72, 72, 80, 255 }); R(2, 15, 2, 1, kInk); R(12, 15, 2, 1, kInk);                              // cart
        R(2, 9, 12, 3, Color{ 28, 28, 32, 255 }); R(4, 10, 6, 1, Color{ 8, 8, 8, 255 }); R(11, 10, 1, 1, RED);              // VCR
        B(0, -7, 16, 15, Color{ 52, 48, 46, 255 }); R(2, -5, 10, 10, Color{ 22, 28, 36, 255 }); R(13, -4, 1, 1, Color{ 150, 150, 150, 255 }); R(13, -1, 1, 1, Color{ 150, 150, 150, 255 });
        if (std::fmod(globalT, 2.0f) < 1.0f) R(3, -4, 8, 1, Fade(WHITE, 0.1f));
        R(3, -4, 2, 1, Fade(WHITE, 0.2f));
    }
    else if (id == "crates") {
        // v0.13: two program crates, the big one with its lid ajar
        B(0, -2, 16, 17, Color{ 120, 128, 96, 255 }); R(1, 3, 14, 1, Color{ 90, 96, 70, 255 }); R(1, 9, 14, 1, Color{ 90, 96, 70, 255 });
        R(3, 5, 10, 3, Color{ 220, 220, 200, 255 }); R(4, 6, 1, 1, Color{ 60, 60, 60, 255 }); R(6, 6, 3, 1, Color{ 60, 60, 60, 255 }); R(10, 6, 2, 1, Color{ 60, 60, 60, 255 });
        B(-1, -6, 18, 4, Color{ 134, 142, 108, 255 });
        B(11, -12, 7, 7, Color{ 110, 118, 88, 255 });
    }
    else if (id == "medfridge") { B(1, -7, 14, 23, Color{ 228, 234, 240, 255 }); R(3, -4, 10, 9, Color{ 130, 196, 226, 255 }); R(3, -4, 10, 1, Color{ 200, 236, 250, 255 }); for (int k = 0; k < 4; ++k) { R(4 + k * 2, 0, 1, 4, Color{ 250, 150, 50, 255 }); R(4 + k * 2, -1, 1, 1, WHITE); } R(3, 8, 10, 1, Color{ 170, 178, 190, 255 }); R(6, 10, 4, 3, Color{ 220, 50, 50, 255 }); R(7, 9, 2, 5, Color{ 220, 50, 50, 255 }); }
    else if (id == "bench") {
        R(-3, 13, 2, 3, Color{ 70, 50, 36, 255 }); R(13, 13, 2, 3, Color{ 70, 50, 36, 255 });
        B(-4, 2, 20, 11, Color{ 124, 92, 62, 255 }); R(-4, 9, 20, 1, Color{ 90, 64, 44, 255 });
        R(-2, 3, 6, 2, Color{ 170, 174, 186, 255 }); R(-3, 4, 2, 3, Color{ 150, 60, 50, 255 }); R(6, 4, 7, 4, Color{ 46, 46, 52, 255 }); R(7, 5, 5, 1, Color{ 96, 96, 106, 255 }); R(13, 3, 1, 5, Color{ 226, 196, 100, 255 });
        for (int k = 0; k < 4; ++k) R(-3 + k * 5, -8, 1, 6 - k % 2, Color{ 150, 154, 168, 255 });                          // tools on the wall
    }
}

// ---------------------------------------------------------------- people
struct Look { Color shirt, pants, hair, skin; float scale; bool longHair; bool beard; Color eyes = BLACK; int style = 0; bool hat = false; };   // style: 0 the kid, 1 Mom, 2 Dad, 3 the visitor
static void DrawPerson(const Actor& a, Look l) {
    if (!a.visible) return;
    float s = l.scale;
    bool moving = a.anim > 0;
    int frame = moving ? ((int)a.anim) % 4 : 0;
    float breathe = moving ? 0 : (std::fmod(globalT + a.pos.x * 0.01f, 1.6f) < 0.8f ? 0 : 1);
    float bob = (frame == 1 || frame == 3) ? -1 : 0;
    float x = a.pos.x, y = a.pos.y + 4 + bob;
    int pass = 0;   // pass 0 draws the outline, pass 1 the person
    auto R = [&](float ox, float oy, float w, float h, Color c) {
        int rx = (int)std::round(x + ox * s), ry = (int)std::round(y + oy * s), rw = (int)std::ceil(w * s), rh = (int)std::ceil(h * s);
        if (pass == 0) DrawRectangle(rx - 1, ry - 1, rw + 2, rh + 2, kInk);
        else DrawRectangle(rx, ry, rw, rh, c);
    };
    auto P = [&](float ox, float oy, float w, float h, Color c) { if (pass == 1) R(ox, oy, w, h, c); };   // detail, never part of the outline
    bool out = IsOutsideT(TileAt(TX(a.pos.x), TX(a.pos.y + 4)));
    DrawEllipse((int)x, (int)(a.pos.y + 4 + 3 * s), 6 * s, 2 * s, out ? Fade(Color{ 60, 76, 140, 255 }, 0.4f) : Fade(BLACK, 0.3f));
    Color pantsD = Mix(l.pants, kInk, 0.3f), shirtD = Mix(l.shirt, kInk, 0.28f), shirtL = Shade(l.shirt, 26), skinD = Mix(l.skin, Color{ 150, 80, 80, 255 }, 0.3f);
    Color shoe = l.style == 3 ? Color{ 14, 14, 16, 255 } : Color{ 54, 38, 30, 255 };
    bool side = a.dir == 2 || a.dir == 3;
    float flip = a.dir == 2 ? -1 : 1;
    float hy = -breathe * 0.5f;
    bool coat = l.style == 3, dress = l.style == 1;
    for (pass = 0; pass < 2; ++pass) {
        // legs
        if (side) {
            float swing = frame == 0 ? 2 : frame == 2 ? -2 : 0;
            R(-2 + swing * flip, -2, 3, 5, pantsD);
            R(-1 - swing * flip, -2, 3, 5, l.pants);
            R(-2 + swing * flip + (flip > 0 ? 1 : -1), 2, 3, 1, shoe); R(-1 - swing * flip + (flip > 0 ? 1 : -1), 2, 3, 1, shoe);
        } else {
            float lift0 = frame == 0 ? 1.5f : 0, lift2 = frame == 2 ? 1.5f : 0;
            R(-4, -2 - lift0, 3, 5, l.pants); R(1, -2 - lift2, 3, 5, l.pants);
            P(-2, -2 - lift0, 1, 4, pantsD); P(3, -2 - lift2, 1, 4, pantsD);
            R(-4, 2 - lift0, 3, 1, shoe); R(1, 2 - lift2, 3, 1, shoe);
        }
        // body
        R(-5, -10 + hy, 10, 9, l.shirt);
        if (coat || dress) { R(-5, -2, 10, coat ? 4 : 3, l.shirt); P(-5, coat ? 1 : 0, 10, 1, shirtD); if (coat && moving) P((frame % 2) ? -5 : 3, 1, 2, 1, l.shirt); }
        else P(-5, -2, 10, 1, shirtD);
        P(-5, -10 + hy, 10, 1, shirtL);
        if (!side) P(a.dir == 1 ? -5 : 3, -9 + hy, 2, coat || dress ? 10 : 7, shirtD);
        // what they wear
        if (l.style == 0 && !l.hat) { if (a.dir != 1) for (int k = 0; k < 4; ++k) P(-4 + k * 2.4f, -7, 1, 1, Color{ 250, 240, 230, 255 }); P(-5, -5, 10, 1, shirtD); }       // his Christmas jumper
        if (l.style == 0 && l.hat) { P(-4, -10 + hy, 8, 2, Color{ 226, 214, 190, 255 }); if (a.dir == 0) P(0, -8, 1, 6, shirtD); }                                        // parka with a fur collar
        if (l.style == 1) {                                                                                                                                                // Mom's apron
            if (a.dir == 0) { P(-3, -7, 6, 8, Color{ 238, 232, 216, 255 }); P(-3, -7, 6, 1, Color{ 200, 80, 76, 255 }); P(-1, -3, 2, 2, Color{ 200, 80, 76, 255 }); P(-3, -10 + hy, 1, 3, Color{ 238, 232, 216, 255 }); P(2, -10 + hy, 1, 3, Color{ 238, 232, 216, 255 }); }
            else if (side) P(flip > 0 ? 2 : -5, -7, 3, 8, Color{ 238, 232, 216, 255 });
            else { P(-3, -6, 6, 1, Color{ 238, 232, 216, 255 }); P(-1, -6, 2, 2, Color{ 238, 232, 216, 255 }); }
        }
        if (l.style == 2) { P(-5, -8, 10, 2, Color{ 232, 228, 214, 255 }); for (int k = 0; k < 5; ++k) P(-5 + k * 2.0f + 0.5f, -8 + (k % 2), 1, 1, Color{ 170, 40, 46, 255 }); }   // Dad's knitted band
        if (l.style == 3 && a.dir == 0) { P(-1, -10, 2, 3, Color{ 214, 214, 220, 255 }); P(0, -9, 1, 4, Color{ 120, 18, 24, 255 }); P(-2, -10, 1, 9, shirtD); P(1, -10, 1, 9, shirtD); P(2, -5, 1, 1, Color{ 70, 70, 80, 255 }); P(2, -2, 1, 1, Color{ 70, 70, 80, 255 }); }
        // arms
        float armSwing = moving ? (frame == 0 ? 2 : frame == 2 ? -2 : 0) : 0;
        Color hand = l.style == 3 ? Color{ 20, 20, 22, 255 } : (l.style == 0 && l.hat) ? Color{ 196, 50, 50, 255 } : l.skin;
        if (side) { R(-1 - armSwing * flip, -9, 3, 6, shirtD); R(-1 - armSwing * flip, -3, 3, 2, hand); }
        else {
            R(-7, -9 + armSwing * 0.5f, 2, 6, shirtD); R(5, -9 - armSwing * 0.5f, 2, 6, shirtD);
            R(-7, -3 + armSwing * 0.5f, 2, 2, hand); R(5, -3 - armSwing * 0.5f, 2, 2, hand);
        }
        // head
        R(-4, -18 + hy, 8, 8, l.skin);
        if (a.dir == 1) { R(-4, -19 + hy, 8, 8, l.hair); P(-4, -12 + hy, 8, 1, Mix(l.hair, kInk, 0.3f)); if (l.longHair) R(-5, -18 + hy, 10, 11, l.hair); }
        else {
            P(side ? (flip > 0 ? -4 : 2) : 2, -16 + hy, 2, 6, skinD);                          // the far side of the face
            R(-4, -19 + hy, 8, 3, l.hair);
            if (!side) { P(-4, -16 + hy, 1, 2, l.hair); P(3, -16 + hy, 1, 2, l.hair); }
            if (side) R(flip > 0 ? -4 : 2, -19 + hy, 2, 6, l.hair);
            if (l.longHair) { R(-5, -18 + hy, 2, 10, l.hair); R(3, -18 + hy, 2, 10, l.hair); P(-5, -9 + hy, 2, 1, Mix(l.hair, kInk, 0.3f)); P(3, -9 + hy, 2, 1, Mix(l.hair, kInk, 0.3f)); }
            bool blink = std::fmod(globalT + a.pos.y * 0.03f, 3.7f) < 0.12f;
            bool brim = l.style == 3;                                                           // you never quite see his eyes
            if (brim) P(-4, -16 + hy, 8, 2, Mix(l.skin, kInk, 0.55f));
            if (!blink && !brim) {
                if (a.dir == 0) { P(-2, -14 + hy, 1, 2, l.eyes); P(1, -14 + hy, 1, 2, l.eyes); }
                if (a.dir == 2) P(-3, -14 + hy, 1, 2, l.eyes);
                if (a.dir == 3) P(2, -14 + hy, 1, 2, l.eyes);
            }
            if (l.style == 0 && a.dir == 0 && !story.tainted) { P(-3, -12 + hy, 1, 1, Color{ 240, 160, 150, 255 }); P(2, -12 + hy, 1, 1, Color{ 240, 160, 150, 255 }); }   // cheeks
            if (l.beard) { R(-4, -12 + hy, 8, 3, l.hair); P(-1, -12 + hy, 2, 1, Mix(l.skin, l.hair, 0.4f)); }
            if (brim && a.dir == 0) P(-1, -12 + hy, 2, 1, Mix(l.skin, kInk, 0.4f));            // a thin mouth
        }
        P(-3, -19 + hy, 4, 1, Shade(l.hair, 34));
        // hats
        if (l.style == 3) { R(-6, -18 + hy, 12, 2, Color{ 16, 16, 20, 255 }); R(-4, -22 + hy, 8, 4, Color{ 22, 22, 26, 255 }); P(-4, -19 + hy, 8, 1, Color{ 70, 16, 20, 255 }); P(-4, -22 + hy, 8, 1, Color{ 52, 52, 60, 255 }); }
        if (l.style == 0 && l.hat) { R(-4, -20 + hy, 8, 4, Color{ 196, 50, 50, 255 }); P(-4, -17 + hy, 8, 1, Color{ 244, 240, 232, 255 }); R(-1, -22 + hy, 2, 2, Color{ 244, 240, 232, 255 }); P(-4, -20 + hy, 8, 1, Color{ 230, 90, 86, 255 }); }
    }
}

// someone on the floor. Lying on their side, not moving much
static void DrawPersonDown(const Actor& a, Look l, bool sob) {
    float s = l.scale, x = a.pos.x, y = a.pos.y + 4;
    float shake = sob ? (std::fmod(globalT, 0.5f) < 0.25f ? 0.5f : 0) : 0;
    DrawEllipse((int)x, (int)y + 2, 11 * s, 2.5f * s, Fade(BLACK, 0.3f));
    for (int pass = 0; pass < 2; ++pass) {
        auto R = [&](float ox, float oy, float w, float h, Color c) {
            int rx = (int)std::round(x + ox * s), ry = (int)std::round(y + oy * s + shake), rw = (int)std::ceil(w * s), rh = (int)std::ceil(h * s);
            if (pass == 0) DrawRectangle(rx - 1, ry - 1, rw + 2, rh + 2, kInk);
            else DrawRectangle(rx, ry, rw, rh, c);
        };
        R(-2, -5, 7, 5, l.pants);                  // legs
        R(5, -4, 2, 4, Color{ 54, 38, 30, 255 });  // shoes
        R(-10, -6, 9, 6, l.shirt);                 // body
        R(-8, -1, 5, 2, l.skin);                   // a hand on the floor
        R(-17, -7, 7, 7, l.skin);                  // head
        R(-17, -8, 7, 3, l.hair);
        if (l.longHair) R(-19, -7, 3, 7, l.hair);
        if (l.beard) R(-12, -3, 2, 3, l.hair);
        if (pass == 1) {
            R(-10, -6, 9, 1, Shade(l.shirt, 26)); R(-10, -1, 9, 1, Mix(l.shirt, kInk, 0.3f)); R(-2, -1, 7, 1, Mix(l.pants, kInk, 0.3f));
            if (l.style == 1) R(-8, -5, 5, 4, Color{ 238, 232, 216, 255 });
            if (l.style == 2) R(-7, -6, 2, 6, Color{ 232, 228, 214, 255 });
            R(-15, -4, 2, 1, Color{ 40, 30, 30, 255 });   // closed eye
        }
    }
}

// v0.18: the box Mom tore off his belt, or the taser Dad tore out of his hand. Lying on the floor, blinking
static void DrawDrop(const std::string& what, Vector2 c) {
    int x = (int)c.x, y = (int)c.y + 3;
    float pulse = 0.5f + 0.5f * std::sin(globalT * 4);
    DrawEllipse(x, y + 3, 6, 1.6f, Fade(BLACK, 0.3f));
    if (what == "box") {
        DrawRectangle(x - 4, y - 3, 8, 6, kInk);
        DrawRectangle(x - 3, y - 2, 6, 4, Color{ 34, 34, 40, 255 });
        DrawRectangle(x - 3, y - 2, 6, 1, Color{ 60, 60, 68, 255 });
        DrawRectangle(x + 1, y - 1, 1, 1, Fade(Color{ 120, 255, 140, 255 }, 0.5f + 0.5f * pulse));
        DrawLine(x - 4, y, x - 8, y + 2, Color{ 150, 40, 40, 255 });   // the torn wire
    } else {
        DrawRectangle(x - 5, y - 2, 10, 5, kInk);
        DrawRectangle(x - 4, y - 1, 6, 3, Color{ 232, 196, 40, 255 });
        DrawRectangle(x + 2, y - 1, 2, 3, Color{ 30, 30, 34, 255 });
        DrawRectangle(x - 3, y, 1, 1, Fade(Color{ 120, 200, 255, 255 }, 0.4f + 0.6f * pulse));
    }
    // a small glint, so you can find it in the dark
    DrawCircleV({ (float)x, (float)y - 1 }, 6 + 2 * pulse, Fade(Color{ 255, 240, 200, 255 }, 0.05f + 0.05f * pulse));
}

static void DrawDoggoDead(const Actor& a) {
    // fade to black, and this is what's left. He doesn't move. A thin line of red keeps running
    float x = a.pos.x, y = a.pos.y + 4;
    float t = doggoDeadT;
    Color fur = { 156, 110, 64, 255 }, dark = { 106, 70, 40, 255 }, blood = { 150, 16, 24, 255 };
    float pool = std::min(8.0f, 2.0f + t * 0.4f);
    DrawEllipse((int)x + 2, (int)y + 2, pool + 2, pool * 0.45f, Fade(blood, 0.9f));
    DrawEllipse((int)x + 1, (int)y + 1, pool * 0.6f, pool * 0.2f, Fade(Color{ 190, 30, 36, 255 }, 0.6f));
    float streak = std::min(18.0f, t * 1.2f);
    DrawRectangle((int)x + 3, (int)y + 2, 1, (int)streak, blood);
    if (std::fmod(t, 1.6f) < 0.8f && streak < 18) DrawRectangle((int)x + 3, (int)(y + 2 + streak), 1, 1, Color{ 160, 20, 24, 255 });
    DrawRectangle((int)x - 8, (int)y - 5, 15, 7, kInk); DrawRectangle((int)x - 14, (int)y - 5, 8, 7, kInk);
    DrawRectangle((int)x - 7, (int)y - 4, 13, 5, fur);                      // on his side
    DrawRectangle((int)x - 7, (int)y - 4, 13, 1, Shade(fur, 22)); DrawRectangle((int)x - 5, (int)y - 1, 9, 2, Color{ 214, 180, 132, 255 });
    DrawRectangle((int)x - 5, (int)y + 1, 4, 2, dark); DrawRectangle((int)x + 1, (int)y + 1, 4, 2, dark);   // legs, still
    DrawRectangle((int)x - 13, (int)y - 4, 6, 5, fur);                      // head flat on the floor
    DrawRectangle((int)x - 12, (int)y - 5, 2, 3, dark);                     // ear
    DrawRectangle((int)x - 11, (int)y - 2, 2, 1, Color{ 30, 20, 20, 255 }); // closed eye
    DrawRectangle((int)x - 8, (int)y - 4, 1, 5, Color{ 190, 40, 44, 255 }); // his collar
    DrawRectangle((int)x + 6, (int)y - 2, 4, 2, dark);                      // tail, not wagging
}

static void DrawDoggo(const Actor& a) {
    if (!a.visible) return;
    bool moving = a.anim > 0;
    int frame = moving ? ((int)(a.anim * 1.4f)) % 4 : 0;
    float bob = moving && (frame == 1 || frame == 3) ? -1 : 0;
    int x = (int)a.pos.x, y = (int)(a.pos.y + 3 + bob);
    Color fur = { 176, 124, 72, 255 }, dark = { 118, 78, 44, 255 }, light = { 226, 192, 142, 255 };
    bool out = IsOutsideT(TileAt(TX(a.pos.x), TX(a.pos.y + 4)));
    DrawEllipse(x, (int)a.pos.y + 6, 8, 2, out ? Fade(Color{ 60, 76, 140, 255 }, 0.4f) : Fade(BLACK, 0.3f));
    int f = a.dir == 2 ? -1 : 1;                                             // which way he faces
    auto R = [&](int ox, int oy, int w, int h, Color c) { DrawRectangle(f > 0 ? x + ox : x - ox - w, y + oy, w, h, c); };
    float wag = std::sin(globalT * (moving ? 20 : 12)) * 2;
    // outline
    R(-7, -7, 14, 8, kInk); R(2, -12, 9, 9, kInk); R(-9, (int)(-9 + wag), 4, 6, kInk); R(8, -8, 4, 5, kInk);
    R(-8, (int)(-8 + wag), 2, 4, dark); R(-8, (int)(-8 + wag), 2, 1, light);  // tail
    R(-6, -6, 12, 6, fur); R(-6, -6, 12, 1, Shade(fur, 26)); R(-5, -2, 10, 2, light); R(-6, -1, 1, 1, fur);
    R(-3, -6, 4, 3, dark);                                                   // the saddle patch
    int l1 = frame == 0 ? 1 : 0, l2 = frame == 2 ? 1 : 0;                    // legs trot in pairs
    R(-5, -l1, 2, 3, dark); R(-2, -l2, 2, 3, fur); R(1, -l1, 2, 3, dark); R(4, -l2, 2, 3, fur);
    R(-2, 2 - l2, 2, 1, light); R(4, 2 - l2, 2, 1, light);
    R(3, -11, 7, 7, fur); R(3, -11, 7, 1, Shade(fur, 26));                   // head
    R(8, -8, 3, 3, light); R(10, -8, 1, 1, kInk);                            // snout and nose
    R(7, -9, 1, 2, kInk); R(7, -9, 1, 1, Color{ 60, 40, 30, 255 });          // eye
    R(3, -12, 2, 5, dark); R(3, -12, 2, 1, Shade(dark, 20));                 // ear
    R(2, -5, 1, 4, Color{ 200, 44, 48, 255 }); R(2, -3, 1, 1, GOLD);         // collar and tag
    if (!moving && std::fmod(globalT, 1.0f) < 0.5f) R(9, -5, 1, 2, Color{ 230, 110, 120, 255 });   // tongue out
}

// the visitor's car, nose toward the gate
static void DrawCar() {
    int x = 20 * TS - 6, y = 31 * TS + 2;
    auto R = [&](int ox, int oy, int w, int h, Color c) { DrawRectangle(x + ox, y + oy, w, h, c); };
    DrawEllipse(x + 14, y + 23, 15, 3, Fade(Color{ 50, 64, 130, 255 }, 0.5f));
    R(-1, 1, 30, 22, kInk); R(1, -1, 26, 3, kInk);
    R(0, 2, 28, 20, Color{ 26, 28, 36, 255 }); R(2, 0, 24, 3, Color{ 26, 28, 36, 255 });
    R(0, 2, 28, 1, Color{ 60, 64, 78, 255 }); R(0, 3, 1, 18, Color{ 44, 48, 60, 255 }); R(27, 3, 1, 18, Color{ 12, 12, 18, 255 });
    Color lamp = story.EngineOn() ? Color{ 255, 246, 200, 255 } : Color{ 92, 96, 108, 255 };   // v0.19: you took the key. Dark lamps
    R(3, 0, 5, 2, lamp); R(20, 0, 5, 2, lamp); R(10, 0, 8, 2, Color{ 60, 64, 76, 255 });      // lamps and grille
    R(3, 5, 22, 5, Color{ 50, 64, 92, 255 }); R(3, 5, 22, 1, Color{ 96, 116, 150, 255 }); R(5, 6, 5, 1, Fade(WHITE, 0.3f));                 // windscreen
    R(14, 5, 1, 5, Color{ 26, 28, 36, 255 });
    R(2, 10, 24, 6, Color{ 244, 248, 254, 255 }); R(2, 15, 24, 1, Color{ 190, 204, 234, 255 }); R(4, 9, 8, 1, Color{ 244, 248, 254, 255 }); // snow on the roof
    Color tail = story.EngineOn() ? Color{ 200, 30, 36, 255 } : Color{ 96, 40, 44, 255 };
    R(3, 17, 22, 3, Color{ 40, 50, 74, 255 }); R(1, 20, 4, 2, tail); R(23, 20, 4, 2, tail);       // rear window, tail lamps
    R(-2, 6, 2, 3, Color{ 26, 28, 36, 255 }); R(28, 6, 2, 3, Color{ 26, 28, 36, 255 });                                                    // mirrors
    R(-1, 4, 2, 5, kInk); R(27, 4, 2, 5, kInk); R(-1, 15, 2, 5, kInk); R(27, 15, 2, 5, kInk);                                              // wheels
    R(10, 21, 8, 1, Color{ 226, 226, 230, 255 });
}
