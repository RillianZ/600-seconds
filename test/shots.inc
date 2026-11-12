// 600 Seconds - test/shots.inc (v0.15, v0.17: traps N, knife, lockparents)
// A tiny script runner for screenshots on a virtual screen (Xvfb). Only built with -DSHOTS, never in a release.
//   SHOTS=script.txt ./600s
// One command per line:
//   start               new game, straight into a loop (the tutorial is skipped by "loop")
//   loop N              pretend this is loop N
//   time M S            set the clock to 16:MM:SS
//   at X Y [DIR]        put the kid on tile X, Y (DIR 0 down 1 up 2 left 3 right)
//   coat | light        put the jacket on | hold Dad's flashlight
//   mem ID              know a memory
//   dark N              set the darkness (6+ is tainted)
//   knock               jump to the knock (the hunt starts once the beats are played)
//   hunt                skip the knock and start the hunt right now
//   mash N              tap E every other frame for N frames (talk through a scene)
//   pick ID             choose this option in the open choice box
//   use ID              interact with ID, CHOOSE ID picks a choice
//   skip                drop whatever text is on screen
//   hold KEY N          hold a key (W A S D E Q T SHIFT) for N frames, PRESS KEY taps it
//   visitor X Y [DIR]   put him somewhere (in the hunt)
//   hide ID             hide in a spot
//   wait N              let N frames pass
//   shot FILE           save a screenshot next to the exe
//   traps N             rig the first N traps (marbles, tripwire, paint) on the front step
//   knife | lockparents hold the knife | Mom bolted in the basement and Dad barred in the shed
//   chores              all five chores done (v0.18)
//   turn mom|dad|both   that parent is on your side (v0.18)
//   darkact             something dark this loop, so they meet him in the Dark stance (v0.18)
//   chase               he sees you right now (v0.18)
//   bar DOOR            furniture against that door (v0.19)
//   itrap NAME          an inside trap: marbles (your door), tripwire (living room door), paint (shed door)
//   carkey WHERE        his car key is: pocket, snow or woodpile
//   tracks              your fake trail out the back held (set before "hunt")
//   goto X Y            send him to look at a tile (he walks there, kicking doors on the way)
//   blind               he can't see you (to watch him from close by)
//   sound LABEL         print what the music and ambience are doing, and every sound id played since the last one (v0.20 sound)
//   title | recap       go to the title screen | open the memory recap (v0.20 sound)
//   jukebox             open the debug jukebox (v0.20 sound)
//   ending ID           show that ending card now (v0.20 sound)
//   quit
#include <fstream>
#include <sstream>
static std::vector<std::string> shotCmds;
static size_t shotI = 0;
static int shotWait = 0;
static int shotHoldKey = 0, shotHoldN = 0, shotMash = 0;
static bool shotsOn = false;
static int ShotKey(const std::string& k) {
    if (k == "W") return KEY_W; if (k == "A") return KEY_A; if (k == "S") return KEY_S; if (k == "D") return KEY_D;
    if (k == "E") return KEY_E; if (k == "Q") return KEY_Q; if (k == "T") return KEY_T; if (k == "SHIFT") return KEY_LEFT_SHIFT;
    if (k == "ENTER") return KEY_ENTER; if (k == "DOWN") return KEY_DOWN; if (k == "UP") return KEY_UP;
    return 0;
}
static void ShotsInit() {
    const char* f = getenv("SHOTS");
    if (!f) return;
    std::ifstream in(f);
    std::string line;
    while (std::getline(in, line)) if (!line.empty() && line[0] != '#') shotCmds.push_back(line);
    shotsOn = !shotCmds.empty();
    if (shotsOn) { screen = Screen::Title; splashT = 99; }
}
static void ShotsStep() {
    if (!shotsOn) return;
    if (shotHoldN > 0) { vDown[shotHoldKey] = true; if (!vPrevDown[shotHoldKey]) vPress[shotHoldKey] = true; --shotHoldN; }
    if (shotMash > 0) { --shotMash; if (shotMash % 2 == 0) { vDown[KEY_E] = true; vPress[KEY_E] = true; } return; }
    if (shotWait > 0) { --shotWait; return; }
    while (shotI < shotCmds.size()) {
        std::istringstream ss(shotCmds[shotI++]);
        std::string c; ss >> c;
        int px = TX(player.pos.x), py = TX(player.pos.y);
        if (c == "start") { StartGame(true); story.beats.clear(); beatActive = false; }
        else if (c == "loop") { ss >> story.loop; }
        else if (c == "time") { int m = 0, s = 0; ss >> m >> s; story.t = m * 60.0f + s; }
        else if (c == "at") { int x, y, d = 0; ss >> x >> y >> d; Snap(player, x, y); player.dir = d; }
        else if (c == "coat") story.hasCoat = true;
        else if (c == "light") story.Hold(Item::Flashlight, true);
        else if (c == "mem") { std::string id; ss >> id; story.memory.insert(id); }
        else if (c == "dark") { ss >> story.darkPoints; story.tainted = story.darkPoints >= DARK_TAINT; }
        else if (c == "knock") story.AdvanceMinutes(std::max(1, story.visitorTime / 60 - (int)(story.t / 60)), px, py);
        else if (c == "use") { std::string id; ss >> id; story.Interact(id, px, py); if (!beatActive) StartNextBeat(); }
        else if (c == "choose") { std::string id; ss >> id; beatActive = false; story.Choose(id, px, py); StartNextBeat(); }
        else if (c == "hunt") {
            story.beats.clear(); beatActive = false;
            story.t = (float)story.visitorTime + 5; story.collection = story.chase = true;
            NpcState v = story.Visitor(); Snap(visitor, v.x, v.y); visitor.visible = true; wasCollection = true;
            scene = SceneMode::Chase; sceneT = 0; StartSearch(false);
        }
        else if (c == "mash") { ss >> shotMash; return; }
        else if (c == "pick") {
            std::string id; ss >> id;
            if (beatActive && cur.type == BeatType::Choice) { beatActive = false; story.Choose(id, px, py); StartNextBeat(); }
        }
        else if (c == "vmode") { std::string m; ss >> m; if (m == "search") { vmode = VMode::Search; vPause = 0; } else if (m == "lost") { vmode = VMode::Lost; vPause = 30; } }
        else if (c == "debug") debugMode = true;
        else if (c == "jukes") ss >> vJukes;
        else if (c == "doggo") { int x, y; ss >> x >> y; Snap(doggo, x, y); }
        else if (c == "traps") { int n; ss >> n; const char* t[3] = {"marbles", "tripwire", "paint"}; for (int i = 0; i < n && i < 3; ++i) story.trapsSet.insert(t[i]); story.traps = n; }   // v0.17
        else if (c == "knife") story.Hold(Item::Knife, true);
        else if (c == "lockparents") { story.momLocked = story.dadLocked = true; }
        else if (c == "nobodyleft") { story.momLocked = story.dadLocked = true; story.doggoOut = true; }
        else if (c == "chores") { story.fed = story.trashDone = story.roomClean = story.dishesDone = story.firewoodDone = true; }   // v0.18
        else if (c == "turn") { std::string w; ss >> w; if (w != "dad") story.momTurned = true; if (w != "mom") story.dadTurned = true; story.parentsAllied = story.momTurned && story.dadTurned; }
        else if (c == "darkact") { story.darkThisLoop = 1; }
        else if (c == "bar") { std::string id; ss >> id; if (auto d = DoorById(id)) { int k = d->y * World::W + d->x; story.barricades.insert(k); story.closedDoors.insert(k); } }   // v0.19
        else if (c == "itrap") { std::string t; ss >> t; story.innerTraps.insert(t); }
        else if (c == "carkey") { ss >> story.carKey; }
        else if (c == "tracks") { story.falseTracks = true; }
        else if (c == "goto") { int x, y; ss >> x >> y; vmode = VMode::Investigate; vNoise = TileCenter(x, y); vNoiseWait = 1.5f; vNoiseT = -30; vPause = 0; vHunchSearch = true; }
        else if (c == "blind") { visitorBlind = true; }
        else if (c == "chase") { vmode = VMode::Chase; vLostT = 0; vLastSeen = player.pos; }
        else if (c == "skip") { story.beats.clear(); beatActive = false; }
        else if (c == "hold" || c == "press") { std::string k; int n = 1; ss >> k >> n; shotHoldKey = ShotKey(k); shotHoldN = c == "press" ? 1 : n; return; }
        else if (c == "visitor") { int x, y, d = 0; ss >> x >> y >> d; Snap(visitor, x, y); visitor.dir = d; }
        else if (c == "hide") {
            std::string id; ss >> id;
            for (auto& o : story.world.objs) if (o.id == id) { hidden = true; hideT = 0; story.hideSpot = id; hidePos = TileCenter(o.x, o.y); }
        }
        else if (c == "wait") { ss >> shotWait; return; }
        else if (c == "shot") { std::string f; ss >> f; TakeScreenshot(f.c_str()); }
        // v0.20 sound: nobody can listen on the build machine, so the runner prints what would be heard
        else if (c == "sound") {
            std::string label; std::getline(ss, label);
            std::string log; for (auto& e : sfxLog) log += e + " "; sfxLog.clear();
            std::printf("SOUND%s | music: %s (fade %.1fs, %s) | mom %.2f muff %.2f | dad %.2f muff %.2f | music muff %.2f | hum %.2f wind %.2f vhs %.0f | DadHums %d MomHums %d | sfx: %s\n",
                label.c_str(), TrackName(musicCur), (float)musicFadeSec, TrackFromFile(musicCur) ? "file" : "code", (float)aMomT, (float)aMomMuffT, (float)aDadT, (float)aDadMuffT,
                (float)aMusicMuffT, (float)aHumT, (float)aWindT, (float)aVhsT, story.DadHums() ? 1 : 0, story.MomHums() ? 1 : 0, log.c_str());
            std::printf("      kid %d,%d  dad %d,%d visible %d  mom %d,%d\n", TX(player.pos.x), TX(player.pos.y), TX(dad.pos.x), TX(dad.pos.y), dad.visible ? 1 : 0, TX(mom.pos.x), TX(mom.pos.y));
            std::fflush(stdout);
        }
        else if (c == "jukebox") { debugMode = true; jukeboxOpen = true; jukeboxSel = musicCur; }
        else if (c == "title") { screen = Screen::Title; story.beats.clear(); beatActive = false; }
        else if (c == "recap") { recapList = { "phone_mom" }; recapOpen = true; recapT = 0; recapShown = 0; }
        else if (c == "ending") { recapOpen = false; std::string id; ss >> id; Beat e; e.type = BeatType::Ending; e.id = id; e.speaker = "ENDING"; e.text = "TEST\n\n" + id; story.beats.clear(); story.beats.push_back(e); beatActive = false; StartNextBeat(); }
        else if (c == "quit") { CloseWindow(); exit(0); }
    }
}
