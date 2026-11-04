// 600 Seconds - Story logic. Engine-free.
#include "Story.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <random>

namespace sixhundred {

// ============================================================ memory log
const std::vector<std::string>& AllTopics() {
    static const std::vector<std::string> t = {
        "The hour", "The house", "Mom", "Dad", "Doggo", "The visitor", "The basement", "The dark"};
    return t;
}

const char* ItemName(Item it) {
    switch (it) {
    case Item::Knife: return "Knife";
    case Item::Key: return "Master key";
    case Item::Stick: return "Stick";
    case Item::Bag: return "Trash bag";
    case Item::DogFood: return "Dog food";
    case Item::Flashlight: return "Flashlight";
    case Item::Tape: return "VHS tape";
    default: return "";
    }
}

const std::vector<EndingEntry>& AllEndings() {
    static const std::vector<EndingEntry> e = {
        {"monster", "Monster", "Somebody has to stop him. Somebody small."},
        {"program", "End of the Program", "Like Monster, but you know where the machine is."},
        {"secret", "The Longest Survivor", "Remember everything. Then stay awake."},
        {"screen", "Screen Time", "Just one more game. And one more. And one more."},
        {"escape", "False Hope", "They'd run with you. If they knew what you know."},
        {"doggo", "Good Boy", "Doggo would do anything for you. Don't let him."},
        {"paid", "They Paid", "Stay hidden. No matter what you hear."},
    };
    return e;
}

const std::vector<MemoryEntry>& AllMemories() {
    static const std::vector<MemoryEntry> m = {
        // ---- The hour
        {"loop", "The hour", "The hour repeats",
         "16:00 to 17:00 on Christmas Eve. The bells start, the watch beeps and I'm back in bed. Nobody else notices.",
         "Something about the watch."},
        {"poop45", "The hour", "The 45 minute toilet",
         "Some days the toilet takes 45 minutes. No matter what. Reality is NOT the same every time.",
         "Some days are... different."},
        {"cold", "The hour", "The snow gets warm",
         "Out in the storm without my jacket I last less than a minute. The snow stops being cold. That's the bad part.",
         "What happens if you stay out in the storm?"},
        // ---- The house
        {"handlers", "The house", "They are not my parents",
         "Reports. Phone calls. Radios. Mom and Dad aren't my parents. They're my handlers.",
         "Put the pieces together."},
        {"no_car", "The house", "We don't have a car",
         "No car keys. No car. 31 km to the nearest neighbour. The road closes behind him at 16:50. The only car up here is his.",
         "How would we even leave?"},
        {"syringe", "The house", "The syringe",
         "Doggo dug it out of the trash bins. An empty syringe. The label says 06 - WEEKLY, in Mom's handwriting.",
         "Doggo's nose finds things. Some of them on darker days."},
        {"no_baby", "The house", "No baby photos",
         "The photo album starts when I'm four. Nothing before that. Not one picture.",
         "Families keep pictures."},
        {"calendar", "The house", "PICKUP",
         "The advent calendar. There's no chocolate behind door 24. One word, in Mom's handwriting: PICKUP.",
         "What's behind door 24?"},
        {"presents", "The house", "Nothing in the boxes",
         "I opened a present early. An empty box, taped shut years ago. Only the one I earn has anything in it.",
         "What's under the tree?"},
        {"static_tv", "The house", "The TV changed",
         "Same channel, different episode. A boy in a box. The audience laughs.",
         "Watch TV after you know the truth."},
        // ---- Mom
        {"allies", "Mom", "They didn't know",
         "I told them about the clean-up. Mom sat down on the kitchen floor. They thought they were retiring. They were next.",
         "Be the kid they'd believe. Then tell them what you know."},
        {"told_mom", "Mom", "Telling Mom doesn't work",
         "I told Mom the day repeats. She sent me to my room for 20 minutes.",
         "What happens if I tell Mom?"},
        {"phone_mom", "Mom", "The 16:22 phone call",
         "The landline rings at 16:22. Mom answers: \"Yes. Today. No, he doesn't suspect a thing.\"",
         "Be in the kitchen at 16:22."},
        {"phone_answer", "Mom", "The caller",
         "I answered first. \"Handler two, confirm pickup at 16:50. Clean-up included.\" Handler?",
         "What if I answer the phone before Mom?"},
        {"reports", "Mom", "Mom's reports",
         "Mom's desk: WEEKLY REPORT. SUBJECT 06. Stable. Unaware. Week 612. That's me.",
         "Mom's desk is always tidy. Too tidy."},
        {"book_notes", "Mom", "Notes in the margins",
         "\"Raising Exceptional Children.\" Mom wrote in the margins: dose, drift, week 400 stable.",
         "Mom reads a lot. What, though?"},
        // ---- Dad
        {"told_dad", "Dad", "Telling Dad makes it worse",
         "I told Dad. He said \"let me make a call.\" The man came at 16:40 instead.",
         "What happens if I tell Dad?"},
        {"dad_radio", "Dad", "Dad's radio",
         "A radio on Dad's bench. Sticker: PROPERTY OF THE PROGRAM. UNIT 06 HANDLER.",
         "Dad spends a lot of time in the shed."},
        {"dad_goodbye", "Dad", "Dad said goodbye",
         "\"Whatever happens today, you were a good kid.\" He wasn't talking about chores.",
         "Maybe help Dad for once."},
        {"dad_key", "Dad", "Dad's key",
         "The basement key is in a tiny box at the bottom of the flagpole.",
         "Dad keeps the key on him. Or does he?"},
        // ---- Doggo
        {"doggo_basement", "Doggo", "Doggo hates the basement",
         "Doggo growls at the basement door. Every time.",
         "Check the basement door with Doggo around."},
        {"doggo_gate", "Doggo", "Doggo hides from him",
         "Doggo won't go near the man at the gate. He hides behind my legs.",
         "Bring Doggo to meet the visitor."},
        {"doggo_stay", "Doggo", "He'll jump in front of me",
         "When the man gets close, Doggo jumps between us. Tell him to stay. Or shut him behind a door. He can't open doors.",
         "Doggo would do anything for you."},
        // ---- The visitor
        {"visitor_time", "The visitor", "The man comes at 16:50",
         "Power cuts out. A knock. I thought it was Julenissen. A big, calm man. He takes me. The others don't make it.",
         "Wait for 16:50."},
        {"he_runs_fast", "The visitor", "He has to find me",
         "He doesn't know where I am. He searches room by room. If he sees me, he runs. If I run, I'm faster. For a bit.",
         "Try to get away from him."},
        {"hiding", "The visitor", "He checks the hiding spots",
         "He checks the hiding spots room by room. Not always the same one first. If he sees me hide, it's over.",
         "Hide when he comes."},
        {"visitor_armed", "The visitor", "He has a gun",
         "I tried to fight him. He shot me in the leg without blinking.",
         "Resist."},
        {"dead_switch", "The visitor", "He takes me with him",
         "I killed him. The box on his belt screamed and something behind my ear burned. When his heart stops, so does mine. The box comes off his belt, though.",
         "What happens if he doesn't get up?"},
        {"run_fail", "The visitor", "There's nowhere to run",
         "I ran into the white. After an hour I found footprints. Small ones. Mine. I had walked in a circle back to the gate.",
         "Leave through the gate."},
        {"visitor_knows", "The visitor", "He expected me to know",
         "\"I know what you are.\" He smiled. \"Good. They said you might.\"",
         "Tell him what you know."},
        {"dad_ran", "The visitor", "Dad fought him",
         "Dad grabbed a lamp, told me to run and went straight at him. It bought me ten seconds. Mom sat on the floor next to him and cried.",
         "Be a kid worth fighting for. Do everything right, all of it."},
        {"they_paid", "The visitor", "He makes them pay",
         "If he can't find me, he starts on them. 16:57 a warning. 16:58 Dad. 16:59 Mom. I heard all of it.",
         "Stay hidden long enough."},
        {"fortress", "The visitor", "Locked doors",
         "I locked everything. He just came through the window. Locks only make him angrier.",
         "Lock him out. Keep it locked."},
        {"prints", "The visitor", "Someone was here",
         "Big footprints in the new snow, all along the house, long before he comes. They stop under the living room window.",
         "Look at the snow outside. Before the storm covers it."},
        // ---- The basement
        {"basement_open", "The basement", "Mom goes down at 16:30",
         "At 16:30 Mom unlocks the basement. Blue light. A hum. The door is open for about two minutes.",
         "Be in the hallway at 16:30."},
        {"basement_screens", "The basement", "Screens",
         "Screens in the basement. One shows my room. One shows me, right now.",
         "Peek while the door is open."},
        {"basement_files", "The basement", "The files",
         "SUBJECT 06. Collect at 16:50. Clean up the handlers. Handlers are NOT to be informed. They think they're retiring.",
         "Get down there. Read everything."},
        {"program_hq", "The basement", "Finse",
         "The program's facility is at Finse. THE MACHINE is underlined three times.",
         "Where does the man take me?"},
        {"treatment", "The basement", "The chair",
         "A chair with straps. Kid-sized. My wrists remember it even if I don't.",
         "There's furniture down there too."},
        {"juice", "The basement", "The juice",
         "S06. RESET. AFTER EACH SESSION. IN JUICE. The juice at 15:55 isn't for naps. It's so I don't remember.",
         "Check what they keep cold down there."},
        {"the_tape", "The basement", "Session one",
         "Dad's cabinet. A tape. The chair, Dad's bored voice, Mom humming. Then the juice. I'll never forgive them.",
         "Dad's cabinet opens with the same key. Play what's inside where it was filmed."},
        {"they_listen", "The basement", "They listen",
         "I used their radio. \"That's not handler two. Stay where you are, 06.\" He came early.",
         "Talk to them on their own radio."},
        // ---- The dark
        {"first_dark", "The dark", "I can't undo it",
         "I did something I can't take back. The game remembers. So do I.",
         "Some doors only open one way."},
        {"mirror_lag", "The dark", "Who's in the mirror",
         "My reflection is older now. It smiles when I don't.",
         "Look at yourself after."},
        {"traps", "The dark", "Not like the movies",
         "Marbles. Tripwire. Paint can. In the movie it's funny. In real life bones make a sound.",
         "Prepare the house for him."},
    };
    return m;
}

const std::vector<DoorInfo>& AllDoors() {
    static const std::vector<DoorInfo> d = {
        {6, 9, "door_kid", "Your door"}, {13, 9, "door_bath", "Bathroom door"}, {22, 9, "door_parents", "Mom and Dad's door"},
        {7, 12, "door_kitchen", "Kitchen door"}, {20, 12, "door_living", "Living room door"}, {13, 16, "door_kl", "Kitchen door"},
        {20, 20, "frontdoor", "Front door"}, {34, 10, "sheddoor", "Shed door"},
    };
    return d;
}
const DoorInfo* DoorById(const std::string& id) { for (auto& d : AllDoors()) if (id == d.id) return &d; return nullptr; }
const DoorInfo* DoorAt(int x, int y) { for (auto& d : AllDoors()) if (d.x == x && d.y == y) return &d; return nullptr; }

static const MemoryEntry* FindMemory(const std::string& id) {
    for (auto& e : AllMemories()) if (e.id == id) return &e;
    return nullptr;
}

// ============================================================ save
void Story::NewGame() {
    // The darkness stays. That's the point.
    loop = 0;
    memory.clear();
    achievements.clear();
    endings.clear();
    seenLines.clear();
    seenToilet45 = false;
    prototypeEndShown = false;
    cleanStreak = 0;
}

// ============================================================ reality shift config
ShiftConfig gShiftCfg;
static std::mt19937 shiftRng(std::random_device{}());
void SeedShifts(unsigned seed) { shiftRng.seed(seed); }
static int ShiftRoll100() { return (int)(shiftRng() % 100); }

const char* ShiftName(Shift s) {
    return s == Shift::Toilet45 ? "45 minute toilet" : s == Shift::NoDoggo ? "no Doggo" : s == Shift::EarlyStorm ? "early storm" : "normal";
}

std::string ShiftConfig::Save() const {
    std::ostringstream o;
    o << "c1|" << enabled << "|" << toilet << "|" << noDoggo << "|" << chance << "|" << minGap << "|"
      << maxGap << "|" << firstLoop << "|" << toiletShare << "|" << (int)std::lround(timeMul * 10) << "|" << earlyStorm;
    return o.str();
}

void ShiftConfig::Load(const std::string& s) {
    std::vector<int> v; std::string cur;
    if (s.rfind("c1|", 0) != 0) return;
    for (size_t i = 3; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == '|') { v.push_back(std::atoi(cur.c_str())); cur.clear(); } else cur += s[i];
    }
    if (v.size() < 8) return;
    enabled = v[0]; toilet = v[1]; noDoggo = v[2];
    chance = std::clamp(v[3], 0, 100); minGap = std::clamp(v[4], 0, 30); maxGap = std::clamp(v[5], 0, 30);
    firstLoop = std::clamp(v[6], 1, 30); toiletShare = std::clamp(v[7], 0, 100);
    if (v.size() >= 9) timeMul = std::clamp(v[8], 10, 20) / 10.0f;
    if (v.size() >= 10) earlyStorm = v[9];
}

Shift Story::RollShift() {
    ShiftConfig& c = gShiftCfg;
    if (c.forceNext != Shift::None) { Shift f = c.forceNext; c.forceNext = Shift::None; return f; }
    if (!c.enabled || loop < c.firstLoop || (!c.toilet && !c.noDoggo && !c.earlyStorm)) return Shift::None;
    if (cleanStreak < c.minGap) return Shift::None;
    bool forced = c.maxGap > 0 && cleanStreak >= std::max(c.maxGap, c.minGap);
    if (!forced && ShiftRoll100() >= c.chance) return Shift::None;
    std::vector<Shift> others;
    if (c.noDoggo) others.push_back(Shift::NoDoggo);
    if (c.earlyStorm) others.push_back(Shift::EarlyStorm);
    if (others.empty()) return Shift::Toilet45;
    Shift other = others[ShiftRoll100() % others.size()];
    if (!c.toilet) return other;
    if (!seenToilet45) return Shift::Toilet45;  // the first anomaly you meet is the toilet, so its memory is always findable
    return ShiftRoll100() < c.toiletShare ? Shift::Toilet45 : other;
}

void Story::Hold(Item it, bool quiet) {
    if (held == it) return;
    if (held != Item::None && it != Item::None && !quiet) {
        static const char* back[] = { "", "(You put the knife back in the block first.)", "(You put the key back where it was first.)",
            "(You drop the stick by the woodpile first.)", "(You put the trash bag back first.)", "(You put the dog food back in the pantry first.)",
            "(You put the flashlight back in the pantry first.)", "(You put the tape back in Dad's cabinet first.)" };
        Say("You", back[(int)held]);
    }
    held = it;
    hasKnife = it == Item::Knife; hasKey = it == Item::Key; hasBag = it == Item::Bag;
    hasStick = it == Item::Stick; hasFood = it == Item::DogFood; hasLight = it == Item::Flashlight;
    if (it == Item::Key) keyInDrawer = false;
    if (it == Item::Knife) knifeTakenLoop = true;
}

void Story::DebugSetDark(int v) {
    darkPoints = std::clamp(v, 0, DARK_MAX);
    tainted = darkPoints >= DARK_TAINT;   // debug only: real players can never go back
}
void Story::DebugAllMemories() { for (auto& m : AllMemories()) memory.insert(m.id); }
void Story::DebugClearMemories() { memory.clear(); }

std::string Story::SaveString() const {
    std::ostringstream o;
    o << "v4|" << loop << "|" << (seenToilet45 ? 1 : 0) << "|" << (prototypeEndShown ? 1 : 0) << "|"
      << darkPoints << "|" << (tainted ? 1 : 0) << "|";
    for (auto& m : memory) o << m << ",";
    o << "|";
    for (auto& a : achievements) o << a << ",";
    o << "|";
    for (auto& e : endings) o << e << ",";
    o << "|" << cleanStreak << "|";
    for (unsigned h : seenLines) o << std::hex << h << std::dec << ",";
    return o.str();
}

void Story::LoadSave(const std::string& s) {
    loop = 0; memory.clear(); achievements.clear(); endings.clear();
    seenToilet45 = prototypeEndShown = false;
    std::vector<std::string> parts;
    std::string cur;
    for (char c : s) { if (c == '|') { parts.push_back(cur); cur.clear(); } else cur += c; }
    parts.push_back(cur);
    auto split = [](const std::string& in, std::set<std::string>& out) {
        std::string c;
        for (char ch : in) { if (ch == ',') { if (!c.empty()) out.insert(c); c.clear(); } else c += ch; }
        if (!c.empty()) out.insert(c);
    };
    if (parts.size() >= 6 && parts[0] == "v1") {
        loop = std::atoi(parts[1].c_str());
        seenToilet45 = parts[2] == "1";
        split(parts[4], memory);
        split(parts[5], achievements);
    } else if (parts.size() >= 9 && (parts[0] == "v2" || parts[0] == "v3" || parts[0] == "v4")) {
        loop = std::atoi(parts[1].c_str());
        seenToilet45 = parts[2] == "1";
        prototypeEndShown = parts[3] == "1";
        darkPoints = std::max(darkPoints, std::atoi(parts[4].c_str()));
        tainted = tainted || parts[5] == "1";
        if (parts[0] == "v2") tainted = darkPoints >= DARK_TAINT; // v0.3 rules: small stuff no longer taints you
        darkPoints = std::min(darkPoints, DARK_MAX);
        split(parts[6], memory);
        split(parts[7], achievements);
        split(parts[8], endings);
        cleanStreak = parts.size() >= 10 ? std::atoi(parts[9].c_str()) : 0;
        seenLines.clear();
        if (parts.size() >= 11) {
            std::set<std::string> hs; split(parts[10], hs);
            for (auto& h : hs) seenLines.insert((unsigned)std::strtoul(h.c_str(), nullptr, 16));
        }
    }
}

// ============================================================ helpers
// v0.9: lines you have read before can be skipped, including the black screens
static unsigned LineHash(const std::string& t) { unsigned h = 2166136261u; for (unsigned char c : t) h = (h ^ c) * 16777619u; return h; }
bool Story::SeenLine(const std::string& text) const { return seenLines.count(LineHash(text)) > 0; }
void Story::MarkSeen(const std::string& text) { seenLines.insert(LineHash(text)); }

float Story::Rand() {
    rng = std::fmod(rng * 9301.0f + 49297.0f * 0.0001f + 0.618f, 1.0f);
    return rng;
}

void Story::Say(const std::string& who, const std::string& text) {
    Beat b; b.type = BeatType::Say; b.text = text;
    b.speaker = who == "Mom" ? MomName() : who;   // v0.9: after the tape she isn't Mom any more
    beats.push_back(b);
    // Quantum instability: the extras don't know their lines. Never in the serious scenes
    if (Stage() >= 2 && who != "" && who != "You" && who != "The visitor" && !collection && !solemn &&
        momAtt == Attitude::Normal && dadAtt == Attitude::Normal && Rand() < 0.12f) beats.push_back(b);
}
void Story::Bubble(const std::string& who, const std::string& text) {
    Beat b; b.type = BeatType::Bubble; b.speaker = who; b.text = text;
    beats.push_back(b);
}
void Story::Narrate(const std::string& text) { Beat b; b.type = BeatType::Narrate; b.text = text; beats.push_back(b); }
void Story::Black(const std::string& text) { Beat b; b.type = BeatType::Black; b.text = text; beats.push_back(b); }
void Story::FadeFor(int minutes, const std::string& label, int tx, int ty) {
    Beat b; b.type = BeatType::Fade; b.minutes = minutes; b.text = label; b.tx = tx; b.ty = ty;
    beats.push_back(b);
}
void Story::Sound(const std::string& id) { Beat b; b.type = BeatType::Sound; b.id = id; beats.push_back(b); }
void Story::Flash() { Beat b; b.type = BeatType::Flash; beats.push_back(b); }
void Story::SceneBeat(const std::string& id) {
    Beat b; b.type = BeatType::Scene; b.id = id; beats.push_back(b);
    if (id == "chase") chase = true;
}
void Story::Achieve(const std::string& name) {
    if (achievements.count(name)) return;
    achievements.insert(name);
    Beat b; b.type = BeatType::Achievement; b.text = name; beats.push_back(b);
}
void Story::AddClue(const std::string& id) {
    if (memory.count(id)) return;
    memory.insert(id);
    loopNew.push_back(id);
    Beat b; b.type = BeatType::Clue; b.id = id;
    if (auto* e = FindMemory(id)) b.text = e->title;
    beats.push_back(b);
    CheckHandlers();
    if (!prototypeEndShown && FullMemory()) {
        prototypeEndShown = true;
        Achieve("Total Recall");
        Narrate("I knew everything now. Every line. Every lie. Even how to stay awake in the trunk.");
    }
}
void Story::Choice(const std::string& prompt, std::vector<ChoiceOpt> opts) {
    Beat b; b.type = BeatType::Choice; b.text = prompt; b.choices = std::move(opts);
    beats.push_back(b);
}
bool Story::Once(const std::string& id) {
    if (firedEvents.count(id)) return false;
    firedEvents.insert(id);
    return true;
}

void Story::CheckHandlers() {
    if (memory.count("handlers")) return;
    bool a = Knows("reports") || Knows("book_notes");
    bool b = Knows("phone_mom") || Knows("phone_answer") || Knows("calendar");
    bool c = Knows("dad_radio") || Knows("basement_screens");
    if (a && b && c) {
        memory.insert("handlers");
        Beat cl; cl.type = BeatType::Clue; cl.id = "handlers";
        cl.text = FindMemory("handlers")->title;
        beats.push_back(cl);
        Narrate("That was the day I stopped calling them Mom and Dad. In my head, anyway.");
        Narrate("Something in the house noticed too.");
    }
}

int Story::Stage() const {
    if (tainted) return Knows("basement_files") ? 4 : 3;
    if (loop <= 1) return 0;
    if (Knows("handlers")) return 2;
    return 1;
}

int Story::Act() const {
    if (tainted) return 3;
    // v0.5.1: act 2 (eerie music, flux) starts at the real act break, finding out about the handlers.
    // Only players who keep looping without finding it get it anyway, from loop 8.
    if (Knows("handlers") || loop >= 8) return 2;
    return 1;
}

bool Story::DoorLocked(int x, int y) const {
    if (x == 6 && y == 9 && grounded && !collection) return true;
    if (x == 20 && y == 20 && frontLocked && !collection) return true;
    return lockedDoors.count(y * World::W + x) > 0;
}
bool Story::DoorClosed(int x, int y) const {
    return DoorLocked(x, y) || closedDoors.count(y * World::W + x) > 0;
}
void Story::OpenDoor(int x, int y) {
    int k = y * World::W + x;
    closedDoors.erase(k); lockedDoors.erase(k);
    if (x == 20 && y == 20) frontLocked = false;
}
void Story::BreakDoor(int x, int y) { OpenDoor(x, y); brokenDoors.insert(y * World::W + x); }

bool Story::Befriended() const {
    // v0.7: every chore done and nothing dark this loop. Then they'll fight for you.
    return !Tutorial() && ChoresDone() == CHORES && darkThisLoop == 0 && momAtt == Attitude::Normal &&
           dadAtt == Attitude::Normal && !grounded && !momLocked && !dadLocked;
}
Stance Story::CurrentStance() const {
    if (Befriended()) return Stance::Befriended;
    if (tainted || darkThisLoop > 0 || momAtt == Attitude::Scared || dadAtt == Attitude::Scared || dadAtt == Attitude::Broken) return Stance::Dark;
    return Stance::Normal;
}

std::string Story::ClockTextSec() const {
    int sec = (int)t;
    int hh = 16 + sec / 3600, mm = (sec / 60) % 60, ss = sec % 60;
    char buf[16];
    std::snprintf(buf, sizeof buf, "%02d:%02d:%02d", hh, mm, ss);
    return buf;
}

std::string Story::ClockText() const {
    int m = (int)t / 60;
    int hh = 16 + m / 60, mm = m % 60;
    char buf[16];
    std::snprintf(buf, sizeof buf, "%02d:%02d", hh, mm);
    return buf;
}

void Story::ChangeMood(int d) {
    mood += d;
    if (mood <= -3 && !grounded && !collection) Ground();
}

void Story::Ground() {
    grounded = true;
    if (momAtt == Attitude::Normal) momAtt = Attitude::Cold;
    if (tainted) Say("Mom", "(she won't come closer) Go to your room. Please. Just... go.");
    else Say("Mom", "That's it. Room. Now. You're not coming out until I say so.");
    Achieve("Grounded");
    FadeFor(1, "Sent to your room", 5, 6);
    inBasement = false;
    Say("You", "(The door is locked from the outside.)");
    if (Stage() >= 1) Say("You", "(Locked in until he comes. Great plan, me.)");
}

void Story::DarkAct(int points, const std::string& line, bool quiet) {
    int before = darkPoints;
    darkPoints = std::min(DARK_MAX, darkPoints + points);
    darkThisLoop += points;
    if (!line.empty()) Say("", line);
    if (quiet && (tainted || darkPoints < DARK_TAINT)) {
        // v0.7: the caller says something that fits better
    } else if (!tainted && darkPoints < DARK_TAINT) {
        // Still just a weird kid. For now.
        static const char* weird[] = {
            "(That was a weird thing to do. Right?)",
            "(I'm just being careful. That's all.)",
            "(Nobody needs to know about that.)",
        };
        Say("You", weird[darkPoints % 3]);
    } else if (!tainted) {
        tainted = true;
        AddClue("first_dark");
        Achieve("Point of No Return");
        Say("You", "(I didn't mean to. I didn't. I didn't.)");
        Say("You", "(My hands won't stop shaking.)");
        Narrate("Something in me stopped shaking that day. It never started again.");
        Narrate("You can start a new game if you want. It won't help. I'll remember.");
    } else if (darkPoints >= DARK_REVEL) {
        static const char* rev[] = {"(That was easy.)", "(I could do that again.)", "(Why did I ever cry about this.)"};
        Say("You", rev[darkPoints % 3]);
    } else {
        Say("You", "(Okay.)");
    }
    if (before < DARK_ARSENAL && darkPoints >= DARK_ARSENAL)
        Narrate("I started looking at the house differently. Stairs. Doorways. Things that could fall.");
    if (before < DARK_INTERROGATE && darkPoints >= DARK_INTERROGATE)
        Narrate("I knew Dad would talk if I asked the right way. I knew what the right way was.");
}

bool Story::TutorialLocked(const std::string& id) const {
    if (!Tutorial()) return false;
    static const std::set<std::string> open = {
        "bed", "toys", "window", "toilet", "mirror", "fridge", "stove", "trashcan", "bowl",
        "tv", "couch", "radio", "bins", "doghouse", "gate", "flag", "mom", "dad", "doggo", "woodstove",
        "calendar", "bookshelf", "album", "wardrobe", "woodpile", "marbles", "frontdoor", "visitor",
        "pantry", "sink", "fliptendo", "tree",
        "door_kid", "door_bath", "door_parents", "door_kitchen", "door_living", "door_kl", "sheddoor"};
    return open.count(id) == 0;
}

bool Story::ShouldHide(const std::string& id) const {
    if (!chase) return false;
    return id == "wardrobe" || id == "bed" || id == "pbed" || id == "toolbox" || id == "couch";
}

// ============================================================ npcs
NpcState Story::Mom() const {
    NpcState n;
    int m = (int)(t / 60);
    if (momLocked) { n.x = 12; n.y = 39; n.visible = inBasement; return n; }
    if (collection && dadOnFloor && stance == Stance::Befriended && !momOnFloor) { n.x = 19; n.y = 17; return n; }   // on her knees next to him
    if (collection) { n.x = 17; n.y = 15; return n; }
    int phone = Stage() >= 2 ? 21 : 22;
    if (t >= phone * 60 - 30 && t < (phone + 2) * 60) { n.x = 4; n.y = 15; return n; }
    if (t >= 28 * 60 && t < 30 * 60) { n.x = 27; n.y = 10; return n; }
    if (t >= 30 * 60 && t < 33 * 60) { n.x = 8; n.y = 40; n.visible = inBasement; return n; }
    if (Stage() >= 2 && ((m >= 12 && m < 18) || (m >= 36 && m < 40))) {
        n.x = 3; n.y = 19; n.facingWall = true; return n; // standing in the corner
    }
    n.x = 8; n.y = 16;
    return n;
}

NpcState Story::Dad() const {
    NpcState n;
    if (dadOnFloor) { n.x = stance == Stance::Befriended ? 20 : 18; n.y = stance == Stance::Befriended ? 17 : 17; return n; }
    if (dadDown) { n.visible = false; n.x = 20; n.y = 26; return n; }
    if (dadLocked) { n.x = 35; n.y = 8; return n; }
    if (collection) { n.x = 18; n.y = 17; return n; }
    if (t < 8 * 60) { n.x = 24; n.y = 16; return n; }
    if (t < 40 * 60) { n.x = 35; n.y = 8; return n; }
    n.x = 10; n.y = 17;
    return n;
}

NpcState Story::Visitor() const {
    NpcState n;
    n.visible = CarHere();
    if (collection) {
        if (frontLocked) { n.x = 25; n.y = 18; } else { n.x = 20; n.y = 19; }
        return n;
    }
    n.x = 20; n.y = 29;
    return n;
}

// ============================================================ loop
void Story::BeginLoop() {
    loop += 1;
    t = 0;
    mood = 0;
    fed = hasBag = trashDone = roomClean = pooped = false;
    allowanceChecked = grounded = collection = false;
    phoneAnswered = helpedDad = false;
    hasKnife = hasKey = frontLocked = momLocked = dadLocked = false;
    held = Item::None; hasStick = hasFood = hasLight = false;
    dishesDone = firewoodDone = earnedPresent = zuldaOpened = knifeTakenLoop = lightUsed = false;
    playStreak = 0; visitorStun = 0;
    hasCoat = openedEarly = false; cold = 0;
    lastLoopNew = loopNew; loopNew.clear();
    doggoGone = doggoOut = dadDown = inBasement = false;
    traps = 0; trapsSet.clear(); flagTouches = 0;
    chase = visitorInjured = visitorDowned = kidShot = switchTaken = false;
    closedDoors.clear(); lockedDoors.clear(); brokenDoors.clear();
    basementUnlocked = drawerOpen = keyInDrawer = false;
    doggoStay = doggoDead = false; doggoDeadX = doggoDeadY = -1;
    darkThisLoop = 0; parentsAllied = false; stance = Stance::Normal; threats = 0;
    dadOnFloor = momOnFloor = kidHidden = doggoWithKid = false;
    momAtt = dadAtt = Attitude::Normal;
    pendingKnifeAct.clear(); knifeChecks = 0;
    visitorTime = 50 * 60;
    firedEvents.clear();
    rng = std::fmod(0.37f + loop * 0.1618f, 1.0f);

    // Reality shifts
    shift = RollShift();
    if (shift == Shift::None) ++cleanStreak; else cleanStreak = 0;

    Sound("beep");
    if (loop == 1 && !tainted) {
        Narrate("I was twelve. It was Christmas Eve, up on Hardangervidda.");
        Narrate("I remember the watch beeping. I remember everything after that, too. That's the problem.");
        Say("Mom", "Kiddo! Get down here, I need you!");
        Say("You", "(Arrows or WASD to walk. E or Space to use things. Hold T to wait.)");
        Say("You", "(Christmas Eve chores. If I do all of them, I get to open a present before dinner. Please let it be Zulda: Flute of Time!)");
    } else if (loop == 1 && tainted) {
        Narrate("New game. Cute.");
        Narrate("I was twelve. It was Christmas Eve. And I remember what I did. Every version of it.");
        Say("Mom", "(from downstairs, unsure) ...Kiddo? You up?");
    } else if (loop == 2 && !tainted) {
        Narrate("The watch beeped. 16:00. Christmas Eve. Again.");
        Narrate("I was in my bed. I remembered all of it. The knock. The trunk. Nobody else did.");
        Say("You", "(Everything is open now. J opens your memory log. Things you remember unlock new choices.)");
        Say("You", "(Knowing stuff lets you skip lines. Press Tab during talking.)");
    } else {
        static const char* s1[] = {
            "16:00. Same bed. Same watch. Same me. Mostly.",
            "I stopped counting the loops. The watch didn't.",
            "Again. Fine. Let's see what I missed.",
        };
        static const char* s2[] = {
            "They were never my parents. I just didn't know it yet. Well. Now I did.",
            "Same hour. Same lies. The house was starting to forget its lines.",
            "I knew the script better than they did now. That should have scared me more.",
        };
        static const char* s3[] = {
            "I didn't correct myself anymore. Why would I.",
            "16:00. They'd be scared of me again by 16:05. Fine.",
            "The watch beeped. I didn't flinch. I used to flinch.",
        };
        static const char* s4[] = {
            "They locked me in a chair for what I'd become. So I became it. Now we're even.",
            "I knew what was in the basement. I knew what was in me. Same thing.",
            "Every loop the house got quieter around me. Like it was holding its breath.",
        };
        const char** pool = Stage() >= 4 ? s4 : Stage() >= 3 ? s3 : Stage() >= 2 ? s2 : s1;
        Narrate(pool[loop % 3]);
    }
    if (tainted && loop >= 2) Say("Mom", "(from the kitchen) ...Is that you? You're up. Good. Good.");

    if (shift == Shift::Toilet45) Say("You", "(My stomach just made a noise like a dying whale.)");
    if (shift == Shift::NoDoggo) Say("You", "(It's quiet. Too quiet. Something's missing.)");
    if (shift == Shift::EarlyStorm) Say("You", "(The wind is already up. That's not how today goes.)");
}

void Story::EndLoop() {
    beats.clear();
    AddClue("loop");
    if (!tainted && darkPoints >= DARK_TAINT) {
        // crossed the line during the collection
        tainted = true;
        AddClue("first_dark");
        Achieve("Point of No Return");
    }
    BeginLoop();
}

// ============================================================ time
void Story::Tick(float realDt, bool waiting, int px, int py) {
    if (collection) {
        // v0.6: the clock keeps going while he hunts you. At 17:00 you can't run any more.
        if (chase) {
            float before = t;
            t = std::min<float>((float)HOUR, t + realDt * TIME_SCALE * gShiftCfg.timeMul);
            // v0.7: he runs out of patience in the last three minutes
            (void)before;
            for (int n = 1; n <= 3; ++n) if (t >= (56 + n) * 60 && threats < n && !visitorChasing) { Threat(n); break; }
            // v0.8: the storm gets you if you stay out in it. A jacket buys time. Indoors you warm up again
            Room here = world.RoomAt(px, py);
            if (here == Room::Outside && !kidHidden) cold = std::min(1.0f, cold + realDt / (hasCoat ? 70.0f : 25.0f));
            else cold = std::max(0.0f, cold - realDt / 12.0f);
            if (cold >= 1.0f && Once("cold")) { ColdOut(); return; }
            if (t >= HOUR && Once("tired")) { if (kidHidden) PaidEnding(); else TiredOut(); }
        }
        return;
    }
    float prev = t;
    // v0.5: waiting is faster and speeds up the longer you hold it (2 to 5 game minutes per second)
    waitHeld = waiting ? waitHeld + realDt : 0;
    float scale = waiting ? WAIT_SCALE + std::min(waitHeld, 2.0f) * 90.0f : TIME_SCALE * gShiftCfg.timeMul;
    t = std::min<float>((float)HOUR, t + realDt * scale);
    CheckEvents(prev, t, px, py);

    // v0.8: somebody has been walking around the house
    if (!Tutorial() && !collection && t >= 28 * 60 && t < 47 * 60 && world.RoomAt(px, py) == Room::Outside &&
        py >= 21 && py <= 24 && px >= 6 && px <= 18 && Once("prints")) {
        Say("You", "(Footprints in the new snow. Big ones. They go all along the house and stop under the living room window.)");
        Say("You", "(Dad's boots aren't that big.)");
        AddClue("prints");
    }

    // Somebody unlocks the front door again
    if (frontLocked && !collection) {
        bool momCan = MomHere() && !(t >= 28 * 60 && t < 33 * 60);
        bool dadCan = DadFree() && (t < 8 * 60 || t >= 40 * 60);
        if ((momCan || dadCan) && t - frontLockedAt > 90) {
            frontLocked = false;
            Say(momCan ? "Mom" : "Dad", "(click) Why is the front door locked? We're not expecting anyone.");
        }
    }

    // Running onto the vidda
    if (!collection && world.RoomAt(px, py) == Room::Vidda && Once("vidda")) {
        Narrate("The nearest neighbour was 31 kilometres away. I knew that. I ran anyway.");
        FadeFor(0, "Running");
        Black("You run until your lungs burn. The snow comes up to your knees.");
        Black("Then it's all white. No house. No fence. No sky.");
        Black("You walk for a long time. You stop feeling your feet.");
        Black("Footprints in front of you. Small ones. Fresh.");
        Black("They're yours. You walked in a circle.");
        Black("Headlights, right there at the gate. He's been waiting.");
        Sound("thud");
        Black("He isn't even out of breath. He carries you in like a sack of potatoes.");
        AddClue("run_fail");
        Achieve("Nowhere to Run");
        t = (float)visitorTime;
        StartCollection("run");
    }
}

void Story::AdvanceMinutes(int minutes, int px, int py) {
    if (collection) return;
    float prev = t;
    t = std::min<float>((float)HOUR, t + minutes * 60.0f);
    CheckEvents(prev, t, px, py);
}

static bool Crossed(float prev, float now, int minute, int second = 0) {
    float at = minute * 60.0f + second;
    return prev < at && now >= at;
}

void Story::CheckEvents(float prev, float now, int px, int py) {
    Room here = world.RoomAt(px, py);
    bool inKitchen = here == Room::Kitchen;
    int phoneMin = Stage() >= 2 ? 21 : 22; // events start firing early

    // v0.5: flavour bubbles only show when you're actually there for the moment, not after a big time skip
    auto fresh = [&](int minute) { return now - minute * 60 < 90; };
    if (Crossed(prev, now, 2) && Once("chores") && MomHere() && fresh(2)) {
        Bubble("Mom", "Chores! Doggo, trash, dishes, firewood, your room. ALL done by 16:40 and you can open a present before dinner!");
        if (Tutorial()) Bubble("You", "(A present! Then Julenissen at ten to five! Best day of the year.)");
    }
    if (Crossed(prev, now, 8) && Once("dad_out") && fresh(8)) {
        Bubble("Dad", "Gonna fix the generator before the storm hits. Holler if you need me.");
    }
    if (Crossed(prev, now, 10) && Once("radio_report")) {
        if (here == Room::Shed && DadFree()) {
            Say("Dad", "(into a radio) Unit 06 is stable. Yeah. I know what today is.");
            Say("Dad", "...Oh. Hey buddy. Didn't see you there.");
            AddClue("dad_radio");
        }
    }
    if (Crossed(prev, now, phoneMin) && Once("phone")) {
        Sound("ring");
        if (inKitchen || here == Room::Hallway || here == Room::LivingRoom) Say("", "*RING RING* (the landline)");
    }
    if (Crossed(prev, now, phoneMin, 40) && Once("phone_mom")) {
        if (!phoneAnswered && MomHere()) {
            phoneAnswered = true;
            if (inKitchen) {
                Say("Mom", "(answering) Hello? ...Yes. Today. ...No, he doesn't suspect a thing.");
                Say("Mom", "(she sees you) Go play, sweetie.");
                AddClue("phone_mom");
            }
        }
    }
    if (Crossed(prev, now, 30) && Once("basement") && MomHere()) {
        if (here == Room::Hallway) {
            Say("", "(Mom unlocks the basement door. Blue light spills out. Something hums down there.)");
            AddClue("basement_open");
        }
        if (inBasement) {
            if (tainted) {
                Say("", "(Mom comes down the stairs. She sees you. She doesn't scream.)");
                Say("", "(She just goes back up. Backwards. Not taking her eyes off you.)");
            } else {
                Say("Mom", "(on the stairs) No. No no no. You can't be down here. You CAN'T be down here.");
                ChangeMood(-3);
            }
        }
    }
    if (Crossed(prev, now, 35) && Once("doggo_door")) {
        if (fresh(35) && DoggoExists() && (here == Room::LivingRoom || here == Room::Hallway))
            Bubble("Doggo", "*whines at the front door*");
    }
    if (Crossed(prev, now, 40) && Once("allowance")) {
        allowanceChecked = true;
        if (!grounded && MomHere() && momAtt == Attitude::Scared) {
            Say("", "(16:40. Mom doesn't mention the chores. She doesn't mention anything.)");
        } else if (!grounded && MomHere()) {
            if (ChoresDone() == CHORES) {
                Say("Mom", "All five? Look at you. Go on, then. One present before dinner.");
                Say("You", Knows("presents") ? "(I know which one. It's the only one with anything in it.)" : Stage() >= 1 ? "(It won't matter. I'm opening it anyway.)" : "(ZULDA! Straight to the tree!)");
                earnedPresent = true;
                mood += 1;
            } else if (ChoresDone() > 0) {
                Say("Mom", "Half done is not done. The presents can wait.");
                ChangeMood(-1);
            } else {
                Say("Mom", "Nothing? Not one chore? Unbelievable.");
                ChangeMood(-2);
            }
        }
    }
    if (Crossed(prev, now, 45) && Once("nisse_soon") && fresh(45) && DadFree() && !tainted)
        Bubble("Dad", "Julenissen comes at ten to five, buddy. Better be good.");
    if (shift == Shift::Toilet45 && !pooped && Crossed(prev, now, 45) && Once("accident")) {
        Say("You", "(Oh no.)");
        Say("You", "(Oh no no no no.)");
        FadeFor(0, "");
        Black("You didn't make it.");
        Black("In this reality, the toilet always wins.");
        Achieve("Should've Gone");
        AddClue("poop45");
        seenToilet45 = true;
        Narrate("No. That's... that is NOT how it went. We're not telling it like that.");
        Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
        collection = true; // freeze the clock
        return;
    }
    if (shift == Shift::EarlyStorm && Crossed(prev, now, 37) && Once("storm"))
        Say("", "(The snowstorm hits. Ten minutes early. It has never been early.)");
    if (shift == Shift::EarlyStorm && Crossed(prev, now, 40) && Once("early_power")) {
        Sound("thud");
        Say("", "(The power dies. It's only 16:40. Nobody is at the door. Not yet.)");
        Achieve("Weather Report");
    }
    if (Crossed(prev, now, 47) && Once("storm")) {
        Say("", "(The snowstorm hits. The lights flicker. You can't see the gate any more.)");
    }
    if (prev < visitorTime - 120 && now >= visitorTime - 120 && Once("car")) {
        if (here == Room::Outside || here == Room::KidsRoom)
            Say("", "(Headlights at the gate. A black car. No plates.)");
    }
    if (now >= visitorTime && !collection) {
        t = (float)visitorTime;
        StartCollection("normal");
    }
}

// ============================================================ the collection
void Story::StartCollection(const std::string& how) {
    if (collection) return;
    stance = CurrentStance();   // v0.7: decided before anything else changes
    collection = true;
    grounded = false;
    if (how == "run") { CollectionScript(); return; }
    if (inBasement) {
        inBasement = false;
        Say("", "(Up in the house, the power dies. You come up the stairs into the dark.)");
        FadeFor(0, "", 27, 11);
    }

    if (Tutorial()) { frontLocked = false; TutorialCollection(); return; }   // v0.8: loop 1 has no chase

    Sound("knock");
    Say("", "(The power dies. Then: three slow knocks.)");
    if (frontLocked) {
        Say("", "(Nobody opens. The handle turns. Locked.)");
        Sound("glass");
        Say("", "(Glass explodes in the living room. He climbs in through the window like it's a door.)");
        AddClue("fortress");
        Achieve("Fortress");
    } else if (MomHere()) {
        if (stance == Stance::Dark) Say("Mom", "(opening the door) Thank God. Take him. Please. Just take him.");
        else if (stance == Stance::Befriended) Say("Mom", "(opening the door, her voice too bright) ...You're early.");
        else Say("Mom", "(opening the door) ...You're early.");
    } else {
        Say("", "(Nobody answers. The door opens anyway. He has a key.)");
    }

    if (traps > 0 && !frontLocked) {
        Sound("crack");
        if (trapsSet.count("marbles")) Say("", "(His boot hits the marbles on the iced step. He goes down hard. Something in his wrist cracks.)");
        if (trapsSet.count("tripwire")) Say("", "(The tripwire catches his shin. He falls through the doorway face first. The sound is wrong. Like a branch snapping.)");
        if (trapsSet.count("paint")) Say("", "(The paint can swings down into his face. In the movie it's funny. His nose is just... flat now.)");
        AddClue("traps");
        Achieve("Home Alone (Realistic Edition)");
        visitorInjured = true;
        if (traps >= 2) {
            visitorDowned = true;
            Say("", "(He's on the floor. Breathing through his teeth. He's not getting up. Yet.)");
        }
        // v0.7: you hurt him. Doggo is the first thing he hurts back.
        if (DoggoExists() && !Tutorial()) DoggoShot(true);
    }

    Say("The visitor", visitorDowned ? "(from the floor, still calm) ...Evening." : "Evening.");

    // v0.8: it's the question Julenissen always asks
    if (!visitorDowned && !frontLocked && MomHere()) {
        Say("The visitor", "Has he been good?");
        if (stance == Stance::Dark) Say("Mom", "(she won't look at you) Just take him.");
        else if (stance == Stance::Befriended) Say("Mom", "(her voice cracks) He's been so good.");
        else Say("Mom", "(very quietly) ...He's been good.");
    }

    // v0.7: they believe you, and they know what clean-up means now
    if (parentsAllied && stance == Stance::Befriended && MomHere() && DadFree() && !visitorDowned) {
        Say("Mom", "(she grabs your hand, so hard it hurts) Car. NOW.");
        Choice("Mom is already pulling you toward the door.", {{"e_run", "Run for his car with Mom and Dad."}, {"e_stay", "No. Hide. Dad, don't!"}});
        return;
    }
    if (DadFree() && dadAtt == Attitude::Broken) {
        Say("", "(Dad doesn't get up. He just looks at the floor.)");
    } else if (DadFree() && stance == Stance::Befriended && !visitorDowned) {
        Say("Dad", "(grabbing a lamp) Buddy. Run. RUN!");
        SceneBeat("dad_runs");   // v0.7: he doesn't run away. He runs AT him
        return;
    } else if (DadFree() && stance == Stance::Dark) {
        Say("Dad", "(he looks at you, then at him) ...He's all yours.");
    } else if (DadFree()) {
        // v0.7: normal day. They hand you over. They think they get to retire after
        Say("Dad", "(quietly, not looking at you) Go with him, buddy. It's okay. It's... it's okay.");
        if (MomHere()) Say("Mom", "(she's holding the door frame like it's the only thing keeping her up)");
    }
    if (!Knows("he_runs_fast") && !Knows("hiding"))
        Say("You", "(He doesn't know where I am yet. Hide. Sneak past him. Or walk up to him and press E.)");
    SceneBeat("chase");
}

void Story::TutorialCollection() {
    // v0.8: loop 1 is the happiest day of his year. He runs to the door himself.
    Sound("knock");
    Say("", "(The lights go out. Then three slow knocks on the front door.)");
    Say("You", "(JULENISSEN!)");
    if (MomHere()) Say("Mom", "(calling, her voice too bright) Kiddo! There's someone at the door for you!");
    FadeFor(0, "You run to the door", 22, 18);
    Say("", "(He's very big. No beard. No sack. Just a black coat with snow on the shoulders.)");
    Say("The visitor", "Evening.");
    Say("The visitor", "(he crouches down to look at you) Have you been good?");
    Say("You", ChoresDone() > 0 ? "...I did my chores." : "...I think so.");
    Say("The visitor", "I know.");
    if (DadFree()) Say("Dad", "(quietly, not looking at you) Go with him, buddy. It's okay. It's... it's okay.");
    if (MomHere()) Say("Mom", "(she's holding the door frame like it's the only thing keeping her up)");
    CollectionScript();
}

void Story::ColdOut() {
    // v0.8: the snowstorm gets you
    chase = false;
    Say("", "(You can't feel your hands any more. The snow has stopped being cold.)");
    Say("", "(That's nice. You sit down. Just for a second.)");
    FadeFor(0, "");
    Black("It's warm now.");
    Black("White.");
    AddClue("cold");
    Achieve("Snow Angel");
    Narrate("No. That's not how it went. The snow didn't get me. Not that time.");
    Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
}

void Story::SceneDone(const std::string& id) {
    if (id == "dad_runs") {
        // v0.7: Dad went at him with a lamp. He's on the floor now, not dead. Not yet.
        dadDown = true; dadOnFloor = true;
        Say("", "(A sound like a plank snapping. Dad folds up on the floor and doesn't get up.)");
        Bubble("Mom", "No no no. Get up. Please get up.");
        AddClue("dad_ran");
        Achieve("Ten Seconds");
        Bubble("The visitor", "Now. Where were we, 06?");
        SceneBeat("chase");
    }
}

void Story::VisitorChoice() {
    std::vector<ChoiceOpt> o;
    if (visitorDowned && hasKnife && tainted) o.push_back({"c_finish", "Finish it.", false, true});
    if (visitorDowned && !hasKnife) o.push_back({"c_kick", "Kick him while he's down."});
    o.push_back({"c_go", threats > 0 ? "\"Stop! I'm here. Leave them alone.\"" : "Go with him."});
    if (!visitorDowned) {
        if (hasKnife) o.push_back({"c_knife", "Pull the knife.", false, true});
        else if (hasStick) o.push_back({"c_stick", "Swing the stick at him.", false, true});
        else o.push_back({"c_fight", "Fight him."});
        if (hasLight && !lightUsed && PowerOut()) o.push_back({"c_light", "Shine the flashlight in his eyes."});
    }
    if (Knows("visitor_time")) o.push_back({"c_know", "\"I know what you are.\"", true});
    if (Knows("handlers") && (MomHere() || DadFree())) o.push_back({"c_parents", "\"Mom. Dad. Did you know?\"", true});
    o.push_back({"none", "(Back away.)"});
    Choice(visitorDowned ? "He's on the floor. He's still bigger than you." : "He's huge. He's calm. He's right in front of you.", o);
}

void Story::OnCaught() {
    if (!chase) return;
    if (doggoWithKid && DoggoExists() && !Tutorial()) { DoggoSacrifice(); return; }
    chase = false;
    Say("", "(A hand like a vice closes on your collar.)");
    Say("The visitor", "Gotcha.");
    AddClue("he_runs_fast");
    CollectionScript();
}

void Story::OnFound() {
    if (!chase) return;
    if (doggoWithKid && DoggoExists() && !Tutorial()) { DoggoSacrifice(); return; }
    chase = false;
    Say("", "(Light. A big hand.)");
    Say("The visitor", "Found you. You always pick that one.");
    AddClue("hiding");
    Achieve("Hide and Seek");
    CollectionScript();
}

void Story::OnLeftPremises() {
    if (!chase) return;
    chase = false;
    Say("", "(Past the fence. Snow to your knees. You're actually doing it.)");
    Say("", "(Then it's all white. No house. No fence. Just footprints ahead of you. Small ones. Yours.)");
    Sound("thud");
    Say("The visitor", "(out of the white, carrying you back like a sack of potatoes) 31 kilometres, 06. In this? You walked in a circle.");
    AddClue("run_fail");
    Achieve("Nowhere to Run");
    CollectionScript();
}

void Story::OnVisitorUp() {
    if (!visitorDowned) return;
    visitorDowned = false;
    Say("The visitor", "(getting up, slow, one arm hanging wrong) ...Okay. That one hurt.");
}

void Story::CollectionScript() {
    chase = false;
    FadeFor(0, "");
    Sound("hush_on");   // v0.9: no music for this part
    Black("Rope. The couch. He ties knots like he's done it a thousand times.");
    bool momLeft = MomHere() || momLocked;
    bool dadLeft = !dadDown;
    if (momLocked) Black("He goes down to the basement. He knows exactly where she is.");
    if (dadLocked) Black("He goes out to the shed. He knows exactly where Dad is.");
    if (!momLocked && MomHere()) {
        if (tainted) Black("He walks Mom into the kitchen. She doesn't look at you on the way.");
        else Black("He walks Mom into the kitchen. She goes quietly. Like she doesn't understand yet.");
    }
    if (!dadLocked && dadLeft && dadOnFloor) Black("He drags Dad in after her. Dad is trying to say your name.");
    else if (!dadLocked && dadLeft) Black("Dad goes too. He tries to say something to you. The man shakes his head.");
    if (DoggoExists()) Black("Doggo follows them. Wagging.");
    if (doggoDead) Black("You look for Doggo. Then you remember.");
    if (momLeft) { Sound("bang"); Black("BANG."); }
    if (dadLeft) { Sound("bang"); Black("BANG."); }
    if (DoggoExists()) { Sound("bang"); Black("Doggo stops barking."); }
    if (stance == Stance::Normal && (momLeft || dadLeft)) Black("They never knew they were part of the clean-up. Somehow that's worse.");
    Black("He carries you out into the storm.");
    Black("The trunk smells like oil and wet dog.");
    AddClue("visitor_time");
    if (FullMemory()) {
        Black("The lid comes down. A hand comes down too. Stay awake. STAY AWAKE.");
        Beat q; q.type = BeatType::QTE; q.text = "STAY AWAKE"; beats.push_back(q);
        return;
    }
    TrunkScript();
}

void Story::TrunkScript() {
    Sound("thud");
    Black("The lid comes down. Something hits your head.");
    Sound("bells");
    Black("Far away, through the storm, church bells. Christmas is being rung in.");
    static const char* lines1[] = {
        "No. That's not how it went. Let me start again.",
        "No, wait. I'm telling it wrong. From the top.",
        "That's not... no. Again.",
    };
    static const char* lines2[] = {
        "Again.",
        "I knew how it ended. I just kept letting it.",
        "They always went quietly. Every single time.",
    };
    static const char* lines3[] = {
        "Yeah. That's how it went. I'm not fixing it.",
        "He took me. Again. Let him think he's winning.",
        "I let it stand. They had it coming. Some of them.",
    };
    int st = Stage();
    Narrate((st >= 3 ? lines3 : st >= 2 ? lines2 : lines1)[loop % 3]);
    Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
}

void Story::QTEResult(bool stayedAwake) {
    if (stayedAwake) { SecretEnding(); return; }
    Black("You almost made it.");
    TrunkScript();
}

void Story::SecretEnding() {
    Black("You don't black out. You pretend to.");
    Black("Two hours of snow chains. Then asphalt. Then a gate. A voice: \"Subject 06, delivered.\"");
    Black("Finse. A building that isn't on any map.");
    Black("You've never been here. You know every hallway anyway. You've had a lot of practice dying.");
    Black("Rooms with kids in them. Twelve years old, all of them. All wearing the same watch.");
    Black("You find the machine. It hums like the basement.");
    Sound("glass");
    Black("You break it. Then you break everything else. You don't stop for a long time.");
    Black("You open every door. They run out into the snow. Nobody stops them. Nobody's left to.");
    Black("Outside the sky is pink. You did it. You actually");
    Black("A red dot on your chest.");
    Sound("bang");
    Flash();
    Sound("beep");
    Black("BEEP BEEP.");
    Black("16:00. Your bed. Your room. Your watch.");
    Black("You scream until there's no sound left. Downstairs, Mom calls your name.");
    Narrate("Whatever was still soft in me stayed in that building. Only the dark came back.");
    Narrate("The longest surviving version of me is still in that bed. He always will be.");
    darkPoints = DARK_MAX;   // v0.7: the darkest ending. You wake up as dark as it gets
    tainted = true;
    endings.insert("secret");
    Achieve("The Longest Survivor");
    Beat e; e.type = BeatType::Ending; e.id = "secret"; e.speaker = "SECRET ENDING";
    e.text = "THE LONGEST SURVIVOR\n\nHe set them free. He never got to leave.";
    beats.push_back(e);
}

void Story::MonsterEnding() {
    bool endProgram = Knows("basement_files") && Knows("program_hq");
    darkPoints = DARK_MAX;
    FadeFor(0, "");
    Black("You don't remember deciding. Just the sound.");
    Black("He doesn't get up.");
    if (MomHere()) {
        Black("Mom comes out of the kitchen. She looks at you like you're the thing that came through the door.");
        Black("\"What ARE you?\"");
    }
    if (!endProgram) {
        Black("The storm blows itself out by morning. You walk out into the white. Nobody follows. Nobody calls your name.");
        Narrate("I survived. That's all I did.");
        endings.insert("monster");
        Achieve("Monster");
        Beat e; e.type = BeatType::Ending; e.id = "monster"; e.speaker = "ENDING";
        e.text = "MONSTER\n\nHe lived. Nobody wanted him to.";
        beats.push_back(e);
        return;
    }
    Black("His keys. His radio. The files said Finse.");
    Black("You can't drive. You learn. It takes nine loops.");
    Black("Loop after loop. The gate code. The guard rota. The hallway that creaks.");
    Black("You find the machine. It hums like the basement.");
    Sound("glass");
    Black("You break it. You keep breaking it after it stops humming.");
    Narrate("I thought that would end it. The loop. Mine.");
    Sound("beep");
    Black("BEEP BEEP. 16:00.");
    Narrate("It didn't. It only ended theirs.");
    endings.insert("program");
    Achieve("End of the Program");
    Beat e; e.type = BeatType::Ending; e.id = "program"; e.speaker = "ENDING";
    e.text = "END OF THE PROGRAM\n\nThe machine is gone. The loop isn't.";
    beats.push_back(e);
}

void Story::MinigameDone(int score) {
    FadeFor(10, "Playing Flappy Plane");
    if (score >= 10) Say("You", "(" + std::to_string(score) + " towers. New personal best. Nobody will ever know.)");
    else if (score > 0) Say("You", "(" + std::to_string(score) + (score == 1 ? " tower" : " towers") + ". The jet is harder than it looks.)");
    else Say("You", "(Zero. The paper plane hates me.)");
    FliptendoPlayed("flappy");
}

void Story::FliptendoPlayed(const std::string& what) {
    (void)what;
    playStreak++;
    if (playStreak == 2) Say("You", "(Okay. One more. Then I'll do something.)");
    if (playStreak >= 3 && !collection) ScreenTimeEnding();
}

void Story::ScreenTimeEnding() {
    FadeFor(0, "");
    Black("One more. Then one more. Then one more.");
    Black("Somewhere far away, three slow knocks.");
    Black("Mom's voice. \"...You're early.\"");
    Black("A big hand on your shoulder. \"Come on, 06.\"");
    Black("\"Can I bring my Fliptendo?\"");
    Black("He shrugs. \"Sure.\"");
    Black("You play the whole drive. You never look up. You never find out where you went.");
    Narrate("The longest loop I ever had was the one where I didn't notice.");
    endings.insert("screen");
    Achieve("Screen Time");
    Beat e; e.type = BeatType::Ending; e.id = "screen"; e.speaker = "JOKE ENDING";
    e.text = "SCREEN TIME\n\nHe reached the last dungeon. He didn't notice anything else.";
    beats.push_back(e);
}

void Story::TiredOut() {
    // v0.6: the clock keeps going during the chase. At 17:00 the kid just runs out.
    chase = false;
    Sound("bells");
    Say("", "(17:00. Far away, bells. You've run. You've hidden. You've run again.)");
    Say("", "(Your legs just stop. You curl up in the snow behind the woodpile for one second.)");
    Say("The visitor", "(much later, gently) There you are. Long day, huh?");
    Achieve("Tired Out");
    CollectionScript();
}

void Story::Threat(int n) {
    // v0.7: he can't find you, so he makes them pay. A blink of black hides him walking back to them.
    threats = n;
    bool dad = DadFree() || dadOnFloor, mom = MomHere() && !momLocked;
    FadeFor(0, "");
    if (n == 1) {
        Sound("knock");
        Say("The visitor", std::string("(loud, from the living room) Zero-six! Three minutes. Then I start on your ") + (dadOnFloor ? "dad again." : dad ? "dad." : mom ? "mom." : "family."));
        Bubble("Doggo", "*whimpers*");
    } else if (n == 2) {
        Sound("crack");
        if (dad) { Say("", "(A sound from the living room. Dad screams. Then he stops screaming.)"); dadOnFloor = true; dadDown = true; }
        else if (mom) { Say("", "(Mom's voice. Begging. Then a crack. Then just crying.)"); momOnFloor = true; }
        else Say("", "(Something heavy breaks downstairs. He's not looking anymore. He's just breaking things.)");
        DarkAct(1, "", true);
        Say("You", darkPoints >= DARK_REVEL ? "(I could come out. I don't want to.)" : "(I could come out. I could. I'm still hiding.)");
        Say("The visitor", mom && dad ? "Two minutes, 06. Mom's next." : "Two minutes, 06.");
    } else {
        Sound("crack");
        if (mom && !momOnFloor) { Say("", "(Mom. Begging. Then a crack. Then just crying, very quietly.)"); momOnFloor = true; }
        else if (dad) Say("", "(Dad again. You put your hands over your ears. It doesn't help.)");
        else Say("", "(Glass. Wood. Your room, probably. He knows where you sleep.)");
        DarkAct(1, "", true);
        Say("You", darkPoints >= DARK_REVEL ? "(Let him.)" : "(Hands over my ears. It doesn't help. Nothing helps.)");
        Say("The visitor", "(quiet now) One minute. I'm not counting out loud anymore.");
        AddClue("they_paid");
    }
}

void Story::PaidEnding() {
    chase = false;
    Sound("bells");
    Say("", "(17:00. Far away, the bells start. The house is completely quiet.)");
    Say("", "(Footsteps. Slow. He doesn't search anymore. He walks straight to your hiding spot. He always knew.)");
    Say("The visitor", "(crouching down) You let them pay for you. Huh.");
    Say("The visitor", "They said you were different. You're not. You're just like us.");
    FadeFor(0, "");
    Black("He carries you out past them.");
    if (MomHere() || momOnFloor) Black("Mom is still crying. She doesn't look up.");
    if (dadOnFloor) Black("Dad isn't moving.");
    Black("You don't cry. You notice that you don't cry.");
    DarkAct(2, "", true);
    endings.insert("paid");
    Achieve("They Paid");
    Beat e; e.type = BeatType::Ending; e.id = "paid"; e.speaker = "ENDING";
    e.text = "THEY PAID\n\nHe stayed hidden. Someone always pays.";
    beats.push_back(e);
}

void Story::EscapeEnding() {
    chase = false;
    FadeFor(0, "");
    Black("Dad swings the lamp before the man is all the way through the door. It doesn't knock him out. It doesn't have to.");
    Black("Mom already has his keys. You're already running.");
    Black("The car smells like oil and wet dog. Mom drives into the storm. Dad holds his ribs in the back seat and laughs.");
    Black("You've never heard him laugh like that. Like it isn't for you.");
    if (DoggoExists()) Black("Doggo is in your lap. His heart is going as fast as yours.");
    Black("His tracks are already filling in. The road is closing behind you. Nobody follows. Nobody could.");
    Black("Mom keeps checking the mirror. After a while she stops checking.");
    Black("\"Where do we go?\" \"Anywhere. Anywhere at all.\"");
    Black("A road sign in the headlights. 31.");
    Sound("beep");
    Black("Beep. From behind your ear. From behind Mom's ear. From behind Dad's.");
    Black("Mom touches the little scar behind her own ear. She didn't know. None of them did.");
    Flash();
    Sound("bang");
    Black("White.");
    Narrate("31 kilometres. Exactly as far as the leash goes. For all of us.");
    endings.insert("escape");
    Achieve("False Hope");
    Beat e; e.type = BeatType::Ending; e.id = "escape"; e.speaker = "ENDING";
    e.text = "FALSE HOPE\n\nThey ran together. The leash was exactly 31 kilometres long.";
    beats.push_back(e);
}

void Story::DoggoShot(bool onScreen) {
    // v0.7: hurt him and he takes Doggo first. Off screen. Then you see what's left.
    doggoDead = true;
    if (onScreen) { doggoDeadX = 19; doggoDeadY = 19; }
    Say("Doggo", "*goes for him. Barking. Teeth. Everything he has.*");
    Sound("bang");
    FadeFor(0, "");
    SceneBeat("doggo_dies");   // the presentation swaps in the body during the black
    Black("BANG.");
    Black("A yelp. Short. Then nothing.");
    if (onScreen) {
        Say("", "(Doggo is lying by the door. He isn't moving. A thin dark line is running out from under him.)");
        Say("You", "(Get up. Get up, boy. Please.)");
        Say("The visitor", "(stepping over him) That was on you, 06.");
    }
}

void Story::DoggoSacrifice() {
    // v0.7: he was in the room. He always is, unless you tell him to stay.
    // v0.9: the first time the loop carries on, so you learn it before it costs you an ending
    bool first = !Knows("doggo_stay");
    chase = false;
    doggoDead = true;
    Say("", "(His hand is already reaching for you.)");
    Say("Doggo", "*comes out of nowhere. Between you and him. Teeth.*");
    Sound("bang");
    FadeFor(0, "");
    SceneBeat("doggo_dies");   // the presentation swaps in the body during the black
    Black("BANG.");
    Black("A yelp. Short. Then nothing.");
    Say("", "(Doggo is lying at your feet. He isn't moving. A thin dark line is running out from under him.)");
    Say("You", "(No. No no no. Get up. GET UP.)");
    Say("The visitor", "(wiping his sleeve) ...Good dog. Shame.");
    AddClue("doggo_stay");
    if (first) {
        Say("You", "(He got in front of me. I never told him to stay.)");
        CollectionScript();
        return;
    }
    Achieve("Good Boy");
    FadeFor(0, "");
    Black("You don't fight when he picks you up. You can't stop looking back.");
    Narrate("He was the only one in that house who never lied to me.");
    endings.insert("doggo");
    Beat e; e.type = BeatType::Ending; e.id = "doggo"; e.speaker = "REALLY BAD ENDING";
    e.text = "GOOD BOY\n\nHe did the only thing he knew how to do.\n\nNext time, tell him to stay. Or shut a door.";
    beats.push_back(e);
}

void Story::ConvinceParents(const std::string& who) {
    // v0.7: befriended + proof. They didn't know they were part of the clean-up.
    bool proofPhone = Knows("phone_answer");
    solemn = true;
    Say("You", "I need you to listen. Really listen. The man at 16:50. The clean-up. It's not just me. It's you too.");
    if (tainted) Narrate("I said it the way they taught me to say things. Soft. Scared. Every word exactly where it would hurt. It worked.");
    Say(who, who == "Mom" ? "(she laughs, then stops) Clean-up is... no. We're being reassigned. They promised." : "(he rubs his face) Buddy, no. We're done after today. We retire. They promised.");
    if (proofPhone) Say("You", "\"Handler two, confirm pickup. Clean-up included.\" That's what they said on the phone. Clean-up is you.");
    else Say("You", "The files in the basement. Handlers are not to be informed. They think they're retiring. That's you.");
    Say(who, "(very quietly) ...How do you know those words?");
    if (who == "Mom") Say("Mom", "(she sits down on the kitchen floor. Just sits.) ...Get your father.");
    else Say("Dad", "(he puts the wrench down very carefully) ...Get your mother.");
    FadeFor(5, "At the kitchen table. All three of you.");
    Say("Dad", "His car. He always leaves the keys in it.");
    Say("Mom", "When he knocks, we go. All three of us. We don't stop for anything.");
    if (Knows("the_tape")) Say("You", "(I'm not doing this because I forgive them. I will never forgive them. I just need them.)");
    else Say("You", "(For one second it feels like having parents.)");
    parentsAllied = true;
    AddClue("allies");
    Achieve("Not Alone");
    solemn = false;
}

void Story::DeadSwitchLoop() {
    // Killing him isn't a way out. Not like this.
    darkPoints = DARK_MAX;
    tainted = true;
    FadeFor(0, "");
    Black("You don't remember deciding. Just the sound.");
    Black("He doesn't get up.");
    Black("Then the box on his belt starts to scream. One long tone.");
    Black("Behind your ear, where the little scar is, something gets hot. Then hotter.");
    Sound("beep");
    Flash();
    Black("White.");
    AddClue("dead_switch");
    Narrate("He took me with him. Of course he did. They'd never send him without a leash on me.");
    Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
}

// ============================================================ knife
void Story::KnifeCheck() {
    int needed = tainted ? 2 : 5;
    std::vector<ChoiceOpt> o = {{"k_take", "Take it.", false, true}, {"k_no", "Put it back."}};
    if (tainted) {
        if (knifeChecks == 0) Narrate("You again. Fine.");
        else Say("You", "(It fits my hand now.)");
    } else {
        switch (knifeChecks) {
        case 0: Say("You", "(A kitchen knife. It's heavier than it looks.)"); break;
        case 1:
            if (DoggoExists()) Say("Doggo", "*stops wagging. Watches you.*");
            else Say("You", "(The house is very quiet.)");
            break;
        case 2: Say("Mom", "(from somewhere) Sweetie? What are you doing in the drawers?"); break;
        case 3: Narrate("I remember the weight of it. I remember thinking: just to hold it."); break;
        default: Narrate("Last chance. Put it back. Please."); break;
        }
    }
    (void)needed;
    Choice(std::string("Take the knife? (") + std::to_string(knifeChecks + 1) + "/" + std::to_string(needed) + ")", o);
}

// ============================================================ choices
void Story::Choose(const std::string& id, int px, int py) {
    (void)px; (void)py;
    if (id.rfind("ft_", 0) != 0 && id != "none") playStreak = 0;   // anything else breaks the gaming streak
    // ---- v0.7 doors
    if (id.rfind("dr_", 0) == 0) {
        size_t c = id.find(':');
        const DoorInfo* d = c == std::string::npos ? nullptr : DoorById(id.substr(c + 1));
        if (!d) return;
        int k = d->y * World::W + d->x;
        std::string act = id.substr(3, c - 3);
        if (act == "open") { OpenDoor(d->x, d->y); Sound("door"); }
        else if (act == "close") { closedDoors.insert(k); Sound("door"); }
        else if (act == "lock" || act == "lockc") {
            closedDoors.insert(k); lockedDoors.insert(k); Sound("door");
            Say("You", "(Click.)");
            if (Once("master_key") && !Knows("dad_key")) Say("You", "(The basement key fits. It fits every lock in this house.)");
        } else if (act == "unlock") { lockedDoors.erase(k); Say("You", "(Click. Unlocked.)"); }
        return;
    }
    if (id == "e_run") { EscapeEnding(); return; }
    if (id == "e_stay") {
        Say("Mom", "(she lets go of your hand) ...Okay. Okay. Hide. HIDE.");
        parentsAllied = false;
        Say("Dad", "(grabbing a lamp) Buddy. Run. RUN!");
        SceneBeat("dad_runs");
        return;
    }
    if (id == "b_unlock") {
        basementUnlocked = true;
        Say("You", "(Click. The basement door is unlocked now. It stays that way until 17:00.)");
        return;
    }
    if (id == "cab_tape" || id == "cab_tape2") {
        if (id == "cab_tape") keyInDrawer = true;   // the key stays in the cabinet lock
        Hold(Item::Tape, true);
        if (Once("tape_take")) Say("You", "(You leave the key in the lock and take the tape. It's heavier than a tape should be.)");
        else Say("You", "(You take the tape again.)");
        return;
    }
    if (id == "cab_key") { Hold(Item::Key, true); keyInDrawer = false; Say("You", "(You take the key back out of the lock.)"); return; }
    if (id == "vt_play") {
        Sound("hush_on");   // v0.9: nothing under the tape but the tape
        FadeFor(0, "");
        Black("The tape crackles. The basement. This basement. The same chair.");
        Black("A boy in the chair. Six, maybe. A watch that's much too big for his wrist.");
        Black("You know the watch. You know the boy.");
        Black("Dad's voice, off camera, bored. \"Session one, week three hundred and twelve. Subject is resisting.\"");
        Black("You don't watch the next part. You can't. You hear it anyway.");
        Black("You hear yourself. You didn't know you could make that sound.");
        Black("You hear Mom. She's humming. The same carol she hums in the kitchen every Christmas Eve.");
        Black("It goes on for a long time. The clock in the corner of the tape says forty-one minutes.");
        Black("Then Mom walks into the picture with a glass of juice. A straw in it. She smiles at the camera.");
        Black("\"There we go, sweetie. Drink up. You won't remember any of this.\"");
        Black("The boy drinks. He stops crying. He smiles back at her.");
        Black("That's the worst part. You're smiling.");
        Sound("hush_off");
        FadeFor(5, "The tape hisses. Snow on the screen.");
        Say("You", "(I'm shaking. I can't make it stop.)");
        Say("You", "(The juice at 15:55. Every day. Every single day.)");
        Narrate("I never called her Mom again. Not out loud. Not in my head. Not even when it would have helped.");
        AddClue("the_tape");
        Achieve("Session One");
        return;
    }
    if (id == "g_come") { doggoStay = false; Say("Doggo", "*bounces up like he's been waiting his whole life for this*"); return; }
    // ---- v0.6 items, chores and the Fliptendo
    if (id == "it_back") {
        static const char* line[] = { "", "(You put the knife back in the block.)", "(You put the key back in the little box.)",
            "(You drop the stick on the woodpile.)", "(You put the trash bag back in the bin.)", "(You put the dog food back.)", "(You put the flashlight back.)",
            "(You put the tape back and take the key out of the lock.)" };
        if (held == Item::Tape) { Say("You", line[(int)held]); Hold(Item::Key, true); keyInDrawer = false; return; }
        Say("You", line[(int)held]);
        Hold(Item::None);
        return;
    }
    if (id == "pa_food") { Hold(Item::DogFood); Say("You", "(A scoop of dog food. Doggo can hear it from anywhere in the house.)"); return; }
    if (id == "pa_light") { Hold(Item::Flashlight); Say("You", "(Dad's big flashlight. Heavy. It'll be dark tonight.)"); return; }
    if (id == "wp_stick") { Hold(Item::Stick); Say("You", "(A good stick. A really good stick. Doggo is already looking at it.)"); return; }
    if (id == "wp_stack") {
        FadeFor(Dur(20), "Stacking firewood. Log. After. Log.");
        firewoodDone = true;
        Say("You", "(Twenty minutes of logs. My arms are noodles. This had better be worth it.)");
        return;
    }
    if (id == "tr_zulda") {
        zuldaOpened = true;
        Say("You", "(Wrapping paper everywhere. A download code. ZULDA: FLUTE OF TIME!)");
        Say("You", "(Best. Day. Ever.)");
        if (Stage() >= 1) Say("You", "(It's the same day as always. It's still the best one.)");
        return;
    }
    if (id == "tr_tags") {
        Say("", "(To Kiddo, from Mom and Dad. To Kiddo, from Julenissen.)");
        Say("You", "(Julenissen has the same handwriting as Mom. That's normal. Right?)");
        if (Knows("handlers")) Say("", "(Small print on the back of every tag: S06.)");
        return;
    }
    if (id == "tr_shake") {
        Say("You", Stage() >= 1 ? "(No rattle. Nothing moves. It never does.)" : "(No rattle. Must be socks. Please don't be socks.)");
        return;
    }
    if (id == "tr_open") {
        openedEarly = true;
        Say("You", "(Just one. Just a corner.)");
        Say("", "(Paper. A box. A bow. You lift the lid.)");
        Say("", "(There's nothing in it. The tape on the inside has gone yellow. Somebody wrapped this years ago.)");
        Say("You", "(I close it again. I put it back exactly where it was.)");
        AddClue("presents");
        ChangeMood(-1);
        return;
    }
    if (id == "c_coat") {
        hasCoat = true;
        Say("You", "(Zipped to the chin. Warm. For now.)");
        return;
    }
    if (id == "ft_zulda") {
        FadeFor(10, "Playing Zulda: Flute of Time");
        Say("You", "(A boy in green plays a song that sends him back in time. Weird. Relatable.)");
        FliptendoPlayed("zulda");
        return;
    }
    if (id == "ft_flappy") { SceneBeat("minigame"); return; }
    if (id == "c_stick") {
        chase = false;
        AddDark(1);
        if (DoggoExists() && !Tutorial()) DoggoShot(false);
        Say("", "(You swing the stick at his head. It snaps across his forearm.)");
        Say("The visitor", "(looking at the two halves) ...Cute.");
        Sound("bang");
        FadeFor(0, "");
        Black("BANG.");
        Black("Your leg stops working.");
        AddClue("visitor_armed");
        Hold(Item::None, true);
        CollectionScript();
        return;
    }
    if (id == "c_light") {
        lightUsed = true;
        visitorStun = 3.5f;
        Say("", "(You shine the flashlight straight into his eyes. He swears and turns his face away.)");
        Say("You", "(Run. RUN.)");
        return;   // the chase goes on
    }
    // ---- collection
    if (id == "c_go") {
        chase = false;
        Say("You", "...Okay.");
        Say("The visitor", "Good boy.");
        CollectionScript();
    } else if (id == "c_fight" || id == "c_kick") {
        chase = false;
        if (id == "c_kick") Say("", "(He catches your ankle without even looking.)");
        else Say("", "(You swing at him with everything you've got. He doesn't even blink.)");
        if (DoggoExists() && !Tutorial()) DoggoShot(false);
        Sound("bang");
        FadeFor(0, "");
        Black("BANG.");
        Black("Your leg stops working. The floor comes up fast.");
        Black("He sighs. Like you made extra paperwork.");
        kidShot = true;
        AddClue("visitor_armed");
        CollectionScript();
    } else if (id == "c_knife") {
        chase = false;
        AddDark(2);
        if (DoggoExists() && !Tutorial()) DoggoShot(false);
        Say("", "(He catches your wrist like he's been expecting it. Twists. The knife clatters.)");
        Say("The visitor", "There it is. They said you'd get there eventually.");
        Sound("bang");
        FadeFor(0, "");
        Black("BANG.");
        Black("Your leg stops working.");
        AddClue("visitor_armed");
        CollectionScript();
    } else if (id == "c_finish") {
        Say("", "(He looks up at you. He doesn't beg. He just looks.)");
        std::vector<ChoiceOpt> o;
        if (Knows("dead_switch") && !switchTaken) o.push_back({"c_switch", "Take the box off his belt first.", true});
        o.push_back({"c_finish2", "Do it.", false, true});
        o.push_back({"none", "(Back away.)"});
        Choice("Are you sure?", o);
    } else if (id == "c_switch") {
        switchTaken = true;
        Say("", "(A black box on his belt. A wire runs up under his shirt. You unclip it. A green light keeps blinking.)");
        Say("The visitor", "(very quietly) ...Who told you about that?");
        Choice("Are you sure?", {{"c_finish2", "Do it.", false, true}, {"none", "(Back away.)"}});
    } else if (id == "c_finish2") {
        chase = false;
        if (switchTaken) MonsterEnding();
        else DeadSwitchLoop();
    } else if (id == "c_know") {
        chase = false;
        Say("You", "I know what you are.");
        Say("The visitor", "(he smiles) Good. They said you might.");
        if (MomHere()) Say("Mom", "...What? What does that mean?");
        AddClue("visitor_knows");
        CollectionScript();
    } else if (id == "c_parents") {
        chase = false;
        Say("You", "Mom. Dad. Did you know?");
        if (MomHere()) Say("Mom", "(she looks at the floor)");
        if (DadFree()) Say("Dad", "Know what? Buddy, he's taking you to the next place. That's all. And we get to... we get to retire.");
        Say("The visitor", "(he smiles at Dad) Sure you do. Everyone gets told something.");
        CollectionScript();
    }
    // ---- knife
    else if (id == "k_take") {
        knifeChecks++;
        if (knifeChecks >= (tainted ? 2 : 5)) {
            Hold(Item::Knife);
            Say("You", tainted ? "(Got it.)" : "(You take it. You hide it in your sleeve. Just in case. Just to hold.)");
        } else {
            KnifeCheck();
        }
    } else if (id == "k_no") {
        knifeChecks = 0;
        Say("You", "(You put it back. Your hand is sweating.)");
    }
    // ---- bed
    else if (id == "m_nisse") {
        Say("You", "Is Julenissen really coming?");
        Say("Mom", tainted ? "(she doesn't smile) Ten to five." : "Ten to five, sweetie. He's never late.");
        if (Knows("visitor_time")) Say("You", "(No. He never is.)");
    }
    else if (id == "bed_sleep") {
        FadeFor(Dur(10), "Napping");
        Say("You", Stage() >= 1 ? "(Sleeping doesn't skip anything. I checked.)" : "(That was nice.)");
    }
    // ---- mom
    else if (id == "m_dinner") {
        Say("Mom", tainted ? "(she watches your hands) ...Ribbe." : "Ribbe.");
        Say("You", "(Again.)");
    } else if (id == "m_allow") {
        Say("Mom", "When the chores are done. You know the deal.");
    } else if (id == "m_snack") {
        FadeFor(Dur(5), "Snack");
        Say("You", "(Pepperkaker. Still warm. Classic.)");
    } else if (id == "m_born") {
        Say("You", "Where was I born?");
        Say("Mom", "Haukeland. In Bergen. It was raining.");
        Say("You", "(She says it like a line she practised.)");
    } else if (id == "m_baby") {
        Say("You", "Why are there no baby photos of me?");
        Say("Mom", "The camera broke.");
        Say("You", "For four years?");
        Say("Mom", "(she starts wiping a clean counter)");
    } else if (id == "m_pickup") {
        Say("You", "Why does it say PICKUP behind door 24?");
        Say("Mom", "(a tiny pause) The presents. Julenissen picks them up at the post office.");
        Say("You", "(Julenissen doesn't use the post office.)");
    } else if (id == "m_back") {
        Say("You", "Chores are stupid.");
        Say("Mom", "Excuse me? Try that again and see what happens.");
        ChangeMood(-1);
    } else if (id == "m_loop" && Befriended()) {
        if (Knows("phone_answer") || Knows("basement_files")) ConvinceParents("Mom");
        else {
            Say("You", "Mom, this day keeps happening. A man comes at 16:50 and...");
            Say("Mom", "(she looks at you for a long time) ...I believe that you believe it, sweetie.");
            Say("You", "(She didn't send me to my room. She just needs proof. Something she'd recognise.)");
        }
    } else if (id == "m_loop") {
        Say("You", "Mom, this day keeps happening. Over and over. A man comes at 16:50 and...");
        Say("Mom", "(her face goes very still)");
        Say("Mom", "That's enough. Your room. Twenty minutes. Go.");
        FadeFor(Dur(20), "Twenty minutes in your room", 5, 6);
        inBasement = false;
        Say("You", "(Okay. Telling Mom was a mistake. Noted.)");
        AddClue("told_mom");
        if (momAtt == Attitude::Normal) momAtt = Attitude::Cold;
    } else if (id == "m_phone") {
        Say("You", "Who's calling at 16:22?");
        Say("Mom", "(a tiny pause) Nobody, sweetie. Wrong number.");
        Say("You", "(It isn't 16:22 yet.)");
        ChangeMood(-1);
    } else if (id == "m_real") {
        Say("You", "Are you even my mom?");
        Say("Mom", "(she looks at you for a long time)");
        Say("Mom", "I raised you. Isn't that the same thing?");
    } else if (id == "m_car") {
        Say("You", "Why don't we have a car?");
        Say("Mom", "We don't need one. Everything we need is right here.");
    } else if (id == "m_knife") {
        Say("You", "(You let her see the knife.)");
        Say("Mom", "(she backs into the corner) ...Okay. Okay, sweetie. What do you want?");
        Say("You", "(Nothing. I just wanted to see her face.)");
        DarkAct(1, "");
        momAtt = Attitude::Scared;
    }
    else if (id == "m_sorry") {
        if (momAtt == Attitude::Scared) {
            Say("You", "I'm sorry. I don't know why I did that.");
            Say("Mom", "(she nods too fast) Okay. Okay. Just... put it back where it was. Please.");
            Say("You", "(She doesn't come out from behind the table.)");
            Once("mom_sorry_scared");
        } else {
            Say("You", "Sorry, Mom.");
            Say("Mom", "(a long breath) ...Fine. But no more stories today. Okay?");
            momAtt = Attitude::Normal;
            if (grounded) Say("You", "(The door is still locked, though.)");
        }
    }
    // ---- dad
    else if (id == "d_sorry") {
        Say("You", "Dad, I'm sorry. I didn't mean it.");
        Say("Dad", "(he doesn't move) ...Yeah. Okay, buddy. Why don't you go inside for a bit.");
        Once("dad_sorry_scared");
    } else if (id == "d_help") {
        helpedDad = true;
        FadeFor(Dur(15), "Helping Dad with the generator");
        Say("Dad", "Hand me the 10. No, the other 10. There you go.");
        Say("Dad", "(muttering to himself) Flag. Forty-four. Don't forget.");
        Say("Dad", "Hey. Whatever happens today... you were a good kid. Remember that.");
        Say("You", "(Whatever happens today?)");
        if (!Tutorial()) AddClue("dad_goodbye");
    } else if (id == "d_what") {
        Say("Dad", "Generator. Storm's coming. Power always goes when it storms up here.");
    } else if (id == "d_cards") {
        FadeFor(Dur(10), "Playing cards with Dad");
        Say("You", "(He lets you win. He always lets you win.)");
        if (Stage() >= 2) Say("You", "(I wonder if that's in the reports too.)");
    } else if (id == "d_pickup") {
        Say("You", "Why does it say PICKUP behind door 24?");
        Say("Dad", "...Presents. Mom's thing.");
        Say("You", "(He doesn't look at me when he says it.)");
    } else if (id == "d_loop" && Befriended() && (Knows("phone_answer") || Knows("basement_files"))) {
        ConvinceParents("Dad");
    } else if (id == "d_loop" && Befriended()) {
        Say("You", "Dad. I'm stuck. The same hour keeps repeating and a man is coming.");
        Say("Dad", "(he looks at you for a long time) ...You're scaring me a little, buddy.");
        Say("You", "(He didn't reach for the radio. He just needs proof. Something he'd recognise.)");
    } else if (id == "d_loop") {
        Say("You", "Dad. I'm stuck. The same hour keeps repeating and a man is coming.");
        Say("Dad", "(he laughs, then stops laughing)");
        Say("Dad", "...Okay, buddy. Okay. Let me make a call.");
        visitorTime = 40 * 60;
        if (t >= visitorTime - 60) visitorTime = (int)t + 120;
        Say("You", "(That felt like a mistake.)");
        AddClue("told_dad");
        if (dadAtt == Attitude::Normal) dadAtt = Attitude::Cold;
    } else if (id == "d_report") {
        Say("You", "Who do you report to on that radio?");
        Say("Dad", "(he goes pale) Go inside. Please. Just... go inside.");
    } else if (id == "d_knife") {
        Say("You", "(You let him see the knife.)");
        Say("Dad", "(he goes very still) Put that down. Please. You don't want to do this.");
        DarkAct(1, "");
        dadAtt = Attitude::Scared;
    } else if (id == "d_talk") {
        Say("Dad", "Buddy. Look at me. Whatever you think you know. This isn't you.");
        Say("Dad", "Please. PLEASE.");
        Choice("He's begging.", {{"d_talk2", "Make him tell you.", false, true}, {"none", "(Stop.)"}});
    } else if (id == "d_talk2") {
        FadeFor(11, "");
        Black("It took eleven minutes.");
        if (darkPoints >= DARK_REVEL) {
            Black("He told you after four.");
            Black("You kept going anyway.");
        } else {
            Black("He told you after four. You stopped.");
            Black("Your hands didn't stop shaking for a long time.");
        }
        Say("Dad", "(barely) ...The flagpole. The little box at the bottom. Please. Please.");
        AddClue("dad_key");
        dadAtt = Attitude::Broken;
        Achieve("Interrogation");
        DarkAct(4, "");
    }
    // ---- doggo
    else if (id == "g_pet") {
        Say("Doggo", "*happy wiggling*");
        if (Stage() >= 2) Say("Doggo", "*stops wiggling. Stares past you at nothing.*");
    } else if (id == "g_sit") {
        doggoStay = true;   // v0.7: he stays put until you call him
        if (chase) { Say("Doggo", "*lies down flat. Ears back. He stays.*"); return; }
        if (tainted) {
            Say("Doggo", "*sits. Instantly. He doesn't take his eyes off you.*");
            Say("You", "(He's never done that before. Like he's scared of what happens if he doesn't.)");
        } else if (Stage() >= 2) {
            Say("Doggo", "*sits... then slowly turns his head toward the hallway. Ears flat.*");
        } else {
            Say("Doggo", "*sits. Lifts a paw. Gets confused and lies down instead. Very proud of himself.*");
        }
        if (!Tutorial()) Say("You", "(Stay, boy. Stay. He'll stay right here until I call him.)");
    } else if (id == "g_ball") {
        FadeFor(Dur(3), "Playing ball with Doggo");
        if (Stage() >= 2) {
            Say("Doggo", "*comes back without the ball. Sits in the hallway. Staring at the basement door.*");
        } else {
            Say("Doggo", "*chases the ball under the couch and comes back with the ball AND a sock. Best day ever.*");
            ChangeMood(1);
        }
    } else if (id == "g_search") {
        FadeFor(Dur(5), "Doggo is searching");
        if (tainted && !hasKey) {
            // the dog knows where Dad hides it. He just never wanted you to have it
            Say("", "(Doggo sniffs his way across the yard and starts digging at the bottom of the flagpole.)");
            Say("", "(A tiny box, painted the same colour as the pole. He paws it open. A key.)");
            Say("Doggo", "*drops it at your feet and backs away. He doesn't want to be near it. Or you.*");
            Hold(Item::Key);
            AddClue("dad_key");
        } else if (darkPoints >= DARK_ARSENAL && !Knows("syringe")) {
            Say("", "(Doggo goes straight for the trash bins and roots around until he finds something.)");
            Say("", "(A syringe. Empty. A strip of tape on it: 06 - WEEKLY. Mom's handwriting.)");
            Say("You", "(Weekly. What am I getting every week?)");
            AddClue("syringe");
        } else {
            static const char* finds[] = {
                "*comes back with one of Dad's work gloves. Soggy. Very pleased.*",
                "*comes back with a pine cone and a look that says you're welcome.*",
                "*comes back with the TV remote. Mom has been looking for that since Tuesday.*",
            };
            Say("Doggo", finds[(int)(Rand() * 3) % 3]);
        }
    } else if (id == "g_tail") {
        Say("Doggo", "*YELP*");
        if (MomHere()) Say("Mom", "HEY! Leave him alone! What is wrong with you?");
        ChangeMood(-2);
    } else if (id == "g_stick") {
        FadeFor(Dur(5), "Throwing sticks for Doggo");
        Say("Doggo", "*brings it back. Every time. Like it's the best thing that ever happened.*");
    } else if (id == "g_shed") {
        Say("Doggo", "*looks up at you. Tail wagging. He'd follow you anywhere.*");
        Choice("Are you sure?", {{"g_shed2", "Take him to the shed.", false, true}, {"none", "No. Never."}});
    } else if (id == "g_shed2") {
        FadeFor(10, "", 34, 11);
        Black("You walk Doggo out to the shed.");
        Black("He's wagging the whole way.");
        Black("Only you walk back out.");
        doggoGone = true;
        DarkAct(10, "");
    }
    // ---- phone
    else if (id == "p_113") {
        Say("You", "(1. 1. 3.)");
        Say("", "(Nothing. Not even a dial tone anymore.)");
        Say("You", "(The line only goes one place. Not the police.)");
    }
    // ---- front door
    else if (id == "f_lock") {
        frontLocked = true;
        closedDoors.insert(20 * World::W + 20);
        frontLockedAt = t;
        Say("You", "(Click. Locked.)");
    } else if (id == "f_unlock") {
        frontLocked = false;
        Say("You", "(Unlocked.)");
    }
    // ---- shed door
    else if (id == "s_bar") {
        dadLocked = true;
        Say("Dad", "Buddy? Hey. HEY. Open the door. This isn't funny.");
        DarkAct(1, "(You drop the plank into the brackets. It fits perfectly. Like it was made for this.)");
    } else if (id == "s_free") {
        dadLocked = false;
        Say("Dad", "(stumbling out) What is WRONG with you?!");
        ChangeMood(-3);
    }
    // ---- basement door
    else if (id == "b_peek") {
        FadeFor(1, "Peeking down the stairs");
        Say("", "(Screens. Dozens of them. One shows your room. One shows the yard.)");
        Say("", "(One shows a boy on the stairs, looking at screens. That's you.)");
        Say("Mom", "(from below) GET OUT! Get out, NOW!");
        AddClue("basement_screens");
        ChangeMood(-2);
    } else if (id == "b_bolt") {
        momLocked = true;
        Say("", "(There's a bolt on the outside of the basement door. Why is there a bolt on the OUTSIDE?)");
        Say("Mom", "(from below) Sweetie? The door. Open the door. OPEN THE DOOR.");
        DarkAct(2, "(Clack.)");
    } else if (id == "b_free") {
        momLocked = false;
        Say("Mom", "(climbing out, shaking) Don't. Don't you ever...");
        ChangeMood(-3);
    } else if (id == "b_down") {
        basementUnlocked = true;
        inBasement = true;
        FadeFor(1, "Unlocking the basement", 6, 38);
        if (Once("first_down") && !achievements.count("Down the Stairs")) {
            Achieve("Down the Stairs");
            if (tainted) Narrate("They built a cage for what I'd become. I became it to find the cage.");
            else Narrate("I'd wanted to see the basement my whole life. I should have kept wanting.");
        }
    }
    // ---- arsenal
    else if (id == "a_marbles" || id == "a_tripwire" || id == "a_paint") {
        std::string trap = id == "a_marbles" ? "marbles" : id == "a_tripwire" ? "tripwire" : "paint";
        if (trapsSet.count(trap)) return;
        trapsSet.insert(trap);
        traps++;
        if (trap == "marbles") FadeFor(5, "Marbles on the front step", 20, 21);
        if (trap == "tripwire") FadeFor(10, "Tripwire across the porch", 20, 21);
        if (trap == "paint") FadeFor(10, "Paint can over the front door", 20, 19);
        Say("You", std::string("(Trap ") + std::to_string(traps) + " of 3. He comes through the front door. He always does.)");
        DarkAct(1, "");
    }
    // ---- basement console
    else if (id == "r_listen") {
        Say("Voice", "(static) ...unit 04 collected... unit 05 collected... unit 06, 16:50...");
    } else if (id == "r_cancel") {
        Say("You", "(into the mic) This is handler two. Cancel the pickup.");
        Say("Voice", "...Handler two?");
        Say("Voice", "That's not handler two. Stay where you are, 06. We're coming now.");
        AddClue("they_listen");
        visitorTime = t < 37 * 60 ? 40 * 60 : (int)t + 180;
    }
    // ---- visitor at the gate
    else if (id == "v_who") {
        Say("You", "Who are you?");
        Say("The visitor", "Just a man whose car broke down. Go inside, kid. It's cold.");
    } else if (id == "v_program") {
        Say("You", "You're from the program.");
        Say("The visitor", "(he smiles) Go inside, 06. I'll be right there.");
    } else if (id == "v_stab") {
        Say("", "(You go for him. He catches your wrist like he's been expecting it.)");
        AddDark(1);
        if (DoggoExists() && !Tutorial()) DoggoShot(false);
        t = (float)visitorTime;
        collection = true;
        Sound("bang");
        FadeFor(0, "");
        Black("BANG.");
        Black("Your leg stops working.");
        AddClue("visitor_armed");
        CollectionScript();
    }
    // ---- flagpole
    else if (id == "fl_key") {
        Hold(Item::Key);
        Say("You", "(The little box. The key. Right where he said.)");
    }
}

// ============================================================ interactions
void Story::Interact(const std::string& id, int px, int py) {
    (void)px; (void)py;
    if (id != "fliptendo") playStreak = 0;
    if (id == "visitor" && chase) { VisitorChoice(); return; }
    // v0.7: doors and Doggo still work while he's hunting you
    if (const DoorInfo* d = DoorById(id); d && id != "frontdoor" && id != "sheddoor" && (!collection || chase)) {
        int k = d->y * World::W + d->x;
        if (id == "door_kid" && grounded && !collection) { Say("You", "(Locked from the outside. Mom has the key.)"); return; }
        if (brokenDoors.count(k)) { Say("You", "(Splinters. He didn't bother with the handle.)"); return; }
        if (lockedDoors.count(k)) {
            if (hasKey) Choice("Locked with the master key.", {{std::string("dr_unlock:") + id, "Unlock it."}, {"none", "Leave it locked."}});
            else Say("You", "(Locked. Only the key opens it.)");
            return;
        }
        bool closed = closedDoors.count(k) > 0;
        if (!closed && px == d->x && py == d->y) { Say("You", "(I'm standing in the doorway.)"); return; }
        if (!hasKey) { if (closed) closedDoors.erase(k); else closedDoors.insert(k); Sound("door"); return; }
        if (closed) Choice("A closed door.", {{std::string("dr_open:") + id, "Open it."}, {std::string("dr_lock:") + id, "Lock it with the master key."}, {"none", "Leave it."}});
        else Choice("An open door.", {{std::string("dr_close:") + id, "Close it."}, {std::string("dr_lockc:") + id, "Close it and lock it."}, {"none", "Leave it."}});
        return;
    }
    if (id == "doggo" && chase) {
        if (doggoStay) Choice("Doggo is flat on the floor, staying.", {{"g_come", "\"Come on, boy!\""}, {"none", "(Let him stay.)"}});
        else Choice("Doggo is pressed against your legs, growling at the dark.", {{"g_sit", "\"Stay!\""}, {"none", "(Keep him close.)"}});
        return;
    }
    if (id == "coats" && chase && !hasCoat) { Choose("c_coat", px, py); return; }
    if (collection) return;
    if (TutorialLocked(id)) {
        Say("You", "(Not today. Maybe some other time.)");
        return;
    }
    bool dadInShed = world.RoomAt(Dad().x, Dad().y) == Room::Shed && DadFree();
    bool dadAtShed = world.RoomAt(Dad().x, Dad().y) == Room::Shed && !dadDown;

    if (id == "bed") {
        Choice("Lie down for a bit? (10 min)", {{"bed_sleep", "Nap."}, {"none", "Nah."}});
    } else if (id == "toys") {
        if (roomClean) { Say("You", "(Clean enough. Mom won't check under the bed. She never does.)"); return; }
        FadeFor(Dur(6), "Cleaning your room");
        roomClean = true;
        Say("You", "(Room: clean. Ish.)");
    } else if (id == "window") {
        if (CarHere()) { Say("You", "(Headlights at the gate. A black car. No plates.)"); return; }
        Say("You", Stage() >= 2 ? "(The vidda. White and endless. Like it's waiting.)"
                                : "(Blue dusk. Snow on everything. Nothing out there for kilometres.)");
    } else if (id == "toilet") {
        if (pooped) { Say("You", "(Already went. I'm good.)"); return; }
        int dur = shift == Shift::Toilet45 ? 45 : 15;
        FadeFor(dur, dur == 45 ? "Toilet. This is taking forever." : "Toilet");
        pooped = true;
        if (dur == 45) {
            Say("You", "(Forty. Five. Minutes. WHY.)");
            Achieve("Constipated by Fate");
            AddClue("poop45");
            seenToilet45 = true;
        }
    } else if (id == "mirror") {
        if (Stage() == 0) Say("You", "(That's me. Messy hair. Origin watch. Looking cool.)");
        else if (Stage() == 1) Say("You", "(That's me. Right? Yeah. That's me.)");
        else if (Stage() == 2) Say("You", "(My reflection blinks a little late. I'm sure it does.)");
        else {
            Say("You", "(My reflection is older. Grown up. It's smiling. I'm not.)");
            if (Stage() >= 4) Say("You", "(It mouths something. \"Welcome back.\")");
            if (!Knows("mirror_lag")) { Achieve("Who's There?"); AddClue("mirror_lag"); }
        }
    } else if (id == "desk") {
        FadeFor(10, "Reading Mom's papers");
        Say("", "WEEKLY REPORT. SUBJECT 06. Stable. Unaware. No signs of drift. Week 612.");
        if (tainted) Say("", "(A newer page underneath: SUBJECT 06. Drift confirmed. Request early collection.)");
        Say("You", "(Subject 06? 612 weeks is... that's my whole life.)");
        AddClue("reports");
        if (t >= 28 * 60 && t < 33 * 60 && MomHere()) {
            Say("Mom", "(from the hallway) What are you doing in our room?!");
            ChangeMood(-2);
        }
    } else if (id == "pbed") {
        Say("You", "(Two pillows. Perfectly made. It doesn't look slept in.)");
    } else if (id == "drawer") {
        // v0.7: Dad's cabinet. Same lock as the basement.
        if (drawerOpen) {
            std::vector<ChoiceOpt> o;
            if (held == Item::Tape) o.push_back({"it_back", "Put the tape back and take the key."});
            else {
                if (keyInDrawer) o.push_back({"cab_key", "Take the key out of the lock."});
                if (keyInDrawer || hasKey) o.push_back({keyInDrawer ? "cab_tape2" : "cab_tape", keyInDrawer ? "Take the tape." : "Take the tape. (leave the key in the lock)"});
            }
            o.push_back({"none", "Leave it."});
            Choice("Dad's cabinet is open. Papers, an old watch, a VHS tape: S06 - SESSION 1 - WEEK 312.", o);
            return;
        }
        if (hasKey) {
            drawerOpen = true;
            Say("You", "(The basement key fits Dad's cabinet too. Of course it does.)");
            Say("", "(Papers. An old watch. A VHS tape with a label in Dad's handwriting: S06 - SESSION 1 - WEEK 312.)");
            Choice("The tape.", {{"cab_tape", "Take the tape. (leave the key in the lock)"}, {"none", "Leave it."}});
            return;
        }
        Say("You", "(Dad's cabinet. Locked. Dad says he keeps the basement key on him. Always. He says it a lot.)");
    } else if (id == "coats") {
        Say("You", "(Dad's jacket. Mom's jacket. My jacket. No car keys anywhere.)");
        Say("You", "(We don't even have a car. 31 km to the nearest neighbour and we don't have a car.)");
        AddClue("no_car");
        if (!hasCoat) Choice("Your jacket is on the hook.", {{"c_coat", "Put it on."}, {"none", "Leave it."}});
    } else if (id == "basement") {
        if (momLocked) {
            Say("", "(Muffled pounding from below. Mom's voice, hoarse now.)");
            Choice("The bolt is still closed.", {{"b_free", "Let her out."}, {"none", "Leave it."}});
            return;
        }
        if (BasementOpen()) {
            std::vector<ChoiceOpt> o = {{"b_peek", "Peek down the stairs."}};
            if (!Tutorial()) o.push_back({"b_bolt", "Slam the door and bolt it.", false, true});
            o.push_back({"none", "Leave."});
            Choice("Mom is down there. The door is open.", o);
            return;
        }
        if (basementUnlocked) { Choice("The basement door. Unlocked. Blue light under it.", {{"b_down", "Go down."}, {"none", "Not yet."}}); return; }
        if (hasKey) {
            Choice("The basement key fits. Of course it fits.", {{"b_down", "Unlock it and go down."}, {"b_unlock", "Just unlock it. (keep it unlocked)"}, {"none", "Not yet."}});
            return;
        }
        Say("You", "(The basement. Locked. It's always locked.)");
        if (DoggoExists() && !Tutorial()) {
            Say("Doggo", "*growls at the door, low and steady*");
            AddClue("doggo_basement");
        }
    } else if (id == "phone") {
        int pm = Stage() >= 2 ? 21 : 22;
        bool ringing = t >= pm * 60 && t < pm * 60 + 40;
        if (ringing && !phoneAnswered) {
            phoneAnswered = true;
            if (Knows("phone_mom")) {
                Say("You", "Hello?");
                Say("Voice", "Handler two, confirm pickup at 16:50. Clean-up is included.");
                Say("You", "...Clean-up?");
                Say("", "*click*");
                if (MomHere()) Say("Mom", "Who was that? Give me that. Who was it?!");
                AddClue("phone_answer");
                ChangeMood(-1);
            } else if (MomHere()) {
                Say("Mom", "(snatching the phone) I've got it, sweetie.");
                Say("Mom", "Hello? ...Yes. Today. ...No, he doesn't suspect a thing.");
                AddClue("phone_mom");
            }
            return;
        }
        std::vector<ChoiceOpt> o = {{"none", "Hang up."}};
        if (Knows("visitor_time")) o.insert(o.begin(), {"p_113", "Call 113 (emergency).", true});
        Say("You", "(Dial tone.)");
        Choice("Who would you even call?", o);
    } else if (id == "fridge") {
        Say("", "CHORES: Feed Doggo. Trash out. Dishes. Stack the firewood. Clean your room. One present before dinner if ALL done by 16:40!");
        Say("You", "(Done: " + std::to_string(ChoresDone()) + " of 5.)" + std::string(earnedPresent ? "" : " (One present before dinner. So close.)"));
    } else if (id == "stove") {
        Say("You", Stage() >= 1 ? "(Ribbe. Again. It's always ribbe.)" : "(Ribbe in the oven. It smells like Christmas.)");
    } else if (id == "trashcan") {
        if (trashDone) { Say("You", "(Already took it out.)"); return; }
        if (hasBag) { Choice("The bag is dripping on your sock.", {{"it_back", "Put it back."}, {"none", "Keep it."}}); return; }
        Hold(Item::Bag);
        Say("You", "(You grab the trash bag. It's leaking. Bins are out in the yard.)");
    } else if (id == "bowl") {
        if (!DoggoExists()) {
            Say("You", "(An empty dog bowl. Whose is this? We don't have a dog. Do we?)");
            if (shift == Shift::NoDoggo) Achieve("Who's a Good Boy?");
            return;
        }
        if (fed) { Say("You", "(He's fed. He's acting like he isn't.)"); return; }
        if (!hasFood) { Say("You", "(Empty. The dog food is in the pantry, next to the fridge.)"); return; }
        FadeFor(Dur(3), "Feeding Doggo");
        fed = true; Hold(Item::None);
        Say("Doggo", "*inhales the entire bowl in two seconds*");
    } else if (id == "tv") {
        if (PowerOut()) { Say("You", "(No power. Just my own face in the glass.)"); return; }
        FadeFor(Dur(20), "Watching TV");
        if (Knows("handlers")) {
            Say("", "(Same channel. Different episode. A boy in a box. The audience laughs.)");
            Say("", "(The host looks straight at you. \"See you at ten to five, Subject 06!\")");
            if (!Knows("static_tv")) {
                Achieve("Previously On");
                AddClue("static_tv");
            }
        } else {
            Say("", "(The Kaptein Fjord Christmas special. You've seen it. Twice. Three times. Ten times.)");
        }
    } else if (id == "couch") {
        FadeFor(Dur(10), "Rotting on the couch");
        Say("You", "(Ten minutes of nothing. Worth it.)");
    } else if (id == "radio") {
        Say("", "(Radio) ...storm warning for Hardangervidda. The road closes at 16:50 and stays closed until the 27th. Do not travel tonight. God jul.");
    } else if (id == "woodstove") {
        Say("You", "(Warm. The only thing in this house that doesn't feel weird.)");
    } else if (id == "bins") {
        if (trashDone) { Say("You", "(Trash is out. Chore done.)"); return; }
        if (!hasBag) { Say("You", "(The bins. I need the bag from the kitchen first.)"); return; }
        FadeFor(Dur(4), "Taking out the trash");
        trashDone = true; Hold(Item::None);
        Say("You", "(Trash: out. Sock: ruined.)");
    } else if (id == "doghouse") {
        if (!DoggoExists()) {
            Say("You", "(A doghouse. Empty. Nobody remembers anyone living here. Nobody but me.)");
            if (shift == Shift::NoDoggo) Achieve("Who's a Good Boy?");
        } else {
            Say("You", "(Doggo's house. It smells like Doggo. Strongly.)");
        }
    } else if (id == "gate") {
        Say("You", "(The gravel road goes on forever. 31 km to anyone.)");
        if (Stage() >= 1) Say("You", "(I could run. He'd catch me. I could still run.)");
    } else if (id == "flag") {
        if (Knows("dad_key") && keyInDrawer) { Say("You", "(The little box is empty. The key is still in the lock of Dad's cabinet.)"); return; }
        if (Knows("dad_key")) {
            if (hasKey) { Choice("The little box at the bottom of the flagpole.", {{"it_back", "Put the key back."}, {"none", "Keep it."}}); return; }
            Choice("The little box at the bottom of the flagpole.", {{"fl_key", "Take the key."}, {"none", "Leave it."}});
            return;
        }
        flagTouches++;
        // The clean route. Deliberately obscure: third look, at exactly 16:44.
        if (flagTouches >= 3 && t >= 44 * 60 && t < 45 * 60 && Knows("dad_goodbye")) {
            Say("You", "(Wait. There's a tiny box bolted to the bottom of the pole. Painted the same colour.)");
            Say("You", "(A key. THE key.)");
            Hold(Item::Key);
            AddClue("dad_key");
            if (darkPoints < DARK_INTERROGATE) {
                Narrate("Hold on. You didn't figure that out. Nobody figures that out. Someone told you.");
                Narrate("Fine. Keep it. I'm watching you.");
                Achieve("Looked It Up");
            }
            return;
        }
        Say("You", "(The flag is snapping in the wind. Dad raised it this morning, in this weather. For who?)");
    } else if (id == "generator") {
        Say("You", "(The generator. Dad's been 'fixing' it for weeks.)");
    } else if (id == "bench") {
        if (dadInShed) {
            Say("Dad", "Hands off the bench, buddy. Sharp stuff.");
            return;
        }
        FadeFor(5, "Snooping on Dad's bench");
        Say("You", "(A radio under a rag. A sticker on the back:)");
        Say("", "PROPERTY OF THE PROGRAM. UNIT 06 HANDLER.");
        AddClue("dad_radio");
    }
    // ---- v0.6 objects
    else if (id == "pantry") {
        std::vector<ChoiceOpt> o;
        if (hasFood || hasLight) o.push_back({"it_back", hasFood ? "Put the dog food back." : "Put the flashlight back."});
        if (!hasFood && !fed) o.push_back({"pa_food", "Take the dog food."});
        if (!hasLight) o.push_back({"pa_light", "Take Dad's big flashlight."});
        o.push_back({"none", "Close it."});
        Choice("The pantry. Knekkebrød, brunost, dog food, a big flashlight.", o);
    } else if (id == "sink") {
        if (dishesDone) { Say("You", "(Dishes: done. My fingers are prunes.)"); return; }
        FadeFor(Dur(3), "Doing the dishes");
        dishesDone = true;
        Say("You", "(Ribbe fat. The worst kind of fat.)");
    } else if (id == "fliptendo") {
        std::vector<ChoiceOpt> o;
        o.push_back({"ft_flappy", "Play Flappy Plane. (10 min)"});
        if (zuldaOpened) o.push_back({"ft_zulda", "Play Zulda: Flute of Time. (10 min)"});
        o.push_back({"none", "Later."});
        if (Tutorial() && Once("ft_intro")) {
            Say("You", "(My Fliptendo Fold 2. Best thing I own. Only one game on it though.)");
            Say("You", "(Zulda: Flute of Time just got a remake. It's the only thing on my wish list.)");
        }
        Choice(zuldaOpened ? "Fliptendo Fold 2. ZULDA IS INSTALLED." : "Fliptendo Fold 2. Battery: 64%.", o);
    }
    // ---- v0.8 the tree
    else if (id == "tree") {
        std::vector<ChoiceOpt> o;
        if (earnedPresent && !zuldaOpened) o.push_back({"tr_zulda", "Open your present."});
        o.push_back({"tr_tags", "Read the tags."});
        o.push_back({"tr_shake", "Shake the big one."});
        if (!Tutorial() && !openedEarly) o.push_back({"tr_open", "Open one early."});
        o.push_back({"none", "Leave them."});
        Choice(zuldaOpened ? "The tree. Two presents left." : "The tree. Three presents with your name on them.", o);
    }
    // ---- v0.2 objects
    else if (id == "wardrobe") {
        Say("You", "(My clothes. Every label says S06. I always thought that was a size.)");
    } else if (id == "calendar") {
        Say("", "(The advent calendar. Door 24 is open. No chocolate. One word in Mom's handwriting: PICKUP.)");
        AddClue("calendar");
    } else if (id == "bookshelf") {
        Say("", "(\"Raising Exceptional Children.\" Mom's handwriting in the margins.)");
        Say("", "(\"dose\", \"drift\", \"week 400: stable\", \"he asked about the chair again\".)");
        AddClue("book_notes");
    } else if (id == "album") {
        Say("You", "(Me at 4. Me at 6. Me at 9 with a fish.)");
        Say("You", "(No baby photos. Not one. The album just... starts.)");
        AddClue("no_baby");
    } else if (id == "woodpile") {
        std::vector<ChoiceOpt> o;
        if (!firewoodDone) o.push_back({"wp_stack", "Stack the firewood. (20 min)"});
        if (hasStick) o.push_back({"it_back", "Drop the stick."});
        else o.push_back({"wp_stick", "Take a good stick."});
        o.push_back({"none", "Leave it."});
        Choice(tainted ? "The woodpile. An axe in the chopping block. Later, maybe." : "The woodpile. An axe in the chopping block. No. Not that.", o);
    } else if (id == "knives") {
        if (hasKnife) { Choice("The knife block.", {{"it_back", "Put the knife back."}, {"none", "Keep it."}}); return; }
        if (knifeTakenLoop) { Hold(Item::Knife); Say("You", "(You take it again. Easier the second time.)"); return; }
        knifeChecks = 0;
        KnifeCheck();
    } else if (id == "marbles") {
        if (darkPoints >= DARK_ARSENAL && !Tutorial()) {
            if (trapsSet.count("marbles")) { Say("You", "(The step is already covered.)"); return; }
            Choice("A jar of marbles.", {{"a_marbles", "Pour them on the iced front step. (5 min)", false, true}, {"none", "Leave them."}});
        } else Say("You", "(Marbles. I used to love these. I still kind of do.)");
    } else if (id == "rope") {
        if (darkPoints >= DARK_ARSENAL) {
            if (trapsSet.count("tripwire")) { Say("You", "(The tripwire's already up.)"); return; }
            Choice("A coil of rope.", {{"a_tripwire", "Tripwire across the porch. (10 min)", false, true}, {"none", "Leave it."}});
        } else Say("You", Knows("visitor_time") ? "(Rope. The man ties good knots. So does Dad.)" : "(Rope. Dad's good with knots.)");
    } else if (id == "toolbox") {
        if (darkPoints >= DARK_ARSENAL) {
            if (trapsSet.count("paint")) { Say("You", "(The paint can's already up there.)"); return; }
            Choice("Hammers, nails, a full can of paint.", {{"a_paint", "Rig the paint can over the front door. (10 min)", false, true}, {"none", "Leave it."}});
        } else Say("You", "(Hammers, nails, a can of paint. Dad's stuff.)");
    } else if (id == "frontdoor") {
        bool closed = closedDoors.count(20 * World::W + 20) > 0;
        if (frontLocked) Choice("The front door is locked.", {{"f_unlock", "Unlock it."}, {"none", "Leave it locked."}});
        else Choice("The front door.", {{closed ? "dr_open:frontdoor" : "dr_close:frontdoor", closed ? "Open it." : "Close it."}, {"f_lock", "Lock it."}, {"none", "Leave it."}});
    } else if (id == "sheddoor") {
        if (dadLocked) {
            Say("Dad", "(through the door) Please. Whatever you think is happening...");
            Choice("The plank is still in place.", {{"s_free", "Let him out."}, {"none", "Leave it."}});
        } else {
            int k = 10 * World::W + 34;
            bool closed = closedDoors.count(k) > 0, locked = lockedDoors.count(k) > 0;
            std::vector<ChoiceOpt> o;
            if (locked) { if (hasKey) o.push_back({"dr_unlock:sheddoor", "Unlock it."}); }
            else o.push_back({closed ? "dr_open:sheddoor" : "dr_close:sheddoor", closed ? "Open it." : "Close it."});
            if (!locked && hasKey) o.push_back({"dr_lockc:sheddoor", "Close it and lock it."});
            if (dadInShed && !Tutorial()) o.push_back({"s_bar", "Bar the door with the plank.", false, true});
            o.push_back({"none", "Leave it."});
            Choice(dadInShed ? "Dad's in the shed. There's a plank leaning by the door." : locked ? "The shed door. Locked." : "The shed door. There's a plank leaning next to it. And brackets. Why are there brackets?", o);
        }
    }
    // ---- basement
    else if (id == "stairs") {
        inBasement = false;
        FadeFor(1, "Back up the stairs", 27, 11);
    } else if (id == "screens") {
        Say("", "(Every room. Every angle. Your bed. The toilet. The yard. The shed.)");
        Say("", "(One small screen in the corner just shows a watch. Counting. " + ClockText() + ".)");
        AddClue("basement_screens");
    } else if (id == "files") {
        FadeFor(5, "Reading the files");
        Say("", "SUBJECT 06. Loop-aware candidate. Suppression: daily. Sessions: weekly. Handlers: 2.");
        Say("", "IF SUBJECT SHOWS VIOLENCE: COLLECT IMMEDIATELY.");
        Say("", "Clean-up of both handlers at collection. Handlers are NOT to be informed. Compliance is better when they believe they are retiring.");
        Say("You", "(They don't know. They think they get to leave after today.)");
        if (tainted) Say("", "(The page on top is new. Today's date. SUBJECT HAS SHOWN VIOLENCE. The ink is still wet.)");
        Say("", "(A letterhead: FINSE FACILITY. Under it, THE MACHINE, underlined three times.)");
        AddClue("basement_files");
        AddClue("program_hq");
        if (tainted) Narrate("They were right about me. They were right to be scared. I made sure of it.");
        else Narrate("If I never hurt anyone, they'd never have a reason. And they took me anyway.");
    } else if (id == "chair") {
        Say("", "(A chair with straps. Kid-sized. Worn smooth where wrists go.)");
        Say("You", "(My wrists itch. I don't know why. I do know why.)");
        AddClue("treatment");
    } else if (id == "console") {
        std::vector<ChoiceOpt> o = {{"r_listen", "Listen."}};
        if (Knows("phone_answer")) o.push_back({"r_cancel", "\"This is handler two. Cancel the pickup.\"", true});
        o.push_back({"none", "Leave it."});
        Choice("The program's radio. A red button: PICKUP CONFIRM.", o);
    } else if (id == "medfridge") {
        Say("", "(Little bottles in neat rows. S06. RESET. AFTER EACH SESSION. IN JUICE.)");
        Say("", "(Syringes on the shelf below. S06. WEEKLY. Somebody drew a smiley face on the label.)");
        Say("You", "(Juice at 15:55. Every day. I always get sleepy. I never remember yesterday. I thought that was just how days were.)");
        AddClue("juice");
    } else if (id == "vcr") {
        if (held == Item::Tape) { Choice("An old TV on a cart, facing the chair. A VCR underneath.", {{"vt_play", "Play the tape."}, {"none", "Not yet."}}); return; }
        Say("You", "(An old TV on a cart. It's turned to face the chair. The VCR under it is empty.)");
        if (Knows("the_tape")) Say("You", "(I don't need to see it again. I can still hear the humming.)");
    }
    // ---- NPCs
    else if (id == "mom" && momAtt == Attitude::Scared) {
        // v0.5: she remembers the knife. No small talk.
        if (Knows("mom_sorry_scared") || firedEvents.count("mom_sorry_scared")) {
            Say("Mom", "(she flinches when you come close)");
            Say("You", "(Leave her alone. I did enough.)");
        } else {
            Say("Mom", "(she keeps the kitchen table between you) ...What do you want?");
            Choice(MomName(), {{"m_sorry", "\"I'm sorry.\""}, {"none", "(Leave her alone.)"}});
        }
    } else if (id == "mom" && momAtt == Attitude::Cold) {
        if (grounded) Say("Mom", "(through the door) I'm not discussing it. You stay in there.");
        else Say("Mom", "(not looking up) Did I say you could come out of your room?");
        Choice(MomName(), {{"m_sorry", "\"Sorry, Mom.\""}, {"none", "(Go.)"}});
    } else if (id == "mom") {
        if (Knows("the_tape") && Once("tape_mom")) Say("You", "(She smiles at me. I can hear her humming. I smile back. I learned that from her.)");
        if (hasKnife && !tainted && Once("mom_sees_knife")) {
            Say("Mom", "Why are you holding that? Put. It. Down.");
            ChangeMood(-1);
        }
        if (tainted && Once("mom_distrust"))
            Say("Mom", "What's wrong with you lately? You look at us like we're strangers.");
        std::vector<ChoiceOpt> o = {
            {"m_dinner", "What's for dinner?"},
            {"m_allow", "Can I open a present?"},
        };
        if (!Tutorial()) o.push_back({"m_snack", "Can I have a snack? (5 min)"});
        if (Stage() >= 1) o.push_back({"m_born", "Where was I born?"});
        if (!Tutorial()) o.push_back({"m_back", "[Talk back] Chores are stupid."});
        if (Knows("no_car")) o.push_back({"m_car", "Why don't we have a car?", true});
        if (Knows("no_baby")) o.push_back({"m_baby", "Why are there no baby photos?", true});
        if (Knows("calendar")) o.push_back({"m_pickup", "What's the pickup?", true});
        if (Knows("loop")) o.push_back({"m_loop", "Mom, this day keeps repeating.", true});
        if (Knows("phone_mom") && t < 22 * 60) o.push_back({"m_phone", "Who's calling at 16:22?", true});
        if (Knows("handlers")) o.push_back({"m_real", "Are you even my mom?", true});
        if (hasKnife) o.push_back({"m_knife", "Show her the knife.", false, true});
        o.push_back({"none", "Never mind."});
        o.insert(o.begin() + 1, {"m_nisse", "Is Julenissen really coming?"});
        if (Mom().facingWall) {
            Say("Mom", "...");
            Say("You", "(She's just standing there. Facing the corner. Breathing.)");
            Say("Mom", "(she turns around, smiling) Oh! Hi, sweetie.");
        }
        Choice(MomName(), o);
    } else if (id == "dad" && dadAtt == Attitude::Broken) {
        Say("Dad", "(he won't look at you. He's holding his hand against his chest.)");
        Say("You", "(There's nothing left to say to him. Not this loop.)");
    } else if (id == "dad" && dadAtt == Attitude::Scared) {
        if (firedEvents.count("dad_sorry_scared")) {
            Say("Dad", "(he keeps his eyes on your hands) ...Go inside, buddy.");
        } else {
            Say("Dad", "(he takes a step back) Hey. Hey. Easy.");
            Choice("Dad", {{"d_sorry", "\"I'm sorry.\""}, {"none", "(Leave.)"}});
        }
    } else if (id == "dad" && dadAtt == Attitude::Cold) {
        Say("Dad", "(checking his watch) Buddy, go inside for a bit, okay? I've got some things to sort out.");
        std::vector<ChoiceOpt> o;
        if (Knows("dad_radio")) o.push_back({"d_report", "Who do you report to?", true});
        o.push_back({"none", "(Go.)"});
        Choice("Dad", o);
    } else if (id == "dad") {
        std::vector<ChoiceOpt> o = {{"d_what", "What are you doing?"}};
        if (dadInShed && !helpedDad) o.insert(o.begin(), {"d_help", "Need help? (15 min)"});
        if (!dadInShed && !Tutorial()) o.push_back({"d_cards", "Wanna play cards? (10 min)"});
        if (Knows("calendar")) o.push_back({"d_pickup", "What's the pickup?", true});
        if (Knows("loop")) o.push_back({"d_loop", "Dad, I'm stuck in a loop.", true});
        if (Knows("dad_radio")) o.push_back({"d_report", "Who do you report to?", true});
        if (hasKnife && dadAtShed && darkPoints >= DARK_INTERROGATE && !Knows("dad_key"))
            o.push_back({"d_talk", "Make him tell you where the basement key is.", false, true});
        else if (hasKnife) o.push_back({"d_knife", "Show him the knife.", false, true});
        o.push_back({"none", "Never mind."});
        if (tainted && Once("dad_distrust")) Say("Dad", "(he keeps the workbench between you) Hey, buddy. ...You okay?");
        if (Knows("the_tape") && Once("tape_dad")) Say("You", "(\"Subject is resisting.\" He sounded bored. He sounds bored now too.)");
        Choice("Dad", o);
    } else if (id == "doggo") {
        std::vector<ChoiceOpt> o = {{"g_pet", "Pet Doggo."}};
        if (doggoStay) o.push_back({"g_come", "\"Come on, boy!\" (stop staying)"});
        else o.push_back({"g_sit", Tutorial() ? "\"Sit!\"" : "\"Sit! Stay!\""});
        if (hasStick && world.RoomAt(px, py) == Room::Outside) o.push_back({"g_stick", "Throw the stick. (5 min)"});
        else if (!Tutorial()) o.push_back({"g_ball", "Play with his ball. (3 min)"});
        if (!Tutorial()) o.push_back({"g_search", "\"Find it, boy!\" (5 min)"});
        if (!Tutorial()) o.push_back({"g_tail", "Pull his tail."});
        if (hasKnife) o.push_back({"g_shed", "Take him out to the shed.", false, true});
        o.push_back({"none", "Leave him."});
        Choice("Doggo", o);
    } else if (id == "visitor") {
        if (DoggoExists()) {
            Say("Doggo", "*hides behind your legs. Shaking.*");
            AddClue("doggo_gate");
        }
        std::vector<ChoiceOpt> o = {{"v_who", "Who are you?"}};
        if (Stage() >= 2) o.push_back({"v_program", "You're from the program.", true});
        if (hasKnife) o.push_back({"v_stab", "Go for him. Now.", false, true});
        o.push_back({"none", "(Back away.)"});
        Choice("A big man in a black coat. Waiting by his car.", o);
    } else {
        Say("You", "(Huh.)");
    }
}

} // namespace sixhundred
