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
    case TRACK_CHASE: return "16:50  -  The Collection";
    default: return "Silence";
    }
}

const char* TrackInfo(int t) {
    switch (t) {
    case TRACK_CUTE: return "96 bpm. Square lead in C major pentatonic, triangle bass C-Am-F-G, sine sparkles.";
    case TRACK_EERIE: return "66 bpm. Music box in A minor over a detuned tritone pad. Every 4th bar loses notes.";
    case TRACK_DARK: return "48 bpm. Dm-Bb-Gm-A sine chords, low D drone, a far bell every 16 s. Heavy echo.";
    case TRACK_CHASE: return "Heartbeat (52 Hz thumps), low growl, thin high whine.";
    default: return "Nothing. Wind, maybe.";
    }
}

float TrackEcho(int t) { return t >= TRACK_DARK ? 0.45f : t == TRACK_EERIE ? 0.35f : 0.2f; }

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

float TrackSample(int track, double t) {
    switch (track) {
    case 1: return TrackCute(t);
    case 2: return TrackEerie(t);
    case 3: return TrackDark(t);
    case 4: return TrackChase(t);
    default: return 0;
    }
}


} // namespace sixhundred
