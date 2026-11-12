// 600 Seconds - art/Look.inc (v0.20 look)
// Darkness you can see in the house instead of a number (T-043), the quiet flux from loop 2 (T-044),
// and the big moments staged with light (T-045): the lights die room by room at 16:50, lightning shows
// his silhouette in the storm, and his headlights sweep the walls when the car turns in at 16:43.
// Everything here is drawn in code. No image files.

// ---------------------------------------------------------------- the power, room by room
static float lkPowerAt = -1;          // when the power went (globalT). -1 = still on
static bool lkPowerPrev = false;
static const float LK_ROOM_GAP = 0.42f;
static int LkPowerOrder(Room r) {
    switch (r) {   // from the generator in the shed, across the house, your room last, then the lamps outside
    case Room::Shed: return 0;
    case Room::LivingRoom: return 1;
    case Room::Kitchen: return 2;
    case Room::Hallway: return 3;
    case Room::ParentsRoom: return 4;
    case Room::Bathroom: return 5;
    case Room::KidsRoom: return 6;
    default: return 7;
    }
}
static float RoomPower(Room r) {
    if (!story.PowerOut()) return 1;
    if (lkPowerAt < 0) return 0;
    float k = globalT - lkPowerAt - LkPowerOrder(r) * LK_ROOM_GAP;
    if (k >= 0) return 0;
    if (k < -0.32f) return 1;
    if (settings.calmFlashes) return -k / 0.32f;                       // calm: a soft fade instead of the flicker
    return std::fmod(globalT * 1000.0f, 110.0f) < 55.0f ? 0.12f : 1.0f; // it fights for a moment, then gives up
}
static float RoomPowerAt(int tx, int ty) { return RoomPower(story.world.RoomAt(tx, ty)); }
static bool LkObjDark(const Obj& o) { return RoomPowerAt(o.x, o.y) < 0.5f; }
static bool LkAnyPower() { return RoomPower(Room::Outside) > 0.01f; }

// ---------------------------------------------------------------- timers
static float lkSweepAt = -100;        // his headlights turning in at the gate
static bool lkCarPrev = false;
static float lkBoltT = 99, lkBoltNext = 4;
static int lkBoltN = 0;
static float lkIdleT = 0;
static Vector2 lkIdlePos{};
static void LookTick(float dt) {
    bool out = story.PowerOut();
    if (out && !lkPowerPrev) lkPowerAt = globalT;
    if (!out) lkPowerAt = -1;
    lkPowerPrev = out;
    bool car = story.CarHere();
    if (car && !lkCarPrev && !story.collection && story.EngineOn()) lkSweepAt = globalT;
    lkCarPrev = car;
    // lightning, every 8 to 16 seconds while the storm is on
    if (story.Storm() && !viewBelow) {
        lkBoltNext -= dt;
        if (lkBoltNext <= 0) { lkBoltT = 0; ++lkBoltN; lkBoltNext = 8.0f + (float)(Hash(lkBoltN, 913) % 9); }
    } else lkBoltNext = std::max(lkBoltNext, 2.5f);
    lkBoltT += dt;
    // the kid, standing still
    bool still = std::fabs(player.pos.x - lkIdlePos.x) < 0.01f && std::fabs(player.pos.y - lkIdlePos.y) < 0.01f;
    if (!still || beatActive || hidden || scene != SceneMode::None || journalOpen) lkIdleT = 0; else lkIdleT += dt;
    lkIdlePos = player.pos;
}
static float LkBolt() {
    float t = lkBoltT;
    if (t > 1.0f) return 0;
    if (settings.calmFlashes) return t < 0.7f ? 0.3f * (1 - t / 0.7f) : 0;
    if (t < 0.07f) return 1;
    if (t < 0.15f) return 0.2f;
    if (t < 0.24f) return 0.85f;
    return std::max(0.0f, 0.85f - (t - 0.24f) * 1.5f);
}

// ---------------------------------------------------------------- 16:43, the headlights
// Two passes. In the light map (LookLightmap) the dark parts of the house and the yard brighten as the beams cross.
// After it (LookGlows, additive) the shape of a window slides across the wall and floor of your room, so it shows
// even with the lamps on. Then the living room and the kitchen, where the light really comes in.
static float LkSweep() { float k = (globalT - lkSweepAt) / 3.2f; return k > 0 && k < 1 ? k : -1; }
static void LookLightmap(float inH) {
    float k = LkSweep();
    if (k < 0 || viewBelow) return;
    float bell = std::sin(k * 3.14159f);
    Color hl = { 196, 186, 140, 255 };
    Glow(9.6f * TS - k * 6.4f * TS, 4.6f * TS, 14, 40, hl, 0.9f * bell * inH);
    Glow(28.0f * TS - k * 24.0f * TS, 16.0f * TS, 18, 60, hl, 0.7f * bell * inH);
    Glow(29.0f * TS - k * 22.0f * TS, 25.0f * TS, 40, 26, hl, 0.6f * bell);
}
static void LookGlows(float inH) {
    float k = LkSweep();
    if (k < 0 || viewBelow) return;
    float bell = std::sin(k * 3.14159f);
    float a = 0.55f * bell * inH;
    if (a < 0.01f) return;
    Color hl = { 255, 248, 214, 255 };
    // your room: the window, four panes of light, walking along the wall and down over the floor
    float x = 9.4f * TS - k * 6.6f * TS, sk = (k - 0.5f) * 10;   // it leans as the car turns
    for (int py = 0; py < 2; ++py) for (int px = 0; px < 2; ++px) {
        float bx = x + px * 7 - 7, by = 2 * TS + 4.0f + py * 7;
        DrawRectangle((int)(bx + sk * py), (int)by, 6, 6, Fade(hl, a));
    }
    for (int r = 0; r < 4; ++r) DrawRectangle((int)(x - 8 + sk * (2 + r)), 3 * TS + r * 8, 15, 8, Fade(hl, a * (0.5f - r * 0.1f)));
    Glow(x, 2 * TS + 10.0f, 16, 12, hl, a * 0.8f);
    // the living room and the kitchen
    float x2 = 27.5f * TS - k * 23.0f * TS;
    for (int r = 0; r < 6; ++r) DrawRectangle((int)(x2 - 10 + (k - 0.5f) * 4 * r), 13 * TS + r * 12, 20, 12, Fade(hl, a * (0.45f - r * 0.06f)));
}

// ---------------------------------------------------------------- stage 1 flux: three small wrong things
// From loop 2, once you're paying attention, the house is not quite the same every time. Seeded by the loop
// number, so it holds still for the whole hour and changes when the hour does.
enum { LK_PICTURE = 1, LK_MUG = 2, LK_CLOCK = 4 };
static int LkFlux() {
    if (story.loop < 2 || story.Stage() < 1) return 0;
    unsigned h = Hash(story.loop, 4401);
    int bits = 0;
    if (h % 100 < 40) bits |= LK_PICTURE;
    if ((h >> 7) % 100 < 35) bits |= LK_MUG;
    if ((h >> 14) % 100 < 35) bits |= LK_CLOCK;
    if (bits == 0 && (h >> 21) % 4 != 0) bits = 1 << ((h >> 3) % 3);   // most loops have one
    return bits;
}
static unsigned LkFluxSeed() { return Hash(story.loop, 4419); }

// rotate a small rect around (cx, cy). ox, oy = the rect's top left, relative to the centre
static void LkRot(float cx, float cy, float ang, float ox, float oy, float w, float h, Color c) {
    DrawRectanglePro({ cx, cy, w, h }, { -ox, -oy }, ang, c);
}

// the family photo in the hallway, over the coats side. Tilted when reality slipped. Scratched when you're dark
static void LkPicture() {
    float cx = 18 * TS + 8.0f, cy = 9 * TS + 9.0f;
    float ang = 0;
    if (LkFlux() & LK_PICTURE) ang = (LkFluxSeed() & 1) ? 9.0f : -8.0f;
    Color frame = { 84, 54, 34, 255 }, photo = { 214, 198, 166, 255 };
    LkRot(cx, cy + 1, ang, -8, -5, 16, 12, Fade(BLACK, 0.25f));                   // its shadow on the wall
    LkRot(cx, cy, ang, -8, -6, 16, 12, kInk);
    LkRot(cx, cy, ang, -7, -5, 14, 10, frame);
    LkRot(cx, cy, ang, -6, -4, 12, 8, photo);
    LkRot(cx, cy, ang, -6, -4, 12, 3, Color{ 150, 176, 210, 255 });            // sky over the vidda
    // Dad, Mom, the kid in front of the house
    bool scratched = story.tainted;
    Color dadC = scratched ? Color{ 60, 50, 50, 255 } : Color{ 56, 86, 154, 255 }, momC = scratched ? Color{ 60, 50, 50, 255 } : Color{ 66, 126, 92, 255 };
    LkRot(cx, cy, ang, -5, -2, 3, 6, dadC); LkRot(cx, cy, ang, -5, -3, 3, 2, scratched ? Color{ 30, 26, 30, 255 } : Color{ 232, 192, 162, 255 });
    LkRot(cx, cy, ang, 2, -2, 3, 6, momC); LkRot(cx, cy, ang, 2, -3, 3, 2, scratched ? Color{ 30, 26, 30, 255 } : Color{ 238, 198, 168, 255 });
    LkRot(cx, cy, ang, -1, 0, 2, 4, Color{ 200, 50, 50, 255 }); LkRot(cx, cy, ang, -1, -1, 2, 1, Color{ 242, 204, 174, 255 });
    if (scratched) { LkRot(cx, cy, ang, -6, -3, 5, 1, Color{ 240, 236, 230, 255 }); LkRot(cx, cy, ang, 1, -3, 5, 1, Color{ 240, 236, 230, 255 }); }   // their faces, scratched out with a nail
    LkRot(cx, cy, ang, -7, -5, 14, 1, Shade(frame, 30));
    LkRot(cx, cy, ang, -1, -9, 2, 3, Color{ 150, 150, 160, 255 });            // the nail
}

// the kitchen clock, on the wall between the pantry and the advent calendar
static void LkClock() {
    float cx = 5 * TS + 8.0f, cy = 12 * TS + 9.0f;
    DrawCircle((int)cx, (int)cy + 1, 6, Fade(BLACK, 0.25f));
    DrawCircle((int)cx, (int)cy, 6, kInk);
    DrawCircle((int)cx, (int)cy, 5, Color{ 196, 64, 56, 255 });
    DrawCircle((int)cx, (int)cy, 4, Color{ 244, 240, 228, 255 });
    for (int k = 0; k < 4; ++k) DrawRectangle((int)(cx + std::cos(k * 1.5708f) * 3.4f), (int)(cy + std::sin(k * 1.5708f) * 3.4f), 1, 1, Color{ 90, 80, 76, 255 });
    float mins = 16 * 60 + story.t / 60.0f;                              // the real time
    if (LkFlux() & LK_CLOCK) {
        unsigned s = LkFluxSeed() >> 3;
        if (s % 3 == 0) mins = 15 * 60 + 55;                            // stopped at juice time
        else if (s % 3 == 1) mins = 17 * 60;                            // stopped at the bells
        else mins = 17 * 60 - story.t / 60.0f;                          // running the wrong way
    }
    float ma = (std::fmod(mins, 60.0f) / 60.0f) * 6.2832f - 1.5708f, ha = (std::fmod(mins / 60.0f, 12.0f) / 12.0f) * 6.2832f - 1.5708f;
    DrawLineEx({ cx, cy }, { cx + std::cos(ma) * 3.6f, cy + std::sin(ma) * 3.6f }, 1.0f, kInk);
    DrawLineEx({ cx, cy }, { cx + std::cos(ha) * 2.3f, cy + std::sin(ha) * 2.3f }, 1.0f, Color{ 120, 30, 30, 255 });
    DrawRectangle((int)cx, (int)cy, 1, 1, kInk);
}

// Mom's mug of coffee. On the counter by the sink. Some loops it's somewhere it shouldn't be
static Vector2 LkMugPos() {
    if (LkFlux() & LK_MUG) {
        unsigned s = LkFluxSeed() >> 9;
        if (s % 3 == 0) return { 7 * TS + 6.0f, 16 * TS + 9.0f };      // in the middle of the floor
        if (s % 3 == 1) return { 9 * TS + 5.0f, 12 * TS + 3.0f };      // on top of the fridge
        return { 5 * TS + 13.0f, 19 * TS + 9.0f };                     // next to Doggo's bowl
    }
    return { 8 * TS + 12.0f, 13 * TS + 11.0f };
}
static float LookMugSortY() { return LkMugPos().y + 2; }
static void LookMug() {
    if (viewBelow) return;
    Vector2 p = LkMugPos();
    int x = (int)p.x, y = (int)p.y;
    DrawRectangle(x - 1, y + 3, 5, 1, Fade(BLACK, 0.25f));
    DrawRectangle(x - 1, y - 1, 5, 5, kInk);
    DrawRectangle(x, y, 3, 3, Color{ 244, 240, 232, 255 });
    DrawRectangle(x, y + 1, 3, 1, Color{ 200, 60, 56, 255 });
    DrawRectangle(x + 3, y + 1, 1, 1, Color{ 244, 240, 232, 255 });
    DrawRectangle(x, y, 3, 1, Color{ 96, 58, 34, 255 });                // coffee
    if (!(LkFlux() & LK_MUG) && std::fmod(globalT, 2.4f) < 1.4f) DrawRectangle(x + 1, y - 3 - (int)(std::fmod(globalT, 2.4f) * 1.5f), 1, 1, Fade(WHITE, 0.5f));   // still warm
}

// ---------------------------------------------------------------- darkness in the house
// Your room keeps count. A tally scratched into the wallpaper over your bed, one line per point
static void LkTally() {
    int n = std::min(story.darkPoints, DARK_MAX);
    if (n <= 0) return;
    float x0 = 4 * TS + 1.0f, y0 = 2 * TS + 8.0f;
    Color cut = story.tainted ? Color{ 214, 150, 160, 255 } : Color{ 216, 212, 236, 255 }, deep = Color{ 30, 22, 40, 255 };
    for (int i = 0; i < n; ++i) {
        int g = i / 5, j = i % 5;
        float gx = x0 + g * 6;
        if (j < 4) { DrawRectangle((int)gx + j, (int)y0 + (j % 2), 1, 5, deep); DrawRectangle((int)gx + j, (int)y0 + (j % 2), 1, 4, cut); }
        else DrawLineEx({ gx - 1, y0 + 4 }, { gx + 4.5f, y0 + 0.5f }, 1.0f, cut);
    }
}
// Your drawing of the four of you. It changes as you do
static void LkDrawing() {
    float x = 7 * TS + 3.0f, y = 2 * TS + 4.0f;
    int d = story.darkPoints;
    DrawRectangle((int)x - 1, (int)y - 1, 13, 12, Fade(BLACK, 0.3f));
    DrawRectangle((int)x, (int)y, 12, 10, Color{ 248, 246, 238, 255 });
    DrawRectangle((int)x + 4, (int)y - 1, 4, 2, Fade(Color{ 230, 226, 200, 255 }, 0.8f));             // tape
    auto P = [&](int ox, int oy, int w, int h, Color c) { DrawRectangle((int)x + ox, (int)y + oy, w, h, c); };
    P(1, 1, 2, 2, d >= 3 ? Color{ 30, 30, 34, 255 } : Color{ 250, 206, 60, 255 });                     // the sun. Coloured over, once you're dark
    P(0, 8, 12, 2, Color{ 200, 224, 236, 255 });                                                       // snow
    P(2, 4, 1, 4, Color{ 56, 86, 154, 255 }); P(2, 3, 1, 1, Color{ 56, 86, 154, 255 });               // Dad
    P(4, 5, 1, 3, Color{ 66, 126, 92, 255 }); P(4, 4, 1, 1, Color{ 66, 126, 92, 255 });               // Mom
    P(6, 6, 1, 2, Color{ 210, 50, 50, 255 }); P(6, 5, 1, 1, Color{ 210, 50, 50, 255 });               // you
    P(8, 7, 2, 1, Color{ 150, 100, 50, 255 }); P(9, 6, 1, 1, Color{ 150, 100, 50, 255 });             // Doggo
    if (d >= 6) { for (int k = 0; k < 3; ++k) { P(1 + k, 3 + k, 1, 1, Color{ 200, 30, 30, 255 }); P(3 - k, 3 + k, 1, 1, Color{ 200, 30, 30, 255 }); P(3 + k, 4 + k, 1, 1, Color{ 200, 30, 30, 255 }); P(5 - k, 4 + k, 1, 1, Color{ 200, 30, 30, 255 }); } }   // them, crossed out
    if (d >= 10) { P(10, 2, 1, 6, Color{ 20, 20, 24, 255 }); P(9, 2, 3, 1, Color{ 20, 20, 24, 255 }); P(10, 1, 1, 1, Color{ 20, 20, 24, 255 }); }   // somebody tall, in a hat, added later
    if (d >= DARK_MAX) P(6, 5, 1, 3, Color{ 20, 20, 24, 255 });                                      // and you, coloured in black
}
static void LookWallProps() {
    if (viewBelow) return;
    LkTally();
    LkDrawing();
    LkPicture();
    LkClock();
}

// ---------------------------------------------------------------- the kid, standing still
// Light: he checks his watch. A little dark: he looks around the room, then at you. Tainted: he just stares
static void LookKidShadow(const Actor& a) {
    if (story.darkPoints < 10 || hidden || !a.visible) return;
    // his shadow is somebody taller. With a hat
    float x = a.pos.x, y = a.pos.y + 6;
    Color s = Fade(Color{ 8, 6, 14, 255 }, 0.3f);
    DrawRectangle((int)x - 3, (int)y - 14, 7, 14, s);
    DrawRectangle((int)x - 2, (int)y - 21, 5, 7, s);
    DrawRectangle((int)x - 4, (int)y - 22, 9, 2, s);
    DrawRectangle((int)x - 2, (int)y - 25, 5, 3, s);
}
static bool LookDrawKid(const Actor& a, const Look& l) {
    if (lkIdleT < 3.5f || story.chase || story.Tutorial()) return false;
    int d = story.darkPoints;
    Actor b = a;
    float c = std::fmod(lkIdleT - 3.5f, 6.0f);
    if (d >= 6) b.dir = 0;
    else if (d >= 3) b.dir = c < 1.2f ? 2 : c < 2.4f ? 3 : c < 3.2f ? 1 : 0;
    DrawPerson(b, l);
    float x = b.pos.x, y = b.pos.y + 4;
    if (d < 3) {
        if (b.dir == 0 && c > 1.5f && c < 3.3f) {
            // the arm up, the watch at his face
            DrawRectangle((int)x - 6, (int)y - 9, 7, 3, kInk);
            DrawRectangle((int)x - 5, (int)y - 8, 5, 1, Mix(l.shirt, kInk, 0.28f));
            DrawRectangle((int)x - 1, (int)y - 9, 3, 2, l.skin);
            DrawRectangle((int)x, (int)y - 10, 2, 1, std::fmod(globalT, 0.5f) < 0.25f ? Color{ 180, 226, 160, 255 } : Color{ 150, 178, 186, 255 });
        }
    } else if (d < 6) {
        if (b.dir == 0) DrawRectangle((int)x - 2, (int)y - 12, 4, 1, Mix(l.skin, kInk, 0.35f));   // not smiling
    } else {
        float p = 0.5f + 0.5f * std::sin(globalT * 2.2f);
        DrawRectangle((int)x - 2, (int)y - 14, 1, 1, Fade(Color{ 255, 60, 60, 255 }, 0.4f + 0.6f * p));
        DrawRectangle((int)x + 1, (int)y - 14, 1, 1, Fade(Color{ 255, 60, 60, 255 }, 0.4f + 0.6f * p));
        if (d >= 10) { DrawRectangle((int)x - 2, (int)y - 11, 4, 1, Color{ 110, 20, 26, 255 }); DrawRectangle((int)x - 3, (int)y - 12, 1, 1, Color{ 110, 20, 26, 255 }); DrawRectangle((int)x + 2, (int)y - 12, 1, 1, Color{ 110, 20, 26, 255 }); }   // a smile that isn't his
    }
    return true;
}

// ---------------------------------------------------------------- lightning
static void LkSilhouette(const Actor& a) {
    Look s = LookVisitor();
    s.shirt = s.pants = s.hair = s.skin = Color{ 4, 4, 8, 255 };
    s.eyes = Color{ 4, 4, 8, 255 };
    // the shadow the flash throws, long, away from the sky
    for (int k = 0; k < 18; ++k) DrawRectangle((int)a.pos.x - 3 + k, (int)a.pos.y + 6 + k / 3, 7, 2, Fade(Color{ 4, 4, 8, 255 }, 0.55f * (1 - k / 18.0f)));
    DrawPerson(a, s);
}
static void LookStormFx(Camera2D cam) {
    float b = LkBolt();
    if (b <= 0.01f || viewBelow) return;
    bool kidOut = OutdoorsAt(player.pos);
    DrawRectangle(0, 0, VW, VH, Fade(Color{ 226, 232, 255, 255 }, b * (kidOut ? 0.6f : 0.28f)));
    if (b < 0.3f) return;
    BeginMode2D(cam);
    bool canSee = !story.collection || VisitorShown() || kidOut;
    if (visitor.visible && canSee && OutdoorsAt(visitor.pos)) LkSilhouette(visitor);
    else if (story.CarHere() && !visitor.visible && !story.collection) {
        // in his car: the windscreen goes white and he's a shape behind the wheel
        int x = 20 * TS - 6, y = 31 * TS + 2;
        DrawRectangle(x + 3, y + 5, 22, 5, Fade(Color{ 220, 228, 250, 255 }, b));
        DrawRectangle(x + 6, y + 6, 5, 4, Color{ 4, 4, 8, 255 }); DrawRectangle(x + 5, y + 6, 7, 1, Color{ 4, 4, 8, 255 });
    }
    EndMode2D();
}
