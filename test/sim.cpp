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
            if (b.id == "dad_runs") s.SceneDone("dad_runs");
            else if (b.id == "minigame") s.MinigameDone(4);
            else if (b.id == "chase") {
                if (chasePlan == "caught") s.OnCaught();
                else if (chasePlan == "hide") s.kidHidden = true;        // stay hidden until 17:00
                else if (chasePlan == "run") s.kidHidden = false;        // keep running until 17:00
                else if (chasePlan == "doggo") { s.doggoWithKid = true; s.OnCaught(); }
                else if (chasePlan == "found") s.OnFound();
                else if (chasePlan == "cold") { At(25, 25); s.kidHidden = false; }   // stand out in the storm, no jacket
                else if (chasePlan == "outside") s.OnLeftPremises();
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
    Story th; th.loop = 3; th.BeginLoop(); th.beats.clear(); th.collection = th.chase = true; th.t = 57 * 60 + 2;
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
    assert(s.parentsAllied && s.Knows("allies"));
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
