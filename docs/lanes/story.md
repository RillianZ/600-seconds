# Lane: story (v0.20)

Branch `claude-lane-story-v0.20`. Tasks T-062, T-064, T-068, T-069, T-089, T-084, T-085.
Every line below is new and is a proposal for Noah. Story test: 101 of 101 memories (was 86), 13 endings, all reachable.

## Where the code is

- New file `src/game/StoryMore.inc`. Story.cpp includes it at the very end, inside the namespace. No build script changes needed: the existing sim, desktop, web and Windows commands keep working as they are. Listed in the README layout.
- Small marked blocks (`// v0.20 story`) in Story.cpp: AllTopics (new topic The watch), three achievements, the memories block at the end of AllMemories, ShiftName, RollShift, BeginLoop (reset + `ShiftOpener()`), one hook each at the top of `Interact` and `Choose`, one line each at the end of `MomQuestions` / `DadQuestions`, one option each in the Mom, Dad and visitor menus, the console option, and one or two lines inside the endings and scenes that now teach something.
- Story.h: two new Shift values, two ShiftConfig flags (not saved, no panel row), per-loop fields `prevEnd`, `predMom`, `predDad`, `fourthNoticed`, `unitNoticed`, and the private declarations.
- main.cpp: one line. The debug panel's "force next reality" now cycles through all 7 shifts instead of 5.
- test/shots.inc: commands `shift fourth|unit|sister|none`, `prevend X`, `journal N`.
- test/sim.cpp: one block before the clean key route (LOOP 20a to 20d, plus fresh-story checks for predictions, the two shifts, the TV and the tainted lines).

## Calls I made (for Noah to react to)

1. **The menus were too long.** Mom's and Dad's "Ask about something I remember" lists already reach the bottom of the screen late in the game. So the new ones sit one step further down: "Tell her/him what happens next (n)" for the predictions, and "Something newer (n)" for everything else. Same for the man at the gate: "Say something I remember (n)".
2. **Secret ending gives no memory.** The Longest Survivor needs 100% memory, so a memory from it could never be had. Instead, having that ending opens a Mom question ("There are other kids at Finse").
3. **Monster and End of the Program share the memory "Confirmed"** (the word his radio wants). The main test story never gets plain Monster, so Program gives it too, plus its own memory "0612".
4. **The Empty House, The Handlers, Last Toll and False Hope already gave a memory.** Only the leash was used for anything. Now Nobody left, Handlers and The last toll each open a new option too.
5. **Predictions** are only offered before the thing happens, once per parent per loop. Each one moves that parent a step: first uneasy (Mom's mood also drops), second Cold, third Scared. A scared parent means the Dark stance at the knock. Telling all four things in one loop (to either of them) gives the achievement Fortune Teller.
6. **Early lessons** (shove Mom, hit Dad) are always in their normal menus from loop 2, in red. 1 dark point each. Mom goes Cold, Dad goes Cold (Scared if you're tainted). The memory Small hands needs both, in any loops.
7. **"No Soul" lines**: I read this as the parents noticing something has gone out of a tainted kid. On even loops a new line replaces the old "You look at us like we're strangers" / "You okay?" first line.
8. **The TV** only shows the new episodes from the eerie stage on (handlers known, or tainted), and never before you've seen "Previously On", so that achievement still comes first. The trunk episode only shows on even loops, since most loops end in the trunk.
9. **The two new shifts give achievements, not memories** (like the sister one), so the story test doesn't depend on luck.
10. **Mirror left alone.** The watch topic was meant to be reachable from the mirror too, but the knife lane owns the mirror. Its five entries come from the bed, the basement screens, hinge D, Dad and Same House.
11. **The 05 contradiction.** The cross out back says 05 and his clipboard says 05 ticked today. I made it a memory ("Same number": they hand the numbers down) instead of a bug. It also fits oh-seven and "same house, next cycle". Dad's line about 05 says "he", the visitor's says "her". I left both as they are; that can be a second 05.

## T-062 Every ending teaches something

| Ending | New memory | Opens |
| --- | --- | --- |
| Monster | Confirmed (The program) | Basement console: "Hold down PICKUP CONFIRM. "Confirmed."" |
| End of the Program | Confirmed + 0612 (The program) | Dad: "What's 0612?" |
| The Longest Survivor | none (needs 100%) | Mom: "There are other kids at Finse." |
| Screen Time | Under the well (The visitor) | The man at the gate: ""Under the well."" |
| False Hope | (already) 31 kilometres | (already) the scar questions, One Good Hour |
| Good Boy | He had one too (Doggo) | The man at the gate: "Did you have a dog?" |
| They Paid | Just like us (The dark) | Dad: "If he can't find me, he starts on you." |
| One Good Hour | Four chairs, three plates (Mom) | Mom: "Can we set a fourth plate tonight?" |
| The Empty House | (already) Nobody left to hurt | The man at the gate: ""You always came out for them."" |
| Same House | Twice round (The watch) | The man at the gate: ""Your watch goes round my wrist twice."" |
| After | Two of us (The visitor) | The man at the gate: ""After. It's after now."" |
| The Handlers | (already) Handlers | Mom: "If he's gone, don't drive me there." |
| Last Toll | (already) The last toll | Dad: "Do you always get the almond?" |

New lines in the endings:
- Monster (no Appendix C): "His car is still at the gate. On the seat, his radio crackles. "Collector, confirm." You don't touch it. It asks all night."
- End of the Program: "The radio on the seat asks for one word. "Collector, confirm." You learn to say it. After that every gate opens." / "The gate code is 0612. Week 612. They used your week."
- Screen Time: "At the third dungeon he leans over without looking. "Under the well." He's right. Nobody knows that."
- Good Boy: "He looks back too. Just once. "I had one like him," he says. "Same spot by the door.""
- One Good Hour: "...He takes two. He holds his fork the way Dad does." (the last sentence is new)

Memories (title: text):
- Confirmed: After, his radio asked for one word. "Collector, confirm." Finse doesn't need to see a face. Only to hear the word.
- 0612: It took nine loops to learn the road to Finse. The gate code was 0612. Week 612. They used my week.
- Under the well: He let me bring my Fliptendo. At the third dungeon he leaned over without looking and said "under the well." Nobody knows that. You only know that if you played it a hundred times.
- He had one too: After Doggo, the man looked back at the door too. Just once. He said he had one like him. Same spot by the door.
- Just like us: I stayed hidden while he made them pay. He said I'm not different. I'm just like them. Not like Mom and Dad. Like him.
- Four chairs, three plates: We sat down and he came in. Four chairs, three plates. He took two potatoes. He held his fork the way Dad does. Mom said sit down or get out, and he sat.
- Two of us: I didn't finish it. He snapped his dose in half. The next hour his car was at the gate at four with the engine off, and he never knocked. Two of us remembered.

The options they open:
- Console, "Confirmed": You: "(You press PICKUP CONFIRM and hold it down.) Confirmed." / Voice: "(a pause) ...Collector? Our clock says you're still at the gate." / Voice: "(another voice, further off) That isn't him. That's the house." / "(The line clicks dead. Upstairs, very faintly, the landline starts to ring. Nobody answers it.)" / You: "(They heard me. They know where the word came from.)"
- Dad, 0612: "What's 0612?" / Dad: "(the wrench stops) That's a gate code. From work. Where did you..." / Dad: "(he checks that the radio is off. Twice.) Don't say numbers out loud in this house, buddy. Somebody's always listening."
- Mom, other kids: "There are other kids at Finse. Rooms full of them. They all wear my watch." / Mom: "(she sits down at the kitchen table. Slowly, like it hurts.) ...How many?" / "All of them." / Mom: "(very quietly) They told us you were the only one. They told us you were special."
- Gate, under the well: "Under the well." / Him: "(he almost smiles) Third dungeon. Everybody looks in the tower." / "(then it's gone) ...How do you know I know that?"
- Gate, dog: "Did you have a dog?" / Him: "(he looks at Doggo behind your legs for a long time) ...Once. Same house." (at the doghouse if no Doggo) / "Go inside, 06."
- Dad, they pay: "If he can't find me, he starts on you." / Dad: "(he puts the wrench down) Who's he?" / "You know who." / Dad: "(a long breath) ...Then you let him find you, buddy. You hear me? Don't you dare stay hidden for us." / You: "(He means it. He's the one who'd scream first.)"
- Mom, fourth plate: "Can we set a fourth plate tonight?" / Mom: "(she laughs) For who? Julenissen?" / "For whoever comes." / "(She looks at you for a long time. Then she takes a fourth plate down from the shelf and puts it on the table. She doesn't ask again.)"
- Gate, came out: "You always came out for them." / Him: "(he goes very still. Snow settles on his shoulders.) ...Who told you that?" / "You did."
- Gate, twice round: "Your watch goes round my wrist twice." / Him: "(he looks down at his own wrist) Twice. It did on me too. The first year." / "(he pulls his glove back over it) You don't know that. Nobody's worn this but me."
- Gate, after: "After. It's after now." / "(His hand goes to his coat. To the pen. It stays there.)" / Him: "...Who says that?" / "You do. Next time."
- Mom, handlers: "If he's gone, don't drive me there. Handlers don't deliver." / Mom: "(the glass stops halfway to the shelf) ...Drive you where?" / "Finse. They walk you round the back of the building." / Mom: "(she puts the glass down very carefully) We're retiring after today, sweetie. They promised." / You: "(She says it like a prayer. It's the only one she knows.)"
- Dad, almond: "Do you always get the almond?" / Dad: "(he grins) Every single year. Your mother's a terrible cheat." / You: "(He doesn't know I watched her do it. He doesn't know there's a last bell.)"

## T-064 The watch (new topic, between The program and The dark)

- Two short, one long (bed, new option "Look at my watch."): My watch beeps at 16:00. Two short, one long. I never set an alarm. There's no button for one. ORIGIN, model 06. I've never seen one in a shop.
  - Lines: "(ORIGIN. Model 06. A plain face. No date. No buttons, except the one that doesn't do anything.)" / "(At 16:00 it beeps. Two short, one long. I never set an alarm. There's no way to set one.)" / holding the tiny watch: "(I hold the tiny one next to it. Same face. Same beep, probably. It just stopped beeping for somebody smaller.)" / knowing his watch: "(His is the same. Scratched grey. Older. Like mine will be.)"
- It counts down (the basement screens, once you've seen them before): The little screen in the basement shows a watch. Not mine. Grey, scratched, a strap that doesn't match. It isn't telling the time. It's counting down to ten to five.
  - Lines: "(The small screen in the corner. A watch. Not yours. Grey, scratched, the strap doesn't match.)" / "(It isn't telling the time. It's counting down. 19:20)" (the real time left) / You: "(Down to ten to five. Somebody wants to know exactly when he gets here. Or he wants to know exactly when to come.)"
- Drift (hinge D, when his watch beeps): When I opened the door for him his watch beeped. Two short, one long. He called it drift. Mine does that every day at 16:00. Maybe it's been saying drift my whole life.
  - Line: "(Two short, one long. Mine does that every day at 16:00.)"
- The next size up (show Dad the tiny watch): Dad says I got a new watch every time the strap got too small. Same face. Same number on the back. You don't take them off. You grow into the next one.
  - Dad: "(quieter) You got a new one every time the strap got too small. Same face. Same number on the back." / "You don't take them off, buddy. You grow into the next one."
- Twice round (Same House): I finished it and took his watch. The strap went round my wrist twice. ANCHOR 06, crooked. My thumb found the letters by itself, like it already knew the way.

The program, two more:
- Zero-four had a dog (the console's Listen, once you've opened his trunk): Under the static on the console: unit 04, Geilo, collected 13:10. Dog destroyed. The dog hair in his trunk wasn't Doggo's colour.
  - Voice: "(static) ...unit 04, Geilo, collected 13:10. Anchor reset. Dog destroyed..." / "...unit 05 collected 14:40... unit 06, 16:50..." / You: "(Dog destroyed. The hair in his trunk wasn't Doggo's colour.)"
- Same number (the cross and his clipboard, whichever you see second): The cross out back says 05. His clipboard says 05, ticked, today. Two kids. One number. They hand the numbers down.
  - "(05 on the cross. 05 on his clipboard, ticked, today. That's two different kids.)" / "(They don't make new numbers. They hand them down.)"

## T-068 New questions (under "Something newer")

Mom:
- New hinges: "Why does my door have new hinges?" / Mom: "(she doesn't look up) Your father did them. The old ones squeaked." / "Every door in this house squeaks. Only mine got new ones." / Mom: "(she keeps stirring. She stirs for a long time.)" Tainted: Mom: "(she looks down the hall at your door. Then at you.) ...So we'd hear you."
- Hand-me-downs: "Did I ever have a big brother?" / Mom: "(a long pause) No, sweetie. Just you. There's only ever been you." / You: "(Only ever. She says it like it means something else.)"
- Thirty kilometres: "What's thirty kilometres from here?" / Mom: "Nothing. Snow. Why?" / "Somebody walked that far once." / Mom: "(the spoon stops) ...Then he must have been very cold." / You: "(I didn't say he.)"
- Piece A: "Who did you hum to before me?" / Mom: "(the plate stays in her hand) ...Nobody. You're the only one I ever hummed to." / You: "(She told him the same thing through the bathroom door. Week 612. He didn't believe her either.)" She goes Cold.
- Piece D: "Did anybody ever open the door before he knocked?" / Mom: "(she laughs, but not really) Once. A long time ago. He stood right there in his socks." / "What happened to him?" / Mom: "(she turns back to the stove) He caught a cold."
- He remembers: "Would he remember this house without his medicine?" / You: "If he didn't take his medicine, would he remember this house?" / Mom: "(she goes very still) Who, sweetie?" / "The man. The pen in his coat." / Mom: "...Go and wash your hands for dinner." / You: "(She didn't say no.)"

Dad:
- Dad's knot: "Who else did you teach your knot?" / Dad: "Nobody, buddy. Just you." / "(he ties one in the rag without looking. Then he looks at it for a long time.)" / "...Just you."
- New hinges: "Why did you put new hinges on my door?" / Dad: "Old ones broke." / "How?" / Dad: "(he picks the wrench back up) Somebody pushed a chest of drawers against it. Long time ago. Before your time." / You: "(Before my time. In my room.)"
- Piece B: "Did you scratch my watch with a nail?" / Dad: "(he looks at his hands) ...Four days after. You kicked so hard I got the six wrong twice." / "I wanted one thing on it that wasn't theirs." / You: "(Crooked letters. On the tiny watch. And on his.)"
- Piece C: "What does reissue mean?" / Dad: "(he turns the radio face down without looking at it) It's when they print a book again. Same book. New cover." / "Same house?" / Dad: "...Go inside, buddy."
- Hand-me-downs: "Whose coat did the man wear before him?" / Dad: "(a long time) I don't know his name. I knew the one before." / "What was he like?" / Dad: "Quiet. Good with knots."
- The ice hole (out back egg): "What's on the rope in the ice out back?" / Dad: "Fish trap. I check it every Christmas." / "The end's cut." / Dad: "(he doesn't look up) ...Then something got away."

No Soul (tainted, even loops, the parent's first line that loop, in turn):
- Mom: "(she watches you come in) ...Your eyes do the smile, kiddo. The rest of you doesn't."
- Mom: "(she looks at you a moment too long) You used to laugh at the TV. You don't any more."
- Mom: "(quietly, to the pot on the stove) Something's gone out of him. I don't know when."
- Dad: "(he doesn't turn round) You used to cry at the end of films, buddy. Even the funny ones."
- Dad: "(he keeps the bench between you) Where'd you go, buddy? You're right there and you're not."
- Dad: "(he looks at you, then at the radio) ...They said this might happen. They didn't say what it'd look like."
- Plus the tainted answers in the questions (Mom's hinges above, shove and hit below).

## T-069 The TV (from the eerie stage, after Previously On)

- Last loop you ran onto the vidda, or False Hope: "(A travel programme. A car drives through snow at night. A road sign in the headlights: 31.)" / "(The car stops. The presenter doesn't get out. "And that's as far as we go today," she says. "That's as far as anyone goes.")"
- Last loop the snow got you: "(A nature programme. A fox curls up in the snow, nose under its tail, and goes to sleep.)" / "("At minus twenty-five the cold stops hurting," says the narrator. "That's how you know.")"
- Last loop Doggo died for you: "(An old film. A dog waits at a train station every day for a man who never comes back.)" / "(You change the channel. It's the same film. The dog is still waiting.)"
- Last loop you were taken (trunk, express, spared, Dad's ten seconds, your name, no key), even loops only: "(A magic show. The magician puts a boy in a box and closes the lid. He taps it twice. He opens it. Empty.)" / "(Applause. The magician checks his watch. "Same time tomorrow," he says. "Same boy.")"
- The first time ever: You: "(It's showing me what happened last time. Nobody else watches it like that.)"
- Anything else: the old episodes as before.

## T-089 Two stranger realities

**The photos** (`Shift::Fourth`, achievement Family Portrait):
- At 16:00: "(The house smells like somebody else's coat.)"
- Album: "(Me at 4. Behind me, a boy of about sixteen in a red sweater. He isn't looking at the camera. He's looking at me.)" / "(Me at 6. The same boy. Older. The sweater is too small for him now.)" / "(Me at 9 with a fish. A man at the back in a black coat. Same face. Snow on his shoulders. Indoors.)" / "(Still no baby photos. Not of me. Not of him.)"
- Coat hooks: "(Dad's jacket. Mom's jacket. My jacket. And a black coat, much too big, with snow still on the shoulders. It's dry underneath.)" / "(Nobody else lives here. Nobody else has ever lived here.)"
- Ask Mom (after the album): "Who's the man at the back of the photos?" / Mom: "(she looks over your shoulder at nothing, smiling) That's you, sweetie." / You: "(She goes back to the stove like she said something normal. I'm in the front. I'm always in the front.)"
- Ask Dad: same question / Dad: "That's just you, buddy. At the back. You always stand at the back." / You: "(I'm in the front. In every single one.)"

**Oh-seven** (`Shift::NextUnit`, achievement Next in Line):
- At 16:00: Dad: "(from downstairs) Oh-seven! Up and at 'em, buddy!" / You: "(Oh-seven? I'm zero-six. I've always been zero-six.)"
- Wardrobe: "(My clothes. Every label says S07. Under it you can feel the stitches where an older label was cut out.)"
- Mom's desk: "WEEKLY REPORT. SUBJECT 07. Week 1. Settling in well. Unaware." / "(Under it, an older page with one line through it: SUBJECT 06. Week 612. REISSUED.)" / You: "(Week one. I've lived here my whole life. Haven't I?)"
- Ask Mom: "Why does everybody call me oh-seven?" / Mom: "(she smiles) Because that's you. Lucky number seven. They gave you a lucky one." / You: "(Then who got six?)"
- Ask Dad: "Why did you call me oh-seven?" / Dad: "(he frowns, like you're joking) Because that's your number, buddy." / "Zero-six was before you. Good kid. You've got his room." / You: "(Same house. Next cycle.)"

Both are in the roll like the sister one (flags `fourth`, `nextUnit` in ShiftConfig, on by default, not saved, no panel row). The debug panel's "force next" reaches them. ShiftFrequencyTest still passes (gaps 5 to 10).

## T-084 Early lessons

- Mom, "[Shove her] Out of my way." (red): "(You shove her. Both hands. Everything you've got.)" / Mom: "(she takes one step back. One. She looks down at you.)" / Mom: "...Did you just PUSH me?" (tainted: "(she doesn't say anything. She just keeps her hands where you can see them.)") / You: "(She barely moved. My wrists hurt.)"
- Dad, "[Hit him] As hard as I can." (red): "(You punch him in the stomach. As hard as you can.)" / Dad: "(it's like hitting the generator. He doesn't even step back.)" / Dad: "(he takes your wrist, gently) Hey. HEY. What's that for, buddy?" (tainted: "(he doesn't catch it. He lets it land. Then he looks at you like he's writing it down.)") / You: "(My knuckles hurt. He's fine. He isn't even angry.)"
- After both: narrator "I was twelve. Hitting only works in films. If I wanted to beat anyone in that house, I'd have to be smarter than them. Or bigger."
- Memory Small hands (The dark): I hit Dad as hard as I could. It was like hitting the generator. I pushed Mom and she moved one step. I'm twelve. Hitting doesn't work. Not on them. Not on him.

## T-085 Predictions (under "Tell her/him what happens next")

Needs: the phone (heard the 16:22 call or answered it), the flagpole (the key memory or watched Dad at 16:44), the storm (watched it hit), the knock (The man comes at 16:50). The phone time says 16:21 once events start early.
- Mom, phone: "The phone's going to ring at 16:22. It's for you." / Mom: "(she laughs) Are you expecting a call, mister?" / "No. You are. They'll ask if I suspect anything." / Mom: "(the smile stays where it is. The rest of her face leaves.)"
- Dad, phone: "At 16:22 Mom gets a phone call. She says I don't suspect a thing." / Dad: "(he stops working) ...Who told you that, buddy?" / "Nobody. It just always happens."
- Mom, flagpole: "At 16:44 Dad goes out and checks the little box under the flagpole." / Mom: "(her eyes go to the window, to the pole, and back) What box, sweetie?" / You: "(She knows what box.)"
- Dad, flagpole: "At 16:44 you'll go out to the flagpole and check the box. You check it every year." / Dad: "(very still) ...Have you been following me?" / "I don't need to."
- Mom, storm: "The storm hits at 16:47. The lights flicker, and you can't see the gate." / Mom: "(she looks out. Clear sky, nearly.) The radio said after five." / "The radio's wrong. It's always wrong."
- Dad, storm: "The storm comes at 16:47. You'll say everybody inside, and nobody opens that door." / Dad: "(he checks the sky, then you) ...That's what I'd say, alright." / Dad: "(he doesn't laugh. He goes back to work and gets it wrong twice.)"
- Mom, knock: "At ten to five the power goes out. Then three slow knocks." / Mom: "(she drops the spoon in the pot) ...Julenissen. You mean Julenissen." / "He doesn't have a beard."
- Dad, knock: "At ten to five the power dies. Then a man knocks three times. You let him in." / Dad: "(the wrench goes down on the bench, very carefully) Buddy. Who have you been talking to?"
- After the first: Mom "(she goes back to the stove. She keeps looking at you sideways.)" / Dad "(he goes back to work. He keeps one eye on you.)"
- After the second (Cold): Mom "(quietly) I think you should go and play in your room for a while." / Dad "(not looking up) Go inside for a bit, buddy. Please."
- After the third (Scared): Mom "(she backs up until the counter stops her) How do you KNOW these things?" / Dad "(he takes a step back from you. A whole step.)"

## Achievements (3 new)
- Family Portrait: "Some days there's somebody else in the photos."
- Next in Line: "Some days you have a different number."
- Fortune Teller: "Tell them what happens before it does. The phone, the flagpole, the storm and the knock."

## What I checked
- Story test: `g++ -std=c++17 -O1 -Isrc test/sim.cpp src/game/Story.cpp` passes, 101/101 memories, all 13 endings, ShiftFrequencyTest passes. The secret ending still comes at LOOP 10.
- New test block: every new question is offered and answered on the main story, predictions move Mom Normal, Cold, Scared and close after the event, both shifts, all four TV episodes, Previously On still first, the tainted lines.
- Linux desktop build with screenshots (`docs/lanes/story-shots/`): Mom's ask menu with the two new entries, a new Mom answer, the fox on TV, the photos shift, the memory log with the Watch tab (all tabs still fit), Dad's "tell him" menu.

## What I didn't do
- The mirror (knife lane). The radio's 16:10 "Unit 06 is stable" doesn't change to 07 in the oh-seven shift (CheckEvents left alone).
- No line in the endings for the two new shifts (the sister shift has a few).
- Nothing in the hunt for the new memories. The new options are all talk.
- No web or Windows build (as asked).
