# Lane: sound (v0.20)

Branch `claude-lane-sound-v0.20`, made from v0.19.0. Tasks T-033, T-034, T-035, T-148 and the engine side of T-100.

**Nothing in this lane was heard by ear.** There is no way to listen on the build machine. Everything below was checked with numbers (offline renders, a capture of the real mixer through a null sound card, the screenshot runner printing what plays). Every sound and every track needs Noah's ears before it ships.

## What was built

### T-033 Music
Five new code tracks in `src/audio/Soundtrack.cpp`, same synth style as the old ones (sines, triangles, a band-limited square, the same note envelopes):

| Track | When it plays | What it is |
| --- | --- | --- |
| Title - Snow on the Vidda | title screen (before: the Act track) | 60 bpm. Mom's song at half speed on a music box, C-Am-F-G sine pad, soft bass. Every other time round a few notes are missing |
| Basement - Blue Light | the kid is in the basement, before the knock | E-F cluster that breathes, two console pings, and every 24 s the first four notes of her song, a quarter tone flat |
| Recap - The Log | the memory recap after a loop | 80 bpm plucked arpeggio Am7-Fmaj7-Cmaj7-Em7. Fits under the recap chimes (E, B, E) |
| Ending - Far Away | every ending card except Screen Time (before: the Act 3 drone) | 54 bpm. Mom's song moved one step down the scale so it's minor, slow sine piano over Am-F-Dm-G, a far bell |
| 16:50 - He Sees You | the hunt while he sees you (vmode Chase) | the same heartbeat twice as fast (every other beat lands with the searching track), a low pulse in 16ths, two whines rubbing |

- The hunt is now "searching" (the old 16:50 track) and "spotted". It goes to spotted in 0.6 s, stays at least 3.5 s after he loses sight (so it doesn't flicker), then fades back in 2 s.
- Screen Time (the joke ending) gets the Act 1 tune on its card, not the sad theme.
- The mixer was rewritten a little: each track has its own gain that glides up or down, so a track change in the middle of a crossfade can't jump. The echo amount glides too.
- The jukebox (debug, M) lists all ten tracks and marks the ones a file replaces.

### T-034 Sound pass
New sounds, all made in code, in `src/audio/SfxV20.inc`. Story beats use `Sound("id")`, the presentation plays them in `Play()`.

| id | Where it's hooked |
| --- | --- |
| `power_cut` | every "the power dies" moment: the knock, the knock in loop 1, hinge C (downstairs), the early storm at 16:40. The knock now waits one second after the power cut. The basement hum also stops when the power is out |
| `lock` | locking and unlocking any door with the key, the front door lock and unlock, the bathroom lock (hinge A), unlocking the basement (both ways), the basement bolt on Mom, Mom unlocking the basement at 16:30, a parent unlocking the front door again |
| `phone_pickup` / `phone_hangup` | you answer the 16:22 call (pickup, then the hang-up on `*click*`), Mom answers it in the kitchen, picking up for a dial tone |
| `vhs_on` / `vhs_off` | the tape in the basement: the tape goes in with a clunk, then hiss under her humming until the "snow on the screen" fade is over. The hum on the tape also wobbles a little, like old tape |
| `trunk` / `trunk_open` | the lid coming down on you (the old thud), the lid in the stay-awake ending, Express Delivery (you pull it down yourself, he opens it), looking in his trunk |
| `kick`, `scrape`, `door_break` | him kicking a locked door, and a furniture barricade (kick, the furniture shifts, kick, ...), and the door giving. Also the bathroom door in the story (Mom locked in). These are placed: quieter with distance, lower and quieter through a wall or a closed door |

### T-035 Muffled through walls
- Mom's hum: clear in the same room, a little dull through an open doorway, quieter and much duller (low-pass about 500 Hz) through a wall or a closed door. Before it was only "same room or not".
- Dad's hum works the same way (T-148).
- Music: a gentle low-pass (about 1.2 kHz) when you're outdoors, and on the hunt music when you're down in the basement. Not on ending cards, black screens, the recap or menus.
- The kicks and the door breaking are muffled the same way (lower and quieter through walls).
- **There are no speech blips in the game.** Talking is text only, so there was nothing to muffle there. If blips get added later, `VoiceThrough()` in main.cpp gives the gain and muffle for any voice.
- Cost: three one-pole filters per sample in the mixer, and one line-of-sight check per frame per humming parent.

### T-148 Dad's hum
- `DadHumSample()` in Soundtrack.cpp: his own tune in G, low (D3 to B3), slow, a phrase and an answer, then a long gap while he works. Notes slide up into place, mouth-shut timbre.
- `Story::DadHums()`: in the shed and at the woodpile, 16:08:30 to 16:40. Not during the two radio calls (16:10, 16:33), not if he's cold with you, scared or broken, not once you're tainted (like Mom), not barred in the shed.
- You hear him within 8 tiles, from the yard too, muffled through the shed walls.
- For players without sound: a bubble now and then, like Mom's.
- One story line, once a loop (not loop 1), when you walk right up to him while he hums.

### T-100 Music files (engine side only)
- `src/audio/MusicFiles.inc`. If `assets/music/<name>.ogg` exists, it plays instead of that code track. No file, nothing changes. No music files were added.
- Names: `act1`, `act2`, `act3`, `hunt_searching`, `hunt_spotted`, `title`, `basement`, `recap`, `ending`.
- Files use the same crossfades and the same outdoor low-pass (a raylib stream processor). They loop. Coming back within 30 s carries on where it was (the hunt flipping between searching and spotted), later it starts from the top.
- `kFileGain = 0.45` turns files down to sit near the code tracks. That number is a guess: set it by ear once there's a real file.
- Web: `--embed-file assets` already takes the whole folder, so files in `assets/music` ride along.

## Lines for Noah to react to
- Dad, once a loop, when you walk right up to him while he hums: "(he stops humming when he sees you) What? A man can hum."
- Dad's hum bubble, now and then near him at work: "mm... mm-hm, mmm..."

## Calls for Noah
1. The title now has its own track, the same for everyone. Before, it followed your Act (a dark save got the dark drone on the title).
2. Mom's hum in the kitchen is now dry (no echo) so it can be muffled properly. The tape is unchanged (echo, alone).
3. Ending theme: all endings share one theme. Screen Time gets the Act 1 tune. Want different themes for the true endings?
4. The knock waits one second after the power cut. Before, the knock was instant.
5. Spotted music: 0.6 s in, holds 3.5 s, 2 s out. Too jumpy or too slow?
6. Outdoors the music gets duller. If it's too much under the wind, it's `0.44f` in the mixer (and in MusicFiles.inc).
7. Dad doesn't hum once you're tainted, same as Mom.
8. Music file names (list above) and `kFileGain`.

## What was checked
- Story test: ALL MEMORIES REACHABLE, 86 / 86 memories, 13 endings (same as v0.19).
- `test/audio_check.cpp` (new): renders every code track for 60 s and both hums, checks loudness, peak, sample jumps (clicks) and silence, plus a searching-to-spotted crossfade. All pass. RMS: Act 1 0.038, Act 2 0.031, Act 3 0.049, searching 0.062, spotted 0.071, title 0.033, basement 0.032, recap 0.029, ending 0.031. `/tmp/audio_check wav` writes 20 s of each track to /tmp for listening somewhere else.
  `g++ -std=c++17 -O2 -Isrc test/audio_check.cpp src/audio/Soundtrack.cpp -o /tmp/audio_check && /tmp/audio_check`
- Linux SHOTS build under xvfb. New runner commands: `sound LABEL` (prints the current track, fade, every voice and muffle level, and every sound id played since the last call), `title`, `recap`, `ending ID`, `jukebox`. Checked this way: title, Act 1, basement, recap, ending, Screen Time card, searching, spotted and back, hunt in the basement (muffled, hum off), Mom through an open door (0.35) and a closed one (1.0), Dad in the shed (clear), outside the shed (muffled), at the woodpile (clear), the lock, phone, tape, trunk ids firing, the power cut then the knock, kicks and scrapes at a barricade (muffled through the door), kicks at the bathroom from the yard (quieter).
- With a null sound card (ALSA null device in a temporary HOME) the real mixer ran and its output was captured: no clipping, levels as expected. Two throwaway ogg files (a sine and noise) in the build folder, not the repo, replaced the title and Act 1 tracks: the log says so, the jukebox marks them, they show up in the capture, and outdoors the noise file loses almost everything above 4 kHz (30% of its energy indoors, 2% outdoors).
- Not checked: how any of it sounds. The web build and the Windows build were not built (main session does that).
