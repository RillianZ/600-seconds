# Lane: knife (v0.20)

Branch `claude-lane-knife-v0.20`, on top of v0.19.0. The kitchen knife matters more: clean jobs first, darker jobs that open as the kid darkens, and Noah's request at the bottom of the dark. Story test: 90 memories, all reachable.

Code: almost everything is in the new file `src/game/Knife.inc`, included at the end of `Story.cpp` (so the story test still builds from one file). Story.cpp only has small hooks, each marked `// v0.20 knife`.

## What I built

### A. Clean jobs (no dark points)
- **The tape on the big present.** Tree, holding the knife: "Slit the tape on the big one. Don't tear the paper." Empty, and a packing slip. New memory **Return to stores** (The program). Also gives the old "Nothing in the boxes" memory. The paper stays whole, so it doesn't count as opening one early (no mood hit).
- **The twine on the firewood.** Woodpile with the knife: "Cut the twine and stack the firewood. (12 min)" instead of 20. Same choice id, so nothing else breaks.
- **Doggo in the ribbon (invented moment).** At 16:15, if you're in the living room and Doggo is with you (not told to stay), he goes under the tree and comes out wrapped in the ribbon off the big present. He stays stuck at the tree, with a red ribbon drawn on him. With the knife: "Cut the ribbon. Blade away from him." Without it: "Untangle him. (3 min)". Leave him and at 16:21 he yanks free and a bauble breaks. New timeline row at 16:15.
- **The frozen window (my pick, below).** Your window sticks in the cold. With the knife: "Run the knife round the frozen frame. Then climb out." No bang.
- **The torn fence out back.** The torn bit of the chain fence (6, -12) is now a thing you can use ("Torn fence"). It's laced shut with old baler twine. Cut it (2 min) and it's a door your size: crossing it goes from 0.35 to 0.8 of your speed. He still doesn't fit. New memory **A door my size** (The house). Twine is drawn in the hole until you cut it.
- Achievement **Blade Down**: three different clean jobs, across any loops.

### In the hunt
- **The knife block works during the hunt.** Took it already this loop: one press, no questions. Otherwise two quick confirmations ("Take it? (1/2)", "(2/2)").
- **Cut the fence wider during the hunt**: one press, instant (no time cost in the hunt, so it stays snappy).
- **My second hunt use: the frozen window.** In the hunt the window in your room costs an extra 1.2 seconds of banging with two thuds before you get out. With the knife you skip that. Why: it's the same kind of job as the fence (open a way out, quietly), it's a real winter trick, and it's the one exit the kid already knows is slow. Call for Noah: the bang could also be something he hears (main.cpp has no hearing hook for it yet).
- Hunt hint lines (one marked block in main.cpp): standing in the torn fence you see "Squeezing through. Slowly.", "Squeezing through. The knife would cut the twine (E).", or "Through the gap you cut. He won't fit."

### B. Dark jobs (each a DarkAct)
- **Phone cord** (dark 3+, 1 point). Needs the knife, before the call, and that you know about the call (phone_mom or phone_answer). Also offered on a phone you already unplugged. No call, no plugging it back in. Mom finds the cut ends at 16:22:40 if you're in the kitchen. At the door, after "It never rang", she says "Somebody cut the cord", and he answers "Somebody did." Being dark, it also rules out the Spared route that unplugging opens.
- **Brake line** (dark 8+, 3 points, 1 minute). His car while he's at the window (CarEmpty), the knife in your hand. Works with the engine running, or after you took his key (the car offers only this then). It pays off whenever his car is driven:
  - He drives you (the trunk ending, and Express Delivery): he goes off the road at the third bend. If the box is still on his belt, his heart stops and yours goes with it. Loop ends. New memory **Brake line** (The dark).
  - The Handlers: whoever drives (Dad, Mom, or Dad with Mom in the back) goes off at the second bend. Still The Handlers ending, new card text.
  - False Hope: Mom drives his car, the first steep bend. Still the False Hope ending, new card text, no leash memory that time.
  - Key in the snow: nobody drives. One line: he never finds out about the brakes.
  - Same House: you drive his car. One line, and the true ending carries on (the car knows the road).
- **The wire to the box** (needs dead_switch, the knife, and "Finish it", so tainted; 1 point). In "Are you sure?": "Cut the wire to the box on his belt." Works like taking the box off his belt (switchTaken), then "Do it" as before.
- The kill paths are unchanged.

### C. The mirror (Noah's request)
- Needs: max darkness (15) and tainted, the knife in your hand, the bathroom mirror, not during the hunt, not in the sister reality, and you have already seen the older reflection once (mirror_lag). Otherwise the mirror works as before.
- Two choices, and the quiet answer is always first, so a stray key press never picks it.
- It cuts to black straight away. Nothing is described. No sound effect, the music stops, no achievement, no "new memory" popup (the memory goes in quietly and only shows in the recap list). Then EndLoop: 16:00, the watch beeps, like every loop.
- The memory **No way out** (The dark) is hidden from the log until you have it, never counts toward the totals (HUD, log %, Total Recall, the secret ending), and the narrator's nudge never points at it. Nobody has to do this to finish the game.

### D. The knife block
Once you've done any clean job, or seen where one is (the tree, the woodpile, the fence or Doggo without the knife each give a hint line), the block asks twice instead of five times, and says what it's for. Tainted it was already two.

## Every new line (for Noah)

**Knife block, after you know a clean job**
- (The kitchen knife. For twine. For tape. For things, not people.)
- (Blade down. Walk, don't run. Like Dad showed me.)
- On taking it: (Point down, by my side. Careful.)

**Hints without the knife**
- Tree: (The tape on the big one is the clear kind. Something thin and sharp would slide right under it. The paper wouldn't even tear.)
- Woodpile: (Every bundle is tied with twine. Dad does it with the kitchen knife. One cut each. I use my teeth.)
- Fence: (The torn bit of the old fence. Somebody laced it shut with baler twine, years ago. I can just about squeeze through.) / (The kitchen knife would go through that twine like nothing.)
- Fence in the hunt: (Twine all through it. With the kitchen knife I'd be through in a second.)

**The present**
- Choice: Slit the tape on the big one. Don't tear the paper.
- (You turn the big one over and slide the blade under the clear tape. Slowly. The paper doesn't tear.)
- (Tissue paper, and nothing. Under the tissue paper, a packing slip.)
- FINSE SUPPLY. UNIT 06. 3 x GIFT, DECORATIVE. RETURN TO STORES AFTER COLLECTION.
- You: (Somebody orders my Christmas. And takes it back after.)
- (You fold it all back the way it was. Nobody could tell.)
- Memory "Return to stores": I slit the tape on the big present and looked in without tearing the paper. Empty. Under the tissue paper, a packing slip: FINSE SUPPLY. UNIT 06. 3 x GIFT, DECORATIVE. RETURN TO STORES AFTER COLLECTION. Hint: The tape on the big present is the clear kind. Something thin and sharp would slide right under it.

**The firewood**
- Choice: Cut the twine and stack the firewood. (12 min)
- Fade: Stacking firewood. One cut per bundle.
- (One cut per bundle, blade away from me. Twelve minutes instead of twenty. Dad's trick.)

**Doggo and the ribbon**
- Doggo: \*noses in under the tree. Comes out wearing the ribbon off the big present. Spins. It gets worse.\*
- Choice: Doggo, wrapped in ribbon. The more he spins, the worse it gets. (without the knife adds: A knife would do it in one go.)
- Cut the ribbon. Blade away from him. / Untangle him. (3 min) / Leave him.
- (You slide the blade under the ribbon, edge away from him, and cut once.)
- Doggo: \*sneezes. Shakes. Leans his whole weight on your legs like you saved his life.\*
- (Three minutes, one very patient dog, and a ribbon that's never going back on that present.)
- 16:21: Doggo: \*yanks free. A bauble goes. He shoots out from under the tree with a bow on one leg.\* Mom: What was THAT? ...Doggo. Oh, Doggo.
- Timeline 16:15: Doggo noses in under the tree and gets the ribbon round his legs.

**The window**
- Choice: Run the knife round the frozen frame. Then climb out.
- (Blade round the frozen frame, slow, all the way round. It gives without a bang.)
- In the hunt: (Blade round the frozen frame. It gives without a sound.)

**The fence**
- Choice: The torn bit of the old fence, laced shut with baler twine. / Cut the twine out. Make it a door my size. (2 min)
- (Three cuts through the old twine, and the loose ends fold back. A door my size.) / (Not his. He'd have to go round by the gate.)
- After: (A door my size. I checked twice. He won't fit.)
- In the hunt: (Three cuts through the old twine. A door my size now. Not his.) / (My size. Not his.)
- Memory "A door my size": Somebody laced the torn bit of the old fence out back shut with baler twine, a long time ago. With the kitchen knife it's a door my size. He still doesn't fit. Hint: The torn bit of the fence out back is tied shut with old twine. Mostly.

**The knife in the hunt**
- The knife block. He's in the house. Take it? (1/2) / Take it. / Leave it.
- (Blade down. Like Dad showed me. Even now.) tainted: (It fits my hand.)
- Take the knife? (2/2)
- (In my sleeve. Point down.) / (Back in my hand. Easier every time.) / (Already got it.)

**Phone cord**
- Choice: Cut the cord behind the bench. (unplugged already: Cut the cord. Then nobody can.)
- (Behind the bench, where nobody looks. Two cuts, so there's a piece missing.)
- (No call. No "confirm pickup". And nobody plugs that back in.)
- After: (Dead. Behind the bench the cord just stops. Nobody's plugging that back in.)
- Mom at 16:22:40: (she pulls the bench out. She finds the cord. She holds the two cut ends and looks at the kitchen door.)
- At the door: Mom: (very quietly) Somebody cut the cord. The visitor: (still looking at you) Somebody did.

**Brake line**
- Choice: Crawl under it. Cut the brake line. (engine already off: You: (The engine's off. The car is already getting cold.) / His car. Dark. Nobody in it.)
- Fade: Under his car
- (On your back in the snow under his car. It's warm under here. Something ticks.)
- (Dad showed you once, under the old car out back. The thin pipe that runs to the wheel. You cut it.)
- (It drips into the snow. The snow covers it.)
- You: (He'll be in a hurry. The road down off the vidda is all bends.)
- He drives you: He drives fast for a man with a kid in his trunk. The road down off the vidda is all bends. / The first bend, he brakes. The second, he brakes harder. Through the lid you hear him say one word. / The third bend, the car doesn't. / Then it's very quiet. Snow ticking on the lid. / The place behind your ear goes hot. Somewhere in front of you his heart has stopped, and the box on his belt doesn't wait for anybody. (box already off: Nobody comes. The storm does the rest.) / Narrator: I cut his brakes. I was in his trunk. I knew both of those things. I did it anyway.
- The Handlers: The road down off the vidda is all bends. Dad takes the first one slowly. He was always careful. / At the second bend Dad puts a foot on the brake, and nothing happens. / "Oh," Dad says. Just that. / (both) Mom finds your hand in the back seat. / Then the snow. / Narrator: I cut his brakes. They were the ones who drove. (one parent: I cut his brakes. Dad was the one who drove.) Card: THE HANDLERS. They finished his job. His car finished theirs.
- False Hope: At the first steep bend Mom brakes, and nothing happens. You know why before she does. / Dad finds your hand in the back seat. / Then the snow. / Narrator: I cut those brakes for him. We were the ones in his car. Card: FALSE HOPE. They ran together, in his car. You had already seen to his car.
- Key in the snow: He never finds out about the brakes. You don't tell him.
- Same House: Halfway down you remember the brake line. The car takes every bend anyway, slow and careful, like somebody else is driving.
- Next loop: His car wasn't at the gate yet. Its brakes still worked. I lay there knowing that. / I could still feel the snow under his car, through my sweater.
- Memory "Brake line": I cut the brake line under his car while he was at the window. Whoever drove it after that didn't get far. Not him. Not them. Not me in the back. Hint: His car stands empty while he's at the window. Underneath it, something thin goes to every wheel.

**The wire to the box**
- Choice: Cut the wire to the box on his belt.
- (The black box on his belt. A wire runs from it up under his shirt. You don't unclip anything. You cut the wire.)
- (The green light doesn't go red. It just goes out.)
- The visitor: (very quietly) ...Huh. Nobody ever did it like that.

**The mirror (C)**
- Choice: The one in the mirror is older. He isn't smiling. The knife is still in your hand. / Look away. / End it.
- Second: Are you sure? / No. / Yes.
- Black screen: Nothing. For no time at all.
- Narrator: It wasn't a way out. There isn't one in here. The hour doesn't let go. It just starts again, and I'm still in it.
- Next loop narrator: 16:00. The watch. The bed. Ribbe downstairs. I lay there a long time. Then Mom called my name, and I answered.
- (Then, as on every tainted loop: Mom: (from the kitchen) ...Is that you? You're up. Good. Good.)
- Memory "No way out" (The dark): I tried to leave the hour the other way. There is no other way. It doesn't let go. It starts again at 16:00, and I'm still the one in it.

**Achievement**: Blade Down. "Three jobs for the kitchen knife. None of them a person."

## Calls for Noah to react to
- Chain-link wire can't be cut with a kitchen knife, so the torn gap is laced with old baler twine, and that's what you cut.
- Doggo's moment is the ribbon under the tree at 16:15, not a leash. Doggo follows you, so it only happens if you're in the living room then.
- The second hunt use is the frozen window (reasons above).
- Gates: phone cord 3, brake line 8, the wire needs tainted (it lives under "Finish it"). Nothing new at 13.
- After any clean job (or a hint), the knife block asks twice instead of five times. That makes the five-step "are you sure" rarer for players who find the clean jobs early.
- Mom still says "Why are you holding that? Put. It. Down." if she sees the knife, clean job or not. Locking her in the bathroom still counts as dark if you took the knife that loop.
- The mirror: no sound effect, the quiet answer first in both choices, the memory hidden and never counted. Noah might want a content line about it in the README and on itch.io (today they mention off screen violence, implied child abuse and the death of a dog). I didn't touch the README text.
- Screen Time is the one drive I didn't change. Cut brakes and then the Screen Time ending, and he still drives you off fine.
- The phone cord rules out the Spared route that unplugging opens, because it's dark.

## What I checked
- Story test passes: 90 / 90 memories, ALL MEMORIES REACHABLE. New loops 20a to 20e on the main story, plus fresh stories for the rules: the block asking twice, no dark options without darkness, the hunt block (two quick confirmations, then one press), the fence and window in the hunt, cord (no ring, no plug back), key in your pocket plus brakes (he gets the key, then the crash), key in the snow (no crash), False Hope crash, Handlers crash, the wire then End of the Program, the mirror (needs max darkness and the knife, quiet answer first in both choices, no Say, Clue or Achievement beats after, EndLoop, the next loop's line), and FullMemory without the quiet memory.
- Linux build with the screenshot runner, no new warnings. Screenshots in `docs/lanes/knife/`:
  - clean: `knife-ribbon.png`, `knife-ribbon-choice.png`, `knife-tree-choice.png`, `knife-tape.png`
  - hunt: `knife-hunt-block.png`, `knife-gap-squeeze.png`, `knife-hunt-gap.png`
  - brake line: `knife-brake-choice.png`
  - the mirror: `knife-mirror-black.png` (the black screen, the only image for it)

## What I didn't do
- No screenshot of the crash scenes or the window cut. Both are black screens or a one-line bubble.
- The `hold` command in the screenshot runner didn't move the kid in the hunt for me, so the shots use `at` to put him in the fence gap.
- No web or Windows build, no zip (main session).

## Files touched
- New: `src/game/Knife.inc`, `docs/lanes/knife.md`, `docs/lanes/knife/*.png`
- `src/game/Story.h`: one block at the end of the per-loop section, plus FullMemory.
- `src/game/Story.cpp`: memories, achievement and timeline blocks at the end of their lists. Hooks in Interact, Choose, CheckEvents and BeginLoop, LoopOpener and Nudge. KnifeCheck/k_take. One option line each at the tree, woodpile, car, phone, window and "Are you sure?". Payoff lines in CollectionScript, ExpressDelivery, NoKeyEnding, HandlersEnding, EscapeEnding and SameHouse. Phone lines at 16:22:40 and at the door. The include at the end.
- `src/game/World.h`: one object, the torn fence.
- `src/main.cpp`: memory totals (HUD, log %, log rows), fence speed, Doggo's spot while tangled, one hunt hint block.
- `src/art/Sprites.inc`: the twine in the fence, the ribbon on Doggo.
- `test/sim.cpp`: one block before the clean key route.
- `README.md`: the layout line.
