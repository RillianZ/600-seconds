// 600 Seconds - Soundtrack. Written from scratch as code.
// Notes are MIDI numbers (60 = middle C). Mtof turns them into Hz.
#include "Soundtrack.h"
#include <algorithm>
#include <cmath>

namespace sixhundred {

static const double PI = 3.14159265358979323846;

const char* TrackName(int t) {
    switch (t) {
    case TRACK_CUTE: return "Act 1  -  Saturday";
    case TRACK_EERIE: return "Act 2  -  Wrong Notes";
    case TRACK_DARK: return "Act 3  -  Nothing Left";
    case TRACK_CHASE: return "16:50  -  The Collection (searching)";
    case TRACK_SPOTTED: return "16:50  -  He Sees You";
    case TRACK_TITLE: return "Title  -  Snow on the Vidda";
    case TRACK_BASEMENT: return "Basement  -  Blue Light";
    case TRACK_RECAP: return "Recap  -  The Log";
    case TRACK_ENDING: return "Ending  -  Far Away";
    default: return "Silence";
    }
}

const char* TrackInfo(int t) {
    switch (t) {
    case TRACK_CUTE: return "96 bpm. Square lead in C major pentatonic, triangle bass C-Am-F-G, sine sparkles.";
    case TRACK_EERIE: return "66 bpm. Music box in A minor over a detuned tritone pad. Every 4th bar loses notes.";
    case TRACK_DARK: return "48 bpm. Dm-Bb-Gm-A sine chords, low D drone, a far bell every 16 s. Heavy echo.";
    case TRACK_CHASE: return "Heartbeat (52 Hz thumps), low growl, thin high whine.";
    case TRACK_SPOTTED: return "Same heart, twice as fast. Low D pulse in 16ths, two whines beating against each other.";
    case TRACK_TITLE: return "60 bpm. Mom's song at half speed on a music box. C-Am-F-G sine pad, soft triangle bass.";
    case TRACK_BASEMENT: return "E-F cluster that breathes, console pings, her first four notes a quarter tone flat.";
    case TRACK_RECAP: return "80 bpm. Plucked arpeggio Am7-Fmaj7-Cmaj7-Em7 under the memory chimes.";
    case TRACK_ENDING: return "54 bpm. Mom's song moved to minor, sine piano over Am-F-Dm-G, a far bell.";
    default: return "Nothing. Wind, maybe.";
    }
}

float TrackEcho(int t) {
    switch (t) {
    case TRACK_DARK: case TRACK_CHASE: case TRACK_BASEMENT: return 0.45f;
    case TRACK_EERIE: case TRACK_TITLE: return 0.35f;
    case TRACK_ENDING: return 0.4f;
    case TRACK_RECAP: return 0.25f;
    default: return 0.2f;   // cute, spotted, silence
    }
}

// v0.20 sound: names for music files that replace a code track (assets/music/<name>.ogg)
const char* TrackFile(int t) {
    switch (t) {
    case TRACK_CUTE: return "act1";
    case TRACK_EERIE: return "act2";
    case TRACK_DARK: return "act3";
    case TRACK_CHASE: return "hunt_searching";
    case TRACK_SPOTTED: return "hunt_spotted";
    case TRACK_TITLE: return "title";
    case TRACK_BASEMENT: return "basement";
    case TRACK_RECAP: return "recap";
    case TRACK_ENDING: return "ending";
    default: return "";
    }
}

static float Mtof(float m) { return 440.0f * std::pow(2.0f, (m - 69.0f) / 12.0f); }
static float Wrap1(double x) { return (float)(x - std::floor(x)); }
// Band-limited square (polyBLEP). The naive version aliases into a harsh crackle.
static const float SR = 44100.0f;
static float Blep(float t, float dt) {
    if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}
static float OscSq(double ph, float duty, float freq) {
    float p = Wrap1(ph), dt = std::min(0.5f, freq / SR);
    float v = p < duty ? 1.0f : -1.0f;
    v += Blep(p, dt);
    v -= Blep(Wrap1(p + 1.0f - duty), dt);
    return v;
}
static float OscTri(double ph) { return 4.0f * std::fabs(Wrap1(ph) - 0.5f) - 1.0f; }
static float OscSin(double ph) { return std::sin(2.0 * PI * Wrap1(ph)); }

// Note envelope: 4 ms attack, exponential decay, and an 8 ms release right
// before the next note on the same voice. Hard starts and cuts were the clicks.
static float Env(double age, double left, float decay) {
    float a = (float)std::min(1.0, age / 0.004);
    float r = (float)std::min(1.0, std::max(0.0, left) / 0.008);
    return a * r * std::exp(-(float)age * decay);
}
// Finds the note that is currently ringing on a step pattern, how long ago it
// started and how long until the next note takes over.
struct Voice { int note = 0; double age = 0, left = 1e9; };
template <typename F>
static Voice FindVoice(F noteAt, double t, double step, int reach = 8) {
    int st = (int)(t / step);
    Voice v;
    for (int k = 0; k <= reach; ++k) {
        int s = st - k; if (s < 0) break;
        int n = noteAt(s);
        if (n) { v.note = n; v.age = t - s * step; break; }
    }
    for (int k = 1; k <= reach; ++k) {
        int s = st + k;
        if (noteAt(s)) { v.left = s * step - t; break; }
    }
    return v;
}

static float TrackCute(double t) {
    // 96 bpm, C major pentatonic lead, bouncy bass. A sunny Saturday.
    const double step = 60.0 / 96.0 / 2.0;
    int st = (int)(t / step); double ts = t - st * step;
    static const int mel[32] = { 72,0,76,79, 0,76,74,0, 72,0,69,72, 74,0,0,0,
                                 76,0,79,81, 0,79,76,0, 74,0,72,74, 72,0,0,0 };
    float v = 0;
    Voice lead = FindVoice([](int s) { return mel[s % 32]; }, t, step);
    if (lead.note) {
        float f = Mtof((float)lead.note);
        v += 0.07f * OscSq(f * lead.age, 0.25f, f) * Env(lead.age, lead.left, 7.0f);
    }
    static const int roots[4] = { 48, 45, 41, 43 };
    auto bassAt = [](int s) {
        int root = roots[(s / 8) % 4], bs = s % 8;
        if (bs == 0 || bs == 4) return root;
        if (bs == 3 || bs == 6) return root + 7;
        return 0;
    };
    Voice bass = FindVoice(bassAt, t, step);
    if (bass.note) v += 0.11f * OscTri(Mtof((float)bass.note) * bass.age) * Env(bass.age, bass.left, 4.0f);
    // soft sparkle arpeggio
    int root = roots[(st / 8) % 4];
    static const int arp[4] = { 0, 4, 7, 12 };
    v += 0.025f * OscSin(Mtof((float)(root + 24 + arp[st % 4])) * ts) * Env(ts, step - ts, 9.0f);
    return v;
}

static float TrackEerie(double t) {
    // 66 bpm. A music box that's slightly wrong, over a tritone pad.
    const double step = 60.0 / 66.0 / 2.0;
    float v = 0;
    v += 0.045f * (OscTri(Mtof(45.0f) * t) + OscTri(Mtof(45.07f) * t)) * (0.6f + 0.4f * (float)std::sin(t * 0.3));
    v += 0.025f * OscTri(Mtof(51.0f) * t) * (0.5f + 0.5f * (float)std::sin(t * 0.17));
    static const int box[32] = { 81,0,0,76, 0,72,0,0, 77,0,0,0, 76,0,74,0,
                                 81,0,0,76, 0,72,0,0, 70,0,0,0, 69,0,0,0 };
    // every fourth bar a note goes missing. reality forgetting its lines
    auto noteAt = [](int s) { int n = box[s % 32]; return (n && !((s / 32) % 4 == 3 && s % 5 == 0)) ? n : 0; };
    Voice mb = FindVoice(noteAt, t, step);
    if (mb.note) {
        float f = Mtof((float)mb.note);
        v += 0.06f * (OscSin(f * mb.age) + 0.35f * OscSin(2.0 * f * mb.age)) * Env(mb.age, mb.left, 2.6f);
    }
    return v;
}

static float TrackDark(double t) {
    // 48 bpm, Dm Bb Gm A. Slow, low, nothing to hope for.
    static const int chords[4][3] = { { 50, 53, 57 }, { 46, 50, 53 }, { 43, 46, 50 }, { 45, 49, 52 } };
    const double len = 8.0;
    int c = (int)(t / len) % 4;
    double local = std::fmod(t, len);
    float env = (float)std::min(1.0, local / 3.0) * (float)std::min(1.0, (len - local) / 1.5);
    float v = 0;
    for (int k = 0; k < 3; ++k) {
        float f = Mtof((float)chords[c][k]);
        v += 0.03f * (OscSin(f * t) + 0.5f * OscSin(f * 1.004 * t) + 0.2f * OscTri(f * 0.5 * t)) * env;
    }
    v += 0.07f * OscSin(Mtof(26.0f) * t) * (0.7f + 0.3f * (float)std::sin(t * 0.5));
    double tb = std::fmod(t, 16.0);
    float bell = Mtof(86.0f);
    v += 0.035f * (OscSin(bell * tb) + 0.5f * OscSin(bell * 2.76 * tb)) * Env(tb, 16.0 - tb, 1.1f);
    return v;
}

static float TrackChase(double t) {
    // Heartbeat and a low growl. The last 600 seconds.
    double hb = std::fmod(t, 0.62);
    float v = 0;
    auto thump = [](double x) { return x > 0 && x < 0.15 ? (float)(std::sin(2.0 * PI * 52.0 * x) * std::exp(-x * 28.0)) : 0.0f; };
    v += 0.4f * (thump(hb) + 0.7f * thump(hb - 0.17));
    v += 0.05f * OscTri(Mtof(29.0f) * t) * (0.6f + 0.4f * (float)std::sin(t * 1.3));
    v += 0.012f * OscSin(Mtof(82.0f + 0.4f * (float)std::sin(t * 0.7)) * t);
    return v;
}

// v0.10: Mom's song. The Act 1 lead, an octave down, slow, hummed. One phrase, then a breath.
// It was in the music the whole time. On the tape it is the only thing left.
float HumSample(double t) {
    static const int mel[32] = { 72,0,76,79, 0,76,74,0, 72,0,69,72, 74,0,0,0,
                                 76,0,79,81, 0,79,76,0, 74,0,72,74, 72,0,0,0 };
    const double step = 60.0 / 96.0 / 2.0 / 0.62;
    Voice v = FindVoice([](int s) { return s % 48 < 32 ? mel[s % 48] : 0; }, t, step);
    if (!v.note) return 0;
    float f = Mtof((float)v.note - 12.0f);
    double ph = f * v.age + 0.12 * std::sin(2.0 * PI * 5.1 * t);   // a little vibrato
    float a = (float)std::min(1.0, v.age / 0.07) * (float)std::min(1.0, std::max(0.0, v.left) / 0.06) * std::exp(-(float)v.age * 0.55f);
    return a * (OscSin(ph) + 0.16f * OscSin(2.0 * ph) + 0.05f * OscSin(3.0 * ph));
}

// ============================================================ v0.20 sound: new tracks
// A soft sine pad on a chord that changes every `len` seconds, faded in and out so the changes don't click
static float Pad(const int (*chords)[4], int nChords, double t, double len, float amp) {
    int c = (int)(t / len) % nChords;
    double local = std::fmod(t, len);
    float env = (float)std::min(1.0, local / 1.2) * (float)std::min(1.0, (len - local) / 0.6);
    float v = 0;
    for (int k = 0; k < 4; ++k) {
        int n = chords[c][k]; if (!n) continue;
        float f = Mtof((float)n);
        v += (OscSin(f * t) + 0.4f * OscSin(f * 1.003 * t)) * env;
    }
    return v * amp;
}

// The Act 1 lead, the tune Mom hums. Shared by the title, the basement and the ending
static const int kMomMel[32] = { 72,0,76,79, 0,76,74,0, 72,0,69,72, 74,0,0,0,
                                 76,0,79,81, 0,79,76,0, 74,0,72,74, 72,0,0,0 };
// One step down the C major pentatonic: C->A, D->C, E->D, G->E, A->G. Same shape, minor
static int PentaDown(int n) {
    static const int pcs[5] = { 0, 2, 4, 7, 9 };
    int oct = n / 12, pc = n % 12, i = 0;
    for (int k = 0; k < 5; ++k) if (pcs[k] == pc) i = k;
    if (i == 0) return (oct - 1) * 12 + pcs[4];
    return oct * 12 + pcs[i - 1];
}

static float TrackSpotted(double t) {
    // He sees you. The same heart, twice as fast (every other beat lands with the searching track, so the crossfade holds),
    // a low pulse that won't let go, and two thin whines rubbing against each other
    auto thump = [](double x) { return x > 0 && x < 0.15 ? (float)(std::sin(2.0 * PI * 52.0 * x) * std::exp(-x * 28.0)) : 0.0f; };
    double hb = std::fmod(t, 0.31);
    float v = 0.36f * (thump(hb) + 0.55f * thump(hb - 0.11));
    const double step = 0.155;
    static const int pat[16] = { 38,0,38,38, 39,0,38,0, 38,0,38,38, 41,0,39,0 };
    Voice p = FindVoice([](int s) { return pat[s % 16]; }, t, step);
    if (p.note) {
        float f = Mtof((float)p.note);
        v += (0.05f * OscTri(f * p.age) + 0.018f * OscSq(f * p.age, 0.3f, f)) * Env(p.age, p.left, 8.0f);
    }
    v += 0.045f * OscTri(Mtof(29.0f) * t) * (0.6f + 0.4f * (float)std::sin(t * 2.6));
    v += 0.009f * (OscSin(Mtof(82.0f) * t) + OscSin(Mtof(82.35f) * t)) * (0.7f + 0.3f * (float)std::sin(t * 0.9));
    return v;
}

static float TrackTitle(double t) {
    // 60 bpm. Mom's song at half speed, on a music box in a cold room. Nobody is singing it yet
    static const int chords[4][4] = { { 48, 52, 55, 59 }, { 45, 48, 52, 55 }, { 41, 45, 48, 52 }, { 43, 47, 50, 55 } };
    const double step = 0.5;
    float v = Pad(chords, 4, t, 8 * step, 0.014f);
    // every other time round, a few notes are left out. It's hard to remember the whole thing
    auto noteAt = [](int s) { int n = kMomMel[s % 32]; return (n && (s / 32) % 2 == 1 && (s % 32 == 3 || s % 32 == 19 || s % 32 == 27)) ? 0 : n; };
    Voice mb = FindVoice(noteAt, t, step);
    if (mb.note) {
        float f = Mtof((float)mb.note);
        v += 0.065f * (OscSin(f * mb.age) + 0.3f * OscSin(2.0 * f * mb.age) + 0.08f * OscSin(5.04 * f * mb.age)) * Env(mb.age, mb.left, 2.2f);
    }
    static const int roots[4] = { 36, 33, 29, 31 };
    double bl = std::fmod(t, 8 * step);
    int r = roots[(int)(t / (8 * step)) % 4];
    v += 0.075f * OscTri(Mtof((float)r) * bl) * Env(bl, 8 * step - bl, 0.9f);
    return v;
}

static float TrackBasement(double t) {
    // Under the house. A cold cluster that breathes, a console that pings, and sometimes the start of her song, a quarter tone flat
    float v = 0;
    static const float cl[3] = { 40.0f, 41.0f, 47.0f };
    for (int k = 0; k < 3; ++k) {
        float f = Mtof(cl[k]);
        float br = 0.5f + 0.5f * (float)std::sin(t * (0.19 + 0.07 * k) + k * 2.1);
        v += 0.03f * (OscSin(f * t) + 0.3f * OscTri(f * 0.5 * t)) * br;
    }
    v += 0.045f * OscSin(Mtof(28.0f) * t) * (0.5f + 0.5f * (float)std::sin(t * 0.13));
    // the console: two pings, a long wait
    const double step = 0.75;
    static const int ping[16] = { 88,0,0,0, 0,0,83,0, 0,0,0,0, 88,0,0,0 };
    Voice p = FindVoice([](int s) { return ping[s % 16]; }, t, step);
    if (p.note) v += 0.02f * OscSin(Mtof((float)p.note) * p.age) * Env(p.age, p.left, 7.0f);
    // every 24 seconds, her first four notes, slow and flat. Then it gives up
    double ph = std::fmod(t, 24.0) - 12.0;
    if (ph > 0 && ph < 6.0) {
        static const int four[4] = { 72, 76, 79, 76 };
        int i = (int)(ph / 1.2);
        if (i < 4) {
            double age = ph - i * 1.2;
            float f = Mtof(four[i] - 0.5f);
            v += 0.035f * (OscSin(f * age) + 0.3f * OscSin(2.0 * f * age)) * Env(age, 1.2 - age, 1.6f);
        }
    }
    return v;
}

static float TrackRecap(double t) {
    // 80 bpm. The log being written. A plucked arpeggio that sits under the memory chimes (E, B, E)
    static const int chords[4][4] = { { 45, 48, 52, 55 }, { 41, 45, 48, 52 }, { 48, 52, 55, 59 }, { 40, 43, 47, 50 } };
    const double step = 60.0 / 80.0 / 2.0;
    float v = Pad(chords, 4, t, 8 * step, 0.011f);
    static const int up[8] = { 0, 1, 2, 3, 4, 3, 2, 1 };
    auto noteAt = [](int s) {
        const int* c = chords[(s / 8) % 4];
        int i = up[s % 8];
        return i < 4 ? c[i] + 24 : c[0] + 36;
    };
    Voice a = FindVoice(noteAt, t, step);
    if (a.note) {
        float f = Mtof((float)a.note);
        v += 0.055f * (OscSin(f * a.age) + 0.35f * OscSin(2.0 * f * a.age)) * Env(a.age, a.left, 4.5f);
    }
    double bl = std::fmod(t, 8 * step);
    v += 0.07f * OscTri(Mtof((float)(chords[(int)(t / (8 * step)) % 4][0] - 12)) * bl) * Env(bl, 8 * step - bl, 1.2f);
    return v;
}

static float TrackEnding(double t) {
    // 54 bpm. Mom's song, moved one step down the scale so it's in minor, played slowly on a sine piano. A far bell.
    // Calm. Sad. Not hopeless like Act 3
    static const int chords[4][4] = { { 45, 48, 52, 57 }, { 41, 45, 48, 53 }, { 38, 41, 45, 50 }, { 43, 47, 50, 55 } };
    const double step = 60.0 / 54.0 / 2.0;
    float v = Pad(chords, 4, t, 8 * step, 0.01f);
    auto noteAt = [](int s) { int n = kMomMel[s % 32]; return n ? PentaDown(n) : 0; };
    Voice m = FindVoice(noteAt, t, step);
    if (m.note) {
        float f = Mtof((float)m.note);
        float bright = std::exp(-(float)m.age * 3.0f);
        v += 0.055f * (OscSin(f * m.age) + 0.45f * bright * OscSin(2.0 * f * m.age) + 0.12f * OscSin(3.0 * f * m.age)) * Env(m.age, m.left, 1.1f);
    }
    double bl = std::fmod(t, 8 * step);
    v += 0.055f * OscTri(Mtof((float)(chords[(int)(t / (8 * step)) % 4][0] - 12)) * bl) * Env(bl, 8 * step - bl, 0.7f);
    double tb = std::fmod(t + 5.0, 16.0);
    float bell = Mtof(81.0f);
    v += 0.02f * (OscSin(bell * tb) + 0.5f * OscSin(bell * 2.76 * tb)) * Env(tb, 16.0 - tb, 1.0f);
    return v;
}

// v0.20 sound (Noah, T-148): Dad's hum. His own tune, low and slow, in G. A working man: one phrase, an answer,
// then a long gap while he does whatever he's doing. Notes slide up into place the way people hum.
float DadHumSample(double t) {
    struct N { int note; int len; };   // len in steps, note 0 = breath
    static const N tune[] = {
        { 50, 2 }, { 55, 2 }, { 55, 1 }, { 57, 1 }, { 59, 2 }, { 57, 1 }, { 55, 1 }, { 52, 2 }, { 50, 4 }, { 0, 3 },
        { 50, 1 }, { 52, 1 }, { 55, 2 }, { 57, 2 }, { 59, 1 }, { 57, 1 }, { 55, 5 }, { 0, 16 },
    };
    const int nTune = (int)(sizeof(tune) / sizeof(tune[0]));
    const double step = 0.45;
    static int total = 0;
    if (!total) for (int k = 0; k < nTune; ++k) total += tune[k].len;
    double cyc = total * step;
    double lt = std::fmod(t, cyc);
    int i = 0; double start = 0;
    while (i < nTune && start + tune[i].len * step <= lt) { start += tune[i].len * step; ++i; }
    if (i >= nTune || !tune[i].note) return 0;
    double age = lt - start, dur = tune[i].len * step, left = dur - age;
    bool tied = i + 1 < nTune && tune[i + 1].note;   // a breath next: let it go properly
    float a = (float)std::min(1.0, age / 0.09) * (float)std::min(1.0, std::max(0.0, left) / (tied ? 0.05 : 0.18));
    a *= 0.75f + 0.25f * std::exp(-(float)age * 1.5f);
    // a scoop up into the note, and a slow wobble
    float bend = -0.6f * (float)std::exp(-age * 18.0);
    float f = Mtof((float)tune[i].note + bend);
    double ph = f * age + 0.08 * std::sin(2.0 * PI * 4.2 * t);
    // hummed with the mouth shut: strong low harmonics, nothing on top
    return a * (OscSin(ph) + 0.42f * OscSin(2.0 * ph) + 0.22f * OscSin(3.0 * ph) + 0.06f * OscSin(4.0 * ph));
}

float TrackSample(int track, double t) {
    switch (track) {
    case 1: return TrackCute(t);
    case 2: return TrackEerie(t);
    case 3: return TrackDark(t);
    case 4: return TrackChase(t);
    case TRACK_SPOTTED: return TrackSpotted(t);
    case TRACK_TITLE: return TrackTitle(t);
    case TRACK_BASEMENT: return TrackBasement(t);
    case TRACK_RECAP: return TrackRecap(t);
    case TRACK_ENDING: return TrackEnding(t);
    default: return 0;
    }
}


} // namespace sixhundred
