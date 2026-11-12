// 600 Seconds - art/Couch.inc (v0.20.1 hunt-systems)
// The couch faces the TV now (its back is toward you), so Mom and Dad can sit on it for Kaptein Fjord at 16:15
// and you see the backs of their heads over the backrest, watching. When the schedule moves them on they stand
// up and walk off as before. Drawn in code like everything else. Included after art/WorldDraw.inc;
// main.cpp declares CouchDraw / CouchSits before art/Sprites.inc so DrawObj and the y-sort can call them.

// Their TV spots in src/game/Schedule.h: Mom (20, 16), Dad (22, 16), both facing up. Sitting = they got there
static bool CouchSits(const Actor& a) {
    if (!a.visible || story.collection) return false;
    bool isMom = &a == &mom;
    if (!isMom && &a != &dad) return false;
    if (isMom && story.momOnFloor) return false;
    if (!isMom && story.dadOnFloor) return false;
    sixhundred::NpcState n = isMom ? story.Mom() : story.Dad();
    int spot = isMom ? 20 : 22;
    if (n.x != spot || n.y != 16 || n.face != 1) return false;
    Vector2 c = TileCenter(spot, 16);
    return std::hypot(a.pos.x - c.x, a.pos.y - c.y) < 3.0f;
}

// x, y: the couch object's tile corner (o.x * TS, o.y * TS), as in DrawObj. It's 40 wide, centred on x + 8
static void CouchDraw(float x, float y) {
    auto R = [&](float ox, float oy, float w, float h, Color c) { DrawRectangle((int)(x + ox), (int)(y + oy), (int)w, (int)h, c); };
    auto B = [&](float ox, float oy, float w, float h, Color c) {
        R(ox - 1, oy - 1, w + 2, h + 2, Mix(c, kInk, 0.72f));
        R(ox, oy, w, h, c);
        R(ox, oy, w, 1, Shade(c, 24)); R(ox, oy + 1, 1, h - 1, Shade(c, 10));
        R(ox + w - 1, oy + 1, 1, h - 1, Shade(c, -20)); R(ox, oy + h - 1, w, 1, Shade(c, -28));
    };
    Color c = { 66, 110, 116, 255 };
    // the seat, seen from above and behind: three cushions, the pillow in the left corner
    B(-12, -6, 40, 11, Shade(c, -4));
    for (int k = 0; k < 3; ++k) { R(-6 + k * 10, -5, 9, 7, Shade(c, 8)); R(-6 + k * 10, -5, 9, 1, Shade(c, 26)); R(2 + k * 10, -4, 1, 6, Shade(c, -22)); }
    R(-7, -5, 6, 5, Color{ 220, 198, 150, 255 }); R(-7, -5, 6, 1, Color{ 240, 222, 180, 255 }); R(-5, -3, 3, 1, Color{ 190, 80, 70, 255 });   // pillow
    // whoever is sitting, between the seat and the backrest. Facing the TV, so their backs are to you
    auto sit = [&](const Actor& a, Look l, float sx) {
        Actor s = a;
        s.pos = { x + sx, y + 2.0f };
        s.dir = 1; s.anim = 0;
        DrawPerson(s, l);
    };
    if (CouchSits(mom)) sit(mom, LookMom(), -1);
    if (CouchSits(dad)) sit(dad, LookDad(), 18);
    // the backrest, toward you, with the red wool blanket over it
    B(-12, 3, 40, 10, Shade(c, -14));
    R(-11, 4, 38, 1, Shade(c, 4));
    for (int k = 0; k < 3; ++k) R(-2 + k * 10, 6, 1, 5, Shade(c, -30));
    R(13, 2, 10, 10, Color{ 168, 50, 52, 255 }); for (int k = 0; k < 3; ++k) R(13, 4 + k * 3, 10, 1, Color{ 232, 222, 200, 255 });
    R(13, 2, 10, 1, Color{ 196, 80, 80, 255 });
    // the arms, front to back
    B(-13, -7, 5, 20, Shade(c, -6)); B(24, -7, 5, 20, Shade(c, -6));
}
