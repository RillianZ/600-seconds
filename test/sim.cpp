// Headless story test. Plays scripted loops against the engine-free logic
// and checks that every memory entry and every ending can be reached.
// Build: g++ -std=c++17 -Isrc test/sim.cpp src/game/Story.cpp -o sim && ./sim
#include "game/Story.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

using namespace sixhundred;

static Story s;
static int px = 5, py = 5;
static bool verbose = false;
static int loopsEnded = 0;
static std::vector<std::string> endingsSeen;

// What to do when the real-time chase starts: "caught", "found", or a visitor choice id
static std::string chasePlan = "caught";
static void At(int x, int y);
static bool qteWin = true;
static std::vector<std::string> pickQueue; // choice ids to prefer, consumed in order
static std::vector<std::string> trueQueue; // v0.17: the order you tell him the five pieces in

static void Drain() {
    int guard = 0;
    while (!s.beats.empty() && guard++ < 800) {
        Beat b = s.beats.front();
        s.beats.pop_front();
        if (verbose && (b.type == BeatType::Say || b.type == BeatType::Narrate || b.type == BeatType::Black))
            std::printf("  [%s] %s: %s\n", s.ClockText().c_str(), b.speaker.c_str(), b.text.c_str());
        switch (b.type) {
        case BeatType::Clue: std::printf("  + memory: %s\n", b.id.c_str()); break;
        case BeatType::Achievement: std::printf("  * achievement: %s\n", b.text.c_str()); break;
        case BeatType::Fade:
            s.AdvanceMinutes(b.minutes, px, py);
            if (b.tx >= 0) { px = b.tx; py = b.ty; }
            break;
        case BeatType::Choice: {
            std::string id = "none";
            for (auto& c : b.choices) if (c.id == "none") id = "none";
            bool found = false;
            for (size_t k = 0; k < pickQueue.size() && !found; ++k)
                for (auto& c : b.choices)
                    if (c.id == pickQueue[k]) { id = c.id; pickQueue.erase(pickQueue.begin() + k); found = true; break; }
            if (!found) id = b.choices.front().id;
            s.Choose(id, px, py);
        } break;
        case BeatType::Scene:
            if (b.id.rfind("climb:", 0) == 0) {
                // v0.15: the presentation climbs through the window, then tells the story
                const WindowInfo* w = WindowById(b.id.substr(6));
                assert(w);
                bool out = !(px == w->outX && py == w->outY);
                At(out ? w->outX : w->inX, out ? w->outY : w->inY);
                s.OnClimbed(w->id, out, false);
            }
            else if (b.id == "dad_runs") s.SceneDone("dad_runs");
            else if (b.id == "minigame") s.MinigameDone(4);
            else if (b.id == "chase") {
                if (chasePlan == "caught") s.OnCaught();
                else if (chasePlan == "hide") { At(9, 5); s.kidHidden = true; }   // in the wardrobe until 17:00 (v0.12: a yard spot would freeze you)
                else if (chasePlan == "run") s.kidHidden = false;        // keep running until 17:00
                else if (chasePlan == "doggo") { s.doggoWithKid = true; s.OnCaught(); }
                else if (chasePlan == "found") s.OnFound();
                else if (chasePlan == "hunt") {
                    // v0.12: everything the hunt can teach in one go, then he finds the wardrobe
                    s.OnHeard("run"); s.OnHeard("breath"); s.OnTracked();
                    bool a = s.OnDecoy(), b2 = s.OnDecoy(), c = s.OnDecoy();
                    assert(a && b2 && !c);
                    s.hideSpot = "wardrobe"; s.OnFound();
                }
                else if (chasePlan == "crates") {}   // v0.13: the test drives this hunt by hand
                else if (chasePlan == "cold") { At(25, 25); s.kidHidden = false; }   // stand out in the storm, no jacket
                else if (chasePlan == "outside") s.OnLeftPremises();
                else if (chasePlan == "litup") { s.OnLitUp(); s.OnCaught(); }   // v0.15: his torch, from across the yard
                else if (chasePlan == "taser") s.OnTased();                      // v0.15.2: kited him one time too many
                else if (chasePlan == "doggodoor") { assert(s.DoggoExists()); s.OnDoggoAtDoor(); s.OnDoggoPutDown(19, 19); assert(s.doggoDead && s.chase); s.OnCaught(); }   // v0.16
                else if (chasePlan == "empty") { s.momLocked = s.dadLocked = true; s.doggoOut = true; At(9, 5); s.kidHidden = true; }   // v0.15.2: nobody he can reach
                else if (chasePlan == "tie") { assert(s.visitorDowned); pickQueue = {"t_tie"}; s.Interact("visitor", px, py); }   // v0.17
                else if (chasePlan == "grab") { assert(s.DadGuardsDoor()); s.OnParentSpots("Mom", Room::Kitchen); s.OnParentFinds("Mom"); assert(s.chase); s.OnParentGrabs("Dad"); assert(!s.chase); }   // v0.18
                else if (chasePlan == "defend_mom" || chasePlan == "defend_dad") {
                    // v0.18: the one parent who knows runs at him, loses, and leaves something on the floor
                    std::string who = chasePlan == "defend_mom" ? "Mom" : "Dad";
                    assert(s.Defender() == who);
                    s.OnDefend(who); s.OnDefendDone(who, 21, 16);
                    assert(s.Defender().empty() && s.dropItem == (who == "Mom" ? "box" : "taser"));
                    s.Interact("drop", 21, 16); assert(s.dropItem.empty());
                    if (who == "Mom") { assert(s.switchTaken && s.momOnFloor); s.OnCaught(); }
                    else { assert(s.hasTaser && s.dadOnFloor); s.Interact("visitor", px, py);
                           bool offered = false; for (auto& c : s.beats.back().choices) if (c.id == "c_tase") offered = true;
                           assert(offered); s.beats.pop_back(); s.Choose("c_tase", px, py);
                           assert(s.visitorDowned && !s.hasTaser && s.chase); pickQueue = {"c_go"}; s.Interact("visitor", px, py); }
                }
                else if (chasePlan == "stage") {
                    // v0.19: everything you set up before he came, paying off in one hunt
                    assert(s.falseTracks && s.Barricaded(6, 9) && s.innerTraps.count("tripwire") && s.carKey == "snow");
                    s.OnFalseTrail(false); s.OnFalseTrail(true); assert(s.falseTrailWalked);
                    s.OnInnerTrap("tripwire", true); assert(!s.innerTraps.count("tripwire") && s.Knows("his_knot"));
                    s.OpenDoor(6, 9); assert(s.DoorLocked(6, 9));          // keys don't move furniture
                    s.OnBarricadeHit(6, 9); s.BreakDoor(6, 9); s.OnBarricadeBroken(6, 9, true);
                    assert(!s.Barricaded(6, 9) && s.DoorBroken(6, 9) && s.Knows("new_hinges"));
                    s.OnCaught();
                }
                else if (chasePlan == "true") { assert(s.visitorDowned); pickQueue = trueQueue; s.Interact("visitor", px, py); }                // v0.17: queue set by the loop
                else { pickQueue.insert(pickQueue.begin(), chasePlan); s.Interact("visitor", px, py); }
            }
            break;
        case BeatType::QTE: s.QTEResult(qteWin); break;
        case BeatType::Ending:
            std::printf("  ### ENDING: %s\n", b.id.c_str());
            endingsSeen.push_back(b.id);
            s.beats.clear();
            s.BeginLoop();            // the title screen's "continue"
            px = 5; py = 5;
            return;
        case BeatType::EndLoop:
            s.EndLoop();
            loopsEnded++;
            px = 5; py = 5;
            break;
        default: break;
        }
    }
}

void At(int x, int y) { px = x; py = y; }
static void WaitUntil(int minute) {
    int startLoop = s.loop;
    while (s.loop == startLoop && s.t < minute * 60 && !s.collection) { s.Tick(0.5f, false, px, py); s.Tick(0.5f, false, px, py); Drain(); }
}
static void Use(const std::string& id, std::vector<std::string> picks = {}) {
    pickQueue = picks;
    s.Interact(id, px, py);
    Drain();
    pickQueue.clear();
}
static void FinishLoop(const std::string& plan = "caught") {
    chasePlan = plan;
    int startLoop = s.loop;
    while (s.loop == startLoop) { s.Tick(12.0f, false, px, py); Drain(); }
}
// Reroll loops that landed in a reality shift, so scripted routes stay predictable
static void FinishLoop(const std::string& plan);
static void CleanLoop() { while (s.shift != Shift::None) FinishLoop("caught"); }
static void Header(const char* t) { std::printf("%s  (loop %d, dark %d)\n", t, s.loop, s.darkPoints); }

static void ShiftFrequencyTest() {
    // v0.4.2: with the default settings, anomalies are 5 to 10 loops apart and the first one is the toilet
    SeedShifts(12345);
    for (int run = 0; run < 50; ++run) {
        Story f; int last = 0, count = 0, toilets = 0, dogs = 0, minG = 99, maxG = 0;
        for (int i = 0; i < 200; ++i) {
            f.loop++;
            Shift sh = f.RollShift();
            if (sh == Shift::None) { f.cleanStreak++; continue; }
            if (count == 0) assert(sh == Shift::Toilet45);
            if (sh == Shift::Toilet45) { toilets++; f.seenToilet45 = true; } else dogs++;
            if (last) { minG = std::min(minG, f.loop - last); maxG = std::max(maxG, f.loop - last); }
            last = f.loop; count++; f.cleanStreak = 0;
        }
        assert(minG >= 5 && maxG <= 10 && toilets > 0 && dogs > 0);
        if (run == 0) std::printf("Reality shifts, defaults: %d in 200 loops (%d toilet, %d no Doggo), gaps %d..%d loops\n", count, toilets, dogs, minG, maxG);
    }
    // switched off = never
    { ShiftConfig keep = gShiftCfg; gShiftCfg.enabled = false; Story f;
      for (int i = 0; i < 100; ++i) { f.loop++; f.cleanStreak = 50; assert(f.RollShift() == Shift::None); }
      gShiftCfg = keep; }
    // force works even when off
    { ShiftConfig keep = gShiftCfg; gShiftCfg.enabled = false; gShiftCfg.forceNext = Shift::NoDoggo; Story f; f.loop = 1;
      assert(f.RollShift() == Shift::NoDoggo && f.RollShift() == Shift::None); gShiftCfg = keep; }
    // config survives a save round trip
    { ShiftConfig a; a.enabled = false; a.chance = 35; a.minGap = 2; a.maxGap = 0; a.firstLoop = 5; a.toiletShare = 30;
      ShiftConfig b; b.Load(a.Save()); assert(!b.enabled && b.chance == 35 && b.minGap == 2 && b.maxGap == 0 && b.firstLoop == 5 && b.toiletShare == 30); }
}

// v0.17: the true ending in fresh stories. Rules first, then every variation printed so the lines can be read
static std::vector<Beat> RunTrue(Story& m, std::vector<std::string> picks, bool print, bool* sawKill = nullptr) {
    std::vector<Beat> out;
    m.t = 49 * 60 + 59; m.Tick(1.0f, false, 21, 17);
    for (int g = 0; g < 600 && !m.beats.empty(); ++g) {
        Beat b = m.beats.front(); m.beats.pop_front(); out.push_back(b);
        if (print && (b.type == BeatType::Black || b.type == BeatType::Narrate || b.type == BeatType::Ending ||
                      (b.type == BeatType::Say && m.toldPieces.size() >= 5)))
            std::printf("    %s%s\n", b.speaker.empty() ? "" : (b.speaker + ": ").c_str(), b.text.c_str());
        if (b.type == BeatType::Scene && b.id == "chase") m.Interact("visitor", 21, 17);
        if (b.type == BeatType::Choice) {
            std::string id = b.choices.front().id;
            for (auto& c : b.choices) if (c.id == "t_kill" && sawKill) *sawKill = true;
            bool f = false;
            for (size_t k = 0; k < picks.size() && !f; ++k) for (auto& c : b.choices) if (c.id == picks[k]) { id = c.id; picks.erase(picks.begin() + k); f = true; break; }
            m.Choose(id, 21, 17);
        }
    }
    return out;
}
static std::string EndingOf(const std::vector<Beat>& v) { for (auto& b : v) if (b.type == BeatType::Ending) return b.id; return ""; }
static void TrueEndingTest() {
    std::vector<std::string> five = {"piece_mom", "piece_dad", "piece_reissue", "piece_door", "piece_car"};
    auto fresh = [&](int dark, bool knife) {
        Story m; m.loop = 9; m.memory = std::set<std::string>(five.begin(), five.end()); m.memory.insert("loop");
        m.darkPoints = dark; m.tainted = dark >= DARK_TAINT;
        m.BeginLoop(); m.beats.clear(); while (m.shift != Shift::None) { m.EndLoop(); m.BeginLoop(); m.beats.clear(); }
        m.trapsSet = {"marbles", "tripwire"}; m.traps = 2;
        if (knife) m.Hold(Item::Knife);
        return m;
    };
    std::vector<std::string> all = {"t_hum", "t_watch", "t_reissue", "t_dose", "t_last", "t_spare"};
    // four pieces is not enough: no options at all
    { Story m = fresh(0, false); m.memory.erase("piece_car"); bool any = false;
      m.t = 49 * 60 + 59; m.Tick(1.0f, false, 21, 17);
      for (int g = 0; g < 300 && !m.beats.empty(); ++g) { Beat b = m.beats.front(); m.beats.pop_front();
        if (b.type == BeatType::Scene && b.id == "chase") { m.Interact("visitor", 21, 17); }
        if (b.type == BeatType::Choice) { for (auto& c : b.choices) if (c.id.rfind("t_", 0) == 0) any = true; m.Choose("none", 21, 17); } }
      assert(!any); }
    // he stays down once you start, and the bells take it if you walk away
    { Story m = fresh(0, false); m.t = 49 * 60 + 59; m.Tick(1.0f, false, 21, 17); m.beats.clear();
      m.Choose("t_dose", 21, 17); m.beats.clear(); assert(m.HoldDown());
      m.OnVisitorUp(); assert(m.visitorDowned);
      m.t = Story::HOUR - 1; m.Tick(1.0f, false, 21, 17);
      bool loopEnd = false; for (auto& b : m.beats) if (b.type == BeatType::EndLoop) loopEnd = true;
      assert(loopEnd && m.threats == 0 && !m.chase); }
    // parents in the room: no kill option. Both out: there is one
    { Story m = fresh(0, true); bool kill = false; auto v = RunTrue(m, all, false, &kill); assert(!kill && EndingOf(v) == "after"); }
    { Story m = fresh(0, true); m.momLocked = m.dadLocked = true; bool kill = false; auto v = RunTrue(m, all, false, &kill); assert(kill && EndingOf(v) == "after"); }
    // tied: riddles, then the bells end the loop (not an ending)
    { Story m = fresh(0, false); m.memory.erase("piece_car"); m.trapsSet.insert("paint"); m.traps = 3;
      m.t = 49 * 60 + 59; m.Tick(1.0f, false, 21, 17); m.beats.clear();
      m.Choose("t_tie", 21, 17); assert(m.visitorTied && m.HoldDown()); m.beats.clear();
      for (int i = 0; i < 3; ++i) m.Choose("t_talk", 21, 17);
      int said = 0; for (auto& b : m.beats) if (b.type == BeatType::Say && b.speaker == "The visitor") said++;
      assert(said == 3 && m.tiedTalk == 3); m.beats.clear();
      m.Choose("t_wait", 21, 17); m.Tick(1.0f, false, 21, 17);
      bool loopEnd = false, restrained = false;
      for (auto& b : m.beats) { if (b.type == BeatType::EndLoop) loopEnd = true; if (b.text.find("Restrained") != std::string::npos) restrained = true; }
      assert(loopEnd && restrained && m.endings.empty()); }
    // two traps is not enough to tie him
    { Story m = fresh(0, false); m.t = 49 * 60 + 59; m.Tick(1.0f, false, 21, 17); m.beats.clear();
      m.Interact("visitor", 21, 17); bool tie = false;
      for (auto& b : m.beats) if (b.type == BeatType::Choice) for (auto& c : b.choices) if (c.id == "t_tie") tie = true;
      assert(!tie); }
    std::puts("  rules: five pieces needed, he stays down once you start, no kill in front of them, tie needs all three traps");
    struct V { const char* name; int dark; bool knife; bool mom, bath, dad, remorse, grad, doggoOut, stay; std::vector<std::string> picks; const char* want; };
    std::vector<V> vs = {
        {"KILL, clean kid, his gun, Mom in the bath humming, Dad in the shed, hum first", 0, false, false, true, true, true, false, false, false, {"t_hum", "t_watch", "t_reissue", "t_dose", "t_last", "t_kill"}, "samehouse"},
        {"KILL, tainted, knife, Mom bolted downstairs, Appendix C read, Doggo out, dose first", 9, true, true, false, true, false, true, true, false, {"t_dose", "t_hum", "t_watch", "t_reissue", "t_last", "t_kill"}, "samehouse"},
        {"KILL, as dark as it gets, knife, Doggo shot at the door, watch first", 14, true, true, false, true, false, true, false, false, {"t_watch", "t_hum", "t_reissue", "t_dose", "t_last", "t_kill"}, "samehouse"},
        {"SPARE, clean kid, parents right there, Mom hummed back to, Doggo stayed, hum first", 0, false, false, false, false, true, false, false, true, {"t_hum", "t_watch", "t_reissue", "t_dose", "t_last", "t_spare"}, "after"},
        {"SPARE, tainted, knife, both locked away, Appendix C, reissue first", 9, true, true, false, true, false, true, false, false, {"t_reissue", "t_hum", "t_watch", "t_dose", "t_last", "t_spare"}, "after"},
    };
    for (auto& v : vs) {
        Story m = fresh(v.dark, v.knife);
        m.momLocked = v.mom; m.momBathLocked = v.bath; m.dadLocked = v.dad; m.momRemorse = v.remorse; m.doggoOut = v.doggoOut; m.doggoStay = v.stay;
        if (v.grad) m.memory.insert("graduation");
        m.memory.insert("grave_05");
        std::printf("  --- %s\n", v.name);
        auto out = RunTrue(m, v.picks, true);
        assert(EndingOf(out) == v.want);
        assert(m.endings.count(v.want) && m.Knows("he_remembers"));
    }
}

static bool MenuHas(Story& st, const std::string& who, const std::string& opt) {
    st.beats.clear(); st.Interact(who, 8, 15);
    for (auto& b : st.beats) if (b.type == BeatType::Choice) for (auto& c : b.choices) if (c.id == opt) return true;
    return false;
}
static void AttitudeTest() {
    // v0.5: after you pull a knife, parents stop small talk for the rest of the loop
    Story a; a.loop = 3; a.BeginLoop(); a.beats.clear(); a.Hold(Item::Knife);
    assert(MenuHas(a, "mom", "m_dinner"));
    a.beats.clear(); a.Choose("m_knife", 8, 15);
    assert(a.momAtt == Attitude::Scared);
    assert(!MenuHas(a, "mom", "m_dinner") && MenuHas(a, "mom", "m_sorry"));
    a.beats.clear(); a.Choose("m_sorry", 8, 15);
    assert(!MenuHas(a, "mom", "m_sorry"));          // after the apology she just flinches
    a.beats.clear(); a.Choose("d_knife", 8, 15);
    assert(a.dadAtt == Attitude::Scared && !MenuHas(a, "dad", "d_what"));
    // telling Mom about the loop makes her cold until you apologise
    Story c; c.loop = 3; c.memory = {"loop"}; c.BeginLoop(); c.beats.clear();
    c.Choose("m_loop", 8, 15); assert(c.momAtt == Attitude::Cold && !MenuHas(c, "mom", "m_dinner"));
    c.beats.clear(); c.Choose("m_sorry", 8, 15); assert(c.momAtt == Attitude::Normal && MenuHas(c, "mom", "m_dinner"));
    // a new loop resets everything
    a.BeginLoop(); assert(a.momAtt == Attitude::Normal && a.dadAtt == Attitude::Normal);
    std::printf("Attitude states: OK\n");
}

static void ItemTest() {
    // v0.6: one thing at a time
    Story i; i.loop = 3; i.BeginLoop(); i.beats.clear();
    i.Choose("pa_food", 4, 14); assert(i.held == Item::DogFood && i.hasFood);
    i.Choose("wp_stick", 29, 25); assert(i.held == Item::Stick && i.hasStick && !i.hasFood);
    i.Hold(Item::Knife); assert(i.hasKnife && !i.hasStick);
    i.Hold(Item::Key); assert(i.hasKey && !i.hasKnife);
    i.Choose("it_back", 10, 26); assert(i.held == Item::None && !i.hasKey);
    // the clock keeps going in the chase, 17:00 = tired out
    Story c; c.loop = 3; c.BeginLoop(); c.beats.clear();
    c.collection = true; c.chase = true; c.t = 59 * 60 + 59;
    c.Tick(1.0f, false, 20, 15);
    bool tired = false;
    for (auto& b : c.beats) if (b.type == BeatType::Achievement && b.text == "Tired Out") tired = true;
    assert(tired && !c.chase);
    // two games, then anything else, breaks the Screen Time streak
    Story g; g.loop = 3; g.BeginLoop(); g.beats.clear(); g.zuldaOpened = true;
    g.Choose("ft_zulda", 8, 7); g.MinigameDone(2); assert(g.playStreak == 2);
    g.Interact("sink", 8, 14); assert(g.playStreak == 0);
    std::printf("Held items + 17:00 + streak: OK\n");
    // v0.7: doors close without a key, lock only with it
    Story d; d.loop = 3; d.BeginLoop(); d.beats.clear();
    d.Interact("door_kid", 6, 8); assert(d.DoorClosed(6, 9) && !d.DoorLocked(6, 9));
    d.Interact("door_kid", 6, 8); assert(!d.DoorClosed(6, 9));
    d.Hold(Item::Key); d.beats.clear(); d.Interact("door_kid", 6, 8);
    d.Choose("dr_lockc:door_kid", 6, 8); assert(d.DoorLocked(6, 9));
    d.Hold(Item::None); d.beats.clear(); d.Interact("door_kid", 6, 8); assert(d.DoorLocked(6, 9));   // no key, no way through
    d.BreakDoor(6, 9); assert(!d.DoorClosed(6, 9) && d.DoorBroken(6, 9));
    // a dark act this loop ruins "befriended"
    Story b; b.loop = 3; b.BeginLoop(); b.beats.clear();
    b.fed = b.trashDone = b.roomClean = b.dishesDone = b.firewoodDone = true;
    assert(b.Befriended() && b.CurrentStance() == Stance::Befriended);
    b.Choose("s_bar", 34, 11); assert(!b.Befriended() && b.CurrentStance() == Stance::Dark);
    // threats wait while he can see you
    Story th; th.loop = 3; th.BeginLoop(); th.beats.clear(); th.collection = th.chase = true; th.t = 57 * 60 + 32;
    th.visitorChasing = true; th.Tick(0.2f, false, 20, 15); assert(th.threats == 0);
    th.visitorChasing = false; th.Tick(0.2f, false, 20, 15); assert(th.threats == 1);
    std::printf("Doors, befriended, threats: OK\n");
}

int main(int argc, char** argv) {
    verbose = argc > 1;
    ShiftFrequencyTest();
    ItemTest();
    AttitudeTest();
    s.NewGame();
    s.BeginLoop();
    Drain();

    Header("LOOP 1 tutorial: chores, Dad runs, caught");
    At(8, 6); Use("toys");
    At(11, 18); Use("trashcan");
    At(5, 22); Use("bins");
    At(4, 14); Use("pantry", {"pa_food"});
    assert(s.hasFood);
    At(5, 18); Use("bowl");
    assert(s.ChoresDone() == 3 && !s.hasFood);
    FinishLoop("caught");
    assert(s.loop == 2 && !s.Knows("dad_ran") && !s.Knows("he_runs_fast") && s.Knows("visitor_time"));   // v0.8: loop 1 has no chase

    Header("LOOP 2: house clues, 16:22 call, 16:30 peek, hide");
    At(4, 10); Use("coats");
    At(6, 14); Use("calendar");
    At(17, 17); Use("album");
    At(16, 14); Use("bookshelf");
    At(25, 16); Use("tree", {"tr_open"});
    assert(s.Knows("presents"));
    At(28, 11); Use("basement");
    At(8, 15); WaitUntil(23);
    At(20, 10); WaitUntil(30);
    At(28, 10); Use("basement", {"b_peek"});
    FinishLoop("found");

    Header("LOOP 3: radio, answer phone, desk, tell Dad, 'I know'");
    At(35, 6); WaitUntil(11);
    At(4, 15); WaitUntil(21);
    Use("phone");
    At(18, 5); Use("desk");
    At(35, 7); Use("dad", {"d_ask", "d_loop"});
    assert(s.visitorTime == 40 * 60);
    gShiftCfg.forceNext = Shift::Toilet45;   // the debug "force the next loop" knob
    FinishLoop("c_know");

    Header("LOOP 4: the 45 minute toilet, then the vidda");
    assert(s.shift == Shift::Toilet45);
    At(12, 5); Use("toilet");
    At(20, 32); s.Tick(0.1f, false, px, py); Drain();

    Header("LOOP 5: help Dad, tell Mom, TV, fight");
    At(35, 7); WaitUntil(9); Use("dad", {"d_help"});
    At(8, 15); Use("mom", {"m_ask", "m_loop"});
    At(21, 14); Use("tv");
    FinishLoop("c_fight");

    CleanLoop();
    Header("LOOP 5b: he looks through the window, his footprints, then the snow gets you");
    assert(!s.hasCoat);
    At(22, 24); WaitUntil(42);
    assert(!s.CarHere() && !s.Knows("prints"));
    WaitUntil(44);
    { NpcState v = s.Visitor(); assert(v.visible && v.x == 25 && v.y == 21 && v.facingWall); }
    WaitUntil(46);
    assert(s.Knows("prints"));
    { NpcState v = s.Visitor(); assert(v.visible && v.x == 25); }
    WaitUntil(47);
    { NpcState v = s.Visitor(); assert(!v.visible); }
    WaitUntil(49);
    { NpcState v = s.Visitor(); assert(v.visible && v.x == 20 && v.y == 21); }
    FinishLoop("cold");
    assert(s.Knows("cold"));

    CleanLoop();
    Header("LOOP 5c: the early storm");
    gShiftCfg.forceNext = Shift::EarlyStorm;
    FinishLoop("caught");
    assert(s.shift == Shift::EarlyStorm && !s.PowerOut());
    At(8, 15); WaitUntil(38);
    assert(s.Storm() && !s.PowerOut());
    WaitUntil(41);
    assert(s.PowerOut() && !s.collection && s.achievements.count("Weather Report"));
    FinishLoop("caught");

    CleanLoop();
    Header("LOOP 5d: talk to him early and the knock comes sooner");
    At(20, 28); WaitUntil(44);
    { int was = s.visitorTime; Use("visitor", {"v_who"}); assert(s.visitorTime < was); }
    assert(s.visitorTime - (int)s.t == Story::VISIT_BACK && !s.collection);
    { NpcState v = s.Visitor(); assert(v.visible && v.x == 20 && v.y == 30); }
    { int was = s.visitorTime; Use("visitor", {"v_leave"}); assert(s.visitorTime == was); }
    FinishLoop("caught");
    assert(s.visitorTime == 50 * 60);

    CleanLoop();
    Header("LOOP 6: meet him at the gate with Doggo");
    At(20, 28); WaitUntil(49);
    Use("visitor", {"v_program"});
    FinishLoop("caught");

    CleanLoop();
    Header("LOOP 7: the knife, bar Dad, scare Mom, bolt Mom, Doggo, mirror, fortress");
    assert(!s.tainted);
    At(12, 15); Use("knives", {"k_take", "k_take", "k_take", "k_take", "k_take"});
    assert(s.hasKnife);
    At(34, 11); WaitUntil(9); Use("sheddoor", {"s_bar"});
    assert(!s.tainted && s.dadLocked);           // a plank is just weird, not dark
    At(8, 15); Use("mom", {"m_knife"});
    At(28, 10); WaitUntil(30); Use("basement", {"b_bolt"});
    assert(s.momLocked && !s.tainted);
    At(6, 6); Use("doggo", {"g_search"});
    assert(s.Knows("syringe"));                  // dark enough for Doggo to find the syringe
    At(6, 6); Use("doggo", {"g_shed", "g_shed2"});
    assert(s.tainted && s.darkPoints >= 10);     // the dog does it
    At(14, 4); Use("mirror");
    At(20, 19); WaitUntil(45); Use("frontdoor", {"f_lock"});
    FinishLoop("caught");

    CleanLoop();
    Header("LOOP 8: interrogate Dad, the basement");
    At(12, 15); Use("knives", {"k_take", "k_take"});
    At(35, 7); WaitUntil(9); Use("dad", {"d_talk", "d_talk2"});
    assert(s.Knows("dad_key"));
    At(10, 26); Use("flag", {"fl_key"});
    assert(s.hasKey);
    At(28, 10); Use("basement", {"b_down"});
    At(9, 38); Use("screens");
    At(14, 38); Use("files");
    At(11, 40); Use("chair");
    At(18, 39); Use("console", {"r_cancel"});
    FinishLoop("caught");

    CleanLoop();
    Header("LOOP 9: arsenal + knife, finish him (End of the Program)");
    At(3, 6); Use("marbles", {"a_marbles"});
    At(32, 8); Use("rope", {"a_tripwire"});
    At(12, 15); Use("knives", {"k_take", "k_take"});
    assert(s.traps == 2 && s.hasKnife);
    FinishLoop("c_finish");      // VisitorChoice -> c_finish -> c_finish2: the dead man's switch takes us too
    assert(s.Knows("dead_switch") && endingsSeen.empty());

    CleanLoop();
    Header("LOOP 9b: same again, take the switch off his belt first (End of the Program)");
    At(3, 6); Use("marbles", {"a_marbles"});
    At(32, 8); Use("rope", {"a_tripwire"});
    At(12, 15); Use("knives", {"k_take", "k_take"});
    FinishLoop("c_finish");      // "Are you sure?" now offers c_switch first, then Do it.
    // v0.18 (Noah's answer 7): they're standing right there and they were never on your side. They take over the hunt
    assert(!endingsSeen.empty() && endingsSeen.back() == "handlers" && s.Knows("handlers_hunt"));

    CleanLoop();
    Header("LOOP 9b2: same again, but Mom bolted downstairs and Dad barred in the shed (End of the Program)");
    At(3, 6); Use("marbles", {"a_marbles"});
    At(32, 8); Use("rope", {"a_tripwire"});
    At(12, 15); Use("knives", {"k_take", "k_take"});
    s.momLocked = s.dadLocked = true;   // loop 7 shows the real way. Here it only has to be true at the knock
    FinishLoop("c_finish");
    assert(!endingsSeen.empty() && endingsSeen.back() == "program");

    CleanLoop();
    Header("LOOP 9c: evade him, then run past the fence");
    FinishLoop("outside");

    auto chores = [&]() {
        At(8, 6); Use("toys");
        At(11, 18); Use("trashcan");
        At(5, 22); Use("bins");
        At(4, 14); Use("pantry", {"pa_food"});
        At(5, 18); Use("bowl");
        At(8, 14); Use("sink");
        At(29, 25); Use("woodpile", {"wp_stack"});
    };

    CleanLoop();
    Header("LOOP 9d: open the cellar, swap the key for the tape, watch it, the juice");
    At(10, 26); Use("flag", {"fl_key"});
    At(28, 10); Use("basement", {"b_unlock"});
    assert(s.basementUnlocked);
    At(27, 5); Use("drawer", {"cab_tape"});
    assert(s.held == Item::Tape && s.keyInDrawer && !s.hasKey);
    At(28, 10); Use("basement", {"b_down"});
    assert(s.inBasement);
    At(7, 42); Use("vcr", {"vt_play"});
    At(18, 42); Use("medfridge");
    assert(s.Knows("the_tape") && s.Knows("juice"));
    FinishLoop("caught");

    CleanLoop();
    Header("LOOP 9e: befriended. Every chore, nothing dark. Dad fights him. Hide through the threats (They Paid)");
    chores();
    assert(s.Befriended());
    FinishLoop("hide");
    assert(s.Knows("dad_ran") && s.Knows("they_paid") && endingsSeen.back() == "paid");

    CleanLoop();
    Header("LOOP 9f: befriended, then tell Mom about the clean-up (False Hope)");
    chores();
    At(8, 15); Use("mom", {"m_ask", "m_loop"});
    assert(s.momTurned && !s.dadTurned && !s.parentsAllied && s.Knows("allies"));   // v0.18: one at a time
    At(35, 7); Use("dad", {"d_ask", "d_loop"});
    assert(s.parentsAllied);
    FinishLoop("caught");          // first choice at the door: run for his car
    assert(endingsSeen.back() == "escape");

    CleanLoop();
    Header("LOOP 9g: Doggo jumps in front of you. First time the loop just goes on");
    { size_t before = endingsSeen.size();
      qteWin = false;   // this can be the last memory: don't win the trunk here
      FinishLoop("doggo");
      qteWin = true;
      assert(s.Knows("doggo_stay") && endingsSeen.size() == before); }

    CleanLoop();
    Header("LOOP 9g2: you knew, and he's still in the room (Good Boy)");
    FinishLoop("doggo");
    assert(endingsSeen.back() == "doggo");

    CleanLoop();
    Header("LOOP 9h: tell Doggo to stay, keep running until 17:00 (Tired Out)");
    At(6, 6); Use("doggo", {"g_sit"});
    assert(s.doggoStay);
    FinishLoop("run");

    // ------------------------------------------------------------ v0.10
    CleanLoop();
    Header("LOOP 12a: the hunt. Marbles, running feet, wet boots, breathing. Found in the wardrobe");
    assert(Story::IsHideSpot("pantry") && Story::IsHideSpot("doghouse") && !Story::IsHideSpot("fridge"));
    At(3, 6); Use("marbles", {"m_pocket"});
    assert(s.pebbles == 3 && s.CaughtIn("wardrobe") == 0);
    FinishLoop("hunt");
    assert(s.Knows("he_hears") && s.Knows("breath") && s.Knows("wet_prints") && s.Knows("decoy") && s.achievements.count("Lost Your Marbles"));
    assert(s.CaughtIn("wardrobe") == 1 && !s.Knows("he_learns") && s.pebbles == 0);

    CleanLoop();
    Header("LOOP 12b: the wardrobe again. He walks straight to it");
    FinishLoop("hunt");
    assert(s.CaughtIn("wardrobe") == 2 && s.Knows("he_learns") && s.achievements.count("Creature of Habit"));
    assert(s.Knows("outlasted"));   // from They Paid and Tired Out above
    { Story r; r.LoadSave(s.SaveString()); assert(r.CaughtIn("wardrobe") == 2 && r.CaughtIn("bed") == 0); }

    CleanLoop();
    Header("LOOP 11a: Doggo wants out at 16:35. Let him");
    At(20, 18); WaitUntil(36);
    assert(s.DoggoAtDoor() && s.SawEvent("doggo_door"));
    Use("doggo", {"g_out"});
    assert(s.doggoOut && !s.DoggoExists());
    FinishLoop("caught");
    assert(s.Knows("doggo_out") && s.achievements.count("Run, Boy"));

    CleanLoop();
    Header("LOOP 11b: pull the phone plug. No call, no clean-up");
    At(4, 15); Use("phone", {"p_unplug"});
    assert(s.phoneUnplugged);
    WaitUntil(24);
    assert(!s.phoneAnswered);
    FinishLoop("c_go");
    assert(s.Knows("off_hook") && s.achievements.count("Clerical Error"));

    CleanLoop();
    Header("LOOP 11c: his car is empty while he looks in the window");
    At(20, 29); WaitUntil(43);
    assert(!s.CarEmpty());
    WaitUntil(45);
    assert(s.CarEmpty());
    Use("car", {"car_look"}); Use("car", {"car_trunk"});
    assert(s.Knows("his_car") && s.Knows("trunk") && s.HasProof());
    { int was = s.loop; Use("car", {"car_in"}); assert(s.loop == was + 1 && s.achievements.count("Express Delivery")); }

    CleanLoop();
    Header("LOOP 11d: the tiny watch in Dad's cabinet. Show it to Mom, then to him. Ask him about every time");
    At(10, 26); Use("flag", {"fl_key"});
    At(27, 5); Use("drawer", {"cab_watch"});
    assert(s.held == Item::BabyWatch && s.keyInDrawer && s.Knows("baby_watch"));
    At(8, 15); Use("mom", {"m_watch"});
    At(20, 29); WaitUntil(45);
    Use("visitor", {"v_watch"});
    assert(s.Knows("collector"));
    Use("visitor", {"v_every"});
    assert(s.Knows("echo"));
    FinishLoop("caught");

    CleanLoop();
    Header("LOOP 11e: the thin folder. The protocol, and Appendix C");
    At(10, 26); Use("flag", {"fl_key"});
    At(28, 10); Use("basement", {"b_down"});
    At(14, 38); Use("files", {"fi_protocol"});
    assert(s.Knows("protocol") && s.Knows("graduation") && s.achievements.count("Entrance Exam"));
    FinishLoop("caught");

    CleanLoop();
    Header("LOOP 11f: hum her song back to her. She says a name");
    At(8, 15); Use("mom", {"m_hum"});
    assert(s.momRemorse && !s.MomHums());
    FinishLoop("c_go");
    assert(s.Knows("my_name") && s.achievements.count("Say It Again"));

    CleanLoop();
    Header("LOOP 11g: befriended, allied, and you know about the leash. Sit down (One Good Hour)");
    chores();
    At(8, 15); Use("mom", {"m_ask", "m_loop"});
    At(35, 7); Use("dad", {"d_ask", "d_loop"});
    assert(s.parentsAllied && s.Knows("leash"));
    pickQueue = {"e_dinner"};
    FinishLoop("caught");
    pickQueue.clear();
    assert(endingsSeen.back() == "bells" && s.endings.count("bells"));

    Header("LOOP 11h: somebody is in the bathroom");
    gShiftCfg.forceNext = Shift::Sister;
    FinishLoop("caught");
    assert(s.shift == Shift::Sister && s.DoorLocked(13, 9));
    At(13, 10); Use("door_bath");
    At(17, 17); Use("album");
    At(8, 15); Use("mom", {"m_ask", "m_sister"});
    assert(s.achievements.count("Table for Four") && s.sisterNoticed);
    FinishLoop("caught");

    // ------------------------------------------------------------ v0.13: the hinges
    Header("Hinge A rules: a trick when she's happy, dark with the knife, darker when she's scared");
    {
        auto lockAt = [](Story& f) {
            f.memory = {"loop", "dad_key"}; f.loop = 1; f.BeginLoop(); f.beats.clear();
            f.Hold(Item::Key, true);
            f.t = 46 * 60 + 40; f.momRoomNow = Room::Bathroom;
            assert(f.MomInBath() && f.Mom().x == 13 && f.Mom().y == 6);
        };
        Story a; lockAt(a);
        a.Interact("door_bath", 13, 10);
        bool offered = false; for (auto& b : a.beats) for (auto& c : b.choices) if (c.id == "bath_lock" && !c.dark) offered = true;
        assert(offered);
        a.beats.clear(); a.Choose("bath_lock", 13, 10);
        assert(a.momBathLocked && !a.bathLockDark && a.darkThisLoop == 0 && a.DoorLocked(13, 9) && !a.MomHere());
        assert(a.achievements.count("Occupied"));
        Story b; lockAt(b); b.knifeTakenLoop = true;
        b.Choose("bath_lock", 13, 10);
        assert(b.bathLockDark && b.darkThisLoop == 2);
        Story c; lockAt(c); c.momAtt = Attitude::Scared;
        c.Choose("bath_lock", 13, 10);
        assert(c.bathLockDark && c.darkThisLoop == 3);
        Story d; lockAt(d); d.momRoomNow = Room::Hallway;   // still on her way: just a door
        d.Interact("door_bath", 13, 10);
        for (auto& bt : d.beats) for (auto& ch : bt.choices) assert(ch.id != "bath_lock");
        Story e; lockAt(e); e.Hold(Item::None, true); e.beats.clear();
        e.Interact("door_bath", 13, 10);
        assert(!e.beats.empty() && e.beats.front().type == BeatType::Say && !e.momBathLocked);   // no key: a hint, no lock
        std::puts("  neutral 0, knife 2, scared 3, only once she is really in there, and only with the key");
    }
    Header("Doors say which side you end up on");
    {
        Story f; Room here, there; int ox, oy;
        assert(f.DoorSides(6, 9, 6, 10, here, there, ox, oy) && here == Room::Hallway && there == Room::KidsRoom && ox == 6 && oy == 8);
        assert(f.DoorSides(13, 16, 12, 16, here, there, ox, oy) && here == Room::Kitchen && there == Room::LivingRoom && ox == 14 && oy == 16);
        assert(f.DoorSides(34, 10, 34, 11, here, there, ox, oy) && here == Room::Outside && there == Room::Shed);
        f.memory = {"loop"}; f.loop = 1; f.BeginLoop(); f.beats.clear(); f.Hold(Item::Key, true);
        f.Choose("dr_lockto:door_kid", 6, 10);
        assert(f.DoorLocked(6, 9));
        bool moved = false; for (auto& b : f.beats) if (b.type == BeatType::Fade && b.tx == 6 && b.ty == 8) moved = true;
        assert(moved);
        std::puts("  your door from the hallway: step into your room and lock it behind you");
    }
    // up to the knock, without playing the hunt yet
    auto toKnock = [&]() { while (!s.collection) { s.Tick(0.5f, false, px, py); if (!s.collection) Drain(); } };
    CleanLoop();
    Header("LOOP 13b: hinge B. Bar Dad in the shed. He goes out there first, with his torch");
    At(34, 11); WaitUntil(9); Use("sheddoor", {"s_bar"});
    assert(s.dadLocked);
    WaitUntil(49); toKnock();
    assert(s.huntStart == "shed");
    FinishLoop("caught");
    assert(s.Knows("piece_dad"));

    CleanLoop();
    Header("LOOP 13c: hinge C. In the basement at the knock. You stay down there");
    At(10, 26); Use("flag", {"fl_key"});
    At(28, 10); Use("basement", {"b_down"});
    assert(s.inBasement && s.BasementInHunt());
    WaitUntil(49); toKnock();
    assert(s.inBasement && s.huntStart.empty());
    chasePlan = "crates";
    Drain();
    assert(s.chase && s.Knows("piece_reissue"));
    At(6, 38); Use("stairs");
    assert(!s.inBasement && px == 27 && py == 11);
    At(28, 10); Use("basement");
    assert(s.inBasement && px == 6 && py == 38);
    s.hideSpot = "crates"; s.OnFound();
    Drain();
    assert(s.CaughtIn("crates") == 1);
    { Story r; r.LoadSave(s.SaveString()); assert(r.CaughtIn("crates") == 1); }

    CleanLoop();
    Header("LOOP 13a: hinge A. Lock Mom in the bathroom at 16:46. He talks to her door");
    At(10, 26); Use("flag", {"fl_key"});
    assert(s.hasKey);
    At(13, 10); WaitUntil(47);
    s.momRoomNow = Room::Bathroom;
    Use("door_bath", {"bath_lock"});
    assert(s.momBathLocked && s.bathLockDark);   // the kid is tainted by now, so she is scared of him
    WaitUntil(49); toKnock();
    assert(s.huntStart == "bath" && s.Mom().x == 13 && s.Mom().y == 5);
    qteWin = false;   // the trunk scene with every memory is LOOP 10's job
    FinishLoop("caught");
    qteWin = true;
    assert(s.Knows("piece_mom") && s.HingePieces() == 3 && s.achievements.count("Three of Five"));

    // v0.14: hinges D and E
    CleanLoop();
    Header("LOOP 13d: hinge D. Open the front door before he knocks");
    {
        Story q; q.memory = {"loop"}; q.loop = 3; q.BeginLoop(); q.beats.clear();
        q.t = 46 * 60; q.Interact("frontdoor", 20, 19);
        for (auto& bt : q.beats) for (auto& ch : bt.choices) assert(ch.id != "f_greet");   // he isn't coming yet, as far as you know
        std::puts("  no greeting before you know he comes, or before he sits back in his car");
    }
    At(20, 19); WaitUntil(48);
    { chasePlan = "crates"; int L = s.loop;
      Use("frontdoor", {"f_greet"});
      assert(s.loop == L && s.collection && s.chase && s.huntStart == "door" && s.doorGreeted);
      assert(px == 20 && py == 19 && s.Knows("piece_door"));
      s.OnCaught(); Drain(); assert(s.loop != L); }

    CleanLoop();
    Header("LOOP 13e: hinge E. Under the blanket in his car. Stay there through the knock");
    {
        Story o; o.memory = {"loop", "his_car"}; o.loop = 3; o.BeginLoop(); o.beats.clear();
        o.t = 45 * 60; o.Interact("car", 21, 29);
        bool offered = false; for (auto& bt : o.beats) for (auto& ch : bt.choices) if (ch.id == "car_hide") offered = true;
        assert(offered); o.beats.clear();
        o.Choose("car_hide", 21, 29);
        assert(o.carHidden && !o.collection && (int)o.t == o.visitorTime - Story::VISIT_OUT);
        o.beats.clear(); o.Choose("cr_out", 21, 29);
        bool moved = false; for (auto& bt : o.beats) if (bt.type == BeatType::Fade && bt.tx == 27 && bt.ty == 27) moved = true;
        assert(moved && !o.collection);
        std::puts("  slip out: back in the yard a minute before the knock, nothing ended");
    }
    At(21, 29); WaitUntil(45);
    { int L = s.loop;
      Use("car", {"car_hide", "cr_stay"});
      assert(s.loop != L && s.Knows("piece_car")); }
    assert(s.HingePieces() == 5 && s.achievements.count("Five of Five") && s.achievements.count("Open House") && s.achievements.count("Stowaway"));

    CleanLoop();
    Header("LOOP 15: outside. The backyard, the windows, the cold, the six things out back");
    {
        // the map grew north into negative rows. Nothing that was there moved
        const World& w = s.world;
        assert(w.RoomAt(6, 1) == Room::Backyard && w.RoomAt(6, -20) == Room::Backyard && w.RoomAt(20, 25) == Room::Outside);
        assert(w.tiles[World::TOP + 1][5] == Tile::Fence && w.RoomAt(29, World::TOP) == Room::Vidda && w.Walkable(29, World::TOP + 1, false));
        assert(!w.Walkable(5, -12, false) && w.Walkable(6, -12, false) && w.tiles[-12][6] == Tile::ChainGap && w.Walkable(30, -12, false));
        assert(w.Walkable(5, -3, false) && w.tiles[-3][5] == Tile::Bush);
        for (auto& win : AllWindows()) {
            assert(w.tiles[win.wy][win.wx] == Tile::Wall && w.Walkable(win.inX, win.inY, false) && w.Walkable(win.outX, win.outY, false));
            assert(IsYard(w.RoomAt(win.outX, win.outY)) && !IsYard(w.RoomAt(win.inX, win.inY)));
        }
        for (const char* id : {"oldcar", "beast", "spruce"}) assert(Story::IsHideSpot(id));
        std::puts("  backyard rows -25 to 1, bushes, chain fence with a gap, six windows that work both ways");
        // grounded: out the window is neutral (Noah)
        Story g; g.memory = {"loop"}; g.loop = 3; g.BeginLoop(); g.beats.clear(); g.grounded = true;
        int mood = g.mood, dark = g.darkPoints;
        g.Interact("window", 6, 3);
        bool offer = false, stayFirst = false;
        for (auto& bt : g.beats) if (bt.type == BeatType::Choice) { stayFirst = bt.choices.front().id == "none"; for (auto& ch : bt.choices) if (ch.id == "win_out:window") offer = true; }
        assert(offer && stayFirst && g.WindowSticks("window"));
        g.beats.clear(); g.Choose("win_out:window", 6, 3);
        bool climb = false; for (auto& bt : g.beats) if (bt.type == BeatType::Scene && bt.id == "climb:window") climb = true;
        assert(climb); g.beats.clear();
        g.OnClimbed("window", true, true);
        assert(g.mood == mood && g.darkPoints == dark && g.darkThisLoop == 0 && g.Knows("out_window") && !g.WindowSticks("window"));
        std::puts("  out the window while grounded: no mood, no dark. It sticks the first time");
        // the cold: without the jacket you don't make it across the yard, with it you can stay out
        Story c; c.memory = {"loop"}; c.loop = 3; c.BeginLoop(); c.beats.clear(); c.collection = c.chase = true; c.t = 51 * 60;
        c.Tick(4.0f, false, 20, -10); assert(c.cold > 0.45f && c.cold < 0.55f && c.chase);
        c.Tick(4.2f, false, 20, -10); assert(!c.chase && c.Knows("cold"));
        Story j; j.memory = {"loop"}; j.loop = 3; j.BeginLoop(); j.beats.clear(); j.collection = j.chase = true; j.t = 51 * 60; j.hasCoat = true;
        for (int i = 0; i < 40; ++i) j.Tick(1.0f, false, 20, -20);
        assert(j.chase && j.cold == 0);
        Story b; b.memory = {"loop"}; b.loop = 3; b.BeginLoop(); b.beats.clear(); b.collection = b.chase = true; b.t = 51 * 60; b.cold = 0.9f;
        b.kidHidden = true; b.hideSpot = "beast"; b.Tick(3.0f, false, 15, -6);
        assert(b.cold < 0.5f && b.achievements.count("Smells Worse Inside"));
        std::puts("  no jacket: frozen in about 8 seconds outside. Jacket: never. Inside the snow beast: warm");
        // a nod, and a dog that isn't there
        Story n; n.memory = {"loop"}; n.loop = 3; n.BeginLoop(); n.beats.clear();
        n.Interact("flag", 10, 26); n.beats.clear(); n.Choose("fl_lick", 10, 26);
        assert(n.achievements.count("Triple Dare") && n.tongueStuck);
        n.OnHusky(); assert(n.achievements.count("Not a Dog"));
        n.t = 46 * 60; n.Interact("vane", 32, 3); assert(n.achievements.count("Same Day Again"));
        n.tainted = true; assert(n.BoardSpells()); n.beats.clear(); n.Interact("lights", 34, 1); assert(n.achievements.count("Spelled Out"));
        std::puts("  tongue on the flagpole, the dog at the fence, the weathervane, the board that spells BELOW");
        // v0.15.1 (Noah): in loop 1 nothing bad happens. Walking out of the gate or the back fence isn't running away
        Story t1; t1.NewGame(); t1.BeginLoop(); t1.beats.clear();
        assert(t1.Tutorial());
        for (int i = 0; i < 4; ++i) { t1.Tick(1.0f, false, 20, 33); t1.Tick(1.0f, false, 29, World::TOP); }
        assert(!t1.collection && !t1.Knows("run_fail") && t1.loop == 1);
        t1.beats.clear(); t1.TooColdToLeave(); t1.TooColdToLeave();
        assert(t1.beats.size() == 2 && t1.beats[0].text != t1.beats[1].text && t1.beats[0].text.find("Zulda") != std::string::npos);
        std::puts("  loop 1: the vidda does nothing, the kid turns back (too cold, Zulda is waiting)");
    }
    // the main story: out your window, the six things out back, then his torch in the dark
    At(6, 3); Use("window", {"win_out:window"});
    assert(px == 6 && py == 1 && s.Knows("out_window"));
    At(9, -7); Use("oldcar", {"oc_look"});
    At(21, -6); Use("swing", {"sw_swing"});
    At(23, -10); Use("smallprints");
    At(4, -2); Use("dish");
    At(34, -15); Use("grave");
    At(26, -20); Use("icehole", {"ih_rope"});
    assert(s.Knows("small_prints") && s.Knows("grave_05") && s.EggsFound() == 6 && s.achievements.count("Out Back"));
    At(15, -6); Use("beast", {"bst_in"});
    At(6, 1); Use("window", {"win_in:window"});
    assert(px == 6 && py == 3);
    FinishLoop("litup");
    assert(s.Knows("lit_up"));
    CleanLoop();
    FinishLoop("taser");
    assert(s.Knows("taser"));
    CleanLoop();
    Header("LOOP 16: v0.16. Doggo goes to the door, parents get tired of the same question, Dad's jokes");
    {
        Story m; m.memory = {"loop"}; m.loop = 3; m.BeginLoop(); m.beats.clear();
        for (int i = 0; i < 4; ++i) { m.Choose("m_dinner", 8, 15); assert(!m.grounded); }
        m.Choose("m_dinner", 8, 15); assert(m.grounded && m.achievements.count("Asked and Answered"));
        Story d; d.memory = {"loop"}; d.loop = 3; d.BeginLoop(); d.beats.clear();
        for (int i = 0; i < 5; ++i) d.Choose("d_storm", 35, 7);
        assert(d.dadAtt == Attitude::Cold && d.nagDad == 4);
        for (int i = 0; i < 3; ++i) d.Choose("d_joke", 35, 7);
        assert(d.achievements.count("Dad Jokes"));
        Story g; g.memory = {"loop"}; g.loop = 3; g.BeginLoop(); g.beats.clear(); g.collection = g.chase = true;
        g.OnDoggoAtDoor(); g.OnDoggoCalled(); assert(g.Knows("doggo_door") && g.achievements.count("Called Back") && g.DoggoExists());
        std::puts("  five times the same question: grounded by Mom, sent off by Dad. Three jokes. Doggo called back in time");
    }
    FinishLoop("doggodoor");
    assert(s.Knows("doggo_door"));
    CleanLoop();
    FinishLoop("empty");
    assert(s.Knows("nobody_left") && s.endings.count("empty") && s.achievements.count("Nobody Left"));

    // v0.17: answer 10 and the true ending (branches map, section 6)
    assert(s.HingePieces() == 5);
    CleanLoop();
    Header("LOOP 17a: all three traps, tie him up with the tripwire, riddles until the bells");
    s.trapsSet = {"marbles", "tripwire", "paint"}; s.traps = 3;
    { int L = s.loop; FinishLoop("tie"); assert(s.loop != L && s.Knows("tied_up") && s.achievements.count("Knots")); }
    CleanLoop();
    Header("LOOP 17b: two traps, Mom and Dad right there. Tell him all five (last pickup first), then spare him");
    s.trapsSet = {"marbles", "tripwire"}; s.traps = 2;
    trueQueue = {"t_last", "t_hum", "t_watch", "t_reissue", "t_dose", "t_spare"};
    FinishLoop("true");
    assert(s.Knows("he_remembers") && s.endings.count("after") && endingsSeen.back() == "after");
    CleanLoop();
    Header("LOOP 17c: knife, Mom bolted in the basement, Dad barred in the shed. Tell him all five, then finish it");
    s.trapsSet = {"marbles", "tripwire"}; s.traps = 2; s.Hold(Item::Knife); s.momLocked = s.dadLocked = true;
    trueQueue = {"t_hum", "t_watch", "t_reissue", "t_dose", "t_last", "t_kill"};
    FinishLoop("true");
    assert(s.endings.count("samehouse") && endingsSeen.back() == "samehouse" && s.tainted && s.darkPoints == DARK_MAX);
    TrueEndingTest();

    // v0.18: whose side are they on (branches map, section 3, and Noah's answer 7)
    CleanLoop();
    Header("LOOP 18a: Dark. They help him look. Mom calls out where you are, Dad holds the door and grabs you");
    s.Hold(Item::Knife); Use("mom", {"m_knife"});
    assert(s.CurrentStance() == Stance::Dark);
    FinishLoop("grab");
    assert(s.Knows("they_hunt") && s.achievements.count("Search Party"));
    CleanLoop();
    Header("LOOP 18b: tell only Mom. She goes for his belt, and the box slides over to you");
    chores();
    At(8, 15); Use("mom", {"m_ask", "m_loop"});
    assert(s.momTurned && !s.dadTurned);
    FinishLoop("defend_mom");
    assert(s.Knows("mom_fought") && s.achievements.count("Just Her"));
    CleanLoop();
    Header("LOOP 18c: tell only Dad. He tears the taser out of his hand. Use it on him");
    chores();
    At(35, 7); Use("dad", {"d_ask", "d_loop"});
    assert(s.dadTurned && !s.momTurned);
    FinishLoop("defend_dad");
    assert(s.Knows("dad_fought") && s.Knows("tased_him") && s.achievements.count("His Own Medicine"));
    CleanLoop();
    Header("LOOP 18d: both of them on your side, and him on the floor at the door. Hold him down (Last Toll)");
    chores();
    At(8, 15); Use("mom", {"m_ask", "m_loop"});
    At(35, 7); Use("dad", {"d_ask", "d_loop"});
    assert(s.parentsAllied);
    s.trapsSet = {"marbles", "tripwire"}; s.traps = 2;
    FinishLoop("c_help");
    assert(s.Knows("last_toll") && endingsSeen.back() == "lasttoll");

    // v0.19: before the knock (branches map, section 4)
    CleanLoop();
    Header("LOOP 19a: barricade your door, tripwire at the living room door, his key in the snow, tracks out the back");
    {
        At(6, 6); Use("door_kid", {"bar_push:door_kid"});
        assert(s.Barricaded(6, 9) && s.DoorClosed(6, 9));
        At(6, 6); Use("door_kid", {"none"});                                    // from inside you can still pull it away (not now)
        At(32, 8); Use("rope", {"a_tripwire_in"});
        assert(s.innerTraps.count("tripwire") && !s.trapsSet.count("tripwire") && s.traps == 0);
        WaitUntil(43);
        s.t = 43 * 60 + 50;
        At(20, 29); Use("car", {"car_key", "ck_snow"});
        assert(s.carKey == "snow" && !s.EngineOn() && s.achievements.count("No Ride Home"));
        At(29, -24); Use("gap", {"tk_walk"});
        assert(s.falseTracksAt > 0);
    }
    FinishLoop("stage");
    assert(s.Knows("thirty") && s.Knows("hand_me_down") && s.achievements.count("Stage Manager") && s.achievements.count("Breadcrumbs"));
    Header("Before the knock: the storm eats an early trail, and the other two places for his key");
    {
        Story k; k.memory = {"loop", "visitor_time"}; k.loop = 5;
        k.BeginLoop(); k.beats.clear(); while (k.shift != Shift::None) { k.EndLoop(); k.BeginLoop(); k.beats.clear(); }
        k.t = 20 * 60; k.Choose("tk_walk", 29, -24); k.beats.clear();
        k.t = 49 * 60 + 59; k.Tick(1.0f, false, 21, 17);
        bool told = false; for (auto& b : k.beats) if (b.type == BeatType::Narrate && b.text.find("tracks out back") != std::string::npos) told = true;
        assert(!k.falseTracks && told);
        for (const char* where : {"pocket", "woodpile"}) {
            Story c; c.memory = {"loop", "visitor_time"}; c.loop = 5;
            c.BeginLoop(); c.beats.clear(); while (c.shift != Shift::None) { c.EndLoop(); c.BeginLoop(); c.beats.clear(); }
            c.t = 43 * 60 + 50; c.Interact("car", 20, 29);
            bool offered = false; for (auto& o : c.beats.back().choices) if (o.id == "car_key") offered = true;
            assert(offered); c.beats.clear();
            c.Choose("car_key", 20, 29); c.beats.clear(); c.Choose(std::string(where) == "pocket" ? "ck_pocket" : "ck_wood", 20, 29); c.beats.clear();
            int knockBefore = c.visitorTime;
            while (!c.collection) { c.Tick(0.5f, false, 21, 17); while (!c.beats.empty() && c.beats.front().type != BeatType::Scene) c.beats.pop_front(); }
            assert(c.visitorTime < knockBefore);                               // he finds a dead car and comes straight up
            c.beats.clear(); c.chase = true; c.OnCaught();
            bool line = false; for (auto& b : c.beats) if (b.type == BeatType::Black && b.text.find(std::string(where) == "pocket" ? "holds out his hand" : "woodpile") != std::string::npos) line = true;
            assert(line);
        }
        // the other two inside traps: marbles at your door, the paint can over the shed door
        Story m; m.memory = {"loop", "visitor_time"}; m.loop = 5; m.darkPoints = 4;
        m.BeginLoop(); m.beats.clear(); while (m.shift != Shift::None) { m.EndLoop(); m.BeginLoop(); m.beats.clear(); }
        m.Choose("a_marbles_in", 3, 7); m.Choose("a_paint_in", 36, 9); m.beats.clear();
        assert(m.InnerTrapAt(6, 9) == "marbles" && m.InnerTrapAt(34, 10) == "paint" && m.InnerTrapAt(20, 12).empty());
        m.Choose("a_marbles", 3, 7); assert(!m.trapsSet.count("marbles"));   // one jar, one place
        std::puts("  early trail filled in, key in the pocket and the woodpile, inside traps placed");
    }

    // v0.20 story: endings that teach, the watch, more questions, the TV, stranger shifts, early lessons, predictions
    {
        auto offers = [](Story& st, const char* menu, const char* opt) {
            st.beats.clear(); st.Choose(menu, 8, 15);
            bool f = false; for (auto& b : st.beats) if (b.type == BeatType::Choice) for (auto& c : b.choices) if (c.id == opt) f = true;
            st.beats.clear(); return f;
        };
        auto said = [](Story& st, const char* bit) {
            for (auto& b : st.beats) if (b.text.find(bit) != std::string::npos) return true;
            return false;
        };
        qteWin = false;   // the secret ending waits for LOOP 10, as before
        CleanLoop();
        Header("LOOP 20a: three games in a row. Screen Time teaches you something about him");
        At(8, 7); Use("fliptendo", {"ft_flappy"}); Use("fliptendo", {"ft_flappy"}); Use("fliptendo", {"ft_flappy"});
        assert(s.Knows("under_well") && s.endings.count("screen"));
        CleanLoop();
        Header("LOOP 20b: the watch. Look at it in bed, show Dad the tiny one, listen to the console, say the word");
        At(5, 6); Use("bed", {"bed_watch"});
        assert(s.Knows("w_beep"));
        At(10, 26); Use("flag", {"fl_key"});
        At(27, 5); Use("drawer", {"cab_watch"});
        At(24, 17); Use("dad", {"d_watch"});
        assert(s.Knows("w_grow") && s.dadAtt == Attitude::Cold);
        At(27, 5); Use("drawer", {"it_back"}); Use("drawer", {"cab_key"});
        assert(s.hasKey);
        At(28, 10); Use("basement", {"b_down"});
        At(10, 38); Use("console", {"r_listen"});
        assert(s.Knows("unit_04"));
        s.beats.clear(); s.Interact("console", px, py);
        { bool f = false; for (auto& c : s.beats.back().choices) if (c.id == "r_confirm") f = true; assert(f); }
        s.beats.clear(); Use("console", {"r_confirm"});
        At(20, 29); WaitUntil(45);
        s.beats.clear(); s.Interact("visitor", px, py);
        { bool f = false; for (auto& c : s.beats.back().choices) if (c.id == "v_ask") f = true; assert(f); }
        s.beats.clear();
        for (const char* v : {"v_well", "v_dog", "v_cameout", "v_twice", "v_after"}) assert(offers(s, "v_ask", v));
        Use("visitor", {"v_ask", "v_after"});
        FinishLoop("caught");
        CleanLoop();
        Header("LOOP 20c: early lessons. Hit Dad, shove Mom. It barely works");
        At(24, 17); Use("dad", {"d_hit"});
        assert(s.dadAtt != Attitude::Normal);
        At(8, 15); Use("mom", {"m_shove"});
        assert(s.momAtt != Attitude::Normal && s.Knows("small_hands"));
        FinishLoop("caught");
        CleanLoop();
        Header("LOOP 20d: the newer memories, asked out loud");
        for (const char* q : {"m_hinges", "m_brother", "m_thirty", "m_before", "m_door", "m_lived", "m_plate", "m_deliver"}) { assert(offers(s, "m_ask", q)); s.Choose(q, 8, 15); Drain(); }
        for (const char* q : {"d_knot", "d_hinges", "d_nail", "d_reissue", "d_coat", "d_ice", "d_0612", "d_hide", "d_almond"}) { assert(offers(s, "d_ask", q)); s.Choose(q, 24, 17); Drain(); }
        FinishLoop("caught");
        std::puts("  every new question offered and answered");

        Header("Predictions: say it before it happens. Uneasy, then cold, then scared");
        {
            Story p; p.memory = {"loop", "phone_mom", "dad_key", "visitor_time"}; p.seenEvents = {"storm"}; p.loop = 5;
            p.BeginLoop(); p.beats.clear(); while (p.shift != Shift::None) { p.EndLoop(); p.BeginLoop(); p.beats.clear(); }
            p.t = 60;
            assert(offers(p, "m_ask", "mp_phone") && offers(p, "d_ask", "dp_knock"));
            p.Choose("mp_phone", 8, 15); assert(p.momAtt == Attitude::Normal && p.predMom == 1);
            assert(!offers(p, "m_ask", "mp_phone"));                // once a loop
            p.Choose("mp_flag", 8, 15); assert(p.momAtt == Attitude::Cold);
            p.Choose("m_sorry", 8, 15); assert(p.momAtt == Attitude::Normal);
            p.Choose("mp_storm", 8, 15); assert(p.momAtt == Attitude::Scared);
            p.Choose("dp_knock", 24, 17); assert(p.dadAtt == Attitude::Normal && p.predDad == 1);
            assert(p.achievements.count("Fortune Teller") && p.CurrentStance() == Stance::Dark);
            p.t = 47 * 60 + 5; assert(!offers(p, "d_ask", "dp_storm"));   // too late, it's here
            std::puts("  phone, flagpole, storm and knock. Mom ends up scared of you");
        }
        Header("Two stranger realities: the photos, and oh-seven");
        {
            Story f; f.memory = {"loop"}; f.loop = 6;
            gShiftCfg.forceNext = Shift::Fourth; f.BeginLoop();
            assert(f.shift == Shift::Fourth && said(f, "somebody else's coat")); f.beats.clear();
            f.Interact("album", 17, 17); assert(f.fourthNoticed && f.achievements.count("Family Portrait") && f.Knows("no_baby"));
            assert(offers(f, "m_ask", "m_fourth") && offers(f, "d_ask", "d_fourth"));
            Story u; u.memory = {"loop"}; u.loop = 6;
            gShiftCfg.forceNext = Shift::NextUnit; u.BeginLoop();
            assert(u.shift == Shift::NextUnit && said(u, "Oh-seven")); u.beats.clear();
            u.Interact("desk", 27, 5); assert(u.Knows("reports") && said(u, "REISSUED"));
            u.beats.clear(); u.Choose("d_unit", 24, 17); assert(u.achievements.count("Next in Line"));
            std::puts("  a man grows up in the photos. Dad calls you oh-seven");
        }
        Header("The TV: what's on depends on how the last hour ended");
        {
            struct E { const char* end; const char* bit; };
            for (E e : {E{"cold", "fox"}, E{"vidda", "31"}, E{"ending:doggo", "station"}, E{"trunk", "magician"}}) {
                Story v; v.memory = {"loop", "handlers", "static_tv"}; v.loop = 6; v.lastEnd = e.end;
                v.BeginLoop(); v.beats.clear(); while (v.shift != Shift::None) { v.lastEnd = e.end; v.loop += 1; v.BeginLoop(); v.beats.clear(); }
                if (v.loop % 2) { v.lastEnd = e.end; v.BeginLoop(); v.beats.clear(); }
                v.Interact("tv", 20, 16); assert(said(v, e.bit));
            }
            Story w; w.memory = {"loop", "handlers"}; w.loop = 6; w.lastEnd = "cold"; w.BeginLoop(); w.beats.clear();
            w.shift = Shift::None; w.Interact("tv", 20, 16); assert(w.Knows("static_tv"));   // Previously On still comes first
            std::puts("  a fox in the snow, a road sign, a dog at a station, a boy in a box");
        }
        Header("No Soul: tainted, they can tell");
        {
            Story n; n.memory = {"loop"}; n.loop = 5; n.darkPoints = 9; n.tainted = true;
            n.BeginLoop(); n.beats.clear(); n.shift = Shift::None; n.loop = 6;
            n.Interact("mom", 8, 15); assert(said(n, "Your eyes do the smile") || said(n, "laugh at the TV") || said(n, "gone out of him"));
            n.beats.clear(); n.Interact("dad", 24, 17); assert(said(n, "buddy") || said(n, "They said"));
        }
    }

    Header("Clean key route, the findable one: watch Dad at the flagpole at 16:44");
    {
        Story k; k.memory = {"loop"}; k.loop = 2;
        k.BeginLoop(); k.beats.clear();
        while (k.t < 45 * 60 && !k.collection) { k.Tick(0.5f, false, 13, 27); k.beats.clear(); }
        assert(k.SawEvent("flag44") && k.KnowsEvent("flag44"));
        k.Interact("flag", 10, 26);
        assert(k.hasKey && k.Knows("dad_key") && !k.achievements.count("Looked It Up"));
        std::puts("  saw him check the box, took the key, no accusation");
    }
    Header("The narrator: reacts to the last loop, and nudges after three dry ones");
    {
        Story o; o.memory = {"loop"}; o.loop = 5; o.lastEnd = "cold"; o.dryLoops = 3;
        o.BeginLoop();
        bool react = false, nudge = false;
        for (auto& b : o.beats) {
            if (b.type == BeatType::Narrate && b.text.find("snow") != std::string::npos) react = true;
            if (b.type == BeatType::Narrate && b.text.find("circles") != std::string::npos) nudge = true;
        }
        assert(react && nudge && o.dryLoops == 0 && o.lastEnd.empty());
        Story p; p.LoadSave(s.SaveString());
        assert(p.seenEvents == s.seenEvents && p.dryLoops == s.dryLoops);
        assert(s.KnowsEvent("knock") && s.KnowsEvent("bells") && s.KnowsEvent("car"));
        size_t known = 0; for (auto& e : AllTimeline()) if (s.KnowsEvent(e.id)) known++;
        std::printf("  timeline: %zu of %zu known\n", known, AllTimeline().size());
        for (auto& a : s.achievements) { bool listed = false; for (auto& e : AllAchievements()) if (e.name == a) listed = true; assert(listed); }
    }

    std::printf("\nMemory: %zu / %zu\n", s.memory.size(), AllMemories().size());
    for (auto& e : AllMemories()) if (!s.Knows(e.id)) std::printf("  MISSING %s\n", e.id.c_str());

    CleanLoop();
    Header("LOOP 10: 100% memory, stay awake in the trunk (secret)");
    qteWin = true;
    FinishLoop("caught");

    Header("Monster ending check on a fresh story with only darkness");
    {
        Story m; m.loop = 1; m.darkPoints = 10; m.tainted = true; m.memory = {"dead_switch"};
        m.BeginLoop(); m.beats.clear();
        m.trapsSet = {"marbles", "tripwire"}; m.traps = 2; m.Hold(Item::Knife);
        m.momLocked = m.dadLocked = true;   // v0.18: with them standing there it's The Handlers instead
        m.t = 49 * 60 + 59; m.Tick(1.0f, false, 21, 17);
        bool chase = false, ending = false;
        for (int g = 0; g < 400 && !m.beats.empty(); ++g) {
            Beat b = m.beats.front(); m.beats.pop_front();
            if (b.type == BeatType::Scene && b.id == "chase") { chase = true; m.Interact("visitor", 21, 17); }
            if (b.type == BeatType::Choice) m.Choose(b.choices.front().id, 21, 17);
            if (b.type == BeatType::Ending) { ending = true; std::printf("  ### ENDING: %s\n", b.id.c_str()); endingsSeen.push_back(b.id); }
        }
        assert(chase && ending);
    }

    Header("Doggo key route: tainted, \"Find it, boy!\"");
    {
        Story g; g.loop = 3; g.darkPoints = 8; g.tainted = true;
        g.BeginLoop(); g.beats.clear();
        g.Choose("g_search", 25, 22);
        assert(g.hasKey && g.Knows("dad_key"));
        std::puts("  Doggo dug up the basement key");
    }

    Header("Clean key route: third look at the flagpole at 16:44");
    {
        Story k; k.memory = {"dad_goodbye", "loop"}; k.loop = 2;
        k.BeginLoop(); k.beats.clear();
        k.Interact("flag", 10, 26); k.Interact("flag", 10, 26);
        k.t = 44 * 60 + 5;
        k.Interact("flag", 10, 26);
        assert(k.hasKey && k.Knows("dad_key") && k.achievements.count("Looked It Up"));
        std::puts("  clean key OK, narrator accused us of cheating");
    }

    std::printf("\nLoops ended: %d, achievements: %zu, dark points: %d, tainted: %d\n",
                loopsEnded, s.achievements.size(), s.darkPoints, s.tainted);
    for (auto& a : s.achievements) std::printf("  - %s\n", a.c_str());
    std::printf("Save: %s\n", s.SaveString().c_str());

    // New game keeps the darkness
    CleanLoop();
    Header("LOOP: all five chores, Zulda, Screen Time");
    { int startLoop = s.loop;
      At(8, 6); Use("toys");
      At(11, 18); Use("trashcan");
      At(5, 22); Use("bins");
      At(4, 14); Use("pantry", {"pa_food"});
      At(5, 18); Use("bowl");
      At(8, 14); Use("sink");
      At(29, 25); Use("woodpile", {"wp_stack"});
      assert(s.ChoresDone() == Story::CHORES && s.t < 40 * 60);
      WaitUntil(41);
      assert(s.earnedPresent);
      At(25, 16); Use("tree", {"tr_zulda"});
      assert(s.zuldaOpened);
      (void)startLoop;
    }
    FinishLoop("caught");
    CleanLoop();
    Header("LOOP: three games in a row");
    At(8, 7); Use("fliptendo", {"ft_flappy"}); Use("fliptendo", {"ft_flappy"}); Use("fliptendo", {"ft_flappy"});
    assert(s.endings.count("screen"));

    Story n; n.LoadSave(s.SaveString()); n.NewGame();
    assert(n.tainted && n.darkPoints == s.darkPoints && n.memory.empty());
    Story r; r.LoadSave(s.SaveString());
    assert(r.memory == s.memory && r.endings == s.endings);
    // v0.9: seen lines survive a save, and the tape changes her name
    { Story a; a.MarkSeen("hello"); Story b; b.LoadSave(a.SaveString()); assert(b.SeenLine("hello") && !b.SeenLine("nope"));
      Story c; c.memory = {"the_tape"}; assert(std::string(c.MomName()) == "Handler two"); }

    bool all = s.memory.size() == AllMemories().size();
    std::printf("Endings seen: ");
    for (auto& e : endingsSeen) std::printf("%s ", e.c_str());
    std::printf("\n%s\n", all ? "ALL MEMORIES REACHABLE" : "INCOMPLETE");
    return all ? 0 : 1;
}
