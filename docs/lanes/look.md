# Lane: look (v0.20)

Branch `claude-lane-look-v0.20`, on top of v0.19.0. Tasks T-043 to T-049 and T-103. Everything is drawn in code, no image files, so it stays cheap if the game moves to Unreal later (D8).

## Where the code is

- `src/art/Look.h`: forward declarations, included near the top of `main.cpp` so the old art files and the beat code can call the new art.
- `src/art/Look.inc`: the world. Power room by room, lightning, headlights, the stage 1 flux props, the darkness details, the kid standing still.
- `src/art/LookUI.inc`: the screen. Portraits, the watch, the HUD and fridge list, memory log handwriting, close-ups, ending pictures.
- Small marked hooks (`// v0.20 look`) in `main.cpp` (includes, `DrawWatch`, `DrawHUD`, the journal entry text, the beat draw switch, the Interact and Choose calls, `LookTick`), `art/WorldDraw.inc` (lamps per room, light map, glow pass, wall props, mug in the y-sort, kid idle, lightning, portraits) and `art/Sprites.inc` (things that light up ask their own room for power).
- No Story changes at all. The close-ups are started by the presentation (see T-046).
- `test/shots.inc`: new commands `luse ID`, `lchoose ID`, `journal TAB`, `newmem ID`, `bolt`, `sweep K`, `idle N`, `ending ID`, `att mom|dad cold|scared|broken`, `say WHO TEXT`, `sister`, in one marked block.

## T-043 Darkness in the world, not a number

The red DARK number is gone from the HUD. It still shows in debug mode (`?debug`, top left).

What shows it instead, all driven by `darkPoints` (0 to 15) and `tainted`:

- **A tally over your bed.** One line scratched into the wallpaper per dark point, in groups of five. Pale, then pinkish once tainted. Calls for Noah: a literal count on the wall, maybe too on the nose? It's small.
- **Your drawing of the family** (on the wall right of your window). 3+: the sun is coloured over in black. 6+ (tainted): Mom and Dad crossed out in red. 10+: a tall figure in a hat added at the edge. 15: you, coloured in black.
- **The family photo in the hallway**: once tainted, Mom's and Dad's faces are scratched out.
- **His shadow.** From 10, the kid's shadow on the floor is a tall man with a hat.
- **Standing still** (3.5 s without moving, no talking, not in the hunt, not loop 1):
  - 0 to 2: he lifts his arm and checks his watch (it blinks).
  - 3 to 5: he looks left, right, back, then at you. Not smiling.
  - 6+: he turns and stares at you. His eyes pulse red. From 10, a smile that isn't his.
- **The ORIGIN watch.** The LCD goes from ice blue toward grey-red (red at 15). The red dot in the Origin mark beats like a pulse from 1 point up, faster the darker you are. The glass cracks at 4, again at 8, shatters at 12. Tainted: the label says UNIT 06 instead of MODEL 06. The watch on the title screen shows it too (darkness survives a new game).

## T-044 Quiet flux from loop 2

Three props that are always there, so you can notice when they're wrong:

- a family photo on the hallway wall (above the corridor, between the bathroom and Mom and Dad's doors),
- a clock on the kitchen wall (between the pantry and the advent calendar), showing the real time,
- Mom's coffee mug on the counter by the sink (steaming).

From loop 2 (Stage 1 and up), each loop is seeded by the loop number: some loops nothing, most loops one thing, now and then two. Stable for the whole hour.

- The photo hangs tilted (left or right).
- The clock is stopped at 15:55 (juice time), stopped at 17:00, or runs backwards.
- The mug is in the middle of the kitchen floor, on top of the fridge, or next to Doggo's bowl. Cold (no steam).

Loop 2 itself happens to be a quiet one with the current seed; loops 3 to 15 have something. The older Act 2 shiver and red shudder are unchanged.

## T-045 The big moments, staged with light

- **16:50, the lights die room by room.** From the generator outward: shed, living room, kitchen, hallway, Mom and Dad's room, bathroom, your room last, then the porch lamp and the eave lights. 0.42 s apart, each one flickers for a moment first (a soft fade with "Calm flashes" on). Windows, lamps, the tree, the TV, the stove dials, the desk lamp all follow their room. The same sequence runs in the early storm reality (16:40).
- **Lightning in the storm.** Every 8 to 16 seconds while it storms (not in the basement): a double flash over the screen (a single soft one with Calm flashes). If he's outside and on screen, he's a black silhouette in the flash with a long shadow. If he's sitting in his car, the windscreen goes white with his shape behind the wheel. In the hunt he only shows in the flash if you could see him anyway, or you're outside too, so it doesn't give his position away through walls. Call for Noah and the audio lane: no thunder sound yet.
- **16:43, his headlights.** When the car turns in (only with the engine on, so not if you already have his key), a window of light slides across the wall and floor of your room, then across the living room and kitchen, and across the snow in front of the house. 3.2 seconds, real time, so it also plays if you're holding wait.

## T-046 Close-ups

A full screen picture while the lines that go with it play, with the dialogue box (or the words of a Black beat) under it:

- **The mirror.** Bathroom tiles, the mirror, your face in it. Stage 0: you, smiling, holding the watch up. Stage 1: you. Stage 2: you, with a faint copy a step behind. Stage 3+: an older you (his hair is your hair), smiling. Stage 4: his mouth moves. Reality "Sister": fogged up, with a smiley drawn in the steam.
- **The wall of screens.** Twelve little CRTs: your bed, the bathroom, the kitchen with Mom, Dad on the couch, the hall, the yard, the shed, the gate (his car once it's there), their bed, the snow beast out back, the front door, and the small one with the watch counting. In the hunt: all black but the watch.
- **The chair with straps.** One bulb, the kid-sized chair, straps at the wrists, chest and ankles, worn pale where the wrists go, a loose strap, the old TV on its cart, a drain in the floor.
- **The tape.** The old TV, the VHS picture of the same chair with a six year old in a red sweater and a watch far too big. Noise, a tracking band rolling down, "PLAY 00:41:xx" and "S06 W312" on screen. When the line mentions Mom walking in or the juice, she's in the picture with the glass and the straw. When the line says he smiles, he smiles.

How it hooks in: the presentation sets the close-up when you use `mirror`, `screens` or `chair`, or pick `vt_play`, and drops it when the beats run out or move on to a Fade with time, a Card, an Ending or a scene. No change to `Story.cpp`, so the knife lane's mirror lines are untouched.

## T-047 Portraits

New painted busts for the kid, Mom, Dad and the visitor (Doggo keeps his old one). Drawn on a 19 x 19 grid in the box over the dialogue. The mouth moves while that person's line is typing out. They blink.

- **You:** smiling at Stage 0, plain at 1, worried at 2+, scared in the hunt. Tainted: pale, staring, red eyes that pulse, a wrong smile from 10. With the jacket on: parka and the red hat.
- **Mom:** smiling at Stage 0 and 1, plain from Stage 2 (the smile is gone). Cold, scared or broken by her attitude. In the hunt: cold if she's against you (Dark stance or the Handlers), scared if she's on your side. After the tape she's handler two: hair pulled back tight, no earrings, and cold.
- **Dad:** same rules. Beard, the knitted band.
- **The visitor:** hat, collar up round his jaw, the tie. Stage 0 to 1 his eyes are only shadow under the brim. From 2 two pale points. On the floor, tied up, or once he remembers: the hat is off and he's an older you (tired, or broken when he remembers).
- "Handler two" as a speaker gets Mom's face too.

## T-048 HUD

- The watch carries the time. The loop number sits small on its strap, under it.
- MEMORY x/y and IN HANDS sit under the watch, smaller and dimmer, on a soft dark band so they read on snow and floor. IN HANDS only shows when you hold something (and DOGGO STAYING).
- CHORES is gone from the HUD. In the kitchen there's a small yellow note top left with five boxes. Walk up to the fridge and it opens into the whole list (the wording from the fridge line: Feed Doggo, Trash out, Dishes, Stack the firewood, Clean your room), ticked and struck through as you go, and "All by 16:40 = a present" or "All done. PRESENT!".
- The DARK number only in debug mode.

## T-049 Memory log handwriting

- Stage 0 and 1: the kid's hand. The UI font in pencil blue-white, each word sitting slightly off the line.
- Stage 2 and up: the narrator's font (Lora), like before.
- On the dark path (tainted), entries you didn't find this loop turn a cold blue-grey. What you found this hour stays warm red.

## T-103 Ending pictures

Every ending card gets a small picture (240 x 96 UI units) over the title, faded in:

- Monster: the house under a red sky, one red window, you in the snow with the knife.
- End of the Program: the wall of screens, every one broken, a spark, you small in front.
- The Longest Survivor: you awake in your window, his car driving away without you.
- Screen Time: the Fliptendo, LAST DUNGEON.
- False Hope: the three of you on the road over the vidda, headlights behind you.
- Good Boy: his house empty, his bowl full, his collar in the snow.
- They Paid: a door, a crack of light, something red on the floor.
- One Good Hour: the table, candles, three bells, you and him at the table.
- The Empty House: Dad's chair with him in it, the tree dark, the stove still going.
- Same House: the house lit, a small kid in red opening the door, you in the coat at the path, the car.
- After: the car at the gate early, lights on, you in the window.
- The Handlers: Finse, a long low building, one window lit, the two of them walked round the back.
- Last Toll: the table, four places, the riskrem, Mom and Dad either side of you, bells.

## Calls for Noah to react to

- A tally of dark points scratched over the bed. Literal, small. Keep, or just the drawing?
- The kid turning to stare at the camera when you leave him standing (6+).
- The man-shaped shadow from 10.
- UNIT 06 on the watch once tainted.
- Order of the lights dying (shed first, your room last) and the gap (0.42 s).
- Lightning every 8 to 16 seconds. Without thunder for now.
- Headlights through your room, though your window faces the back. Asked for in the task, physically a stretch.
- Mug, clock, photo as the three flux props, and the clock stopping at 15:55 (juice time).
- Mom's face after the tape: hair back, cold, for the rest of the game.
- The visitor's face under the hat is an older you when the hat comes off.
- The mirror at Stage 3+ showing that same older face.
- All ending pictures and what's in them.
- No new in-game lines. The only new words on screen: "CHORES", the five chores (taken from the fridge line), "All by 16:40 = a present", "All done. PRESENT!", "LOOP n", "UNIT 06", the screen labels (BED, BATH, KITCHEN, LIVING, HALL, YARD, SHED, GATE, M+D, BACK, DOOR, S06), "PLAY 00:41:xx", "S06 W312", "LAST DUNGEON".

## What I checked

Screenshots from the Linux SHOTS build for every task: the kid's room at dark 0, 4 and 11 (tally, drawing, stare, shadow), the kitchen and hallway props in flux and not, the fridge note small and open, the power sequence frame by frame from 16:49:56, the headlight sweep, lightning with him in the car and walking up, all four close-ups (the mirror at Stage 1, 2, 3, 4 and in the sister reality), the tape with and without Mom, all four portraits in each expression, Mom as handler two, the visitor at Stage 1 and 2, all 13 ending cards, the log in kid hand, narrator, and dark with old cold and new warm, the title screen, loop 1 and the start of the hunt. Story test: ALL MEMORIES REACHABLE, unchanged (no Story changes).

## Not done / left open

- Thunder (audio lane).
- The fridge note doesn't show while Mom is talking to you in the kitchen in a non-Say beat (choices hide it), on purpose.
- The things in `Sprites.inc` that light up now follow their room; anything drawn outside `DrawObj` that checks `PowerOut()` directly (the board behind the shed, the snow beast's eye) still switches at once. They are outside, so they go last anyway.
