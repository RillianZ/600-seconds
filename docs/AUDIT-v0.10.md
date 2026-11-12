# 600 Seconds audit and the v0.10.0 branch

Written 12 Nov 2026 by Claude (Fable 5.2). Branch: `fable-audit-20261112`. Nothing here is merged. Big spoilers.

## How it was checked

- Read all of `Story.cpp`, `Story.h`, `World.h`, `Soundtrack.cpp` and the test. Read `main.cpp` in full for sound, the log, the HUD, input, the chase and the frame loop.
- Compared with the task list sheet (67 tasks, decisions D1 to D15).
- The story test passes: 56 of 56 memories reachable, all 8 endings reached, every achievement is in the new list.
- Played the desktop build on a virtual screen and the browser build in Chromium, with screenshots: the log pages, Doggo at the door, the car at the gate, the basement and the tape.
- Not checked: sound by ear, a phone, a real Windows PC. Sound levels were only measured as numbers.

## What the audit found

### Bugs
| Found | Status |
| --- | --- |
| Answering the phone while Mom is locked in the basement did nothing. No line, and the call was used up. | Fixed |
| The basement was drawn under the yard when you stood at the gate. From the basement you could see the yard, and snow fell indoors. | Fixed |
| Doggo was moved down into the basement with you. The story says he will not go near it. | Fixed |
| Walking up to his car put you on the vidda and ended the loop as "running away". | Fixed, the car is solid |
| Mom's menu could reach 13 rows and Dad's 9. One more question and they would leave the screen. | Fixed with a sub menu |
| Task list: T-030 (ambience) was marked Done on 5 Nov by mistake. It was not built. | It is built now |

### Threads the game started and never finished
| Thread | Where it was | Status |
| --- | --- | --- |
| "An old watch" in Dad's cabinet | Mentioned twice, did nothing | Paid off |
| "We do this every time", "You always pick that one" | The man's lines | Paid off |
| "They said you'd get there eventually" | When you pull the knife | Paid off in Appendix C |
| Doggo whines at the door at 16:35 | A bubble, no effect | Paid off |
| "Flag. Forty-four." | Dad mutters it, only a guide could use it | Paid off, you can watch him |
| Why the hour repeats | Never said anywhere | Said once, in the thin folder |
| The 31 km leash | Shown in False Hope, gave no memory | Now a memory that opens an ending |
| Mom hums "a carol" on the tape | The player had never heard it | She hums it in the kitchen now |
| The kid has no name | Kiddo, buddy, sweetie, 06 | Used on purpose |
| A clean player tops out at False Hope | Task T-063 | One Good Hour |

### Still thin (not done on this branch)
- The chase: you still see him through walls in the dark (T-042). Footsteps help, the drawing does not.
- No close ups or portraits (T-046, T-047). Every reveal is still text.
- Only four music tracks (T-033). Every ending plays the Act 3 drone.
- Mom and Dad mostly stand still between events (T-087). Dad now walks to the flagpole, that is all.
- No noise distraction in the chase (T-088).
- The dark route got one new page (Appendix C) and a few ending lines. The clean route got most of the new work.
- `main.cpp` is now about 3,500 lines in one file (T-106).

## What is new in v0.10.0

The full list is in `CHANGELOG.md`. In short:

- 11 new memories (45 to 56) and a new log topic, The program.
- 6 small quests: the tiny watch, his car, Doggo out, the phone plug, Mom's song, Dad at the flagpole.
- 1 new ending (One Good Hour), 1 new reality shift (somebody is in the bathroom), 8 new achievements (28 to 36).
- The log has a Timeline page and an Awards page.
- The narrator reacts to how the last loop ended, and gives a hint after three dry loops.
- 13 new questions for Mom and Dad. Radio, TV and several objects change over the hour and over the game.
- Ambience, footsteps and Mom's humming.
- A GitHub Action that runs the story test on every pull request.

## Story calls Claude made. Yours to keep or throw out

These touch decisions you have not answered in the sheet. Each one is text only and easy to remove.

| Decision | What this branch does | To undo |
| --- | --- | --- |
| D3, how much of the premise | Said once in plain words, in the thin folder (ORIGIN PROTOCOL). The watch is the anchor. Today's juice was a placebo, so he keeps this hour. | Remove `fi_protocol` and the `protocol` memory |
| D4, who the visitor is | He wears an old Origin watch. He was a subject once. He does not remember loops, but lines leak through. | Remove `ShowWrist`, `v_every`, and the `collector` and `echo` memories |
| D2, the trap | Kept. Appendix C says a subject who kills his collector, and knows to take the switch first, replaces him. Monster and End of the Program get extra lines only if you have read it. | Remove the appendix block and the `graduation` memory |
| D5, a clean ending | Added One Good Hour. Nobody is saved. | Remove `DinnerEnding` and the `e_dinner` option |
| New, the clean key route | Watching Dad at 16:44 gives you the key with no darkness. The old hidden route still works and still gets the narrator's accusation. | Remove the `flag44` event |
| New, the watch as anchor | Fits your idea that the watch is the thread through the series. | Part of D3 |

## Things for you to test

1. Sound. Wind outside, the storm, the basement hum, footsteps on each floor, his footsteps through a wall, and Mom humming in the kitchen. Then the tape: the hum should be the only sound.
2. 16:35: let Doggo out (talk to him or use the front door), then get caught.
3. 16:44: stand in the yard near the flagpole. Then take the key and go down.
4. The gate between 16:44 and 16:46: the car. Debug key 3 hops there.
5. Dad's cabinet: take the old watch. Show it to Mom, Dad and the man.
6. The filing cabinet: the thin folder. Read it again once you know about the box on his belt.
7. Phone: with "The caller" known, pull the plug before 16:22, then go with him quietly.
8. After the tape: hum her song.
9. False Hope, then convince them again and choose to sit down.
10. Debug panel, force the next loop: "the bathroom".
11. The log: Timeline and Awards pages. On a phone too, the tab row is tighter now.
