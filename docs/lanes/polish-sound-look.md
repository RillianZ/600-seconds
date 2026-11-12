# Lane: polish-sound-look (v0.20.1)

Branch `claude-polish-sound-look-v0.20.1`, made from v0.20.0. Thunder with the lightning, speech blips, and the last two outdoor lights in the 16:50 blackout.

**Nothing in this lane was heard by ear.** There is no way to listen on the build machine. Everything was checked with numbers: an offline render of every new sound (`test/audio_check.cpp`), the screenshot runner's `sound` command printing which sounds fire and how muffled, and screenshots for the lights. Every new sound needs Noah's ears before it ships.

## Where the code is

- `src/audio/VoiceSynth.h` (new, plain C++, no raylib): how the blips and the thunder are made, the speaker table, the delay rule, and the text rules (which characters blip).
- `src/audio/Voices.inc` (new, included in `main.cpp` after `art/LookUI.inc`): turns them into sounds and plays them. Thunder after each flash, blips while a line types out, blips on bubbles.
- `main.cpp`: small hooks marked `// v0.20.1 sound-look`: the `blips` setting, three forward declarations, `InitSoundsV201()` in `InitSounds`, `VoiceBubble()` at the end of `AddBubble`, `VoicesTick(dt)` after `LookTick(dt)`, the pause row, and saving the setting.
- `art/Backyard.inc` and `art/WorldDraw.inc`: the board and the dish light ask `RoomPower(Room::Backyard)` instead of `story.PowerOut()`.
- `test/audio_check.cpp`: a new block for blips and thunder. `test/shots.inc`: new commands `sayt`, `bubble`, `calm`, `blips`, `pause` (one marked block).
- README: two layout lines.

## 1. Thunder

- Every lightning flash from `art/Look.inc` gets a thunder roll 0.5 to 2 seconds later. The delay is random per flash, and it decides the distance:
  - under 0.95 s: close. A rough crack, then a loud low roll (about 4 s).
  - 0.95 to 1.5 s: middle. A soft crack, a roll (about 5 s).
  - over 1.5 s: far. No crack, a slow low swell (about 5.5 s), quieter.
- Made in code like the other effects: brown noise through a low-pass, with five to seven rumbles inside it at random times, a little mid grit so it still reads on laptop speakers, and the crack is low-passed noise that tears on and off.
- Outdoors: clear. Indoors: the muffled version (everything above about 170 Hz taken off, what's left is the floor shaking) at 70%. In the basement: muffled at 30% (there's no lightning down there, but a roll that was already coming still arrives).
- "Calm flashes" on: the thunder is still there, at half volume, and a close one loses its crack (it plays the middle one).
- No thunder on ending cards, black screens, the quiet scenes (hush), the title, or while paused / in the memory log. A flash that comes while the game is paused gets no thunder.
- Follows the Sound volume.

## 2. Speech blips

- While a line types out in the dialogue box, a soft short syllable every few letters. Not a square wave: a little sung tone (harmonics shaped by two vowel formants, a soft start, a rounded end). Four syllables per speaker so a line doesn't drone, never the same one twice in a row, a tiny random pitch wobble.

| Speaker | Pitch | Colour | Pace |
| --- | --- | --- | --- |
| You (the kid) | high (300 Hz) | bright, a little rising | quick (every 0.075 s) |
| Mom (and "Handler two") | mid (215 Hz) | warm, open vowel | 0.085 s |
| Dad | low (116 Hz) | round, a bit of breath | 0.10 s |
| The visitor | lower (88 Hz) | breathy, falling | slow (0.13 s) |
| Finse / Voice (radio, console) | 140 Hz, flat | thin, band-limited, a little crackle | 0.09 s |
| A girl's voice | high (335 Hz) | airy | 0.08 s |
| Doggo | one tiny huff through his nose per line (none when the line barks, the bark plays instead) | | |
| The narrator, Black text, lines with no speaker | nothing | | |

- Only letters and digits blip. Nothing inside (stage directions) or *actions*, so "(he stops humming when he sees you) What?" blips only on "What", and "*sobbing*" is silent.
- Pressing E to show the rest of the line stops the blips. With "Instant" text speed a line gets one blip.
- Speech bubbles: a short run of 2 to 7 blips when the bubble pops up (more for longer text). Humming bubbles ("hm hm hmm", "mm-hm") get none, the hum itself is heard. A new bubble from the same person cuts the old run.
- Muffling, through the same `VoiceThrough()` the humming uses: same room clear; through an open doorway a little dull (low-pass about 1.6 kHz) at 75%; through a wall or a closed door dull (about 450 Hz) at 45%. Bubbles also fade with distance and stop at 14 tiles, and nobody you can't see (not visible) makes a bubble sound. Lines in the box use the same muffle when the speaker stands somewhere else in the house (say Mom calling through the wall), and are clear when the speaker has no body (radio, console).
- Quiet: a typing line sits at about 0.013 to 0.019 RMS, against 0.03 to 0.07 for the music tracks, and the peaks are well under the footsteps.
- Follows the Sound volume.
- Pause menu: new row **Voice blips: On / Off**, saved with the other settings (a sixth field; old settings files read as On).

## 3. The board and the dish in the blackout

- The painted board behind the shed now goes out with the yard lamps, last in the 16:50 room-by-room sequence (it flickers with them, or fades with Calm flashes). Before, it went out the moment the power cut. When it spells BELOW on a dark loop, that still works as before.
- "The snow beast's eye": the beast has no light of its own (its eyes are buttons). The only other outdoor light that checked the power directly is the red battery light on the satellite dish next to it. I took that to be the one meant. It now starts blinking when the yard lamps go out, not at the power cut. It was also invisible at night (drawn under the light map), so it got a tiny red glow when it blinks.

## Calls for Noah

1. Thunder delay 0.5 to 2 s, random per flash, and the distance follows the delay (close = sooner = crack + louder).
2. Thunder indoors at 70%, muffled. In the basement 30%.
3. Calm flashes: thunder at half volume and never the crack.
4. No thunder over ending cards, black screens and the hushed scenes.
5. Blips are on by default. Speaker pitches and pace in the table above. The visitor is the lowest and slowest, a little breathy.
6. Finse and the console voice get a thin radio blip. The girl's voice gets a high airy one. Not in the task, my call.
7. Doggo gets one small huff per line (not per letter), none when he barks.
8. No blips for stage directions in brackets or *actions*, none for humming bubbles.
9. Bubbles you hear through walls give away where somebody is, also the visitor in the hunt ("Knock knock, 06." through a door). That matches his footsteps, which already work that way.
10. The dish light read as "the snow beast's eye", plus its new small glow.

No new in-game lines. The only new words on screen: "Voice blips", "On", "Off" in the pause menu.

## What was checked

- Story test: ALL MEMORIES REACHABLE, 106 / 106, endings as before (no Story changes).
- `test/audio_check.cpp` (all OK): every speaker's four syllables in all three muffle levels: loudness (RMS 0.004 to 0.03 while typing), peak under 0.3, no click at the edges, and the dull / muffled versions keep much less above 1.5 kHz than the clear one (muffled under 35% of clear). The huff. The text rules (brackets, stars, humming, speaker names). Thunder close, mid and far, clear and muffled: no clipping, no clicks at the edges, right length, closer is louder, muffled loses the top. The delay always between 0.5 and 2 s.
  `g++ -std=c++17 -O2 -Isrc test/audio_check.cpp src/audio/Soundtrack.cpp -o /tmp/audio_check && /tmp/audio_check` (`wav` as an argument writes them to /tmp to listen somewhere else).
- Screenshot runner (`sound` prints the ids): a flash outdoors gives `thunder_mid+1.2s`, indoors `thunder_mid+1.4s(muffled)(vol 70%)`, with Calm flashes `thunder_far+1.5s(muffled)(vol 35%)`, and nothing 0.4 s after a flash. Mom's line in the kitchen: `blip:mom x5`. Her bubble from your room (open doors): `(dull)(vol 42%)`, from outside the kitchen wall: `(muffled)(vol 28%)`, her line through the wall `(muffled)(vol 45%)`, from 19 tiles away nothing. Humming bubble: nothing. Doggo: `huff:doggo x1`. The visitor, the radio, the kid: their own blips. Blips off: nothing.
- Screenshots: the board through the blackout frame by frame (the house windows go first, the board after them), the dish light blinking after the yard lamps, the pause menu with the new row, the setting saved Off and an old settings file read as On.
- Not checked: how any of it sounds. The runner has no sound card here, so the raylib side (volumes, pitch) only ran as code. Web and Windows builds not built (main session).
