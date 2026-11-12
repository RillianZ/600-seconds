// 600 Seconds - art/LookUI.inc (v0.20 look)
// The screen side of the v0.20 look: faces in the dialogue box (T-047), the ORIGIN watch going dark (T-043),
// a quieter HUD with the chores on the fridge (T-048), handwriting in the memory log (T-049),
// full screen close-ups (T-046) and a small picture on every ending card (T-103).
// All of it painted from rectangles. No image files.

// ---------------------------------------------------------------- a painter on a grid of cells (screen pixels)
struct LkPaint {
    float ox, oy, c;          // top left of cell 0,0 and the cell size, in screen pixels
    float a = 1;              // alpha for everything
    void R(float x, float y, float w, float h, Color col) const {
        if (a < 1) col = Fade(col, a * col.a / 255.0f);
        DrawRectangle((int)std::round(ox + x * c), (int)std::round(oy + y * c), (int)std::round((x + w) * c) - (int)std::round(x * c), (int)std::round((y + h) * c) - (int)std::round(y * c), col);
    }
};
static LkPaint LkUI(float x, float y, float cell) { return { x * US, y * US, cell * US }; }   // x, y, cell in virtual UI units

// ---------------------------------------------------------------- T-047: faces
enum LkExpr { LX_NORMAL, LX_SMILE, LX_COLD, LX_SCARED, LX_BROKEN, LX_STARE, LX_WORRIED, LX_TIRED };
// style: 0 you, 1 Mom, 2 Dad, 3 the visitor, 4 the visitor without his hat (you, older)
static void LkBustDraw(const LkPaint& p, int style, LkExpr e, bool talking, bool blinkOK, float seed) {
    Look l = style == 0 ? LookKid() : style == 1 ? LookMom() : style == 2 ? LookDad() : LookVisitor();
    Color skin = l.skin, hair = l.hair, shirt = l.shirt;
    if (style == 4) { hair = Color{ 86, 58, 34, 255 }; skin = Color{ 214, 196, 186, 255 }; }   // his hair is your hair
    if (e == LX_STARE) skin = Mix(skin, Color{ 210, 206, 214, 255 }, 0.5f);
    Color skinD = Mix(skin, Color{ 150, 80, 80, 255 }, 0.3f), hairD = Mix(hair, kInk, 0.35f), shirtD = Mix(shirt, kInk, 0.3f);
    Color eyeC = (style == 0 && story.tainted) ? Color{ 150, 0, 0, 255 } : style >= 3 ? Color{ 40, 30, 30, 255 } : Color{ 30, 22, 30, 255 };
    Color lip = Mix(skin, Color{ 120, 40, 50, 255 }, 0.5f), dark = Color{ 70, 22, 30, 255 };
    bool coat = style == 0 && story.hasCoat;
    bool handler = style == 1 && story.Knows("the_tape");
    // outline pass, then the person
    for (int pass = 0; pass < 2; ++pass) {
        auto B = [&](float x, float y, float w, float h, Color col) { if (pass == 0) p.R(x - 0.5f, y - 0.5f, w + 1, h + 1, kInk); else p.R(x, y, w, h, col); };
        // shoulders
        if (style >= 3) { B(1, 14, 17, 5, shirt); }
        else B(2, 14, 15, 5, coat ? Color{ 48, 78, 128, 255 } : shirt);
        // long hair behind the shoulders
        if (style == 1 && !handler) { B(3, 4, 2, 12, hair); B(14, 4, 2, 12, hair); }
        B(8, 12, 3, 3, skinD);                           // neck
        B(5, 3, 9, 10, skin);                            // head
        if (style == 2) B(5, 10, 9, 4, hair);            // Dad's beard
        // hair on top
        if (style == 0 && !coat) { B(4, 2, 11, 3, hair); }
        if (style == 1) { B(4, 2, 11, 3, hair); if (handler) B(8, 0, 3, 2, hair); }
        if (style == 2) B(5, 2, 9, 2, hair);
        if (style == 4) { B(4, 2, 11, 2, hair); }
        // hats
        if (coat) { B(4, 1, 11, 4, Color{ 196, 50, 50, 255 }); B(8, -1, 3, 2, Color{ 244, 240, 232, 255 }); }
        if (style >= 3) { B(3, 10, 3, 5, shirt); B(13, 10, 3, 5, shirt); }                       // his collar, up round his jaw
        if (style == 3) { B(5, 0, 9, 3, Color{ 22, 22, 26, 255 }); B(2, 3, 15, 2, Color{ 16, 16, 20, 255 }); }
    }
    // ---- details
    auto P = [&](float x, float y, float w, float h, Color col) { p.R(x, y, w, h, col); };
    P(13, 4, 1, 8, skinD); P(6, 12, 7, 1, skinD);       // the far side, under the chin
    P(4, 7, 1, 2, skinD); P(14, 7, 1, 2, skinD);       // ears
    // clothes
    if (style == 0 && !coat) { P(2, 14, 15, 1, Shade(shirt, 26)); for (int k = 0; k < 6; ++k) P(3 + k * 2.4f, 16, 1, 1, Color{ 250, 240, 230, 255 }); P(2, 17, 15, 1, shirtD); P(8, 13, 3, 1, Color{ 250, 240, 230, 255 }); }
    if (coat) { P(4, 13, 11, 2, Color{ 226, 214, 190, 255 }); P(9, 15, 1, 4, Color{ 30, 50, 90, 255 }); for (int k = 0; k < 3; ++k) P(3 + k * 6, 17, 3, 1, Color{ 210, 220, 140, 255 }); }
    if (style == 1) { P(6, 14, 1, 5, Color{ 238, 232, 216, 255 }); P(12, 14, 1, 5, Color{ 238, 232, 216, 255 }); P(7, 17, 5, 2, Color{ 238, 232, 216, 255 }); P(7, 17, 5, 1, Color{ 200, 80, 76, 255 }); P(8, 13, 3, 1, shirtD); if (!handler) { P(4, 9, 1, 1, Color{ 226, 196, 100, 255 }); P(14, 9, 1, 1, Color{ 226, 196, 100, 255 }); } }
    if (style == 2) { P(2, 15, 15, 2, Color{ 232, 228, 214, 255 }); for (int k = 0; k < 7; ++k) P(2.5f + k * 2, 15 + (k % 2), 1, 1, Color{ 170, 40, 46, 255 }); P(2, 14, 15, 1, Shade(shirt, 24)); P(8, 13, 3, 1, shirtD); }
    if (style >= 3) {
        P(3, 10, 1, 5, Mix(shirt, WHITE, 0.1f)); P(15, 10, 1, 5, Mix(shirt, kInk, 0.4f));               // the collar, lit on one side
        P(8, 13, 3, 2, Color{ 214, 214, 220, 255 }); P(9, 14, 1, 5, Color{ 120, 18, 24, 255 });        // shirt and tie
        P(1, 14, 17, 1, Mix(shirt, WHITE, 0.06f));
    }
    // hair detail
    if (style == 0 && !coat) { P(5, 1, 2, 1, hair); P(9, 1, 1, 1, hair); P(12, 1, 2, 1, hair); P(6, 5, 2, 1, hair); P(9, 5, 1, 1, hair); P(5, 5, 1, 2, hair); P(13, 5, 1, 1, hair); P(5, 2, 4, 1, Shade(hair, 30)); }
    if (style == 1) { P(9, 2, 1, 2, hairD); P(5, 5, 1, 1, hair); P(13, 5, 1, 1, hair); P(5, 2, 3, 1, Shade(hair, 30)); if (handler) { P(4, 4, 1, 3, hairD); P(14, 4, 1, 3, hairD); } }
    if (style == 2) { P(5, 4, 1, 1, hair); P(13, 4, 1, 2, hair); P(6, 2, 3, 1, Shade(hair, 30)); }
    if (style == 4) { P(5, 1, 2, 1, hair); P(11, 1, 2, 1, hair); P(6, 4, 2, 1, hair); P(12, 4, 1, 2, Color{ 150, 150, 156, 255 }); P(4, 2, 1, 4, hair); }
    if (style == 3) { P(5, 2, 9, 1, Color{ 70, 16, 20, 255 }); P(5, 0, 9, 1, Color{ 52, 52, 60, 255 }); P(3, 3, 13, 1, Color{ 40, 40, 48, 255 }); P(5, 5, 9, 3, Mix(skin, kInk, 0.68f)); P(5, 8, 9, 1, Mix(skin, kInk, 0.3f)); }   // the band, and the brim's shadow
    if (style >= 3) { P(6, 9, 1, 2, skinD); P(12, 9, 1, 2, skinD); }                                   // hollow cheeks
    if (style == 4 && e != LX_SMILE) for (int k = 0; k < 6; ++k) P(6 + k * 1.3f, 11 + (k % 2), 1, 1, Fade(Color{ 60, 56, 60, 255 }, 0.45f));   // stubble
    // ---- the face
    float t = globalT + seed;
    bool blink = blinkOK && e != LX_STARE && std::fmod(t, 3.7f) < 0.13f;
    bool hidden = style == 3 && story.Stage() < 2;      // under the brim you don't see his eyes. Not at first
    Color brow = style == 1 ? hairD : style >= 3 ? Color{ 50, 40, 36, 255 } : hairD;
    if (style == 3) brow = Mix(skin, kInk, 0.75f);
    // brows
    switch (e) {
    case LX_COLD: P(6, 5, 1, 1, brow); P(7, 6, 2, 1, brow); P(10, 6, 2, 1, brow); P(12, 5, 1, 1, brow); break;
    case LX_SCARED: P(7, 5, 2, 1, brow); P(10, 5, 2, 1, brow); break;
    case LX_WORRIED: case LX_BROKEN: P(6, 6, 1, 1, brow); P(7, 5, 2, 1, brow); P(10, 5, 2, 1, brow); P(12, 6, 1, 1, brow); break;
    default: if (style != 3) { P(6, 6, 3, 1, brow); P(10, 6, 3, 1, brow); } break;
    }
    // eyes
    if (hidden) {
        if (story.Stage() >= 1 && std::fmod(t, 5.0f) < 2.5f) { P(7, 7, 1, 1, Fade(Color{ 200, 200, 214, 255 }, 0.45f)); P(11, 7, 1, 1, Fade(Color{ 200, 200, 214, 255 }, 0.45f)); }
    } else if (style == 3) {
        Color g = story.Stage() >= 3 ? Color{ 214, 214, 226, 255 } : Color{ 170, 170, 186, 255 };
        if (!blink) { P(7, 7, 1, 1, g); P(11, 7, 1, 1, g); }
    } else if (blink) { P(7, 8, 2, 1, skinD); P(10, 8, 2, 1, skinD); }
    else switch (e) {
    case LX_COLD: P(6, 7, 3, 1, skinD); P(10, 7, 3, 1, skinD); P(7, 8, 2, 1, eyeC); P(10, 8, 2, 1, eyeC); break;
    case LX_SCARED: P(7, 7, 2, 2, Color{ 246, 244, 240, 255 }); P(10, 7, 2, 2, Color{ 246, 244, 240, 255 }); P(7, 8, 1, 1, eyeC); P(11, 8, 1, 1, eyeC); P(14, 4, 1, 2, Color{ 170, 210, 240, 255 }); break;
    case LX_BROKEN: P(7, 7, 2, 1, skinD); P(10, 7, 2, 1, skinD); P(7, 8, 2, 1, eyeC); P(10, 8, 2, 1, eyeC); P(7, 9, 1, 2, Color{ 150, 196, 236, 255 }); break;
    case LX_STARE: { float g = 0.5f + 0.5f * std::sin(globalT * 2.2f); P(7, 7, 1, 2, eyeC); P(11, 7, 1, 2, eyeC); P(7, 7, 1, 1, Fade(Color{ 255, 80, 80, 255 }, g)); P(11, 7, 1, 1, Fade(Color{ 255, 80, 80, 255 }, g)); P(7, 9, 1, 1, Color{ 160, 130, 150, 255 }); P(11, 9, 1, 1, Color{ 160, 130, 150, 255 }); } break;
    case LX_TIRED: P(7, 7, 2, 1, skinD); P(10, 7, 2, 1, skinD); P(7, 8, 1, 1, eyeC); P(11, 8, 1, 1, eyeC); P(7, 9, 2, 1, Mix(skinD, Color{ 120, 110, 140, 255 }, 0.4f)); P(10, 9, 2, 1, Mix(skinD, Color{ 120, 110, 140, 255 }, 0.4f)); break;
    default: P(7, 7, 1, 2, eyeC); P(11, 7, 1, 2, eyeC); break;
    }
    // nose
    P(9, 9, 1, 1, skinD);
    // cheeks: only the kid, only while he's still a kid
    if (style == 0 && !story.tainted && (e == LX_SMILE || e == LX_NORMAL)) { P(6, 10, 1, 1, Color{ 240, 160, 150, 255 }); P(12, 10, 1, 1, Color{ 240, 160, 150, 255 }); }
    // mouth
    float my = style == 2 ? 11 : 11;
    Color mc = style == 2 ? Mix(hair, kInk, 0.5f) : style == 3 ? Mix(skin, kInk, 0.55f) : lip;
    bool open = talking && std::fmod(globalT, 0.24f) < 0.12f;
    if (open && style == 3) P(8, my, 3, 1, kInk);
    else if (open) { P(8, my - 0.5f, 3, 1.5f, dark); }
    else switch (e) {
    case LX_SMILE: P(7, my - 1, 1, 1, mc); P(11, my - 1, 1, 1, mc); P(8, my, 3, 1, mc); break;
    case LX_COLD: P(7, my, 5, 1, mc); break;
    case LX_SCARED: P(8, my - 1, 3, 2, dark); break;
    case LX_BROKEN: P(8, my - 1, 3, 1, mc); P(7, my, 1, 1, mc); P(11, my, 1, 1, mc); break;
    case LX_STARE: P(8, my, 3, 1, Mix(mc, kInk, 0.3f)); if (story.darkPoints >= 10) { P(7, my - 1, 1, 1, dark); P(11, my - 1, 1, 1, dark); P(8, my, 3, 1, dark); } break;
    default: P(8, my, 3, 1, mc); break;
    }
}
static LkExpr LkExprFor(const std::string& who) {
    bool hunt = story.chase || story.collection;
    if (who == "You") {
        if (story.tainted) return LX_STARE;
        if (hunt) return LX_SCARED;
        if (story.Stage() >= 2) return LX_WORRIED;
        return story.Stage() == 0 ? LX_SMILE : LX_NORMAL;
    }
    if (who == "Mom" || who == "Dad" || who == "Handler two") {
        bool mom = who != "Dad";
        Attitude a = mom ? story.momAtt : story.dadAtt;
        if (a == Attitude::Cold) return LX_COLD;
        if (a == Attitude::Scared) return LX_SCARED;
        if (a == Attitude::Broken) return LX_BROKEN;
        if (hunt) {
            bool turned = mom ? story.momTurned : story.dadTurned;
            if (story.handlers || (story.stance == Stance::Dark && !turned)) return LX_COLD;
            return LX_SCARED;
        }
        if (mom && story.Knows("the_tape")) return LX_COLD;      // you know what she is now. She looks it
        return story.Stage() <= 1 ? LX_SMILE : LX_NORMAL;      // stage 2: the smile is gone, the face is just a face
    }
    // the visitor
    if (story.remembered) return LX_BROKEN;
    if (story.visitorDowned || story.visitorTied) return LX_TIRED;
    return LX_NORMAL;
}
static bool DrawBust(const std::string& who, float cx, float cy, float px) {
    int style = who == "You" ? 0 : (who == "Mom" || who == "Handler two") ? 1 : who == "Dad" ? 2 : who == "The visitor" ? 3 : -1;
    if (style < 0) return false;
    if (style == 3 && (story.visitorDowned || story.visitorTied || story.remembered)) style = 4;   // the hat came off. Look who it is
    LkPaint p{ cx - 9.5f * px, cy - 9.0f * px, px };
    bool talking = beatActive && cur.type == BeatType::Say && cur.speaker == who && typed < (float)cur.text.size();
    LkBustDraw(p, style, LkExprFor(who), talking, true, (float)style * 1.3f);
    return true;
}

// ---------------------------------------------------------------- T-043: the watch goes dark with you
static Color LookLcd() {
    float d = std::min(1.0f, story.darkPoints / (float)DARK_MAX);
    Color c = Mix(Color{ 150, 178, 186, 255 }, Color{ 170, 152, 156, 255 }, d * 0.8f);
    if (story.darkPoints >= DARK_MAX) c = Color{ 160, 104, 104, 255 };
    return c;
}
static void LookWatchFace(float bx, float by, float w, float bh, float lx, float ly, float lw, float lh) {
    int d = story.darkPoints;
    // the red dot in the Origin mark beats. Faster the darker you are
    if (d >= 1) {
        float rate = 1.0f + d * 0.25f, ph = std::fmod(globalT * rate, 1.0f);
        float k = ph < 0.12f ? 1 - ph / 0.12f : ph > 0.25f && ph < 0.37f ? 0.6f * (1 - (ph - 0.25f) / 0.12f) : 0;
        if (k > 0) DrawCircleV({ (bx + 7.5f) * US, (by + 4.8f) * US }, (1.0f + 0.9f * k) * US, Fade(Color{ 255, 90, 70, 255 }, 0.8f * k));
    }
    // the glass cracks, a little more every few points
    Color crack = Fade(WHITE, 0.6f), cs = Fade(Color{ 20, 26, 34, 255 }, 0.45f);
    auto L = [&](float x0, float y0, float x1, float y1) {
        DrawLineEx({ x0 * US, y0 * US + 1 }, { x1 * US, y1 * US + 1 }, std::max(1.0f, US * 0.5f), cs);
        DrawLineEx({ x0 * US, y0 * US }, { x1 * US, y1 * US }, std::max(1.0f, US * 0.5f), crack);
    };
    if (d >= 4) { L(lx + lw, ly + 2, lx + lw - 6, ly + 5); L(lx + lw - 6, ly + 5, lx + lw - 9, ly + lh * 0.6f); L(lx + lw - 6, ly + 5, lx + lw - 2, ly + lh - 1); }
    if (d >= 8) { L(lx, ly + lh - 3, lx + 7, ly + lh - 6); L(lx + 7, ly + lh - 6, lx + 12, ly + lh); L(lx + 7, ly + lh - 6, lx + 9, ly + 2); }
    if (d >= 12) {
        float sx = lx + lw * 0.55f, sy = ly + lh * 0.45f;
        for (int k = 0; k < 6; ++k) { float a = k * 1.05f + 0.3f; L(sx, sy, sx + std::cos(a) * 9, sy + std::sin(a) * 6); }
        DrawCircleLines((int)(sx * US), (int)(sy * US), 2.5f * US, crack);
    }
}

// ---------------------------------------------------------------- T-048: the HUD, quieter
// The watch carries the time (and the loop, on its strap). Memory and hands sit under it, small.
// The chores live on the fridge: a list you see in the kitchen.
static void LkChoreList(float x, float y, float alpha) {
    const float w = 92, h = 64;
    float tilt = -0.0f;
    (void)tilt;
    DrawRectangle((int)((x + 1.5f) * US), (int)((y + 2) * US), (int)(w * US), (int)(h * US), Fade(BLACK, 0.3f * alpha));
    DrawRectangle((int)(x * US), (int)(y * US), (int)(w * US), (int)(h * US), Fade(Color{ 252, 238, 150, 255 }, alpha));
    DrawRectangle((int)(x * US), (int)(y * US), (int)(w * US), (int)(5 * US), Fade(Color{ 244, 222, 118, 255 }, alpha));
    DrawCircleV({ (x + w / 2) * US, (y + 2.5f) * US }, 2.2f * US, Fade(Color{ 220, 60, 60, 255 }, alpha));   // the magnet
    Color ink = Fade(Color{ 40, 50, 110, 255 }, alpha), done = Fade(Color{ 120, 120, 120, 255 }, alpha);
    UIText("CHORES", x + 6, y + 6, 7, ink);
    struct C { const char* t; bool d; } cs[5] = { { "Feed Doggo", story.fed }, { "Trash out", story.trashDone }, { "Dishes", story.dishesDone }, { "Stack the firewood", story.firewoodDone }, { "Clean your room", story.roomClean } };
    for (int i = 0; i < 5; ++i) {
        float ly = y + 16 + i * 8.5f;
        DrawRectangleLines((int)((x + 6) * US), (int)((ly + 0.5f) * US), (int)(5 * US), (int)(5 * US), ink);
        UIText(cs[i].t, x + 14, ly, 6, cs[i].d ? done : ink);
        if (cs[i].d) {
            DrawLineEx({ (x + 6.5f) * US, (ly + 3) * US }, { (x + 8.5f) * US, (ly + 5) * US }, std::max(1.0f, US * 0.7f), Fade(Color{ 200, 50, 50, 255 }, alpha));
            DrawLineEx({ (x + 8.5f) * US, (ly + 5) * US }, { (x + 11.5f) * US, (ly - 0.5f) * US }, std::max(1.0f, US * 0.7f), Fade(Color{ 200, 50, 50, 255 }, alpha));
            DrawRectangle((int)((x + 14) * US), (int)((ly + 3.2f) * US), (int)(UIMeasure(cs[i].t, 6) * US), (int)std::max(1.0f, US * 0.5f), done);
        }
    }
    const char* foot = story.earnedPresent ? "All done. PRESENT!" : "All by 16:40 = a present";
    UIText(foot, x + 6, y + h - 7.5f, 5, story.earnedPresent ? Fade(Color{ 200, 50, 50, 255 }, alpha) : Fade(Color{ 90, 90, 120, 255 }, alpha));
}
static void LookHUD(float watchBottom) {
    // the loop, small, on the watch strap
    std::string lp = "LOOP " + std::to_string(story.loop);
    float rx = VW - 8;
    UIText(lp.c_str(), rx - UIMeasure(lp.c_str(), 5) - 2, watchBottom + 1, 5, Fade(WHITE, 0.45f));
    // memory and hands: small, under the watch, on a soft dark backing so they read over snow and floor alike
    std::string mem = "MEMORY " + std::to_string(story.Awareness()) + "/" + std::to_string(AllMemories().size());
    std::string hand = story.held == Item::None ? "" : std::string("IN HANDS ") + ItemName(story.held);
    if (story.doggoStay && story.DoggoExists()) hand += hand.empty() ? "DOGGO STAYING" : "  DOGGO STAYING";
    float y = watchBottom + 9;
    float wMem = UIMeasure(mem.c_str(), 5.5f), wHand = hand.empty() ? 0 : UIMeasure(hand.c_str(), 5.5f);
    float bw = std::max(wMem, wHand) + 10, bh = hand.empty() ? 9 : 17;
    DrawRectangleGradientH((int)((rx - bw) * US), (int)((y - 1.5f) * US), (int)(bw * US), (int)(bh * US), Fade(BLACK, 0), Fade(BLACK, 0.45f));
    UIText(mem.c_str(), rx - wMem - 2, y, 5.5f, Fade(Color{ 200, 170, 255, 255 }, 0.75f));
    if (!hand.empty()) {
        Color hc = story.hasKnife ? Color{ 235, 90, 90, 255 } : story.hasKey ? GOLD : Color{ 210, 230, 250, 255 };
        UIText(hand.c_str(), rx - wHand - 2, y + 8, 5.5f, Fade(hc, 0.8f));
    }
    // the fridge list, in the kitchen
    Room r = story.world.RoomAt(TX(player.pos.x), TX(player.pos.y));
    if (r == Room::Kitchen && !story.collection && !(beatActive && cur.type != BeatType::Say)) {
        bool near = std::hypot(player.pos.x - (9 * TS + 8), player.pos.y - (13 * TS + 8)) < 3.2f * TS;
        static float a = 0;
        a += ((near ? 1.0f : 0.55f) - a) * std::min(1.0f, GetFrameTime() * 6);
        LkChoreList(8, 36, a);
    }
}

// ---------------------------------------------------------------- T-049: handwriting in the memory log
// Before you know anything, you write like a twelve year old. Later the narrator writes for you.
// On the dark path the old entries go cold. Only what you found this hour is still warm.
static void LookJournalText(const std::string& line, float x, float y, int lineNo, bool adult, bool newThisLoop) {
    if (adult && hasNarratorFont) {
        Color c = story.tainted && !newThisLoop ? Color{ 150, 172, 204, 255 } : Color{ 220, 170, 170, 255 };
        DrawTextEx(narratorFont, line.c_str(), { x * US, y * US }, 11 * US, 1, c);
        return;
    }
    // the kid's hand: pencil grey-blue, a wobble per word, letters that don't quite sit on the line
    Color pencil = story.tainted && !newThisLoop ? Color{ 150, 170, 200, 255 } : Color{ 214, 214, 232, 255 };
    float cx = x;
    size_t i = 0;
    int w = 0;
    while (i < line.size()) {
        size_t j = line.find(' ', i);
        if (j == std::string::npos) j = line.size();
        std::string word = line.substr(i, j - i);
        unsigned h = Hash(lineNo * 31 + w, (int)word.size() + 77);
        float dy = ((int)(h % 3) - 1) * 0.45f;
        UIText(word.c_str(), cx, y + dy, 8, pencil);
        cx += UIMeasure(word.c_str(), 8) + UIMeasure(" ", 8);
        i = j + 1; ++w;
    }
}
static bool LookNewThisLoop(const std::string& title) {
    for (auto& id : story.loopNew) for (auto& e : AllMemories()) if (e.id == id && e.title == title) return true;
    return false;
}

// ---------------------------------------------------------------- T-046: close-ups
// A full screen picture while the lines that go with it play. Started by the presentation when you use
// the mirror, the wall of screens or the chair, or play the tape. Gone when the beats move on.
static std::string lkClose;
static void LookOnInteract(const std::string& id) {
    if (id == "mirror" || id == "screens" || id == "chair") lkClose = id;
    if (id == "vt_play") lkClose = "tape";
}
static void LookCloseTick() {
    if (lkClose.empty()) return;
    if (!beatActive && story.beats.empty()) { lkClose.clear(); return; }
    if (beatActive) {
        BeatType t = cur.type;
        bool keep = t == BeatType::Say || t == BeatType::Narrate || t == BeatType::Black || t == BeatType::Clue || t == BeatType::Achievement || t == BeatType::Sound || t == BeatType::Choice || t == BeatType::Bubble ||
                    (t == BeatType::Fade && cur.minutes == 0 && cur.text.empty());
        if (!keep) lkClose.clear();
    }
}
// shared bits, on a grid of 4 UI units per cell, 80 x 42 cells, at (80, 8)
static const float LKC_X = 80, LKC_Y = 8, LKC_CELL = 4;
static void LkFrame(const LkPaint& p) {
    p.R(-1, -1, 82, 44, Color{ 230, 230, 240, 255 });
    p.R(-0.5f, -0.5f, 81, 43, kInk);
}
static void LkMirror(const LkPaint& p) {
    // bathroom tiles
    p.R(0, 0, 80, 42, Color{ 150, 190, 186, 255 });
    for (int x = 0; x < 80; x += 6) p.R((float)x, 0, 0.5f, 42, Color{ 120, 160, 158, 255 });
    for (int y = 0; y < 42; y += 6) p.R(0, (float)y, 80, 0.5f, Color{ 120, 160, 158, 255 });
    // the mirror
    p.R(21, 1, 38, 38, Color{ 110, 78, 40, 255 }); p.R(22, 2, 36, 36, Color{ 150, 112, 60, 255 }); p.R(22, 2, 36, 1, Color{ 190, 150, 86, 255 });
    Color glass = { 178, 206, 220, 255 };
    p.R(24, 4, 32, 32, glass);
    // what's in it
    int st = story.Stage();
    LkPaint q{ p.ox + (40.0f - 9.5f * 1.6f) * p.c, p.oy + 5.6f * p.c, p.c * 1.6f };
    if (story.shift == Shift::Sister) {
        p.R(24, 4, 32, 32, Color{ 226, 234, 238, 255 });
        // somebody drew a smiley in the steam
        for (int k = 0; k < 24; ++k) { float a = k * 0.2618f; p.R(40 + std::cos(a) * 9, 20 + std::sin(a) * 9, 1, 1, Color{ 160, 190, 206, 255 }); }
        p.R(36, 16, 1.5f, 2, Color{ 160, 190, 206, 255 }); p.R(43, 16, 1.5f, 2, Color{ 160, 190, 206, 255 });
        for (int k = 0; k < 7; ++k) p.R(36 + k, 23 + (k == 0 || k == 6 ? 0 : k == 1 || k == 5 ? 1 : 1.5f), 1, 1, Color{ 160, 190, 206, 255 });
        for (int k = 0; k < 4; ++k) p.R(37 + k * 2.5f, 32 + (k % 2), 0.6f, 3, Color{ 190, 214, 226, 255 });   // drips
    } else if (st >= 3) {
        LkBustDraw(q, 4, LX_SMILE, st >= 4 && std::fmod(globalT, 3.0f) < 1.4f, false, 0);   // older. Grown up. Smiling
    } else {
        if (st == 2) { LkPaint g = q; g.ox += 0.6f * p.c; g.a = 0.35f; LkBustDraw(g, 0, LX_NORMAL, false, false, 0); }   // a step behind you
        LkBustDraw(q, 0, st == 0 ? LX_SMILE : LX_NORMAL, false, st != 2 || std::fmod(globalT, 3.7f) > 0.5f, st == 2 ? 0.35f : 0);
        if (st == 0) { p.R(26, 28, 6, 8, Color{ 200, 50, 50, 255 }); p.R(27, 26, 4, 3, Color{ 242, 204, 174, 255 }); p.R(26.5f, 28, 5, 2.5f, Color{ 20, 26, 34, 255 }); p.R(27.5f, 28.5f, 3, 1.5f, Color{ 150, 178, 186, 255 }); }   // the watch, held up. Looking cool
    }
    // the glass
    for (int k = 0; k < 6; ++k) p.R(26 + k, 6 + k * 2, 1, 3, Fade(WHITE, 0.35f));
    p.R(50, 6, 4, 1, Fade(WHITE, 0.25f));
    p.R(24, 4, 32, 32, Fade(Color{ 200, 230, 240, 255 }, 0.08f));
    // the shelf, the soap, the toothbrush
    p.R(20, 38, 40, 3, Color{ 234, 236, 238, 255 }); p.R(20, 40, 40, 1, Color{ 170, 176, 186, 255 });
    p.R(25, 36, 4, 2, Color{ 110, 190, 150, 255 }); p.R(52, 33, 1, 5, Color{ 230, 90, 100, 255 }); p.R(52, 32, 1, 1, WHITE);
}
static void LkScreens(const LkPaint& p) {
    p.R(0, 0, 80, 42, Color{ 18, 20, 28, 255 });
    for (int y = 0; y < 42; y += 5) p.R(0, (float)y, 80, 0.3f, Color{ 26, 28, 38, 255 });
    bool dead = story.chase;
    const char* lab[12] = { "BED", "BATH", "KITCHEN", "LIVING", "HALL", "YARD", "SHED", "GATE", "M+D", "BACK", "DOOR", "S06" };
    for (int i = 0; i < 12; ++i) {
        int cx = 3 + (i % 4) * 19, cy = 2 + (i / 4) * 13;
        p.R((float)cx - 1, (float)cy - 1, 18, 12, Color{ 40, 42, 52, 255 });
        p.R((float)cx, (float)cy, 16, 10, Color{ 10, 12, 16, 255 });
        if (i == 11) {
            // the little one in the corner, on its battery: a watch, counting
            p.R((float)cx + 1, (float)cy + 1, 14, 8, Color{ 150, 178, 186, 255 });
            std::string clk = story.ClockText();
            UIText(clk.c_str(), (p.ox / US) + (cx + 2.5f) * (p.c / US), (p.oy / US) + (cy + 2.2f) * (p.c / US), 12, Color{ 22, 32, 44, 255 });
            continue;
        }
        if (dead) continue;
        Color bg = { 60, 100, 150, 255 };
        p.R((float)cx, (float)cy, 16, 10, bg);
        auto S = [&](float x, float y, float w, float h, Color c) { p.R(cx + x, cy + y, w, h, c); };
        switch (i) {
        case 0: S(3, 3, 6, 6, Color{ 70, 110, 180, 255 }); S(3, 3, 6, 2, Color{ 230, 230, 240, 255 }); S(11, 2, 3, 6, Color{ 130, 88, 56, 255 }); break;     // your bed
        case 1: S(6, 3, 4, 5, Color{ 236, 240, 244, 255 }); S(6, 2, 4, 1, Color{ 200, 206, 214, 255 }); break;                                                  // the toilet
        case 2: S(2, 2, 4, 6, Color{ 226, 230, 236, 255 }); S(8, 4, 6, 4, Color{ 196, 200, 206, 255 }); S(6, 5, 2, 4, Color{ 66, 126, 92, 255 }); S(6, 4, 2, 1, Color{ 186, 134, 62, 255 }); break;   // Mom at the sink
        case 3: S(9, 2, 5, 7, Color{ 40, 100, 70, 255 }); S(11, 1, 1, 1, Color{ 255, 226, 110, 255 }); S(2, 5, 6, 3, Color{ 66, 110, 116, 255 }); S(4, 3, 2, 3, Color{ 56, 86, 154, 255 }); break;  // the tree, Dad on the couch
        case 4: S(0, 4, 16, 2, Color{ 40, 96, 66, 255 }); S(2, 1, 2, 7, Color{ 120, 84, 52, 255 }); break;
        case 5: S(0, 0, 16, 10, Color{ 200, 210, 230, 255 }); S(5, 3, 3, 3, Color{ 120, 70, 50, 255 }); S(11, 6, 2, 3, Color{ 60, 50, 50, 255 }); break;   // the yard, the doghouse
        case 6: S(3, 3, 10, 5, Color{ 90, 70, 50, 255 }); S(4, 4, 3, 2, Color{ 140, 140, 150, 255 }); break;
        case 7: S(0, 0, 16, 10, Color{ 196, 206, 226, 255 }); S(5, 0, 6, 10, Color{ 150, 150, 160, 255 }); if (story.CarHere()) { S(6, 4, 4, 4, Color{ 26, 28, 36, 255 }); S(6, 3, 1, 1, Color{ 255, 246, 200, 255 }); S(9, 3, 1, 1, Color{ 255, 246, 200, 255 }); } break;
        case 8: S(3, 3, 10, 6, Color{ 150, 58, 58, 255 }); S(3, 3, 10, 2, Color{ 232, 230, 222, 255 }); break;
        case 9: S(0, 0, 16, 10, Color{ 190, 200, 224, 255 }); S(6, 3, 5, 3, Color{ 220, 220, 230, 255 }); S(7, 4, 1, 1, kInk); break;   // the snow beast
        case 10: S(5, 1, 6, 9, Color{ 44, 86, 70, 255 }); S(9, 5, 1, 1, GOLD); break;
        }
        // scanlines and a roll
        for (int y = 0; y < 10; y += 2) p.R((float)cx, (float)cy + y, 16, 0.4f, Fade(BLACK, 0.25f));
        float roll = std::fmod(globalT * 3 + i * 1.7f, 12.0f);
        if (roll < 10) p.R((float)cx, cy + roll, 16, 0.6f, Fade(WHITE, 0.12f));
        p.R((float)cx, (float)cy, 16, 10, Fade(Color{ 80, 150, 255, 255 }, 0.12f));
        UIText(lab[i], (p.ox / US) + (cx + 0.6f) * (p.c / US), (p.oy / US) + (cy + 0.4f) * (p.c / US), 4.5f, Fade(WHITE, 0.6f));
    }
    // the record light
    if (std::fmod(globalT, 1.2f) < 0.6f) p.R(77, 1, 1.5f, 1.5f, Color{ 230, 40, 40, 255 });
}
static void LkChair(const LkPaint& p, bool tapeLook) {
    // concrete, one bulb
    p.R(0, 0, 80, 42, Color{ 30, 32, 40, 255 });
    p.R(0, 26, 80, 16, Color{ 48, 48, 54, 255 });
    for (int k = 0; k < 14; ++k) { float w = 6 + k * 3.2f; p.R(40 - w / 2, 4 + k * 2.6f, w, 2.6f, Fade(Color{ 255, 236, 180, 255 }, 0.05f)); }   // the cone of light
    p.R(39, 0, 2, 3, Color{ 60, 60, 66, 255 }); p.R(38.5f, 3, 3, 2, Color{ 255, 236, 170, 255 });
    p.R(30, 37, 20, 3, Fade(BLACK, 0.4f));                                       // the shadow
    // the chair. Kid-sized
    Color wood = { 110, 84, 60, 255 }, woodD = { 78, 58, 42, 255 }, strap = { 70, 44, 30, 255 }, buckle = { 200, 190, 160, 255 };
    p.R(33, 10, 14, 15, kInk); p.R(34, 11, 12, 13, wood); p.R(34, 11, 12, 1, Shade(wood, 26));   // back
    p.R(31, 23, 18, 4, kInk); p.R(32, 24, 16, 2, woodD);                                            // seat
    p.R(30, 19, 3, 6, kInk); p.R(47, 19, 3, 6, kInk); p.R(29, 18, 6, 2, wood); p.R(45, 18, 6, 2, wood);   // arms
    p.R(30.5f, 18, 3, 1, Color{ 170, 140, 104, 255 }); p.R(46.5f, 18, 3, 1, Color{ 170, 140, 104, 255 });   // worn smooth where the wrists go
    for (int lx : { 32, 46 }) { p.R((float)lx, 26, 2, 11, kInk); p.R(lx + 0.5f, 26, 1, 11, woodD); }
    // the straps: wrists, chest, ankles
    p.R(28.5f, 17.5f, 7, 2.2f, strap); p.R(44.5f, 17.5f, 7, 2.2f, strap); p.R(31, 17.8f, 1.5f, 1.5f, buckle); p.R(47.5f, 17.8f, 1.5f, 1.5f, buckle);
    p.R(33, 15, 14, 2, strap); p.R(39, 15, 2, 2, buckle);
    p.R(31, 32, 4, 1.6f, strap); p.R(45, 32, 4, 1.6f, strap);
    p.R(30, 33.6f, 1, 3, strap); p.R(49, 33.6f, 1, 3, strap);                 // a strap hangs loose
    // the old TV on its cart, turned to face it
    if (!tapeLook) { p.R(6, 16, 14, 11, kInk); p.R(7, 17, 12, 9, Color{ 40, 40, 46, 255 }); p.R(8, 18, 10, 7, Color{ 20, 24, 30, 255 }); p.R(9, 19, 3, 1, Fade(WHITE, 0.15f)); p.R(8, 27, 10, 9, Color{ 70, 70, 76, 255 }); p.R(9, 29, 8, 2, Color{ 26, 26, 30, 255 }); }
    // a drain in the floor
    p.R(60, 34, 6, 2, Color{ 30, 30, 34, 255 }); for (int k = 0; k < 3; ++k) p.R(60.5f + k * 2, 34.4f, 1, 1.2f, Color{ 60, 60, 66, 255 });
}
static void LkTape(const LkPaint& p) {
    // the TV itself
    p.R(0, 0, 80, 42, Color{ 26, 24, 28, 255 });
    p.R(4, 2, 62, 38, Color{ 52, 50, 56, 255 }); p.R(4, 2, 62, 1, Color{ 80, 78, 86, 255 });
    p.R(68, 6, 8, 3, Color{ 40, 40, 44, 255 }); p.R(70, 12, 4, 4, Color{ 70, 70, 76, 255 }); p.R(70, 18, 4, 4, Color{ 70, 70, 76, 255 });
    LkPaint s{ p.ox + 7 * p.c, p.oy + 5 * p.c, p.c * 0.7f };                     // the picture, 80 x 42 at 0.7
    // what's on it: the chair, from the camera in the corner. Washed out
    LkChair(s, true);
    const std::string& tx = cur.text;
    bool mom = tx.find("Mom walks") != std::string::npos || tx.find("Drink up") != std::string::npos || tx.find("drinks") != std::string::npos || tx.find("smiling") != std::string::npos;
    bool smile = tx.find("smil") != std::string::npos;
    // the boy, six maybe, in the chair. The watch much too big
    s.R(36, 14, 8, 10, Color{ 200, 60, 60, 255 }); s.R(37, 7, 6, 7, Color{ 236, 200, 172, 255 }); s.R(36.5f, 6, 7, 2.5f, Color{ 96, 62, 32, 255 });
    s.R(38, 10, 1, 1, kInk); s.R(41, 10, 1, 1, kInk);
    if (smile) { s.R(38, 12, 1, 1, kInk); s.R(41, 12, 1, 1, kInk); s.R(39, 13, 2, 1, kInk); } else s.R(38.5f, 12.5f, 3, 1.5f, Color{ 70, 22, 30, 255 });
    s.R(31, 18, 3, 3, Color{ 20, 26, 34, 255 }); s.R(31.5f, 18.5f, 2, 2, Color{ 150, 178, 186, 255 });
    s.R(37, 24, 2, 6, Color{ 50, 60, 110, 255 }); s.R(41, 24, 2, 6, Color{ 50, 60, 110, 255 });
    if (mom) {
        // she walks into the picture with a glass of juice. A straw in it
        s.R(52, 8, 8, 20, Color{ 66, 126, 92, 255 }); s.R(53, 2, 6, 6, Color{ 238, 198, 168, 255 }); s.R(52, 1, 8, 3, Color{ 186, 134, 62, 255 }); s.R(51, 3, 2, 9, Color{ 186, 134, 62, 255 }); s.R(59, 3, 2, 9, Color{ 186, 134, 62, 255 });
        s.R(54, 6, 4, 1, Color{ 140, 60, 60, 255 }); s.R(53, 10, 6, 10, Color{ 238, 232, 216, 255 });
        s.R(47, 14, 3, 4, Color{ 250, 170, 60, 255 }); s.R(48, 11, 0.6f, 4, WHITE); s.R(50, 14, 2, 2, Color{ 238, 198, 168, 255 });
        s.R(52, 28, 3, 6, Color{ 84, 66, 60, 255 }); s.R(57, 28, 3, 6, Color{ 84, 66, 60, 255 });
    }
    // VHS: washed out, noise, a band rolling down, the counter in the corner
    s.R(0, 0, 80, 42, Fade(Color{ 120, 140, 160, 255 }, 0.28f));
    for (int k = 0; k < 140; ++k) { unsigned h = Hash(k, (int)(globalT * 24)); s.R((float)(h % 80), (float)((h >> 8) % 42), 1, 0.5f, Fade(WHITE, 0.08f + (h >> 20) % 10 * 0.02f)); }
    float band = std::fmod(globalT * 6, 52.0f) - 5;
    s.R(0, band, 80, 3, Fade(WHITE, 0.12f)); s.R(0, band + 3, 80, 1, Fade(BLACK, 0.2f));
    for (int y = 0; y < 42; y += 2) s.R(0, (float)y, 80, 0.5f, Fade(BLACK, 0.18f));
    int secs = 41 * 60 + (int)(globalT * 1) % 60;
    char tc[32]; std::snprintf(tc, sizeof tc, "PLAY  00:%02d:%02d", secs / 60, secs % 60);
    UIText(tc, s.ox / US + 3 * s.c / US, s.oy / US + 2 * s.c / US, 7, Fade(WHITE, 0.8f));
    UIText("S06  W312", s.ox / US + 3 * s.c / US, s.oy / US + 37 * s.c / US, 7, Fade(WHITE, 0.6f));
    // the curve of the glass
    p.R(7, 5, 56, 1, Fade(WHITE, 0.06f));
}
// draws the close-up. For Black beats it also draws the words (under the picture) and returns true
static bool LookCloseUp() {
    LookCloseTick();
    if (lkClose.empty() || !beatActive) return false;
    BeatType t = cur.type;
    if (t != BeatType::Say && t != BeatType::Black && t != BeatType::Narrate && t != BeatType::Choice) return false;
    DrawRectangle(0, 0, SW, SH, Color{ 6, 6, 10, 255 });
    LkPaint p = LkUI(LKC_X, LKC_Y, LKC_CELL);
    LkFrame(p);
    if (lkClose == "mirror") LkMirror(p);
    else if (lkClose == "screens") LkScreens(p);
    else if (lkClose == "chair") LkChair(p, false);
    else if (lkClose == "tape") LkTape(p);
    // dim the edges like an old photo
    for (int k = 0; k < 10; ++k) DrawRectangleLines((int)((LKC_X + k) * US), (int)((LKC_Y + k) * US), (int)((320 - 2 * k) * US), (int)((168 - 2 * k) * US), Fade(BLACK, 0.06f * (10 - k)));
    if (t == BeatType::Black) {
        std::string vis = cur.text.substr(0, std::min<size_t>(cur.text.size(), (size_t)typed));
        auto lines = Wrap(cur.text, 400, 9, false);
        float y = 186;
        size_t used = 0;
        for (auto& l : lines) {
            std::string part = vis.size() > used ? vis.substr(used, l.size()) : "";
            used += l.size() + 1;
            UIText(part.c_str(), VW / 2.0f - UIMeasure(l.c_str(), 9) / 2, y, 9, Color{ 220, 220, 220, 255 });
            y += 12;
        }
        if (curSeen && !touchMode) UIText("TAB: skip", VW - 46, VH - 14, 6, Fade(WHITE, 0.4f));
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- T-103: a picture on every ending card
// 100 x 40 cells of 2 UI units, centred over the title
static void LkSky(const LkPaint& p, Color top, Color bottom, int stars) {
    for (int y = 0; y < 26; ++y) p.R(0, (float)y, 100, 1, Mix(top, bottom, y / 25.0f));
    for (int i = 0; i < stars; ++i) { unsigned h = Hash(i, 811); p.R((float)(h % 100), (float)((h >> 8) % 20), 0.5f, 0.5f, Fade(WHITE, 0.4f + 0.3f * std::sin(globalT * 1.3f + i))); }
    p.R(0, 26, 100, 14, Color{ 84, 100, 160, 255 }); p.R(0, 26, 100, 1, Color{ 150, 168, 220, 255 });
}
static void LkHouseSmall(const LkPaint& p, float x, float y, int lit, bool red) {   // lit: bitmask of the 3 windows
    p.R(x - 2, y - 8, 30, 8, Color{ 150, 168, 216, 255 }); p.R(x, y - 10, 26, 2, Color{ 176, 192, 232, 255 });
    p.R(x, y, 26, 12, Color{ 92, 32, 36, 255 });
    for (int k = 0; k < 3; ++k) {
        bool on = (lit >> k) & 1;
        p.R(x + 2 + k * 9, y + 3, 5, 5, Color{ 196, 196, 208, 255 });
        p.R(x + 2.5f + k * 9, y + 3.5f, 4, 4, on ? (red ? Color{ 200, 40, 40, 255 } : Color{ 255, 210, 130, 255 }) : Color{ 20, 26, 48, 255 });
    }
    p.R(x + 20, y + 5, 4, 7, Color{ 28, 58, 50, 255 });
}
static void LkMan(const LkPaint& p, float x, float y, Color c) {   // him: coat and hat, 4 x 12
    p.R(x, y + 4, 4, 8, c); p.R(x + 0.5f, y + 1, 3, 3, c); p.R(x - 1, y + 1, 6, 1, c); p.R(x + 0.5f, y - 1, 3, 2, c);
}
static void LkKid(const LkPaint& p, float x, float y, Color shirt) {   // 3 x 7
    p.R(x, y + 2, 3, 3, shirt); p.R(x + 0.5f, y, 2, 2, Color{ 242, 204, 174, 255 }); p.R(x + 0.5f, y - 0.5f, 2, 1, Color{ 96, 62, 32, 255 }); p.R(x, y + 5, 1, 2, Color{ 50, 60, 110, 255 }); p.R(x + 2, y + 5, 1, 2, Color{ 50, 60, 110, 255 });
}
static void LkAdult(const LkPaint& p, float x, float y, Color shirt, Color hair) {   // 4 x 10
    p.R(x, y + 3, 4, 5, shirt); p.R(x + 0.5f, y, 3, 3, Color{ 236, 198, 168, 255 }); p.R(x + 0.5f, y - 0.5f, 3, 1.2f, hair); p.R(x + 0.5f, y + 8, 1, 2, Color{ 62, 52, 44, 255 }); p.R(x + 2.5f, y + 8, 1, 2, Color{ 62, 52, 44, 255 });
}
static void LkCarSmall(const LkPaint& p, float x, float y, bool lights) {
    p.R(x, y + 2, 16, 4, Color{ 14, 14, 20, 255 }); p.R(x + 4, y, 8, 3, Color{ 14, 14, 20, 255 }); p.R(x + 5, y + 0.5f, 3, 2, Color{ 40, 52, 84, 255 });
    p.R(x + 2, y + 6, 2, 1, Color{ 6, 6, 10, 255 }); p.R(x + 12, y + 6, 2, 1, Color{ 6, 6, 10, 255 });
    if (lights) { p.R(x - 1, y + 3, 1.5f, 1, Color{ 255, 248, 210, 255 }); for (int k = 0; k < 8; ++k) p.R(x - 2 - k * 3, y + 2.5f - k * 0.2f, 3, 1 + k * 0.4f, Fade(Color{ 255, 244, 200, 255 }, 0.3f - k * 0.03f)); }
    p.R(x + 15.5f, y + 3, 1, 1, Color{ 220, 30, 36, 255 });
}
static void LkTable(const LkPaint& p, float x, float y, int seats, bool riskrem) {
    p.R(0, 0, 100, 40, Color{ 26, 16, 14, 255 });
    for (int k = 0; k < 9; ++k) p.R(50 - 6 - k * 4.5f, 6 + k * 2.4f, 12 + k * 9, 2.4f, Fade(Color{ 255, 200, 120, 255 }, 0.05f));   // candlelight
    p.R(x, y, 60, 12, Color{ 120, 30, 34, 255 }); p.R(x, y, 60, 1, Color{ 170, 60, 60, 255 }); p.R(x + 2, y + 12, 2, 8, Color{ 70, 44, 28, 255 }); p.R(x + 56, y + 12, 2, 8, Color{ 70, 44, 28, 255 });
    for (int k = 0; k < seats; ++k) { float px = x + 6 + k * (48.0f / std::max(1, seats - 1)); p.R(px - 2, y + 3, 5, 3, Color{ 240, 236, 226, 255 }); p.R(px - 1, y + 3.5f, 3, 1.5f, Color{ 150, 90, 60, 255 }); }
    for (int k = 0; k < 2; ++k) { float cx = x + 24 + k * 12; p.R(cx, y - 6, 1.5f, 6, Color{ 240, 236, 220, 255 }); p.R(cx, y - 8 + (std::fmod(globalT * 5 + k, 1.0f) < 0.5f ? 0 : 0.4f), 1.5f, 2, Color{ 255, 210, 110, 255 }); }
    if (riskrem) { p.R(x + 28, y + 4, 6, 3, Color{ 246, 240, 226, 255 }); p.R(x + 29, y + 3.5f, 4, 1, Color{ 200, 40, 50, 255 }); }
}
static void LkBell(const LkPaint& p, float x, float y, float swing) {
    float s = std::sin(globalT * 2.2f) * swing;
    p.R(x + 1 + s, y, 4, 1, Color{ 226, 196, 100, 255 }); p.R(x + 0.5f + s, y + 1, 5, 3, Color{ 214, 180, 80, 255 }); p.R(x + s, y + 4, 6, 1, Color{ 240, 210, 110, 255 }); p.R(x + 2.5f - s, y + 5, 1, 1, Color{ 150, 120, 50, 255 });
}
static void LookEndingArt(const std::string& id, float x, float y) {
    LkPaint p = LkUI(x, y, 2.4f);
    p.R(-1, -1, 102, 42, Color{ 90, 80, 70, 255 });
    const Color night0 = { 8, 10, 28, 255 }, night1 = { 44, 54, 104, 255 };
    const Color red = { 200, 50, 50, 255 };
    if (id == "bells" || id == "lasttoll") {
        bool toll = id == "lasttoll";
        LkTable(p, 20, 22, toll ? 4 : 3, toll);
        if (toll) { LkAdult(p, 26, 10, Color{ 66, 126, 92, 255 }, Color{ 186, 134, 62, 255 }); LkAdult(p, 70, 10, Color{ 56, 86, 154, 255 }, Color{ 110, 70, 40, 255 }); LkKid(p, 48, 13, red); }
        else { LkKid(p, 30, 13, red); LkMan(p, 66, 8, Color{ 20, 20, 24, 255 }); }
        LkBell(p, 46, 2, 1.2f); LkBell(p, 38, 4, 0.8f); LkBell(p, 54, 4, 0.8f);
    } else if (id == "monster" || id == "program") {
        bool prog = id == "program";
        if (prog) {
            p.R(0, 0, 100, 40, Color{ 18, 20, 28, 255 });
            for (int i = 0; i < 8; ++i) { float sx = 10 + (i % 4) * 21, sy = 4 + (i / 4) * 12; p.R(sx, sy, 16, 9, Color{ 40, 42, 52, 255 }); p.R(sx + 1, sy + 1, 14, 7, Color{ 6, 6, 8, 255 }); p.R(sx + 3 + i % 3, sy + 2, 4, 0.5f, Fade(WHITE, 0.3f)); p.R(sx + 7, sy + 4, 0.5f, 3, Fade(WHITE, 0.2f)); }   // broken, all of them
            if (std::fmod(globalT, 0.9f) < 0.08f) p.R(52, 16, 3, 3, Color{ 180, 220, 255, 255 });
            p.R(0, 30, 100, 10, Color{ 40, 40, 46, 255 });
            LkKid(p, 47, 24, Color{ 120, 30, 34, 255 });
        } else {
            LkSky(p, Color{ 30, 6, 10, 255 }, Color{ 90, 30, 40, 255 }, 10);
            LkHouseSmall(p, 14, 16, 0b010, true);
            LkKid(p, 66, 26, Color{ 120, 30, 34, 255 });
            p.R(69.5f, 28, 0.6f, 4, Color{ 222, 222, 232, 255 });                                   // the knife
            p.R(60, 33, 14, 1, Fade(Color{ 10, 8, 20, 255 }, 0.5f));
        }
    } else if (id == "secret") {
        LkSky(p, night0, Color{ 70, 80, 140, 255 }, 30);
        LkHouseSmall(p, 10, 16, 0b001, false);
        p.R(13, 20, 2, 3, kInk);                                                                           // you, in the window. Awake
        LkCarSmall(p, 70, 28, false); p.R(86, 31, 1, 1, Color{ 255, 40, 40, 255 });                        // driving away. Not with you
    } else if (id == "screen") {
        p.R(0, 0, 100, 40, Color{ 6, 6, 12, 255 });
        p.R(30, 6, 40, 28, Color{ 60, 60, 70, 255 }); p.R(33, 9, 34, 17, Color{ 110, 200, 140, 255 });   // the Fliptendo Fold 2
        p.R(35, 11, 8, 6, Color{ 40, 100, 60, 255 }); p.R(52, 14, 6, 10, Color{ 40, 100, 60, 255 }); p.R(44, 16, 3, 2, Color{ 250, 220, 90, 255 });
        UIText("LAST DUNGEON", (p.ox / US) + 35 * 2.4f, (p.oy / US) + 20 * 2.5f, 7, Color{ 20, 50, 30, 255 });
        p.R(36, 28, 4, 4, Color{ 40, 40, 46, 255 }); p.R(58, 28, 3, 3, Color{ 200, 60, 60, 255 }); p.R(63, 29, 3, 3, Color{ 200, 60, 60, 255 });
        for (int k = 0; k < 6; ++k) p.R(28 - k * 3, 2 + k, 1, 36 - 2 * k, Fade(Color{ 110, 200, 140, 255 }, 0.05f));
    } else if (id == "escape") {
        LkSky(p, night0, night1, 24);
        p.R(0, 30, 100, 2, Color{ 150, 150, 160, 255 });                                                  // the road over the vidda
        LkAdult(p, 30, 22, Color{ 56, 86, 154, 255 }, Color{ 110, 70, 40, 255 }); LkAdult(p, 36, 22, Color{ 66, 126, 92, 255 }, Color{ 186, 134, 62, 255 }); LkKid(p, 42, 25, red);
        LkCarSmall(p, 84, 26, true);                                                                        // 31 kilometres later, there it is
    } else if (id == "doggo") {
        LkSky(p, night0, night1, 16);
        p.R(40, 18, 18, 10, Color{ 120, 70, 50, 255 }); p.R(38, 15, 22, 4, Color{ 176, 192, 232, 255 }); p.R(46, 22, 6, 6, Color{ 20, 14, 12, 255 });   // his house, empty
        p.R(30, 31, 8, 2, Color{ 204, 62, 60, 255 }); p.R(30, 30, 8, 1, Color{ 232, 96, 90, 255 });       // his bowl, full
        p.R(64, 31, 7, 1.5f, Color{ 200, 40, 40, 255 }); p.R(67, 31, 1, 1.5f, GOLD);                      // his collar in the snow
    } else if (id == "paid") {
        p.R(0, 0, 100, 40, Color{ 12, 8, 10, 255 });
        p.R(40, 2, 20, 36, Color{ 36, 26, 22, 255 }); p.R(49.5f, 2, 1, 36, Color{ 255, 220, 160, 255 });   // a door, a crack of light
        p.R(49, 2, 2, 36, Fade(Color{ 255, 200, 120, 255 }, 0.3f));
        p.R(51, 30, 0.6f, 8, Color{ 150, 20, 26, 255 });
    } else if (id == "empty") {
        p.R(0, 0, 100, 40, Color{ 20, 16, 22, 255 });
        p.R(0, 28, 100, 12, Color{ 60, 40, 30, 255 });
        p.R(70, 8, 10, 18, Color{ 30, 60, 40, 255 }); p.R(74, 6, 2, 2, Color{ 130, 118, 70, 255 });    // the tree, dark
        p.R(30, 20, 16, 10, Color{ 66, 110, 116, 255 }); p.R(30, 18, 16, 3, Color{ 54, 92, 98, 255 });   // Dad's chair
        LkMan(p, 36, 12, Color{ 12, 12, 16, 255 });                                                        // him in it, talking to the room
        p.R(84, 22, 6, 8, Color{ 40, 40, 46, 255 }); p.R(86, 25, 2, 2, Color{ 255, 150, 60, 255 });     // the stove, still going
    } else if (id == "samehouse") {
        LkSky(p, night0, night1, 22);
        LkHouseSmall(p, 18, 16, 0b111, false);
        p.R(38.5f, 21, 3, 7, Color{ 255, 230, 160, 255 });                                                 // the door, open before you knock
        LkKid(p, 39, 22, red);
        LkMan(p, 60, 18, Color{ 10, 10, 14, 255 });                                                        // you. Coat, watch, radio
        LkCarSmall(p, 78, 28, true);
    } else if (id == "after") {
        LkSky(p, Color{ 20, 30, 70, 255 }, Color{ 90, 110, 170, 255 }, 12);
        LkHouseSmall(p, 14, 16, 0b111, false);
        LkCarSmall(p, 64, 27, true);                                                                       // early, at the gate. He never comes in
        p.R(57, 26, 1, 8, Color{ 50, 40, 44, 255 }); p.R(80, 26, 1, 8, Color{ 50, 40, 44, 255 });
        p.R(16.5f, 19.5f, 1.5f, 2.5f, red);                                                                // you, in the window
        LkMan(p, 71, 22, Fade(Color{ 10, 10, 14, 255 }, 0.0f));
    } else if (id == "handlers") {
        LkSky(p, night0, Color{ 60, 66, 100, 255 }, 18);
        p.R(10, 18, 80, 10, Color{ 70, 74, 84, 255 }); p.R(10, 17, 80, 1, Color{ 176, 192, 232, 255 });   // Finse. A long low building
        for (int k = 0; k < 8; ++k) p.R(14 + k * 9.5f, 21, 4, 3, k == 5 ? Color{ 255, 230, 160, 255 } : Color{ 30, 34, 50, 255 });
        LkAdult(p, 84, 25, Color{ 66, 126, 92, 255 }, Color{ 186, 134, 62, 255 }); LkAdult(p, 89, 25, Color{ 56, 86, 154, 255 }, Color{ 110, 70, 40, 255 });   // walked round the back
        LkMan(p, 78, 23, Color{ 14, 14, 18, 255 });
    } else {
        LkSky(p, night0, night1, 20);
        LkHouseSmall(p, 37, 16, 0b111, story.tainted);
    }
    // the corners darken like an old print
    for (int k = 0; k < 4; ++k) DrawRectangleLines((int)((x + k) * US), (int)((y + k) * US), (int)((240 - 2 * k) * US), (int)((96 - 2 * k) * US), Fade(BLACK, 0.12f * (4 - k)));
}
static void LookEndingCard() {
    DrawRectangle(0, 0, SW, SH, Color{ 8, 6, 10, 255 });
    float fade = std::min(1.0f, beatTime / 1.2f);
    LookEndingArt(cur.id, VW / 2.0f - 120, 12);
    DrawRectangle(0, 0, SW, SH, Fade(Color{ 8, 6, 10, 255 }, 1 - fade));
    UIText(cur.speaker.c_str(), VW / 2.0f - UIMeasure(cur.speaker.c_str(), 14) / 2, 116, 14, Color{ 200, 60, 60, 255 });
    auto lines = Wrap(cur.text, 360, 10, false);
    float y = 138;
    for (auto& l : lines) { UIText(l.c_str(), VW / 2.0f - UIMeasure(l.c_str(), 10) / 2, y, 10, WHITE); y += 14; }
}
