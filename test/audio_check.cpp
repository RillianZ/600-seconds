// 600 Seconds - test/audio_check.cpp (v0.20 sound)
// Nobody can listen in the build machine, so this renders every code track offline and checks the numbers:
// not silent, not clipping, no big jumps between samples (clicks), loudness close to the others.
//   g++ -std=c++17 -O2 -Isrc test/audio_check.cpp src/audio/Soundtrack.cpp -o /tmp/audio_check && /tmp/audio_check
// Optional: /tmp/audio_check wav   writes each track to /tmp/600s_<track>.wav (20 s) to listen to somewhere else.
#include "audio/Soundtrack.h"
#include "audio/VoiceSynth.h"   // v0.20.1 sound-look
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace sixhundred;

static void WriteWav(const std::string& path, const std::vector<float>& v, int sr) {
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return;
    int n = (int)v.size(), bytes = n * 2;
    auto u32 = [&](unsigned x) { std::fwrite(&x, 4, 1, f); };
    auto u16 = [&](unsigned short x) { std::fwrite(&x, 2, 1, f); };
    std::fwrite("RIFF", 1, 4, f); u32(36 + bytes); std::fwrite("WAVEfmt ", 1, 8, f);
    u32(16); u16(1); u16(1); u32(sr); u32(sr * 2); u16(2); u16(16);
    std::fwrite("data", 1, 4, f); u32(bytes);
    for (float s : v) { float c = std::tanh(s * 3.0f) * 0.9f; short q = (short)(c * 32000); std::fwrite(&q, 2, 1, f); }
    std::fclose(f);
}

struct Stats { double rms = 0, peak = 0, jump = 0, quietShare = 0; };
static Stats Measure(const std::vector<float>& v, int sr) {
    Stats s;
    double sum = 0;
    for (size_t i = 0; i < v.size(); ++i) {
        sum += v[i] * (double)v[i];
        s.peak = std::max(s.peak, (double)std::fabs(v[i]));
        if (i) s.jump = std::max(s.jump, (double)std::fabs(v[i] - v[i - 1]));
    }
    s.rms = std::sqrt(sum / v.size());
    int win = sr / 2, quiet = 0, wins = 0;
    for (size_t i = 0; i + win <= v.size(); i += win, ++wins) {
        double e = 0;
        for (int k = 0; k < win; ++k) e += v[i + k] * (double)v[i + k];
        if (std::sqrt(e / win) < 0.002) ++quiet;
    }
    s.quietShare = wins ? quiet / (double)wins : 0;
    return s;
}

int main(int argc, char** argv) {
    const int SR = 44100;
    bool wav = argc > 1 && std::strcmp(argv[1], "wav") == 0;
    int fails = 0;
    std::printf("%-42s %8s %8s %8s %8s\n", "track", "rms", "peak", "jump", "quiet");
    auto check = [&](const std::string& name, float (*f)(int, double), int arg, double seconds, double minRms, double maxQuiet, bool isTrack) {
        std::vector<float> v((size_t)(SR * seconds));
        for (size_t i = 0; i < v.size(); ++i) v[i] = f(arg, i / (double)SR);
        Stats s = Measure(v, SR);
        // tracks: absolute (the mixer adds them as they are). Hums: relative, the mixer scales them down to a few percent.
        // The jump limit is loose because the Act 1 square lead has steep edges on purpose; a real click is far bigger
        double peakMax = isTrack ? 0.95 : 1.5, jumpMax = isTrack ? 0.15 : 0.12 * s.peak;
        bool ok = s.rms >= minRms && s.peak < peakMax && s.jump < jumpMax && s.quietShare <= maxQuiet;
        if (isTrack && s.rms > 0.09) ok = false;   // much louder than the old tracks (0.035 to 0.06)
        std::printf("%-42s %8.4f %8.4f %8.4f %7.0f%% %s\n", name.c_str(), s.rms, s.peak, s.jump, s.quietShare * 100, ok ? "ok" : "FAIL");
        if (!ok) ++fails;
        if (wav) {
            std::vector<float> w(v.begin(), v.begin() + std::min(v.size(), (size_t)(SR * 20)));
            std::string stem = isTrack ? TrackFile(arg) : name;
            WriteWav("/tmp/600s_" + stem + ".wav", w, SR);
        }
    };
    for (int t = 1; t < TRACK_COUNT; ++t)
        check(TrackName(t), [](int k, double x) { return TrackSample(k, x); }, t, 60.0, 0.02, 0.05, true);
    // the hums are voices: they rest a lot (Dad works in between), so only check that they are there at all
    check("hum_mom", [](int, double x) { return HumSample(x); }, 0, 60.0, 0.05, 0.6, false);
    check("hum_dad", [](int, double x) { return DadHumSample(x); }, 0, 60.0, 0.05, 0.6, false);
    // crossfade searching -> spotted at the same clock: no click where they meet
    {
        std::vector<float> v((size_t)(SR * 6));
        for (size_t i = 0; i < v.size(); ++i) {
            double x = 30.0 + i / (double)SR;
            float g = (float)std::min(1.0, std::max(0.0, (i / (double)SR - 2.0) / 0.8));
            v[i] = TrackSample(TRACK_CHASE, x) * (1 - g) + TrackSample(TRACK_SPOTTED, x) * g;
        }
        Stats s = Measure(v, SR);
        bool ok = s.jump < 0.08 && s.peak < 0.95;
        std::printf("%-42s %8.4f %8.4f %8.4f %8s %s\n", "crossfade searching -> spotted", s.rms, s.peak, s.jump, "", ok ? "ok" : "FAIL");
        if (!ok) ++fails;
    }
    // v0.20.1 sound-look: speech blips and thunder (audio/VoiceSynth.h). What the game plays, after the raylib volume
    {
        namespace vsy = voicesynth;
        // the share of a sound's energy above about 1.5 kHz: muffled versions must have much less of it
        auto hiShare = [&](const std::vector<float>& v) {
            // two 2-pole high-passes at 1.5 kHz in a row (24 dB per octave)
            double all = 0, hi = 0;
            float k = 2 * std::sin(3.14159265f * 1500 / SR), lo1 = 0, bp1 = 0, lo2 = 0, bp2 = 0;
            for (float x : v) {
                float h1 = x - lo1 - 1.4f * bp1; bp1 += k * h1; lo1 += k * bp1;
                float h2 = h1 - lo2 - 1.4f * bp2; bp2 += k * h2; lo2 += k * bp2;
                hi += h2 * (double)h2; all += x * (double)x;
            }
            return all > 0 ? hi / all : 0.0;
        };
        static const char* who[vsy::SP_COUNT] = { "kid", "mom", "dad", "visitor", "radio", "girl" };
        std::printf("\n%-42s %8s %8s %8s %8s\n", "blip / thunder", "rms", "peak", "jump", "hi%");
        for (int sp = 0; sp < vsy::SP_COUNT; ++sp) {
            for (int m = 0; m < vsy::kMuffles; ++m) {
                std::vector<float> all;
                double hs = 0, maxEdge = 0;
                static double hiClear[vsy::SP_COUNT];
                for (int k = 0; k < vsy::kVariants; ++k) {
                    auto v = vsy::RenderBlip(sp, k, m);
                    maxEdge = std::max(maxEdge, (double)std::max(std::fabs(v.front()), std::fabs(v.back())));
                    hs += hiShare(v) / vsy::kVariants;
                    float g = vsy::Params(sp).vol * 28000 / 32768.0f;
                    for (float& x : v) x *= g;
                    all.insert(all.end(), v.begin(), v.end());
                    all.insert(all.end(), (size_t)(SR * std::max(0.0f, vsy::Params(sp).gap - v.size() / (float)SR)), 0.0f);   // the gap while typing
                }
                Stats s = Measure(all, SR);
                // quiet: a typing line must sit well under the music (0.03 to 0.07 RMS) and never clip; muffled: little top left
                bool ok = s.rms > 0.004 && s.rms < 0.03 && s.peak < 0.3 && maxEdge < 0.01 && (m == 0 || hs < hiClear[sp] * (m == 1 ? 0.8 : 0.35));
                if (m == 0) hiClear[sp] = hs;
                char name[64]; std::snprintf(name, sizeof name, "blips %s %s", who[sp], m == 0 ? "clear" : m == 1 ? "dull" : "muffled");
                std::printf("%-42s %8.4f %8.4f %8.4f %7.1f%% %s\n", name, s.rms, s.peak, s.jump, hs * 100, ok ? "ok" : "FAIL");
                if (!ok) ++fails;
                if (wav && m != 1) WriteWav(std::string("/tmp/600s_") + name + ".wav", all, SR);
            }
        }
        {
            auto v = vsy::RenderHuff(0);
            Stats s = Measure(v, SR);
            bool ok = s.peak > 0.05 && s.peak < 0.9;
            std::printf("%-42s %8.4f %8.4f %8.4f %8s %s\n", "huff doggo", s.rms, s.peak, s.jump, "", ok ? "ok" : "FAIL");
            if (!ok) ++fails;
        }
        // the mask: no blips in stage directions or *actions*, nothing for pure humming
        {
            bool ok = vsy::SpokenCount("(he stops humming when he sees you) What?") == 4 && vsy::SpokenCount("*sobbing*") == 0 &&
                      vsy::OnlyHumming("hm hm hmm, hm hmm...") && vsy::OnlyHumming("mm... mm-hm, mmm...") && !vsy::OnlyHumming("Hmm. Who locked this?") &&
                      vsy::SpeakerFor("Handler two") == vsy::SP_MOM && vsy::SpeakerFor("") < 0 && vsy::SpeakerFor("Doggo") < 0;
            std::printf("%-42s %s\n", "blip text rules", ok ? "ok" : "FAIL");
            if (!ok) ++fails;
        }
        double prevPeak = 9;
        for (int d = 0; d < 3; ++d) {
            double hiClear = 0;
            for (int m = 0; m < 2; ++m) {
                auto v = vsy::RenderThunder(d, m != 0);
                double hs = hiShare(v);
                if (m == 0) hiClear = hs;
                float g = vsy::ThunderVol(d) * (m ? 0.7f : 1.0f) * 28000 / 32768.0f;   // indoors it also plays at 70%
                for (float& x : v) x *= g;
                Stats s = Measure(v, SR);
                double edge = std::max(std::fabs(v.front()), std::fabs(v.back()));
                bool ok = s.peak < 0.8 && s.rms > 0.01 && edge < 0.01 && std::fabs(v.size() / (double)SR - vsy::ThunderLength(d)) < 0.01;
                if (m == 0) { ok = ok && s.peak <= prevPeak + 1e-6; prevPeak = s.peak; }   // closer is louder
                else ok = ok && hs < hiClear * 0.5;                                       // muffled: the top is gone
                char name[64]; std::snprintf(name, sizeof name, "thunder %s %s", d == 0 ? "close" : d == 1 ? "mid" : "far", m ? "muffled" : "clear");
                std::printf("%-42s %8.4f %8.4f %8.4f %7.1f%% %s\n", name, s.rms, s.peak, s.jump, hs * 100, ok ? "ok" : "FAIL");
                if (!ok) ++fails;
                if (wav) WriteWav(std::string("/tmp/600s_") + name + ".wav", v, SR);
            }
        }
        // delays: 0.5 to 2 s, and the distance follows the delay
        {
            bool ok = true;
            for (unsigned h = 0; h < 5000; h += 7) {
                float d = vsy::ThunderDelay(h * 2654435761u);
                ok = ok && d >= 0.5f && d <= 2.0f;
            }
            ok = ok && vsy::ThunderDist(0.6f) == 0 && vsy::ThunderDist(1.2f) == 1 && vsy::ThunderDist(1.9f) == 2;
            std::printf("%-42s %s\n", "thunder delay 0.5 to 2 s, nearer = sooner", ok ? "ok" : "FAIL");
            if (!ok) ++fails;
        }
    }
    std::printf(fails ? "\n%d FAILED\n" : "\nALL TRACKS OK\n", fails);
    return fails ? 1 : 0;
}
