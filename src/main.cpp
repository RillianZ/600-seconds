// 600 Seconds - raylib presentation layer (prototype)
// Sund Origin Studio. All game rules live in src/game (engine-free).
// This file only draws, plays sounds and reads input.
#include "raylib.h"
#include "rlgl.h"
#include "game/Story.h"
#include "audio/Soundtrack.h"
#include "art/Bake.h"
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
static const char* kVersion = "v0.13.0";
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
enum class VMode { Grace, Search, Chase, Lost, CheckSpot, GoToHide, Investigate };
static VMode vmode = VMode::Grace;
static float vPause = 0, vLostT = 0, vBarkT = 0, vSpotCooldown = 0;
static std::vector<Vector2> vPath;     // waypoints in the current room
static size_t vPathI = 0;
// v0.12: the hunt. He hears, he follows wet prints, he falls for a marble (twice). You run out of breath
static Vector2 vNoise{};               // where the sound came from
static float vNoiseWait = 0, vNoiseT = 0, vHearCD = 0;
static bool vSure = false;             // the next room plan checks every spot
static float vShowT = 0;               // you only see him while you could really see him
static float stamina = 1, breath = 1, breathHeardT = 0, wetT = 0, wetCD = 0, vStepAcc = 0;
static bool winded = false, holdingBreath = false, gasped = false;
struct Marble { Vector2 from, to; float t, dur; bool landed; };
static std::vector<Marble> marbles;
struct Ring { Vector2 p; float t; int kind; };   // kind 0: his steps, heard not seen. 1: a noise you made
static std::vector<Ring> rings;
static void AddRing(Vector2 p, int kind) { rings.push_back({ p, 0, kind }); if (rings.size() > 40) rings.erase(rings.begin()); }
static void ResetHunt() {
    stamina = breath = 1; breathHeardT = wetT = wetCD = vStepAcc = 0; winded = holdingBreath = gasped = false;
    marbles.clear(); rings.clear(); vSure = false; vShowT = 0.4f; vHearCD = 0;
}
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
// v0.10: footsteps by surface (wood, tile, snow, stone). Set 0 is the kid, set 1 is him
static Sound sStep[2][4];
static bool doggoRanOff = false;        // he made it out through the gate
// v0.10: ambience and Mom's humming live in the music callback, so they loop without a seam.
// The frame sets targets, the audio thread glides to them
static volatile float gMusicVol = 0.7f, gSfxVol = 1.0f;
static volatile float aWindT = 0, aBrightT = 1, aHumT = 0, aStoveT = 0, aMomT = 0;
static volatile bool aHumSolo = false;   // the tape, and the dinner: nothing but her
static bool audioOk = false;
// v0.9: settings, saved apart from the game
struct Settings { int music = 7, sfx = 10, text = 1; bool calmFlashes = false; bool roofs = true; };
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
    for (int who = 0; who < 2; ++who) {
        sStep[who][0] = MakeSound(SFX_SR * 0.09f, [](float t, int i) { return (std::sin(t * 2 * PI * 95) * 0.7f + Noise(i) * 0.22f) * std::exp(-t * 38); });                       // planks
        sStep[who][1] = MakeSound(SFX_SR * 0.07f, [](float t, int i) { return std::sin(t * 2 * PI * 880) * 0.2f * std::exp(-t * 95) + Noise(i) * 0.3f * std::exp(-t * 75) + std::sin(t * 2 * PI * 150) * 0.3f * std::exp(-t * 50); });   // kitchen tiles
        sStep[who][2] = MakeSound(SFX_SR * 0.16f, [](float t, int i) { return Noise(i / 2) * (0.55f + 0.45f * std::sin(t * 150)) * std::exp(-t * 20) * std::min(1.0f, t / 0.012f) * 0.7f; });   // snow
        sStep[who][3] = MakeSound(SFX_SR * 0.08f, [](float t, int i) { return Noise(i) * 0.55f * std::exp(-t * 48) + std::sin(t * 2 * PI * 210) * 0.2f * std::exp(-t * 60); });           // gravel, concrete
    }
    sBark = MakeSound(SFX_SR * 0.25f, [](float t, int i) {
        float f = 300 + 400 * std::exp(-t * 20);
        return (std::sin(t * 2 * PI * f) * 0.6f + Noise(i) * 0.25f) * std::exp(-t * 14) * (t < 0.2f ? 1 : 0);
    });
}

static bool hushMusic = false;   // v0.9: the story asks for silence
static void Play(const std::string& id) {
    if (id == "hush_on") { hushMusic = true; return; }
    if (id == "hush_off") { hushMusic = false; aHumSolo = false; return; }
    if (id == "hum_on") { aHumSolo = true; return; }
    if (id == "hum_off") { aHumSolo = false; return; }
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
// audio thread only
static float aWind = 0, aBright = 1, aHum = 0, aStove = 0, aMom = 0, aSolo = 0, aLP1 = 0, aLP2 = 0, aCrack = 0;
static unsigned aSeed = 22222;
static double aT = 0;
static const float kWindGain = 1.6f;   // measured offline: full storm outside is about 0.057 RMS, the music tracks 0.035 to 0.06
static void MusicCallback(void* buffer, unsigned int frames) {
    float* out = (float*)buffer;
    const float lpA = 0.6f;   // gentle low-pass (~6.5 kHz), takes the edge off
    for (unsigned int i = 0; i < frames; ++i) {
        musicT += 1.0 / MUSIC_RATE;
        if (musicFade < 1) musicFade = std::min(1.0f, musicFade + 1.0f / (MUSIC_RATE * 2.5f));
        int curT = musicCur, prevT = musicPrev;
        float v = TrackSample(curT, musicT) * musicFade;
        if (musicFade < 1) v += TrackSample(prevT, musicT) * (1 - musicFade);
        v *= gMusicVol;
        // v0.10: glide to the frame's targets (about a third of a second)
        const float glide = 1.0f / (MUSIC_RATE * 0.35f);
        aWind += (aWindT - aWind) * glide; aBright += (aBrightT - aBright) * glide; aHum += (aHumT - aHum) * glide;
        aStove += (aStoveT - aStove) * glide; aMom += (aMomT - aMom) * glide; aSolo += ((aHumSolo ? 1.0f : 0.0f) - aSolo) * glide;
        // her song. It is the Act 1 tune, slow. In the kitchen it sits under the music, on the tape it is alone
        if (aMom > 0.002f || aSolo > 0.002f) v += HumSample(musicT) * (aMom * 0.04f + aSolo * 0.075f) * gSfxVol;
        // echo: more of it the darker it gets
        float wet = TrackEcho(curT);
        float e = echoBuf[echoPos];
        echoBuf[echoPos] = v + e * wet * 0.85f;
        echoPos = (echoPos + 1) % (int)echoBuf.size();
        v += e * wet;
        musicLP += (v - musicLP) * lpA;
        // ambience: wind (two soft low-passes on noise), the basement hum, the stove
        aT += 1.0 / MUSIC_RATE;
        aSeed = aSeed * 1664525u + 1013904223u;
        float n = ((aSeed >> 9) & 0xffff) / 32768.0f - 1.0f;
        float amb = 0;
        if (aWind > 0.002f) {
            float cut = 0.010f + 0.045f * aBright * (0.6f + 0.4f * (float)std::sin(aT * 0.37));
            aLP1 += (n - aLP1) * cut; aLP2 += (aLP1 - aLP2) * cut;
            float gust = 0.55f + 0.45f * (float)std::sin(aT * 0.53 + 1.7 * std::sin(aT * 0.11));
            amb += aLP2 * aWind * gust * kWindGain;
        }
        if (aHum > 0.002f)
            amb += aHum * (float)(0.016 * std::sin(aT * 2 * PI * 50) + 0.014 * std::sin(aT * 2 * PI * 100) + 0.008 * std::sin(aT * 2 * PI * 150.4) + 0.004 * std::sin(aT * 2 * PI * 200.9));
        if (aStove > 0.002f) {
            if ((aSeed & 0xfffff) < (unsigned)(aStove * 85)) aCrack = 0.25f + ((aSeed >> 20) & 0xff) / 255.0f * 0.5f;
            aCrack *= 0.9925f;
            amb += aCrack * n * 0.10f * aStove;
        }
        out[i] = std::tanh(musicLP * 1.1f + amb * gSfxVol) * 0.85f;
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
    SetAudioStreamVolume(musicStream, 1.0f);   // v0.10: music and ambience get their own gains inside the callback
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
    // v0.10: his car is solid, so walking up to it doesn't send you running onto the vidda. The left side of the gate stays open
    if (story.CarHere() && (x == 20 || x == 21) && (y == 31 || y == 32)) return false;
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
                                     Room::Bathroom, Room::ParentsRoom, Room::Shed, Room::Outside, Room::Basement };
static const int kRoomCount = 9;   // v0.13: the basement, only while its door is unlocked

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
static bool IsHideObj(const std::string& id) { return Story::IsHideSpot(id); }
// v0.12: he is only drawn while you have a line to him, or he is right on top of you
static bool VisitorShown() {
    if (!visitor.visible) return false;
    if (scene != SceneMode::Chase || !story.chase) return true;
    return vShowT > 0;
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
    // v0.12: a spot he has found you in before is always checked, and first
    float chance = vSure ? 1.0f : vSweep == 0 ? 0.7f : 1.0f;
    bool known = false;
    for (int i = 0; i < (int)story.world.objs.size(); ++i) {
        const Obj& o = story.world.objs[i];
        if (!IsHideObj(o.id) || story.world.RoomAt(o.x, o.y) != r) continue;
        if (story.CaughtIn(o.id) > 0) { known = true; vSpots.push_back(i); }
        else if (GetRandomValue(0, 999) < chance * 1000) vSpots.push_back(i);
    }
    if (vSpots.size() > 1 && GetRandomValue(0, 1)) std::swap(vSpots[0], vSpots[1]);
    std::stable_sort(vSpots.begin(), vSpots.end(), [](int a, int b) { return story.CaughtIn(story.world.objs[a].id) > story.CaughtIn(story.world.objs[b].id); });
    if (known && !vSure) { vPath.clear(); vPathI = 0; }   // no looking around first. He walks straight to it
    vSure = false;
}
static void PickNextRoom() {
    int best = -1; float bd = 1e9f;
    for (int pass = 0; pass < 2 && best < 0; ++pass) {
        for (int i = 0; i < kRoomCount; ++i) {
            if (vRoomDone[i]) continue;
            Room r = kSearchRooms[i];
            if (r == Room::Basement && !story.BasementInHunt()) continue;
            // rough distance to the room, house first, yard and shed last
            float sx = 0, sy = 0; int n = 0;
            for (int y = 0; y < World::H; y += 2) for (int x = 0; x < World::W; x += 2)
                if (story.world.RoomAt(x, y) == r) { sx += x; sy += y; ++n; }
            if (!n) continue;
            float d = std::hypot(sx / n * TS - visitor.pos.x, sy / n * TS - visitor.pos.y);
            if (r == Room::Outside || r == Room::Shed) d += 260;
            d += GetRandomValue(0, 60);
            if (vSweep == 0) for (auto& o : story.world.objs)   // v0.12: the rooms he has found you in come first
                if (IsHideObj(o.id) && story.world.RoomAt(o.x, o.y) == r) d -= 3000.0f * story.CaughtIn(o.id);
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
    ResetHunt();
    vPath.clear(); vSpots.clear(); vRoom = -1;
    if (dadRan) {
        vmode = VMode::Grace;               // he walks back in from the yard. Panic time
        vGraceGoal = TileCenter(20, 19);
        vGraceWait = 1.2f;
    } else {
        vmode = VMode::Grace;
        vGraceGoal = visitor.pos;
        vGraceWait = 2.5f;
        // v0.13: the hinges. At Mom's door he takes his time. Out at the shed he has a long walk back
        if (story.huntStart == "bath") { vGraceWait = 5.0f; visitor.dir = 1; }
        else if (story.huntStart == "shed") { vGraceWait = 1.5f; visitor.dir = 1; }
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
        if (b.type == BeatType::Clue) {
            ToastFor("NEW MEMORY  [J]", b.text, Color{ 190, 150, 255, 255 }); SaveGame();
            // v0.10: the log opens where the new memory is
            for (auto& m : AllMemories()) if (m.id == b.id) {
                int pos = 0;
                for (auto& o : AllMemories()) { if (o.id == m.id) break; if (o.topic == m.topic) pos++; }
                for (size_t k = 0; k < AllTopics().size(); ++k) if (AllTopics()[k] == m.topic) { journalTopic = (int)k + 1; journalScroll = std::max(0, pos - 3); }
            }
            continue;
        }
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
        else if (f.type == BeatType::Sound) { if (f.id.rfind("hush", 0) == 0 || f.id.rfind("hum_", 0) == 0) Play(f.id); story.beats.pop_front(); }
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
            hushMusic = false; aHumSolo = false;
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
            if (cur.tx >= 0) { Snap(player, cur.tx, cur.ty); player.dir = 0; if (story.DoggoExists() && !story.doggoStay && cur.ty < 34) Snap(doggo, cur.tx + 1, cur.ty); }   // v0.10: he never comes down the basement stairs
            // v0.13: the hinges. A blink of black while he walks to Mom's door, or out to the shed
            if (cur.id == "hinge_bath") { Snap(visitor, 13, 10); visitor.dir = 1; }
            if (cur.id == "hinge_shed") { Snap(visitor, 34, 11); visitor.dir = 1; }
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
            hushMusic = false; aHumSolo = false;
            // v0.6: Outer Wilds style recap of what you learned this loop
            recapList = story.lastLoopNew;
            if (!recapList.empty()) { recapOpen = true; recapT = 0; recapShown = 0; }
            prints.clear(); ResetHunt();
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
    if (!story.collection && story.CarHere()) {
        // v0.10: his car, just past the gate. Only while he is away from it
        Vector2 cc = { 20 * TS + 8.0f, 31 * TS + 13.0f };
        if (!visitor.visible || std::hypot(visitor.pos.x - cc.x, visitor.pos.y - cc.y) > 44)
            consider("car", "His car", std::hypot(cc.x - front.x, cc.y - front.y), 24, 0);
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
#include "art/Sprites.inc"
#include "art/WorldDraw.inc"

// ============================================================ UI drawing (screen space)
static void Panel(float x, float y, float w, float h, Color bg, Color border) {
    // v0.11: a soft shadow under every panel, and a little light along its top edge
    float u = std::max(1.0f, US);
    if (bg.a > 120) DrawRectangle((int)(x * US + u), (int)(y * US + 2 * u), (int)(w * US), (int)(h * US), Fade(BLACK, 0.35f));
    DrawRectangle((int)(x * US), (int)(y * US), (int)(w * US), (int)(h * US), bg);
    if (bg.a > 120 && h > 8) DrawRectangleGradientV((int)(x * US), (int)(y * US), (int)(w * US), (int)(std::min(h, 14.0f) * US), Fade(WHITE, 0.07f), Fade(WHITE, 0));
    DrawRectangleLinesEx({ x * US, y * US, w * US, h * US }, u, border);
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
    Color nc = cur.speaker == "The visitor" ? Color{ 120, 20, 20, 255 } : cur.speaker == "You" ? Color{ 160, 50, 50, 255 } : cur.speaker == "Mom" ? Color{ 46, 100, 76, 255 } :
               cur.speaker == "Doggo" ? Color{ 140, 96, 52, 255 } : Color{ 50, 70, 120, 255 };
    // v0.11: whoever is talking gets a face, standing on the top edge of the box
    float tagX = x + 8;
    const std::string& sp = cur.speaker;
    if (sp == "You" || sp == "Mom" || sp == "Dad" || sp == "The visitor" || sp == "Doggo") {
        const float ps = 38;
        Color sky = sp == "The visitor" ? Color{ 20, 10, 16, 255 } : Mix(nc, Color{ 16, 16, 28, 255 }, 0.55f);
        Panel(x + 8, y - ps + 2, ps, ps, sky, Color{ 230, 230, 240, 255 });
        DrawPortrait(sp, (x + 8 + ps / 2) * US, (y - ps + 2 + ps / 2) * US, 2.0f * US);
        tagX = x + 8 + ps + 4;
    }
    DrawRectangle((int)(x * US), (int)(y * US), (int)(w * US), (int)(h * US), Color{ 16, 16, 28, 255 });
    Panel(x, y, w, h, Color{ 16, 16, 28, 235 }, Color{ 230, 230, 240, 255 });
    std::string vis = cur.text.substr(0, std::min<size_t>(cur.text.size(), (size_t)typed));
    if (!cur.speaker.empty()) {
        float nw = UIMeasure(cur.speaker.c_str(), 10) + 12;
        Panel(tagX, y - 12, nw, 15, nc, WHITE);
        UIText(cur.speaker.c_str(), tagX + 6, y - 9, 10, WHITE);
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
    // v0.12: sounds. His steps when you can't see him, and the noise you make yourself
    for (size_t i = 0; i < rings.size();) {
        Ring& r = rings[i];
        r.t += dt;
        if (r.t > 0.8f) { rings.erase(rings.begin() + i); continue; }
        ++i;
        Vector2 c = WorldToUI({ r.p.x, r.p.y + 5 });
        float k = r.t / 0.8f;
        Color col = r.kind == 0 ? Color{ 235, 120, 120, 255 } : Color{ 245, 220, 130, 255 };
        DrawEllipseLines((int)(c.x * US), (int)(c.y * US), (3 + k * 13) * US, (1.6f + k * 7) * US, Fade(col, 0.7f * (1 - k)));
        DrawEllipseLines((int)(c.x * US), (int)(c.y * US), (2 + k * 9) * US, (1.0f + k * 5) * US, Fade(col, 0.4f * (1 - k)));
    }
    if (scene == SceneMode::Chase && VisitorShown() && !beatActive && downedT <= 0) {
        const char* mark = vmode == VMode::Chase ? "!" : vmode == VMode::Lost || vmode == VMode::Investigate ? "?" : nullptr;
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
            if (story.ShouldHide(tg.id)) tg.name = "Hide: " + tg.name;
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
    const char* t = scene == SceneMode::DadRuns ? "DAD IS FIGHTING HIM. RUN." : hidden ? (touchMode ? "HIDING  (E: out, RUN: hold breath)" : "HIDING  (E: out, Shift: hold breath)") :
                    story.threats >= 3 ? "HE'S DONE WAITING" : story.threats > 0 ? "HE'S HURTING THEM" : "THE LAST 600 SECONDS";
    float p = 0.5f + 0.5f * std::sin(globalT * 5);
    UIText(t, VW / 2.0f - UIMeasure(t, 10) / 2, 40, 10, Fade(Color{ 240, 70, 70, 255 }, 0.6f + 0.4f * p));
    if (scene == SceneMode::Chase) {
        const char* h = "";
        if (story.visitorDowned) h = "He's down. For now.";
        else if (hidden) {
            bool near = vmode == VMode::GoToHide || (vmode == VMode::CheckSpot && vSpotObj >= 0 &&
                        story.world.RoomAt(story.world.objs[vSpotObj].x, story.world.objs[vSpotObj].y) == story.world.RoomAt(TX(hidePos.x), TX(hidePos.y)));
            h = gasped ? "You gasped." : holdingBreath ? "Holding it." : near || breathHeardT > 0 ? "He's close. HOLD YOUR BREATH." : "Stay still. Listen.";
        }
        else if (vmode == VMode::Investigate) h = "He heard something. He's going to look.";
        else if (winded) h = "Out of breath. You can't run yet.";
        else if (vmode == VMode::Grace) h = "He's coming back in. Get out of sight.";
        else if (vmode == VMode::Chase) h = "HE SEES YOU. Run (Shift). Break line of sight.";
        else if (vmode == VMode::Lost) h = "He lost you. Keep moving.";
        else if (story.threats > 0) h = "Come out and face him (E). Or don't.";
        else h = story.pebbles > 0 ? (touchMode ? "He's searching. Walk, don't run. THROW a marble to move him." : "He's searching. Walk, don't run. Q throws a marble.")
                                   : "He's searching. Walk, don't run. He hears running. Hide (E) or face him (E).";
        if (story.cold > 0.3f) h = story.hasCoat ? "It's getting into your jacket. Get inside." : "You're freezing. Get inside.";
        else if (story.doggoWithKid && story.Knows("doggo_stay") && !hidden) h = "Doggo is right next to you. Tell him to stay (E on Doggo).";
        UIText(h, VW / 2.0f - UIMeasure(h, 6) / 2, 54, 6, Fade(WHITE, 0.65f));
        // v0.12: air and legs
        auto bar = [&](float y, float v, const char* label, Color c) {
            float w = 60, x = VW / 2.0f - w / 2;
            DrawRectangleRounded({ (x - 1) * US, (y - 1) * US, (w + 2) * US, 5 * US }, 0.6f, 6, Fade(BLACK, 0.6f));
            DrawRectangleRounded({ x * US, y * US, std::max(0.0f, v) * w * US, 3 * US }, 0.6f, 6, Fade(c, 0.9f));
            UIText(label, x - UIMeasure(label, 5) - 4, y - 1, 5, Fade(WHITE, 0.6f));
        };
        if (hidden) bar(64, breath, "AIR", breath < 0.3f ? Color{ 240, 80, 80, 255 } : Color{ 150, 200, 255, 255 });
        else if (stamina < 0.995f) bar(64, stamina, "LEGS", winded ? Color{ 240, 80, 80, 255 } : Color{ 240, 210, 110, 255 });
        if (story.pebbles > 0 && !hidden) {
            std::string m = std::string(touchMode ? "THROW" : "Q") + ": marble x" + std::to_string(story.pebbles);
            UIText(m.c_str(), VW / 2.0f - UIMeasure(m.c_str(), 5) / 2, 71, 5, Fade(Color{ 245, 220, 130, 255 }, 0.8f));
        }
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
        if (scene == SceneMode::Chase) {
            static const char* vm[] = { "grace", "search", "chase", "lost", "check spot", "go to your spot", "investigate" };
            rs += std::string("  hunt: ") + vm[(int)vmode] + "  sweep " + std::to_string(vSweep) + "  wet " + std::to_string((int)wetT);
        }
        if (visitorBlind) rs += "  VISITOR BLIND";
        UIText(rs.c_str(), 8, VH - 22, 6, Color{ 120, 220, 140, 200 });
    }
    DrawChaseBanner();
    if (IsKeyDown(KEY_T)) UIText(">> waiting", VW / 2.0f - 25, VH - 14, 8, YELLOW);
    else if (!touchMode) UIText("T: wait   J: memory   E: use", 8, VH - 12, 6, Fade(WHITE, 0.45f));
}

// v0.10: the log has three more pages. Tab 0 is the hour as a timeline, then the topics, the endings, the awards
static int JournalTabs() { return (int)AllTopics().size() + 3; }

static void DrawTimeline() {
    const Color ice = { 150, 210, 230, 255 }, dim = { 86, 88, 100, 255 };
    if (story.Tutorial()) {
        // loop 1: he has no idea there is anything to keep track of
        const char* l1 = "Christmas Eve. One hour until Julenissen.";
        const char* l2 = "What is there to keep track of?";
        UIText(l1, VW / 2.0f - UIMeasure(l1, 10) / 2, 118, 10, Fade(WHITE, 0.8f));
        UIText(l2, VW / 2.0f - UIMeasure(l2, 8) / 2, 136, 8, dim);
        return;
    }
    auto& tl = AllTimeline();
    const float top = 53, rowH = 12.2f;
    int next = -1;
    for (size_t i = 0; i < tl.size(); ++i) if (next < 0 && tl[i].minute * 60 + tl[i].second > story.t) next = (int)i;
    DrawRectangle((int)(61 * US), (int)((top + 3) * US), (int)std::max(1.0f, US), (int)((tl.size() - 1) * rowH * US), Fade(ice, 0.25f));
    for (size_t i = 0; i < tl.size(); ++i) {
        auto& e = tl[i];
        float y = top + i * rowH;
        bool known = story.KnowsEvent(e.id), past = e.minute * 60 + e.second <= story.t;
        char tb[16]; std::snprintf(tb, sizeof tb, "%02d:%02d", 16 + e.minute / 60, e.minute % 60);
        if ((int)i == next) {
            DrawRectangle((int)(16 * US), (int)((y - 2) * US), (int)((VW - 32) * US), (int)((rowH - 1) * US), Fade(ice, 0.10f));
            UIText(">", 18, y, 8, Color{ 255, 210, 90, 255 });
        }
        UIText(tb, 28, y, 8, known ? (past ? Fade(ice, 0.5f) : ice) : dim);
        DrawCircleV({ 61.5f * US, (y + 4.2f) * US }, (known ? 2.2f : 1.5f) * US, known ? (past ? Fade(ice, 0.5f) : ice) : dim);
        UIText(known ? e.text.c_str() : "?", 70, y, 8, known ? (past ? Fade(WHITE, 0.5f) : WHITE) : dim);
    }
    std::string now = "NOW " + story.ClockText() + (story.collection ? "   the last 600 seconds" : "");
    UIText(now.c_str(), VW - 16 - UIMeasure(now.c_str(), 7), 53, 7, Color{ 255, 210, 90, 255 });
}

static void DrawJournal() {
    DrawRectangle(0, 0, SW, SH, Fade(Color{ 10, 8, 14, 255 }, 0.94f));
    bool dark = story.Stage() >= 2;
    UIText("MEMORY LOG", 16, 12, 14, dark ? Color{ 200, 60, 60, 255 } : Color{ 190, 150, 255, 255 });
    std::string pct = std::to_string(story.Awareness() * 100 / AllMemories().size()) + "%   endings " + std::to_string(story.endings.size()) + "/" + std::to_string(AllEndings().size()) +
                      "   awards " + std::to_string(story.achievements.size()) + "/" + std::to_string(AllAchievements().size());
    UIText(pct.c_str(), VW - 16 - UIMeasure(pct.c_str(), 8), 16, 8, WHITE);
    auto& topics = AllTopics();
    const int N = (int)topics.size(), nTabs = JournalTabs(), tabEnd = N + 1, tabAwards = N + 2;
    const Color teal = { 120, 200, 220, 255 };
    float tx = 16;
    for (int i = 0; i < nTabs; ++i) {
        std::string name = i == 0 ? "Timeline" : i <= N ? topics[i - 1] : i == tabEnd ? "Endings" : "Awards";
        if (name.rfind("The ", 0) == 0) { name = name.substr(4); name[0] = (char)std::toupper((unsigned char)name[0]); }
        float w = UIMeasure(name.c_str(), 6.5f) + 7;
        bool sel = i == journalTopic, gold = i >= tabEnd;
        int found = 0;
        if (i == 0) { for (auto& e : AllTimeline()) if (story.KnowsEvent(e.id)) found++; }
        else if (i == tabEnd) found = (int)story.endings.size();
        else if (i == tabAwards) found = (int)story.achievements.size();
        else for (auto& e : AllMemories()) if (e.topic == topics[i - 1] && story.Knows(e.id)) found++;
        Color lit = gold ? GOLD : i == 0 ? teal : Color{ 190, 150, 255, 255 };
        Color fill = sel ? (gold ? Color{ 90, 74, 30, 255 } : i == 0 ? Color{ 28, 70, 84, 255 } : Color{ 70, 50, 110, 255 }) : Color{ 25, 22, 35, 255 };
        Panel(tx, 32, w, 13, fill, found ? lit : Color{ 80, 80, 90, 255 });
        UIText(name.c_str(), tx + 3.5f, 35.2f, 6.5f, found ? WHITE : GRAY);
        tx += w + 2;
    }
    const float top = 52, bottom = VH - 22;
    if (journalTopic == 0) {
        DrawTimeline();
        const char* help = touchMode ? "What I've watched happen. A ? is still worth being around for.    tap: close"
                                     : "What I've watched happen. A ? is still worth being around for.    LEFT/RIGHT page    J or ESC close";
        UIText(help, 16, VH - 14, 7, Fade(WHITE, 0.5f));
        return;
    }
    if (journalTopic == tabAwards) {
        // compact list: the name once you have it, a nudge until then
        auto& all = AllAchievements();
        const float rowH = 14;
        int visible = (int)((bottom - top) / rowH);
        int maxScroll = std::max(0, (int)all.size() - visible);
        journalScroll = std::max(0, std::min(journalScroll, maxScroll));
        float y = top;
        for (int idx = journalScroll; idx < (int)all.size() && idx < journalScroll + visible; ++idx) {
            bool have = story.achievements.count(all[idx].name) > 0;
            if (idx % 2 == 0) DrawRectangle((int)(16 * US), (int)((y - 1.5f) * US), (int)((VW - 32) * US), (int)((rowH - 1) * US), Fade(WHITE, 0.03f));
            if (have) {
                DrawPoly({ 23 * US, (y + 4.5f) * US }, 4, 3.4f * US, 45 + globalT * 20, GOLD);
                UIText(all[idx].name.c_str(), 32, y, 8, GOLD);
                UIText(all[idx].hint.c_str(), 200, y + 0.5f, 7, Fade(WHITE, 0.45f));
            } else {
                UIText("?", 21, y, 8, Color{ 80, 80, 90, 255 });
                UIText(all[idx].hint.c_str(), 32, y, 8, Color{ 120, 120, 135, 255 });
            }
            y += rowH;
        }
        if (journalScroll > 0) UIText("^ more", VW - 60, top - 9, 7, Fade(WHITE, 0.6f));
        if (journalScroll < maxScroll) UIText("v more", VW - 60, bottom - 2, 7, Fade(WHITE, 0.6f));
        const char* help = touchMode ? "D-pad: page / scroll    tap: close" : "LEFT/RIGHT page    UP/DOWN scroll    J or ESC close";
        UIText(help, touchMode ? 120 : 16, VH - 14, 7, Fade(WHITE, 0.5f));
        return;
    }
    // entries for this tab, scrollable (v0.6)
    struct Row { bool known; std::string title, text, hint; bool ending; };
    std::vector<Row> rows;
    if (journalTopic == tabEnd) {
        for (auto& e : AllEndings()) rows.push_back({ story.endings.count(e.id) > 0, e.title, "Found.", e.hint, true });
    } else {
        for (auto& e : AllMemories()) if (e.topic == topics[journalTopic - 1]) rows.push_back({ story.Knows(e.id), e.title, e.text, e.hint, false });
    }
    const float rowH = 42;
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
    const char* help = touchMode ? "D-pad: page / scroll    tap: close" : "LEFT/RIGHT page    UP/DOWN scroll    J or ESC close";
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
    case D_FORCE: if (tog) { int f = ((int)c.forceNext + (dir < 0 ? 4 : 1)) % 5; c.forceNext = (Shift)f; } break;
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
        const char* d1 = "DEBUG: O panel, M jukebox, 3 gate, 4 basement + tape, 5 living room, 6 chores, 7 Fliptendo, 8 +5, 9 16:49, 0 knock";
        const char* d2 = hardResetArm > 0 ? "Shift+R again to WIPE EVERYTHING (memories, darkness, endings, achievements)" : "Shift+R: hard reset";
        // v0.9.3: top left, the bottom of the title is full
        UIText(d1, 8, 8, 6, Color{ 120, 220, 140, 200 });
        UIText(d2, 8, 18, 6, hardResetArm > 0 ? Color{ 240, 80, 80, 255 } : Color{ 120, 220, 140, 200 });
    }
    const char* c2 = "Contains off-screen violence, implied child abuse and the death of a dog. Flashing lights.";
    UIText(c2, VW / 2.0f - UIMeasure(c2, 6) / 2, 236, 6, Fade(WHITE, 0.35f));
}

// ============================================================ v0.9 pause menu and settings
static void SaveSettings() {
    std::string s = std::to_string(settings.music) + "|" + std::to_string(settings.sfx) + "|" + std::to_string(settings.text) + "|" + std::to_string(settings.calmFlashes ? 1 : 0) + "|" + std::to_string(settings.roofs ? 1 : 0);
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
    int a, b, c, d, e = 1;
    if (std::sscanf(s.c_str(), "%d|%d|%d|%d|%d", &a, &b, &c, &d, &e) >= 4) {
        settings.roofs = e != 0;
        settings.music = std::max(0, std::min(10, a)); settings.sfx = std::max(0, std::min(10, b));
        settings.text = std::max(0, std::min(3, c)); settings.calmFlashes = d != 0;
    }
}
static const int PAUSE_ROWS = 7;
static const float PAUSE_Y = 76, PAUSE_H = 20, PAUSE_X = 110, PAUSE_W = 260;
static void DrawPause() {
    DrawRectangle(0, 0, (int)(VW * US), (int)(VH * US), Fade(Color{ 8, 8, 14, 255 }, 0.86f));
    UIText("PAUSED", VW / 2.0f - UIMeasure("PAUSED", 16) / 2, 48, 16, Color{ 230, 60, 60, 255 });
    static const char* speed[4] = { "Slow", "Normal", "Fast", "Instant" };
    std::string val[PAUSE_ROWS] = { "", std::to_string(settings.music) + " / 10", std::to_string(settings.sfx) + " / 10", speed[settings.text],
                                    settings.calmFlashes ? "Calm" : "Normal", settings.roofs ? "On" : "Off", "" };
    static const char* name[PAUSE_ROWS] = { "Resume", "Music", "Sound", "Text speed", "Flashes", "Roofs", "Back to the title" };
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
    case 1: if (dir) { settings.music = std::max(0, std::min(10, settings.music + dir)); } break;
    case 2: if (dir) { settings.sfx = std::max(0, std::min(10, settings.sfx + dir)); Play("chime"); } break;
    case 3: if (dir) settings.text = std::max(0, std::min(3, settings.text + dir)); break;
    case 4: if (dir || use) settings.calmFlashes = !settings.calmFlashes; break;
    case 5: if (dir || use) settings.roofs = !settings.roofs; break;
    case 6:
        if (use) {
            pauseOpen = false; SaveSettings(); SaveGame();
            story.beats.clear(); beatActive = false; hushMusic = false; aHumSolo = false; hidden = false; recapOpen = false; journalOpen = false;
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
    TouchButton(l.wait, l.sr, story.chase ? "THROW" : "WAIT", vDown[KEY_T], 6);
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
#if !defined(PLATFORM_WEB)
    // v0.9.1: the page handles F in the browser. The exe has to do it itself
    if (IsKeyPressed(KEY_F)) ToggleBorderlessWindowed();
#endif
    int cw = GetScreenWidth(), ch = GetScreenHeight();
    if (cw != SW || ch != SH) { SW = cw; SH = ch; }
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
    // v0.10: ambience follows where the kid is standing. The audio thread glides to these
    {
        gMusicVol = settings.music / 10.0f; gSfxVol = settings.sfx / 10.0f;
        float wind = 0, bright = 1, hum = 0, stove = 0, momHum = 0;
        if (screen == Screen::Title) { wind = 0.2f; bright = 0.55f; }
        else if (screen == Screen::Game && !hushMusic && !pauseOpen && !journalOpen && !miniOpen && !recapOpen) {
            int px = TX(player.pos.x), py = TX(player.pos.y);
            Room r = story.world.RoomAt(px, py);
            bool storm = story.Storm() || story.collection;
            if (story.inBasement || r == Room::Basement) hum = 1;
            else if (r == Room::Outside || r == Room::Vidda) wind = storm ? 1.0f : 0.32f;
            else {
                wind = storm ? 0.42f : 0.07f; bright = 0.3f;   // through the walls
                float db = std::hypot((float)(px - World::BASEMENT_X), (float)(py - World::BASEMENT_Y));
                if (db < 5) hum = (story.BasementOpen() ? 0.6f : 0.22f) * (1 - db / 5);
                float dsv = std::hypot((float)(px - 27), (float)(py - 18));
                if (r == Room::LivingRoom && dsv < 6) stove = 1 - dsv / 6;
                if (story.MomHums() && mom.visible) {
                    float dm = std::hypot(mom.pos.x - player.pos.x, mom.pos.y - player.pos.y) / TS;
                    if (dm < 9) momHum = (1 - dm / 9) * (story.world.RoomAt(TX(mom.pos.x), TX(mom.pos.y)) == r ? 1.0f : 0.45f);
                }
            }
        }
        aWindT = wind; aBrightT = bright; aHumT = hum; aStoveT = stove; aMomT = momHum;
        if (screen != Screen::Game) aHumSolo = false;
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
        DrawTitleScene();
        BeginDrawing();
        float ox = (SW - VW * US) / 2, oy = (SH - VH * US) / 2;
        ClearBackground(BLACK);
        DrawTexturePro(target.texture, { 0, 0, (float)VW, -(float)VH }, { ox, oy, VW * US, VH * US }, { 0, 0 }, 0, WHITE);
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
        int n = (int)AllTopics().size() + 3;   // Timeline + topics + Endings + Awards
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
        // v0.12: in the hunt you can't sprint forever
        if (story.chase) {
            if (winded && stamina > 0.35f) winded = false;
            if (winded) running = false;
        } else { stamina = 1; winded = false; }
        kidRunning = running && !story.kidShot && (mv.x != 0 || mv.y != 0) && !hidden;
        if (story.chase) {
            if (kidRunning) { stamina -= dt / 3.5f; if (stamina <= 0) { stamina = 0; winded = true; } }
            else stamina = std::min(1.0f, stamina + dt / ((mv.x != 0 || mv.y != 0) ? 9.0f : 5.0f));
        }
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

        // v0.12: throw a marble the way you are facing
        if (story.chase && scene == SceneMode::Chase && !hidden && story.pebbles > 0 && (IsKeyPressed(KEY_Q) || (touchMode && vPress[KEY_T]))) {
            const float fx[4] = { 0, 0, -1, 1 }, fy[4] = { 1, -1, 0, 0 };
            Vector2 at = player.pos, land = player.pos;
            for (int i = 0; i < 26; ++i) {
                at.x += fx[player.dir] * 4; at.y += fy[player.dir] * 4;
                if (BlocksSight(TX(at.x), TX(at.y))) break;
                if (story.world.Walkable(TX(at.x), TX(at.y), false)) land = at;
            }
            float d = std::hypot(land.x - player.pos.x, land.y - player.pos.y);
            if (d > 10) { marbles.push_back({ player.pos, land, 0, d / 230.0f, false }); story.pebbles--; }
        }
        if (interactCooldown > 0) interactCooldown -= dt;
        else if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE)) {
            if (hidden) {
                hidden = false; breathHeardT = 0; gasped = false;
            } else {
                Target tg = FindTarget();
                if (!tg.id.empty()) {
                    if (story.ShouldHide(tg.id)) {
                        hidden = true; hideT = 0; story.hideSpot = tg.id; breathHeardT = 0; gasped = false;
                        const Obj* o = nullptr;
                        for (auto& ob : story.world.objs) if (ob.id == tg.id) o = &ob;
                        hidePos = o ? TileCenter(o->x, o->y) : player.pos;
                    } else if (tg.id == lastTalkId && globalT - lastTalkT < 0.9f) {
                        // v0.6: a stray extra click right after a conversation doesn't restart it
                    } else { talkingTo = tg.id; lastTalkId = tg.id; story.Interact(tg.id, TX(player.pos.x), TX(player.pos.y)); }
                }
            }
        }
        // v0.13 debug: H sets up the next hinge. A: key in hand, outside the bathroom at 16:46. B: at the shed, Dad inside. C: key in hand at the basement door
        if (debugMode && IsKeyPressed(KEY_H) && !story.collection && !story.Tutorial()) {
            static int hinge = 0;
            int x = TX(player.pos.x), y = TX(player.pos.y);
            if (hinge == 0) { story.Hold(Item::Key, true); story.AdvanceMinutes(std::max(0, 46 - (int)(story.t / 60)), x, y); story.t = std::max(story.t, 46 * 60 + 25.0f); Snap(player, 13, 10); player.dir = 1; ToastFor("DEBUG", "Hinge A: Mom goes in at 16:46. Lock her in", SKYBLUE); }
            else if (hinge == 1) { if (story.t < 9 * 60) story.AdvanceMinutes(9 - (int)(story.t / 60), x, y); Snap(player, 34, 11); player.dir = 1; ToastFor("DEBUG", "Hinge B: Dad is in the shed. Bar the door", SKYBLUE); }
            else { story.Hold(Item::Key, true); if (story.t < 45 * 60) story.AdvanceMinutes(45 - (int)(story.t / 60), x, y); Snap(player, 28, 10); player.dir = 3; ToastFor("DEBUG", "Hinge C: go down and wait", SKYBLUE); }
            hinge = (hinge + 1) % 3;
        }
        if (debugMode && IsKeyPressed(KEY_FOUR) && !story.inBasement && !story.collection) story.DebugToBasement();   // debug: the tape, downstairs
        if (debugMode && IsKeyPressed(KEY_THREE) && !story.inBasement) { Snap(player, 20, 29); player.dir = 0; }   // debug: hop to the gate
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
        story.momRoomNow = story.world.RoomAt(TX(mom.pos.x), TX(mom.pos.y));   // v0.13: hinge A needs her really in there

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
                if (x == 13 && y == 9 && story.momBathLocked) continue;   // v0.13: the key is on your side
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
                if (!story.collection) {
                    bool visitorTalking = beatActive && talkingTo == "visitor";
                    if (!vs.visible) Snap(visitor, vs.x, vs.y);               // v0.9.2: he's in the car
                    else if (visitorTalking) face(visitor);
                    else MoveActor(visitor, vs.x, vs.y, 50, dt, false);
                    if (vs.facingWall && !visitorTalking && TX(visitor.pos.x) == vs.x && TX(visitor.pos.y) == vs.y) visitor.dir = 1;
                }
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
            // v0.13: the basement is its own floor. He takes the stairs (the top is by the basement door)
            {
                bool vDown = visitor.pos.y >= 34 * TS, tDown = ty >= 34;
                if (vDown != tDown) {
                    int sx = vDown ? 6 : 28, sy = vDown ? 38 : 11;
                    Vector2 sc = TileCenter(sx, sy);
                    if (std::hypot(visitor.pos.x - sc.x, visitor.pos.y - sc.y) < 6) {
                        Snap(visitor, vDown ? 28 : 6, vDown ? 11 : 38);
                        Play("door"); AddRing(visitor.pos, 0);
                        return false;
                    }
                    tx = sx; ty = sy;
                }
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
                    // ---- v0.12: what he hears
                    bool busy = vmode == VMode::Grace || vmode == VMode::GoToHide || (vmode == VMode::Chase && (sees || vLostT < 0.3f));
                    float dk = std::hypot(visitor.pos.x - player.pos.x, visitor.pos.y - player.pos.y);
                    auto hear = [&](Vector2 at, float wait) { vmode = VMode::Investigate; vNoise = at; vNoiseWait = wait; vNoiseT = 0; vPause = 0; };
                    vHearCD -= dt;
                    if (!busy && !hidden && !visitorBlind) {
                        bool loudOut = IsOutsideT(TileAt(TX(player.pos.x), TX(player.pos.y + 6))) && story.Storm();
                        if (kidRunning && dk < TS * (loudOut ? 5.0f : 10.0f)) {
                            // running feet carry through walls. In the storm, less
                            if (vHearCD <= 0) { vHearCD = 0.45f; AddRing(player.pos, 1); }
                            bool first = vmode != VMode::Investigate;
                            hear(player.pos, 1.2f);
                            if (first) { story.OnHeard("run"); StartNextBeat(); }
                        } else if (player.anim > 0 && dk < TS * 2.6f) {
                            // a walking kid he only hears when he is close. He turns around
                            float dx = player.pos.x - visitor.pos.x, dy = player.pos.y - visitor.pos.y;
                            visitor.dir = std::fabs(dx) > std::fabs(dy) ? (dx > 0 ? 3 : 2) : (dy > 0 ? 0 : 1);
                            if (!LineOfSight(visitor.pos, player.pos) && vmode != VMode::Investigate) hear(player.pos, 1.0f);
                        }
                    }
                    // ---- holding your breath in a hiding spot
                    if (hidden) {
                        float dh = std::hypot(visitor.pos.x - hidePos.x, visitor.pos.y - hidePos.y);
                        bool close = dh < TS * 3.2f && LineOfSight(visitor.pos, hidePos);
                        bool want = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
                        if (!want) gasped = false;
                        holdingBreath = want && !gasped && breath > 0;
                        if (holdingBreath) {
                            breath -= dt / 6.0f;
                            if (breath <= 0) {
                                breath = 0; gasped = true; holdingBreath = false; AddRing(hidePos, 1);
                                if (!busy && dh < TS * 9 && !visitorBlind) { story.OnHeard("gasp"); StartNextBeat(); vmode = VMode::GoToHide; }
                            }
                        } else breath = std::min(1.0f, breath + dt / 7.0f);
                        if (close && !holdingBreath && !busy && !visitorBlind) {
                            breathHeardT += dt;
                            if (breathHeardT > 0.9f) { story.OnHeard("breath"); StartNextBeat(); vmode = VMode::GoToHide; breathHeardT = 0; }
                        } else breathHeardT = std::max(0.0f, breathHeardT - dt * 0.5f);
                    } else { holdingBreath = false; breath = std::min(1.0f, breath + dt / 7.0f); }
                    // ---- marbles in the air
                    for (auto& m : marbles) {
                        if (m.landed) continue;
                        m.t += dt;
                        if (m.t < m.dur) continue;
                        m.landed = true; Play("point"); AddRing(m.to, 1);
                        bool deaf = vmode == VMode::Grace || vmode == VMode::GoToHide || (vmode == VMode::Chase && sees) || visitorBlind;
                        if (!deaf && std::hypot(visitor.pos.x - m.to.x, visitor.pos.y - m.to.y) < TS * 12) {
                            if (story.OnDecoy()) hear(m.to, 2.4f);
                            StartNextBeat();
                        }
                    }
                    // ---- you only see him when you could. Otherwise you hear where he walks
                    {
                        Vector2 eye = hidden ? hidePos : player.pos;
                        float de = std::hypot(visitor.pos.x - eye.x, visitor.pos.y - eye.y);
                        if (de < TS * 2.2f || LineOfSight(eye, visitor.pos)) vShowT = 0.3f; else vShowT -= dt;
                        static Vector2 lastStep = visitor.pos;
                        vStepAcc += std::min(12.0f, std::hypot(visitor.pos.x - lastStep.x, visitor.pos.y - lastStep.y));
                        lastStep = visitor.pos;
                        if (vStepAcc > 30) { vStepAcc = 0; if (vShowT <= 0 && de < TS * 11) AddRing(visitor.pos, 0); }
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
                    case VMode::Investigate:
                        // v0.12: he goes to the sound, listens, then turns that room inside out
                        vNoiseT += dt;
                        if (vPause > 0) {
                            vPause -= dt;
                            if (std::fmod(vPause, 0.4f) < dt) visitor.dir = (visitor.dir + 1 + GetRandomValue(0, 2)) % 4;
                            if (vPause <= 0) {
                                Room r = story.world.RoomAt(TX(vNoise.x), TX(vNoise.y));
                                int ri = -1;
                                for (int k = 0; k < kRoomCount; ++k) if (kSearchRooms[k] == r) ri = k;
                                vmode = VMode::Search;
                                if (ri >= 0) { vSure = true; PlanRoom(ri); } else PickNextRoom();
                            }
                        } else if (goTo(vNoise, searchSp * 1.7f, 4) || vNoiseT > 8) vPause = vNoiseWait;
                        break;
                    case VMode::Search:
                        // v0.12: indoors he reads the wet prints you bring in from the snow
                        if (vPrintCD <= 0.05f) {
                            Vector2 newest{}; bool any = false, near = false;
                            for (auto it = prints.rbegin(); it != prints.rend(); ++it) {
                                if (it->who != 3 || it->t > 40) continue;
                                if (!any) { newest = it->p; any = true; }
                                if (std::hypot(it->p.x - visitor.pos.x, it->p.y - visitor.pos.y) < 52 && LineOfSight(visitor.pos, it->p)) { near = true; break; }
                            }
                            if (near && std::hypot(newest.x - visitor.pos.x, newest.y - visitor.pos.y) > 14) {
                                story.OnTracked(); StartNextBeat();
                                vPrintCD = 3;
                                if (hidden && std::hypot(newest.x - hidePos.x, newest.y - hidePos.y) < TS * 2.6f) { vmode = VMode::GoToHide; break; }
                                vmode = VMode::Investigate; vNoise = newest; vNoiseWait = 0.8f; vNoiseT = 0; vPause = 0;
                                break;
                            }
                        }
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
        // v0.12: snow on your boots. It comes off on the floor for a while
        if (IsOutsideT(TileAt(TX(player.pos.x), TX(player.pos.y + 6)))) wetT = 14;
        else if (wetT > 0 && player.anim > 0 && !hidden && player.pos.y < 34 * TS) {
            wetT -= dt; wetCD -= dt;
            if (wetCD <= 0) { wetCD = 0.17f; printSide = 1 - printSide; prints.push_back({ { player.pos.x, player.pos.y + 6 }, 0, 3, printSide }); if (prints.size() > 500) prints.erase(prints.begin()); }
        }
        if (visitor.visible) AddPrint(visitor, 1, dt, 0.28f);
        if (doggo.visible && (story.DoggoExists() || (story.doggoOut && !doggoRanOff))) AddPrint(doggo, 2, dt, 0.12f);

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
        } else if (story.doggoOut && story.shift != Shift::NoDoggo && !story.doggoGone) {
            // v0.10: you let him out. Flat out for the gate, and gone
            story.doggoWithKid = false;
            if (!doggoRanOff) {
                doggo.visible = true;
                gMover = 2; MoveActor(doggo, 20, 33, 170, dt, false, 0); gMover = 0;
                if (doggo.pos.y > 32 * TS || story.collection) doggoRanOff = true;
            }
        } else story.doggoWithKid = false;
        if (!story.doggoOut) doggoRanOff = false;

        // v0.10: she hums while she thinks nobody is listening (for players without sound, too)
        static float momHumT = 5;
        if (story.MomHums() && mom.visible && !beatActive && std::hypot(mom.pos.x - player.pos.x, mom.pos.y - player.pos.y) < TS * 7) {
            momHumT -= dt;
            if (momHumT <= 0) { AddBubble("Mom", "hm hm hmm, hm hmm...", 2.4f); momHumT = (float)GetRandomValue(10, 16); }
        }
    }

    // v0.10: footsteps by surface. His carry through walls, duller and quieter, so you can hear where he is
    {
        static Vector2 lastP = player.pos, lastV = visitor.pos;
        static float accP = 0, accV = 0;
        auto surface = [&](Vector2 p) {
            int x = TX(p.x), y = TX(p.y + 5);
            Tile t = TileAt(x, y);
            Room r = story.world.RoomAt(x, y);
            if (t == Tile::Gravel || t == Tile::ShedFloor) return 3;
            if (IsOutsideT(t)) return 2;
            return (r == Room::Kitchen || r == Room::Bathroom) ? 1 : 0;
        };
        float dp = std::hypot(player.pos.x - lastP.x, player.pos.y - lastP.y), dv = std::hypot(visitor.pos.x - lastV.x, visitor.pos.y - lastV.y);
        lastP = player.pos; lastV = visitor.pos;
        bool quietBeat = beatActive && cur.type != BeatType::Say && cur.type != BeatType::Choice;
        bool live = audioOk && !journalOpen && !pauseOpen && !recapOpen && !miniOpen && !quietBeat && !hushMusic;
        if (live && dp > 0.01f && dp < 12 && !hidden) {
            accP += dp;
            if (accP > 26) {
                accP = 0;
                Sound& sn = sStep[0][surface(player.pos)];
                SetSoundVolume(sn, gSfxVol * (kidRunning ? 0.34f : 0.2f));
                SetSoundPitch(sn, 0.92f + GetRandomValue(0, 16) / 100.0f);
                PlaySound(sn);
            }
        }
        if (live && visitor.visible && dv > 0.01f && dv < 12) {
            accV += dv;
            if (accV > 30) {
                accV = 0;
                float d = std::hypot(visitor.pos.x - player.pos.x, visitor.pos.y - player.pos.y);
                float nearK = std::max(0.0f, 1 - d / (TS * 15.0f));
                if (nearK > 0) {
                    bool los = LineOfSight(visitor.pos, player.pos);
                    Sound& sn = sStep[1][surface(visitor.pos)];
                    SetSoundVolume(sn, gSfxVol * nearK * nearK * (los ? 0.6f : 0.36f));
                    SetSoundPitch(sn, (los ? 0.74f : 0.6f) + GetRandomValue(0, 6) / 100.0f);
                    SetSoundPan(sn, 0.5f - 0.35f * std::max(-1.0f, std::min(1.0f, (visitor.pos.x - player.pos.x) / 160.0f)));
                    PlaySound(sn);
                }
            }
        }
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
    LoadArt();
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
    UnloadArt();
    CloseWindow();
#endif
    return 0;
}
