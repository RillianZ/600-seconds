// 600 Seconds - src/audio/MusicFiles.inc (v0.20 sound, T-100)
// Streamed music files. If assets/music/<name>.ogg exists (names: TrackFile() in Soundtrack.cpp), that file plays
// instead of the code track, with the same crossfades and the same low-pass when you're outdoors.
// No file, nothing changes: the code synth plays as before. Included by main.cpp after SetMusic.
// Web: the files ride along with --embed-file assets. raylib reads ogg on every platform we build.

static const float kFileGain = 0.45f;   // a mastered file is much louder than the code tracks (about 0.03 to 0.07 RMS). Tune by ear
struct FileTrack { Music m{}; bool ok = false, playing = false; float g = 0, silentFor = 999; };
static FileTrack fileTracks[TRACK_COUNT];
static volatile float fileLpA = 0.6f;   // follows the music muffle (main thread writes, the mixer reads)
static float fileLp[TRACK_COUNT][2];

// raylib hands stream processors no user data, so each track gets its own copy of the filter
template <int K> static void FileLowPass(void* buffer, unsigned int frames) {
    float* b = (float*)buffer;
    float a = fileLpA;
    if (a >= 0.59f) { fileLp[K][0] = b[frames * 2 - 2]; fileLp[K][1] = b[frames * 2 - 1]; return; }   // not muffled: leave it alone
    for (unsigned int i = 0; i < frames; ++i) {
        fileLp[K][0] += (b[2 * i] - fileLp[K][0]) * a; b[2 * i] = fileLp[K][0];
        fileLp[K][1] += (b[2 * i + 1] - fileLp[K][1]) * a; b[2 * i + 1] = fileLp[K][1];
    }
}
static_assert(TRACK_COUNT == 10, "one FileLowPass per track");
static AudioCallback kFileLowPass[TRACK_COUNT] = { FileLowPass<0>, FileLowPass<1>, FileLowPass<2>, FileLowPass<3>, FileLowPass<4>,
                                                  FileLowPass<5>, FileLowPass<6>, FileLowPass<7>, FileLowPass<8>, FileLowPass<9> };

// Before the music stream starts (the mixer reads trackIsFile without a lock)
static void InitMusicFiles() {
    if (!audioOk) return;
    for (int k = 1; k < TRACK_COUNT; ++k) {
        const char* stem = TrackFile(k);
        if (!stem || !*stem) continue;
        const char* path = TextFormat("assets/music/%s.ogg", stem);
        if (!FileExists(path)) continue;
        FileTrack& f = fileTracks[k];
        f.m = LoadMusicStream(path);
        if (!IsMusicValid(f.m)) continue;
        f.m.looping = true;
        AttachAudioStreamProcessor(f.m.stream, kFileLowPass[k]);
        f.ok = true;
        trackIsFile[k] = true;
        TraceLog(LOG_INFO, "600s: music file %s replaces the code track %d", path, k);
    }
}

// Once a frame. The same glide as the mixer: up to 1 for the current track, down to 0 for the rest
static void UpdateMusicFiles(float dt, float muffle) {
    fileLpA = 0.6f - 0.44f * muffle;
    int cur = musicCur;
    float step = dt / std::max(0.05f, (float)musicFadeSec);
    for (int k = 1; k < TRACK_COUNT; ++k) {
        FileTrack& f = fileTracks[k];
        if (!f.ok) continue;
        float tgt = k == cur ? 1.0f : 0.0f;
        f.g = tgt > f.g ? std::min(tgt, f.g + step) : std::max(tgt, f.g - step);
        if (f.g > 0 && !f.playing) {
            // back within half a minute (the hunt flipping between searching and spotted): carry on where it was. Later: from the top
            if (f.silentFor > 30) { StopMusicStream(f.m); PlayMusicStream(f.m); } else ResumeMusicStream(f.m);
            f.playing = true;
        }
        if (f.g <= 0 && f.playing) { PauseMusicStream(f.m); f.playing = false; f.silentFor = 0; }
        if (!f.playing) f.silentFor += dt;
        else { SetMusicVolume(f.m, f.g * gMusicVol * kFileGain); UpdateMusicStream(f.m); }
    }
}
static bool TrackFromFile(int track) { return track > 0 && track < TRACK_COUNT && fileTracks[track].ok; }
