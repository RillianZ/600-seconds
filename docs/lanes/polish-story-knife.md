# Lane: polish-story-knife (v0.20.1)

Branch `claude-polish-story-knife-v0.20.1`, from v0.20.0. Polish on the v0.20 story and knife work before the 600 Seconds release.
Story test: 107 of 107 memories (one new), ALL MEMORIES REACHABLE, same endings as before (the secret one still comes at LOOP 10).
Every line below is new and is a proposal for Noah.

## Where the code is

- New file `src/game/PolishStoryKnife.inc`, included by Story.cpp right after Knife.inc. Listed in the README layout. No build script changes.
- Story.h: one block at the end of the v0.20 systems part (fields and methods), and `CaughtIn` returns 0 in the oh-seven reality.
- Story.cpp, small marked lines (`// v0.20.1 story-knife`): Say / Bubble / Black run their text through `UnitLine`, the BeginLoop reset, the memory at the end of AllMemories, one hook each in TrunkScript, OnCaught, OnFound (2), StartCollection (2), Mom's knife line, ScreenTimeEnding.
- Knife.inc: BladeCheck marks a clean job this loop; KnifeInteract and KnifeChoose call the polish hooks first.
- main.cpp: AddBubble runs the visitor's barks through `UnitLine`; the watch throw (input, landing, drawing, HUD hint) and the `slip` scene, each a marked block.
- test/sim.cpp: one block before the clean key route, plus one line in Drain (a `slip` scene means he grabs you again).
- test/shots.inc: `trunkshift`, `cleanjob`, `screenbrakes`.

## 1. Oh-seven says 07

`UnitLine(who, text)` changes the number only in the oh-seven reality, and only where it's heard:
- The visitor, the radio voice ("Voice", "Finse"): "Zero-six" becomes "Zero-seven", "06" becomes "07".
- Dad and Mom: "06" becomes "07" (that's Dad's 16:10 radio line: "Unit 07 is stable."). Dad's own "Zero-six was before you" stays.
- Black screens and text with no speaker: only inside quotes (the TV host, "Subject 07, delivered", "Come on, 07", "Outstanding, 07").
- Never: your own lines, papers you read (SUBJECT 06, S06, UNIT 06 HANDLER), memories, "ANCHOR 06" (the watch, hinge B), "model 06".
- The hunt barks in main.cpp ("Zero-six. Come on out.", "Hey. 06.", "Knock knock, 06." ...) go through the same filter in AddBubble.

Calls for Noah:
- The visitor says **"Zero-seven"**, not "Oh-seven". Dad's "Oh-seven" is warm, his is clinical, so his word stays the same shape.
- Hinge C's console line and hinge A's last line ("Zero-six. Let's not drag it out.") change too, since they're spoken. The hinge pieces themselves (the memories, ANCHOR 06, week 612) don't change.
- The console's "unit 04 collected... unit 05 collected... unit 07, 16:50" skips 06. I left that: 06 is the one that got reissued.
- The black screens that say "Not Kiddo. Not sweetie. Not 06. A name." aren't quoted speech, so they stay 06.

## 2. The last beat of the two new realities

On the way to the bells (TrunkScript: every trunk, Express Delivery, a lost QTE). No new ending ids.
- The photos (`Fourth`):
  - "Under the blanket in the trunk, something soft. A red sweater. Too small for a man. Still a bit too big for you."
  - (if you saw the album) "The boy in the photos wore it. Then he grew out of it. Then he grew into the coat."
  - "It smells like snow, and like your house."
- Oh-seven (`NextUnit`):
  - "Through the gap behind the back seat: the dome light, and his clipboard. 06 has a line through it. Over it, in Mom's handwriting: 07."
  - "\"Seven,\" he says to nobody. \"Huh. They told me six.\""
  - (if you saw the labels or Mom's report) "Week one, Mom's report said. You lie there and wonder who gets your room for week two."

## 3. Hunt payoffs for the v0.20 memories (calls for Noah)

1. **Oh-seven: he doesn't know your spots.** He's hunting 07. The spots he caught you in before (caughtSpots) don't count that loop, and finding you doesn't teach him a new one. Found line, first time in a spot: "(he looks at the spot, then at you, a long time) Found you. ...You hide like somebody I used to know."
2. **The watch is a lure** (needs "Two short, one long" or "Drift"). Out of marbles, Q throws your watch instead, once a loop. It lands like a marble, beeps, and he always goes to look (it doesn't count toward his "that's three marbles"), and he waits there 4 seconds instead of 2.4. Bubble: "(two short, one long, from across the room) ...Drift?" HUD: "Q: your watch (it beeps)". You don't get it back that loop. I didn't do the muffling idea.
3. **Small hands: you slip one grab.** The first time he catches you in a loop, you don't fight. Lines: "(A hand like a vice closes on your collar.)" / You: "(Hitting doesn't work. Not on him. So I don't.)" / "(You go limp. Arms up, all of you at once, like a sack of flour. Your sweater comes off over your head.)" / Bubble, the visitor: "(he's holding a red sweater) ...Huh. I used to do that." He stands for 1.5 seconds, then the hunt goes on. The second grab holds. This one changes balance the most: once you have Small hands, every hunt has one free mistake. Easy to gate harder (only when he's not seen you yet, or only from loop N) if it's too kind. The kid's sprite still wears the sweater.
4. **You told Dad about the knock: he's in the door.** If you said "At ten to five the power dies..." to Dad that loop, and he isn't scared of you (fewer than three predictions), not Dark, not already on your side: "Dad: (he's already there, wrench in his hand. He fills the doorway and doesn't step aside.)" / "The visitor: (he looks at the wrench. Then past Dad, at you.) ...Somebody told you I was coming." / "Dad: (without turning round) Go to your room, buddy. Now." / You: "(He believed me. Not enough to run. Enough to stand there.)" The usual "Go with him, buddy" is gone, and the hunt starts like hinge D: he stands on the step 4 seconds counting rooms. I didn't do "he hides you".

## 4. Knife

a. **Screen Time with the brake line cut: the crash, off screen.** After "Under the well":
  - "Two bends later he brakes. Then he brakes harder." / "\"Huh,\" he says. Not to you." / "You don't look up. You're nearly at the boss." / "GAME OVER, says the Fliptendo. For once it's right." / "The screen stays lit in the snow for a long time. Nobody presses continue."
  - Narrator: "I cut his brakes. Then I played three games in a row and forgot. That's the kind of hour it was."
  - Card: "SCREEN TIME / He reached the last dungeon. The car reached the last bend." Same ending id, gives the Brake line memory too.

b. **DARK_REVEL (13): carve 06 into your door frame.** I picked the door frame over his radio: the radio is used by six endings (Monster, Program, After, Handlers, Last Toll, Same House), and cutting it would have meant changing all of them. The door frame touches no ending.
  - Your door, with the knife, at 13+, before the knock: "Your door. Mom's pencil lines up the frame, a date by every one." / "Carve 06 into the frame. Where she measures me." (red) / "Just the door." / "Leave it."
  - "(Mom's pencil lines up the frame. 4. 6. 9. 12. A date by every one, in her writing.)" / (oh-seven) You: "(Everybody calls me oh-seven today. I carve 06 anyway.)" / "(You put the point in above the last line and carve. 0. 6. Deep, so it won't paint over.)" / "(The paint flakes off round it. Under the paint there are more lines. Older. Scratched, not pencil.)" / "(They don't stop at twelve. They go on up the frame, past your head. The top one is a man's height.)" / "(Next to it, scratched in, crooked: 06.)" / You: "(Somebody grew up in this doorway. All the way up.)"
  - 2 dark points. Once a loop.
  - New memory **Under the paint** (The visitor): "I carved 06 into my door frame, where Mom measures me. Under the paint the knife found older lines. They went on up past my head, all the way to a man's height. Next to the top one, somebody had scratched 06 already. Crooked." Hint: "Mom's pencil lines on your door frame have been painted over more than once. You'd have to be very dark to dig."
  - Calls: it teaches that he grew up in this house (D4). It isn't in the rumor map (Links.h); it could link to "Twice round" or "Same house". Nobody reacts to the carving yet.

c. **Mom and a clean knife.** If you've done a clean job this loop, Doggo is in the ribbon, or you're by the tree (tape not cut) or the woodpile (wood not stacked), and you've done nothing dark this loop: no scolding, no mood hit.
  - Mom: "(she sees the knife. She doesn't shout.) Careful with that. What are you cutting?"
  - Doggo: You: "Doggo's all tangled up in the ribbon." / Mom: "(she looks at the blade, then at you) Away from him. Not towards. Go on."
  - Tree: You: "Just the tape on the big present. I won't tear the paper." / Mom: "(something crosses her face) ...Leave the presents, sweetie. They're for after."
  - Woodpile: You: "The twine on the firewood. Like Dad does." / Mom: "Like Dad does. Blade away from you, then."
  - After a job: You: "Just twine and stuff. I'm careful." / Mom: "(she watches the blade, not you) Point down. Like your father showed you."
  - A dark act this loop: the old "Why are you holding that? Put. It. Down." Tainted she never said either line, and still doesn't.

## What I checked

- Story test passes: 107 / 107, ALL MEMORIES REACHABLE, endings list the same as v0.20.0. Under the paint comes on the main story (knife, door, carve) before LOOP 10, with the trunk QTE held off so the secret ending still comes at LOOP 10.
- The new sim block checks: the filter on every kind of line (visitor, radio, Dad, you, papers, quotes, ANCHOR 06, normal reality), the real 16:10 radio line and the nobody-home line in oh-seven, no known spots and nothing learned, both trunk beats (with and without the album/labels, nothing in a normal loop), the watch (needs the memory, once, works past three marbles), the slip (once, then the grab holds, nothing without the memory), Dad in the door (and not without the prediction), Screen Time with brakes (card, both memories), Mom (scolds by default, worried after a job, at the tree, after the firewood, scolds again with a dark act), the door frame (nothing at 12, dark option at 13, "Just the door" still toggles the door).
- Linux desktop build, no new warnings. Screenshots in `docs/lanes/polish-story-knife/`: the radio saying 07, the oh-seven and photos trunk beats, the Screen Time crash, the watch hint and the watch landing in the hunt, the door frame choice and the line at the top, Mom worried.

## What I didn't do

- The watch can't be muffled, and you don't get it back. No line when he picks it up.
- Dad doesn't hide you; he only stands in the door.
- Nobody notices the carved door frame.
- No web or Windows build (as asked).
