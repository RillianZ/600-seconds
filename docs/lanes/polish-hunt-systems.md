# Lane: polish hunt-systems (v0.20.1)

Branch `claude-polish-hunt-systems-v0.20.1`, on top of v0.20.0. Six polish tasks for the hunt and the v0.20 systems.
Story test: passes, ALL MEMORIES REACHABLE, 13 endings. No new memories, no new achievements, no new timeline rows.
Screenshots (Linux build, 960x540) in `docs/lanes/polish-hunt-systems/`.

## What I built

### 1. He hears your window
- Banging your stuck window open in the hunt (the first climb out of your room each loop) now carries like a slammed door: 14 tiles, through walls, storm or not (the bang is inside the house). He stops what he's doing and goes to look at your window. A ring shows where the sound came from.
- New `OnHeard("window")`: one bubble per loop, "(a bang on a window frame, somewhere) Still sticks, that one." It also gives Running is loud, like the other noises.
- The knife's quiet way (v0.20 knife) sets `windowBanged` before the climb starts, so there's no bang and nothing extra to hear. The climb itself is still loud, as it always was.
- Climbing through any window: checked, it is heard. It uses the running-feet rule (10 tiles, 5 if you're outside in the storm). I climbed through the bathroom window next to him in a test build and he went to look. No change needed.
- Where: one block in main.cpp's hunt hearing, right before the running-feet check, marked `// v0.20.1 hunt-systems`.

### 2. They sit on the couch for Kaptein Fjord
- New `src/art/Couch.inc`. The couch is turned round so it faces the TV (its back is toward you, the red blanket over the backrest, the pillow in the corner). Mom and Dad sit on it from the moment they get to their TV spots (16:14:30) until the schedule moves them (16:20). You see the backs of their heads and shoulders over the backrest, watching. Then they stand up and walk off as before.
- They sit only once they've arrived (no sliding into a seat mid-walk), never in the hunt, never on the floor.
- Hooks: `DrawObj` draws the couch with `CouchDraw` (the old couch art moved to Couch.inc), the y-sort skips Mom and Dad while `CouchSits` (they're drawn with the couch, between the seat and the backrest). Two forward declarations and one include in main.cpp.

### 3. The timeline in time order
- `DrawTimeline` sorts a copy of `AllTimeline()` by minute and second (stable, so rows at the same time keep their list order). `AllTimeline()` itself is unchanged; `KnowsEvent` and the sim still read it in list order.
- The page had already run off the bottom with 20 rows (12.2 units each). Rows now get closer together when there are more of them, and the text shrinks a little if it has to, so every row fits. 16:15 (ribbon, Kaptein Fjord) and 16:40:40 (lefse) are in their place now.

### 4. Open a card on the rumor map
- New `src/art/RumorCard.inc`. Keyboard: **E, ENTER or SPACE** opens a card (the found card nearest the top of what you're looking at). While a card is open the **arrows / WASD** move to the nearest found card in that direction (the page scrolls with you), and **E / ENTER / SPACE / ESC** close it. With no card open everything is as before: LEFT/RIGHT page, UP/DOWN pan, J closes.
  - Why: LEFT/RIGHT turn pages and UP/DOWN pan on every page of the log, so the arrows only change meaning while a card is open. E is the game's "use" key everywhere else, so "E to read a card" needs no explaining; ENTER and SPACE do the same for people who reach for those. ESC closes the card before it closes the log.
- Mouse: click a card to open it, click anywhere else to close it.
- Phone: tap a card to open it. Tap anywhere else closes the card; with no card open, a tap closes the log as before. The d-pad moves between cards while one is open. On a phone the box sits right under (or over) the card, so the d-pad and buttons don't cover it.
- The box shows the memory's title, its topic and its text in the log's handwriting (the kid's pencil early, the narrator later, cold on the dark path). The open card gets a white edge and its lines turn gold.
- Only found cards open. "?" cards can't be selected.

### 5. The locked front door in the Dark stance
- At the knock with the front door locked, if they're in the Dark stance and somebody is still in the house, that parent opens it for him. Mom if she can, Dad if Mom is shut away (or on your side, or broken).
- The scene: "(Nobody opens. The handle turns. Locked.)" / "(Then Mom's slippers on the floorboards. She doesn't hurry. She turns the lock herself.)" / Mom: "(opening the door) Sorry. He locked it. Come in. Please. Just take him." / You: "(I locked everything. She just opened it.)" Then the knock goes on like an open door in the Dark stance ("Evening.", "Has he been good?", "Just take him.", the hunt with them helping him).
- Dad's version: "(Then Dad's boots on the floorboards. He turns the lock himself. He doesn't look at you.)" / Dad: "(opening the door) Sorry about the door. Come in." / You: "(I locked everything. He just opened it.)"
- The kid's line is the Fortress memory's own first sentence turned round ("I locked everything. He just came through the window."). No new memory, and no Fortress memory or achievement from this scene: he wasn't locked out. You still learn those the old way.
- Since he comes in the front, traps on the front step trigger on him like any open door.
- Normal and Befriended: as before (nobody opens, glass, the living room window). Nobody Home (both shut away): as before. The hinges: Mom in the bathroom means Dad opens, then hinge A plays as before; Dad barred in the shed means Mom opens, then hinge B. Down in the basement (hinge C): as before, you hear glass.
- Where: `Story::DarkOpener()` and `Story::DarkOpensDoor()` (after `NobodyToOpen`), one `letIn` line in the knock, per-loop `frontOpened`.

### 6. A Nod Across the Yard in the early loops
- New `Story::NodAt()`: the nod is 20 seconds into his look at the window (`visitorTime - VISIT_LOOK + 20`) and at least 10 seconds after Dad gets to the flagpole (16:43:30). It only happens while he's still at the window and Dad is still at the pole.
- A normal day: 16:44:40, right after the box, exactly as before (same lines, same place).
- The early knock (16:48): the nod is at 16:43:40, on its own: "(Dad stops at the flagpole. He looks up, across the yard, at the big man by the living room window. Dad nods. The man nods back.)" / "(They know each other.)". The box at 16:44:40 still happens after it, without the nod (he's gone back to his car by then).
- Dad's flagpole stays at 16:44 every loop. "Forty-four" is his code, so I moved the nod, not Dad.

## Calls for Noah to react to
- The window bubble: "(a bang on a window frame, somewhere) Still sticks, that one." He knows the window (D4).
- The bang carries 14 tiles, as far as a slammed door.
- The couch now faces the TV. You see its back and the backs of their heads. Before, it faced you and the TV was behind it.
- The Dark-stance door lines (Mom and Dad versions above). "Sorry. He locked it." is Mom apologising to him, not to you.
- Mom opens it if she can, Dad only if she can't. A Broken parent doesn't get up to open it.
- No Fortress memory or achievement when they open it for him.
- Rumor map keys: E / ENTER / SPACE to open and close a card, arrows to move while it's open, ESC closes the card first.
- In the early loops the nod comes a minute before Dad checks the box.

## Shared spots I touched (all marked `// v0.20.1 hunt-systems`)
- Story.h: one block at the end of the per-loop section (`frontOpened`, `DarkOpener`, `DarkOpensDoor`, `NodAt`).
- Story.cpp: BeginLoop reset (one line), `OnHeard("window")`, the knock (`letIn`), the nod block before the flag44 block and its condition inside it, the new functions after `NobodyToOpen`, the Dad/Mom door lines.
- main.cpp: the window bang block in the hunt hearing, `DrawTimeline` (sorted copy, row height, font size), the rumor map (`#include "art/RumorCard.inc"`, card hits, open card highlight and lines, the box, help text), the journal key branch (`journalOpen && RumorKeys()`), the phone tap line in the log, the couch forward declarations and `#include "art/Couch.inc"`.
- art/Sprites.inc: the couch branch calls `CouchDraw` (its old art moved to Couch.inc, turned round).
- art/WorldDraw.inc: two lines, Mom and Dad skip their normal drawing while they sit.
- test/sim.cpp: one block before "Clean key route, the findable one".
- test/shots.inc: `see ID`, `climbwin [WIN]`, `rtap X Y`.
- README: two layout lines.

## What I checked
- Story test after every step: ALL MEMORIES REACHABLE. New checks: the window bubble once per loop and Running is loud; the knife opens the window with a climb and no stuck window; the knock with the door locked in Dark (Mom opens, no Fortress, he stands inside the door), Normal (window, Fortress), Befriended (window, Fortress), Dark with Mom in the bathroom (Dad opens, hunt starts at the bathroom), Dark with both shut away (Nobody Home, he bangs your window); an early-knock loop where you stand in the yard: the nod at 16:43:40, then the box; a normal day still nods at 16:44:40.
- Screenshots: Dad sitting while Mom walks over (16:15), both sitting (16:16), the empty couch after (16:20), the sorted timeline with the new rows known, a rumor card open at the top and one low on the page (box moves to the top), a phone tap on a card, the Dark-stance door (Mom unlocks, opens, the kid's line), the window bang heard in the hunt (the bubble and the ring).
- In a test build: climbing through the bathroom window next to him in the hunt is heard (running-feet rule). He doesn't hear anything during his grace period at the start of the hunt, the same as running.

## Not done / left open
- While they sit, they always face the TV, also when you talk to them. They turn to you only once they stand up.
- The phone tap path was checked by calling the tap handler from the screenshot script (the real touch input only exists in the web build). Worth a tap on a phone after the merge.
- I didn't build the web or Windows versions.
