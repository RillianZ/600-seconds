// 600 Seconds - Soundtrack
// Every song in the game lives here. No audio files: each track is a pure
// function of time that returns one sample (-1..1). Engine-free C++, so the
// same code can feed raylib now and Unreal later.
#pragma once

namespace sixhundred {

enum Track { TRACK_SILENCE = 0, TRACK_CUTE = 1, TRACK_EERIE = 2, TRACK_DARK = 3, TRACK_CHASE = 4, TRACK_COUNT = 5 };

// Display names for the debug jukebox
const char* TrackName(int track);
const char* TrackInfo(int track);

// One sample of `track` at time `t` seconds (mono, any sample rate)
float TrackSample(int track, double t);

// How much echo each track wants (0..1)
float TrackEcho(int track);

} // namespace sixhundred
