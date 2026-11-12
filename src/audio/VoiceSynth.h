// 600 Seconds - src/audio/VoiceSynth.h (v0.20.1 sound-look)
// Speech blips and thunder, made in code. Plain C++, no raylib, so test/audio_check.cpp can render and measure them.
// main.cpp turns the buffers into sounds (src/audio/Voices.inc).
//   Blips: one short sung syllable per blip. A soft voice-like tone (harmonics shaped by two vowel formants), not a square wave.
//          Each speaker has a pitch, a vowel colour, a length and a little breath. Four syllables each, so a line doesn't drone.
//          Every syllable comes in three versions: clear, a little dull (through an open doorway), dull (through a wall or a door).
//   Thunder: a low roll of brown noise with a few rumbles in it. Close ones start with a crack. Far ones swell up slowly.
//          Each comes clear (outdoors) and muffled (from inside the house).
#pragma once
#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>

namespace voicesynth {

static const int SR = 44100;
static const float kPi = 3.14159265f;

// speakers that get blips
enum Speaker { SP_KID, SP_MOM, SP_DAD, SP_VISITOR, SP_RADIO, SP_GIRL, SP_COUNT };
static const int kVariants = 4;    // syllables per speaker
static const int kMuffles = 3;     // clear, a little dull, dull

struct VoiceParams {
    float f0;        // pitch (Hz)
    float glide;     // how much the pitch moves over the syllable (+ up, - down)
    float f1, f2;    // vowel formants (Hz)
    float dur;       // seconds
    float breath;    // noise in the voice
    float gap;       // seconds between blips while a line types out
    float vol;       // loudness at full sound volume (raylib volume, before the muffle gain)
};
static VoiceParams Params(int sp) {
    switch (sp) {
    case SP_KID:     return { 300, 0.06f, 520, 1900, 0.050f, 0.05f, 0.075f, 0.30f };   // high, quick, a little rising
    case SP_MOM:     return { 215, -0.04f, 660, 1250, 0.060f, 0.04f, 0.085f, 0.28f };  // warm, mid
    case SP_DAD:     return { 116, -0.05f, 480, 980, 0.072f, 0.08f, 0.100f, 0.28f };   // low, slower
    case SP_VISITOR: return { 88, -0.12f, 400, 820, 0.090f, 0.20f, 0.130f, 0.28f };    // lower still, slow, breathy, falling
    case SP_RADIO:   return { 140, 0.0f, 700, 1600, 0.060f, 0.0f, 0.090f, 0.22f };     // Finse on the radio: thin and flat
    default:         return { 335, 0.04f, 460, 2200, 0.046f, 0.16f, 0.080f, 0.22f };   // the girl's voice: high and airy
    }
}
static int SpeakerFor(const std::string& who) {
    if (who == "You") return SP_KID;
    if (who == "Mom" || who == "Handler two") return SP_MOM;
    if (who == "Dad") return SP_DAD;
    if (who == "The visitor") return SP_VISITOR;
    if (who == "Finse" || who == "Voice") return SP_RADIO;
    if (who == "A girl's voice") return SP_GIRL;
    return -1;   // the narrator, Black text, an empty speaker, Doggo (he gets a huff instead)
}

// a tiny deterministic noise
struct Rng { unsigned s; float Next() { s = s * 1664525u + 1013904223u; return ((s >> 9) & 0xffff) / 32768.0f - 1.0f; } };
// state-variable band-pass, for breath and the radio
struct Svf { float lo = 0, bp = 0; float Run(float x, float f, float q) { float k = 2 * std::sin(kPi * std::min(f, SR / 6.0f) / SR); lo += k * bp; float hi = x - lo - q * bp; bp += k * hi; return bp; } };
// two one-pole low-passes in a row
struct Lp2 { float a = 0, b = 0; float Run(float x, float fc) { float k = 1 - std::exp(-2 * kPi * fc / SR); a += (x - a) * k; b += (a - b) * k; return b; } };

// the same edges and soft clip MakeSound in main.cpp gives every effect: 3 ms in, 10 ms out, tanh instead of a hard clip
static void Finish(std::vector<float>& v) {
    int n = (int)v.size();
    for (int i = 0; i < n; ++i) {
        float fin = std::min(1.0f, i / (SR * 0.003f)), fout = std::min(1.0f, (n - 1 - i) / (SR * 0.010f));
        v[i] = std::tanh(v[i] * fin * fout * 1.2f) / std::tanh(1.2f);
    }
}

// muffle: 0 clear, 1 a little dull (about 1.6 kHz), 2 dull (about 450 Hz, like the hums through a wall)
static float MuffleCut(int muffle) { return muffle == 0 ? 0 : muffle == 1 ? 1600.0f : 450.0f; }

static std::vector<float> RenderBlip(int sp, int variant, int muffle) {
    VoiceParams p = Params(sp);
    // four syllables: the pitch steps around a little, the vowel changes
    static const float pitchK[kVariants] = { 1.0f, 1.12f, 0.93f, 1.06f };
    static const float f1K[kVariants] = { 1.0f, 0.82f, 1.12f, 0.9f }, f2K[kVariants] = { 1.0f, 1.18f, 0.86f, 1.08f };
    float f0 = p.f0 * pitchK[variant], f1 = p.f1 * f1K[variant], f2 = p.f2 * f2K[variant];
    float dur = p.dur * (variant == 2 ? 1.15f : variant == 1 ? 0.9f : 1.0f);
    int n = (int)(SR * (dur + 0.03f));   // a short tail for the low-pass to ring out
    std::vector<float> v(n, 0.0f);
    Rng rng{ 977u + 131u * sp + 17u * variant };
    Svf br; Lp2 mu; Lp2 rLo; float rHp = 0;
    float ph = 0;
    float cut = MuffleCut(muffle);
    for (int i = 0; i < n; ++i) {
        float t = i / (float)SR, u = std::min(1.0f, t / dur);
        float f = f0 * (1 + p.glide * u);
        ph += 2 * kPi * f / SR;
        if (ph > 2 * kPi * 64) ph -= 2 * kPi * 64;
        // harmonics up to 4 kHz, each weighted by how close it sits to the two formants
        float s = 0;
        for (int k = 1; k * f < 4000 && k <= 24; ++k) {
            float hf = k * f;
            float r1 = 1 / (1 + ((hf - f1) / 160) * ((hf - f1) / 160)), r2 = 1 / (1 + ((hf - f2) / 260) * ((hf - f2) / 260));
            s += std::sin(ph * k) * (0.35f / k + r1 + 0.55f * r2) / std::sqrt((float)k);
        }
        s *= 0.16f;
        if (p.breath > 0) s += br.Run(rng.Next(), f2, 0.7f) * p.breath * 1.4f;
        // the envelope: a soft start, a rounded end. No hard gate, so it never clicks or buzzes
        float env = t < dur ? std::min(1.0f, t / 0.008f) * std::pow(1 - u, 1.4f) * (0.7f + 0.3f * std::sin(kPi * std::min(1.0f, u * 1.6f))) : 0;
        float x = s * env;
        if (sp == SP_RADIO) {
            // a radio: no lows, no highs, a little crackle
            x += rng.Next() * 0.025f * (t < dur ? 1 : 0);
            rHp += (x - rHp) * (1 - std::exp(-2 * kPi * 350.0f / SR));
            x = rLo.Run(x - rHp, 2800);
            x *= 1.6f;
        }
        if (cut > 0) x = mu.Run(x, cut);
        v[i] = x;
    }
    Finish(v);
    return v;
}

// Doggo doesn't talk. Once per line, a tiny huff through his nose
static std::vector<float> RenderHuff(int muffle) {
    int n = (int)(SR * 0.14f);
    std::vector<float> v(n);
    Rng rng{ 4242u };
    Svf bp; Lp2 mu;
    float cut = MuffleCut(muffle);
    for (int i = 0; i < n; ++i) {
        float t = i / (float)SR;
        float env = std::min(1.0f, t / 0.012f) * std::exp(-t * 26);
        float x = bp.Run(rng.Next(), 900 - 1400 * t, 1.1f) * env * 0.5f;
        if (cut > 0) x = mu.Run(x, cut);
        v[i] = x;
    }
    Finish(v);
    return v;
}

// thunder. dist: 0 close (a crack, then the roll), 1 middle, 2 far (no crack, a slow swell, lower). muffled: from inside the house
static float ThunderLength(int dist) { return dist == 0 ? 4.2f : dist == 1 ? 4.8f : 5.6f; }
static std::vector<float> RenderThunder(int dist, bool muffled) {
    float len = ThunderLength(dist);
    int n = (int)(SR * len);
    std::vector<float> v(n);
    Rng rng{ 9001u + 77u * dist };
    // the rumbles inside the roll: when they hit, how hard, how fast they die
    struct Boom { float at, amp, decay; };
    std::vector<Boom> booms;
    Rng br{ 31337u + 7u * dist };
    float at = dist == 0 ? 0.0f : dist == 1 ? 0.12f : 0.35f;
    for (int k = 0; k < 7; ++k) {
        float a = (k == 0 ? 1.0f : 0.45f + 0.35f * std::fabs(br.Next())) * (1 - k * 0.08f);
        booms.push_back({ at, a, 1.6f + 1.2f * std::fabs(br.Next()) });
        at += 0.25f + 0.45f * std::fabs(br.Next()) + dist * 0.12f;
        if (at > len - 1.2f) break;
    }
    float brown = 0;
    Lp2 roll, crackLp, mu; Svf mid;
    float rollCut = dist == 0 ? 260.0f : dist == 1 ? 190.0f : 130.0f;
    float attack = dist == 0 ? 0.03f : dist == 1 ? 0.15f : 0.45f;
    for (int i = 0; i < n; ++i) {
        float t = i / (float)SR;
        float w = rng.Next();
        brown = brown * 0.985f + w * 0.12f;
        float body = roll.Run(brown, rollCut) * 3.2f;
        float env = 0;
        for (auto& b : booms) {
            float s = t - b.at;
            if (s > 0) env += b.amp * std::min(1.0f, s / attack) * std::exp(-s * b.decay);
        }
        // the roll fades out over the last second and a half
        env *= std::min(1.0f, (len - t) / 1.5f);
        float x = body * env;
        // a little grit in the middle of the sound so it reads on small speakers
        x += mid.Run(w, 520 - 200 * dist, 1.4f) * env * (0.10f - 0.03f * dist);
        if (dist == 0) {
            // the crack: a tearing burst of noise in the first quarter second, rough on purpose
            float gate = std::fmod(t * 37, 1.0f) < 0.55f ? 1.0f : 0.35f;
            x += crackLp.Run(w, 3200) * 1.3f * std::exp(-t * 14) * gate * std::min(1.0f, t / 0.004f);
        } else if (dist == 1) {
            x += crackLp.Run(w, 1400) * 0.45f * std::exp(-std::max(0.0f, t - 0.1f) * 8) * std::min(1.0f, t / 0.05f);
        }
        if (muffled) x = mu.Run(x, 170) * 1.5f;   // the house walls take the top off. What's left is the floor shaking
        v[i] = x;
    }
    // level: close loud, far quieter. Peak before the soft clip
    float peak = 0;
    for (float s : v) peak = std::max(peak, std::fabs(s));
    float want = (dist == 0 ? 0.85f : dist == 1 ? 0.7f : 0.5f) * (muffled ? 0.85f : 1.0f);
    if (peak > 0) for (float& s : v) s *= want / peak;
    Finish(v);
    return v;
}

// the thunder for one flash: its delay after the flash (0.5 to 2 s), and from that how far away it was
static float ThunderDelay(unsigned h) { return 0.5f + 1.5f * ((h % 1000) / 999.0f); }
static int ThunderDist(float delay) { return delay < 0.95f ? 0 : delay < 1.5f ? 1 : 2; }
static float ThunderVol(int dist) { return dist == 0 ? 0.62f : dist == 1 ? 0.5f : 0.4f; }   // raylib volume at full sound volume

// which characters of a line get blips: letters and digits, not inside (stage directions) or *actions*
static std::vector<bool> SpokenMask(const std::string& s) {
    std::vector<bool> m(s.size(), false);
    int paren = 0; bool star = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '(') { ++paren; continue; }
        if (c == ')') { paren = std::max(0, paren - 1); continue; }
        if (c == '*') { star = !star; continue; }
        m[i] = !paren && !star && (std::isalnum((unsigned char)c) || (unsigned char)c >= 0x80);
    }
    return m;
}
// a bubble or a line that is only humming (hm hm hmm, mm-hm) gets no blips: the hum itself is heard
static bool OnlyHumming(const std::string& s) {
    auto m = SpokenMask(s);
    bool any = false;
    for (size_t i = 0; i < s.size(); ++i) if (m[i]) { any = true; char c = (char)std::tolower((unsigned char)s[i]); if (c != 'h' && c != 'm') return false; }
    return any;
}
static int SpokenCount(const std::string& s) { auto m = SpokenMask(s); return (int)std::count(m.begin(), m.end(), true); }

}  // namespace voicesynth
