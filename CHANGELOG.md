# Changelog

Every version of the 600 Seconds prototype, newest first. Spoilers everywhere.

## What's in v0.15.2
Same branch, `claude-batch-v0.15-outside`. Noah's playtest of the hunt: too much sitting and waiting, then he kills your parents. More chase now.

- **The hunt is longer.** 16:50 to 17:00 now takes about five real minutes instead of two. The debug panel (O) has "Hunt clock": real time (ten minutes), half (the new default) or the old speed.
- **Your prints stay.** Two minutes in the snow, even in the storm. He finds the trail and follows it to the newest print. If you hid at the end of it, he walks straight there.
- **A tiny sixth sense.** About every 20 to 25 seconds without a sighting he gets a hunch: "I used to hide over... here. Didn't I?" He walks to the area you are in and searches that room. He doesn't know the spot.
- **More hunt before he starts on them, and more time between.** The warning comes at 16:57:30, then 16:58:40, then 16:59:40. He says "Come out" instead of counting minutes.
- **He has a taser.** Kite him (lose him out of a chase, or go out a window in front of him) twice and the next time he sees you within eight tiles he stops, aims and fires. After one juke, standing where he can see you but can't get to you (across a fence, out a window, the far side of the stream) does it too. Break his line of sight while he aims and he misses ("Tch.").
- **He gets more psychic toward 17:00.** The hunches come more often (every 7 to 9 seconds at the end) and land closer to you. In the last minute he walks to your exact area and checks every spot there.
- **New ending: The Empty House.** Bolt Mom in the basement, bar Dad in the shed, send Doggo out the front door, then last the whole hunt. At the bells he stops, reports in, sits down in Dad's chair and talks to the empty room. Achievement Nobody Left.
- New memories: He doesn't run any more, Nobody left to hurt. The story test now has 73 memories and 9 endings.

## What's in v0.15.1
Same branch, `claude-batch-v0.15-outside`. A fix from Noah's playtest.

- In the first loop nothing bad is supposed to happen, but walking out of the gate still ended it as running away. Now in loop 1 you can't leave the yard: the kid turns around, steps back and says it's way too cold and Zulda is waiting inside. Three lines, in turn. Out the gate and through the gap in the back fence both.

## What's in v0.15.0
Built on the branch `claude-batch-v0.15-outside`, on top of v0.14.0 (`claude-batch-v0.14-hinges-de`). Not merged. Expansion 5: the outside. The story test now has 71 memories and the same 8 endings.

**The backyard**
- Behind the house there's a backyard now, about twice the size of the house. The map grew north into negative rows, so nothing that was already there moved.
- Two lines of bushes with gaps. Pushing through a bush is slow. He gets slowed too, but less.
- An old chain fence across the middle, with an open gate at one end and a torn gap at the other. You fit through the gap, slowly. He doesn't.
- A frozen stream along the far end. Past it the back fence is down in one place. Out there is the vidda, and the same running away ending as the gate.

**Windows**
- Every window in the house works both ways: your room, the bathroom, Mom and Dad's room, the kitchen and both living room windows.
- Climbing takes about two and a half seconds. Your own window sticks in the cold, so the first time each loop you have to bang it open first.
- In the hunt it's loud. He hears it like running, and if he gets to you before you're through, he pulls you back.
- Doggo can't follow you out. He whines at the glass.
- Climbing out while grounded is neutral. No mood, no dark.

**Cold**
- Without the jacket you don't make it across the yard in the hunt. About eight seconds in the open. The shed and Doggo's house are still in reach from the front door.
- With the jacket the cold never gets you. You just light up in his torch.
- Hiding out of the wind is slower: the woodpile, the bins and Doggo's house, and out back the spruce and the old car.
- A WARM bar shows how long you have.

**Dark**
- From 16:45 it gets darker outside, step by step. By 16:53 you only see a small circle around you. Dad's flashlight is a real beam out there.
- The windows at the back of the house throw light on the snow while the power is on.

**His torch**
- Outside, after the power dies, he has a big torch. Long, wide, and it shines straight through bushes and the chain fence.
- He spots your red sweater from about 15 tiles away. With the jacket on, the reflective stripes make it 20.

**New hiding spots**: the snow beast, the old car and the big spruce.

**Six things out back**
- An old car on blocks, no wheels, snowed in to the door handles. A child seat in the back with the straps cut.
- Small boot prints that start at the swing, walk north and stop. The snow never fills them in.
- A little wooden cross with 05 carved into it.
- A satellite dish whose cable goes straight down into the snow.
- A swing set built for somebody smaller.
- A hole cut in the ice, with a rope going down.
- Find all six for an achievement.

**Five nods**
- A huge ride-on toy snow beast, grey-white, three horns, button eyes, a zip down the belly. You can climb inside. It's warm in there.
- The flagpole. You can lick it.
- A weathervane on the shed roof shaped like a fat rodent. Look up at it late in the hour.
- A board behind the shed with the alphabet painted on it and a bulb over each letter. On a dark night it spells something.
- A husky at the far fence that Doggo growls at. Walk up to it and there's nobody there.

**New memories (4)**: The other way out, Somebody smaller, Zero-five, Lit up.
**New achievements (7)**: Out the Window, Smells Worse Inside, Triple Dare, Same Day Again, Spelled Out, Not a Dog, Out Back.

**Debug**
- `2` hops you out back, next to the snow beast.
- `test/shots.inc`: a small script runner for screenshots on a virtual screen. Only built with `-DSHOTS`.

## What's in v0.14.0
Built on the branch `claude-batch-v0.14-hinges-de`, on top of v0.13.0 (`claude-batch-v0.13-hinges`). Not merged. Expansion 1 of 4 is now complete: hinges D and E. All five pieces can be found. The true ending that uses them is not built yet. The story test now has 67 memories and the same 8 endings.

**Hinge D: open the door yourself**
- Once you know he comes at 16:50, and he has sat back down in his car (from 16:47), the front door offers "Open it and wait for him in the doorway."
- There is no knock. He walks up the path and stops on the step. Nobody has ever opened the door for him, and for a minute he is off his script. What he says on the step is the fourth piece.
- Traps on the step don't go off. He steps over them.
- He doesn't grab you. The hunt starts with him on the step, counting rooms, and you get four seconds.

**Hinge E: under a blanket in his car**
- While he is at the window, his car has a new option (once you have looked in it or its trunk before): get in the back and hide on the floor.
- He gets back in and sits there with you for two minutes. The radio talks to him. That is the fifth piece.
- When he gets out you choose. Slip out, and you are back in the yard a minute before the knock. Or stay, and hear the rest of the hour from the car. It still ends at the bells.

**The pieces**
- Two new memories: After, always after, and The new collector.
- New achievements: Open House, Stowaway, Five of Five (with a narrator line).

**Debug**
- `H` now cycles through all five hinges. D puts you at the front door at 16:47. E puts you at his car at 16:44.

## What's in v0.13.0
Built on the branch `claude-batch-v0.13-hinges`, on top of v0.12.0 (`claude-batch-v0.12-hunt`). Not merged. Expansion 1 of 4: the first three hinges. Each hinge is a door you shut on somebody before he comes. He has to deal with them first, and you get to listen. The story test now has 65 memories and the same 8 endings.

**Hinge A: Mom in the bathroom**
- Every loop at 16:46 Mom goes to the bathroom for two and a half minutes.
- With the master key you can lock her in. When she's fine with you it's a trick, nothing dark. If you have had the knife this loop it's a dark act. If she's already scared of you it's darker.
- At the knock he goes to her door before he starts on yours. What she tells him through it is the first piece.
- He starts the hunt from the bathroom door, after a longer pause. If he can't find you, it's the bathroom door he kicks in.

**Hinge B: Dad in the shed**
- Bar Dad in the shed (the plank, as before). At the knock he asks where the father is and walks out to the shed with his torch. He leaves the plank where it is.
- Dad makes him read the back of his watch. That's the second piece.
- He starts the hunt out by the shed, so you get a head start in the house.

**Hinge C: in the basement at the knock**
- If you are downstairs when the power dies, you stay downstairs. One small screen keeps running on a battery and the radio channel is open. What Finse tells him is the third piece.
- The stairs work during the hunt, both ways. He takes them too, once he gets round to the basement (only if its door is unlocked).
- New hiding spot down there: the storage crates.

**The pieces**
- Three new memories: Not like with you, Crooked letters, Same house, next cycle. Together they say he was you once, and you will be him.
- The true ending needs a piece from every hinge, so it isn't built yet. Two more hinges to go.
- New achievements: Occupied, Three of Five.

**Doors**
- Doors now say which side you end up on. With the master key you can also step through and close or lock the door behind you. Closing a door without the key shows where you are standing. The front door says if you are about to lock yourself out.

**Debug**
- `H` sets up the next hinge (A, B, C in turn).

## What's in v0.12.0
Built on the branch `claude-batch-v0.12-hunt`, on top of v0.11.0 (`fable-visuals-20261112`). Not merged. The first of four expansions: the hunt. The story test now has 62 memories and the same 8 endings.

**He learns**
- He remembers every hiding spot he has pulled you out of, across loops and saves. Next time he goes to that room first and checks that spot first, with no looking around.
- "You always pick that one" is no longer said the first time. He says it the second time. After that he says he doesn't know how he knows. He just walks there.

**He hears**
- Running is loud. He hears it through walls, from about ten tiles away (less out in the storm). He goes to the sound and turns that room inside out.
- Walking he only hears when he is close. He turns around.
- In a hiding spot he hears you breathe when he stands right outside. Hold Shift (RUN on a phone) to hold your breath. The AIR bar runs out in about six seconds, and then you gasp.

**He tracks**
- Snow on your boots. Come in from the yard and you leave wet prints on the floor for about fifteen seconds of walking. They dry in about 45. He follows them, all the way to your hiding spot.

**You can**
- Take a handful of marbles from the jar in your room (once you know he comes at 16:50). Q throws one the way you face. He goes to where it lands. Twice. The third time he just smiles.
- Hide in five new places: the pantry, behind the Christmas tree, the woodpile, Doggo's house and the trash bins. The yard ones are cold, but slower to freeze in than standing out in the open.
- Sprint for about three and a half seconds. Then your legs give out and you walk until they come back (the LEGS bar).

**You see less**
- During the hunt you only see him when you could really see him. Otherwise you see where his steps land, as red rings, and your own noise as yellow rings.

**New memories (6)**: He remembers where he caught me, Running is loud, Hold my breath, Snow on my boots, He looks where it lands, Hiding doesn't end it (make it to 17:00 without being found).
**New achievements (2)**: Creature of Habit, Lost Your Marbles.

**Under the hood**
- The save has one more field (the spots he found you in). Older saves load fine.
- The debug line shows the hunt state while the hunt runs.

## What's in v0.11.0
Built on the branch `fable-visuals-20261112`, on top of the v0.10.0 audit branch. Not merged. A visual overhaul. No story changes, the story test is the same 56 memories and 8 endings.

The prototype look is still the reference: a top-down house drawn in code, no image files. Everything is just drawn with a lot more care.

**The world**
- **Painted ground.** Floors, walls and snow are now painted once, pixel by pixel, when the game starts (`src/art/Bake.h`). Pine planks with knots and nails, a checkered kitchen, small bathroom tiles, carpet, cracked concrete in the basement.
- **Every room has its own wall.** Pine panels in the living room, stars in your room, white tiles in the kitchen, wainscot in the hallway.
- **Snow with shape.** Drifts, wind ripples on the plateau, ploughed banks along the road, snow up against the south wall, icicles under the eave.
- **Rugs.** A red wool rug under the couch, a runner in the hallway, a town play mat in your room, a door mat.
- **The plateau goes on.** South of the gate there used to be a black void. Now the road and the snow continue.

**Roofs**
- The house and the shed have roofs with snow on them. Step inside and the roof lifts away. Step out and it is back.
- A brick chimney over the wood stove, with smoke. The storm tears the smoke sideways.
- A string of coloured bulbs along the eave, and a porch lamp by the front door. They die with the power. They go red if you have gone dark.
- From outside you no longer see into the house, only the lit windows. If you want the old view: Pause, Roofs, Off.

**Light**
- A new light pass. Soft round lights, warm pools from the windows on the snow, headlight beams up the road, and a glow on everything that shines (tree bulbs, the fire, the TV, the screens in the basement).
- The night is a deeper blue. Indoors is warmer.
- Snow falls in three depths and is drawn after the light, so you can see it at night. The storm adds white air.
- A smooth vignette.

**Things and people**
- Every object is redrawn with an outline, a lit edge and a shaded edge. The tree has branches, a garland, 11 bulbs and a star. The flag waves properly.
- People have shading, faces and clothes: your Christmas jumper, a parka and a red hat when you take the coat, Mom's apron, Dad's knitted band. The visitor has a long coat and a hat. You never quite see his eyes.
- Doggo has a snout, a collar and a tongue.
- Breath in the cold. Deeper footprints.
- The car has headlights, tail lights and snow on the roof.

**Screens**
- **A title picture.** The house at 16:49, the car at the gate, and him halfway to the door.
- **Faces in the dialogue box.** Whoever talks stands on the top edge of the box.
- Panels have a soft shadow.

**Under the hood**
- New files: `src/art/Bake.h`, `src/art/Sprites.inc`, `src/art/WorldDraw.inc`. `main.cpp` lost about 800 lines of drawing code to them.
- A debug jump could leave the kid in the basement while the game thought he was upstairs. The camera now decides which floor to draw.

## What's in v0.10.0
Built on the branch `fable-audit-20261112` after a full audit. Not merged. The audit and the story calls Claude made are in [docs/AUDIT-v0.10.md](docs/AUDIT-v0.10.md).

**New things to find (56 memories now, was 45)**
- **The tiny watch.** The "old watch" in Dad's cabinet was never used. Now you can take it. It is yours, from when you were a baby: ANCHOR 06. Show it to Mom, to Dad and to the man at the gate.
- **The thin folder.** The filing cabinet has a second folder, ORIGIN PROTOCOL. It says in plain words why the hour repeats. A last page, Appendix C, only shows if you know about the box on his belt.
- **His car.** While he stands at the window his car is empty at the gate. Look in through the window, open the trunk, or climb in yourself (Express Delivery).
- **Doggo wants out.** He whines at the front door at 16:35. Open it and he is far away when the banging starts.
- **Pull the phone plug.** If you know what the 16:22 call is, you can stop it. No call, no confirmation, and he leaves Mom and Dad alive. If you behave.
- **Hum her song.** After the tape you can hum the carol back to Mom. At the end she says your name.
- **Ask him.** "We do this every time." You can ask the man what he meant. He doesn't know he said it.
- **Dad at the flagpole.** At 16:44 Dad walks out and checks the tiny box under the flag. Watch him do it and you can take the key. The clean route to the basement is now something you can find by paying attention.
- **A new topic in the log:** The program.

**Endings**
- **One Good Hour.** A new ending for a kid who never went dark. False Hope now teaches you the leash (a memory), and that unlocks staying instead of running.
- Monster and End of the Program gain a few lines if you have read Appendix C.

**Reality**
- **Somebody is in the bathroom.** A fourth reality shift. Achievement: Table for Four.

**The log**
- **Timeline.** A new first page: the hour from 16:00 to 17:00. Things show up once you have watched them happen.
- **Awards.** A page with all 36 achievements and a nudge for each one you don't have.
- A new memory opens the log on the right page.

**The narrator**
- He now mentions how the last loop ended, and has twice as many opening lines.
- After three loops with nothing new he drops a hint.

**Talking**
- Everything a memory lets you ask Mom or Dad sits behind "Ask about something I remember". The lists had grown taller than the screen.
- 13 new questions and answers for Mom and Dad. Any proof now works on them: the phone call, the files, his clipboard or the scar.
- The radio follows the hour. The TV has more wrong episodes. Several objects have new lines later in the game.

**Sound (not heard by a person yet)**
- Wind outside and through the walls, the storm, the hum of the basement, the stove.
- Footsteps on planks, tiles, snow and gravel. His can be heard through walls, duller and from the side he is on.
- Mom hums in the kitchen. It is the Act 1 tune, slow. On the tape it is the only sound.

**Fixes**
- Answering the phone with Mom locked away did nothing at all. Now somebody is on the line.
- The basement was drawn under the yard when you stood at the gate.
- His car is solid, so walking up to it no longer sends you running onto the vidda.
- Debug key 3 hops to the gate.

## What's in v0.9.3
- **Debug text on the title** moved to the top left, so it no longer covers the content note.
- **Debug key 4** takes you to the basement with the tape in your hand.

## What's in v0.9.2
- **He has a look first.** The car comes at 16:43 now. He walks up to the living room window and looks in, goes back and sits in the car, then gets out with something under his coat and walks to the door.
- **Footprints are his.** The big prints are the real tracks he leaves on the way to the window, so they only show up after he arrives. The storm takes them at 16:47.
- **A face at the window.** Be in the living room while he looks in. In loop 1 it's Julenissen, early.
- **Talking to him brings it closer.** Walk up to him before he goes back to the car and he checks his watch and goes straight there. The knock comes sooner.

## What's in v0.9.1
- **Fullscreen in the exe.** F now works there too, and the picture scales when you resize or maximize the window.

## What's in v0.9
- **Tab skips what you've read.** Black screens and narrator lines too, so the collection doesn't cost you a minute every loop. New lines still have to be read.
- **Doggo gets a warning.** The first time he jumps in front of you the loop just goes on, and you learn to tell him to stay. Good Boy only ends the run after that.
- **Early Storm.** A new reality shift: the storm comes at 16:37 and the power dies at 16:40, ten minutes before he does. Achievement: Weather Report.
- **Pause menu.** P or Esc: music, sound, text speed and calmer flashes. Settings are saved.
- **After the tape she isn't Mom any more.** The dialogue box says Handler two.
- **Silence.** No music under the tape, and none while he does what he came to do.
- The tape is week 312 now. Week 3 would have made him a baby.
- The repeated line glitch stays out of serious scenes.
- Mom walks to the phone in time when it rings early.
- Doggo only barks on lines where he actually barks.
- New content note on the title screen.
- `package_play.sh` makes a zip with a Windows exe and a one file browser version.

## What's in v0.8
- **It's Christmas Eve.** The hour now happens on 24 December. Snow on the yard, the vidda, the roofs and the fence. Blue dusk at 16:00, night by about 16:25, warm light from the windows until the power goes.
- **The house is ready for Christmas.** A tree with lights that die as things go wrong, advent candles in the windows, a wreath, an advent calendar with PICKUP behind door 24, and a little nisse who turns to face the wall.
- **Presents instead of 100 kroner.** All five chores earn one present before dinner, and that's where Zulda is. Read the tags, shake them, or open one early and find out what's in the others (new memory).
- **Loop 1 is the happiest day of his year.** The family is waiting for Julenissen at ten to five. When he knocks, the kid runs to the door. No chase in loop 1.
- **"Has he been good?"** The visitor asks, and Mom answers based on how the day went.
- **The snow stops you.** Run for it and you walk in a circle back to your own footprints. Tired Out, Monster and False Hope changed to match. The road closes behind him at 16:50.
- **The snow gets you.** Stay out in the storm during the chase and you freeze, faster with no jacket. The jacket is on the coat rack (new memory, achievement Snow Angel).
- **Footprints.** You, Doggo and the visitor leave tracks. Outside he can follow yours, and the storm fills them in. Big prints along the house at 16:28 are a clue (new memory).
- **Bells.** Christmas is rung in at 17:00, so the kid never reaches it.
- Ribbe replaces fiskesuppe. It's always ribbe now.
- 3 new memories (45 in total).

## What's in v0.7.2
- **The watch is its own thing now.** It's an Origin Model 06: new name on the face, an origin mark, a colder case and an ice blue screen. The kid calls it his watch.

## What's in v0.7.1
- **The flag is Norwegian now.** It was red with a white cross, which is Denmark. It got its blue cross.

## What's in v0.7
- **Doors.** Every door can be opened and closed (E). Only the master key (the basement key) locks them. Mom and Dad open doors on their way and close them behind them. The man walks through closed doors and kicks in locked ones. Doggo can't open doors at all.
- **Basement and Dad's cabinet.** Unlock the basement with the key and it stays unlocked. The same key opens Dad's cabinet: swap the key for a VHS tape and play it on the old TV in the basement. The medicine fridge explains the juice.
- **Parents react to how the day went.** Normal day: they hand you over (they don't know they're part of the clean-up). Dark day: they want him gone. All five chores and nothing dark this loop: they fight for you. Dad goes at him with a lamp, Mom sits on the floor next to him crying.
- **Convince them.** On a befriended day, tell Mom or Dad about the loop with proof (the phone call or the files) and they'll run with you. False Hope ending.
- **The man runs out of patience.** If he can't find you, he warns you at 16:57 and hurts Dad at 16:58 and Mom at 16:59 (dark points each time). Stay hidden to 17:00 and you get They Paid. Keep running instead and you get Tired Out.
- **Doggo.** "Sit! Stay!" keeps him out of it (or shut him behind a door). If you hurt the man, Doggo is the first thing he hurts back. If Doggo is in the room when he grabs you, he jumps in front: Good Boy, a really bad ending.
- **The secret ending** now leaves you at maximum darkness.
- 5 new memories (42 in total), 3 new endings (7 in total).

## What's in v0.6
- **Held items.** The kid holds one thing at a time: keys, knife, stick, trash, dog food or a flashlight. Grabbing something new means putting the old thing back first. "Put it back" works on the spot it came from. The HUD shows IN HANDS and the kid carries it.
- **Pantry** (dog food, flashlight) and **sink** (dishes). Doggo's bowl now needs food from the pantry.
- **Stick** from the woodpile: for throwing with Doggo, or swing it at the visitor (+1 dark, knife is now +2). **Flashlight** blinds him for a few seconds during the power cut.
- **5 chores, 100 kr.** Doggo, trash, room, dishes and firewood (20 minutes). All done by 16:40 gives the allowance. It's tight on purpose.
- **Zulda: Flute of Time.** The kid needs his last 100 kr for the remake on his Fliptendo Fold 2. Buy it after the allowance. The Fliptendo also runs a tiny Flappy Plane mini game.
- **Screen Time** joke ending: play the Fliptendo 3 times in a row (10 min each).
- **Memory recap.** At the end of a loop, every new memory shows up one at a time with a chime.
- **Memory log** scrolls and has an Endings tab with hints.
- **The clock keeps running in the chase.** Survive until 17:00 and you run and hide until you're too tired (Tired Out).
- **No more repeated dialogue** from fast clicks.

## What's in v0.5.1
- **Game speed** in the debug panel (1.0x to 2.0x in 0.2 steps) to playtest shorter loops.
- **Act 2 starts later.** The eerie music now starts when you find out about the handlers (or from loop 8), not automatically at loop 4.

## What's in v0.5
- **New look, same open floor plan.** Plank floors, tiled kitchen and bathroom, 3/4 view walls with wallpaper, a red log cabin outside, furniture shadows and detail, outlined characters, a real light map (lamps at dusk, stove and TV glow, headlights, a dark power cut), rain in the storm, dust when you run.
- **Casio with seconds**, F-91W style. It rolls forward through skipped time.
- **Faster waiting.** Hold T for 2 game minutes per second, ramping up to 5.
- **Parents react to big choices.** Pull a knife, tell them about the loop or interrogate Dad and they lock into a state for the rest of the loop (scared, cold or broken). No more small talk about dinner after the knife.
- **Interaction fixes.** E picks whatever is closest in front of you. People stop and face you while you talk. Announcements no longer fire late after a time skip.

## What's in v0.4.2
- **Rarer reality shifts.** Never two within 5 loops, one guaranteed within 10 (by default). The first one is always the 45 minute toilet so that memory stays findable.
- **Debug panel** (`O` with ?debug) to tune all of that live, plus quick test tools.
- **Phone mode**: touch controls, fullscreen + landscape on first tap, a "turn your phone sideways" screen in portrait.

## What's in v0.4.1

- **Sharp on scaled displays.** The canvas is sized in real device pixels (Windows 125% / 150% scaling made the browser stretch it with nearest-neighbour, so text and the logo looked blocky).
- **F for fullscreen**, version number in the corner of the title screen.
- **Debug hard reset** (Shift+R twice on the title screen).
- **More Doggo.** "Sit!", play with his ball (indoors), and "Find it, boy!": normally junk, at 3+ dark points he digs a syringe out of the bins (new memory "The syringe", 37 total), and once you're tainted he digs up the basement key from the flagpole.

## What's in v0.4

- **Cleaner audio.** The music is now pulled by the audio device (callback) at 44.1 kHz instead of being topped up once per frame at 22 kHz, every note gets a short fade in and out, the square lead is band-limited, and there's a gentle low-pass and soft clip on the master. Sound effects got the same treatment. This is what was crackling.
- **Speech bubbles.** Mom's chore list, Dad heading to the shed, Doggo at the door and the visitor muttering now pop up over their heads instead of stopping the game with a dialogue box.
- **Shift to run** and a slightly faster walk.
- **Softer text and a crisp logo.** UI font is Poppins (OFL, see `assets/ui-font-OFL.txt`), mipmapped so it stays smooth at any size. The logo is redrawn as vector art at 2048x1024. The splash dissolve is slower.
- **The visitor has to find you.** He searches room by room (living room, kitchen, hallway, bedrooms, bathroom, then the shed and the yard), checks some hiding spots and not others, and sees you in a cone with walls blocking his view. If he sees you he runs (faster than you walk, slower than you run). Break line of sight and he loses you, then searches that room again. Each sweep he gets more thorough. If he sees you hide, he walks straight to it.
- **More panic time.** After Dad runs, the visitor walks back in from the yard before the search starts.
- **You still can't win by running.** Past the fence: headlights, carried back.
- **Killing him takes you with him.** First time: a dead man's switch on his belt, something behind your ear, white, loop. That gives a new memory ("He takes me with him"). Knowing it, you can take the box off his belt first, which is the only way to the Monster and End of the Program endings now.

## What's in v0.3

- **Soundtrack.** Procedural synth, one mood per act: cute and bouncy, then a wrong-sounding music box, then a slow hopeless drone. A heartbeat during the collection. No audio files, it's all generated in code.
- **Dark points are a soft counter (0 to 15).** 0 to 5 is just a weird kid (planks, bolts). Tainted at 6. Doggo is worth 10 on his own.
- **Calmer weirdness.** No glitches in act 1. Shivering furniture, scanlines and barking fits start in act 2 (after the handlers clue or a few failed loops) and are rarer.
- **Doggo at half volume.**
- **Sund Origin Studio splash and a MADE WITH AI badge** on the splash, title screen and memory log.

## What's in v0.2

Everything from v0.1, plus:

- **Interactive 16:50.** The visitor walks in for real. Stand still and he walks. Run and he's twice your speed. Hide in the wardrobe, under a bed, behind the couch or in the toolbox (v0.4: he has to search for you now). Walk up to him and press E to talk, fight or use what you know. If Dad is free he yells RUN and bolts first. The visitor chases him down on screen. Off screen bang.
- **The dark route.** The knife needs 5 confirmations (2 once you're dark). Dark points are saved forever, even through New Game. The kid, the narrator, the title screen and Mom and Dad all change once you're tainted.
- **Arsenal** at 3 dark points: marbles, tripwire, paint can. Realistic results. Two traps put him on the floor.
- **Dad's key.** Dark route: interrogate Dad in the shed at 6 dark points (off screen). Clean route: very obscure, and the narrator accuses you of looking it up.
- **The basement.** Screens, files, the chair, the medicine fridge and the program's radio.
- **Fortress.** Lock the front door and keep Mom and Dad from unlocking it. He comes through the window instead.
- **Endings.** Monster, End of the Program, and the secret ending (100% memory log, mash E in the trunk).
- **Act break 2:** Mirror Lag. Narrator stages 3 and 4.
- **More stuff to poke:** photo album, calendar, bookshelf, wardrobe, woodpile, new dialogue for everyone, the visitor at the gate.
- **4 frame walk** with arms and blinking. Doggo trots.
- 35 memories in 8 topics, 18 achievements.
