// 600 Seconds - art/RumorCard.inc (v0.20.1 hunt-systems)
// Open a card on the rumor map (the last page of the memory log) and read the memory on it.
//   Keyboard: E, ENTER or SPACE opens the card nearest the top of the page. Then the arrows (or WASD) move
//   between found cards, and E / ENTER / SPACE / ESC close it again. While a card is open the arrows don't
//   turn pages, so LEFT/RIGHT stay "page" and UP/DOWN stay "pan" the rest of the time, like every other page.
//   Mouse: click a card. Phone: tap a card. Tap anywhere else closes it (and with no card open, the log).
// Included in main.cpp right after game/Links.h. DrawRumors() fills rumorHits and calls DrawRumorCard().

static std::string rumorSel;                 // the open card ("" = none)
struct RumorHit { std::string id; float x, y, w, h; };
static std::vector<RumorHit> rumorHits;      // found cards as they were drawn last frame (UI units)
static int rumorVisibleRows = 6;             // set by DrawRumors
static double rumorSeenAt = -1;              // last frame the rumor page asked for keys. Closing the log closes the card

static int RumorTab() { return JournalTabs() - 1; }   // Rumors is the last page
static const RumorNode* RumorNodeOf(const std::string& id) {
    for (auto& r : AllRumorNodes()) if (id == r.id) return &r;
    return nullptr;
}
static void RumorScrollTo(const RumorNode* r) {
    if (!r) return;
    if (r->row < journalScroll) journalScroll = r->row;
    if (r->row >= journalScroll + rumorVisibleRows) journalScroll = r->row - rumorVisibleRows + 1;
}
static void RumorOpenFirst() {
    // the found card nearest the top left of what you're looking at
    const RumorNode* best = nullptr;
    auto key = [&](const RumorNode& r) { return (r.row < journalScroll ? 1000 : 0) + r.row * 10 + r.col; };
    for (auto& r : AllRumorNodes()) if (story.Knows(r.id) && (!best || key(r) < key(*best))) best = &r;
    if (best) { rumorSel = best->id; RumorScrollTo(best); }
}
static void RumorMove(int dx, int dy) {
    const RumorNode* cur = RumorNodeOf(rumorSel);
    if (!cur) return;
    const RumorNode* best = nullptr; float bestD = 1e9f;
    for (auto& r : AllRumorNodes()) {
        if (&r == cur || !story.Knows(r.id)) continue;
        float ex = (r.col - cur->col) * 92.0f, ey = (r.row - cur->row) * 30.0f;   // the card grid in UI units
        float along = ex * dx + ey * dy, across = std::fabs(ex * dy) + std::fabs(ey * dx);
        if (along <= 0) continue;
        float d = along + across * 2.0f;
        if (d < bestD) { bestD = d; best = &r; }
    }
    if (best) { rumorSel = best->id; RumorScrollTo(best); }
}
static const RumorHit* RumorHitAt(Vector2 p) {
    for (auto& h : rumorHits) if (p.x >= h.x && p.x <= h.x + h.w && p.y >= h.y && p.y <= h.y + h.h) return &h;
    return nullptr;
}
// Phone: a tap on the rumor page. True if it was for a card (open it, or close the open one)
static bool RumorTap(Vector2 p) {
    if (journalTopic != RumorTab()) return false;
    if (const RumorHit* h = RumorHitAt(p)) { rumorSel = h->id; return true; }
    if (!rumorSel.empty()) { rumorSel.clear(); return true; }
    return false;
}
// Keys while the log is open. True if the rumor page used them (then the log doesn't page or pan this frame)
static bool RumorKeys() {
    double now = GetTime();
    if (now - rumorSeenAt > 0.25) rumorSel.clear();
    rumorSeenAt = now;
    if (journalTopic != RumorTab()) { rumorSel.clear(); return false; }
    if (!rumorSel.empty() && !story.Knows(rumorSel)) rumorSel.clear();
    if (!touchMode && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        float ox = (SW - VW * US) / 2, oy = (SH - VH * US) / 2;
        Vector2 m = GetMousePosition();
        Vector2 p = { (m.x - ox) / US, (m.y - oy) / US };
        if (const RumorHit* h = RumorHitAt(p)) { rumorSel = h->id; return true; }
        if (!rumorSel.empty()) { rumorSel.clear(); return true; }
    }
    bool toggle = IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    if (rumorSel.empty()) {
        if (toggle) { RumorOpenFirst(); return true; }
        return false;
    }
    if (toggle || IsKeyPressed(KEY_ESCAPE)) { rumorSel.clear(); return true; }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) RumorMove(1, 0);
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) RumorMove(-1, 0);
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) RumorMove(0, 1);
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) RumorMove(0, -1);
    GetMouseWheelMove();   // eaten: the page stays where the card is
    return true;
}
// The memory on the open card, in a box over the map. At the bottom, or at the top if the card is down there
static void DrawRumorCard(float top, float bottom, float cardY) {
    if (rumorSel.empty()) return;
    const MemoryEntry* m = nullptr;
    for (auto& e : AllMemories()) if (e.id == rumorSel) m = &e;
    if (!m) return;
    bool adult = story.Stage() >= 2;
    const float w = VW - 64, x = 32;
    auto lines = Wrap(m->text, w - 16, adult ? 11 : 8, adult);
    float lineH = adult ? 12.0f : 10.0f;
    int nl = std::min<int>((int)lines.size(), 6);
    float h = 26 + nl * lineH + 4;
    float y = cardY > (top + bottom) / 2 ? top + 2 : bottom - h - 2;
    Panel(x, y, w, h, Color{ 22, 18, 32, 250 }, Color{ 255, 210, 90, 255 });
    UIText(m->title.c_str(), x + 8, y + 5, 10, WHITE);
    std::string topic = m->topic;
    UIText(topic.c_str(), x + w - 8 - UIMeasure(topic.c_str(), 6.5f), y + 7, 6.5f, Fade(WHITE, 0.45f));
    bool fresh = std::find(story.loopNew.begin(), story.loopNew.end(), m->id) != story.loopNew.end();
    float ly = y + 20;
    for (int i = 0; i < nl; ++i) { LookJournalText(lines[i], x + 8, ly, i, adult, fresh); ly += lineH; }
}
