// 600 Seconds - Story logic
// Engine-free C++. The raylib layer only feeds it input and draws what it says.
// Everything here should move to Unreal (or anything else) without changes.
#pragma once
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "World.h"

namespace sixhundred {

// ---------------------------------------------------------------- beats
// The story talks to the presentation layer through a queue of "beats".
// Free roaming only happens while the queue is empty.
enum class BeatType {
    Say,         // speaker + text in the dialogue box (kid font)
    Narrate,     // the narrator, adult font, over a dimmed screen
    Black,       // text on a black screen (off-screen events)
    Fade,        // fade to black, the watch jumps `minutes`, optional teleport
    Choice,      // pick one option, presentation calls Story::Choose(id)
    Achievement, // popup
    Clue,        // "new memory" popup (id = memory entry id)
    Sound,       // id = beep, knock, bang, bark, ring, thud, glass, crack, bells
    EndLoop,     // Watch beep, Story::EndLoop() is called, new loop starts
    Card,        // big title card (speaker = title)
    Scene,       // real-time scene, id = dad_runs | chase. Presentation calls SceneDone(id)
    QTE,         // mash E. Presentation calls QTEResult(bool)
    Ending,      // ending card, then back to the title screen (id = ending id)
    Flash,       // white flash
    Bubble,      // non-blocking speech bubble over speaker's head (v0.4)
};

struct ChoiceOpt {
    std::string id;
    std::string text;
    bool memory = false; // knowledge check option, drawn with a memory icon
    bool dark = false;   // dark option, drawn in red
};

struct Beat {
    BeatType type = BeatType::Say;
    std::string speaker;
    std::string text;
    std::string id;
    int minutes = 0;
    int tx = -1, ty = -1; // teleport target for Fade
    std::vector<ChoiceOpt> choices;
};

// ---------------------------------------------------------------- memory log
struct MemoryEntry {
    std::string id;
    std::string topic;
    std::string title;
    std::string text;
    std::string hint; // shown as "?" card text before it's found
};

const std::vector<MemoryEntry>& AllMemories();
const std::vector<std::string>& AllTopics();

// ---------------------------------------------------------------- npcs
struct NpcState {
    int x = 0, y = 0;
    bool visible = true;
    bool facingWall = false;
};

enum class Shift { None, Toilet45, NoDoggo, EarlyStorm, Sister };

// v0.5: how a parent feels about you right now (per loop). Big choices lock them into a state,
// so you can't chat about dinner with someone you just pulled a knife on.
enum class Attitude { Normal, Cold, Scared, Broken };

// v0.6: the kid holds one thing at a time
enum class Item { None, Knife, Key, Stick, Bag, DogFood, Flashlight, Tape, BabyWatch };

// v0.7: how the parents act when he comes. Normal = reluctantly hand you over (they don't know they're
// next). Dark = they want him gone. Befriended = all chores, nothing dark this loop: they fight for you.
enum class Stance { Normal, Dark, Befriended };

// v0.7: every door in the house can be closed. Only the master key locks them.
struct DoorInfo { int x, y; const char* id; const char* name; };
const std::vector<DoorInfo>& AllDoors();
const DoorInfo* DoorById(const std::string& id);
const DoorInfo* DoorAt(int x, int y);
const char* ItemName(Item it);

struct EndingEntry { std::string id, title, hint; };
const std::vector<EndingEntry>& AllEndings();

// v0.10: the hour as a list. An entry shows once you have seen it happen (or hold the memory that proves it)
struct TimelineEntry { int minute, second; std::string id, text, memory; };
const std::vector<TimelineEntry>& AllTimeline();
// v0.10: every achievement, with a nudge for the ones you don't have
struct AchievementEntry { std::string name, hint; };
const std::vector<AchievementEntry>& AllAchievements();

// v0.4.2: how often reality shifts. Tweakable live from the debug panel (O with ?debug).
// Default: never two anomalies within 5 loops, and one is guaranteed within 10.
struct ShiftConfig {
    bool enabled = true;
    bool toilet = true;          // the 45 minute toilet
    bool noDoggo = true;         // the reality without Doggo
    bool earlyStorm = true;      // v0.9: the storm at 16:37, the power out at 16:40
    bool sister = true;          // v0.10: somebody is in the bathroom. Not saved, no panel row yet
    int chance = 20;             // % chance per loop once the cooldown is over
    int minGap = 4;              // normal loops needed after an anomaly before the next one can happen
    int maxGap = 9;              // after this many normal loops one is guaranteed (0 = never forced)
    int firstLoop = 3;           // nothing weird before this loop
    int toiletShare = 60;        // % of anomalies that are the toilet (the rest: no Doggo)
    Shift forceNext = Shift::None; // one shot, not saved
    float timeMul = 1.0f;        // v0.5.1: game clock speed, 1.0x to 2.0x (debug panel)
    std::string Save() const;
    void Load(const std::string& s);
};
extern ShiftConfig gShiftCfg;
void SeedShifts(unsigned seed);
const char* ShiftName(Shift s);

// Dark points are a soft counter, 0 to 15.
// 0-5: weird kid behaviour. 6+: tainted for good. 15: as dark as it gets.
constexpr int DARK_ARSENAL = 3;
constexpr int DARK_TAINT = 6;
constexpr int DARK_INTERROGATE = 8;
constexpr int DARK_REVEL = 13;
constexpr int DARK_MAX = 15;

// ---------------------------------------------------------------- story
class Story {
public:
    static constexpr float TIME_SCALE = 5.0f;   // game seconds per real second
    static constexpr float WAIT_SCALE = 120.0f; // while holding the wait key (ramps up to 300)
    static constexpr int HOUR = 3600;

    World world;
    std::deque<Beat> beats;

    // ---- persistent (survives loops and page reloads)
    int loop = 0;
    std::set<std::string> memory;
    std::set<std::string> achievements;
    std::set<std::string> endings;
    bool seenToilet45 = false;
    bool prototypeEndShown = false;
    int cleanStreak = 0;       // normal loops in a row since the last reality shift
    std::set<unsigned> seenLines;   // v0.9: hashes of every line already read (so it can be skipped)
    // v0.10 persistent
    std::set<std::string> seenEvents;   // things you watched happen: they fill in the timeline
    int dryLoops = 0;                   // loops in a row without a new memory (the narrator nudges at 3)
    std::string lastEnd;                // how the last loop ended, so the next one can mention it
    // v0.12 persistent: the spots he has pulled you out of. He checks those first, every loop after
    std::map<std::string, int> caughtSpots;
    // ---- darkness. Survives even a New Game. You can't undo it.
    int darkPoints = 0;
    bool tainted = false;

    // ---- per loop
    float t = 0;               // game seconds since 16:00
    float waitHeld = 0;
    Attitude momAtt = Attitude::Normal, dadAtt = Attitude::Normal;   // v0.5 reaction states        // v0.5: how long T has been held (wait ramps up)
    Shift shift = Shift::None;
    int mood = 0;
    bool fed = false, hasBag = false, trashDone = false, roomClean = false;
    bool pooped = false;
    bool allowanceChecked = false;
    bool grounded = false;
    int visitorTime = 50 * 60;
    // v0.9.2: his routine, in game seconds before the knock
    static constexpr int VISIT_ARRIVE = 420, VISIT_WALK = 400, VISIT_LOOK = 340, VISIT_BACK = 210, VISIT_IN_CAR = 180, VISIT_OUT = 60;
    bool visitorLooked = false;   // he got to the window this loop, so his tracks are out there
    bool collection = false;
    bool phoneAnswered = false;
    bool helpedDad = false;
    std::set<std::string> firedEvents;
    // v0.2 per loop
    bool hasKnife = false, hasKey = false;   // mirrors of `held` (kept for readability)
    // v0.6 per loop
    Item held = Item::None;
    bool hasStick = false, hasFood = false, hasLight = false;
    bool dishesDone = false, firewoodDone = false;
    bool earnedPresent = false, zuldaOpened = false, knifeTakenLoop = false;
    int playStreak = 0;        // Fliptendo sessions in a row (3 = Screen Time)
    bool lightUsed = false;    // flashlight in his eyes, once per loop
    float visitorStun = 0;     // seconds the visitor is blinded (main counts it down)
    std::vector<std::string> loopNew;      // memories found this loop (for the recap)
    std::vector<std::string> lastLoopNew;  // what the recap shows after a reset
    bool frontLocked = false, momLocked = false, dadLocked = false;
    bool doggoGone = false, doggoOut = false;
    bool dadDown = false;      // shot during the collection
    bool inBasement = false;
    int traps = 0;             // marbles, tripwire, paint can
    float frontLockedAt = 0;
    std::set<std::string> trapsSet;
    int flagTouches = 0;
    bool chase = false;        // real-time collection is running
    bool visitorInjured = false, visitorDowned = false;
    bool kidShot = false;
    bool switchTaken = false;  // took the dead man's switch off his belt (v0.4)
    // v0.7 per loop
    std::set<int> closedDoors, lockedDoors, brokenDoors;   // key: y * World::W + x
    bool basementUnlocked = false, drawerOpen = false, keyInDrawer = false;
    bool doggoStay = false, doggoDead = false;
    int doggoDeadX = -1, doggoDeadY = -1;      // where the body ends up (-1 = where he is)
    int darkThisLoop = 0;                      // any dark act this loop ruins "befriended"
    bool parentsAllied = false;                // you convinced them about the clean-up
    Stance stance = Stance::Normal;            // set when he knocks
    int threats = 0;                           // 16:57, 16:58, 16:59
    bool dadOnFloor = false, momOnFloor = false;
    bool kidHidden = false;                    // set by the presentation every frame of the chase
    bool doggoWithKid = false;
    bool visitorChasing = false;               // he can see you right now: the threats wait                 // same: Doggo is in the room with you and not told to stay
    // v0.8 per loop
    bool hasCoat = false, openedEarly = false;
    float cold = 0;                            // 0 to 1 outside in the storm during the chase. 1 = the snow gets you
    // v0.10 per loop
    bool phoneUnplugged = false;               // no call, no confirmation
    bool momRemorse = false;                   // you hummed it back to her
    bool sisterNoticed = false;
    bool echoAsked = false;
    // v0.12 per loop: the hunt
    std::string hideSpot;                      // set by the presentation: the object the kid is hiding in
    int pebbles = 0;                           // marbles in your pocket, to throw
    int decoys = 0;                            // how many he has fallen for this loop
    std::string pendingKnifeAct;
    int knifeChecks = 0;
    // v0.13 per loop: the hinges. Each one changes the knock and where the hunt starts
    bool momBathLocked = false;                // hinge A: Mom locked in the bathroom (16:46 to 16:48)
    bool bathLockDark = false;                 // she was scared, or you had the knife: that one counts
    std::string huntStart;                     // "", "bath", "shed" or "basement": the presentation reads it
    Room momRoomNow = Room::Kitchen;           // set by the presentation: where Mom really stands right now
    // v0.14 per loop: hinges D and E
    bool doorGreeted = false;                  // hinge D: you opened the front door for him before he knocked
    bool carHidden = false;                    // hinge E: you sat under the blanket in his car while he was in it
    // v0.15 per loop: the outside
    bool windowBanged = false;                 // your window sticks in the cold. The first time each loop it needs a bang
    bool tongueStuck = false;                  // the flagpole. Once a loop is plenty
    bool backyardCold = false;                 // the first time out back without the jacket you notice

    void NewGame();            // wipes progress, keeps the darkness
    void LoadSave(const std::string& s);
    std::string SaveString() const;

    void BeginLoop();
    Shift RollShift();       // decides this loop's reality (uses gShiftCfg)
    void DebugSetDark(int v);
    void DebugAllMemories();
    void DebugToBasement();   // v0.9.3: go down with the tape in your hand
    void DebugClearMemories();
    void EndLoop();

    // Called every frame while free roaming (beat queue empty).
    void Tick(float realDt, bool waiting, int px, int py);
    // Jump the clock (called by the presentation during a Fade beat).
    void AdvanceMinutes(int minutes, int px, int py);

    void Interact(const std::string& id, int px, int py);
    void Choose(const std::string& id, int px, int py);

    // ---- collection hooks (called by the presentation)
    void SceneDone(const std::string& id);
    void OnCaught();           // the visitor reached the kid
    void OnFound();            // he found the hiding spot
    void OnVisitorUp();        // he got back up after the traps
    void OnLeftPremises();     // ran past the fence during the chase (v0.4)
    void QTEResult(bool stayedAwake);
    bool ShouldHide(const std::string& id) const;
    // v0.12: the hunt
    static bool IsHideSpot(const std::string& id);
    int CaughtIn(const std::string& id) const { auto it = caughtSpots.find(id); return it == caughtSpots.end() ? 0 : it->second; }
    void OnHeard(const std::string& how);   // run = running feet, breath = you breathed with him next to you, gasp = you ran out of air
    void OnTracked();                       // he followed wet prints indoors
    bool OnDecoy();                         // a thrown marble landed where he could hear it. True if he goes to look
    bool VisitorEntersWindow() const { return frontLocked; }
    // v0.15: the outside
    static bool IsWindow(const std::string& id) { return WindowById(id) != nullptr; }
    bool WindowSticks(const std::string& id) const { return id == "window" && !windowBanged; }
    void OnClimbed(const std::string& win, bool out, bool doggoLeft);   // the presentation finished the climb
    void OnLitUp();                           // his torch found you from far away, out in the dark
    void OnHusky();
    void TooColdToLeave();                    // v0.15.1: loop 1. You walk up to the edge of the yard and turn back                           // you walked up to the dog at the far fence. Nobody there
    bool BoardSpells() const { return (tainted || Act() >= 3) && t >= 45 * 60; }   // the lights behind the shed spell at you
    bool Warm() const { return chase && kidHidden && hideSpot == "beast"; }
    static int OutbackEggs(const std::set<std::string>& seen);
    int EggsFound() const { return OutbackEggs(seenEvents); }
    // v0.13: the hinges
    bool MomInBath() const;                 // her two minutes in the bathroom, every loop
    bool BathLockIsDark() const;            // Noah: a trick when she's happy, a dark act when she's scared or you hold the knife
    int HingePieces() const { int n = 0; for (const char* id : {"piece_mom", "piece_dad", "piece_reissue", "piece_door", "piece_car"}) n += Knows(id) ? 1 : 0; return n; }
    bool CanGreet() const;                  // v0.14: hinge D. He is in his car or on the step, and you know he's coming
    bool BasementInHunt() const { return basementUnlocked; }   // he only searches down there if the door is open
    // v0.13: which room is on the other side of a door, from where you stand
    bool DoorSides(int dx, int dy, int px, int py, Room& here, Room& there, int& ox, int& oy) const;

    // queries
    int Awareness() const { return (int)memory.size(); }
    int Stage() const;            // 0 tutorial, 1 noticing, 2 digging, 3 dark, 4 the basement
    int Act() const;              // 1 cute, 2 eerie (after a few failed runs), 3 dark. Drives music and visuals
    bool Tutorial() const { return loop == 1; }
    bool Knows(const std::string& id) const { return memory.count(id) > 0; }
    bool BasementOpen() const { return !collection && t >= 30 * 60 && t < 32 * 60 + 30 && !momLocked; }
    bool DoorLocked(int x, int y) const;   // can't pass (grounded, front latch, master key)
    bool DoorClosed(int x, int y) const;   // closed or locked
    bool DoorBroken(int x, int y) const { return brokenDoors.count(y * World::W + x) > 0; }
    void OpenDoor(int x, int y);           // a grown-up opens it (unlocks it if needed)
    void BreakDoor(int x, int y);          // he doesn't use handles
    bool Befriended() const;
    Stance CurrentStance() const;
    void VisitorNoticesYou();              // v0.9.2: talking to him brings the knock closer
    void DoggoSacrifice();                 // the presentation calls this when he jumps in front of you
    bool PowerOut() const { return collection || t >= visitorTime || (shift == Shift::EarlyStorm && t >= 40 * 60); }
    bool Storm() const { return t >= (shift == Shift::EarlyStorm ? 37 : 47) * 60; }
    bool CarHere() const { return t >= visitorTime - VISIT_ARRIVE || collection; }
    bool DoggoExists() const { return shift != Shift::NoDoggo && !doggoGone && !doggoDead && !doggoOut; }   // v0.10: out = safe, and not in the house
    bool MomHere() const { return !momLocked && !momBathLocked; }
    bool DadFree() const { return !dadLocked && !dadDown; }
    bool FullMemory() const { return memory.size() >= AllMemories().size(); }
    // v0.10
    bool KnowsEvent(const std::string& id) const;          // timeline: seen it, or hold the memory for it
    bool SawEvent(const std::string& id) const { return seenEvents.count(id) > 0; }
    bool HasProof() const { return Knows("phone_answer") || Knows("basement_files") || Knows("his_car") || Knows("leash"); }
    bool CarEmpty() const;                                 // he is at the window, the car is running at the gate
    bool DoggoAtDoor() const { return DoggoExists() && !collection && t >= 35 * 60 && t < 40 * 60; }
    bool Spared() const;                                   // nobody confirmed the clean-up and nobody gave him a reason
    bool MomHums() const;                                  // she hums in the kitchen while she thinks nobody listens
    NpcState Mom() const;
    NpcState Dad() const;
    NpcState Visitor() const;
    std::string ClockText() const;
    std::string ClockTextSec() const;   // v0.5: HH:MM:SS for the watch
    int ChoresDone() const { return (fed ? 1 : 0) + (trashDone ? 1 : 0) + (roomClean ? 1 : 0) + (dishesDone ? 1 : 0) + (firewoodDone ? 1 : 0); }
    static constexpr int CHORES = 5;
    bool SeenLine(const std::string& text) const;
    void MarkSeen(const std::string& text);
    const char* MomName() const { return Knows("the_tape") ? "Handler two" : "Mom"; }
    void Hold(Item it, bool quiet = false);   // v0.6: one thing at a time, the old one goes back
    void MinigameDone(int score);             // v0.6: the Fliptendo easter egg finished

private:
    float rng = 0.1234f;
    bool solemn = false;        // v0.9: the repeated line glitch stays out of serious scenes
    float Rand();

    void Say(const std::string& who, const std::string& text);
    void Bubble(const std::string& who, const std::string& text);
    void Narrate(const std::string& text);
    void Black(const std::string& text);
    void FadeFor(int minutes, const std::string& label, int tx = -1, int ty = -1);
    void Sound(const std::string& id);
    void Flash();
    void SceneBeat(const std::string& id);
    void Achieve(const std::string& name);
    void AddClue(const std::string& id);
    void Choice(const std::string& prompt, std::vector<ChoiceOpt> opts);
    void CheckHandlers();
    void CheckEvents(float prev, float now, int px, int py);
    bool Once(const std::string& id);
    void StartCollection(const std::string& how);
    void CollectionScript();
    void TutorialCollection();
    void ColdOut();
    void TrunkScript(bool quiet = false);
    void SecretEnding();
    void MonsterEnding();
    void DeadSwitchLoop();
    void Ground();
    void ChangeMood(int d);
    void DarkAct(int points, const std::string& firstLine, bool quiet = false);
    void KnifeCheck();
    void VisitorChoice();
    void TiredOut();
    void PaidEnding();
    void EscapeEnding();
    void Threat(int n);
    void DoggoShot(bool onScreen);
    void ConvinceParents(const std::string& who);
    // v0.13: the hinges at the door
    void HingeBath();
    void HingeShed();
    void HingeBasement();
    void HingeDoor();          // v0.14: D, you opened the door yourself
    void HingeCar();           // v0.14: E, under the blanket in his car
    void GoneEnding();         // v0.14: E, you stayed in the car through the knock
    void PieceCheck();
    void Egg(const std::string& id);   // v0.15: one of the six things out back
    void AddDark(int n) { darkPoints = std::min(DARK_MAX, darkPoints + n); darkThisLoop += n; }
    void ScreenTimeEnding();
    // v0.10
    void See(const std::string& ev) { seenEvents.insert(ev); }
    bool CleanUp();                 // the black screen lines for the handlers and Doggo. True if they lived
    void DinnerEnding();
    void ExpressDelivery();
    void ShowWrist(bool downed);
    void LoopOpener(const std::string& how);
    void Nudge();
    std::vector<ChoiceOpt> MomQuestions() const;   // what you can ask her because of what you remember
    std::vector<ChoiceOpt> DadQuestions() const;
    void FliptendoPlayed(const std::string& what);
    int Dur(int minutes) const { return Tutorial() ? minutes * 2 : minutes; }
    bool TutorialLocked(const std::string& id) const;
};

} // namespace sixhundred
