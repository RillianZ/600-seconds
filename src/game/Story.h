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
    Sound,       // id = beep, knock, bang, bark, ring, thud, glass, crack
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

enum class Shift { None, Toilet45, NoDoggo };

// v0.5: how a parent feels about you right now (per loop). Big choices lock them into a state,
// so you can't chat about dinner with someone you just pulled a knife on.
enum class Attitude { Normal, Cold, Scared, Broken };

// v0.6: the kid holds one thing at a time
enum class Item { None, Knife, Key, Stick, Bag, DogFood, Flashlight, Tape };

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

// v0.4.2: how often reality shifts. Tweakable live from the debug panel (O with ?debug).
// Default: never two anomalies within 5 loops, and one is guaranteed within 10.
struct ShiftConfig {
    bool enabled = true;
    bool toilet = true;          // the 45 minute toilet
    bool noDoggo = true;         // the reality without Doggo
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
    bool gotAllowance = false, zuldaBought = false, knifeTakenLoop = false;
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
    std::string pendingKnifeAct;
    int knifeChecks = 0;

    void NewGame();            // wipes progress, keeps the darkness
    void LoadSave(const std::string& s);
    std::string SaveString() const;

    void BeginLoop();
    Shift RollShift();       // decides this loop's reality (uses gShiftCfg)
    void DebugSetDark(int v);
    void DebugAllMemories();
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
    bool VisitorEntersWindow() const { return frontLocked; }

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
    void DoggoSacrifice();                 // the presentation calls this when he jumps in front of you
    bool PowerOut() const { return collection || t >= visitorTime; }
    bool Storm() const { return t >= 47 * 60; }
    bool CarHere() const { return t >= visitorTime - 120 || collection; }
    bool DoggoExists() const { return shift != Shift::NoDoggo && !doggoGone && !doggoDead; }
    bool MomHere() const { return !momLocked; }
    bool DadFree() const { return !dadLocked && !dadDown; }
    bool FullMemory() const { return memory.size() >= AllMemories().size(); }
    NpcState Mom() const;
    NpcState Dad() const;
    NpcState Visitor() const;
    std::string ClockText() const;
    std::string ClockTextSec() const;   // v0.5: HH:MM:SS for the watch
    int ChoresDone() const { return (fed ? 1 : 0) + (trashDone ? 1 : 0) + (roomClean ? 1 : 0) + (dishesDone ? 1 : 0) + (firewoodDone ? 1 : 0); }
    static constexpr int CHORES = 5;
    void Hold(Item it, bool quiet = false);   // v0.6: one thing at a time, the old one goes back
    void MinigameDone(int score);             // v0.6: the Fliptendo easter egg finished

private:
    float rng = 0.1234f;
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
    void TrunkScript();
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
    void AddDark(int n) { darkPoints = std::min(DARK_MAX, darkPoints + n); darkThisLoop += n; }
    void ScreenTimeEnding();
    void FliptendoPlayed(const std::string& what);
    int Dur(int minutes) const { return Tutorial() ? minutes * 2 : minutes; }
    bool TutorialLocked(const std::string& id) const;
};

} // namespace sixhundred
