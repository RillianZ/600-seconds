// 600 Seconds - test/audio_check.cpp (v0.20 sound)
// Nobody can listen in the build machine, so this renders every code track offline and checks the numbers:
// not silent, not clipping, no big jumps between samples (clicks), loudness close to the others.
//   g++ -std=c++17 -O2 -Isrc test/audio_check.cpp src/audio/Soundtrack.cpp -o /tmp/audio_check && /tmp/audio_check
// Optional: /tmp/audio_check wav   writes each track to /tmp/600s_<track>.wav (20 s) to listen to somewhere else.
#include "audio/Soundtrack.h"
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
    std::printf(fails ? "\n%d FAILED\n" : "\nALL TRACKS OK\n", fails);
    return fails ? 1 : 0;
}
