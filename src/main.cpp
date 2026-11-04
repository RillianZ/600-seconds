// 600 Seconds - raylib presentation layer (prototype)
// Sund Origin Studio. All game rules live in src/game (engine-free).
// This file only draws, plays sounds and reads input.
#include "raylib.h"
#include "game/Story.h"
#include "audio/Soundtrack.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <queue>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

using namespace sixhundred;

// ============================================================ phone mode input layer (v0.4.2)
// Touch controls feed "virtual keys", so the rest of the game just keeps asking for keys.
static bool touchMode = false;        // turns on with the first touch (or a touch-first device)
static bool vDown[512], vPrevDown[512], vPress[512];
static float newGameArm = 0;          // phone: NEW GAME needs two taps
static bool visitorBlind = false;     // debug panel
static inline bool KeyDownV(int k) { return IsKeyDown(k) || (k >= 0 && k < 512 && vDown[k]); }
static inline bool KeyPressedV(int k) { return IsKeyPressed(k) || (k >= 0 && k < 512 && vPress[k]); }
// raylib turns every tap into a left click. In phone mode the touch layer decides what a tap means instead.
static inline bool MousePressedV(int b) { return !touchMode && IsMouseButtonPressed(b); }
#define IsKeyDown KeyDownV
#define IsKeyPressed KeyPressedV
#define IsMouseButtonPressed MousePressedV

// ============================================================ config
static const int VW = 480, VH = 270;   // virtual world resolution
static const int TS = 16;               // tile size
static int SW = 960, SH = 540;          // screen
static float US = 2.0f;                 // UI scale (screen px per virtual px)

// ============================================================ state
enum class Screen { Splash, Title, Game };
static Screen screen = Screen::Splash;
static float splashT = 0;
static Texture2D logoTex{};
static bool hasLogo = false;
static Story story;
static RenderTexture2D target;
static Font narratorFont;
static Font uiFont;                  // v0.4: soft UI font (Poppins, OFL)
static bool hasUiFont = false;
static Camera2D worldCam{};
static RenderTexture2D lightRT;      // v0.5: light map, multiplied over the world
static bool kidRunning = false;      // v0.5: for dust puffs          // last world camera, for bubbles drawn in UI space
static void DrawAIBadge(float x, float y, Color fg);
static bool hasNarratorFont = false;

struct Actor {
    Vector2 pos{};      // pixel center
    int dir = 0;        // 0 down 1 up 2 left 3 right
    float anim = 0;
    bool visible = true;
};
static Actor player, mom, dad, doggo, visitor;

// Beat playback
static bool beatActive = false;
static Beat cur;
static float beatTime = 0;
static float typed = 0;      // chars shown
static int choiceSel = 0;
static bool fadeApplied = false;
static std::string fadeFrom, fadeTo;
static bool curSeen = false;   // v0.9: the player has read this line before
static std::string talkingTo;   // v0.5: who you are talking to stops walking
// v0.6: end-of-loop recap, journal scroll, the Fliptendo mini-game
static bool recapOpen = false; static float recapT = 0; static int recapShown = 0; static std::vector<std::string> recapList;
static int journalScroll = 0;
static bool miniOpen = false; static float miniY = 0, miniV = 0, miniT = 0, miniDeadT = 0; static int miniScore = 0; static bool miniDead = false, miniStarted = false;
struct MiniPipe { float x, gapY; bool scored; };
static std::vector<MiniPipe> miniPipes;
static std::string lastTalkId; static float lastTalkT = 0;   // v0.6: no instant re-talk after a conversation
static float fadeFromT = 0, fadeToT = 0;   // v0.5: the watch rolls between these
// v0.7
static int gMover = 0;                 // who is pathfinding: 0 grown-up, 1 the visitor, 2 Doggo (can't open doors)
static float vDoorT = 0;               // the visitor kicking in a locked door
static int vDoorX = -1, vDoorY = -1;
static int lastThreats = 0;
static float doggoDeadT = 0, momSobT = 0, struggleT = 0;
static bool prevDoggoDead = false;
static bool doggoShownDead = false;   // the body only shows after the fade to black
struct HeldDoor { int x = -1, y = -1; bool locked = false; };
static HeldDoor momDoor, dadDoor;      // grown-ups close doors behind them
struct Print { Vector2 p; float t; int who; int side; };   // who: 0 kid, 1 visitor, 2 Doggo
static std::vector<Print> prints;

struct Toast { std::string title, text; float t; Color c; };
static std::vector<Toast> toasts;

static bool journalOpen = false;
static int journalTopic = 0;
static float flashT = 0;       // white flash on loop end
static float globalT = 0;
static float barkT = 0;        // instability barking
static std::string barkText;
static bool hasSave = false;
static bool debugMode = false;
static const char* kVersion = "v0.9";
static float hardResetArm = 0;     // debug: press Shift+R twice to wipe everything   // ?debug in the URL: 8 = +5 min, 9 = jump to 16:49
static float interactCooldown = 0; // stops a confirm key from also re-triggering the thing you just used

// v0.2: real-time collection
enum class SceneMode { None, DadRuns, Chase };
static SceneMode scene = SceneMode::None;
static float sceneT = 0;
static bool hidden = false;
static float hideT = 0;
static Vector2 hidePos{};
static float downedT = 0;
static bool wasCollection = false;
static float qteBar = 0;

// v0.4: speech bubbles over heads (announcements, barks, the visitor muttering)
struct SpeechBubble { std::string who, text; float t, life; };
static std::vector<SpeechBubble> bubbles;

// v0.4: the visitor searches the house instead of homing in on you
enum class VMode { Grace, Search, Chase, Lost, CheckSpot, GoToHide };
static VMode vmode = VMode::Grace;
static float vPause = 0, vLostT = 0, vBarkT = 0, vSpotCooldown = 0;
static std::vector<Vector2> vPath;     // waypoints in the current room
static size_t vPathI = 0;
static std::vector<int> vSpots;        // hiding spots (obj index) still to check in this room
static int vRoom = -1, vSweep = 0, vSpotObj = -1;
static bool vRoomDone[16] = {};
static Vector2 vLastSeen{};
static Vector2 vGraceGoal{};
static float vGraceWait = 0;

// debug jukebox (M with ?debug)
static bool jukeboxOpen = false;
static int jukeboxSel = 1;

// Sounds
static Sound sBeep, sKnock, sBang, sRing, sThud, sBark, sGlass, sCrack, sChime, sFlap, sPoint, sDoor, sBells;
static bool audioOk = false;
// v0.9: settings, saved apart from the game
struct Settings { int music = 7, sfx = 10, text = 1; bool calmFlashes = false; };
static Settings settings;
static bool pauseOpen = false;
static int pauseSel = 0, pauseTapRow = -1, pauseTapDir = 0;

// ============================================================ persistence
static void SaveGame() {
#if defined(PLATFORM_WEB)
    std::string s = story.SaveString();
    EM_ASM({ try { localStorage.setItem('600s_save', UTF8ToString($0)); } catch (e) {} }, s.c_str());
#else
    SaveFileText("600s_save.txt", (char*)story.SaveString().c_str());
#endif
}

static std::string LoadSaveString() {
#if defined(PLATFORM_WEB)
    char* p = (char*)EM_ASM_PTR({
        var s = null;
        try { s = localStorage.getItem('600s_save'); } catch (e) {}
        if (!s) s = "";
        var len = lengthBytesUTF8(s) + 1;
        var buf = _malloc(len);
        stringToUTF8(s, buf, len);
        return buf;
    });
    std::string out = p ? p : "";
    free(p);
    return out;
#else
    if (!FileExists("600s_save.txt")) return "";
    char* t = LoadFileText("600s_save.txt");
    std::string out = t ? t : "";
    UnloadFileText(t);
    return out;
#endif
}

// ============================================================ audio synth
static const int SFX_SR = 44100;
static Sound MakeSound(int frames, float (*f)(float t, int i)) {
    Wave w{};
    w.frameCount = frames;
    w.sampleRate = SFX_SR;
    w.sampleSize = 16;
    w.channels = 1;
    short* d = (short*)MemAlloc(frames * sizeof(short));
    for (int i = 0; i < frames; ++i) {
        float v = f(i / (float)SFX_SR, i / 2);
        // 3 ms in, 10 ms out: no clicks at the edges
        float fin = std::min(1.0f, i / (SFX_SR * 0.003f));
        float fout = std::min(1.0f, (frames - 1 - i) / (SFX_SR * 0.010f));
        v *= fin * fout;
        v = std::tanh(v * 1.2f) / std::tanh(1.2f);   // soft clip instead of hard clip
        d[i] = (short)(v * 28000);
    }
    w.data = d;
    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    return s;
}
static float Noise(int i) { unsigned x = i * 2654435761u; x ^= x >> 13; x *= 1274126177u; return ((x & 0xffff) / 32768.0f) - 1.0f; }

static void InitSounds() {
    InitAudioDevice();
    audioOk = IsAudioDeviceReady();
    if (!audioOk) return;
    sBeep = MakeSound(SFX_SR * 0.32f, [](float t, int) {
        // Watch beep: two short blips, band-limited so it doesn't fizz
        auto gate = [](float t, float a, float b) { return std::max(0.0f, std::min(1.0f, std::min((t - a) / 0.003f, (b - t) / 0.003f))); };
        float g = gate(t, 0.0f, 0.09f) + gate(t, 0.16f, 0.25f);
        float ph = 2 * PI * 2000.0f * t;
        return g * 0.22f * (std::sin(ph) + std::sin(3 * ph) / 3 + std::sin(5 * ph) / 5);
    });
    sKnock = MakeSound(SFX_SR * 1.3f, [](float t, int i) {
        float v = 0;
        for (int k = 0; k < 3; ++k) {
            float s = t - k * 0.42f;
            if (s > 0 && s < 0.12f) v += (std::sin(s * 2 * PI * 90) * 0.9f + Noise(i) * 0.3f) * std::exp(-s * 40);
        }
        return v;
    });
    sBang = MakeSound(SFX_SR * 0.9f, [](float t, int i) {
        return (Noise(i) * 0.9f + std::sin(t * 2 * PI * 60) * 0.5f) * std::exp(-t * 7);
    });
    sRing = MakeSound(SFX_SR * 1.6f, [](float t, int) {
        float c = std::fmod(t, 0.8f);
        float env = 0.3f * std::max(0.0f, std::min(1.0f, std::min(c / 0.01f, (0.5f - c) / 0.02f)));
        return env * (std::sin(t * 2 * PI * 880) + std::sin(t * 2 * PI * 1046)) * 0.5f * (0.7f + 0.3f * std::sin(t * 2 * PI * 25));
    });
    sThud = MakeSound(SFX_SR * 0.7f, [](float t, int i) {
        return (std::sin(t * 2 * PI * 55) + Noise(i) * 0.2f) * std::exp(-t * 9);
    });
    sGlass = MakeSound(SFX_SR * 1.0f, [](float t, int i) {
        float v = Noise(i) * std::exp(-t * 5) * 0.8f;
        for (int k = 0; k < 6; ++k) { float s = t - k * 0.09f; if (s > 0) v += std::sin(s * 2 * PI * (2400 + k * 700)) * std::exp(-s * 30) * 0.25f; }
        return v;
    });
    sCrack = MakeSound(SFX_SR * 0.3f, [](float t, int i) {
        return (Noise(i * 3) * 0.9f) * std::exp(-t * 35) + std::sin(t * 2 * PI * 140) * std::exp(-t * 20) * 0.5f;
    });
    sChime = MakeSound(SFX_SR * 1.4f, [](float t, int) {
        // v0.6: memory recap, a soft bell like a ship log entry appearing
        float v = 0;
        const float f[3] = { 659.25f, 987.77f, 1318.5f };
        for (int k = 0; k < 3; ++k) { float s = t - k * 0.07f; if (s > 0) v += std::sin(s * 2 * PI * f[k]) * std::exp(-s * 3.2f) * (0.22f - k * 0.05f); }
        return v + std::sin(t * 2 * PI * 329.6f) * std::exp(-t * 2.0f) * 0.12f;
    });
    sDoor = MakeSound(SFX_SR * 0.35f, [](float t, int i) {
        // v0.7: a wooden door, a little creak and a soft bump
        float creak = (t < 0.22f) ? std::sin(t * 2 * PI * (180 + 60 * std::sin(t * 70))) * 0.08f * (1 - t / 0.22f) : 0;
        float s = t - 0.22f;
        float bump = s > 0 ? (std::sin(s * 2 * PI * 85) * 0.6f + Noise(i) * 0.15f) * std::exp(-s * 30) : 0;
        return creak + bump;
    });
    sFlap = MakeSound(SFX_SR * 0.08f, [](float t, int) { return std::sin(t * 2 * PI * (500 + t * 4000)) * 0.25f * std::exp(-t * 30); });
    sPoint = MakeSound(SFX_SR * 0.15f, [](float t, int) { return (t < 0.06f ? std::sin(t * 2 * PI * 1046) : std::sin(t * 2 * PI * 1568)) * 0.2f; });
    // v0.8: Christmas being rung in, far away. Two church bells, soft and with long tails
    sBells = MakeSound(SFX_SR * 7.0f, [](float t, int) {
        auto bell = [](float age, float f) {
            if (age < 0) return 0.0f;
            float v = 0;
            static const float part[5] = { 0.5f, 1.0f, 1.19f, 1.5f, 2.0f }, amp[5] = { 0.5f, 1.0f, 0.5f, 0.3f, 0.2f };
            for (int k = 0; k < 5; ++k) v += amp[k] * std::sin(age * 2 * PI * f * part[k]) * std::exp(-age * (0.7f + k * 0.5f));
            return v * std::min(1.0f, age / 0.01f);
        };
        float v = 0;
        for (int k = 0; k < 4; ++k) v += bell(t - k * 1.6f, 262.0f) + bell(t - 0.8f - k * 1.6f, 220.0f);
        return v * 0.05f * std::min(1.0f, (7.0f - t) / 1.0f);
    });
    sBark = MakeSound(SFX_SR * 0.25f, [](float t, int i) {
        float f = 300 + 400 * std::exp(-t * 20);
        return (std::sin(t * 2 * PI * f) * 0.6f + Noise(i) * 0.25f) * std::exp(-t * 14) * (t < 0.2f ? 1 : 0);
    });
}

static bool hushMusic = false;   // v0.9: the story asks for silence
static void Play(const std::string& id) {
    if (id == "hush_on") { hushMusic = true; return; }
    if (id == "hush_off") { hushMusic = false; return; }
    if (!audioOk) return;
    auto P = [&](Sound& s) { SetSoundVolume(s, settings.sfx / 10.0f * (id == "bark" ? 0.5f : 1.0f)); PlaySound(s); };
    if (id == "beep") P(sBeep);
    else if (id == "knock") P(sKnock);
    else if (id == "bang") P(sBang);
    else if (id == "ring") P(sRing);
    else if (id == "thud") P(sThud);
    else if (id == "bark") P(sBark);
    else if (id == "glass") P(sGlass);
    else if (id == "crack") P(sCrack);
    else if (id == "chime") P(sChime);
    else if (id == "flap") P(sFlap);
    else if (id == "point") P(sPoint);
    else if (id == "door") P(sDoor);
    else if (id == "bells") P(sBells);
}
// v0.9: only Doggo lines that are actually loud get the bark
static bool IsBark(const std::string& t) {
    return t.find("ark") != std::string::npos || t.find("YELP") != std::string::npos || t.find("growl") != std::string::npos;
}

// ============================================================ music
// Procedural soundtrack, one mood per act. Pure functions of time, so it
// never repeats a buffer and costs no assets.
//   0 silence, 1 cute (EarthBound-ish), 2 eerie music box, 3 hopeless drone, 4 the collection
static AudioStream musicStream{};
static bool musicOk = false;
static volatile int musicCur = 0, musicPrev = 0;
static float musicFade = 1.0f;     // 0..1 crossfade progress
static double musicT = 0;
static const int MUSIC_RATE = 44100;
static std::vector<float> echoBuf;
static int echoPos = 0;
static float musicLP = 0;

// v0.4: the audio device asks for samples when it needs them (no more topping
// up once per frame, which could run dry in the browser and crackle)
static void MusicCallback(void* buffer, unsigned int frames) {
    float* out = (float*)buffer;
    const float lpA = 0.6f;   // gentle low-pass (~6.5 kHz), takes the edge off
    for (unsigned int i = 0; i < frames; ++i) {
        musicT += 1.0 / MUSIC_RATE;
        if (musicFade < 1) musicFade = std::min(1.0f, musicFade + 1.0f / (MUSIC_RATE * 2.5f));
        int curT = musicCur, prevT = musicPrev;
        float v = TrackSample(curT, musicT) * musicFade;
        if (musicFade < 1) v += TrackSample(prevT, musicT) * (1 - musicFade);
        // echo: more of it the darker it gets
        float wet = TrackEcho(curT);
        float e = echoBuf[echoPos];
        echoBuf[echoPos] = v + e * wet * 0.85f;
        echoPos = (echoPos + 1) % (int)echoBuf.size();
        v += e * wet;
        musicLP += (v - musicLP) * lpA;
        out[i] = std::tanh(musicLP * 1.1f) * 0.85f;
    }
}

static void InitMusic() {
    if (!audioOk) return;
    SetAudioStreamBufferSizeDefault(4096);
    musicStream = LoadAudioStream(MUSIC_RATE, 32, 1);
    musicOk = IsAudioStreamValid(musicStream);
    if (!musicOk) return;
    echoBuf.assign((size_t)(MUSIC_RATE * 0.36f), 0.0f);
    SetAudioStreamCallback(musicStream, MusicCallback);
    SetAudioStreamVolume(musicStream, settings.music / 10.0f);
    PlayAudioStream(musicStream);
}

static void SetMusic(int track) {
    if (track == musicCur) return;
    musicPrev = musicCur;
    musicFade = 0;
    musicCur = track;
}

static void UpdateMusic() { /* the callback does the work now */ }

// ============================================================ helpers
static Vector2 TileCenter(int x, int y) { return { x * TS + TS / 2.0f, y * TS + TS / 2.0f }; }
static int TX(float px) { return (int)std::floor(px / TS); }

static bool PlayerWalkable(int x, int y) {
    if (story.DoorClosed(x, y)) return false;
    return story.world.Walkable(x, y, false);
}

static bool CanStand(float cx, float cy) {
    const float h = 5;
    float xs[2] = { cx - h, cx + h }, ys[2] = { cy - h + 3, cy + h };
    for (float x : xs) for (float y : ys) if (!PlayerWalkable(TX(x), TX(y))) return false;
    return true;
}

// BFS one step toward a target tile
static Vector2 NextStep(Vector2 from, int tx, int ty, bool basement) {
    int fx = TX(from.x), fy = TX(from.y);
    if (fx == tx && fy == ty) return TileCenter(tx, ty);
    static int dist[World::H][World::W];
    for (auto& row : dist) for (int& d : row) d = -1;
    std::queue<std::pair<int, int>> q;
    if (!story.world.InBounds(tx, ty)) return from;
    dist[ty][tx] = 0; q.push({ tx, ty });
    const int dx[4] = { 1, -1, 0, 0 }, dy[4] = { 0, 0, 1, -1 };
    while (!q.empty()) {
        auto [x, y] = q.front(); q.pop();
        if (x == fx && y == fy) break;
        for (int k = 0; k < 4; ++k) {
            int nx = x + dx[k], ny = y + dy[k];
            if (!story.world.InBounds(nx, ny) || dist[ny][nx] >= 0) continue;
            bool ok = story.world.Walkable(nx, ny, basement) || (nx == fx && ny == fy);
            if (ok && gMover == 2 && story.DoorClosed(nx, ny) && !(nx == fx && ny == fy)) ok = false;   // Doggo can't open doors
            if (!ok) continue;
            dist[ny][nx] = dist[y][x] + 1;
            q.push({ nx, ny });
        }
    }
    int best = -1, bx = fx, by = fy;
    for (int k = 0; k < 4; ++k) {
        int nx = fx + dx[k], ny = fy + dy[k];
        if (!story.world.InBounds(nx, ny)) continue;
        int d = dist[ny][nx];
        if (d >= 0 && (best < 0 || d < best)) { best = d; bx = nx; by = ny; }
    }
    return TileCenter(bx, by);
}

static void MoveActor(Actor& a, int tx, int ty, float speed, float dt, bool basement, float stopDist = 0) {
    Vector2 goal = TileCenter(tx, ty);
    float gd = std::hypot(goal.x - a.pos.x, goal.y - a.pos.y);
    if (gd <= stopDist + 0.5f) { a.anim = 0; return; }
    if (gd > TS * 12) { /* too far, just snap when offscreen-ish */ }
    Vector2 step = NextStep(a.pos, tx, ty, basement);
    // finish centering on the current tile before turning a corner
    Vector2 c = TileCenter(TX(a.pos.x), TX(a.pos.y));
    Vector2 aim = step;
    bool offX = std::fabs(a.pos.x - c.x) > 1, offY = std::fabs(a.pos.y - c.y) > 1;
    bool stepX = std::fabs(step.x - c.x) > 0.1f, stepY = std::fabs(step.y - c.y) > 0.1f;
    if ((offX && !stepX) || (offY && !stepY)) aim = c;
    float dx = aim.x - a.pos.x, dy = aim.y - a.pos.y;
    float d = std::hypot(dx, dy);
    if (d < 0.01f) { a.pos = aim; return; }
    float m = std::min(d, speed * dt);
    a.pos.x += dx / d * m; a.pos.y += dy / d * m;
    if (std::fabs(dx) > std::fabs(dy)) a.dir = dx > 0 ? 3 : 2; else a.dir = dy > 0 ? 0 : 1;
    a.anim += dt * 8;
}

static void Snap(Actor& a, int tx, int ty) { a.pos = TileCenter(tx, ty); }

// ============================================================ text helpers (UI space)
static void UIText(const char* t, float x, float y, float size, Color c) {
    if (hasUiFont) { DrawTextEx(uiFont, t, { x * US, (y - size * 0.12f) * US }, size * US * 1.02f, size * US * 0.02f, c); return; }
    DrawTextEx(GetFontDefault(), t, { x * US, y * US }, size * US, size * US / 10.0f, c);
}
static float UIMeasure(const char* t, float size) {
    if (hasUiFont) return MeasureTextEx(uiFont, t, size * US * 1.02f, size * US * 0.02f).x / US;
    return MeasureTextEx(GetFontDefault(), t, size * US, size * US / 10.0f).x / US;
}
static std::vector<std::string> Wrap(const std::string& s, float maxW, float size, bool narrator) {
    std::vector<std::string> lines;
    std::string line, word;
    auto width = [&](const std::string& t) {
        if (narrator && hasNarratorFont) return MeasureTextEx(narratorFont, t.c_str(), size * US, 1).x / US;
        return UIMeasure(t.c_str(), size);
    };
    auto flush = [&]() {
        if (word.empty()) return;
        std::string test = line.empty() ? word : line + " " + word;
        if (width(test) > maxW && !line.empty()) { lines.push_back(line); line = word; }
        else line = test;
        word.clear();
    };
    for (char ch : s) {
        if (ch == ' ') flush();
        else if (ch == '\n') { flush(); lines.push_back(line); line.clear(); }
        else word += ch;
    }
    flush();
    if (!line.empty()) lines.push_back(line);
    return lines;
}

// ============================================================ bubbles
static Actor* ActorFor(const std::string& who) {
    if (who == "Mom") return &mom;
    if (who == "Dad") return &dad;
    if (who == "Doggo") return &doggo;
    if (who == "You") return &player;
    if (who == "The visitor") return &visitor;
    return nullptr;
}
static void AddBubble(const std::string& who, const std::string& text, float life = -1) {
    for (size_t i = 0; i < bubbles.size(); ++i) if (bubbles[i].who == who) { bubbles.erase(bubbles.begin() + i); break; }
    if (life < 0) life = 2.4f + text.size() * 0.05f;
    bubbles.push_back({ who, text, 0, life });
}

// ============================================================ the visitor searches (v0.4)
static const Room kSearchRooms[] = { Room::LivingRoom, Room::Kitchen, Room::Hallway, Room::KidsRoom,
                                     Room::Bathroom, Room::ParentsRoom, Room::Shed, Room::Outside };
static const int kRoomCount = 8;

static bool BlocksSight(int x, int y) {
    if (!story.world.InBounds(x, y)) return true;
    Tile t = story.world.tiles[y][x];
    return t == Tile::Wall || t == Tile::Fence || t == Tile::Void || t == Tile::BasementDoor || (t == Tile::Door && story.DoorClosed(x, y));
}
static bool LineOfSight(Vector2 a, Vector2 b) {
    float d = std::hypot(b.x - a.x, b.y - a.y);
    int n = (int)(d / 4) + 1;
    for (int i = 1; i < n; ++i) {
        float k = i / (float)n;
        if (BlocksSight(TX(a.x + (b.x - a.x) * k), TX(a.y + (b.y - a.y) * k))) return false;
    }
    return true;
}
static bool VisitorSeesKid() {
    if (hidden || visitorBlind) return false;
    float dx = player.pos.x - visitor.pos.x, dy = player.pos.y - visitor.pos.y;
    float d = std::hypot(dx, dy);
    if (d > TS * 7) return false;
    if (d > 26) {
        // he looks where he's walking: a wide cone, not eyes in the back of his head
        const float fx[4] = { 0, 0, -1, 1 }, fy[4] = { 1, -1, 0, 0 };
        float dot = (dx * fx[visitor.dir] + dy * fy[visitor.dir]) / d;
        if (dot < 0.25f) return false;
    }
    return LineOfSight(visitor.pos, player.pos);
}
static bool IsHideObj(const std::string& id) {
    return id == "wardrobe" || id == "bed" || id == "pbed" || id == "toolbox" || id == "couch";
}
// A walkable tile next to an object, the closest one to `from`
static Vector2 SpotNextTo(const Obj& o, Vector2 from) {
    const int dx[4] = { 1, -1, 0, 0 }, dy[4] = { 0, 0, 1, -1 };
    Vector2 best = TileCenter(o.x, o.y); float bd = 1e9f;
    for (int k = 0; k < 4; ++k) {
        int nx = o.x + dx[k], ny = o.y + dy[k];
        if (!story.world.Walkable(nx, ny, false)) continue;
        Vector2 c = TileCenter(nx, ny);
        float d = std::hypot(c.x - from.x, c.y - from.y);
        if (d < bd) { bd = d; best = c; }
    }
    return best;
}
static void PlanRoom(int ri) {
    vRoom = ri; vPath.clear(); vPathI = 0; vSpots.clear();
    Room r = kSearchRooms[ri];
    if (r == Room::Outside) {
        const int pts[4][2] = { { 6, 23 }, { 25, 26 }, { 33, 12 }, { 20, 27 } };
        int start = GetRandomValue(0, 3);
        for (int k = 0; k < 4; ++k) vPath.push_back(TileCenter(pts[(start + k) % 4][0], pts[(start + k) % 4][1]));
    } else {
        std::vector<Vector2> tiles;
        float cx = 0, cy = 0;
        for (int y = 0; y < World::H; ++y) for (int x = 0; x < World::W; ++x)
            if (story.world.RoomAt(x, y) == r && story.world.tiles[y][x] != Tile::Door && story.world.Walkable(x, y, false)) {
                tiles.push_back(TileCenter(x, y)); cx += x; cy += y;
            }
        if (tiles.empty()) return;
        cx = cx / tiles.size() * TS + TS / 2.0f; cy = cy / tiles.size() * TS + TS / 2.0f;
        Vector2 center = tiles[0]; float bd = 1e9f;
        for (auto& t : tiles) { float d = std::hypot(t.x - cx, t.y - cy); if (d < bd) { bd = d; center = t; } }
        vPath.push_back(center);
        if (tiles.size() > 12) vPath.push_back(tiles[GetRandomValue(0, (int)tiles.size() - 1)]);
    }
    // the hiding spots in here: he doesn't always check every one, and not always first
    float chance = vSweep == 0 ? 0.5f : vSweep == 1 ? 0.8f : 1.0f;
    for (int i = 0; i < (int)story.world.objs.size(); ++i) {
        const Obj& o = story.world.objs[i];
        if (IsHideObj(o.id) && story.world.RoomAt(o.x, o.y) == r && GetRandomValue(0, 999) < chance * 1000) vSpots.push_back(i);
    }
    if (vSpots.size() > 1 && GetRandomValue(0, 1)) std::swap(vSpots[0], vSpots[1]);
}
static void PickNextRoom() {
    int best = -1; float bd = 1e9f;
    for (int pass = 0; pass < 2 && best < 0; ++pass) {
        for (int i = 0; i < kRoomCount; ++i) {
            if (vRoomDone[i]) continue;
            Room r = kSearchRooms[i];
            // rough distance to the room, house first, yard and shed last
            float sx = 0, sy = 0; int n = 0;
            for (int y = 0; y < World::H; y += 2) for (int x = 0; x < World::W; x += 2)
                if (story.world.RoomAt(x, y) == r) { sx += x; sy += y; ++n; }
            if (!n) continue;
            float d = std::hypot(sx / n * TS - visitor.pos.x, sy / n * TS - visitor.pos.y);
            if (r == Room::Outside || r == Room::Shed) d += 260;
            d += GetRandomValue(0, 60);
            if (d < bd) { bd = d; best = i; }
        }
        if (best < 0) { ++vSweep; for (bool& b : vRoomDone) b = false; }
    }
    if (best < 0) best = 0;
    vRoomDone[best] = true;
    PlanRoom(best);
}
static void StartSearch(bool dadRan) {
    for (bool& b : vRoomDone) b = false;
    vSweep = 0; vPause = 0; vLostT = 0; vBarkT = 5; vSpotCooldown = 0; vSpotObj = -1;
    vPath.clear(); vSpots.clear(); vRoom = -1;
    if (dadRan) {
        vmode = VMode::Grace;               // he walks back in from the yard. Panic time
        vGraceGoal = TileCenter(20, 19);
        vGraceWait = 1.2f;
    } else {
        vmode = VMode::Grace;
        vGraceGoal = visitor.pos;
        vGraceWait = 2.5f;
    }
}
static void VisitorBark(const char* const* lines, int n) {
    AddBubble("The visitor", lines[GetRandomValue(0, n - 1)], 2.6f);
}
static const char* kSearchLines[] = { "Zero-six. Come on out.", "We do this every time, you know.", "I can hear you breathing.",
                                      "Your mom's waiting. Sort of.", "Ready or not.", "Cold out tonight. Let's not drag it out." };
static const char* kSpotLines[] = { "There you are.", "Peekaboo.", "Hey. 06." };
static const char* kLostLines[] = { "...Clever.", "Fine. The slow way.", "Where'd you go?" };

// ============================================================ beats

static void ToastFor(const std::string& title, const std::string& text, Color c) {
    toasts.push_back({ title, text, 3.2f, c });
}

static const float MINI_W = 200, MINI_H = 120;
static void StartNextBeat() {
    while (!story.beats.empty()) {
        Beat b = story.beats.front();
        story.beats.pop_front();
        if (b.type == BeatType::Sound) { Play(b.id); continue; }
        if (b.type == BeatType::Achievement) { ToastFor("ACHIEVEMENT", b.text, GOLD); continue; }
        if (b.type == BeatType::Clue) { ToastFor("NEW MEMORY  [J]", b.text, Color{ 190, 150, 255, 255 }); SaveGame(); continue; }
        if (b.type == BeatType::Flash) { flashT = 0.9f; continue; }
        if (b.type == BeatType::Bubble) { AddBubble(b.speaker, b.text); if (b.speaker == "Doggo" && IsBark(b.text)) Play("bark"); continue; }
        if (b.type == BeatType::Scene) {
            sceneT = 0;
            hidden = false;
            if (b.id == "doggo_dies") {
                doggoShownDead = true; doggoDeadT = 0;
                if (story.doggoDeadX >= 0) Snap(doggo, story.doggoDeadX, story.doggoDeadY);
                else {
                    // between you and him, and he steps back so you can see
                    float dx = visitor.pos.x - player.pos.x, dy = visitor.pos.y - player.pos.y, d = std::hypot(dx, dy);
                    if (d < 1) { dx = 0; dy = 1; d = 1; }
                    dx /= d; dy /= d;
                    doggo.pos = { player.pos.x + dx * 13, player.pos.y + dy * 13 };
                    for (float back : { 44.0f, 34.0f, 24.0f }) {   // his sprite is tall, give the body some room
                        Vector2 v = { doggo.pos.x + dx * back, doggo.pos.y + dy * back };
                        if (story.world.Walkable(TX(v.x), TX(v.y), false)) { visitor.pos = v; break; }
                    }
                }
                continue;
            }
            if (b.id == "minigame") {
                miniOpen = true; miniY = MINI_H * 0.5f; miniV = 0; miniT = 0; miniScore = 0; miniDead = false; miniStarted = false; miniDeadT = 0; miniPipes.clear();
                return;   // the beat queue waits until the game is over
            }
            if (b.id == "dad_runs") scene = SceneMode::DadRuns;
            else if (b.id == "chase") {
                scene = SceneMode::Chase;
                if (story.visitorDowned) downedT = 7.0f;
                StartSearch(story.dadDown && std::hypot(visitor.pos.x - TileCenter(20, 19).x, visitor.pos.y - TileCenter(20, 19).y) > TS * 3);
            }
            continue;
        }
        if (b.type == BeatType::QTE) qteBar = 0.15f;
        cur = b;
        beatActive = true;
        beatTime = 0;
        typed = 0;
        choiceSel = 0;
        fadeApplied = false;
        curSeen = false;
        if (b.type == BeatType::Say || b.type == BeatType::Black || b.type == BeatType::Narrate) { curSeen = story.SeenLine(b.text); story.MarkSeen(b.text); }
        if (b.type == BeatType::Say && b.speaker == "Doggo" && IsBark(b.text)) Play("bark");
        return;
    }
    beatActive = false;
    interactCooldown = 0.45f;   // v0.6: was 0.3, fast clicking could reopen the same talk
}

// v0.9: Tab skips talk from loop 2, and any line you've read before, black screens too
static bool CanSkip() {
    if (!beatActive) return false;
    if (cur.type == BeatType::Say) return story.Stage() >= 1 || curSeen;
    return (cur.type == BeatType::Black || cur.type == BeatType::Narrate) && curSeen;
}
static void SkipSeen() {
    while (!story.beats.empty()) {
        Beat& f = story.beats.front();
        bool text = f.type == BeatType::Say || f.type == BeatType::Black || f.type == BeatType::Narrate;
        if (f.type == BeatType::Say && story.Stage() >= 1) story.beats.pop_front();
        else if (text && story.SeenLine(f.text)) story.beats.pop_front();
        else if (f.type == BeatType::Sound) { if (f.id.rfind("hush", 0) == 0) Play(f.id); story.beats.pop_front(); }
        else if (f.type == BeatType::Fade && f.minutes == 0 && f.tx < 0 && f.text.empty()) story.beats.pop_front();
        else break;
    }
    StartNextBeat();
}
static float TextMul() { static const float m[4] = { 0.6f, 1.0f, 1.7f, 200.0f }; return m[settings.text]; }
static bool Confirm() {
    return IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void UpdateBeat(float dt) {
    beatTime += dt;
    int px = TX(player.pos.x), py = TX(player.pos.y);
    switch (cur.type) {
    case BeatType::Say:
    case BeatType::Black: {
        typed += dt * 55 * TextMul();
        if (IsKeyPressed(KEY_TAB) && CanSkip()) { SkipSeen(); return; }
        if (Confirm()) {
            if (typed < (float)cur.text.size()) typed = 1e9f;
            else StartNextBeat();
        }
    } break;
    case BeatType::Narrate: {
        typed += dt * 32 * TextMul();
        if (IsKeyPressed(KEY_TAB) && CanSkip()) { SkipSeen(); return; }
        if (Confirm()) {
            if (typed < (float)cur.text.size()) typed = 1e9f;
            else StartNextBeat();
        }
    } break;
    case BeatType::Card: {
        if (beatTime > 1.0f && Confirm()) StartNextBeat();
    } break;
    case BeatType::Ending: {
        if (beatTime > 2.0f && Confirm()) {
            beatActive = false;
            story.beats.clear();
            SaveGame();
            hasSave = true;
            hushMusic = false;
            scene = SceneMode::None;
            screen = Screen::Title;
        }
    } break;
    case BeatType::QTE: {
        if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) qteBar += 0.085f;
        qteBar -= dt * 0.22f;
        if (qteBar < 0) qteBar = 0;
        if (qteBar >= 1.0f || beatTime > 5.0f) {
            bool won = qteBar >= 1.0f;
            beatActive = false;
            story.QTEResult(won);
            StartNextBeat();
        }
    } break;
    case BeatType::Choice: {
        int n = (int)cur.choices.size();
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) choiceSel = (choiceSel + 1) % n;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) choiceSel = (choiceSel + n - 1) % n;
        if (beatTime > 0.15f && (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))) {
            std::string id = cur.choices[choiceSel].id;
            beatActive = false;
            story.Choose(id, px, py);
            StartNextBeat();
        }
    } break;
    case BeatType::Fade: {
        bool quick = cur.minutes == 0 && cur.text.empty();
        float tIn = quick ? 0.25f : 0.35f, hold = quick ? 0.05f : 1.1f;
        if (!fadeApplied && beatTime >= tIn) {
            fadeApplied = true;
            fadeFrom = story.ClockTextSec(); fadeFromT = story.t;
            story.AdvanceMinutes(cur.minutes, px, py);
            fadeTo = story.ClockTextSec(); fadeToT = story.t;
            if (cur.tx >= 0) { Snap(player, cur.tx, cur.ty); player.dir = 0; if (story.DoggoExists() && !story.doggoStay) Snap(doggo, cur.tx + 1, cur.ty); }
            if (story.threats != lastThreats) {
                // v0.7: the blink of black hides him walking back to the living room
                lastThreats = story.threats;
                Snap(visitor, 18, 15);
                vmode = VMode::Search; vPause = 1.2f; vDoorT = 0; PickNextRoom();
            }
        }
        if (beatTime >= tIn + hold + tIn) StartNextBeat();
    } break;
    case BeatType::EndLoop: {
        if (beatTime < 0.01f) { Play("beep"); flashT = 0.6f; }
        if (beatTime > 0.5f) {
            beatActive = false;
            story.EndLoop();
            SaveGame();
            hushMusic = false;
            // v0.6: Outer Wilds style recap of what you learned this loop
            recapList = story.lastLoopNew;
            if (!recapList.empty()) { recapOpen = true; recapT = 0; recapShown = 0; }
            prints.clear();
            Snap(player, 5, 5); player.dir = 0;
            Snap(mom, 8, 16); Snap(dad, 24, 16); Snap(doggo, 6, 6); Snap(visitor, 20, 29);
            momDoor = HeldDoor(); dadDoor = HeldDoor(); lastThreats = 0; vDoorT = 0; struggleT = 0;
            scene = SceneMode::None; hidden = false; wasCollection = false;
            StartNextBeat();
        }
    } break;
    default: StartNextBeat();
    }
}

// ============================================================ interaction target
struct Target { std::string id, name; };
static Target FindTarget() {
    // v0.5: pick whatever is closest to the spot in front of you (people, Doggo or objects),
    // instead of people always winning and Doggo always losing
    int px = TX(player.pos.x), py = TX(player.pos.y);
    const int fdx[4] = { 0, 0, -1, 1 }, fdy[4] = { 1, -1, 0, 0 };
    Vector2 front = { player.pos.x + fdx[player.dir] * 12.0f, player.pos.y + fdy[player.dir] * 12.0f };
    Target best; float bestD = 1e9f;
    auto consider = [&](const std::string& id, const std::string& name, float d, float r, float bias) {
        if (d < r && d - bias < bestD) { bestD = d - bias; best = { id, name }; }
    };
    auto dist = [&](const Actor& a) { return std::hypot(a.pos.x - front.x, a.pos.y - front.y); };
    if (visitor.visible && (story.chase || !story.collection)) consider("visitor", "The visitor", dist(visitor), 18, 4);
    if (story.chase && story.DoggoExists() && doggo.visible) consider("doggo", "Doggo", dist(doggo), 14, 0);
    if (!story.collection) {
        if (mom.visible) consider("mom", "Mom", dist(mom), 14, 2);
        if (dad.visible) consider("dad", "Dad", dist(dad), 14, 2);
        if (story.DoggoExists() && doggo.visible) consider("doggo", "Doggo", dist(doggo), 14, 0);
    }
    auto objAt = [&](int x, int y, float bias) {
        const Obj* o = story.world.ObjAt(x, y);
        if (!o) return;
        Vector2 c = TileCenter(o->x, o->y);
        consider(o->id, o->name, std::hypot(c.x - front.x, c.y - front.y), 20, bias);
    };
    objAt(TX(front.x), TX(front.y), 3);   // the tile you face counts a little extra
    objAt(px, py, 0);
    for (int k = 0; k < 4; ++k) {
        const Obj* c = story.world.ObjAt(px + fdx[k], py + fdy[k]);
        if (c && std::hypot(TileCenter(c->x, c->y).x - player.pos.x, TileCenter(c->x, c->y).y - player.pos.y) < 17) objAt(c->x, c->y, -2);
    }
    return best;
}

// ============================================================ world drawing
static Color Mix(Color a, Color b, float t) {
    return { (unsigned char)(a.r + (b.r - a.r) * t), (unsigned char)(a.g + (b.g - a.g) * t),
             (unsigned char)(a.b + (b.b - a.b) * t), (unsigned char)(a.a + (b.a - a.a) * t) };
}
static unsigned Hash(int x, int y) { unsigned h = x * 374761393u + y * 668265263u; h = (h ^ (h >> 13)) * 1274126177u; return h ^ (h >> 16); }

// v0.5: richer tiles. Same open floor plan, more texture: plank floors, tiled kitchen and bathroom,
// 3/4 view walls with wallpaper, a red log cabin outside, soft shadows where floors meet walls.
static Tile TileAt(int x, int y) {
    if (x < 0 || y < 0 || x >= World::W || y >= World::H) return Tile::Void;
    return story.world.tiles[y][x];
}
static bool IsWallT(int x, int y) { return TileAt(x, y) == Tile::Wall; }
static bool IsOutsideT(Tile t) { return t == Tile::Grass || t == Tile::Gravel || t == Tile::Vidda || t == Tile::Fence; }
static Color Shade(Color c, int d) {
    auto cl = [](int v) { return (unsigned char)std::max(0, std::min(255, v)); };
    return { cl(c.r + d), cl(c.g + d), cl(c.b + d), c.a };
}
static Color WallpaperFor(Room r) {
    switch (r) {
    case Room::KidsRoom: return { 132, 150, 178, 255 };
    case Room::Bathroom: return { 206, 216, 220, 255 };
    case Room::ParentsRoom: return { 142, 156, 128, 255 };
    case Room::Hallway: return { 190, 170, 132, 255 };
    case Room::Kitchen: return { 214, 204, 170, 255 };
    case Room::LivingRoom: return { 182, 138, 92, 255 };
    case Room::Shed: return { 128, 98, 70, 255 };
    case Room::Basement: return { 86, 90, 98, 255 };
    default: return { 150, 140, 120, 255 };
    }
}
static void Planks(int px, int py, int x, int y, Color base, int seamLen) {
    for (int k = 0; k < 4; ++k) {
        int gy = y * 4 + k;
        int off = (int)(Hash(gy, 9) % (unsigned)seamLen);
        for (int i = 0; i < TS; ) {
            int gx = x * TS + i + off;
            int idx = gx / seamLen, rem = seamLen - gx % seamLen;
            int w = std::min(rem, TS - i);
            int v = (int)(Hash(idx, gy) % 13) - 6;
            DrawRectangle(px + i, py + k * 4, w, 4, Shade(base, v));
            if (gx % seamLen == 0) DrawRectangle(px + i, py + k * 4, 1, 4, Shade(base, -28));
            i += w;
        }
        DrawRectangle(px, py + k * 4 + 3, TS, 1, Shade(base, -18));
        if (Hash(x * 7 + k, y * 3) % 9 == 0) DrawRectangle(px + (int)(Hash(x, gy) % 12) + 2, py + k * 4 + 1, 3, 1, Shade(base, -12)); // grain
    }
}
static void FloorShadows(int px, int py, int x, int y) {
    if (IsWallT(x, y - 1)) DrawRectangleGradientV(px, py, TS, 7, Fade(BLACK, 0.38f), Fade(BLACK, 0));
    if (IsWallT(x - 1, y)) DrawRectangleGradientH(px, py, 4, TS, Fade(BLACK, 0.22f), Fade(BLACK, 0));
    if (IsWallT(x + 1, y)) DrawRectangleGradientH(px + TS - 4, py, 4, TS, Fade(BLACK, 0), Fade(BLACK, 0.22f));
}

static void DrawTile(int x, int y) {
    Tile t = story.world.tiles[y][x];
    Room r = story.world.rooms[y][x];
    int px = x * TS, py = y * TS;
    unsigned h = Hash(x, y);
    bool storm = story.Storm();
    switch (t) {
    case Tile::Vidda: {
        // v0.8: deep snow, with rocks sticking through
        Color base = { 214, 222, 236, 255 };
        if (storm) base = Mix(base, WHITE, 0.4f);
        DrawRectangle(px, py, TS, TS, base);
        for (int k = 0; k < 3; ++k) {
            unsigned q = Hash(x * 5 + k, y * 11);
            DrawRectangle(px + q % 13, py + (q >> 5) % 14, 4, 1, Shade(base, -(int)(q % 3) * 6 - 4));
        }
        if (h % 9 == 0) {
            int rx = px + (int)(h % 8), ry = py + (int)((h >> 6) % 8);
            DrawRectangle(rx, ry + 2, 7, 3, Color{ 112, 112, 118, 255 });
            DrawRectangle(rx + 1, ry, 5, 2, base);
            DrawRectangle(rx, ry + 5, 7, 1, Fade(Color{ 60, 70, 110, 255 }, 0.25f));
        }
    } break;
    case Tile::Grass: {
        // v0.8: the yard is under snow. The tile keeps its old name
        Color base = (Hash(x / 2, y / 2) % 2) ? Color{ 234, 239, 247, 255 } : Color{ 228, 234, 244, 255 };
        if (storm) base = Mix(base, WHITE, 0.4f);
        DrawRectangle(px, py, TS, TS, base);
        for (int k = 0; k < 3; ++k) {
            unsigned q = Hash(x * 3 + k, y * 13 + k);
            DrawRectangle(px + q % 12, py + (q >> 4) % 14, 3 + q % 3, 1, Shade(base, -8));
        }
        if (h % 23 == 0 && !storm && std::fmod(globalT + h % 7, 3.0f) < 0.5f) DrawRectangle(px + h % 13 + 1, py + (h >> 3) % 13 + 1, 1, 1, WHITE);
        if (TileAt(x, y - 1) == Tile::Wall) DrawRectangleGradientV(px, py, TS, 7, Fade(Color{ 60, 70, 120, 255 }, 0.3f), Fade(Color{ 60, 70, 120, 255 }, 0));
    } break;
    case Tile::Gravel: {
        // v0.8: the road, ploughed this morning. Two wheel ruts, filling in once the storm comes
        Color base = { 204, 210, 224, 255 };
        if (storm) base = Mix(base, WHITE, 0.45f);
        DrawRectangle(px, py, TS, TS, base);
        Color rut = storm ? Color{ 196, 204, 222, 255 } : Color{ 164, 172, 192, 255 };
        if (x == 19 || x == 21) { DrawRectangle(px + 5, py, 4, TS, rut); DrawRectangle(px + 5, py, 1, TS, Shade(rut, -12)); }
        if (TileAt(x, y - 1) == Tile::Wall) DrawRectangleGradientV(px, py, TS, 7, Fade(Color{ 60, 70, 120, 255 }, 0.3f), Fade(Color{ 60, 70, 120, 255 }, 0));
    } break;
    case Tile::Floor:
    case Tile::ShedFloor: {
        if (r == Room::Kitchen) {
            for (int j = 0; j < 2; ++j) for (int i = 0; i < 2; ++i) {
                bool dark = ((x * 2 + i) + (y * 2 + j)) % 2;
                DrawRectangle(px + i * 8, py + j * 8, 8, 8, dark ? Color{ 168, 170, 162, 255 } : Color{ 214, 212, 198, 255 });
            }
            DrawRectangle(px, py + 7, TS, 1, Fade(BLACK, 0.08f)); DrawRectangle(px + 7, py, 1, TS, Fade(BLACK, 0.08f));
        } else if (r == Room::Bathroom) {
            for (int j = 0; j < 2; ++j) for (int i = 0; i < 2; ++i)
                DrawRectangle(px + i * 8, py + j * 8, 8, 8, (Hash(x * 2 + i, y * 2 + j) % 5 == 0) ? Color{ 168, 194, 204, 255 } : Color{ 186, 208, 216, 255 });
            DrawRectangle(px, py + 7, TS, 1, Color{ 150, 172, 182, 255 }); DrawRectangle(px + 7, py, 1, TS, Color{ 150, 172, 182, 255 });
            DrawRectangle(px, py + 15, TS, 1, Color{ 150, 172, 182, 255 }); DrawRectangle(px + 15, py, 1, TS, Color{ 150, 172, 182, 255 });
        } else if (r == Room::KidsRoom) {
            Color c = { 146, 124, 156, 255 };
            DrawRectangle(px, py, TS, TS, c);
            for (int k = 0; k < 7; ++k) { unsigned q = Hash(x * 31 + k, y * 7); DrawRectangle(px + q % 16, py + (q >> 4) % 16, 1, 1, Shade(c, (q & 1) ? 10 : -10)); }
        } else if (r == Room::Basement) {
            Color c = { 70, 74, 82, 255 };
            DrawRectangle(px, py, TS, TS, c);
            if (h % 5 == 0) DrawCircle(px + (int)(h % 10) + 3, py + (int)((h >> 4) % 10) + 3, 3, Shade(c, -8));
            DrawRectangle(px, py + 15, TS, 1, Shade(c, -10)); DrawRectangle(px + 15, py, 1, TS, Shade(c, -10));
        } else if (t == Tile::ShedFloor) {
            Planks(px, py, x, y, Color{ 116, 100, 84, 255 }, 32);
        } else {
            Color c = r == Room::LivingRoom ? Color{ 170, 122, 82, 255 } : r == Room::ParentsRoom ? Color{ 152, 108, 74, 255 } : Color{ 142, 102, 68, 255 };
            Planks(px, py, x, y, c, 48);
        }
        FloorShadows(px, py, x, y);
    } break;
    case Tile::Wall: {
        Tile below = TileAt(x, y + 1);
        Color cap = { 56, 48, 46, 255 };
        // v0.8: snow on the outer walls
        if (IsOutsideT(TileAt(x, y - 1)) || IsOutsideT(TileAt(x, y + 1)) || IsOutsideT(TileAt(x - 1, y)) || IsOutsideT(TileAt(x + 1, y)))
            cap = (h % 3) ? Color{ 216, 224, 238, 255 } : Color{ 204, 214, 232, 255 };
        DrawRectangle(px, py, TS, TS, cap);
        if (!IsWallT(x, y - 1)) DrawRectangle(px, py, TS, 1, Shade(cap, 22));
        if (!IsWallT(x - 1, y) && TileAt(x - 1, y) != Tile::Door) DrawRectangle(px, py, 1, TS, Shade(cap, 14));
        if (below == Tile::Floor || below == Tile::ShedFloor || below == Tile::BasementDoor) {
            Room rb = story.world.rooms[std::min(World::H - 1, y + 1)][x];
            Color wp = WallpaperFor(rb);
            DrawRectangle(px, py + 6, TS, 10, wp);
            if (rb == Room::KidsRoom && (x % 2 == 0)) DrawRectangle(px + 4, py + 6, 3, 8, Shade(wp, 12));          // stripes
            if (rb == Room::Bathroom || rb == Room::Kitchen) { DrawRectangle(px, py + 10, TS, 1, Shade(wp, -16)); DrawRectangle(px + (y % 2 ? 7 : 0), py + 6, 1, 4, Shade(wp, -16)); }
            if (rb == Room::Hallway) DrawRectangle(px, py + 10, TS, 1, Shade(wp, -24));                             // wainscot rail
            if (rb == Room::Shed) for (int k = 0; k < 3; ++k) DrawRectangle(px, py + 7 + k * 3, TS, 1, Shade(wp, -20));
            DrawRectangle(px, py + 6, TS, 1, Shade(cap, -10));
            DrawRectangle(px, py + 14, TS, 2, rb == Room::Basement ? Shade(wp, -20) : Color{ 92, 66, 46, 255 });      // skirting
        } else if (IsOutsideT(below)) {
            // falu red log cabin, like every proper house on the vidda
            Color red = { 140, 52, 42, 255 };
            if (story.Storm()) red = Mix(red, Color{ 120, 110, 120, 255 }, 0.15f);
            DrawRectangle(px, py + 5, TS, 11, red);
            for (int k = 0; k < 4; ++k) DrawRectangle(px, py + 7 + k * 3, TS, 1, Shade(red, -30));
            if (x % 6 == 0) DrawRectangle(px + 7, py + 5, 2, 11, Color{ 226, 220, 206, 255 });                    // white corner trim
            DrawRectangle(px, py + 5, TS, 1, Color{ 226, 220, 206, 255 });
            DrawRectangle(px + (int)(h % 12), py + 5, 1, 2 + (int)(h % 3), Color{ 226, 236, 246, 255 });          // icicle
            if (y == 20 && (x == 8 || x == 15 || x == 25)) {
                // v0.8: a window with an advent candle bridge. Bulbs start dying once you've gone dark
                bool on = !story.PowerOut();
                DrawRectangle(px + 1, py + 6, 14, 9, Color{ 226, 220, 206, 255 });
                DrawRectangle(px + 2, py + 7, 12, 7, on ? Color{ 255, 206, 120, 255 } : Color{ 24, 30, 48, 255 });
                for (int k = 0; k < 7; ++k) {
                    int dy = std::abs(k - 3);
                    bool dead = !on || (story.tainted && (k + x) % 3 == 0);
                    DrawRectangle(px + 2 + k * 2 - 0, py + 8 + dy, 1, 5 - dy, Color{ 240, 236, 226, 255 });
                    DrawRectangle(px + 2 + k * 2, py + 7 + dy, 1, 1, dead ? Color{ 90, 84, 80, 255 } : Color{ 255, 250, 200, 255 });
                }
            }
            if (y == 20 && x == 21) {
                // the wreath next to the front door
                DrawCircle(px + 8, py + 10, 4.5f, Color{ 40, 92, 62, 255 });
                DrawCircle(px + 8, py + 10, 2.0f, red);
                DrawRectangle(px + 7, py + 13, 3, 2, Color{ 200, 40, 40, 255 });
            }
        }
    } break;
    case Tile::Door: {
        bool horiz = IsWallT(x - 1, y) || IsWallT(x + 1, y);
        Room rb = story.world.rooms[std::min(World::H - 1, y + 1)][x];
        Tile below = TileAt(x, y + 1);
        if (IsOutsideT(below) || IsOutsideT(TileAt(x, y - 1))) Planks(px, py, x, y, Color{ 120, 90, 62, 255 }, 32);
        else Planks(px, py, x, y, Color{ 142, 102, 68, 255 }, 48);
        (void)rb;
        Color frame = { 96, 66, 42, 255 };
        if (horiz) { DrawRectangle(px, py, 2, TS, frame); DrawRectangle(px + TS - 2, py, 2, TS, frame); DrawRectangle(px, py + TS - 2, TS, 2, Shade(frame, -12)); }
        else { DrawRectangle(px, py, TS, 2, frame); DrawRectangle(px, py + TS - 2, TS, 2, frame); }
        if (story.DoorClosed(x, y)) {
            DrawRectangle(px + 1, py + 1, TS - 2, TS - 2, Color{ 92, 60, 36, 255 });
            DrawRectangle(px + 2, py + 2, TS - 4, TS - 4, Color{ 112, 74, 46, 255 });
            DrawRectangle(px + 3, py + 3, TS - 6, TS - 6, Color{ 132, 90, 56, 255 });
            DrawRectangle(px + 4, py + 4, TS - 8, 3, Color{ 146, 102, 64, 255 });
            DrawRectangle(px + TS - 6, py + 7, 2, 2, Color{ 220, 196, 110, 255 });
            if (story.DoorLocked(x, y)) {   // a little brass lock plate
                DrawRectangle(px + TS - 7, py + 9, 4, 4, Color{ 190, 160, 70, 255 });
                DrawRectangle(px + TS - 6, py + 10, 1, 2, Color{ 40, 30, 20, 255 });
            }
        } else if (story.DoorBroken(x, y)) {
            DrawRectangle(px + 1, py + 2, 3, 2, Color{ 132, 90, 56, 255 }); DrawRectangle(px + 10, py + 11, 4, 2, Color{ 112, 74, 46, 255 });
            DrawRectangle(px + 5, py + 13, 2, 1, Color{ 150, 110, 70, 255 }); DrawRectangle(px + 12, py + 3, 2, 3, Color{ 92, 60, 36, 255 });
        }
    } break;
    case Tile::Fence: {
        Tile in = (y == 0 || x == 0 || y == World::H - 1 || x == World::W - 1) ? Tile::Vidda : Tile::Grass;
        DrawRectangle(px, py, TS, TS, in == Tile::Vidda ? Color{ 214, 222, 236, 255 } : Color{ 230, 236, 245, 255 });
        bool vertical = TileAt(x, y - 1) == Tile::Fence && TileAt(x, y + 1) == Tile::Fence && TileAt(x - 1, y) != Tile::Fence;
        Color wood = { 120, 86, 54, 255 };
        if (vertical) {
            DrawRectangle(px + 5, py, 2, TS, wood); DrawRectangle(px + 9, py, 2, TS, Shade(wood, -14));
            DrawRectangle(px + 4, py + 4, 8, 4, Shade(wood, -20));
            DrawRectangle(px + 4, py + 3, 8, 1, WHITE);
        } else {
            DrawRectangle(px, py + 5, TS, 2, wood); DrawRectangle(px, py + 10, TS, 2, wood);
            DrawRectangle(px, py + 12, TS, 1, Fade(BLACK, 0.2f));
            DrawRectangle(px + 6, py + 2, 3, 12, Shade(wood, -16)); DrawRectangle(px + 6, py + 2, 3, 1, Shade(wood, 16));
            DrawRectangle(px + 5, py + 14, 5, 1, Fade(BLACK, 0.25f));
            DrawRectangle(px, py + 4, TS, 1, WHITE); DrawRectangle(px, py + 9, TS, 1, Color{ 236, 240, 248, 255 });   // snow on the rails
            DrawRectangle(px + 6, py + 1, 3, 1, WHITE);
        }
    } break;
    case Tile::Void:
        DrawRectangle(px, py, TS, TS, Color{ 4, 4, 8, 255 });
        break;
    case Tile::BasementDoor:
        if (story.BasementOpen()) {
            DrawRectangle(px, py, TS, TS, Color{ 10, 14, 30, 255 });
            float pulse = 0.5f + 0.5f * std::sin(globalT * 4);
            DrawRectangle(px + 2, py + 2, TS - 4, TS - 4, Fade(Color{ 70, 140, 255, 255 }, 0.25f + 0.25f * pulse));
            for (int k = 0; k < 3; ++k) DrawRectangle(px + 3, py + 4 + k * 4, TS - 6, 1, Fade(Color{ 150, 190, 255, 255 }, 0.4f));
        } else {
            Planks(px, py, x, y, Color{ 142, 102, 68, 255 }, 48);
            DrawRectangle(px + 1, py + 1, TS - 2, TS - 2, Color{ 76, 78, 86, 255 });
            DrawRectangle(px + 2, py + 2, TS - 4, TS - 4, Color{ 98, 102, 112, 255 });
            DrawRectangle(px + 2, py + 2, TS - 4, 1, Color{ 130, 134, 146, 255 });
            DrawRectangle(px + TS - 6, py + 7, 3, 3, Color{ 210, 196, 120, 255 });
        }
        break;
    }
}

static void DrawObj(const Obj& o, Vector2 jit) {
    float x = o.x * TS + jit.x, y = o.y * TS + jit.y;
    const std::string& id = o.id;
    auto R = [&](float ox, float oy, float w, float h, Color c) { DrawRectangle((int)(x + ox), (int)(y + oy), (int)w, (int)h, c); };
    // v0.5: soft contact shadow under furniture
    static const char* flat[] = { "toys", "window", "mirror", "calendar", "frontdoor", "sheddoor", "rope", "stairs", "gate", "basement", "screens", "fliptendo",
                                  "door_kid", "door_bath", "door_parents", "door_kitchen", "door_living", "door_kl" };
    bool isFlat = false;
    for (auto f : flat) if (id == f) isFlat = true;
    if (!isFlat) {
        float sw = (id == "couch" || id == "pbed") ? 36.0f : (id == "bench" || id == "woodpile") ? 24.0f : 15.0f;
        float sx = (id == "couch") ? 8.0f : (id == "pbed") ? 8.0f : 8.0f;
        DrawEllipse((int)(x + sx), (int)(y + 15), sw / 2, 2.5f, Fade(BLACK, 0.22f));
    }
    if (id == "bed") {
        R(0, -5, 16, 21, Color{ 104, 72, 46, 255 });                       // frame
        R(1, -4, 14, 19, Color{ 236, 234, 226, 255 });                     // sheet
        R(2, -3, 12, 5, Color{ 250, 250, 246, 255 }); R(2, 1, 12, 1, Color{ 210, 208, 200, 255 }); // pillow
        R(1, 3, 14, 12, Color{ 74, 104, 168, 255 });                       // duvet
        R(1, 3, 14, 2, Color{ 104, 134, 196, 255 });                       // fold
        for (int k = 0; k < 3; ++k) R(3 + k * 4, 7 + (k % 2) * 3, 2, 2, Color{ 230, 200, 90, 255 }); // stars
        R(1, 14, 14, 1, Color{ 54, 80, 136, 255 });
    }
    else if (id == "toys") { if (!story.roomClean) { R(1, 3, 4, 4, RED); R(8, 8, 5, 3, YELLOW); R(4, 10, 3, 3, SKYBLUE); R(11, 2, 3, 5, GREEN); } }
    else if (id == "window") { R(1, -6, 14, 8, story.Storm() ? Color{ 214, 222, 234, 255 } : Color{ 92, 112, 164, 255 }); R(7, -6, 2, 8, BROWN); }
    else if (id == "toilet") { R(3, -2, 10, 5, Color{ 236, 238, 240, 255 }); R(3, -2, 10, 1, WHITE); R(4, 3, 8, 11, Color{ 242, 244, 246, 255 }); R(5, 5, 6, 6, Color{ 170, 196, 210, 255 }); R(4, 13, 8, 1, Color{ 190, 196, 200, 255 }); }
    else if (id == "mirror") { R(3, -6, 10, 10, Color{ 200, 220, 230, 255 }); R(4, -5, 3, 3, WHITE); }
    else if (id == "desk") {
        R(0, 2, 16, 10, Color{ 122, 80, 48, 255 }); R(0, 2, 16, 1, Color{ 150, 104, 66, 255 });
        R(1, 12, 2, 4, Color{ 90, 58, 34, 255 }); R(13, 12, 2, 4, Color{ 90, 58, 34, 255 });
        R(3, 4, 6, 4, RAYWHITE); R(4, 5, 4, 1, Color{ 150, 150, 160, 255 }); R(4, 7, 3, 1, Color{ 150, 150, 160, 255 });
        R(10, 1, 4, 2, Color{ 230, 200, 120, 255 }); R(11, 3, 2, 3, Color{ 70, 70, 74, 255 });    // lamp
    }
    else if (id == "pbed") {
        R(-8, -5, 32, 23, Color{ 104, 72, 46, 255 });
        R(-7, -4, 30, 21, Color{ 236, 234, 226, 255 });
        R(-6, -3, 12, 5, Color{ 250, 250, 246, 255 }); R(10, -3, 12, 5, Color{ 250, 250, 246, 255 });
        R(-7, 3, 30, 14, Color{ 156, 64, 60, 255 }); R(-7, 3, 30, 2, Color{ 184, 92, 86, 255 });
        R(8, 5, 1, 12, Color{ 130, 50, 48, 255 });
    }
    else if (id == "drawer") {
        R(1, 1, 14, 14, Color{ 100, 64, 36, 255 }); R(4, 4, 8, 2, GOLD); R(4, 9, 8, 2, GOLD);
        if (story.drawerOpen) { R(2, 7, 12, 6, Color{ 40, 26, 16, 255 }); if (story.held != Item::Tape) R(4, 9, 7, 3, Color{ 20, 20, 24, 255 }); if (story.keyInDrawer) R(12, 8, 2, 2, GOLD); }
    }
    else if (id == "coats") { R(6, -6, 3, 20, BROWN); R(1, -4, 6, 10, DARKBLUE); R(9, -3, 6, 9, MAROON); }
    else if (id == "phone") { R(2, 3, 12, 9, Color{ 220, 200, 120, 255 }); R(3, 1, 10, 4, Color{ 60, 60, 60, 255 }); }
    else if (id == "fridge") {
        R(0, -10, 16, 26, Color{ 226, 228, 232, 255 }); R(0, -10, 16, 1, WHITE); R(14, -10, 2, 26, Color{ 196, 198, 204, 255 });
        R(0, -1, 16, 1, Color{ 180, 182, 188, 255 });                       // freezer line
        R(12, -7, 1, 4, Color{ 150, 152, 160, 255 }); R(12, 2, 1, 7, Color{ 150, 152, 160, 255 });
        R(2, -7, 6, 5, Color{ 250, 236, 140, 255 }); R(3, -6, 4, 1, Color{ 120, 120, 120, 255 }); // chore list
        R(4, 4, 2, 2, RED); R(7, 6, 2, 2, Color{ 80, 140, 220, 255 });     // magnets
    }
    else if (id == "stove") {
        R(0, 0, 16, 16, Color{ 222, 222, 216, 255 }); R(0, 0, 16, 7, Color{ 70, 70, 76, 255 });
        DrawCircle((int)x + 4, (int)y + 3, 2.5f, Color{ 40, 40, 44, 255 }); DrawCircle((int)x + 11, (int)y + 3, 2.5f, Color{ 40, 40, 44, 255 });
        DrawCircleLines((int)x + 4, (int)y + 3, 2, Color{ 200, 80, 50, 255 });
        R(2, 8, 12, 6, Color{ 60, 60, 66, 255 }); R(3, 9, 10, 4, Color{ 120, 110, 90, 255 }); R(3, 7, 10, 1, Color{ 160, 160, 160, 255 });
    }
    else if (id == "trashcan") { R(3, 2, 10, 13, Color{ 80, 110, 80, 255 }); if (!story.hasBag && !story.trashDone) R(4, 0, 8, 4, BLACK); }
    else if (id == "bowl") { R(3, 9, 10, 5, Color{ 200, 60, 60, 255 }); if (!story.fed) R(5, 10, 6, 2, Color{ 120, 80, 40, 255 }); }
    else if (id == "tv") {
        R(-1, 8, 18, 8, Color{ 110, 74, 46, 255 }); R(-1, 8, 18, 1, Color{ 140, 100, 64, 255 });   // cabinet
        R(0, -2, 16, 11, Color{ 30, 30, 34, 255 }); R(0, -2, 16, 1, Color{ 70, 70, 76, 255 });
        Color sc = story.PowerOut() ? Color{ 10, 10, 10, 255 } : (story.Stage() >= 2 && std::fmod(globalT, 4.0f) < 0.6f ? (((int)(globalT * 12)) % 2 ? Color{ 150, 150, 150, 255 } : Color{ 60, 60, 60, 255 }) : Color{ 90, 160, 220, 255 });
        R(2, 0, 12, 7, sc);
        if (!story.PowerOut()) { R(3, 1, 4, 1, Fade(WHITE, 0.35f)); R(2, 2 + ((int)(globalT * 9)) % 5, 12, 1, Fade(BLACK, 0.12f)); }
    }
    else if (id == "couch") {
        Color c = { 70, 112, 116, 255 };
        R(-12, -5, 40, 7, Shade(c, -16)); R(-12, -5, 40, 1, Shade(c, 8));                // back
        R(-12, 1, 40, 13, c);
        R(-12, -2, 5, 16, Shade(c, -10)); R(23, -2, 5, 16, Shade(c, -10));              // arms
        R(-7, 2, 1, 10, Shade(c, -20)); R(3, 2, 1, 10, Shade(c, -20)); R(13, 2, 1, 10, Shade(c, -20)); // cushions
        R(-6, 0, 8, 4, Color{ 214, 196, 150, 255 });                                      // pillow
        R(-12, 13, 40, 1, Shade(c, -30));
    }
    else if (id == "radio") {
        R(2, 4, 12, 8, Color{ 140, 80, 50, 255 }); R(4, 6, 4, 4, Color{ 40, 40, 40, 255 });
        // a little nisse on top. From stage 2 he has turned to face the wall
        R(10, 1, 3, 3, Color{ 190, 40, 40, 255 }); R(11, -1, 1, 2, Color{ 190, 40, 40, 255 });
        if (story.Stage() < 2) { R(10, 2, 3, 1, Color{ 240, 210, 180, 255 }); R(10, 3, 3, 1, WHITE); }
    }
    else if (id == "tree") {
        // v0.8: the tree. Lights die as the stages go on, red once you're dark, off in the power cut
        R(6, 12, 4, 4, Color{ 92, 60, 36, 255 });
        Color g = { 40, 96, 66, 255 };
        for (int k = 0; k < 4; ++k) { R(5 - k * 2 + 0, -14 + k * 7, 6 + k * 4, 8, k % 2 ? g : Shade(g, 10)); }
        R(7, -18, 2, 5, Shade(g, 16));
        DrawPoly({ x + 8, y - 19 }, 5, 3, globalT * 8, story.PowerOut() ? Color{ 120, 110, 70, 255 } : GOLD);
        static const int bx[9] = { 7, 4, 10, 2, 12, 6, 0, 8, 14 }, by[9] = { -12, -8, -6, -2, -1, 1, 6, 5, 7 };
        for (int k = 0; k < 9; ++k) {
            bool on = !story.PowerOut() && !(story.Stage() >= 2 && k % 3 == 0) && std::fmod(globalT * 0.7f + k * 0.37f, 2.0f) < 1.4f;
            Color c = story.tainted ? Color{ 220, 40, 40, 255 } : (k % 3 == 0 ? Color{ 255, 220, 120, 255 } : k % 3 == 1 ? Color{ 240, 90, 80, 255 } : Color{ 120, 180, 250, 255 });
            R(bx[k], by[k], 2, 2, on ? c : Color{ 30, 60, 44, 255 });
        }
        R(-3, 10, 6, 6, Color{ 180, 50, 50, 255 }); R(0, 10, 1, 6, Color{ 240, 220, 150, 255 });
        if (!story.zuldaOpened) { R(10, 11, 7, 5, Color{ 60, 90, 170, 255 }); R(13, 11, 1, 5, Color{ 240, 240, 240, 255 }); }
        R(4, 13, 5, 3, Color{ 220, 200, 100, 255 });
    }
    else if (id == "woodstove") {
        R(6, -14, 4, 15, Color{ 36, 36, 40, 255 }); R(6, -14, 1, 15, Color{ 70, 70, 76, 255 });       // pipe
        R(1, 0, 14, 15, Color{ 40, 40, 44, 255 }); R(1, 0, 14, 1, Color{ 80, 80, 86, 255 });
        float fl = std::sin(globalT * 7) * std::sin(globalT * 3.1f);
        R(4, 5, 8, 7, Color{ 30, 20, 16, 255 });
        R(5, 7, 6, 5, Color{ 255, (unsigned char)(120 + 40 * fl), 40, 255 });
        R(6, 9, 4, 3, Color{ 255, 220, 120, 255 });
        R(0, 15, 16, 1, Color{ 60, 60, 64, 255 });
    }
    else if (id == "bins") { R(0, 2, 7, 12, Color{ 40, 70, 40, 255 }); R(9, 2, 7, 12, Color{ 40, 40, 90, 255 }); R(0, 1, 7, 3, WHITE); R(9, 1, 7, 3, WHITE); }
    else if (id == "doghouse") {
        R(0, 3, 16, 12, Color{ 150, 60, 50, 255 }); for (int k = 0; k < 3; ++k) R(0, 6 + k * 3, 16, 1, Color{ 120, 44, 38, 255 });
        R(5, 7, 6, 8, Color{ 24, 18, 16, 255 }); R(5, 7, 6, 1, Color{ 60, 40, 34, 255 });
        R(-1, -1, 18, 5, Color{ 70, 64, 62, 255 }); R(-1, -2, 18, 3, Color{ 236, 240, 248, 255 });
        R(4, 1, 8, 2, Color{ 230, 220, 200, 255 });                                            // name board
    }
    else if (id == "gate") { R(0, 4, 3, 10, Color{ 90, 64, 40, 255 }); }
    else if (id == "flag") { R(7, -20, 2, 34, LIGHTGRAY); float w = std::sin(globalT * 9) * 2; Color navy = { 0, 32, 91, 255 }; R(9, -20, 10 + w, 7, RED); R(12, -20, 3, 7, RAYWHITE); R(9, -18, 10 + w, 3, RAYWHITE); R(13, -20, 1, 7, navy); R(9, -17, 10 + w, 1, navy); }
    else if (id == "generator") {
        R(1, 2, 14, 12, Color{ 206, 164, 44, 255 }); R(1, 2, 14, 1, Color{ 236, 200, 90, 255 });
        R(3, 4, 10, 4, Color{ 40, 40, 40, 255 }); for (int k = 0; k < 4; ++k) R(4 + k * 2, 5, 1, 2, Color{ 90, 90, 90, 255 });
        R(2, 14, 3, 1, BLACK); R(11, 14, 3, 1, BLACK);
    }
    else if (id == "pantry") {
        R(0, -12, 16, 28, Color{ 222, 214, 186, 255 }); R(0, -12, 16, 2, Color{ 240, 234, 210, 255 }); R(14, -10, 2, 26, Color{ 196, 188, 160, 255 });
        R(7, -10, 1, 24, Color{ 170, 160, 130, 255 });
        R(2, -8, 4, 10, Color{ 232, 226, 200, 255 }); R(9, -8, 4, 10, Color{ 232, 226, 200, 255 });
        R(5, 0, 1, 3, Color{ 120, 110, 90, 255 }); R(9, 0, 1, 3, Color{ 120, 110, 90, 255 });
    }
    else if (id == "sink") {
        R(0, 0, 16, 16, Color{ 214, 204, 176, 255 }); R(0, 0, 16, 1, Color{ 236, 230, 210, 255 });
        R(2, 2, 12, 7, Color{ 170, 180, 186, 255 }); R(3, 3, 10, 5, Color{ 130, 146, 156, 255 });
        R(7, -2, 2, 4, Color{ 190, 196, 200, 255 }); R(6, -3, 4, 1, Color{ 190, 196, 200, 255 });
        if (!story.dishesDone) { R(4, 4, 4, 3, Color{ 240, 240, 236, 255 }); R(9, 5, 3, 2, Color{ 210, 170, 120, 255 }); }
        R(2, 10, 12, 5, Color{ 196, 184, 150, 255 }); R(7, 10, 1, 5, Color{ 170, 158, 124, 255 });
    }
    else if (id == "fliptendo") {
        // a beanbag with the handheld on it
        DrawEllipse((int)x + 8, (int)y + 10, 8, 5, Color{ 160, 60, 70, 255 });
        DrawEllipse((int)x + 7, (int)y + 9, 6, 3, Color{ 186, 80, 90, 255 });
        R(4, 6, 8, 5, Color{ 40, 40, 46, 255 }); R(5, 7, 6, 3, story.zuldaOpened ? Color{ 90, 190, 110, 255 } : Color{ 120, 170, 230, 255 });
        if (std::fmod(globalT, 1.4f) < 0.7f) R(6, 8, 1, 1, WHITE);
    }
    else if (id == "wardrobe") {
        R(0, -12, 16, 28, Color{ 128, 86, 54, 255 }); R(0, -12, 16, 2, Color{ 156, 110, 72, 255 }); R(14, -10, 2, 26, Color{ 100, 66, 40, 255 });
        R(7, -10, 1, 24, Color{ 70, 45, 30, 255 });
        R(2, -8, 4, 10, Color{ 140, 96, 62, 255 }); R(9, -8, 4, 10, Color{ 140, 96, 62, 255 });
        R(5, 0, 1, 3, GOLD); R(9, 0, 1, 3, GOLD);
    }
    else if (id == "marbles") { R(4, 4, 8, 10, Fade(Color{ 200, 230, 255, 255 }, 0.8f)); R(5, 9, 2, 2, RED); R(8, 10, 2, 2, BLUE); R(6, 11, 2, 2, GREEN); R(4, 3, 8, 2, Color{ 150, 150, 160, 255 }); }
    else if (id == "calendar") {
        // the advent calendar, every door open except that the last one is red
        R(3, -6, 10, 11, Color{ 200, 56, 52, 255 }); R(3, -6, 10, 1, GOLD);
        for (int k = 0; k < 12; ++k) R(4 + (k % 4) * 2, -4 + (k / 4) * 2, 1, 1, RAYWHITE);
        R(10, 0, 2, 2, Color{ 20, 20, 24, 255 });
    }
    else if (id == "knives") { R(4, 4, 8, 10, Color{ 110, 70, 40, 255 }); if (!story.hasKnife) R(5, 0, 2, 5, LIGHTGRAY); R(8, 1, 2, 4, LIGHTGRAY); R(10, 2, 1, 3, LIGHTGRAY); }
    else if (id == "bookshelf") {
        R(0, -10, 16, 26, Color{ 96, 62, 36, 255 }); R(0, -10, 16, 1, Color{ 130, 88, 54, 255 });
        static const Color bk[6] = { { 150, 50, 50, 255 }, { 50, 80, 140, 255 }, { 60, 120, 80, 255 }, { 200, 170, 90, 255 }, { 120, 90, 140, 255 }, { 210, 200, 180, 255 } };
        for (int k = 0; k < 3; ++k) {
            float bx = 2;
            for (int b = 0; b < 5 && bx < 14; ++b) {
                unsigned q = Hash(k * 7 + b, 3);
                int w = 1 + (int)(q % 3), hgt = 4 + (int)((q >> 3) % 3);
                R(bx, -2 + k * 8 - hgt, (float)w, (float)hgt, bk[q % 6]); bx += w + ((q >> 6) % 2 ? 1 : 0);
            }
            R(1, -2 + k * 8, 14, 1, Color{ 70, 44, 26, 255 });
        }
    }
    else if (id == "album") { R(3, 5, 10, 7, Color{ 120, 40, 50, 255 }); R(4, 6, 8, 1, GOLD); }
    else if (id == "frontdoor") {
        if (story.frontLocked && !story.collection) R(6, 2, 4, 4, Color{ 220, 200, 80, 255 });
        if (story.trapsSet.count("marbles")) for (int k = 0; k < 7; ++k) R((float)((k * 5) % 14), 18 + (float)((k * 3) % 6), 2, 2, k % 2 ? RED : SKYBLUE);
        if (story.trapsSet.count("tripwire")) R(-2, 22, 20, 1, Color{ 230, 230, 210, 255 });
        if (story.trapsSet.count("paint")) { R(3, -14, 10, 9, Color{ 220, 60, 60, 255 }); R(7, -18, 1, 4, GRAY); }
    }
    else if (id == "sheddoor") { if (story.dadLocked) R(-2, 6, 20, 3, Color{ 160, 120, 70, 255 }); }
    else if (id == "rope") { DrawCircleLines((int)x + 8, (int)y + 8, 5, Color{ 200, 180, 120, 255 }); DrawCircleLines((int)x + 8, (int)y + 8, 3, Color{ 200, 180, 120, 255 }); }
    else if (id == "toolbox") { R(1, 5, 14, 9, Color{ 190, 40, 40, 255 }); R(5, 3, 6, 2, GRAY); R(11, -2, 5, 7, Color{ 230, 230, 230, 255 }); }
    else if (id == "woodpile") { for (int k = 0; k < 3; ++k) R(-4 + k * 7, 8, 7, 7, Color{ 130, 90, 55, 255 }); R(4, 0, 8, 9, Color{ 110, 75, 45, 255 }); R(9, -4, 2, 8, Color{ 140, 140, 150, 255 }); R(-4, 7, 21, 2, WHITE); R(4, -1, 8, 2, WHITE); }
    else if (id == "stairs") { for (int k = 0; k < 4; ++k) R(0, k * 4, 16, 3, Color{ 90, 92, 100, 255 }); }
    else if (id == "screens") {
        for (int k = 0; k < 6; ++k) {
            float fx = (float)((k % 3) * 11 - 8), fy = (float)((k / 3) * 9 - 12);
            R(fx, fy, 10, 8, Color{ 20, 20, 24, 255 });
            Color glow = Mix(Color{ 60, 140, 255, 255 }, Color{ 150, 200, 255, 255 }, 0.5f + 0.5f * std::sin(globalT * 3 + k));
            R(fx + 1, fy + 1, 8, 6, glow);
        }
    }
    else if (id == "files") { R(1, -6, 14, 22, Color{ 120, 125, 130, 255 }); for (int k = 0; k < 3; ++k) R(5, -3 + k * 7, 6, 2, Color{ 60, 60, 66, 255 }); }
    else if (id == "console") { R(-2, 2, 20, 12, Color{ 50, 55, 60, 255 }); R(10, 5, 4, 4, RED); for (int k = 0; k < 3; ++k) R(1 + k * 3, 5, 2, 2, GREEN); }
    else if (id == "chair") { R(2, -6, 12, 20, Color{ 80, 40, 40, 255 }); R(0, 2, 16, 2, Color{ 60, 50, 40, 255 }); R(0, 8, 16, 2, Color{ 60, 50, 40, 255 }); R(3, -4, 10, 2, Color{ 60, 50, 40, 255 }); }
    else if (id == "vcr") {
        R(1, 8, 14, 7, Color{ 70, 70, 76, 255 }); R(2, 14, 2, 2, Color{ 30, 30, 30, 255 }); R(12, 14, 2, 2, Color{ 30, 30, 30, 255 });   // cart
        R(2, 9, 12, 3, Color{ 30, 30, 34, 255 }); R(4, 10, 6, 1, Color{ 10, 10, 10, 255 }); R(11, 10, 1, 1, RED);                          // VCR
        R(0, -6, 16, 14, Color{ 50, 46, 44, 255 }); R(2, -4, 10, 10, Color{ 24, 30, 36, 255 });                                              // TV
        if (std::fmod(globalT, 2.0f) < 1.0f) R(3, -3, 8, 1, Fade(WHITE, 0.08f));
    }
    else if (id == "medfridge") { R(1, -6, 14, 22, Color{ 230, 235, 240, 255 }); R(3, -3, 10, 8, Fade(SKYBLUE, 0.6f)); for (int k = 0; k < 4; ++k) R(4 + k * 2, 0, 1, 3, ORANGE); }
    else if (id == "bench") { R(-4, 2, 20, 12, Color{ 120, 90, 60, 255 }); R(0, 3, 5, 3, GRAY); R(8, 4, 6, 4, Color{ 50, 50, 50, 255 }); }
}

// Simple pixel people with a 4 frame walk
struct Look { Color shirt, pants, hair, skin; float scale; bool longHair; bool beard; Color eyes = BLACK; };
static void DrawPerson(const Actor& a, Look l) {
    if (!a.visible) return;
    float s = l.scale;
    bool moving = a.anim > 0;
    int frame = moving ? ((int)a.anim) % 4 : 0;
    float breathe = moving ? 0 : (std::fmod(globalT + a.pos.x * 0.01f, 1.6f) < 0.8f ? 0 : 1);
    float bob = (frame == 1 || frame == 3) ? -1 : 0;
    float x = a.pos.x, y = a.pos.y + 4 + bob;
    int pass = 0;   // v0.5: pass 0 draws a dark outline, pass 1 the person
    Color outline = { 26, 20, 22, 230 };
    auto R = [&](float ox, float oy, float w, float h, Color c) {
        int rx = (int)std::round(x + ox * s), ry = (int)std::round(y + oy * s), rw = (int)std::ceil(w * s), rh = (int)std::ceil(h * s);
        if (pass == 0) DrawRectangle(rx - 1, ry - 1, rw + 2, rh + 2, outline);
        else DrawRectangle(rx, ry, rw, rh, c);
    };
    DrawEllipse((int)x, (int)(a.pos.y + 4 + 3 * s), 6 * s, 2 * s, Fade(BLACK, 0.3f));
    for (pass = 0; pass < 2; ++pass) {
    Color pantsD = Mix(l.pants, BLACK, 0.25f), shirtD = Mix(l.shirt, BLACK, 0.2f);
    bool side = a.dir == 2 || a.dir == 3;
    float flip = a.dir == 2 ? -1 : 1;
    // legs
    if (side) {
        float swing = frame == 0 ? 2 : frame == 2 ? -2 : 0;
        R(-2 + swing * flip, -2, 3, 5, pantsD);
        R(-1 - swing * flip, -2, 3, 5, l.pants);
        R(-2 + swing * flip + (flip > 0 ? 1 : -1), 2, 3, 1, Color{ 40, 30, 25, 255 });
    } else {
        float lift0 = frame == 0 ? 1.5f : 0, lift2 = frame == 2 ? 1.5f : 0;
        R(-4, -2 - lift0, 3, 5, l.pants);
        R(1, -2 - lift2, 3, 5, l.pants);
        R(-4, 2 - lift0, 3, 1, Color{ 40, 30, 25, 255 });
        R(1, 2 - lift2, 3, 1, Color{ 40, 30, 25, 255 });
    }
    // body
    R(-5, -10 - breathe * 0.5f, 10, 9, l.shirt);
    R(-5, -2, 10, 1, shirtD);
    // arms
    float armSwing = moving ? (frame == 0 ? 2 : frame == 2 ? -2 : 0) : 0;
    if (side) {
        R(-1 - armSwing * flip, -9, 3, 6, shirtD);
        R(-1 - armSwing * flip, -3, 3, 2, l.skin);
    } else {
        R(-7, -9 + armSwing * 0.5f, 2, 6, shirtD);
        R(5, -9 - armSwing * 0.5f, 2, 6, shirtD);
        R(-7, -3 + armSwing * 0.5f, 2, 2, l.skin);
        R(5, -3 - armSwing * 0.5f, 2, 2, l.skin);
    }
    // head
    R(-4, -18 - breathe * 0.5f, 8, 8, l.skin);
    float hy = -breathe * 0.5f;
    if (a.dir == 1) R(-4, -19 + hy, 8, 8, l.hair);
    else {
        R(-4, -19 + hy, 8, 3, l.hair);
        if (side) R(flip > 0 ? -4 : 2, -19 + hy, 2, 6, l.hair);
        if (l.longHair) { R(-5, -18 + hy, 2, 9, l.hair); R(3, -18 + hy, 2, 9, l.hair); }
        bool blink = std::fmod(globalT + a.pos.y * 0.03f, 3.7f) < 0.12f;
        if (!blink) {
            if (a.dir == 0) { R(-2, -14 + hy, 1, 2, l.eyes); R(1, -14 + hy, 1, 2, l.eyes); }
            if (a.dir == 2) R(-3, -14 + hy, 1, 2, l.eyes);
            if (a.dir == 3) R(2, -14 + hy, 1, 2, l.eyes);
        }
        if (l.beard) R(-4, -12 + hy, 8, 3, l.hair);
    }
    if (pass == 1) {
        // a little light on the hair and shoulders
        R(-3, -19 + hy, 4, 1, Shade(l.hair, 30));
        R(-5, -10 - breathe * 0.5f, 10, 1, Shade(l.shirt, 22));
    }
    }
}

// v0.7: someone on the floor. Lying on their side, not moving much.
static void DrawPersonDown(const Actor& a, Look l, bool sob) {
    float s = l.scale, x = a.pos.x, y = a.pos.y + 4;
    float shake = sob ? (std::fmod(globalT, 0.5f) < 0.25f ? 0.5f : 0) : 0;
    DrawEllipse((int)x, (int)y + 2, 11 * s, 2.5f * s, Fade(BLACK, 0.3f));
    for (int pass = 0; pass < 2; ++pass) {
        auto R = [&](float ox, float oy, float w, float h, Color c) {
            int rx = (int)std::round(x + ox * s), ry = (int)std::round(y + oy * s + shake), rw = (int)std::ceil(w * s), rh = (int)std::ceil(h * s);
            if (pass == 0) DrawRectangle(rx - 1, ry - 1, rw + 2, rh + 2, Color{ 26, 20, 22, 230 });
            else DrawRectangle(rx, ry, rw, rh, c);
        };
        R(-2, -5, 7, 5, l.pants);                  // legs
        R(5, -4, 2, 4, Color{ 40, 30, 25, 255 });  // shoes
        R(-10, -6, 9, 6, l.shirt);                 // body
        R(-8, -1, 5, 2, l.skin);                   // a hand on the floor
        R(-17, -7, 7, 7, l.skin);                  // head
        R(-17, -8, 7, 3, l.hair);
        if (l.longHair) R(-19, -7, 3, 7, l.hair);
        if (l.beard) R(-12, -3, 2, 3, l.hair);
        if (pass == 1) R(-15, -4, 2, 1, Color{ 40, 30, 30, 255 });   // closed eye
    }
}

static void DrawDoggoDead(const Actor& a) {
    // v0.7: fade to black, and this is what's left. He doesn't move. A thin line of red keeps running.
    float x = a.pos.x, y = a.pos.y + 4;
    float t = doggoDeadT;
    Color fur = { 150, 106, 62, 255 }, dark = { 106, 70, 40, 255 }, blood = { 160, 18, 24, 255 };
    float pool = std::min(8.0f, 2.0f + t * 0.4f);
    DrawEllipse((int)x + 2, (int)y + 2, pool + 2, pool * 0.45f, Fade(blood, 0.85f));
    float streak = std::min(18.0f, t * 1.2f);
    DrawRectangle((int)x + 3, (int)y + 2, 1, (int)streak, blood);
    if (std::fmod(t, 1.6f) < 0.8f && streak < 18) DrawRectangle((int)x + 3, (int)(y + 2 + streak), 1, 1, Color{ 160, 20, 24, 255 });
    DrawRectangle((int)x - 8, (int)y - 5, 15, 7, Color{ 26, 20, 22, 220 });
    DrawRectangle((int)x - 7, (int)y - 4, 13, 5, fur);                      // on his side
    DrawRectangle((int)x - 7, (int)y - 4, 13, 1, Shade(fur, 20));
    DrawRectangle((int)x - 5, (int)y + 1, 4, 2, dark); DrawRectangle((int)x + 1, (int)y + 1, 4, 2, dark);   // legs, still
    DrawRectangle((int)x - 13, (int)y - 3, 6, 5, fur);                      // head flat on the floor
    DrawRectangle((int)x - 12, (int)y - 4, 2, 2, dark);                     // ear
    DrawRectangle((int)x - 11, (int)y - 1, 2, 1, Color{ 30, 20, 20, 255 }); // closed eye
    DrawRectangle((int)x + 6, (int)y - 2, 4, 2, dark);                      // tail, not wagging
}

static void DrawDoggo(const Actor& a) {
    if (!a.visible) return;
    bool moving = a.anim > 0;
    int frame = moving ? ((int)(a.anim * 1.4f)) % 4 : 0;
    float bob = moving && (frame == 1 || frame == 3) ? -1 : 0;
    float x = a.pos.x, y = a.pos.y + 3 + bob;
    Color fur = { 170, 120, 70, 255 }, dark = { 120, 80, 45, 255 };
    DrawEllipse((int)x, (int)a.pos.y + 6, 7, 2, Fade(BLACK, 0.3f));
    bool left = a.dir == 2;
    {   // v0.5: outline
        Color o = { 26, 20, 22, 220 };
        DrawRectangle((int)x - 7, (int)y - 7, 14, 8, o);
        DrawRectangle(left ? (int)x - 10 : (int)x + 2, (int)y - 12, 8, 9, o);
    }
    DrawRectangle((int)x - 6, (int)y - 6, 12, 6, fur);
    DrawRectangle((int)x - 6, (int)y - 6, 12, 1, Shade(fur, 25));
    // legs trot in pairs
    int l1 = frame == 0 ? 1 : 0, l2 = frame == 2 ? 1 : 0;
    DrawRectangle((int)x - 5, (int)y - l1, 2, 3, dark); DrawRectangle((int)x - 2, (int)y - l2, 2, 3, dark);
    DrawRectangle((int)x + 1, (int)y - l1, 2, 3, dark); DrawRectangle((int)x + 4, (int)y - l2, 2, 3, dark);
    int hx = left ? (int)x - 9 : (int)x + 3;
    DrawRectangle(hx, (int)y - 10, 6, 6, fur);
    DrawRectangle(left ? hx : hx + 4, (int)y - 7, 2, 2, BLACK);
    DrawRectangle(left ? hx + 4 : hx, (int)y - 11, 2, 3, dark);
    float wag = std::sin(globalT * (moving ? 20 : 12)) * 2;
    DrawRectangle(left ? (int)x + 6 : (int)x - 8, (int)(y - 7 + wag), 2, 4, dark);
}

static void DrawCar() {
    float x = 20 * TS - 6, y = 31 * TS + 2;
    DrawRectangle((int)x, (int)y, 28, 22, Color{ 20, 20, 24, 255 });
    DrawRectangle((int)x + 4, (int)y + 4, 20, 7, Color{ 60, 70, 90, 255 });
    DrawRectangle((int)x + 2, (int)y + 11, 24, 3, Color{ 232, 238, 246, 255 });   // snow on the roof
    DrawRectangle((int)x + 4, (int)y + 10, 9, 1, Color{ 232, 238, 246, 255 });
    DrawCircleGradient((int)x + 6, (int)y, 16, Fade(Color{ 255, 250, 200, 255 }, 0.2f), Fade(Color{ 255, 250, 200, 255 }, 0));
    DrawCircleGradient((int)x + 22, (int)y, 16, Fade(Color{ 255, 250, 200, 255 }, 0.2f), Fade(Color{ 255, 250, 200, 255 }, 0));
}

// ============================================================ v0.5 lighting, rain, dust
// v0.8: footprints in the snow. The storm fills them in fast
static float printCD[3] = { 0, 0, 0 };
static float vPrintCD = 0;
static int printSide = 0;
static void AddPrint(const Actor& a, int who, float dt, float every) {
    printCD[who] -= dt;
    if (printCD[who] > 0 || a.anim <= 0) return;
    if (!IsOutsideT(TileAt(TX(a.pos.x), TX(a.pos.y + 6)))) return;
    printCD[who] = every;
    printSide = 1 - printSide;
    prints.push_back({ { a.pos.x, a.pos.y + 6 }, 0, who, printSide });
    if (prints.size() > 500) prints.erase(prints.begin());
}
static void UpdatePrints(float dt) {
    float life = story.Storm() ? 16.0f : 150.0f;
    for (auto& p : prints) p.t += dt;
    prints.erase(std::remove_if(prints.begin(), prints.end(), [&](const Print& p) { return p.t > life; }), prints.end());
}
static void DrawPrints() {
    float life = story.Storm() ? 16.0f : 150.0f;
    Color c = { 142, 156, 190, 255 };
    for (auto& p : prints) {
        float a = 0.8f * (1 - p.t / life);
        int ox = p.side ? 2 : -2;
        if (p.who == 1) DrawRectangle((int)p.p.x + ox - 1, (int)p.p.y, 4, 2, Fade(c, a));
        else if (p.who == 2) DrawRectangle((int)p.p.x + ox / 2, (int)p.p.y, 1, 1, Fade(c, a));
        else DrawRectangle((int)p.p.x + ox, (int)p.p.y, 2, 1, Fade(c, a));
    }
    // somebody big walked along the house, long before the car came. The storm takes them
    if (story.t >= 28 * 60 && story.t < 47 * 60 && !story.Tutorial())
        for (int i = 0; i < 26; ++i) {
            int fx = 6 * TS + i * 10, fy = 22 * TS + 2 + (i % 2) * 5 + (int)(Hash(i, 9) % 3);
            DrawRectangle(fx, fy, 4, 2, Fade(c, 0.75f));
        }
}
struct Puff { Vector2 p; float t; };
static std::vector<Puff> puffs;
static float puffTimer = 0;

static Color LerpC(Color a, Color b, float t) { return Mix(a, b, std::max(0.0f, std::min(1.0f, t))); }

static void BuildLightmap(Camera2D cam) {
    float m = story.t / 60.0f;
    float dusk = std::min(1.0f, m / 45.0f);
    // v0.8: the sun went down at about 15:25. Blue dusk at 16:00, night by about 16:25. The warmth is all indoors
    Color amb = LerpC(Color{ 196, 204, 228, 255 }, Color{ 128, 138, 182, 255 }, m / 25.0f);
    if (story.Storm()) amb = Shade(amb, -14);
    bool powerOut = story.PowerOut();
    if (powerOut) amb = Color{ 96, 102, 146, 255 };
    if (story.inBasement) amb = Color{ 92, 100, 140, 255 };
    BeginTextureMode(lightRT);
    ClearBackground(amb);
    if (!powerOut && !story.inBasement) {
        // the house and the shed stay warm while the power lasts
        float flick = 1;
        if (story.Storm() && !settings.calmFlashes) { float f = std::sin(globalT * 23) * std::sin(globalT * 7.3f); if (f > 0.85f) flick = 0.72f; }
        Color warm = LerpC(Color{ 255, 246, 228, 255 }, Color{ 226, 206, 182, 255 }, dusk);
        warm.r = (unsigned char)(warm.r * flick); warm.g = (unsigned char)(warm.g * flick); warm.b = (unsigned char)(warm.b * flick);
        float ox = VW / 2.0f - cam.target.x, oy = VH / 2.0f - cam.target.y;
        DrawRectangle((int)(3 * TS + ox), (int)(2 * TS + 6 + oy), 26 * TS, 18 * TS - 6, warm);
        DrawRectangle((int)(32 * TS + ox), (int)(4 * TS + 6 + oy), 5 * TS, 6 * TS - 6, warm);
    }
    BeginBlendMode(BLEND_ADDITIVE);
    auto light = [&](float wx, float wy, float r, Color c, float k) {
        if (k <= 0.01f) return;
        float sx = wx - cam.target.x + VW / 2.0f, sy = wy - cam.target.y + VH / 2.0f;
        if (sx < -r || sy < -r || sx > VW + r || sy > VH + r) return;
        Color in = { (unsigned char)(c.r * k), (unsigned char)(c.g * k), (unsigned char)(c.b * k), 255 };
        DrawCircleGradient((int)sx, (int)sy, r, in, Color{ 0, 0, 0, 255 });
    };
    auto tile = [](float t) { return t * TS + TS / 2.0f; };
    if (!powerOut) {
        float lamp = std::max(0.0f, dusk - 0.1f) * 1.1f;
        if (story.Storm() && !settings.calmFlashes) { float f = std::sin(globalT * 23) * std::sin(globalT * 7.3f); if (f > 0.85f) lamp *= 0.3f; }
        Color warm = { 150, 104, 52, 255 };
        const float L[][2] = { { 6, 5 }, { 13, 5 }, { 22, 5 }, { 8, 10 }, { 24, 10 }, { 7, 16 }, { 19, 15 }, { 25, 17 }, { 34, 7 } };
        for (auto& p : L) light(tile(p[0]), tile(p[1]), 78, warm, lamp);
        light(tile(21), tile(13) + 6, 42, Color{ 40, 70, 120, 255 }, 0.5f + 0.2f * std::sin(globalT * 11));   // TV
        // window light on the snow, and the tree
        const float W[][2] = { { 8, 22 }, { 15, 22 }, { 25, 22 } };
        for (auto& p : W) {
            light(tile(p[0]), tile(p[1]) - 10, 40, Color{ 74, 44, 6, 255 }, 0.9f);
            light(tile(p[0]), tile(20) + 2, 12, Color{ 120, 90, 40, 255 }, 1.0f);
        }
        light(tile(26), tile(15) - 8, 34, Color{ 90, 70, 40, 255 }, 0.8f);
    }
    float fire = 0.75f + 0.25f * std::sin(globalT * 7) * std::sin(globalT * 3.1f);
    light(tile(27), tile(18), 46, Color{ 150, 70, 20, 255 }, fire * (0.4f + dusk * 0.6f + (powerOut ? 0.5f : 0)));
    light(tile(10), tile(37) - 4, 90, Color{ 40, 80, 150, 255 }, 0.9f);                                      // basement screens
    if (story.CarHere()) {
        light(20 * TS - 6 + 6, 31 * TS + 2, 60, Color{ 150, 140, 100, 255 }, 0.4f);
        light(20 * TS - 6 + 22, 31 * TS + 2, 60, Color{ 150, 140, 100, 255 }, 0.4f);
    }
    if (powerOut) light(player.pos.x, player.pos.y - 6, 72, Color{ 70, 70, 90, 255 }, IsOutsideT(TileAt(TX(player.pos.x), TX(player.pos.y))) ? 0.35f : 1.0f);              // eyes adjusting
    if (story.hasLight) {
        // v0.6: Dad's flashlight, a beam in the direction you face
        const float fx[4] = { 0, 0, -1, 1 }, fy[4] = { 1, -1, 0, 0 };
        float k = powerOut ? 1.0f : 0.35f;
        for (int i = 1; i <= 3; ++i) light(player.pos.x + fx[player.dir] * 26 * i, player.pos.y - 4 + fy[player.dir] * 26 * i, 26 + 8 * i, Color{ 170, 165, 130, 255 }, k);
    }
    EndBlendMode();
    EndTextureMode();
}

static void DrawRainAndDust(Camera2D cam, float dt) {
    float left = cam.target.x - VW / 2.0f, top = cam.target.y - VH / 2.0f;
    // dust puffs when running
    puffTimer -= dt;
    if (kidRunning && puffTimer <= 0) { puffs.push_back({ { player.pos.x + (float)GetRandomValue(-3, 3), player.pos.y + 6 }, 0 }); puffTimer = 0.09f; }
    for (auto& p : puffs) p.t += dt;
    puffs.erase(std::remove_if(puffs.begin(), puffs.end(), [](const Puff& p) { return p.t > 0.45f; }), puffs.end());
    for (auto& p : puffs) {
        Tile t = TileAt(TX(p.p.x), TX(p.p.y));
        Color c = IsOutsideT(t) ? Color{ 246, 248, 252, 255 } : Color{ 220, 210, 190, 255 };
        DrawCircle((int)p.p.x, (int)(p.p.y - p.t * 6), 1.5f + p.t * 6, Fade(c, 0.45f * (1 - p.t / 0.45f)));
    }
    // v0.8: it snows the whole hour. The storm turns it sideways
    bool storm = story.Storm();
    int flakes = storm ? 460 : 120;
    float wind = storm ? 170.0f : 9.0f, fall = storm ? 170.0f : 26.0f;
    for (int i = 0; i < flakes; ++i) {
        float wobble = storm ? 0 : std::sin(globalT * 0.9f + i) * 5.0f;
        float wx = left + std::fmod((float)(Hash(i, 1) % 997) + globalT * wind * (0.8f + (Hash(i, 4) % 5) * 0.1f), (float)VW + 20) - 10 + wobble;
        float wy = top + std::fmod((float)(Hash(i, 2) % 991) + globalT * (fall + (Hash(i, 3) % 40)), (float)VH + 20) - 10;
        Tile t = TileAt(TX(wx), TX(wy));
        if (!IsOutsideT(t)) continue;
        if (storm) DrawRectangle((int)wx, (int)wy, 3, 1, Fade(WHITE, 0.75f));
        else DrawRectangle((int)wx, (int)wy, 1 + i % 2, 1 + i % 2, Fade(WHITE, 0.9f));
    }
}

static void DrawWorld(Camera2D cam) {
    ClearBackground(BLACK);
    BeginMode2D(cam);
    int x0 = std::max(0, TX(cam.target.x - VW / 2.0f) - 1), x1 = std::min(World::W - 1, TX(cam.target.x + VW / 2.0f) + 1);
    int y0 = std::max(0, TX(cam.target.y - VH / 2.0f) - 1), y1 = std::min(World::H - 1, TX(cam.target.y + VH / 2.0f) + 2);
    for (int y = y0; y <= y1; ++y) for (int x = x0; x <= x1; ++x) DrawTile(x, y);

    DrawPrints();
    bool flux = story.Act() >= 2;
    // Draw objects and actors sorted by y
    struct D { float y; int kind; const Obj* o; const Actor* a; };
    std::vector<D> ds;
    for (auto& o : story.world.objs) ds.push_back({ o.y * TS + 8.0f, 0, &o, nullptr });
    if (!hidden) ds.push_back({ player.pos.y, 1, nullptr, &player });
    if (mom.visible) ds.push_back({ mom.pos.y, 2, nullptr, &mom });
    if (dad.visible) ds.push_back({ dad.pos.y, 3, nullptr, &dad });
    if (story.DoggoExists() || story.doggoDead) ds.push_back({ doggo.pos.y, 4, nullptr, &doggo });
    if (visitor.visible) ds.push_back({ visitor.pos.y, 5, nullptr, &visitor });
    std::sort(ds.begin(), ds.end(), [](const D& a, const D& b) { return a.y < b.y; });
    for (auto& d : ds) {
        switch (d.kind) {
        case 0: {
            Vector2 j{ 0, 0 };
            // one object at a time, a short shiver every ~9 seconds (more often once you're dark)
            if (flux) {
                float period = story.Act() >= 3 ? 5.0f : 9.0f;
                int slot = (int)(globalT / period);
                unsigned pick = Hash(slot, 77) % (unsigned)story.world.objs.size();
                if (&story.world.objs[pick] == d.o && std::fmod(globalT, period) < 0.25f) j = { (float)((int)(globalT * 30) % 3 - 1), 0 };
            }
            DrawObj(*d.o, j);
        } break;
        case 1: {
            Look kid{ Color{ 200, 50, 50, 255 }, Color{ 50, 60, 110, 255 }, Color{ 90, 60, 30, 255 }, Color{ 240, 200, 170, 255 }, 1.0f, false, false };
            if (story.tainted) { kid.skin = Color{ 222, 208, 204, 255 }; kid.shirt = Color{ 120, 30, 34, 255 }; kid.eyes = Color{ 150, 0, 0, 255 }; }
            if (story.Stage() >= 4) { kid.skin = Color{ 205, 195, 200, 255 }; kid.shirt = Color{ 50, 20, 24, 255 }; }
            if (story.hasCoat) kid.shirt = story.tainted ? Color{ 30, 34, 54, 255 } : Color{ 48, 78, 128, 255 };
            DrawPerson(*d.a, kid);
            {
                // v0.6: whatever is in the kid's hands
                int hx = (int)d.a->pos.x + 6, hy = (int)d.a->pos.y - 4;
                switch (story.held) {
                case Item::Knife: DrawRectangle(hx, hy, 1, 5, Color{ 210, 210, 220, 255 }); DrawRectangle(hx, hy + 5, 1, 2, Color{ 90, 60, 40, 255 }); break;
                case Item::Key: DrawRectangle(hx, hy + 1, 3, 2, GOLD); DrawRectangle(hx + 2, hy + 3, 1, 2, GOLD); break;
                case Item::Stick: DrawLine(hx, hy + 6, hx + 4, hy - 4, Color{ 130, 90, 50, 255 }); break;
                case Item::Bag: DrawRectangle(hx - 1, hy + 1, 5, 6, Color{ 30, 30, 34, 255 }); DrawRectangle(hx, hy, 3, 1, Color{ 60, 60, 66, 255 }); break;
                case Item::DogFood: DrawRectangle(hx - 1, hy + 2, 4, 4, Color{ 200, 60, 60, 255 }); DrawRectangle(hx, hy + 1, 2, 1, Color{ 150, 100, 50, 255 }); break;
                case Item::Flashlight: DrawRectangle(hx, hy + 1, 4, 2, Color{ 60, 60, 66, 255 }); DrawRectangle(hx + 4, hy, 1, 4, Color{ 250, 240, 180, 255 }); break;
                case Item::Tape: DrawRectangle(hx - 1, hy + 1, 5, 3, Color{ 20, 20, 24, 255 }); DrawRectangle(hx, hy + 2, 3, 1, Color{ 220, 220, 210, 255 }); break;
                default: break;
                }
            }
        }
            // Watch glint
            DrawRectangle((int)d.a->pos.x + 4, (int)d.a->pos.y - 2, 2, 2, Color{ 180, 220, 160, 255 });
            break;
        case 2: {
            Look ml{ Color{ 70, 130, 90, 255 }, Color{ 70, 130, 90, 255 }, Color{ 180, 130, 60, 255 }, Color{ 235, 195, 165, 255 }, 1.15f, true, false };
            if (story.momOnFloor) DrawPersonDown(*d.a, ml, true); else DrawPerson(*d.a, ml);
        } break;
        case 3: {
            Look dl{ Color{ 60, 90, 160, 255 }, Color{ 60, 50, 40, 255 }, Color{ 110, 70, 40, 255 }, Color{ 230, 190, 160, 255 }, 1.2f, false, true };
            if (story.dadOnFloor && scene != SceneMode::DadRuns) DrawPersonDown(*d.a, dl, false); else DrawPerson(*d.a, dl);
        } break;
        case 4: if (story.doggoDead && doggoShownDead) DrawDoggoDead(*d.a); else DrawDoggo(*d.a); break;
        case 5: DrawPerson(*d.a, { Color{ 25, 25, 28, 255 }, Color{ 25, 25, 28, 255 }, Color{ 20, 20, 20, 255 }, Color{ 210, 180, 160, 255 }, 1.55f, false, false }); break;
        }
    }
    if (story.CarHere()) DrawCar();

    DrawRainAndDust(cam, std::min(GetFrameTime(), 0.05f));
    // (bubbles and the E prompt are drawn in UI space with the soft font, see DrawBubbles)
    EndMode2D();

    // v0.5 light: the light map (built before this pass) multiplied over the world
    BeginBlendMode(BLEND_MULTIPLIED);
    DrawTextureRec(lightRT.texture, { 0, 0, (float)VW, -(float)VH }, { 0, 0 }, WHITE);
    EndBlendMode();
    // soft vignette
    for (int k = 0; k < 10; ++k) DrawRectangleLines(k, k, VW - 2 * k, VH - 2 * k, Fade(BLACK, 0.035f * (10 - k)));
    if (scene == SceneMode::Chase) {
        float p = 0.5f + 0.5f * std::sin(globalT * 6);
        for (int k = 0; k < 18; ++k) {
            float a = (18 - k) / 18.0f * (0.18f + 0.12f * p);
            DrawRectangleLines(k, k, VW - 2 * k, VH - 2 * k, Fade(Color{ 160, 0, 0, 255 }, a));
        }
    }
    if (story.cold > 0) {
        // v0.8: the frost closes in
        for (int k = 0; k < 26; ++k) DrawRectangleLines(k, k, VW - 2 * k, VH - 2 * k, Fade(Color{ 220, 236, 255, 255 }, (26 - k) / 26.0f * 0.5f * story.cold));
    }
    if (flux) {
        // reality struggling to keep up
        if (std::fmod(globalT, 23.0f) < 0.1f) DrawRectangle(0, (int)(std::fmod(globalT * 997, (float)VH)), VW, 3, Fade(WHITE, 0.2f));
    }
}

// ============================================================ UI drawing (screen space)
static void Panel(float x, float y, float w, float h, Color bg, Color border) {
    DrawRectangle((int)(x * US), (int)(y * US), (int)(w * US), (int)(h * US), bg);
    DrawRectangleLinesEx({ x * US, y * US, w * US, h * US }, std::max(1.0f, US), border);
}

static void Seg(float x, float y, float s, int digit, Color on) {
    // 7-segment digit, s = segment length (UI units)
    static const unsigned char map[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
    unsigned char m = map[digit % 10];
    float th = s * 0.22f;
    Color off = Fade(on, 0.08f);
    auto H = [&](float xx, float yy, bool lit) { DrawRectangle((int)((xx + th) * US), (int)(yy * US), (int)((s - th) * US), (int)(th * US), lit ? on : off); };
    auto V = [&](float xx, float yy, bool lit) { DrawRectangle((int)(xx * US), (int)((yy + th) * US), (int)(th * US), (int)((s - th) * US), lit ? on : off); };
    H(x, y, m & 1); V(x + s, y, m & 2); V(x + s, y + s, m & 4); H(x, y + 2 * s, m & 8);
    V(x, y + s, m & 16); V(x, y, m & 32); H(x, y + s, m & 64);
}

// v0.7.2: the Origin Model 06. The kid's watch and the series mark. "HH:MM" big, ":SS" small.
static float WatchWidth(float s) { return s * 6.6f + s * 0.35f + s * 0.62f * 3.0f + 22; }
static void DrawWatch(const std::string& clock, float x, float y, float s) {
    // x, y = top-left of the first big digit
    bool hasSec = clock.size() >= 8;
    float w = hasSec ? WatchWidth(s) : s * 6.6f + 22;
    float bx = x - 9, by = y - 11, bh = s * 2 + 22;
    // case: blue-black, cold, with the origin mark (a ring with a dot in it) next to the name
    Panel(bx, by, w, bh, Color{ 20, 26, 34, 255 }, Color{ 74, 88, 104, 255 });
    DrawRectangle((int)((bx + 2) * US), (int)((by + 2) * US), (int)((w - 4) * US), (int)(1 * US), Fade(WHITE, 0.08f));
    Color ice = { 196, 216, 228, 255 };
    Vector2 mark = { (bx + 7.5f) * US, (by + 4.8f) * US };
    DrawRing(mark, 1.8f * US, 2.3f * US, 0, 360, 32, ice);
    DrawCircleV(mark, 0.8f * US, Color{ 214, 70, 56, 255 });
    UIText("ORIGIN", bx + 11.5f, by + 2, 5, ice);
    const char* sub = "MODEL 06";
    UIText(sub, bx + w - 5 - UIMeasure(sub, 3.2f), by + 3.4f, 3.2f, Color{ 136, 150, 164, 255 });
    // LCD: colder than a normal watch, ice blue instead of green
    float lx = x - 4, ly = y - 3, lw = w - 10, lh = s * 2 + 7;
    DrawRectangle((int)(lx * US), (int)(ly * US), (int)(lw * US), (int)(lh * US), Color{ 150, 178, 186, 255 });
    DrawRectangle((int)(lx * US), (int)(ly * US), (int)(lw * US), (int)(std::max(1.0f, 1.2f * US)), Fade(BLACK, 0.18f));
    Color on = { 22, 32, 44, 255 };
    float cx = x;
    size_t bigEnd = hasSec ? 5 : clock.size();
    for (size_t i = 0; i < bigEnd; ++i) {
        char ch = clock[i];
        if (ch == ':') {
            if (((int)(globalT * 2)) % 2 == 0) {
                DrawRectangle((int)((cx + s * 0.1f) * US), (int)((y + s * 0.5f) * US), (int)(s * 0.22f * US), (int)(s * 0.22f * US), on);
                DrawRectangle((int)((cx + s * 0.1f) * US), (int)((y + s * 1.3f) * US), (int)(s * 0.22f * US), (int)(s * 0.22f * US), on);
            }
            cx += s * 0.6f;
        } else { Seg(cx, y, s, ch - '0', on); cx += s * 1.5f; }
    }
    if (hasSec) {
        float ss = s * 0.62f;
        cx += s * 0.35f;
        float sy = y + 2 * s - 2 * ss;   // bottom aligned, like the real watch
        for (size_t i = 6; i < 8 && i < clock.size(); ++i) { Seg(cx, sy, ss, clock[i] - '0', on); cx += ss * 1.5f; }
    }
}

static Color NarratorColor() {
    int st = story.Stage();
    if (st == 0) return Color{ 245, 230, 200, 255 };
    if (st == 1) return Color{ 200, 210, 225, 255 };
    if (st == 2) return Color{ 200, 60, 60, 255 };
    if (st == 3) return Color{ 170, 25, 30, 255 };
    return Color{ 235, 235, 235, 255 };
}

static void DrawNarrator(const std::string& text, float shown) {
    DrawRectangle(0, 0, SW, SH, story.Stage() >= 4 ? Fade(Color{ 70, 0, 6, 255 }, 0.85f) : Fade(BLACK, 0.72f));
    if (curSeen && !touchMode) UIText("TAB: skip", VW - 46, VH - 14, 6, Fade(WHITE, 0.4f));
    std::string vis = text.substr(0, std::min<size_t>(text.size(), (size_t)shown));
    float size = 15;
    auto lines = Wrap(text, 360, size, true);
    float y = VH / 2.0f - lines.size() * size * 0.65f;
    size_t used = 0;
    Color c = NarratorColor();
    for (auto& l : lines) {
        std::string part = vis.size() > used ? vis.substr(used, l.size()) : "";
        used += l.size() + 1;
        float w = hasNarratorFont ? MeasureTextEx(narratorFont, l.c_str(), size * US, 1).x / US : UIMeasure(l.c_str(), size);
        float x = VW / 2.0f - w / 2;
        float jx = story.Stage() >= 2 ? (float)GetRandomValue(-1, 1) * 0.3f : 0;
        if (hasNarratorFont) DrawTextEx(narratorFont, part.c_str(), { (x + jx) * US, y * US }, size * US, 1, c);
        else UIText(part.c_str(), x, y, size * 0.8f, c);
        y += size * 1.3f;
    }
    UIText("NARRATOR", VW / 2.0f - UIMeasure("NARRATOR", 6) / 2, VH - 30, 6, Fade(c, 0.5f));
}

static void DrawDialogue() {
    float x = 12, y = VH - 74, w = VW - 24, h = 64;
    Panel(x, y, w, h, Color{ 16, 16, 28, 235 }, Color{ 230, 230, 240, 255 });
    std::string vis = cur.text.substr(0, std::min<size_t>(cur.text.size(), (size_t)typed));
    if (!cur.speaker.empty()) {
        float nw = UIMeasure(cur.speaker.c_str(), 10) + 12;
        Color nc = cur.speaker == "The visitor" ? Color{ 120, 20, 20, 255 } : cur.speaker == "You" ? Color{ 160, 50, 50, 255 } : Color{ 50, 70, 120, 255 };
        Panel(x + 8, y - 12, nw, 15, nc, WHITE);
        UIText(cur.speaker.c_str(), x + 14, y - 9, 10, WHITE);
    }
    auto lines = Wrap(cur.text, w - 20, 10, false);
    size_t used = 0; float ly = y + 10;
    for (auto& l : lines) {
        std::string part = vis.size() > used ? vis.substr(used, l.size()) : "";
        used += l.size() + 1;
        if (story.Stage() >= 4 && cur.speaker == "You" && hasNarratorFont)
            DrawTextEx(narratorFont, part.c_str(), { (x + 10) * US, (ly - 2) * US }, 13 * US, 1, Color{ 225, 120, 120, 255 });
        else UIText(part.c_str(), x + 10, ly, 10, WHITE);
        ly += 13;
    }
    if (typed >= cur.text.size() && ((int)(globalT * 3)) % 2 == 0) UIText("v", x + w - 14, y + h - 12, 10, YELLOW);
    if (story.Stage() >= 1) UIText("TAB: skip talk", x + w - 76, y + h + 2, 6, Fade(WHITE, 0.5f));
}

static void DrawBlack() {
    DrawRectangle(0, 0, SW, SH, BLACK);
    std::string vis = cur.text.substr(0, std::min<size_t>(cur.text.size(), (size_t)typed));
    auto lines = Wrap(cur.text, 380, 10, false);
    float y = VH / 2.0f - lines.size() * 7.0f;
    size_t used = 0;
    for (auto& l : lines) {
        std::string part = vis.size() > used ? vis.substr(used, l.size()) : "";
        used += l.size() + 1;
        UIText(part.c_str(), VW / 2.0f - UIMeasure(l.c_str(), 10) / 2, y, 10, Color{ 220, 220, 220, 255 });
        y += 14;
    }
    if (curSeen && !touchMode) UIText("TAB: skip", VW - 46, VH - 14, 6, Fade(WHITE, 0.4f));
}

static void DrawChoice() {
    float w = 300, lineH = 16;
    float h = 26 + cur.choices.size() * lineH;
    float x = VW / 2.0f - w / 2, y = VH - h - 14;
    Panel(x, y, w, h, Color{ 16, 16, 28, 240 }, Color{ 230, 230, 240, 255 });
    UIText(cur.text.c_str(), x + 10, y + 7, 8, Color{ 180, 180, 200, 255 });
    for (size_t i = 0; i < cur.choices.size(); ++i) {
        auto& c = cur.choices[i];
        float ly = y + 22 + i * lineH;
        bool sel = (int)i == choiceSel;
        if (sel) DrawRectangle((int)((x + 4) * US), (int)((ly - 3) * US), (int)((w - 8) * US), (int)(lineH * US), Color{ 60, 60, 100, 255 });
        float tx = x + 14;
        if (c.memory) {
            // memory icon: a small diamond
            Vector2 cc = { (x + 14) * US, (ly + 4) * US };
            float r = 4 * US;
            DrawTriangle({ cc.x, cc.y - r }, { cc.x - r, cc.y }, { cc.x + r, cc.y }, Color{ 190, 150, 255, 255 });
            DrawTriangle({ cc.x - r, cc.y }, { cc.x, cc.y + r }, { cc.x + r, cc.y }, Color{ 190, 150, 255, 255 });
            tx += 10;
        }
        UIText(c.text.c_str(), tx, ly, 10, c.dark ? Color{ 235, 80, 80, 255 } : c.memory ? Color{ 215, 195, 255, 255 } : WHITE);
    }
}

static void DrawFade() {
    bool quick = cur.minutes == 0 && cur.text.empty();
    float tIn = quick ? 0.25f : 0.35f, hold = quick ? 0.05f : 1.1f;
    float a;
    if (beatTime < tIn) a = beatTime / tIn;
    else if (beatTime < tIn + hold) a = 1;
    else a = 1 - (beatTime - tIn - hold) / tIn;
    a = std::max(0.0f, std::min(1.0f, a));
    DrawRectangle(0, 0, SW, SH, Fade(BLACK, a));
    if (!quick && beatTime >= tIn && beatTime < tIn + hold) {
        float k = std::min(1.0f, (beatTime - tIn) / 0.7f);
        // v0.5: the watch rolls forward through the skipped time instead of flickering
        float e = 1 - (1 - k) * (1 - k) * (1 - k);
        int tt = (int)(fadeFromT + (fadeToT - fadeFromT) * e);
        char cb[16]; std::snprintf(cb, sizeof cb, "%02d:%02d:%02d", 16 + tt / 3600, (tt / 60) % 60, tt % 60);
        std::string clock = cb;
        DrawWatch(clock, VW / 2.0f - WatchWidth(9) / 2 + 9, VH / 2.0f - 26, 9);
        if (!cur.text.empty()) UIText(cur.text.c_str(), VW / 2.0f - UIMeasure(cur.text.c_str(), 10) / 2, VH / 2.0f + 14, 10, Color{ 220, 220, 220, 255 });
        if (cur.minutes > 0) {
            std::string plus = "+" + std::to_string(cur.minutes) + " min";
            UIText(plus.c_str(), VW / 2.0f - UIMeasure(plus.c_str(), 8) / 2, VH / 2.0f + 30, 8, Color{ 255, 200, 90, 255 });
        }
    }
}

static void DrawCard() {
    DrawRectangle(0, 0, SW, SH, Color{ 8, 6, 10, 255 });
    UIText(cur.speaker.c_str(), VW / 2.0f - UIMeasure(cur.speaker.c_str(), 20) / 2, 70, 20, Color{ 200, 60, 60, 255 });
    auto lines = Wrap(cur.text, 360, 10, false);
    float y = 110;
    for (auto& l : lines) { UIText(l.c_str(), VW / 2.0f - UIMeasure(l.c_str(), 10) / 2, y, 10, WHITE); y += 14; }
}

static void DrawQTE() {
    DrawRectangle(0, 0, SW, SH, BLACK);
    float shake = (float)GetRandomValue(-2, 2) * qteBar;
    const char* t = "STAY AWAKE";
    UIText(t, VW / 2.0f - UIMeasure(t, 24) / 2 + shake, 90, 24, Color{ 230, 60, 60, 255 });
    const char* h = "MASH  E";
    UIText(h, VW / 2.0f - UIMeasure(h, 10) / 2, 130, 10, Fade(WHITE, 0.5f + 0.5f * std::sin(globalT * 12)));
    Panel(VW / 2.0f - 100, 155, 200, 14, Color{ 20, 20, 24, 255 }, WHITE);
    DrawRectangle((int)((VW / 2.0f - 98) * US), (int)(157 * US), (int)(196 * std::min(1.0f, qteBar) * US), (int)(10 * US), Color{ 230, 60, 60, 255 });
    float left = std::max(0.0f, 5.0f - beatTime);
    std::string sec = std::to_string((int)std::ceil(left));
    UIText(sec.c_str(), VW / 2.0f - 4, 180, 12, GRAY);
}

static Vector2 WorldToUI(Vector2 p) {
    return { p.x - worldCam.target.x + VW / 2.0f, p.y - worldCam.target.y + VH / 2.0f };
}
static float HeadHeight(const Actor* a) {
    if (a == &visitor) return 34;
    if (a == &doggo) return 14;
    if (a == &player) return 24;
    return 30;
}
static void DrawBubbles(float dt) {
    for (size_t i = 0; i < bubbles.size();) {
        auto& b = bubbles[i];
        b.t += dt;
        if (b.t >= b.life) { bubbles.erase(bubbles.begin() + i); continue; }
        ++i;
        Actor* a = ActorFor(b.who);
        if (!a || !a->visible || (a == &player && hidden)) continue;
        Vector2 h = WorldToUI(a->pos);
        h.y -= HeadHeight(a);
        if (h.x < -20 || h.x > VW + 20 || h.y < -10 || h.y > VH + 40) continue;
        float alpha = std::min(1.0f, std::min(b.t / 0.15f, (b.life - b.t) / 0.3f));
        const float size = 6.5f, maxW = 120;
        auto lines = Wrap(b.text, maxW, size, false);
        float w = 0;
        for (auto& l : lines) w = std::max(w, UIMeasure(l.c_str(), size));
        float lh = size * 1.25f, bw = w + 10, bh = lines.size() * lh + 6;
        float lift = (a == &player && !beatActive && !FindTarget().id.empty()) ? 14.0f : 0.0f;
        float bx = std::max(4.0f, std::min(VW - 4 - bw, h.x - bw / 2)), by = std::max(4.0f, h.y - bh - 5 - lift);
        bool thought = b.who == "You";
        bool scary = b.who == "The visitor";
        Color bg = thought ? Color{ 222, 232, 250, 255 } : scary ? Color{ 30, 22, 26, 255 } : Color{ 252, 249, 240, 255 };
        Color fg = scary ? Color{ 240, 210, 210, 255 } : Color{ 30, 34, 52, 255 };
        Color edge = scary ? Color{ 150, 40, 40, 255 } : Color{ 40, 44, 66, 255 };
        Rectangle r{ bx * US, by * US, bw * US, bh * US };
        // tail
        float tx = std::max(bx + 6, std::min(bx + bw - 6, h.x));
        if (lift == 0) DrawTriangle({ (tx - 3) * US, (by + bh - 0.5f) * US }, { tx * US, (h.y - 1) * US }, { (tx + 3) * US, (by + bh - 0.5f) * US }, Fade(bg, alpha));
        DrawRectangleRounded(r, 0.35f, 8, Fade(bg, alpha * 0.96f));
        DrawRectangleRoundedLinesEx(r, 0.35f, 8, std::max(1.0f, US * 0.6f), Fade(edge, alpha * 0.8f));
        for (size_t k = 0; k < lines.size(); ++k) UIText(lines[k].c_str(), bx + 5, by + 3 + k * lh, size, Fade(fg, alpha));
    }
    // the visitor's mood, over his head
    if (scene == SceneMode::Chase && visitor.visible && !beatActive && downedT <= 0) {
        const char* mark = vmode == VMode::Chase ? "!" : vmode == VMode::Lost ? "?" : nullptr;
        if (mark) {
            Vector2 h = WorldToUI(visitor.pos);
            UIText(mark, h.x - UIMeasure(mark, 12) / 2, h.y - HeadHeight(&visitor) - 12, 12, vmode == VMode::Chase ? Color{ 240, 60, 60, 255 } : Color{ 240, 200, 90, 255 });
        }
    }
    // interaction prompt
    if (!beatActive && !journalOpen && !hidden) {
        Target tg = FindTarget();
        if (!tg.id.empty()) {
            Vector2 h = WorldToUI(player.pos);
            const float size = 6.5f;
            float w = UIMeasure(tg.name.c_str(), size);
            float bw = w + 18, bx = std::max(4.0f, std::min(VW - 4 - bw, h.x - bw / 2)), by = h.y - 36;
            DrawRectangleRounded({ bx * US, by * US, bw * US, 11 * US }, 0.5f, 8, Fade(BLACK, 0.72f));
            DrawRectangleRounded({ (bx + 2) * US, (by + 1.5f) * US, 8 * US, 8 * US }, 0.3f, 6, Color{ 245, 200, 80, 255 });
            UIText("E", bx + 3.6f, by + 2.2f, 6.5f, BLACK);
            UIText(tg.name.c_str(), bx + 13, by + 2.2f, size, WHITE);
        }
    }
}

static void DrawChaseBanner() {
    if (scene == SceneMode::None) return;
    const char* t = scene == SceneMode::DadRuns ? "DAD IS FIGHTING HIM. RUN." : hidden ? "HIDING... (E to come out)" :
                    story.threats >= 3 ? "HE'S DONE WAITING" : story.threats > 0 ? "HE'S HURTING THEM" : "THE LAST 600 SECONDS";
    float p = 0.5f + 0.5f * std::sin(globalT * 5);
    UIText(t, VW / 2.0f - UIMeasure(t, 10) / 2, 40, 10, Fade(Color{ 240, 70, 70, 255 }, 0.6f + 0.4f * p));
    if (scene == SceneMode::Chase) {
        const char* h = "";
        if (story.visitorDowned) h = "He's down. For now.";
        else if (hidden) {
            bool near = vmode == VMode::GoToHide || (vmode == VMode::CheckSpot && vSpotObj >= 0 &&
                        story.world.RoomAt(story.world.objs[vSpotObj].x, story.world.objs[vSpotObj].y) == story.world.RoomAt(TX(hidePos.x), TX(hidePos.y)));
            h = near ? "Don't. Breathe." : "Stay still. Listen.";
        }
        else if (vmode == VMode::Grace) h = "He's coming back in. Get out of sight.";
        else if (vmode == VMode::Chase) h = "HE SEES YOU. Run (Shift). Break line of sight.";
        else if (vmode == VMode::Lost) h = "He lost you. Keep moving.";
        else if (story.threats > 0) h = "Come out and face him (E). Or don't.";
        else h = "He's searching room by room. Hide (E), sneak past, or face him (E).";
        if (story.cold > 0.3f) h = story.hasCoat ? "It's getting into your jacket. Get inside." : "You're freezing. Get inside.";
        else if (story.doggoWithKid && story.Knows("doggo_stay") && !hidden) h = "Doggo is right next to you. Tell him to stay (E on Doggo).";
        UIText(h, VW / 2.0f - UIMeasure(h, 6) / 2, 54, 6, Fade(WHITE, 0.65f));
    }
}

static void DrawToasts(float dt) {
    float y = 8;
    for (size_t i = 0; i < toasts.size();) {
        auto& t = toasts[i];
        t.t -= dt;
        if (t.t <= 0) { toasts.erase(toasts.begin() + i); continue; }
        float a = std::min(1.0f, t.t * 2);
        float w = std::max(UIMeasure(t.text.c_str(), 10), UIMeasure(t.title.c_str(), 6)) + 20;
        float x = VW / 2.0f - w / 2;
        Panel(x, y, w, 26, Fade(Color{ 10, 10, 20, 255 }, 0.9f * a), Fade(t.c, a));
        UIText(t.title.c_str(), x + 10, y + 3, 6, Fade(t.c, a));
        UIText(t.text.c_str(), x + 10, y + 12, 10, Fade(WHITE, a));
        y += 30;
        ++i;
    }
}

static void DrawHUD() {
    // Watch
    DrawWatch(story.ClockTextSec(), VW - 8 - WatchWidth(8) + 9, 16, 8);
    // Room + loop
    Room r = story.world.RoomAt(TX(player.pos.x), TX(player.pos.y));
    Panel(6, 6, UIMeasure(RoomName(r), 10) + 12, 15, Fade(BLACK, 0.6f), Fade(WHITE, 0.3f));
    UIText(RoomName(r), 12, 9, 10, WHITE);
    std::string lp = "LOOP " + std::to_string(story.loop);
    UIText(lp.c_str(), 8, 25, 6, Fade(WHITE, 0.6f));
    std::string ch = "CHORES " + std::to_string(story.ChoresDone()) + "/" + std::to_string(Story::CHORES) + (story.earnedPresent ? "  PRESENT!" : "");
    UIText(ch.c_str(), VW - 84, 46, 6, Fade(WHITE, 0.7f));
    std::string mem = "MEMORY " + std::to_string(story.Awareness()) + "/" + std::to_string(AllMemories().size()) + "  [J]";
    UIText(mem.c_str(), VW - 84, 54, 6, Color{ 190, 150, 255, 200 });
    float iy = 62;
    {
        // v0.6: one thing in your hands
        std::string hand = std::string("IN HANDS: ") + (story.held == Item::None ? "nothing" : ItemName(story.held));
        if (story.doggoStay && story.DoggoExists()) hand += "   DOGGO: STAYING";
        Color hc = story.hasKnife ? Color{ 235, 80, 80, 255 } : story.hasKey ? GOLD : story.held == Item::None ? Fade(WHITE, 0.45f) : Color{ 200, 230, 255, 255 };
        UIText(hand.c_str(), VW - 84, iy, 6, hc); iy += 8;
    }
    if (story.tainted) { std::string dk = "DARK " + std::to_string(story.darkPoints); UIText(dk.c_str(), 8, 33, 6, Color{ 200, 40, 40, 220 }); }
    if (debugMode) {
        std::string rs = std::string("DEBUG  reality: ") + ShiftName(story.shift) + "  streak " + std::to_string(story.cleanStreak) + "  [O]";
        if (visitorBlind) rs += "  VISITOR BLIND";
        UIText(rs.c_str(), 8, VH - 22, 6, Color{ 120, 220, 140, 200 });
    }
    DrawChaseBanner();
    if (IsKeyDown(KEY_T)) UIText(">> waiting", VW / 2.0f - 25, VH - 14, 8, YELLOW);
    else if (!touchMode) UIText("T: wait   J: memory   E: use", 8, VH - 12, 6, Fade(WHITE, 0.45f));
}

static void DrawJournal() {
    DrawRectangle(0, 0, SW, SH, Fade(Color{ 10, 8, 14, 255 }, 0.94f));
    bool dark = story.Stage() >= 2;
    UIText("MEMORY LOG", 16, 12, 14, dark ? Color{ 200, 60, 60, 255 } : Color{ 190, 150, 255, 255 });
    std::string pct = std::to_string(story.Awareness() * 100 / AllMemories().size()) + "%   endings " + std::to_string(story.endings.size()) + "/" + std::to_string(AllEndings().size());
    UIText(pct.c_str(), VW - 16 - UIMeasure(pct.c_str(), 9), 15, 9, WHITE);
    auto& topics = AllTopics();
    int nTabs = (int)topics.size() + 1;   // v0.6: + Endings
    float tx = 16;
    for (int i = 0; i < nTabs; ++i) {
        bool isEnd = i == (int)topics.size();
        const char* name = isEnd ? "Endings" : topics[i].c_str();
        float w = UIMeasure(name, 7) + 8;
        bool sel = i == journalTopic;
        int found = 0;
        if (isEnd) found = (int)story.endings.size();
        else for (auto& e : AllMemories()) if (e.topic == topics[i] && story.Knows(e.id)) found++;
        Color edge = isEnd ? (found ? GOLD : Color{ 80, 80, 90, 255 }) : (found ? Color{ 190, 150, 255, 255 } : Color{ 80, 80, 90, 255 });
        Panel(tx, 32, w, 13, sel ? (isEnd ? Color{ 90, 74, 30, 255 } : Color{ 70, 50, 110, 255 }) : Color{ 25, 22, 35, 255 }, edge);
        UIText(name, tx + 4, 35, 7, found ? WHITE : GRAY);
        tx += w + 3;
    }
    // entries for this tab, scrollable (v0.6)
    struct Row { bool known; std::string title, text, hint; bool ending; };
    std::vector<Row> rows;
    if (journalTopic == (int)topics.size()) {
        for (auto& e : AllEndings()) rows.push_back({ story.endings.count(e.id) > 0, e.title, "Found.", e.hint, true });
    } else {
        for (auto& e : AllMemories()) if (e.topic == topics[journalTopic]) rows.push_back({ story.Knows(e.id), e.title, e.text, e.hint, false });
    }
    const float top = 52, rowH = 42, bottom = VH - 22;
    int visible = (int)((bottom - top) / rowH);
    int maxScroll = std::max(0, (int)rows.size() - visible);
    journalScroll = std::max(0, std::min(journalScroll, maxScroll));
    float y = top;
    for (int idx = journalScroll; idx < (int)rows.size() && idx < journalScroll + visible; ++idx) {
        auto& r = rows[idx];
        bool adult = r.known && dark && !r.ending;
        Color fill = r.known ? (r.ending ? Color{ 40, 34, 18, 255 } : Color{ 30, 26, 40, 255 }) : Color{ 18, 18, 22, 255 };
        Color edge = r.known ? (r.ending ? GOLD : Color{ 120, 100, 170, 255 }) : Color{ 50, 50, 60, 255 };
        Panel(16, y, VW - 32, rowH - 4, fill, edge);
        if (r.known) {
            UIText(r.title.c_str(), 24, y + 4, 10, r.ending ? GOLD : WHITE);
            auto lines = Wrap(r.ending ? std::string("You reached this ending.") : r.text, VW - 52, adult ? 11 : 8, adult);
            float ly = y + 17;
            for (size_t i = 0; i < lines.size() && i < 2; ++i) {
                if (adult && hasNarratorFont) DrawTextEx(narratorFont, lines[i].c_str(), { 24 * US, ly * US }, 11 * US, 1, Color{ 220, 170, 170, 255 });
                else UIText(lines[i].c_str(), 24, ly, 8, Color{ 210, 210, 220, 255 });
                ly += 10;
            }
        } else {
            UIText("?", 24, y + 6, 18, Color{ 80, 80, 90, 255 });
            UIText(r.hint.c_str(), 40, y + 14, 8, Color{ 110, 110, 125, 255 });
        }
        y += rowH;
    }
    if (journalScroll > 0) UIText("^ more", VW - 60, top - 9, 7, Fade(WHITE, 0.6f));
    if (journalScroll < maxScroll) UIText("v more", VW - 60, bottom - 2, 7, Fade(WHITE, 0.6f));
    const char* help = touchMode ? "D-pad: topic / scroll    tap: close" : "LEFT/RIGHT topic    UP/DOWN scroll    J or ESC close";
    UIText(help, touchMode ? 120 : 16, VH - 14, 7, Fade(WHITE, 0.5f));
}

// ============================================================ v0.6 loop recap (Outer Wilds style)
static void UpdateRecap(float dt) {
    recapT += dt;
    int want = std::min((int)recapList.size(), (int)((recapT - 0.6f) / 0.85f) + 1);
    if (recapT < 0.6f) want = 0;
    while (recapShown < want) { recapShown++; Play("chime"); }
    bool allShown = recapShown >= (int)recapList.size();
    if ((IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && recapT > 0.4f) {
        if (!allShown) { recapShown = (int)recapList.size(); recapT = 0.6f + 0.85f * recapShown; }
        else { recapOpen = false; interactCooldown = 0.4f; }
    }
}
static void DrawRecap() {
    DrawRectangle(0, 0, SW, SH, Color{ 6, 6, 12, 245 });
    float a = std::min(1.0f, recapT / 0.5f);
    const char* h = "WHAT I REMEMBER";
    UIText(h, VW / 2.0f - UIMeasure(h, 14) / 2, 22, 14, Fade(Color{ 190, 150, 255, 255 }, a));
    std::string sub = "Loop " + std::to_string(story.loop - 1) + " is over. This stays with me.";
    UIText(sub.c_str(), VW / 2.0f - UIMeasure(sub.c_str(), 7) / 2, 40, 7, Fade(WHITE, 0.55f * a));
    float y = 58;
    int start = std::max(0, recapShown - 5);
    for (int i = start; i < recapShown; ++i) {
        const MemoryEntry* e = nullptr;
        for (auto& m : AllMemories()) if (m.id == recapList[i]) e = &m;
        if (!e) continue;
        float age = recapT - (0.6f + 0.85f * i);
        float k = std::max(0.0f, std::min(1.0f, age / 0.35f));
        float slide = (1 - k) * 12;
        Panel(70 + slide, y, VW - 140, 30, Fade(Color{ 30, 26, 44, 255 }, k), Fade(Color{ 190, 150, 255, 255 }, k));
        UIText(e->topic.c_str(), 78 + slide, y + 4, 6, Fade(Color{ 160, 140, 200, 255 }, k));
        UIText(e->title.c_str(), 78 + slide, y + 13, 10, Fade(WHITE, k));
        y += 34;
    }
    bool allShown = recapShown >= (int)recapList.size();
    const char* f = allShown ? "E / tap: wake up" : "E / tap: show all";
    UIText(f, VW / 2.0f - UIMeasure(f, 8) / 2, VH - 22, 8, Fade(WHITE, 0.4f + 0.3f * std::sin(globalT * 3)));
}

// ============================================================ v0.6 Fliptendo easter egg: Flappy Plane
static void FinishMini() {
    miniOpen = false;
    interactCooldown = 0.4f;
    story.MinigameDone(miniScore);
    StartNextBeat();
}
static void UpdateMini(float dt) {
    bool flap = IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (IsKeyPressed(KEY_ESCAPE)) { FinishMini(); return; }
    if (miniDead) {
        miniDeadT += dt;
        if (miniDeadT > 0.8f && flap) FinishMini();
        return;
    }
    if (!miniStarted) { if (flap) { miniStarted = true; miniV = -110; Play("flap"); } return; }
    miniT += dt;
    if (flap) { miniV = -110; Play("flap"); }
    miniV += 330 * dt;
    miniY += miniV * dt;
    if (miniPipes.empty() || miniPipes.back().x < MINI_W - 78) miniPipes.push_back({ MINI_W + 10, 30 + (float)GetRandomValue(0, 60), false });
    for (auto& p : miniPipes) {
        p.x -= 62 * dt;
        if (!p.scored && p.x + 14 < 40) { p.scored = true; miniScore++; Play("point"); }
        bool inX = 40 + 5 > p.x && 40 - 5 < p.x + 14;
        if (inX && (miniY - 4 < p.gapY - 22 || miniY + 4 > p.gapY + 22)) { miniDead = true; Play("thud"); }
    }
    miniPipes.erase(std::remove_if(miniPipes.begin(), miniPipes.end(), [](const MiniPipe& p) { return p.x < -20; }), miniPipes.end());
    if (miniY < 4) { miniY = 4; if (miniV < 0) miniV = 0; }   // the ceiling is soft, the floor is not
    if (miniY > MINI_H - 6) { miniDead = true; Play("thud"); }
}
static void DrawMini() {
    DrawRectangle(0, 0, SW, SH, Fade(BLACK, 0.7f));
    float dw = MINI_W + 60, dh = MINI_H + 44;
    float dx = VW / 2.0f - dw / 2, dy = VH / 2.0f - dh / 2;
    // the Fliptendo Fold 2
    Panel(dx, dy, dw, dh, Color{ 40, 40, 48, 255 }, Color{ 90, 90, 104, 255 });
    DrawCircle((int)((dx + 15) * US), (int)((dy + dh / 2) * US), 7 * US, Color{ 70, 70, 80, 255 });
    DrawCircle((int)((dx + dw - 15) * US), (int)((dy + dh / 2 - 6) * US), 4 * US, Color{ 200, 70, 80, 255 });
    DrawCircle((int)((dx + dw - 22) * US), (int)((dy + dh / 2 + 4) * US), 4 * US, Color{ 70, 160, 220, 255 });
    UIText("FLIPTENDO FOLD 2", dx + dw / 2 - UIMeasure("FLIPTENDO FOLD 2", 6) / 2, dy + dh - 13, 6, Color{ 170, 170, 186, 255 });
    float sx = dx + 30, sy = dy + 10;
    auto R = [&](float x, float y, float w, float h, Color c) {
        float x0 = std::max(0.0f, x), x1 = std::min(MINI_W, x + w);
        if (x1 <= x0) return;
        DrawRectangle((int)((sx + x0) * US), (int)((sy + y) * US), (int)std::ceil((x1 - x0) * US), (int)std::ceil(h * US), c);
    };
    R(0, 0, MINI_W, MINI_H, Color{ 255, 214, 170, 255 });
    R(0, MINI_H * 0.6f, MINI_W, MINI_H * 0.4f, Color{ 250, 190, 160, 255 });
    for (int k = 0; k < 6; ++k) { float bx = std::fmod(k * 40 - miniT * 15, MINI_W + 40) - 20; if (bx < -20) bx += MINI_W + 40; R(bx, MINI_H - 30 - (k % 3) * 8, 22, 30 + (k % 3) * 8, Color{ 230, 160, 170, 255 }); }
    for (auto& p : miniPipes) {
        R(p.x, 0, 14, p.gapY - 22, Color{ 170, 80, 60, 255 }); R(p.x - 1, p.gapY - 26, 16, 4, Color{ 140, 60, 44, 255 });
        R(p.x, p.gapY + 22, 14, MINI_H - p.gapY - 22, Color{ 170, 80, 60, 255 }); R(p.x - 1, p.gapY + 22, 16, 4, Color{ 140, 60, 44, 255 });
    }
    // the paper plane
    float tilt = std::max(-3.0f, std::min(3.0f, miniV / 50));
    DrawTriangle({ (sx + 46) * US, (sy + miniY) * US }, { (sx + 34) * US, (sy + miniY - 4 - tilt) * US }, { (sx + 36) * US, (sy + miniY + 4 - tilt) * US }, WHITE);
    std::string sc = std::to_string(miniScore);
    UIText(sc.c_str(), sx + MINI_W / 2 - UIMeasure(sc.c_str(), 12) / 2, sy + 4, 12, Color{ 90, 50, 60, 255 });
    if (!miniStarted) UIText("FLAPPY PLANE  -  E / tap to flap", sx + 18, sy + MINI_H / 2 - 4, 8, Color{ 90, 50, 60, 255 });
    if (miniDead) {
        const char* go = miniDeadT > 0.8f ? "CRASHED  -  E: put it down" : "CRASHED";
        UIText(go, sx + MINI_W / 2 - UIMeasure(go, 9) / 2, sy + MINI_H / 2 - 4, 9, Color{ 120, 30, 40, 255 });
    }
}

// ============================================================ splash
static void DrawAIBadge(float x, float y, Color fg) {
    const char* t = "MADE WITH AI";
    float w = UIMeasure(t, 6) + 10;
    DrawRectangleLinesEx({ x * US, y * US, w * US, 11 * US }, std::max(1.0f, US * 0.5f), fg);
    UIText(t, x + 5, y + 2.5f, 6, fg);
}

static void DrawSplash() {
    float t = splashT;
    float in = std::min(1.0f, t / 1.0f);
    float out = t > 3.4f ? std::max(0.0f, 1 - (t - 3.4f) / 1.3f) : 1.0f;
    out = out * out * (3 - 2 * out);   // ease the dissolve
    Color bg = { 255, 255, 255, 255 };
    DrawRectangle(0, 0, (int)(VW * US), (int)(VH * US), Fade(bg, out));
    float a = in * out;
    if (hasLogo) {
        // logo.png is 2048x1024 (power of two for mipmaps); the art sits in the middle 1478 px
        Rectangle srcR = logoTex.width == 2048 ? Rectangle{ 285, 0, 1478, 1024 } : Rectangle{ 0, 0, (float)logoTex.width, (float)logoTex.height };
        float h = 150, w = h * srcR.width / srcR.height;
        DrawTexturePro(logoTex, srcR,
                       { (VW / 2.0f - w / 2) * US, 40 * US, w * US, h * US }, { 0, 0 }, 0, Fade(WHITE, a));
    } else {
        const char* n = "SUND ORIGIN STUDIO";
        UIText(n, VW / 2.0f - UIMeasure(n, 16) / 2, 110, 16, Fade(Color{ 28, 45, 85, 255 }, a));
    }
    const char* p = "presents a prototype";
    UIText(p, VW / 2.0f - UIMeasure(p, 7) / 2, 200, 7, Fade(Color{ 90, 95, 110, 255 }, a));
    float bw = UIMeasure("MADE WITH AI", 6) + 10;
    DrawAIBadge(VW / 2.0f - bw / 2, 222, Fade(Color{ 28, 45, 85, 255 }, a));
}

// ============================================================ jukebox (debug)
static void DrawJukebox() {
    DrawRectangle(0, 0, (int)(VW * US), (int)(VH * US), Fade(Color{ 8, 8, 14, 255 }, 0.94f));
    UIText("JUKEBOX", 24, 20, 16, Color{ 230, 60, 60, 255 });
    UIText("debug  -  every song is code in src/audio/Soundtrack.cpp", 24, 40, 6, Fade(WHITE, 0.5f));
    for (int i = 0; i < TRACK_COUNT; ++i) {
        float y = 62 + i * 30;
        bool sel = i == jukeboxSel, playing = i == musicCur;
        if (sel) DrawRectangle((int)(20 * US), (int)((y - 4) * US), (int)((VW - 40) * US), (int)(28 * US), Color{ 50, 40, 80, 255 });
        std::string name = std::string(playing ? "> " : "  ") + TrackName(i);
        UIText(name.c_str(), 28, y, 10, playing ? Color{ 255, 210, 90, 255 } : WHITE);
        UIText(TrackInfo(i), 40, y + 13, 6, Fade(WHITE, 0.55f));
    }
    // a tiny live scope of what's playing
    float sx = VW - 150, sy = 26;
    DrawRectangleLines((int)(sx * US), (int)(sy * US), (int)(126 * US), (int)(24 * US), Fade(WHITE, 0.3f));
    for (int k = 0; k < 125; ++k) {
        double t0 = musicT - (125 - k) * 0.0008;
        float v = TrackSample(musicCur, t0) * 3.0f;
        v = std::max(-1.0f, std::min(1.0f, v));
        DrawRectangle((int)((sx + 1 + k) * US), (int)((sy + 12 - v * 10) * US), (int)std::max(1.0f, US), (int)std::max(1.0f, US), Color{ 120, 220, 140, 255 });
    }
    UIText("UP/DOWN pick    ENTER play    M or ESC close", 24, VH - 16, 7, Fade(WHITE, 0.5f));
}

static bool UpdateJukebox() {
    // returns true while the jukebox owns the input
    if (debugMode && IsKeyPressed(KEY_M)) { jukeboxOpen = !jukeboxOpen; if (jukeboxOpen) jukeboxSel = musicCur; return true; }
    if (!jukeboxOpen) return false;
    if (IsKeyPressed(KEY_ESCAPE)) { jukeboxOpen = false; return true; }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) jukeboxSel = (jukeboxSel + 1) % TRACK_COUNT;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) jukeboxSel = (jukeboxSel + TRACK_COUNT - 1) % TRACK_COUNT;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E)) { musicPrev = musicCur; musicCur = jukeboxSel; musicFade = 0.6f; }
    return true;
}

// ============================================================ debug panel (O with ?debug)
// Live knobs for playtesting without rebuilding. Settings are kept in this browser.
static bool dbgOpen = false;
static int dbgSel = 0;
static bool dbgRestartLoop = false;
enum DbgRow { D_SPEED, D_ON, D_TOILET, D_DOGGO, D_CHANCE, D_MINGAP, D_MAXGAP, D_FIRST, D_SHARE, D_FORCE, D_RESTART,
              D_DARK, D_ALLMEM, D_CLEARMEM, D_BLIND, D_DEFAULTS, D_COUNT };

static void SaveDebugCfg() {
    std::string s = gShiftCfg.Save() + "|" + (visitorBlind ? "1" : "0");
#if defined(PLATFORM_WEB)
    EM_ASM({ try { localStorage.setItem('600s_debug', UTF8ToString($0)); } catch (e) {} }, s.c_str());
#else
    SaveFileText("600s_debug.txt", (char*)s.c_str());
#endif
}
static void LoadDebugCfg() {
    std::string s;
#if defined(PLATFORM_WEB)
    char* p = (char*)EM_ASM_PTR({
        var s = null; try { s = localStorage.getItem('600s_debug'); } catch (e) {}
        if (!s) s = ""; var len = lengthBytesUTF8(s) + 1; var buf = _malloc(len); stringToUTF8(s, buf, len); return buf;
    });
    s = p ? p : ""; free(p);
#else
    if (FileExists("600s_debug.txt")) { char* t = LoadFileText("600s_debug.txt"); s = t ? t : ""; UnloadFileText(t); }
#endif
    gShiftCfg.Load(s);
    visitorBlind = s.size() >= 2 && s.substr(s.size() - 2) == "|1";
}

static std::string DbgValue(int row) {
    const ShiftConfig& c = gShiftCfg;
    auto onoff = [](bool b) { return std::string(b ? "ON" : "OFF"); };
    switch (row) {
    case D_SPEED: { char b[16]; std::snprintf(b, sizeof b, "%.1fx", c.timeMul); return b; }
    case D_ON: return onoff(c.enabled);
    case D_TOILET: return onoff(c.toilet);
    case D_DOGGO: return onoff(c.noDoggo);
    case D_CHANCE: return std::to_string(c.chance) + "%";
    case D_MINGAP: return std::to_string(c.minGap) + " loops";
    case D_MAXGAP: return c.maxGap == 0 ? std::string("never") : std::to_string(c.maxGap) + " loops";
    case D_FIRST: return "loop " + std::to_string(c.firstLoop);
    case D_SHARE: return std::to_string(c.toiletShare) + "% toilet / " + std::to_string(100 - c.toiletShare) + "% no Doggo";
    case D_FORCE: return c.forceNext == Shift::None ? std::string("off") : std::string(ShiftName(c.forceNext));
    case D_RESTART: return screen == Screen::Game ? std::string("ENTER") : std::string("(in game only)");
    case D_DARK: return std::to_string(story.darkPoints) + (story.tainted ? "  (tainted)" : "");
    case D_ALLMEM: case D_CLEARMEM: return std::to_string(story.Awareness()) + "/" + std::to_string(AllMemories().size());
    case D_BLIND: return onoff(visitorBlind);
    case D_DEFAULTS: return "ENTER";
    }
    return "";
}
static const char* DbgLabel(int row) {
    static const char* L[D_COUNT] = {
        "Game speed (how fast the clock runs)", "Reality shifts", "  45 minute toilet", "  No Doggo", "  Chance per loop (after cooldown)",
        "  Normal loops before the next can happen", "  Guaranteed after", "  First possible loop", "  Mix",
        "  Force the next loop", "Restart this loop now", "Dark points", "Give all memories", "Clear memories (keeps darkness)",
        "Visitor can't see you", "Reset these settings to default" };
    return L[row];
}

static void DbgChange(int row, int dir, bool enter) {
    ShiftConfig& c = gShiftCfg;
    auto step = [&](int& v, int lo, int hi, int st) { v = std::clamp(v + dir * st, lo, hi); };
    bool tog = enter || dir != 0;
    switch (row) {
    case D_SPEED: if (dir) c.timeMul = std::clamp(std::round((c.timeMul + dir * 0.2f) * 10) / 10.0f, 1.0f, 2.0f); break;
    case D_ON: if (tog) c.enabled = !c.enabled; break;
    case D_TOILET: if (tog) c.toilet = !c.toilet; break;
    case D_DOGGO: if (tog) c.noDoggo = !c.noDoggo; break;
    case D_CHANCE: step(c.chance, 0, 100, 5); break;
    case D_MINGAP: step(c.minGap, 0, 30, 1); break;
    case D_MAXGAP: step(c.maxGap, 0, 30, 1); break;
    case D_FIRST: step(c.firstLoop, 1, 30, 1); break;
    case D_SHARE: step(c.toiletShare, 0, 100, 10); break;
    case D_FORCE: if (tog) { int f = ((int)c.forceNext + (dir < 0 ? 3 : 1)) % 4; c.forceNext = (Shift)f; } break;
    case D_RESTART: if (enter && screen == Screen::Game) { dbgRestartLoop = true; dbgOpen = false; } break;
    case D_DARK: if (dir) { story.DebugSetDark(story.darkPoints + dir); SaveGame(); } break;
    case D_ALLMEM: if (enter) { story.DebugAllMemories(); SaveGame(); ToastFor("DEBUG", "Every memory unlocked.", Color{ 120, 220, 140, 255 }); } break;
    case D_CLEARMEM: if (enter) { story.DebugClearMemories(); SaveGame(); ToastFor("DEBUG", "Memories cleared. The darkness stays.", Color{ 120, 220, 140, 255 }); } break;
    case D_BLIND: if (tog) visitorBlind = !visitorBlind; break;
    case D_DEFAULTS: if (enter) { gShiftCfg = ShiftConfig(); visitorBlind = false; } break;
    }
    SaveDebugCfg();
}

static bool UpdateDebugPanel() {
    // returns true while the panel owns the input
    if (debugMode && !jukeboxOpen && IsKeyPressed(KEY_O)) { dbgOpen = !dbgOpen; return true; }
    if (!dbgOpen) return false;
    if (IsKeyPressed(KEY_ESCAPE)) { dbgOpen = false; return true; }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) dbgSel = (dbgSel + 1) % D_COUNT;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) dbgSel = (dbgSel + D_COUNT - 1) % D_COUNT;
    int dir = (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) ? 1 : (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) ? -1 : 0;
    bool enter = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E);
    if (dir || enter) DbgChange(dbgSel, dir, enter);
    return true;
}

static void DrawDebugPanel() {
    DrawRectangle(0, 0, (int)(VW * US), (int)(VH * US), Fade(Color{ 6, 10, 8, 255 }, 0.95f));
    UIText("DEBUG PANEL", 20, 12, 14, Color{ 120, 220, 140, 255 });
    const ShiftConfig& c = gShiftCfg;
    std::string info = "Loop " + std::to_string(story.loop) + "   this loop: " + ShiftName(story.shift) +
                       "   normal loops in a row: " + std::to_string(story.cleanStreak);
    UIText(info.c_str(), 20, 30, 6, Fade(WHITE, 0.6f));
    std::string rule = !c.enabled ? std::string("Reality shifts are OFF.") :
        "So: no shifts before loop " + std::to_string(c.firstLoop) + ", at least " + std::to_string(c.minGap) +
        " normal loops between, " + std::to_string(c.chance) + "% per loop after that" +
        (c.maxGap ? ", forced after " + std::to_string(std::max(c.maxGap, c.minGap)) + " normal loops." : ".");
    UIText(rule.c_str(), 20, 39, 6, Fade(WHITE, 0.6f));
    for (int i = 0; i < D_COUNT; ++i) {
        float y = 50 + i * 12.0f;
        bool sel = i == dbgSel;
        if (sel) DrawRectangle((int)(16 * US), (int)((y - 2) * US), (int)((VW - 32) * US), (int)(12 * US), Color{ 30, 60, 40, 255 });
        bool section = i == D_SPEED || i == D_ON || i == D_RESTART || i == D_DARK || i == D_ALLMEM || i == D_BLIND || i == D_DEFAULTS;
        UIText(DbgLabel(i), 22, y, 7, section ? WHITE : Fade(WHITE, 0.8f));
        std::string v = DbgValue(i);
        UIText(v.c_str(), VW - 24 - UIMeasure(v.c_str(), 7), y, 7, sel ? Color{ 255, 210, 90, 255 } : Color{ 120, 220, 140, 255 });
    }
    UIText("UP/DOWN pick    LEFT/RIGHT change    ENTER toggle / do it    O or ESC close    (saved in this browser)", 20, VH - 14, 6, Fade(WHITE, 0.5f));
}

// ============================================================ title
static void DrawTitle() {
    ClearBackground(Color{ 6, 6, 10, 255 });
    float pulse = 0.5f + 0.5f * std::sin(globalT * 2);
    const char* t = "600 SECONDS";
    UIText(t, VW / 2.0f - UIMeasure(t, 30) / 2, 60, 30, Color{ 220, 60, 60, 255 });
    {
        // the title watch counts down the last seconds before the knock, forever
        char tb[16]; std::snprintf(tb, sizeof tb, "16:49:%02d", 50 + ((int)globalT) % 10);
        DrawWatch(tb, VW / 2.0f - WatchWidth(9) / 2 + 9, 112, 9);
    }
    if (story.tainted) {
        // the title remembers
        if (std::fmod(globalT, 3.0f) < 0.1f) UIText(t, VW / 2.0f - UIMeasure(t, 30) / 2 + 3, 60, 30, Fade(WHITE, 0.4f));
        const char* y = "You did this.";
        UIText(y, VW / 2.0f - UIMeasure(y, 8) / 2, 162, 8, Color{ 170, 25, 30, 255 });
    }
    if (!story.endings.empty()) {
        std::string e = "Endings: ";
        for (auto& x : story.endings) e += x + "  ";
        UIText(e.c_str(), VW / 2.0f - UIMeasure(e.c_str(), 6) / 2, 250, 6, Fade(GOLD, 0.6f));
    }
    const char* sub = "a Sund Origin Studio prototype";
    UIText(sub, VW / 2.0f - UIMeasure(sub, 8) / 2, 150, 8, GRAY);
    const char* st = hasSave ? (story.tainted ? "ENTER / CLICK: continue      N: new game (it won't help)" : "ENTER / CLICK: continue      N: new game") : "ENTER / CLICK: start";
    if (touchMode) st = hasSave ? "TAP: continue" : "TAP: start";
    UIText(st, VW / 2.0f - UIMeasure(st, 10) / 2, 180, 10, Fade(WHITE, 0.5f + 0.5f * pulse));
    if (touchMode && hasSave) {
        const char* ng = newGameArm > 0 ? (story.tainted ? "tap again. It won't help." : "tap again to start over") : "NEW GAME";
        Panel(VW / 2.0f - 60, 196, 120, 18, Fade(BLACK, 0.5f), newGameArm > 0 ? Color{ 240, 80, 80, 255 } : Fade(WHITE, 0.4f));
        UIText(ng, VW / 2.0f - UIMeasure(ng, 8) / 2, 200, 8, newGameArm > 0 ? Color{ 240, 80, 80, 255 } : Fade(WHITE, 0.8f));
    }
    const char* c1 = touchMode ? "D-pad walk  -  RUN / WAIT hold  -  tap or E to use  -  tap top left for memories" :
        "WASD/arrows walk  -  Shift run  -  E/Space use  -  hold T wait  -  J memory log  -  Tab skip  -  P pause  -  F fullscreen";
    UIText(c1, VW / 2.0f - UIMeasure(c1, 6) / 2, 220, 6, Fade(WHITE, 0.6f));
    DrawAIBadge(8, VH - 18, Fade(WHITE, 0.55f));
    UIText(kVersion, VW - 8 - UIMeasure(kVersion, 7), 8, 7, Fade(WHITE, 0.45f));
    if (debugMode) {
        const char* d1 = "DEBUG: O panel, M jukebox, 5 living room, 6 chores, 7 Fliptendo, 8 +5, 9 16:49, 0 knock";
        const char* d2 = hardResetArm > 0 ? "Shift+R again to WIPE EVERYTHING (memories, darkness, endings, achievements)" : "Shift+R: hard reset";
        UIText(d1, VW - 8 - UIMeasure(d1, 6), VH - 34, 6, Color{ 120, 220, 140, 200 });
        UIText(d2, VW - 8 - UIMeasure(d2, 6), VH - 24, 6, hardResetArm > 0 ? Color{ 240, 80, 80, 255 } : Color{ 120, 220, 140, 200 });
    }
    const char* c2 = "Contains off-screen violence, implied child abuse and the death of a dog. Flashing lights.";
    UIText(c2, VW / 2.0f - UIMeasure(c2, 6) / 2, 236, 6, Fade(WHITE, 0.35f));
}

// ============================================================ v0.9 pause menu and settings
static void SaveSettings() {
    std::string s = std::to_string(settings.music) + "|" + std::to_string(settings.sfx) + "|" + std::to_string(settings.text) + "|" + std::to_string(settings.calmFlashes ? 1 : 0);
#if defined(PLATFORM_WEB)
    EM_ASM({ try { localStorage.setItem('600s_settings', UTF8ToString($0)); } catch (e) {} }, s.c_str());
#else
    SaveFileText("600s_settings.txt", (char*)s.c_str());
#endif
}
static void LoadSettings() {
    std::string s;
#if defined(PLATFORM_WEB)
    char* p = (char*)EM_ASM_PTR({
        var s = null;
        try { s = localStorage.getItem('600s_settings'); } catch (e) {}
        if (!s) s = "";
        var len = lengthBytesUTF8(s) + 1;
        var buf = _malloc(len);
        stringToUTF8(s, buf, len);
        return buf;
    });
    if (p) s = p;
    free(p);
#else
    if (FileExists("600s_settings.txt")) { char* t = LoadFileText("600s_settings.txt"); if (t) s = t; UnloadFileText(t); }
#endif
    int a, b, c, d;
    if (std::sscanf(s.c_str(), "%d|%d|%d|%d", &a, &b, &c, &d) == 4) {
        settings.music = std::max(0, std::min(10, a)); settings.sfx = std::max(0, std::min(10, b));
        settings.text = std::max(0, std::min(3, c)); settings.calmFlashes = d != 0;
    }
}
static const int PAUSE_ROWS = 6;
static const float PAUSE_Y = 84, PAUSE_H = 20, PAUSE_X = 110, PAUSE_W = 260;
static void DrawPause() {
    DrawRectangle(0, 0, (int)(VW * US), (int)(VH * US), Fade(Color{ 8, 8, 14, 255 }, 0.86f));
    UIText("PAUSED", VW / 2.0f - UIMeasure("PAUSED", 16) / 2, 48, 16, Color{ 230, 60, 60, 255 });
    static const char* speed[4] = { "Slow", "Normal", "Fast", "Instant" };
    std::string val[PAUSE_ROWS] = { "", std::to_string(settings.music) + " / 10", std::to_string(settings.sfx) + " / 10", speed[settings.text],
                                    settings.calmFlashes ? "Calm" : "Normal", "" };
    static const char* name[PAUSE_ROWS] = { "Resume", "Music", "Sound", "Text speed", "Flashes", "Back to the title" };
    for (int i = 0; i < PAUSE_ROWS; ++i) {
        float y = PAUSE_Y + i * PAUSE_H;
        if (i == pauseSel) DrawRectangle((int)(PAUSE_X * US), (int)((y - 3) * US), (int)(PAUSE_W * US), (int)((PAUSE_H - 2) * US), Color{ 50, 40, 80, 255 });
        UIText(name[i], PAUSE_X + 8, y, 10, WHITE);
        if (!val[i].empty()) UIText(val[i].c_str(), PAUSE_X + PAUSE_W - 8 - UIMeasure(val[i].c_str(), 10), y, 10, Color{ 255, 210, 90, 255 });
    }
    const char* h = touchMode ? "Tap a row. Left side lowers, right side raises." : "UP/DOWN pick    LEFT/RIGHT change    ENTER use    P or ESC close";
    UIText(h, VW / 2.0f - UIMeasure(h, 6) / 2, PAUSE_Y + PAUSE_ROWS * PAUSE_H + 8, 6, Fade(WHITE, 0.5f));
}
static void UpdatePause() {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) { pauseOpen = false; SaveSettings(); return; }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) pauseSel = (pauseSel + 1) % PAUSE_ROWS;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) pauseSel = (pauseSel + PAUSE_ROWS - 1) % PAUSE_ROWS;
    int dir = (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) ? 1 : (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) ? -1 : 0;
    bool use = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E);
    if (pauseTapRow >= 0) { pauseSel = pauseTapRow; dir = pauseTapDir; use = true; pauseTapRow = -1; }
    switch (pauseSel) {
    case 0: if (use) { pauseOpen = false; SaveSettings(); } break;
    case 1: if (dir) { settings.music = std::max(0, std::min(10, settings.music + dir)); if (musicOk) SetAudioStreamVolume(musicStream, settings.music / 10.0f); } break;
    case 2: if (dir) { settings.sfx = std::max(0, std::min(10, settings.sfx + dir)); Play("chime"); } break;
    case 3: if (dir) settings.text = std::max(0, std::min(3, settings.text + dir)); break;
    case 4: if (dir || use) settings.calmFlashes = !settings.calmFlashes; break;
    case 5:
        if (use) {
            pauseOpen = false; SaveSettings(); SaveGame();
            story.beats.clear(); beatActive = false; hushMusic = false; hidden = false; recapOpen = false; journalOpen = false;
            scene = SceneMode::None; hasSave = true; screen = Screen::Title;
        }
        break;
    }
}

static void StartGame(bool fresh) {
    if (fresh) story.NewGame();
    else story.LoadSave(LoadSaveString());
    story.beats.clear();
    story.BeginLoop();
    Snap(player, 5, 5); player.dir = 0;
    Snap(mom, 8, 16); Snap(dad, 24, 16); Snap(doggo, 6, 6); Snap(visitor, 20, 29);
    scene = SceneMode::None; hidden = false; wasCollection = false; flashT = 0; journalOpen = false;
    screen = Screen::Game;
    SaveGame();
    StartNextBeat();
}

// ============================================================ phone mode (v0.4.2)
// Left thumb: D-pad. Right thumb: E (use / talk / continue), RUN and WAIT (hold), SKIP during talk.
// Tap anywhere else = E. Tap the top left corner = memory log. Choices can be tapped directly.
struct TouchPt { float x, y; };          // in virtual UI units (can be outside 0..VW in the letterbox bars)
static std::vector<TouchPt> touchLive, touchTaps;
struct TouchLayout { float L, R, T, B, dx, dy, dr; Vector2 e, run, wait, skip; float er, sr; };

static TouchLayout Layout() {
    TouchLayout l;
    float ox = (SW - VW * US) / 2, oy = (SH - VH * US) / 2;
    l.L = -ox / US; l.R = VW + ox / US; l.T = -oy / US; l.B = VH + oy / US;
    // use the black bars on wide phones, but never sit further than 60 units outside the picture
    float padL = std::max(l.L, -60.0f), padR = std::min(l.R, VW + 60.0f);
    l.dr = 40; l.dx = padL + 50; l.dy = l.B - 50;
    l.er = 21; l.sr = 14;
    l.e = { padR - 38, l.B - 44 };
    l.run = { padR - 88, l.B - 26 };
    l.wait = { padR - 38, l.B - 94 };
    l.skip = { padR - 90, l.B - 70 };
    return l;
}
static bool In(TouchPt p, Vector2 c, float r) { return std::hypot(p.x - c.x, p.y - c.y) <= r; }
static bool SkipShown() { return CanSkip(); }

static void ReadTouches() {
    touchLive.clear(); touchTaps.clear();
#if defined(PLATFORM_WEB)
    static float buf[64];
    int any = EM_ASM_INT({
        var T = window._t600; if (!T) return 0;
        var n = 0; var k;
        for (k in T.live) { if (n >= 8) break; HEAPF32[($0 >> 2) + n * 2] = T.live[k][0]; HEAPF32[($0 >> 2) + n * 2 + 1] = T.live[k][1]; n++; }
        var m = 0;
        while (T.taps.length && m < 8) { var t = T.taps.shift(); HEAPF32[($0 >> 2) + 16 + m * 2] = t[0]; HEAPF32[($0 >> 2) + 17 + m * 2] = t[1]; m++; }
        HEAPF32[($0 >> 2) + 40] = n; HEAPF32[($0 >> 2) + 41] = m;
        return T.any ? 1 : 0;
    }, buf);
    if (any && !touchMode) {
        touchMode = true;
        EM_ASM({ var b = document.getElementById('fs'); if (b) b.style.display = 'none'; });
    }
    float ox = (SW - VW * US) / 2, oy = (SH - VH * US) / 2;
    int n = (int)buf[40], m = (int)buf[41];
    for (int i = 0; i < n; ++i) touchLive.push_back({ (buf[i * 2] - ox) / US, (buf[i * 2 + 1] - oy) / US });
    for (int i = 0; i < m; ++i) touchTaps.push_back({ (buf[16 + i * 2] - ox) / US, (buf[17 + i * 2] - oy) / US });
#endif
}

static void DpadDirs(TouchPt p, const TouchLayout& l, bool out[4]) {
    // 8-way: up, down, left, right
    float dx = p.x - l.dx, dy = p.y - l.dy, d = std::hypot(dx, dy);
    out[0] = out[1] = out[2] = out[3] = false;
    if (d > l.dr * 1.5f || d < 6) return;
    if (dy < -0.38f * d) out[0] = true;
    if (dy > 0.38f * d) out[1] = true;
    if (dx < -0.38f * d) out[2] = true;
    if (dx > 0.38f * d) out[3] = true;
}

static void UpdateTouch() {
    std::memcpy(vPrevDown, vDown, sizeof(vDown));
    std::memset(vDown, 0, sizeof(vDown));
    std::memset(vPress, 0, sizeof(vPress));
    ReadTouches();
    if (newGameArm > 0) newGameArm -= GetFrameTime();
    if (!touchMode) return;
    TouchLayout l = Layout();
    const int dirKey[4] = { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT };
    bool inGame = screen == Screen::Game;

    // held controls
    for (auto& p : touchLive) {
        if (!inGame) break;
        bool d[4]; DpadDirs(p, l, d);
        for (int k = 0; k < 4; ++k) if (d[k]) vDown[dirKey[k]] = true;
        if (In(p, l.run, l.sr + 4)) vDown[KEY_LEFT_SHIFT] = true;
        if (In(p, l.wait, l.sr + 4)) vDown[KEY_T] = true;
        if (In(p, l.e, l.er + 4)) vDown[KEY_E] = true;
    }
    for (int k = 0; k < 512; ++k) if (vDown[k] && !vPrevDown[k]) vPress[k] = true;

    // taps (also catches taps shorter than one frame)
    for (auto& p : touchTaps) {
        if (screen == Screen::Splash) { vPress[KEY_ENTER] = true; continue; }
        if (screen == Screen::Title) {
            if (hasSave && p.x > VW / 2.0f - 60 && p.x < VW / 2.0f + 60 && p.y > 196 && p.y < 214) {
                if (newGameArm > 0) { vPress[KEY_N] = true; newGameArm = 0; } else newGameArm = 3.0f;
            } else vPress[KEY_ENTER] = true;
            continue;
        }
        if (pauseOpen) {
            int row = (int)((p.y - PAUSE_Y + 3) / PAUSE_H);
            if (row >= 0 && row < PAUSE_ROWS) { pauseTapRow = row; pauseTapDir = p.x < VW / 2.0f ? -1 : 1; }
            continue;
        }
        if (p.x > VW - 34 && p.y > 54 && p.y < 78 && !beatActive && !journalOpen && !recapOpen && !miniOpen) { vPress[KEY_P] = true; continue; }
        bool d[4]; DpadDirs(p, l, d);
        bool onDpad = std::hypot(p.x - l.dx, p.y - l.dy) <= l.dr * 1.5f;
        if (onDpad) { for (int k = 0; k < 4; ++k) if (d[k]) vPress[dirKey[k]] = true; continue; }
        if (In(p, l.e, l.er + 4)) { vPress[KEY_E] = true; continue; }
        if (In(p, l.run, l.sr + 4) || In(p, l.wait, l.sr + 4)) continue;
        if (SkipShown() && In(p, l.skip, l.sr + 4)) { vPress[KEY_TAB] = true; continue; }
        if (p.x < 100 && p.y < 44) { vPress[KEY_J] = true; continue; }   // top left: memory log
        if (journalOpen) { vPress[KEY_J] = true; continue; }              // tap anywhere closes the log
        if (beatActive && cur.type == BeatType::Choice) {
            // tap a choice directly (same geometry as DrawChoice)
            float w = 300, lineH = 16, h = 26 + cur.choices.size() * lineH;
            float x = VW / 2.0f - w / 2, y = VH - h - 14;
            for (size_t i = 0; i < cur.choices.size(); ++i) {
                float ly = y + 22 + i * lineH;
                if (p.x > x && p.x < x + w && p.y > ly - 3 && p.y < ly - 3 + lineH) { choiceSel = (int)i; break; }
            }
        }
        vPress[KEY_E] = true;
    }
}

static void TouchButton(Vector2 c, float r, const char* label, bool lit, float size) {
    DrawCircleV({ c.x * US, c.y * US }, r * US, Fade(lit ? Color{ 255, 210, 90, 255 } : WHITE, lit ? 0.35f : 0.13f));
    DrawCircleLinesV({ c.x * US, c.y * US }, r * US, Fade(WHITE, 0.45f));
    UIText(label, c.x - UIMeasure(label, size) / 2, c.y - size * 0.62f, size, Fade(WHITE, 0.85f));
}

static void DrawTouchControls() {
    if (!touchMode || screen != Screen::Game) return;
    TouchLayout l = Layout();
    if (!beatActive && !journalOpen && !pauseOpen) UIText("II", VW - 22, 60, 10, Fade(WHITE, 0.6f));
    // D-pad
    DrawCircleV({ l.dx * US, l.dy * US }, l.dr * US, Fade(WHITE, 0.08f));
    DrawCircleLinesV({ l.dx * US, l.dy * US }, l.dr * US, Fade(WHITE, 0.35f));
    const int dirKey[4] = { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT };
    const float ax[4] = { 0, 0, -1, 1 }, ay[4] = { -1, 1, 0, 0 };
    for (int k = 0; k < 4; ++k) {
        Vector2 c = { (l.dx + ax[k] * 26) * US, (l.dy + ay[k] * 26) * US };
        float s = 7 * US;
        Vector2 tip = { c.x + ax[k] * s, c.y + ay[k] * s };
        Vector2 b1 = { c.x - ay[k] * s - ax[k] * s * 0.4f, c.y + ax[k] * s - ay[k] * s * 0.4f };
        Vector2 b2 = { c.x + ay[k] * s - ax[k] * s * 0.4f, c.y - ax[k] * s - ay[k] * s * 0.4f };
        Color col = Fade(vDown[dirKey[k]] ? Color{ 255, 210, 90, 255 } : WHITE, vDown[dirKey[k]] ? 0.9f : 0.5f);
        DrawTriangle(tip, b1, b2, col); DrawTriangle(tip, b2, b1, col);
    }
    TouchButton(l.e, l.er, "E", vDown[KEY_E], 12);
    TouchButton(l.run, l.sr, "RUN", vDown[KEY_LEFT_SHIFT], 6);
    TouchButton(l.wait, l.sr, "WAIT", vDown[KEY_T], 6);
    if (SkipShown()) TouchButton(l.skip, l.sr, "SKIP", false, 6);
    if (!journalOpen && !beatActive) UIText("tap here: memory", 8, 33 + (story.tainted ? 8 : 0), 5, Fade(WHITE, 0.4f));
}

static void DrawRotateHint() {
    if (!touchMode || SH <= SW) return;
    DrawRectangle(-2000, -2000, 8000, 8000, Color{ 6, 6, 10, 255 });
    const char* a = "Turn your phone sideways";
    const char* b = "600 Seconds plays in landscape";
    UIText(a, VW / 2.0f - UIMeasure(a, 40) / 2, VH / 2.0f - 30, 40, WHITE);
    UIText(b, VW / 2.0f - UIMeasure(b, 20) / 2, VH / 2.0f + 22, 20, Fade(WHITE, 0.55f));
}

// ============================================================ frame
static void UpdateDrawFrame() {
    float dt = std::min(GetFrameTime(), 0.05f);
    globalT += dt;
#if defined(PLATFORM_WEB)
    int cw = GetScreenWidth(), ch = GetScreenHeight();
    if (cw != SW || ch != SH) { SW = cw; SH = ch; }
#endif
    US = std::min(SW / (float)VW, SH / (float)VH);
    UpdateTouch();

    // soundtrack follows the act
    {
        int track = 0;
        if (screen == Screen::Title) track = story.Act();
        else if (screen == Screen::Game) {
            if (story.collection && !(beatActive && (cur.type == BeatType::Ending || cur.type == BeatType::EndLoop))) track = 4;
            else track = story.Act();
            if (hushMusic) track = 0;
            if (beatActive && cur.type == BeatType::Ending) track = 3;
        }
        if (!jukeboxOpen) SetMusic(track);
        UpdateMusic();
    }

    if (screen == Screen::Splash) {
        splashT += dt;
        if (splashT > 0.4f && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) splashT = std::max(splashT, 3.4f);
        if (splashT >= 4.7f) screen = Screen::Title;
        BeginDrawing();
        ClearBackground(BLACK);
        float ox = (SW - VW * US) / 2, oy = (SH - VH * US) / 2;
        Camera2D ui{}; ui.offset = { ox, oy }; ui.zoom = 1;
        BeginMode2D(ui);
        DrawSplash();
        EndMode2D();
        EndDrawing();
        return;
    }

    if (screen == Screen::Title) {
        if (!dbgOpen && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_D)) {
            debugMode = !debugMode; // debug without the URL
            if (debugMode) LoadDebugCfg(); else { gShiftCfg = ShiftConfig(); visitorBlind = false; }
        }
        if (hardResetArm > 0) hardResetArm -= GetFrameTime();
        if (debugMode && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) && IsKeyPressed(KEY_R)) {
            if (hardResetArm > 0) {
                // a true hard reset: a brand new story, nothing kept, save wiped
                story = Story();
#if defined(PLATFORM_WEB)
                EM_ASM({ try { localStorage.removeItem('600s_save'); } catch (e) {} });
#else
                if (FileExists("600s_save.txt")) remove("600s_save.txt");
#endif
                hasSave = false; hardResetArm = 0;
                ToastFor("HARD RESET", "Everything is gone. Like it never happened.", Color{ 240, 80, 80, 255 });
            } else hardResetArm = 3.0f;
        }
        bool jb = UpdateDebugPanel() || UpdateJukebox();
        if (jb) {}
        else if (IsKeyPressed(KEY_N)) StartGame(true);
        else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) StartGame(!hasSave);
        BeginDrawing();
        BeginTextureMode(target); ClearBackground(BLACK); EndTextureMode();
        float ox = (SW - VW * US) / 2, oy = (SH - VH * US) / 2;
        ClearBackground(BLACK);
        // UI is drawn in "virtual" units scaled by US; offset by letterbox
        Camera2D ui{}; ui.offset = { ox, oy }; ui.zoom = 1;
        BeginMode2D(ui);
        DrawTitle();
        DrawToasts(GetFrameTime());
        if (jukeboxOpen) DrawJukebox();
        if (dbgOpen) DrawDebugPanel();
        DrawRotateHint();
        EndMode2D();
        EndDrawing();
        return;
    }

    // ---------------- update
    bool jbGame = UpdateDebugPanel() || UpdateJukebox();
    if (screen == Screen::Game && !jbGame && !pauseOpen && !journalOpen && !recapOpen && !miniOpen && !jukeboxOpen && !dbgOpen &&
        (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE))) { pauseOpen = true; pauseSel = 0; jbGame = true; }
    else if (pauseOpen) { UpdatePause(); jbGame = true; }
    if (dbgRestartLoop) {
        // debug: skip straight to the next loop (applies "force the next loop")
        dbgRestartLoop = false; jbGame = true;
        story.beats.clear(); journalOpen = false; hidden = false;
        Beat e; e.type = BeatType::EndLoop; story.beats.push_back(e);
        beatActive = false; StartNextBeat();
    }
    if (!jbGame && !recapOpen && !miniOpen && IsKeyPressed(KEY_J) && (!beatActive || journalOpen)) journalOpen = !journalOpen;
    if (jbGame) {
        // paused while the jukebox is open
    } else if (miniOpen) {
        UpdateMini(dt);
    } else if (recapOpen) {
        UpdateRecap(dt);
    } else if (journalOpen) {
        if (IsKeyPressed(KEY_ESCAPE)) journalOpen = false;
        int n = (int)AllTopics().size() + 1;   // + Endings
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) { journalTopic = (journalTopic + 1) % n; journalScroll = 0; }
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) { journalTopic = (journalTopic + n - 1) % n; journalScroll = 0; }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) journalScroll++;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) journalScroll = std::max(0, journalScroll - 1);
        float wheel = GetMouseWheelMove();
        if (wheel < 0) journalScroll++; else if (wheel > 0) journalScroll = std::max(0, journalScroll - 1);
    } else if (beatActive) {
        UpdateBeat(dt);
    } else {
        // free roam
        Vector2 mv{ 0, 0 };
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) mv.x += 1;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) mv.x -= 1;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) mv.y += 1;
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) mv.y -= 1;
        bool running = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        kidRunning = running && !story.kidShot && (mv.x != 0 || mv.y != 0) && !hidden;
        float speed = story.kidShot ? 26 : running ? 128 : 84;
        if (IsOutsideT(TileAt(TX(player.pos.x), TX(player.pos.y + 6)))) speed *= 0.88f;   // v0.8: snow to your shins
        if (hidden) mv = { 0, 0 };
        if (mv.x != 0 || mv.y != 0) {
            float l = std::hypot(mv.x, mv.y); mv.x /= l; mv.y /= l;
            float nx = player.pos.x + mv.x * speed * dt, ny = player.pos.y + mv.y * speed * dt;
            if (CanStand(nx, player.pos.y)) player.pos.x = nx;
            if (CanStand(player.pos.x, ny)) player.pos.y = ny;
            if (std::fabs(mv.x) > std::fabs(mv.y)) player.dir = mv.x > 0 ? 3 : 2; else player.dir = mv.y > 0 ? 0 : 1;
            player.anim += dt * (running && !story.kidShot ? 15 : 10);
        } else player.anim = 0;

        if (interactCooldown > 0) interactCooldown -= dt;
        else if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE)) {
            if (hidden) {
                hidden = false;
            } else {
                Target tg = FindTarget();
                if (!tg.id.empty()) {
                    if (story.ShouldHide(tg.id)) {
                        hidden = true; hideT = 0;
                        const Obj* o = nullptr;
                        for (auto& ob : story.world.objs) if (ob.id == tg.id) o = &ob;
                        hidePos = o ? TileCenter(o->x, o->y) : player.pos;
                    } else if (tg.id == lastTalkId && globalT - lastTalkT < 0.9f) {
                        // v0.6: a stray extra click right after a conversation doesn't restart it
                    } else { talkingTo = tg.id; lastTalkId = tg.id; story.Interact(tg.id, TX(player.pos.x), TX(player.pos.y)); }
                }
            }
        }
        if (debugMode && IsKeyPressed(KEY_FIVE) && !story.inBasement) { Snap(player, 16, 16); player.dir = 3; }   // debug: hop to the living room
        if (debugMode && IsKeyPressed(KEY_SIX)) { story.fed = story.trashDone = story.roomClean = story.dishesDone = story.firewoodDone = true; ToastFor("DEBUG", "All five chores done", SKYBLUE); }
        if (debugMode && IsKeyPressed(KEY_SEVEN)) story.Choose("ft_flappy", TX(player.pos.x), TX(player.pos.y));   // debug: jump into the Fliptendo
        if (debugMode && IsKeyPressed(KEY_EIGHT)) story.AdvanceMinutes(5, TX(player.pos.x), TX(player.pos.y));
        if (debugMode && IsKeyPressed(KEY_NINE) && story.t < 49 * 60) story.AdvanceMinutes(49 - (int)(story.t / 60), TX(player.pos.x), TX(player.pos.y));
        if (debugMode && IsKeyPressed(KEY_ZERO) && !story.collection) story.AdvanceMinutes(std::max(1, story.visitorTime / 60 - (int)(story.t / 60)), TX(player.pos.x), TX(player.pos.y));
        story.Tick(dt, IsKeyDown(KEY_T), TX(player.pos.x), TX(player.pos.y));
        if (!story.beats.empty()) StartNextBeat();
    }

    // NPCs move whenever the journal is closed (they keep walking during talk, that's fine)
    if (!journalOpen && !jukeboxOpen && !dbgOpen && !recapOpen && !miniOpen && !pauseOpen) {
        float npcSpeed = story.Stage() >= 2 ? 62.0f : 44.0f;
        NpcState ms = story.Mom(), ds = story.Dad(), vs = story.Visitor();
        if (story.collection && !wasCollection) {
            // 16:50: everyone takes their places
            Snap(mom, ms.x, ms.y); Snap(dad, ds.x, ds.y); Snap(visitor, vs.x, vs.y);
            if (story.DoggoExists() && story.Tutorial()) Snap(doggo, 16, 18);
            lastThreats = 0; vDoorT = 0;
        }
        wasCollection = story.collection;
        if (beatActive && cur.type == BeatType::Fade && fadeApplied) {
            Snap(mom, ms.x, ms.y);
            if (scene == SceneMode::None) Snap(dad, ds.x, ds.y);
        }
        mom.visible = ms.visible;
        if (!beatActive && !talkingTo.empty()) { lastTalkT = globalT; talkingTo.clear(); }
        auto face = [&](Actor& a) {
            float dx = player.pos.x - a.pos.x, dy = player.pos.y - a.pos.y;
            if (std::fabs(dx) > std::fabs(dy)) a.dir = dx > 0 ? 3 : 2; else a.dir = dy > 0 ? 0 : 1;
            a.anim = 0;
        };
        bool momTalking = beatActive && talkingTo == "mom" && !story.collection;
        bool dadTalking = beatActive && talkingTo == "dad" && !story.collection && scene == SceneMode::None;
        if (momTalking) face(mom); else MoveActor(mom, ms.x, ms.y, npcSpeed, dt, true);
        if (ms.facingWall && TX(mom.pos.x) == ms.x && TX(mom.pos.y) == ms.y) mom.dir = 1;

        // v0.7: grown-ups open closed doors on the way (they have keys) and close them again behind them
        auto humanDoors = [&](Actor& a, HeldDoor& hd, const char* who) {
            auto restore = [&]() {
                if (hd.x < 0) return;
                int k = hd.y * World::W + hd.x;
                bool busy = (TX(player.pos.x) == hd.x && TX(player.pos.y) == hd.y) ||
                            (story.DoggoExists() && TX(doggo.pos.x) == hd.x && TX(doggo.pos.y) == hd.y) ||
                            (TX(a.pos.x) == hd.x && TX(a.pos.y) == hd.y) || story.DoorBroken(hd.x, hd.y);
                if (busy) return;
                story.closedDoors.insert(k);
                if (hd.locked) { if (hd.x == 20 && hd.y == 20) story.frontLocked = true; else story.lockedDoors.insert(k); }
                hd.x = -1;
            };
            const int fdx[4] = { 0, 0, -1, 1 }, fdy[4] = { 1, -1, 0, 0 };
            int ax = TX(a.pos.x), ay = TX(a.pos.y);
            int fx = TX(a.pos.x + fdx[a.dir] * 10), fy = TX(a.pos.y + fdy[a.dir] * 10);
            for (int k = 0; k < 2; ++k) {
                int x = k ? fx : ax, y = k ? fy : ay;
                if (!story.world.InBounds(x, y) || story.world.tiles[y][x] != Tile::Door || !story.DoorClosed(x, y)) continue;
                if (x == 6 && y == 9 && story.grounded) continue;   // your door stays locked while you're grounded
                if (hd.x >= 0 && (hd.x != x || hd.y != y)) restore();
                hd.x = x; hd.y = y; hd.locked = story.DoorLocked(x, y);
                story.OpenDoor(x, y);
                if (std::hypot(a.pos.x - player.pos.x, a.pos.y - player.pos.y) < TS * 9) {
                    Play("door");
                    if (hd.locked && !(x == 20 && y == 20)) AddBubble(who, "(click) ...Who locked this?");
                }
            }
            if (hd.x >= 0 && !(ax == hd.x && ay == hd.y) && !(fx == hd.x && fy == hd.y)) restore();
        };
        humanDoors(mom, momDoor, "Mom");

        if (scene == SceneMode::DadRuns) {
            // v0.7: Dad doesn't run away anymore. He runs at him, and buys you a few seconds
            sceneT += dt;
            dad.visible = true;
            visitor.visible = true;
            if (!beatActive) {
                float dv = std::hypot(visitor.pos.x - dad.pos.x, visitor.pos.y - dad.pos.y);
                if (dv > 12 && struggleT <= 0) {
                    MoveActor(dad, TX(visitor.pos.x), TX(visitor.pos.y), 110, dt, false, 9);
                    float dx = dad.pos.x - visitor.pos.x, dy = dad.pos.y - visitor.pos.y;
                    visitor.dir = std::fabs(dx) > std::fabs(dy) ? (dx > 0 ? 3 : 2) : (dy > 0 ? 0 : 1);
                    if (sceneT > 3.0f) struggleT = 0.01f;   // never get stuck walking
                } else {
                    struggleT += dt;
                    dad.anim += dt * 14; visitor.anim += dt * 10;
                    dad.pos.x += std::sin(globalT * 37) * 0.35f; visitor.pos.x -= std::sin(globalT * 37) * 0.25f;
                    if (std::fmod(struggleT, 0.55f) < dt) { Play("thud"); AddBubble(GetRandomValue(0, 1) ? "Dad" : "Mom", GetRandomValue(0, 1) ? "RUN!" : "GO!", 0.8f); }
                    if (struggleT > 2.6f) {
                        Play("crack");
                        flashT = 0.35f;
                        struggleT = 0;
                        scene = SceneMode::None;
                        story.SceneDone("dad_runs");
                        NpcState d2 = story.Dad();
                        Snap(dad, d2.x, d2.y);
                        dad.anim = 0; visitor.anim = 0;
                        StartNextBeat();
                    }
                }
            }
        } else {
            dad.visible = ds.visible;
            if (dadTalking) face(dad);
            else if (!story.collection) MoveActor(dad, ds.x, ds.y, npcSpeed, dt, false);
            else if (story.dadOnFloor) { Snap(dad, ds.x, ds.y); dad.anim = 0; }
            if (!story.collection) humanDoors(dad, dadDoor, "Dad");
            if (scene != SceneMode::Chase) {
                visitor.visible = vs.visible;
                if (!story.collection) MoveActor(visitor, vs.x, vs.y, 50, dt, false);
            }
        }
        if (story.collection && !beatActive && (story.dadOnFloor || story.momOnFloor) && scene == SceneMode::Chase) {
            momSobT -= dt;
            if (momSobT <= 0 && story.MomHere()) { AddBubble("Mom", story.momOnFloor ? "*crying, very quietly*" : GetRandomValue(0, 1) ? "*sobbing*" : "Please wake up. Please.", 2.0f); momSobT = (float)GetRandomValue(5, 8); }
        }

        // v0.7: the visitor walks through unlocked doors and kicks in locked ones
        auto visitorStep = [&](int tx, int ty, float sp, float stop) -> bool {
            if (vDoorT > 0) {
                vDoorT -= dt;
                if (std::fmod(vDoorT, 0.5f) < dt) Play("thud");
                if (vDoorT <= 0) { Play("crack"); story.BreakDoor(vDoorX, vDoorY); AddBubble("The visitor", "There we go.", 1.4f); }
                visitor.anim = 0;
                return true;
            }
            gMover = 1;
            Vector2 st = NextStep(visitor.pos, tx, ty, false);
            int sx = TX(st.x), sy = TX(st.y);
            if (story.world.InBounds(sx, sy) && story.world.tiles[sy][sx] == Tile::Door && story.DoorClosed(sx, sy) &&
                std::hypot(st.x - visitor.pos.x, st.y - visitor.pos.y) < TS + 3) {
                if (story.DoorLocked(sx, sy)) {
                    vDoorT = 1.6f; vDoorX = sx; vDoorY = sy; Play("thud");
                    AddBubble("The visitor", GetRandomValue(0, 1) ? "Locked? Really?" : "Knock knock, 06.", 1.6f);
                    gMover = 0;
                    return true;
                }
                story.OpenDoor(sx, sy);
                Play("door");
            }
            MoveActor(visitor, tx, ty, sp, dt, false, stop);
            gMover = 0;
            return false;
        };

        if (scene == SceneMode::Chase) {
            if (!story.chase) { scene = SceneMode::None; hidden = false; }
            else if (!beatActive) {
                sceneT += dt;
                visitor.visible = true;
                bool injured = story.visitorInjured;
                float searchSp = injured ? 40.0f : 54.0f;
                float chaseSp = injured ? 80.0f : 108.0f;    // faster than walking, slower than running
                auto goTo = [&](Vector2 p, float sp, float stop) {
                    visitorStep(TX(p.x), TX(p.y), sp, stop);
                    return std::hypot(visitor.pos.x - p.x, visitor.pos.y - p.y) <= stop + 1.5f;
                };
                auto caught = [&]() {
                    scene = SceneMode::None;
                    Play("thud");
                    story.OnCaught();
                    StartNextBeat();
                };
                if (vSpotCooldown > 0) vSpotCooldown -= dt;

                if (story.world.RoomAt(TX(player.pos.x), TX(player.pos.y)) == Room::Vidda) {
                    // past the fence: hope, then headlights
                    scene = SceneMode::None; hidden = false;
                    story.OnLeftPremises();
                    StartNextBeat();
                } else if (story.visitorStun > 0) {
                    // v0.6: blinded by the flashlight
                    story.visitorStun -= dt;
                    visitor.dir = (int)(globalT * 6) % 4;
                } else if (downedT > 0) {
                    downedT -= dt;
                    visitor.dir = 0;
                    if (downedT <= 0 && story.visitorDowned) { story.OnVisitorUp(); StartNextBeat(); vmode = VMode::Search; PickNextRoom(); }
                } else {
                    bool sees = VisitorSeesKid();
                    if (sees && vmode != VMode::Grace && vmode != VMode::GoToHide) {
                        if (vmode != VMode::Chase && vSpotCooldown <= 0) { VisitorBark(kSpotLines, 3); vSpotCooldown = 6; }
                        vmode = VMode::Chase; vLostT = 0; vLastSeen = player.pos;
                    }
                    switch (vmode) {
                    case VMode::Grace:
                        if (goTo(vGraceGoal, 34, 0)) {
                            vGraceWait -= dt;
                            if (vGraceWait <= 0) { vmode = VMode::Search; PickNextRoom(); }
                        }
                        break;
                    case VMode::Chase:
                        if (hidden) {
                            // he saw you dive in there
                            if (vLostT < 0.6f) { vmode = VMode::GoToHide; break; }
                        }
                        if (sees) {
                            vLastSeen = player.pos; vLostT = 0;
                            // stand still and he just walks up to you (so you can face him with E)
                            float dk = std::hypot(visitor.pos.x - player.pos.x, visitor.pos.y - player.pos.y);
                            float sp = (player.anim == 0 && dk < 44) ? 30.0f : chaseSp;
                            visitorStep(TX(player.pos.x), TX(player.pos.y), sp, 6);
                        }
                        else {
                            vLostT += dt;
                            if (goTo(vLastSeen, chaseSp * 0.9f, 2) || vLostT > 4.0f) {
                                vmode = VMode::Lost; vPause = 1.6f; VisitorBark(kLostLines, 3);
                            }
                        }
                        if (std::hypot(visitor.pos.x - player.pos.x, visitor.pos.y - player.pos.y) < 9 && sceneT > 0.4f) caught();
                        break;
                    case VMode::Lost:
                        vPause -= dt;
                        if (std::fmod(vPause, 0.5f) < dt) visitor.dir = (visitor.dir + 1 + GetRandomValue(0, 2)) % 4;
                        if (vPause <= 0) {
                            // search the room he lost you in again, then carry on
                            Room r = story.world.RoomAt(TX(vLastSeen.x), TX(vLastSeen.y));
                            int ri = -1;
                            for (int k = 0; k < kRoomCount; ++k) if (kSearchRooms[k] == r) ri = k;
                            vmode = VMode::Search;
                            if (ri >= 0) PlanRoom(ri); else PickNextRoom();
                        }
                        break;
                    case VMode::Search:
                        // v0.8: outside he reads your tracks
                        vPrintCD -= dt;
                        if (vPrintCD <= 0 && story.world.RoomAt(TX(visitor.pos.x), TX(visitor.pos.y)) == Room::Outside) {
                            vPrintCD = 1.0f;
                            for (auto it = prints.rbegin(); it != prints.rend(); ++it) {
                                if (it->who != 0 || it->t > 14) continue;
                                if (std::hypot(it->p.x - visitor.pos.x, it->p.y - visitor.pos.y) < 56) {
                                    vmode = VMode::Chase; vLostT = 0; vLastSeen = prints.back().who == 0 ? prints.back().p : it->p;
                                    AddBubble("The visitor", "Little feet.", 1.4f); vPrintCD = 5;
                                }
                                break;
                            }
                            if (vmode == VMode::Chase) break;
                        }
                        vBarkT -= dt;
                        if (vBarkT <= 0) { VisitorBark(kSearchLines, 6); vBarkT = (float)GetRandomValue(8, 13); }
                        if (vPause > 0) {
                            vPause -= dt;
                            if (std::fmod(vPause, 0.45f) < dt) visitor.dir = GetRandomValue(0, 3);
                        } else if (vPathI < vPath.size()) {
                            if (goTo(vPath[vPathI], searchSp, 0)) { ++vPathI; vPause = 0.8f; }
                        } else if (!vSpots.empty()) {
                            vSpotObj = vSpots.front(); vSpots.erase(vSpots.begin());
                            vmode = VMode::CheckSpot; vPause = 0;
                        } else PickNextRoom();
                        break;
                    case VMode::CheckSpot: {
                        const Obj& o = story.world.objs[vSpotObj];
                        Vector2 at = SpotNextTo(o, visitor.pos);
                        if (vPause <= 0 && goTo(at, searchSp, 0)) {
                            vPause = 1.0f;
                            Vector2 oc = TileCenter(o.x, o.y);
                            float dx = oc.x - visitor.pos.x, dy = oc.y - visitor.pos.y;
                            visitor.dir = std::fabs(dx) > std::fabs(dy) ? (dx > 0 ? 3 : 2) : (dy > 0 ? 0 : 1);
                        }
                        if (vPause > 0) {
                            vPause -= dt;
                            if (vPause <= 0) {
                                bool there = hidden && std::hypot(hidePos.x - TileCenter(o.x, o.y).x, hidePos.y - TileCenter(o.x, o.y).y) < 2;
                                if (there) { hidden = false; scene = SceneMode::None; story.OnFound(); StartNextBeat(); }
                                else { vmode = VMode::Search; vPause = 0.3f; }
                            }
                        }
                    } break;
                    case VMode::GoToHide:
                        if (!hidden) { vmode = VMode::Chase; vLostT = 0; break; }
                        if (goTo(hidePos, searchSp * 1.2f, 12)) { hidden = false; scene = SceneMode::None; story.OnFound(); StartNextBeat(); }
                        break;
                    }
                    // v0.7: he's closing in and Doggo is right there with you
                    if (scene == SceneMode::Chase && story.chase && story.doggoWithKid && vmode == VMode::Chase && !story.Tutorial() &&
                        std::hypot(visitor.pos.x - player.pos.x, visitor.pos.y - player.pos.y) < 36) {
                        scene = SceneMode::None; hidden = false;
                        story.DoggoSacrifice();
                        StartNextBeat();
                    }
                }
                story.visitorChasing = vmode == VMode::Chase;
            }
        }
        story.kidHidden = hidden;
        UpdatePrints(dt);
        if (!hidden) AddPrint(player, 0, dt, 0.2f);
        if (visitor.visible) AddPrint(visitor, 1, dt, 0.28f);
        if (doggo.visible && story.DoggoExists()) AddPrint(doggo, 2, dt, 0.12f);

        // v0.7: Doggo. Follows you unless you told him to stay. Can't open doors.
        if (!story.doggoDead) doggoShownDead = false;
        prevDoggoDead = story.doggoDead;
        if (story.doggoDead) { doggoDeadT += dt; doggo.visible = true; doggo.anim = 0; story.doggoWithKid = false; }
        else if (story.DoggoExists()) {
            doggo.visible = true;
            int gx = TX(player.pos.x), gy = TX(player.pos.y);
            float stop = 18;
            bool tut = story.Tutorial();
            if (story.t >= 35 * 60 && story.t < 40 * 60 && !story.collection) { gx = 20; gy = 19; stop = 0; }
            if (story.collection && tut) { gx = 16; gy = 18; stop = 0; }
            if (story.inBasement) { gx = 28; gy = 11; stop = 0; }
            float dd = std::hypot(doggo.pos.x - player.pos.x, doggo.pos.y - player.pos.y);
            if (!story.doggoStay) {
                gMover = 2;
                if (dd > TS * 14 && !story.collection && !story.inBasement) {
                    Vector2 st = NextStep(doggo.pos, gx, gy, false);
                    if (TX(st.x) != TX(doggo.pos.x) || TX(st.y) != TX(doggo.pos.y)) Snap(doggo, gx, gy);   // only if a door isn't in the way
                }
                MoveActor(doggo, gx, gy, story.Stage() >= 2 ? 90.0f : 76.0f, dt, false, stop);
                gMover = 0;
            } else doggo.anim = 0;
            bool sameRoom = story.world.RoomAt(TX(doggo.pos.x), TX(doggo.pos.y)) == story.world.RoomAt(TX(player.pos.x), TX(player.pos.y));
            story.doggoWithKid = story.collection && !tut && !story.doggoStay && sameRoom && dd < TS * 6;
            // instability, or the man at the door: barking fits
            if ((story.Act() >= 2 || scene == SceneMode::Chase) && !beatActive && barkT <= 0 && GetRandomValue(0, scene == SceneMode::Chase ? 200 : 2400) == 0) { barkT = 2.5f; }
            if (barkT > 0) {
                barkT -= dt;
                if (std::fmod(barkT, 0.35f) < dt) { Play("bark"); barkText = GetRandomValue(0, 1) ? "WOOF" : "WOOF WOOF"; AddBubble("Doggo", barkText, 0.6f); }
            }
        } else story.doggoWithKid = false;
    }

    // v0.7: if a door closed on your shoulder, slide back to the middle of your tile
    if (!CanStand(player.pos.x, player.pos.y)) {
        Vector2 c = TileCenter(TX(player.pos.x), TX(player.pos.y));
        if (CanStand(c.x, c.y)) player.pos = c;
    }

    // ---------------- draw world
    Camera2D cam{};
    float cxw = std::max(VW / 2.0f, std::min(World::W * TS - VW / 2.0f, player.pos.x));
    float cyw = std::max(VH / 2.0f, std::min(World::H * TS - VH / 2.0f, player.pos.y));
    cam.target = { std::round(cxw), std::round(cyw) };
    cam.offset = { VW / 2.0f, VH / 2.0f };
    cam.zoom = 1;
    worldCam = cam;
    BuildLightmap(cam);
    BeginTextureMode(target);
    DrawWorld(cam);
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    float ox = (SW - VW * US) / 2, oy = (SH - VH * US) / 2;
    Rectangle src{ 0, 0, (float)VW, -(float)VH };
    Rectangle dst{ ox, oy, VW * US, VH * US };
    DrawTexturePro(target.texture, src, dst, { 0, 0 }, 0, WHITE);
    if (story.Act() >= 2 && std::fmod(globalT, story.Act() >= 3 ? 14.0f : 31.0f) < 0.12f) {
        // flux: the world shudders
        DrawTexturePro(target.texture, src, { ox + 3 * US, oy, VW * US, VH * US }, { 0, 0 }, 0, Fade(RED, 0.25f));
    }

    Camera2D ui{}; ui.offset = { ox, oy }; ui.zoom = 1;
    BeginScissorMode((int)ox, (int)oy, (int)(VW * US), (int)(VH * US));
    BeginMode2D(ui);
    // overlays sized to the virtual screen
    int saveSW = SW, saveSH = SH;
    SW = (int)(VW * US); SH = (int)(VH * US);
    DrawBubbles(dt);
    DrawHUD();
    if (beatActive) {
        switch (cur.type) {
        case BeatType::Say: DrawDialogue(); break;
        case BeatType::Narrate: DrawNarrator(cur.text, typed); break;
        case BeatType::Black: DrawBlack(); break;
        case BeatType::Choice: DrawChoice(); break;
        case BeatType::Fade: DrawFade(); break;
        case BeatType::Card: DrawCard(); break;
        case BeatType::Ending: DrawCard(); break;
        case BeatType::QTE: DrawQTE(); break;
        case BeatType::EndLoop: DrawRectangle(0, 0, SW, SH, BLACK); break;
        default: break;
        }
    }
    if (journalOpen) DrawJournal();
    if (recapOpen) DrawRecap();
    if (miniOpen) DrawMini();
    if (jukeboxOpen) DrawJukebox();
    if (dbgOpen) DrawDebugPanel();
    if (pauseOpen) DrawPause();
    if (flashT > 0) { flashT -= dt; DrawRectangle(0, 0, SW, SH, Fade(settings.calmFlashes ? BLACK : WHITE, flashT)); }
    DrawToasts(dt);
    SW = saveSW; SH = saveSH;
    EndMode2D();
    EndScissorMode();
    BeginMode2D(ui);
    DrawTouchControls();
    DrawRotateHint();
    EndMode2D();
    EndDrawing();
}

#if defined(PLATFORM_WEB)
extern "C" EMSCRIPTEN_KEEPALIVE void OnResize(int w, int h) {
    if (!IsWindowReady() || w < 100 || h < 100) return;
    SetWindowSize(w, h);
    SW = w; SH = h;
}
#endif

int main() {
#if defined(PLATFORM_WEB)
    // No FLAG_WINDOW_RESIZABLE on web: raylib's own resize handler would reset the canvas to CSS
    // pixels and the browser would stretch it on scaled displays. The page calls OnResize with
    // real device pixels instead.
    SetConfigFlags(FLAG_VSYNC_HINT);
#else
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
#endif
    InitWindow(SW, SH, "600 Seconds - prototype");
#if !defined(PLATFORM_WEB)
    ChangeDirectory(GetApplicationDirectory());   // v0.9: find assets and saves next to the exe, however it was started
#endif
    SetExitKey(KEY_NULL);
#if defined(PLATFORM_WEB)
    {
        int w = EM_ASM_INT({ return Math.round(window.innerWidth * (window.devicePixelRatio || 1)); });
        int h = EM_ASM_INT({ return Math.round(window.innerHeight * (window.devicePixelRatio || 1)); });
        if (w > 100 && h > 100) { SetWindowSize(w, h); SW = w; SH = h; }
    }
#endif
    target = LoadRenderTexture(VW, VH);
    lightRT = LoadRenderTexture(VW, VH);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    {
        // Latin-1 plus curly quotes and ellipsis, rendered big and mipmapped so it stays soft at any size
        std::vector<int> cps;
        for (int c = 32; c < 127; ++c) cps.push_back(c);
        for (int c = 160; c < 256; ++c) cps.push_back(c);
        for (int c : { 0x2018, 0x2019, 0x201C, 0x201D, 0x2026, 0x2022 }) cps.push_back(c);
        auto load = [&](const char* path, int size, Font& f) {
            if (!FileExists(path)) return false;
            f = LoadFontEx(path, size, cps.data(), (int)cps.size());
            if (f.glyphCount <= 0) return false;
            GenTextureMipmaps(&f.texture);
            SetTextureFilter(f.texture, TEXTURE_FILTER_TRILINEAR);
            return true;
        };
        hasNarratorFont = load("assets/narrator.ttf", 64, narratorFont);
        hasUiFont = load("assets/ui.ttf", 72, uiFont);
    }
    InitSounds();
    LoadSettings();
    InitMusic();
    if (FileExists("assets/logo.png")) {
        logoTex = LoadTexture("assets/logo.png");
        GenTextureMipmaps(&logoTex);
        SetTextureFilter(logoTex, TEXTURE_FILTER_TRILINEAR);
        hasLogo = logoTex.id > 0;
    }
    {
        std::string sv = LoadSaveString();
        hasSave = !sv.empty();
        if (hasSave) story.LoadSave(sv);
    }
#if defined(PLATFORM_WEB)
    debugMode = EM_ASM_INT({ return location.search.indexOf('debug') >= 0 ? 1 : 0; });
#endif
    if (debugMode) LoadDebugCfg();   // your playtest settings only apply in debug mode
#if defined(PLATFORM_WEB)
    // phone mode: track touches ourselves (raylib's multi-touch bookkeeping drops fingers),
    // go fullscreen + landscape on the first tap, and re-fit the canvas when the phone rotates
    emscripten_run_script(R"JS((function () {
        var c = Module.canvas; if (c) {
        var T = window._t600 = { live: {}, taps: [], any: (window.matchMedia && matchMedia('(pointer: coarse)').matches && !matchMedia('(hover: hover)').matches) ? 1 : 0 };
        c.style.touchAction = 'none';
        function pos(t) { var r = c.getBoundingClientRect(); return [(t.clientX - r.left) * c.width / r.width, (t.clientY - r.top) * c.height / r.height]; }
        function upd(e) {
            e.preventDefault(); T.any = 1; T.live = {};
            for (var i = 0; i < e.touches.length; i++) T.live[e.touches[i].identifier] = pos(e.touches[i]);
        }
        var asked = false;
        c.addEventListener('touchstart', function (e) {
            upd(e);
            for (var i = 0; i < e.changedTouches.length; i++) T.taps.push(pos(e.changedTouches[i]));
            if (!asked) {
                asked = true;
                try {
                    var d = document, el = d.documentElement, rq = el.requestFullscreen || el.webkitRequestFullscreen;
                    if (rq && !d.fullscreenElement && !d.webkitFullscreenElement) {
                        var p = rq.call(el);
                        var lock = function () { try { screen.orientation.lock('landscape').catch(function () {}); } catch (x) {} };
                        if (p && p.then) p.then(lock).catch(function () {}); else lock();
                    }
                } catch (x) {}
            }
        }, { passive: false });
        c.addEventListener('touchmove', upd, { passive: false });
        c.addEventListener('touchend', upd, { passive: false });
        c.addEventListener('touchcancel', upd, { passive: false });
        var refit = function () { setTimeout(function () { window.dispatchEvent(new Event('resize')); }, 250); };
        window.addEventListener('orientationchange', refit);
        document.addEventListener('fullscreenchange', refit);
        }
    })();)JS");
#endif
    Snap(player, 5, 5); Snap(mom, 8, 16); Snap(dad, 24, 16); Snap(doggo, 6, 6); Snap(visitor, 20, 29);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) UpdateDrawFrame();
    CloseWindow();
#endif
    return 0;
}
