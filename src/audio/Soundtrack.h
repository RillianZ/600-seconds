// 600 Seconds - Soundtrack
// Every song in the game lives here. No audio files: each track is a pure
// function of time that returns one sample (-1..1). Engine-free C++, so the
// same code can feed raylib now and Unreal later.
#pragma once

namespace sixhundred {

enum Track { TRACK_SILENCE = 0, TRACK_CUTE = 1, TRACK_EERIE = 2, TRACK_DARK = 3, TRACK_CHASE = 4,
             // v0.20 sound: the hunt is split in two (TRACK_CHASE is "searching"), plus four places and moments
             TRACK_SPOTTED = 5, TRACK_TITLE = 6, TRACK_BASEMENT = 7, TRACK_RECAP = 8, TRACK_ENDING = 9,
             TRACK_COUNT = 10 };

// Display names for the debug jukebox
const char* TrackName(int track);
const char* TrackInfo(int track);

// One sample of `track` at time `t` seconds (mono, any sample rate)
float TrackSample(int track, double t);

// v0.10: Mom's song, hummed. The Act 1 lead, slow and an octave down (mono, -1..1)
float HumSample(double t);

// How much echo each track wants (0..1)
float TrackEcho(int track);

// v0.20 sound: Dad's hum. Deeper, slower, his own tune, with long gaps while he works (mono, -1..1)
float DadHumSample(double t);

// v0.20 sound: the file name stem for a track. If assets/music/<stem>.ogg exists, the game plays that instead of the code ("" = silence, never a file)
const char* TrackFile(int track);

} // namespace sixhundred
