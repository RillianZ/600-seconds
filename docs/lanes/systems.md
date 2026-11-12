# Lane: systems (v0.20)

Branch `claude-lane-systems-v0.20`, on top of v0.19.0. Five tasks: T-087, T-083, T-086, T-088, T-092.
Story test: passes, ALL MEMORIES REACHABLE, 87 memories (one new), 13 endings.
Every in-game line below is new and a proposal for Noah to react to.

## What I built

### T-087 Fuller parent days
- New data file `src/game/Schedule.h`: a table of spots for Mom and for Dad across the hour, each with an optional idle line.
  `Story::Mom()` and `Story::Dad()` still check every fixed beat first (locked away, bathroom, phone, basement, corner, firewood, flagpole, the knock), then fall back to the table.
- Mom: kitchen, peeling potatoes at the sink (16:02:30), the ribbe in the oven (16:06:30), fixing the tree (16:11), **Kaptein Fjord on the couch (16:14:30 to 16:20)**, kitchen, phone (as before), basting at the stove (16:24), basement (as before), presents under the tree (16:33), kitchen, **lefse snack in the kitchen (16:40 to 16:43)**, stove, bathroom (as before), kitchen.
- Dad: the paper (as before), shed with the radio (as before), **Kaptein Fjord on the couch (16:14:30 to 16:20)**, shed (whistling at 16:25 as before), shovelling the path to the gate (16:27 to 16:31), shed (the 16:33 radio, as before), firewood (as before), shed, **the snack in the kitchen (16:40)**, flagpole (16:43:30, as before), feeding the wood stove (16:45:30), kitchen.
- On the couch they face the TV, at the stove and sink they face the counter (new `NpcState::face`, applied in main.cpp).
- The idle lines are bubbles, only when you're within 9 tiles when it happens, only on a normal day (not scared, not cold, not after the hum).
- Two new timeline rows: 16:15 "Mom and Dad sit down for Kaptein Fjord's Christmas special." and 16:40:40 "Lefse in the kitchen. One each." You see them by being in the living room at 16:15 / the kitchen at 16:40:40.
- Kept: phone at 16:22, basement 16:28 to 16:33, bathroom 16:46 to 16:48:30 (hinge A), Dad in the shed whenever the hinge B plank and "Need help?" need him (16:08 to 16:14:30, 16:20 to 16:27, 16:31 to 16:40 minus the firewood), the flagpole at 16:44, the corner.
- Changed to fit: Mom's corner (later loops) is now 16:11 to 16:14:30 instead of 16:12 to 16:18, so she can sit down for the show. Dad's "Come on, old girl" generator bubble moved from 16:15 to 16:12 (he's on the couch at 16:15). Mom only hums in the kitchen now.

### T-083 Fortress, expanded
- **They fight you on the lock.** Whoever is in the house (not in the shed, basement, bathroom or yard) unlocks it after 90 seconds. First time as before. Second time a scolding and mood -1. From the third time they watch the door: 30 seconds, and another mood hit. Keep at it and you get grounded.
- **Nobody home (new scene).** Lock the front door with both of them shut away (Mom bolted in the basement or locked in the bathroom, Dad barred in the shed). At the knock nobody opens. He talks to you through the door: he locked this door once too, got everybody out of the way first, thought it wouldn't count if nobody opened it. It counts. It just means he comes in the back. You don't have a back door. "You've got a window that sticks, though."
  If Dad is in the shed, the hinge B scene happens on his way round. Then one bang low on the frame of YOUR window, exactly where you always hit it, and he climbs into your room. The hunt starts there ("He's in your room. Get out of there."). If Mom is locked in the bathroom, hinge A plays after, from inside.
- New memory `nobody_home` "The back way" (The visitor). New achievement Nobody Home. Fortress and Locked doors are still given.
- The old Fortress (living room window) still happens whenever somebody is still in the house.

### T-086 The early knock
- Tainted and 10 or more darkness points (`DARK_EARLY` in Schedule.h): the knock moves to 16:48 (`EARLY_KNOCK` = 120 s). Everything he does is counted back from the knock, so the car comes at 16:41, he's at the window 16:42:20 to 16:44:30, back in the car at 16:45.
- Narrator, once ever (kept in the timeline's seen events): "He came early that night. Two minutes. After what I'd done, I don't think they wanted to give me two more minutes in that house." It plays when the car arrives.
- The car key (v0.19) still works on top: take his key and he walks up 40 seconds after he finds the car dead, whichever is earlier. Telling Dad and the console still move it to 16:40.

### T-088 Doors in the hunt
- One block in main.cpp's hunt: if you open, shut or slam a door (you're right at it, nobody else is), he hears it like running feet. Open: 6 tiles, shut: 8 tiles, slam (shut within 0.6 s of running): 14 tiles and a thud. He goes to look at the door.
- `OnHeard("door_open" | "door_shut" | "slam")` in Story: one bubble each per loop, and it gives Running is loud.

### T-092 The rumor map
- New last page in the memory log: **Rumors** (after Awards). Data in `src/game/Links.h`: 63 cards on a 5 column grid, 58 links.
- Found memories are cards with their title. An unknown memory linked to one you have is a "?" card. Everything else is hidden. Lines between both-found cards are teal, to a "?" grey. Cards you found this loop have a gold edge.
- UP/DOWN (or the wheel) pans. LEFT/RIGHT still turns pages. "N of 63 on the map" bottom right.
- Clusters, top to bottom: who they are (handlers, the tape), he was me (the five pieces around He remembers, the back way, his car), nowhere to go (31 km, zero-five), downstairs (basement, protocol, the box), Doggo and who looks for you.

## Calls for Noah to react to
- All the idle lines (Schedule.h), the Nobody Home scene lines, the narrator line, the three door bubbles ("(a hinge, somewhere) ...Hello, door.", "Shutting doors on me, 06?", "(he turns toward the bang) Temper, temper."), the two front door scoldings, the memory text and the achievement hint.
- Dad shovelling "Nice and clear for... for Julenissen." (he knows who's coming).
- Mom shaking one of your presents and listening (they're empty, see "Nothing in the boxes").
- Nobody Home needs BOTH parents out of the way. Only Mom or only Dad gone gives the old window scene.
- The early knock threshold (tainted and 10+ darkness) and size (2 minutes). One side effect: in those loops he's back from the window before Dad checks the flagpole at 16:44:40, so no "A Nod Across the Yard" there.
- The corner moved and got shorter (16:11 to 16:14:30).
- The rumor map's links are my reading of the memories. Easy to change in Links.h.

## Shared spots I touched (all marked `// v0.20 systems`)
- Story.h: `NpcState::face`, one per-loop block (`fortressFights`, `nobodyHome`, three methods).
- Story.cpp: Mom()/Dad() tails, MomHums (kitchen only), Visitor() (on the step while he talks), Tick front door rule, one CheckEvents block (idle lines, tv15/snack40, early knock), the knock (`NobodyToOpen()`), skip "Evening." after Nobody Home, HingeShed (outside variant), skip the second HingeShed, the dad_gen bubble minute, OnHeard, new NobodyHome() after HingeShed, memory/achievement/timeline blocks, BeginLoop reset.
- main.cpp: JournalTabs +1, the rumor map block (before DrawJournal), tab name/count lines, journal page count, face block, `nobody_home` fade, `back` hunt start and its banner, the door hearing block, `#include <set>`.
- test/sim.cpp: includes Schedule.h and Links.h, `gEarlyKnock = false;` at the top of main (the older loops are all timed for 16:50, and the main story is dark from loop 8 on), my block before "Clean key route, the findable one".
- test/shots.inc: `journal`, `jscroll`, `unjournal`, `allmem`, `forget`, `lockfront`, `state`.
- README: two layout lines.

## What I checked
- Story test after every step. The new loops: parent positions at 16:09, 16:15, 16:22, 16:31, 16:36, 16:40, 16:44, 16:47; the TV bubble and timeline row; the four front door fights and grounding; nobody unlocks it with both shut away; the Nobody Home loop (Dad barred, Mom in the bathroom: huntStart is "bath", window banged, he stands on the step); the early knock at 16:48, car at 16:41, narrator once, plus the car key on top (he comes before 16:46); not dark enough stays 16:50; the slam bubble once; every rumor card is a real memory, no two on one spot, every link points at a card.
- Loop 7 in the old script (Dad barred, Mom bolted, front door locked) now gets the Nobody Home scene too.
- Screenshots (Linux build, 960x540) in `docs/lanes/systems/`: TV at 16:15, the snack, through the door, in your room, the hunt start, the rumor map (early and full, top and bottom), he hears a door, the early narrator line.

## Not done / for later
- No new art: they stand next to the couch, they don't sit on it. The look lane could add a sitting pose.
- The timeline draws rows in list order, so the two new rows sit at the bottom of it. Sorting the timeline by time would fix it for every lane.
- The rumor map has no way to read a card's text. A cursor and the memory text in a box would be the next step.
- The parents' fight over the door is only in the hour. At the knock, a parent who is in the house still doesn't open the locked door (as before).
- No early-knock row on the timeline (it would show "?" at 16:48 for everyone).
