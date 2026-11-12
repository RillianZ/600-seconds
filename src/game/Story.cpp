// 600 Seconds - Story logic. Engine-free.
#include "Story.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <random>

namespace sixhundred {

// ============================================================ memory log
const std::vector<std::string>& AllTopics() {
    static const std::vector<std::string> t = {
        "The hour", "The house", "Mom", "Dad", "Doggo", "The visitor", "The basement", "The program", "The watch", "The dark"};   // v0.20 story: The watch
    return t;
}

const char* ItemName(Item it) {
    switch (it) {
    case Item::Knife: return "Knife";
    case Item::Key: return "Master key";
    case Item::Stick: return "Stick";
    case Item::Bag: return "Trash bag";
    case Item::DogFood: return "Dog food";
    case Item::Flashlight: return "Flashlight";
    case Item::Tape: return "VHS tape";
    case Item::BabyWatch: return "Tiny watch";
    default: return "";
    }
}

const std::vector<EndingEntry>& AllEndings() {
    static const std::vector<EndingEntry> e = {
        {"monster", "Monster", "Somebody has to stop him. Somebody small."},
        {"program", "End of the Program", "Like Monster, but you know where the machine is."},
        {"secret", "The Longest Survivor", "Remember everything. Then stay awake."},
        {"screen", "Screen Time", "Just one more game. And one more. And one more."},
        {"escape", "False Hope", "They'd run with you. If they knew what you know."},
        {"doggo", "Good Boy", "Doggo would do anything for you. Don't let him."},
        {"paid", "They Paid", "Stay hidden. No matter what you hear."},
        {"bells", "One Good Hour", "Running doesn't work. You'd have to know that first. Then stay."},
        {"empty", "The Empty House", "Leave him nobody to make pay. Not even the dog. Then last until the bells."},
        // v0.17: the true ending, two ways
        {"samehouse", "Same House", "Five doors. Him on the floor. Nobody else in the room. Then finish it."},
        {"after", "After", "Five doors. Him on the floor. Say all of it. Then don't."},
        // v0.18: Noah's answer 7. Beat him, and it matters whose side they were on
        {"handlers", "The Handlers", "Finish him in front of them, while they're still on his side."},
        {"lasttoll", "Last Toll", "Get them both on your side. Then beat him."},
    };
    return e;
}

const std::vector<TimelineEntry>& AllTimeline() {
    static const std::vector<TimelineEntry> t = {
        {2, 0, "chores", "Mom lists the chores. All five by 16:40.", ""},
        {8, 0, "dad_out", "Dad goes out to the shed to fix the generator.", ""},
        {10, 0, "radio", "Dad reports on a radio in the shed.", "dad_radio"},
        {22, 0, "phone", "The landline rings. Mom answers after 40 seconds.", "phone_mom"},
        {30, 0, "basement", "Mom unlocks the basement. The door is open for two minutes.", "basement_open"},
        {35, 0, "doggo_door", "Doggo whines at the front door until 16:40.", "doggo_out"},
        {40, 0, "present", "Mom checks the chores. Dad comes in from the shed.", ""},
        {43, 0, "car", "Headlights at the gate. A black car, no plates.", "his_car"},
        {44, 20, "window", "The man stands at the living room window and looks in.", "prints"},
        {44, 40, "flag44", "Dad checks a tiny box at the bottom of the flagpole.", ""},
        {46, 0, "mom_bath", "Mom goes into the bathroom. Two and a half minutes.", ""},
        {46, 30, "car_in", "The man goes back and sits in his car.", ""},
        {47, 0, "storm", "The snowstorm hits. The road closes behind him.", ""},
        {49, 0, "car_out", "He gets out again. Something under his coat.", ""},
        {50, 0, "knock", "The power dies. Three slow knocks.", "visitor_time"},
        {57, 30, "threat", "If he can't find me he starts on them.", "they_paid"},
        {60, 0, "bells", "Bells, far away. The watch beeps. 16:00 again.", "loop"},
        {15, 0, "ribbon", "Doggo noses in under the tree and gets the ribbon round his legs.", ""},   // v0.20 knife
    };
    return t;
}

const std::vector<AchievementEntry>& AllAchievements() {
    static const std::vector<AchievementEntry> a = {
        {"Grounded", "Push Mom far enough."},
        {"Constipated by Fate", "Some days the toilet takes its time."},
        {"Should've Gone", "Some days you really should have gone."},
        {"Who's a Good Boy?", "Some days the bowl is nobody's."},
        {"Weather Report", "Some days the storm is early."},
        {"Table for Four", "Some days somebody else is in the bathroom."},
        {"Previously On", "Watch TV once you know who they are."},
        {"Down the Stairs", "Get into the basement."},
        {"Session One", "Play what is in Dad's cabinet."},
        {"Looked It Up", "Find Dad's key at exactly the right minute, without being shown."},
        {"A Nod Across the Yard", "Be out in the yard when Dad and the man are both there."},
        {"Run, Boy", "Give Doggo what he asks for at 16:35."},
        {"Clerical Error", "Make sure nobody confirms anything. Then behave."},
        {"Express Delivery", "Save him the trouble."},
        {"Say It Again", "Make her hear the song."},
        {"Not Alone", "Make them believe you."},
        {"Ten Seconds", "Be worth fighting for."},
        {"Hide and Seek", "Get found."},
        {"Creature of Habit", "Get found in the same spot twice."},
        {"Lost Your Marbles", "Make him look the wrong way."},
        {"Nowhere to Run", "Leave through the gate."},
        {"Tired Out", "Keep running until the bells."},
        {"Snow Angel", "Stay out in the storm."},
        {"Fortress", "Lock him out."},
        {"Home Alone (Realistic Edition)", "Prepare the house for him."},
        {"Interrogation", "Ask Dad the hard way."},
        {"Point of No Return", "Do something you can't take back."},
        {"Who's There?", "Look in the mirror after."},
        {"Entrance Exam", "Read the last page of the protocol."},
        {"Good Boy", "An ending. Doggo would do anything for you."},
        {"Screen Time", "An ending. Just one more game."},
        {"They Paid", "An ending. Stay hidden."},
        {"False Hope", "An ending. Run together."},
        {"One Good Hour", "An ending. Stay together."},
        {"Monster", "An ending. Somebody has to stop him."},
        {"End of the Program", "An ending. Stop him, and know where the machine is."},
        {"The Longest Survivor", "An ending. Remember everything."},
        {"Total Recall", "Fill the memory log."},
        {"Occupied", "Somebody is in the bathroom. Keep it that way."},
        {"Three of Five", "Find a piece behind three different doors."},
        {"Open House", "Don't wait for the knock."},
        {"Stowaway", "Ride along. Not in the trunk."},
        {"Five of Five", "Find a piece behind all five doors."},
        // v0.15: outside
        {"Out the Window", "Leave a room the other way."},
        {"Smells Worse Inside", "Out back there's a present nobody unwrapped. Get in."},
        {"Triple Dare", "Steel, minus twenty-five. Everybody knows. Do it anyway."},
        {"Same Day Again", "Look up at the shed roof, late in the hour."},
        {"Spelled Out", "Read the board behind the shed on a dark night."},
        {"Not a Dog", "Go and see what Doggo growls at."},
        {"Out Back", "Find all six things somebody left out back."},
        {"Nobody Left", "Leave him nobody to make pay. Then last the whole hunt."},
        {"Called Back", "Get Doggo away from the door in time."},
        {"Asked and Answered", "Ask the same thing until they've had enough."},
        {"Dad Jokes", "Hear every joke Dad has."},
        // v0.17: the true ending
        {"Knots", "Knock him down three times. Then make sure he stays down."},
        {"He Remembers", "Tell him all five things while he can't get up."},
        {"Same House", "An ending. Five doors, then finish it."},
        {"After", "An ending. Five doors, then don't."},
        // v0.18: whose side are they on
        {"Search Party", "Make them want him gone. Then hide from all three."},
        {"Just Her", "Tell only Mom. Then let him get close."},
        {"Just Him", "Tell only Dad. Then let him get close."},
        {"His Own Medicine", "Use what Dad left on the floor."},
        {"Family Business", "An ending. Finish him in front of them, while they're still on his side."},
        {"Last Toll", "An ending. Both of them on your side, and him beaten."},
        // v0.19: before the knock
        {"Not Through Here", "Push something heavy against a door. From the inside."},
        {"No Ride Home", "His engine is running and he isn't in the car."},
        {"Breadcrumbs", "Leave him a trail out the back. Late enough that the storm doesn't eat it."},
        {"House Rules", "Not every trap goes on the front step."},
        {"Stage Manager", "Use all fifty minutes. Furniture, keys, tracks and an inside trap."},
        // v0.20 story
        {"Family Portrait", "Some days there's somebody else in the photos."},
        {"Next in Line", "Some days you have a different number."},
        {"Fortune Teller", "Tell them what happens before it does. The phone, the flagpole, the storm and the knock."},
        // v0.20 knife
        {"Blade Down", "Three jobs for the kitchen knife. None of them a person."},
    };
    return a;
}

const std::vector<MemoryEntry>& AllMemories() {
    static const std::vector<MemoryEntry> m = {
        // ---- The hour
        {"loop", "The hour", "The hour repeats",
         "16:00 to 17:00 on Christmas Eve. The bells start, the watch beeps and I'm back in bed. Nobody else notices.",
         "Something about the watch."},
        {"poop45", "The hour", "The 45 minute toilet",
         "Some days the toilet takes 45 minutes. No matter what. Reality is NOT the same every time.",
         "Some days are... different."},
        {"cold", "The hour", "The snow gets warm",
         "Without my jacket I don't even make it across the yard. The snow stops being cold. That's the bad part.",
         "What happens if you stay out in the storm?"},
        {"lit_up", "The hour", "Lit up",
         "Out in the dark his torch finds me from across the yard. My red sweater. The stripes on my jacket. I light up like the tree.",
         "What does he see from far away, out in the dark?"},
        // ---- The house
        {"out_window", "The house", "The other way out",
         "My window sticks in the cold. Hit the frame with the heel of your hand and it opens. Every window in the house does, if you're small enough. Doggo can't follow me out.",
         "Even locked in your room there's another way out."},
        {"small_prints", "The house", "Somebody smaller",
         "Small boot prints out back, smaller than mine. They start at the swing and walk north. Then they stop. The snow never fills them in.",
         "Somebody smaller walked out back."},
        {"handlers", "The house", "They are not my parents",
         "Reports. Phone calls. Radios. Mom and Dad aren't my parents. They're my handlers.",
         "Put the pieces together."},
        {"no_car", "The house", "We don't have a car",
         "No car keys. No car. 31 km to the nearest neighbour. The road closes behind him at 16:50. The only car up here is his.",
         "How would we even leave?"},
        {"syringe", "The house", "The syringe",
         "Doggo dug it out of the trash bins. An empty syringe. The label says 06 - WEEKLY, in Mom's handwriting.",
         "Doggo's nose finds things. Some of them on darker days."},
        {"no_baby", "The house", "No baby photos",
         "The photo album starts when I'm four. Nothing before that. Not one picture.",
         "Families keep pictures."},
        {"calendar", "The house", "PICKUP",
         "The advent calendar. There's no chocolate behind door 24. One word, in Mom's handwriting: PICKUP.",
         "What's behind door 24?"},
        {"presents", "The house", "Nothing in the boxes",
         "I opened a present early. An empty box, taped shut years ago. Only the one I earn has anything in it.",
         "What's under the tree?"},
        {"static_tv", "The house", "The TV changed",
         "Same channel, different episode. A boy in a box. The audience laughs.",
         "Watch TV after you know the truth."},
        // ---- Mom
        {"allies", "Mom", "They didn't know",
         "I told them about the clean-up. Mom sat down on the kitchen floor. They thought they were retiring. They were next.",
         "Be the kid they'd believe. Then tell them what you know."},
        // v0.18: whose side are they on
        {"they_hunt", "Mom", "They looked too",
         "\"He's all yours.\" They meant it. Mom checked under the beds with him and called my name like it was dinner. Dad stood in front of the door and didn't move.",
         "Make them want him gone. Then see who comes looking."},
        {"mom_fought", "Mom", "She went for the box",
         "I only told Mom. When he got close to me she came out of nowhere and went for his belt, not his face. She got the box off him before he put her down. It slid across the floor to me.",
         "Turn only Mom. Then let him get close."},
        {"handlers_hunt", "Mom", "Handlers",
         "I finished him in front of them. Dad took the radio off his belt and said \"Copy.\" Then they looked for me. They know every hiding spot in the house. They put half of them there.",
         "Finish him while they're still on his side."},
        {"told_mom", "Mom", "Telling Mom doesn't work",
         "I told Mom the day repeats. She sent me to my room for 20 minutes.",
         "What happens if I tell Mom?"},
        {"phone_mom", "Mom", "The 16:22 phone call",
         "The landline rings at 16:22. Mom answers: \"Yes. Today. No, he doesn't suspect a thing.\"",
         "Be in the kitchen at 16:22."},
        {"phone_answer", "Mom", "The caller",
         "I answered first. \"Handler two, confirm pickup at 16:50. Clean-up included.\" Handler?",
         "What if I answer the phone before Mom?"},
        {"reports", "Mom", "Mom's reports",
         "Mom's desk: WEEKLY REPORT. SUBJECT 06. Stable. Unaware. Week 612. That's me.",
         "Mom's desk is always tidy. Too tidy."},
        {"book_notes", "Mom", "Notes in the margins",
         "\"Raising Exceptional Children.\" Mom wrote in the margins: dose, drift, week 400 stable.",
         "Mom reads a lot. What, though?"},
        {"my_name", "Mom", "I have a name",
         "I hummed her song back at her. At the end she turned around in the kitchen door and said a name. Mine. The bang came before I could keep it.",
         "She hums so she won't hear. Make her hear."},
        // ---- Dad
        {"dad_fought", "Dad", "He went for the taser",
         "I only told Dad. When the man got close to me Dad hit him low, like he'd been trained to, and tore the taser out of his hand before he went down. It skidded under the couch. One shot left.",
         "Turn only Dad. Then let him get close."},
        {"tased_him", "Dad", "His own medicine",
         "Dad left me the taser. I waited until the man was right on top of me. Every muscle he has locked at once, the way mine do. Then he was on the floor and I was standing.",
         "Use what Dad left on the floor."},
        {"last_toll", "Dad", "The last toll",
         "They were both on my side. He was beaten. We ate riskrem and Dad found the almond and laughed. Mom counted the bells out loud like it was a game. On the last one, 16:00. Nobody else remembered.",
         "Get them both on your side. Then beat him."},
        {"told_dad", "Dad", "Telling Dad makes it worse",
         "I told Dad. He said \"let me make a call.\" The man came at 16:40 instead.",
         "What happens if I tell Dad?"},
        {"dad_radio", "Dad", "Dad's radio",
         "A radio on Dad's bench. Sticker: PROPERTY OF THE PROGRAM. UNIT 06 HANDLER.",
         "Dad spends a lot of time in the shed."},
        {"dad_goodbye", "Dad", "Dad said goodbye",
         "\"Whatever happens today, you were a good kid.\" He wasn't talking about chores.",
         "Maybe help Dad for once."},
        {"dad_key", "Dad", "Dad's key",
         "The basement key is in a tiny box at the bottom of the flagpole.",
         "Dad keeps the key on him. Or does he?"},
        // ---- Doggo
        {"doggo_basement", "Doggo", "Doggo hates the basement",
         "Doggo growls at the basement door. Every time.",
         "Check the basement door with Doggo around."},
        {"doggo_gate", "Doggo", "Doggo hides from him",
         "Doggo won't go near the man at the gate. He hides behind my legs.",
         "Bring Doggo to meet the visitor."},
        {"doggo_door", "Doggo", "He goes to see who it is",
         "When the man knocks, Doggo goes to the door to see who it is. Unless I told him to stay, let him out, or shut him in somewhere. The man gives him a few seconds. Then he doesn't.",
         "Where does Doggo go when somebody knocks?"},
        {"doggo_stay", "Doggo", "He'll jump in front of me",
         "When the man gets close, Doggo jumps between us. Tell him to stay. Or shut him behind a door. He can't open doors.",
         "Doggo would do anything for you."},
        {"doggo_out", "Doggo", "He lives if he's outside",
         "At 16:35 he whines at the front door. I opened it. When the banging started, the barking was already far away.",
         "He asks for something every day at 16:35."},
        // ---- The visitor
        {"visitor_time", "The visitor", "The man comes at 16:50",
         "Power cuts out. A knock. I thought it was Julenissen. A big, calm man. He takes me. The others don't make it.",
         "Wait for 16:50."},
        {"he_runs_fast", "The visitor", "He has to find me",
         "He doesn't know where I am. He searches room by room. If he sees me, he runs. If I run, I'm faster. For a bit.",
         "Try to get away from him."},
        {"hiding", "The visitor", "He checks the hiding spots",
         "He checks the hiding spots room by room. Not always the same one first. If he sees me hide, it's over.",
         "Hide when he comes."},
        {"he_learns", "The visitor", "He remembers where he caught me",
         "He pulled me out of the same spot twice. The second time he didn't search. He walked straight to it. A spot he has found me in is burned. A spot he hasn't is still mine.",
         "Get found in the same spot twice."},
        {"he_hears", "The visitor", "Running is loud",
         "I ran and he turned around two rooms away. He hears running through walls. Walking he only hears when he is close.",
         "Run while he can't see you."},
        {"breath", "The visitor", "Hold my breath",
         "He stood right outside my hiding spot and heard me breathe. When he is that close I have to hold it. I can't hold it for long.",
         "Hide. Let him come close."},
        {"wet_prints", "The visitor", "Snow on my boots",
         "I came in from the yard and hid. He followed the wet prints across the floor right to me. The floor dries. It takes a while.",
         "Come in from the snow and hide."},
        {"decoy", "The visitor", "He looks where it lands",
         "I threw a marble. He went to the sound, not to me. It worked twice. The third time he just smiled.",
         "The marble jar in my room."},
        {"outlasted", "The visitor", "Hiding doesn't end it",
         "I made it to 17:00 without being found. The bells rang and he came anyway. Not being found is not the same as getting out.",
         "Don't get found. All the way to the bells."},
        {"visitor_armed", "The visitor", "He has a gun",
         "I tried to fight him. He shot me in the leg without blinking.",
         "Resist."},
        {"dead_switch", "The visitor", "He takes me with him",
         "I killed him. The box on his belt screamed and something behind my ear burned. When his heart stops, so does mine. The box comes off his belt, though.",
         "What happens if he doesn't get up?"},
        {"run_fail", "The visitor", "There's nowhere to run",
         "I ran into the white. After an hour I found footprints. Small ones. Mine. I had walked in a circle back to the gate.",
         "Leave through the gate."},
        {"visitor_knows", "The visitor", "He expected me to know",
         "\"I know what you are.\" He smiled. \"Good. They said you might.\"",
         "Tell him what you know."},
        {"dad_ran", "The visitor", "Dad fought him",
         "Dad grabbed a lamp, told me to run and went straight at him. It bought me ten seconds. Mom sat on the floor next to him and cried.",
         "Be a kid worth fighting for. Do everything right, all of it."},
        {"taser", "The visitor", "He doesn't run any more",
         "Lead him round a fence or out a window a couple of times, or stand where he can see me but can't get to me, and he stops chasing. He has a taser. Two little hooks, and every muscle I have locks at once.",
         "What does he do when he can't catch you?"},
        {"nobody_left", "The visitor", "Nobody left to hurt",
         "Mom bolted in the basement, Dad barred in the shed, Doggo gone through the gate. I lasted to the bells. He sat down in Dad's chair and talked to the empty room. He said he never got that far. He always came out for them.",
         "Leave him nobody to make pay. Then last the whole hunt."},
        // v0.17: answer 10 and the true ending
        {"tied_up", "The visitor", "Not going anywhere",
         "All three traps, and he went down hard. I tied his wrists with the tripwire. He didn't fight. He just talked in riddles until the bells, and then the hour started over anyway.",
         "Knock him down three times. The tripwire is still lying there."},
        {"he_remembers", "The visitor", "He remembers",
         "He was on the floor and I told him all five. The song, the watch, reissue, the dose, his last pickup. He remembered the house. Then he whispered so only I could hear: he stood where I'm standing once, and the man on the floor told him all of it, and he didn't believe it either.",
         "Five pieces. Say them to him while he can't get up."},
        {"they_paid", "The visitor", "He makes them pay",
         "If he can't find me, he starts on them. 16:57 a warning. 16:58 Dad. 16:59 Mom. I heard all of it.",
         "Stay hidden long enough."},
        {"fortress", "The visitor", "Locked doors",
         "I locked everything. He just came through the window. Locks only make him angrier.",
         "Lock him out. Keep it locked."},
        {"prints", "The visitor", "Someone was here",
         "Big footprints in the new snow, from the gate to the living room window. He looks in at us before he knocks.",
         "Look at the snow outside. Before the storm covers it."},
        {"his_car", "The visitor", "His clipboard",
         "On the passenger seat: 04, ticked. 05, ticked. 06: COLLECT 1, CLEAN 2. And a bun from the bakery in Geilo.",
         "His car is empty while he's at the window."},
        {"trunk", "The visitor", "I'm the third one today",
         "His trunk: a grey blanket, two empty juice boxes, a small mitten that isn't mine. Dog hair that isn't Doggo's.",
         "His trunk isn't locked."},
        {"off_hook", "The visitor", "No call, no clean-up",
         "I pulled the phone out of the wall before it rang. Nobody confirmed anything. He took me and left them standing in the doorway. Alive.",
         "What does he do without his orders? Keep everybody calm and find out."},
        {"echo", "The visitor", "He doesn't remember",
         "\"We do this every time.\" I asked him what he meant. He didn't know he'd said it. He doesn't loop. Something in him leaks anyway.",
         "He repeats himself. Ask him about it before he knocks."},
        // ---- The basement
        {"basement_open", "The basement", "Mom goes down at 16:30",
         "At 16:30 Mom unlocks the basement. Blue light. A hum. The door is open for about two minutes.",
         "Be in the hallway at 16:30."},
        {"basement_screens", "The basement", "Screens",
         "Screens in the basement. One shows my room. One shows me, right now.",
         "Peek while the door is open."},
        {"basement_files", "The basement", "The files",
         "SUBJECT 06. Collect at 16:50. Clean up the handlers. Handlers are NOT to be informed. They think they're retiring.",
         "Get down there. Read everything."},
        {"program_hq", "The basement", "Finse",
         "The program's facility is at Finse. THE MACHINE is underlined three times.",
         "Where does the man take me?"},
        {"treatment", "The basement", "The chair",
         "A chair with straps. Kid-sized. My wrists remember it even if I don't.",
         "There's furniture down there too."},
        {"juice", "The basement", "The juice",
         "S06. RESET. AFTER EACH SESSION. IN JUICE. The juice at 15:55 isn't for naps. It's so I don't remember.",
         "Check what they keep cold down there."},
        {"the_tape", "The basement", "Session one",
         "Dad's cabinet. A tape. The chair, Dad's bored voice, Mom humming. Then the juice. I'll never forgive them.",
         "Dad's cabinet opens with the same key. Play what's inside where it was filmed."},
        {"they_listen", "The basement", "They listen",
         "I used their radio. \"That's not handler two. Stay where you are, 06.\" He came early.",
         "Talk to them on their own radio."},
        // ---- The program
        {"baby_watch", "The program", "I've always worn it",
         "A tiny Origin in Dad's cabinet. The strap would fit a baby. Scratched into the back: ANCHOR 06. DO NOT REMOVE.",
         "There's more than a tape in Dad's cabinet."},
        {"protocol", "The program", "The watch is the anchor",
         "ORIGIN PROTOCOL. When I die I wake up in the version where I didn't. The watch pins it to 16:00. Today's juice was just juice. They wanted me to keep this one.",
         "The filing cabinet has more than one folder."},
        {"collector", "The program", "He wears one too",
         "Under his glove: an Origin, scratched grey, the strap replaced over and over. He was a subject once. They don't come off.",
         "Show him something small. Or look at his wrist when he can't stop you."},
        {"leash", "The program", "31 kilometres",
         "We got 31 kilometres. Then the scar behind my ear beeped. Behind Mom's too. And Dad's. That's how long the leash is. For all of us.",
         "They'd run with you. See how far."},
        {"graduation", "The program", "The entrance exam",
         "APPENDIX C. A subject who kills his collector, and knows to take the switch first, replaces him. Going dark isn't failing their test. It's passing it.",
         "The last page of the protocol. For kids who know about the box on his belt."},
        // v0.15: out back
        {"grave_05", "The program", "Zero-five",
         "Behind the chain fence there's a little wooden cross with 05 carved into it. No name. Dad calls me unit 06. Somebody was here before me. They're still here.",
         "Behind the chain fence, something small with a number on it."},
        // v0.13: the hinges. One piece behind each door. Together they open the last path (not built yet)
        {"piece_mom", "The program", "Not like with you",
         "Mom was locked in the bathroom when he came, so he talked to her door instead of to me. She told him he used to hum her song under the duvet. Week 612. He said he never lived here.",
         "Lock Mom away somewhere he has to talk to her through a door."},
        {"piece_dad", "The program", "Crooked letters",
         "Dad was barred in the shed. He made the man read the back of his watch. ANCHOR 06, scratched with a nail. Dad said he scratched it himself, four days after I was born.",
         "Leave Dad somewhere the man has to go and find him."},
        {"piece_reissue", "The program", "Same house, next cycle",
         "I was in the basement when the power died. The radio console runs on a battery. Finse told him: wipe, age, reissue. Same house, next cycle. He said copy.",
         "Be downstairs when he knocks."},
        // v0.14: hinges D and E (pieces are Claude's proposal)
        {"piece_door", "The program", "After, always after",
         "I opened the door before he knocked. He knew my window sticks in the cold. His watch beeped DRIFT. He took out a needle, then put it back: they make him take it after. It keeps the house out of his head. Most nights he sees a kid in a red sweater, standing in a door.",
         "Don't wait for the knock."},
        {"piece_car", "The program", "The new collector",
         "I hid on the floor of his car, under the blanket. Finse told him this is his last pickup. His anchor drifts past the dose, so they bring him in. Then they made him say it back: the new collector is in the house.",
         "His car has a back seat too."},
        // ---- The dark
        {"first_dark", "The dark", "I can't undo it",
         "I did something I can't take back. The game remembers. So do I.",
         "Some doors only open one way."},
        {"mirror_lag", "The dark", "Who's in the mirror",
         "My reflection is older now. It smiles when I don't.",
         "Look at yourself after."},
        {"traps", "The dark", "Not like the movies",
         "Marbles. Tripwire. Paint can. In the movie it's funny. In real life bones make a sound.",
         "Prepare the house for him."},
        // v0.19: before the knock. Fifty minutes to set the stage, and one thing you only hear because you did
        {"new_hinges", "The house", "New hinges",
         "I pushed my chest of drawers against my door. He needed a long time to get through. In the doorway, out of breath, he said he did that too, once. Same door. They took it off its hinges after, so he couldn't. Every door in this house has old brass hinges. Mine are new.",
         "Barricade your own door. Be close enough to hear him when he gets through."},
        {"hand_me_down", "The visitor", "Hand-me-downs",
         "I took his car key and threw it in the snow. We sat in his dead car and waited for a plough that never came. He put his coat over me. The car isn't his. Neither is the coat, or the watch. All of it belonged to the one before him. They hand it down.",
         "His engine runs while he looks in the window. What if it didn't?"},
        {"thirty", "The visitor", "Thirty kilometres",
         "I walked tracks out the back and came back in my own steps. He followed them all the way to the fence. When he got me he said he did that once, for real. They found him sitting in the snow at thirty kilometres. He couldn't make his legs do thirty-one.",
         "Give him a trail to follow. Late enough that the storm leaves it."},
        {"his_knot", "Dad", "Dad's knot",
         "I tied the tripwire across the living room doorway with the knot Dad taught me. He went down, and lay there looking at it. He said Dad taught him that knot too.",
         "Not every trap goes on the front step."},
        // v0.20 story: every ending teaches something (T-062)
        {"confirmed", "The program", "Confirmed",
         "After, his radio asked for one word. \"Collector, confirm.\" Finse doesn't need to see a face. Only to hear the word.",
         "Somebody has to stop him. Then listen to his radio."},
        {"finse_gate", "The program", "0612",
         "It took nine loops to learn the road to Finse. The gate code was 0612. Week 612. They used my week.",
         "Get all the way to the machine."},
        {"under_well", "The visitor", "Under the well",
         "He let me bring my Fliptendo. At the third dungeon he leaned over without looking and said \"under the well.\" Nobody knows that. You only know that if you played it a hundred times.",
         "Just one more game. All the way to the car."},
        {"his_dog", "Doggo", "He had one too",
         "After Doggo, the man looked back at the door too. Just once. He said he had one like him. Same spot by the door.",
         "Doggo would do anything for you. Watch the man when he does."},
        {"just_like_us", "The dark", "Just like us",
         "I stayed hidden while he made them pay. He said I'm not different. I'm just like them. Not like Mom and Dad. Like him.",
         "Stay hidden. No matter what you hear."},
        {"three_plates", "Mom", "Four chairs, three plates",
         "We sat down and he came in. Four chairs, three plates. He took two potatoes. He held his fork the way Dad does. Mom said sit down or get out, and he sat.",
         "Don't run. Sit down."},
        {"two_remember", "The visitor", "Two of us",
         "I didn't finish it. He snapped his dose in half. The next hour his car was at the gate at four with the engine off, and he never knocked. Two of us remembered.",
         "Five doors. Then don't."},
        // v0.20 story: the watch (T-064)
        {"w_beep", "The watch", "Two short, one long",
         "My watch beeps at 16:00. Two short, one long. I never set an alarm. There's no button for one. ORIGIN, model 06. I've never seen one in a shop.",
         "Lie down and look at your watch."},
        {"w_screen", "The watch", "It counts down",
         "The little screen in the basement shows a watch. Not mine. Grey, scratched, a strap that doesn't match. It isn't telling the time. It's counting down to ten to five.",
         "Look at the screens again. The small one in the corner."},
        {"w_drift", "The watch", "Drift",
         "When I opened the door for him his watch beeped. Two short, one long. He called it drift. Mine does that every day at 16:00. Maybe it's been saying drift my whole life.",
         "Be there when his watch beeps."},
        {"w_grow", "The watch", "The next size up",
         "Dad says I got a new watch every time the strap got too small. Same face. Same number on the back. You don't take them off. You grow into the next one.",
         "Show Dad the tiny watch."},
        {"w_wrist", "The watch", "Twice round",
         "I finished it and took his watch. The strap went round my wrist twice. ANCHOR 06, crooked. My thumb found the letters by itself, like it already knew the way.",
         "Same house. Take everything he has."},
        // v0.20 story: the program, 04 and 05
        {"unit_04", "The program", "Zero-four had a dog",
         "Under the static on the console: unit 04, Geilo, collected 13:10. Dog destroyed. The dog hair in his trunk wasn't Doggo's colour.",
         "Listen to the console once you've seen his trunk."},
        {"same_number", "The program", "Same number",
         "The cross out back says 05. His clipboard says 05, ticked, today. Two kids. One number. They hand the numbers down.",
         "A number out back. The same number on his clipboard."},
        // v0.20 story: early lessons (T-084)
        {"small_hands", "The dark", "Small hands",
         "I hit Dad as hard as I could. It was like hitting the generator. I pushed Mom and she moved one step. I'm twelve. Hitting doesn't work. Not on them. Not on him.",
         "Hit somebody bigger than you."},
        // v0.20 knife: what the kitchen knife teaches
        {"packing_slip", "The program", "Return to stores",
         "I slit the tape on the big present and looked in without tearing the paper. Empty. Under the tissue paper, a packing slip: FINSE SUPPLY. UNIT 06. 3 x GIFT, DECORATIVE. RETURN TO STORES AFTER COLLECTION.",
         "The tape on the big present is the clear kind. Something thin and sharp would slide right under it."},
        {"kid_door", "The house", "A door my size",
         "Somebody laced the torn bit of the old fence out back shut with baler twine, a long time ago. With the kitchen knife it's a door my size. He still doesn't fit.",
         "The torn bit of the fence out back is tied shut with old twine. Mostly."},
        {"brake_line", "The dark", "Brake line",
         "I cut the brake line under his car while he was at the window. Whoever drove it after that didn't get far. Not him. Not them. Not me in the back.",
         "His car stands empty while he's at the window. Underneath it, something thin goes to every wheel."},
        {"no_way_out", "The dark", "No way out",
         "I tried to leave the hour the other way. There is no other way. It doesn't let go. It starts again at 16:00, and I'm still the one in it.",
         "Some things aren't a way out."},
    };
    return m;
}

const std::vector<DoorInfo>& AllDoors() {
    static const std::vector<DoorInfo> d = {
        {6, 9, "door_kid", "Your door"}, {13, 9, "door_bath", "Bathroom door"}, {22, 9, "door_parents", "Mom and Dad's door"},
        {7, 12, "door_kitchen", "Kitchen door"}, {20, 12, "door_living", "Living room door"}, {13, 16, "door_kl", "Kitchen door"},
        {20, 20, "frontdoor", "Front door"}, {34, 10, "sheddoor", "Shed door"},
    };
    return d;
}
const DoorInfo* DoorById(const std::string& id) { for (auto& d : AllDoors()) if (id == d.id) return &d; return nullptr; }
const DoorInfo* DoorAt(int x, int y) { for (auto& d : AllDoors()) if (d.x == x && d.y == y) return &d; return nullptr; }

static const MemoryEntry* FindMemory(const std::string& id) {
    for (auto& e : AllMemories()) if (e.id == id) return &e;
    return nullptr;
}

// ============================================================ save
void Story::NewGame() {
    // The darkness stays. That's the point.
    loop = 0;
    memory.clear();
    achievements.clear();
    endings.clear();
    seenLines.clear();
    seenEvents.clear();
    dryLoops = 0;
    lastEnd.clear();
    seenToilet45 = false;
    prototypeEndShown = false;
    cleanStreak = 0;
    caughtSpots.clear();
}

// ============================================================ reality shift config
ShiftConfig gShiftCfg;
static std::mt19937 shiftRng(std::random_device{}());
void SeedShifts(unsigned seed) { shiftRng.seed(seed); }
static int ShiftRoll100() { return (int)(shiftRng() % 100); }

const char* ShiftName(Shift s) {
    return s == Shift::Toilet45 ? "45 minute toilet" : s == Shift::NoDoggo ? "no Doggo" : s == Shift::EarlyStorm ? "early storm" : s == Shift::Sister ? "the bathroom" :
           s == Shift::Fourth ? "the photos" : s == Shift::NextUnit ? "oh-seven" : "normal";   // v0.20 story: the last two
}

std::string ShiftConfig::Save() const {
    std::ostringstream o;
    o << "c1|" << enabled << "|" << toilet << "|" << noDoggo << "|" << chance << "|" << minGap << "|"
      << maxGap << "|" << firstLoop << "|" << toiletShare << "|" << (int)std::lround(timeMul * 10) << "|" << earlyStorm << "|" << (int)std::lround(huntScale * 10);
    return o.str();
}

void ShiftConfig::Load(const std::string& s) {
    std::vector<int> v; std::string cur;
    if (s.rfind("c1|", 0) != 0) return;
    for (size_t i = 3; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == '|') { v.push_back(std::atoi(cur.c_str())); cur.clear(); } else cur += s[i];
    }
    if (v.size() < 8) return;
    enabled = v[0]; toilet = v[1]; noDoggo = v[2];
    chance = std::clamp(v[3], 0, 100); minGap = std::clamp(v[4], 0, 30); maxGap = std::clamp(v[5], 0, 30);
    firstLoop = std::clamp(v[6], 1, 30); toiletShare = std::clamp(v[7], 0, 100);
    if (v.size() >= 9) timeMul = std::clamp(v[8], 10, 20) / 10.0f;
    if (v.size() >= 10) earlyStorm = v[9];
    if (v.size() >= 11) huntScale = std::clamp(v[10], 10, 50) / 10.0f;
}

Shift Story::RollShift() {
    ShiftConfig& c = gShiftCfg;
    if (c.forceNext != Shift::None) { Shift f = c.forceNext; c.forceNext = Shift::None; return f; }
    if (!c.enabled || loop < c.firstLoop || (!c.toilet && !c.noDoggo && !c.earlyStorm)) return Shift::None;
    if (cleanStreak < c.minGap) return Shift::None;
    bool forced = c.maxGap > 0 && cleanStreak >= std::max(c.maxGap, c.minGap);
    if (!forced && ShiftRoll100() >= c.chance) return Shift::None;
    std::vector<Shift> others;
    if (c.noDoggo) others.push_back(Shift::NoDoggo);
    if (c.earlyStorm) others.push_back(Shift::EarlyStorm);
    if (c.sister) others.push_back(Shift::Sister);
    if (c.fourth) others.push_back(Shift::Fourth);       // v0.20 story
    if (c.nextUnit) others.push_back(Shift::NextUnit);   // v0.20 story
    if (others.empty()) return Shift::Toilet45;
    Shift other = others[ShiftRoll100() % others.size()];
    if (!c.toilet) return other;
    if (!seenToilet45) return Shift::Toilet45;  // the first anomaly you meet is the toilet, so its memory is always findable
    return ShiftRoll100() < c.toiletShare ? Shift::Toilet45 : other;
}

void Story::Hold(Item it, bool quiet) {
    if (held == it) return;
    if (held != Item::None && it != Item::None && !quiet) {
        static const char* back[] = { "", "(You put the knife back in the block first.)", "(You put the key back where it was first.)",
            "(You drop the stick by the woodpile first.)", "(You put the trash bag back first.)", "(You put the dog food back in the pantry first.)",
            "(You put the flashlight back in the pantry first.)", "(You put the tape back in Dad's cabinet first.)",
            "(You put the tiny watch back in Dad's cabinet first.)" };
        Say("You", back[(int)held]);
    }
    held = it;
    hasKnife = it == Item::Knife; hasKey = it == Item::Key; hasBag = it == Item::Bag;
    hasStick = it == Item::Stick; hasFood = it == Item::DogFood; hasLight = it == Item::Flashlight;
    if (it == Item::Key) keyInDrawer = false;
    if (it == Item::Knife) knifeTakenLoop = true;
}

void Story::DebugSetDark(int v) {
    darkPoints = std::clamp(v, 0, DARK_MAX);
    tainted = darkPoints >= DARK_TAINT;   // debug only: real players can never go back
}
void Story::DebugAllMemories() { for (auto& m : AllMemories()) memory.insert(m.id); }
void Story::DebugClearMemories() { memory.clear(); }
void Story::DebugToBasement() { Hold(Item::Tape, true); Choose("b_down", 6, 38); }

std::string Story::SaveString() const {
    std::ostringstream o;
    o << "v4|" << loop << "|" << (seenToilet45 ? 1 : 0) << "|" << (prototypeEndShown ? 1 : 0) << "|"
      << darkPoints << "|" << (tainted ? 1 : 0) << "|";
    for (auto& m : memory) o << m << ",";
    o << "|";
    for (auto& a : achievements) o << a << ",";
    o << "|";
    for (auto& e : endings) o << e << ",";
    o << "|" << cleanStreak << "|";
    for (unsigned h : seenLines) o << std::hex << h << std::dec << ",";
    // v0.10: fields 11 to 13. Older builds stop reading at 10
    o << "|";
    for (auto& e : seenEvents) o << e << ",";
    o << "|" << dryLoops << "|" << lastEnd;
    // v0.12: field 14, the spots he has found you in (id:count)
    o << "|";
    for (auto& c : caughtSpots) o << c.first << ":" << c.second << ",";
    return o.str();
}

void Story::LoadSave(const std::string& s) {
    loop = 0; memory.clear(); achievements.clear(); endings.clear();
    seenToilet45 = prototypeEndShown = false;
    std::vector<std::string> parts;
    std::string cur;
    for (char c : s) { if (c == '|') { parts.push_back(cur); cur.clear(); } else cur += c; }
    parts.push_back(cur);
    auto split = [](const std::string& in, std::set<std::string>& out) {
        std::string c;
        for (char ch : in) { if (ch == ',') { if (!c.empty()) out.insert(c); c.clear(); } else c += ch; }
        if (!c.empty()) out.insert(c);
    };
    if (parts.size() >= 6 && parts[0] == "v1") {
        loop = std::atoi(parts[1].c_str());
        seenToilet45 = parts[2] == "1";
        split(parts[4], memory);
        split(parts[5], achievements);
    } else if (parts.size() >= 9 && (parts[0] == "v2" || parts[0] == "v3" || parts[0] == "v4")) {
        loop = std::atoi(parts[1].c_str());
        seenToilet45 = parts[2] == "1";
        prototypeEndShown = parts[3] == "1";
        darkPoints = std::max(darkPoints, std::atoi(parts[4].c_str()));
        tainted = tainted || parts[5] == "1";
        if (parts[0] == "v2") tainted = darkPoints >= DARK_TAINT; // v0.3 rules: small stuff no longer taints you
        darkPoints = std::min(darkPoints, DARK_MAX);
        split(parts[6], memory);
        split(parts[7], achievements);
        split(parts[8], endings);
        cleanStreak = parts.size() >= 10 ? std::atoi(parts[9].c_str()) : 0;
        seenLines.clear();
        if (parts.size() >= 11) {
            std::set<std::string> hs; split(parts[10], hs);
            for (auto& h : hs) seenLines.insert((unsigned)std::strtoul(h.c_str(), nullptr, 16));
        }
        seenEvents.clear(); dryLoops = 0; lastEnd.clear();
        if (parts.size() >= 12) split(parts[11], seenEvents);
        if (parts.size() >= 13) dryLoops = std::atoi(parts[12].c_str());
        if (parts.size() >= 14) lastEnd = parts[13];
        caughtSpots.clear();
        if (parts.size() >= 15) {
            std::set<std::string> cs; split(parts[14], cs);
            for (auto& c : cs) { size_t k = c.find(':'); if (k != std::string::npos) caughtSpots[c.substr(0, k)] = std::max(1, std::atoi(c.c_str() + k + 1)); }
        }
    }
}

// ============================================================ helpers
// v0.9: lines you have read before can be skipped, including the black screens
static unsigned LineHash(const std::string& t) { unsigned h = 2166136261u; for (unsigned char c : t) h = (h ^ c) * 16777619u; return h; }
bool Story::SeenLine(const std::string& text) const { return seenLines.count(LineHash(text)) > 0; }
void Story::MarkSeen(const std::string& text) { seenLines.insert(LineHash(text)); }

float Story::Rand() {
    rng = std::fmod(rng * 9301.0f + 49297.0f * 0.0001f + 0.618f, 1.0f);
    return rng;
}

void Story::Say(const std::string& who, const std::string& text) {
    Beat b; b.type = BeatType::Say; b.text = text;
    b.speaker = who == "Mom" ? MomName() : who;   // v0.9: after the tape she isn't Mom any more
    beats.push_back(b);
    // Quantum instability: the extras don't know their lines. Never in the serious scenes
    if (Stage() >= 2 && who != "" && who != "You" && who != "The visitor" && !collection && !solemn &&
        momAtt == Attitude::Normal && dadAtt == Attitude::Normal && Rand() < 0.12f) beats.push_back(b);
}
void Story::Bubble(const std::string& who, const std::string& text) {
    Beat b; b.type = BeatType::Bubble; b.speaker = who; b.text = text;
    beats.push_back(b);
}
void Story::Narrate(const std::string& text) { Beat b; b.type = BeatType::Narrate; b.text = text; beats.push_back(b); }
void Story::Black(const std::string& text) { Beat b; b.type = BeatType::Black; b.text = text; beats.push_back(b); }
void Story::FadeFor(int minutes, const std::string& label, int tx, int ty) {
    Beat b; b.type = BeatType::Fade; b.minutes = minutes; b.text = label; b.tx = tx; b.ty = ty;
    beats.push_back(b);
}
void Story::Sound(const std::string& id) { Beat b; b.type = BeatType::Sound; b.id = id; beats.push_back(b); }
void Story::Flash() { Beat b; b.type = BeatType::Flash; beats.push_back(b); }
void Story::SceneBeat(const std::string& id) {
    Beat b; b.type = BeatType::Scene; b.id = id; beats.push_back(b);
    if (id == "chase") chase = true;
}
void Story::Achieve(const std::string& name) {
    if (achievements.count(name)) return;
    achievements.insert(name);
    Beat b; b.type = BeatType::Achievement; b.text = name; beats.push_back(b);
}
void Story::AddClue(const std::string& id) {
    if (memory.count(id)) return;
    memory.insert(id);
    loopNew.push_back(id);
    Beat b; b.type = BeatType::Clue; b.id = id;
    if (auto* e = FindMemory(id)) b.text = e->title;
    beats.push_back(b);
    CheckHandlers();
    if (!prototypeEndShown && FullMemory()) {
        prototypeEndShown = true;
        Achieve("Total Recall");
        Narrate("I knew everything now. Every line. Every lie. Even how to stay awake in the trunk.");
    }
}
void Story::Choice(const std::string& prompt, std::vector<ChoiceOpt> opts) {
    Beat b; b.type = BeatType::Choice; b.text = prompt; b.choices = std::move(opts);
    beats.push_back(b);
}
bool Story::Once(const std::string& id) {
    if (firedEvents.count(id)) return false;
    firedEvents.insert(id);
    return true;
}

void Story::CheckHandlers() {
    if (memory.count("handlers")) return;
    bool a = Knows("reports") || Knows("book_notes");
    bool b = Knows("phone_mom") || Knows("phone_answer") || Knows("calendar");
    bool c = Knows("dad_radio") || Knows("basement_screens");
    if (a && b && c) {
        memory.insert("handlers");
        Beat cl; cl.type = BeatType::Clue; cl.id = "handlers";
        cl.text = FindMemory("handlers")->title;
        beats.push_back(cl);
        Narrate("That was the day I stopped calling them Mom and Dad. In my head, anyway.");
        Narrate("Something in the house noticed too.");
    }
}

int Story::Stage() const {
    if (tainted) return Knows("basement_files") ? 4 : 3;
    if (loop <= 1) return 0;
    if (Knows("handlers")) return 2;
    return 1;
}

int Story::Act() const {
    if (tainted) return 3;
    // v0.5.1: act 2 (eerie music, flux) starts at the real act break, finding out about the handlers.
    // Only players who keep looping without finding it get it anyway, from loop 8.
    if (Knows("handlers") || loop >= 8) return 2;
    return 1;
}

bool Story::DoorLocked(int x, int y) const {
    if (Barricaded(x, y)) return true;   // v0.19: keys don't help against a chest of drawers
    if (x == 6 && y == 9 && grounded && !collection) return true;
    if (x == 20 && y == 20 && frontLocked && !collection) return true;
    return lockedDoors.count(y * World::W + x) > 0;
}
bool Story::DoorClosed(int x, int y) const {
    return DoorLocked(x, y) || closedDoors.count(y * World::W + x) > 0;
}
void Story::OpenDoor(int x, int y) {
    int k = y * World::W + x;
    if (barricades.count(k)) return;   // v0.19: only he gets through that, and not with the handle
    closedDoors.erase(k); lockedDoors.erase(k);
    if (x == 20 && y == 20) frontLocked = false;
}
void Story::BreakDoor(int x, int y) { barricades.erase(y * World::W + x); OpenDoor(x, y); brokenDoors.insert(y * World::W + x); }

// ============================================================ v0.19: before the knock (branches map, section 4)
// The bedroom-row doors can be barricaded from the room side. The room is the one north of the door
Room Story::BarricadeRoom(int x, int y) {
    if (y == 9 && (x == 6 || x == 13 || x == 22)) return x == 6 ? Room::KidsRoom : x == 13 ? Room::Bathroom : Room::ParentsRoom;
    return Room::Outside;
}
void Story::InnerTrapDoor(const std::string& trap, int& x, int& y) {
    if (trap == "marbles") { x = 6; y = 9; }        // your door
    else if (trap == "tripwire") { x = 20; y = 12; } // the living room door. He comes in that way, every time
    else { x = 34; y = 10; }                          // the paint can, over the shed door
}
std::string Story::InnerTrapAt(int x, int y) const {
    for (auto& tr : innerTraps) { int dx, dy; InnerTrapDoor(tr, dx, dy); if (dx == x && dy == y) return tr; }
    return "";
}
void Story::OnBarricadeHit(int x, int y) {
    if (!chase) return;
    Room r = BarricadeRoom(x, y);
    Bubble("The visitor", r == Room::KidsRoom ? "(the handle turns. The door doesn't move) ...Furniture?" : "(he leans on the door. Something heavy leans back) Huh.");
}
void Story::OnBarricadeBroken(int x, int y, bool kidNear) {
    if (!chase) return;
    Achieve("Not Through Here");
    Room r = BarricadeRoom(x, y);
    if (r == Room::KidsRoom && kidNear) {
        // the one thing you only hear because you made him work for it
        Bubble("The visitor", "(in the doorway, out of breath) Same door. I did that too. They took it off its hinges after.");
        AddClue("new_hinges");
        StageCheck();
    } else Bubble("The visitor", kidNear ? "(out of breath) ...Good. Make me work for it." : "(somewhere in the house, wood gives)");
}
void Story::OnInnerTrap(const std::string& trap, bool kidNear) {
    if (!chase || !innerTraps.count(trap)) return;
    innerTraps.erase(trap);
    if (trap != "paint") visitorInjured = true;   // the paint only blinds him
    Achieve("House Rules");
    if (trap == "marbles") Bubble("The visitor", "(flat on his back in your doorway) ...Marbles. Indoors. That's just rude.");
    else if (trap == "paint") Bubble("The visitor", "(red to the elbows, wiping his eyes) Red. Of course it's red.");
    else if (kidNear) {
        Bubble("The visitor", "(from the floor, looking at the knot, not at you) That's his knot. Your dad's. He taught me that one.");
        AddClue("his_knot");
        StageCheck();
    } else Bubble("The visitor", "(from the floor) ...Hm. Good knot.");
}
void Story::OnFalseTrail(bool arrived) {
    if (!chase) return;
    if (!arrived) { Bubble("The visitor", "(he looks at the snow out back for a long time) ...Tracks. Out the back."); return; }
    falseTrailWalked = true;
    Bubble("The visitor", "(at the fence, his torch on the vidda) Out. And back in your own steps.");
    Achieve("Breadcrumbs");
}
void Story::OnBarricadeTold(const std::string& who) {
    if (!chase) return;
    Bubble(who, who == "Mom" ? "(rattling your door) He's put something against it! He's IN here!" : "(shoulder against the door) He's barricaded himself in. In here!");
}
void Story::Thirty() {
    // he followed your fake trail all the way out, and it cost him
    if (!falseTrailWalked || Knows("thirty")) return;
    Say("The visitor", "(still out of breath from the yard) Out the back, and back in your own steps. That's a good one.");
    Say("The visitor", "I did that once. For real. No coming back.");
    Say("The visitor", "They found me sitting in the snow at thirty kilometres. I couldn't make my legs do thirty-one.");
    Say("You", Knows("leash") ? "(Thirty-one. The leash. He got as far as it goes, minus one.)" : "(Thirty-one. What's at thirty-one?)");
    AddClue("thirty");
    StageCheck();
}
void Story::StageCheck() {
    int n = 0;
    for (const char* id : {"new_hinges", "hand_me_down", "thirty", "his_knot"}) n += Knows(id) ? 1 : 0;
    if (n >= 4 && !achievements.count("Stage Manager")) {
        Achieve("Stage Manager");
        if (!chase) Narrate("I used to spend the fifty minutes waiting for him. Then I spent them moving the furniture.");   // not in the middle of the hunt
    }
}
void Story::NoKeyEnding() {
    // the key is somewhere in a drift by the gate. Nobody drives anywhere tonight
    Black("He carries you down to his car and opens the door. Then he stops.");
    Black("He pats his coat. He looks out at the snow by the gate, where you threw it. The storm is already filling in the hole.");
    Black("He puts you in the passenger seat and sits down behind the wheel. He doesn't even try the engine.");
    if (brakesCut) Black("He never finds out about the brakes. You don't tell him.");   // v0.20 knife
    Black("\"Collector to Finse. No key.\"");
    Black("A long hiss. \"...Then you sit with him until the plough comes. Keep him warm. We need him warm.\"");
    Black("He takes his coat off and puts it over you. It's heavy, and it smells like the car.");
    Black("The cuffs have been turned up and let down so many times there's a line in the cloth.");
    Black("\"It's not mine, you know. The car. The coat. The watch. All of it was his. The one before me.\"");
    Black("\"They hand it down. Like a big brother's jacket. It doesn't fit at first.\"");
    Black("\"Who gets it after you?\"");
    Black("He doesn't answer. He looks at you. That's the answer.");
    if (Knows("piece_car")) Black("The new collector is in the house. He's sitting in the car, under the coat.");
    AddClue("hand_me_down");
    StageCheck();
    Sound("bells");
    Black("Far away, through the storm: bells. The plough never comes.");
    See("bells");
    lastEnd = "nokey";
    Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
}

bool Story::Befriended() const {
    // v0.7: every chore done and nothing dark this loop. Then they'll fight for you.
    return !Tutorial() && ChoresDone() == CHORES && darkThisLoop == 0 && momAtt == Attitude::Normal &&
           dadAtt == Attitude::Normal && !grounded && !momLocked && !dadLocked;
}
Stance Story::CurrentStance() const {
    if (Befriended()) return Stance::Befriended;
    if (tainted || darkThisLoop > 0 || momAtt == Attitude::Scared || dadAtt == Attitude::Scared || dadAtt == Attitude::Broken) return Stance::Dark;
    return Stance::Normal;
}

std::string Story::ClockTextSec() const {
    int sec = (int)t;
    int hh = 16 + sec / 3600, mm = (sec / 60) % 60, ss = sec % 60;
    char buf[16];
    std::snprintf(buf, sizeof buf, "%02d:%02d:%02d", hh, mm, ss);
    return buf;
}

std::string Story::ClockText() const {
    int m = (int)t / 60;
    int hh = 16 + m / 60, mm = m % 60;
    char buf[16];
    std::snprintf(buf, sizeof buf, "%02d:%02d", hh, mm);
    return buf;
}

void Story::ChangeMood(int d) {
    mood += d;
    if (mood <= -3 && !grounded && !collection) Ground();
}

void Story::Ground() {
    grounded = true;
    if (momAtt == Attitude::Normal) momAtt = Attitude::Cold;
    if (tainted) Say("Mom", "(she won't come closer) Go to your room. Please. Just... go.");
    else Say("Mom", "That's it. Room. Now. You're not coming out until I say so.");
    Achieve("Grounded");
    FadeFor(1, "Sent to your room", 5, 6);
    inBasement = false;
    Say("You", "(The door is locked from the outside.)");
    if (Stage() >= 1) Say("You", "(Locked in until he comes. Great plan, me.)");
}

void Story::DarkAct(int points, const std::string& line, bool quiet) {
    int before = darkPoints;
    darkPoints = std::min(DARK_MAX, darkPoints + points);
    darkThisLoop += points;
    if (!line.empty()) Say("", line);
    if (quiet && (tainted || darkPoints < DARK_TAINT)) {
        // v0.7: the caller says something that fits better
    } else if (!tainted && darkPoints < DARK_TAINT) {
        // Still just a weird kid. For now.
        static const char* weird[] = {
            "(That was a weird thing to do. Right?)",
            "(I'm just being careful. That's all.)",
            "(Nobody needs to know about that.)",
        };
        Say("You", weird[darkPoints % 3]);
    } else if (!tainted) {
        tainted = true;
        AddClue("first_dark");
        Achieve("Point of No Return");
        Say("You", "(I didn't mean to. I didn't. I didn't.)");
        Say("You", "(My hands won't stop shaking.)");
        Narrate("Something in me stopped shaking that day. It never started again.");
        Narrate("You can start a new game if you want. It won't help. I'll remember.");
    } else if (darkPoints >= DARK_REVEL) {
        static const char* rev[] = {"(That was easy.)", "(I could do that again.)", "(Why did I ever cry about this.)"};
        Say("You", rev[darkPoints % 3]);
    } else {
        Say("You", "(Okay.)");
    }
    if (before < DARK_ARSENAL && darkPoints >= DARK_ARSENAL)
        Narrate("I started looking at the house differently. Stairs. Doorways. Things that could fall.");
    if (before < DARK_INTERROGATE && darkPoints >= DARK_INTERROGATE)
        Narrate("I knew Dad would talk if I asked the right way. I knew what the right way was.");
}

bool Story::TutorialLocked(const std::string& id) const {
    if (!Tutorial()) return false;
    static const std::set<std::string> open = {
        "bed", "toys", "window", "toilet", "mirror", "fridge", "stove", "trashcan", "bowl",
        "tv", "couch", "radio", "bins", "doghouse", "gate", "flag", "mom", "dad", "doggo", "woodstove",
        "calendar", "bookshelf", "album", "wardrobe", "woodpile", "marbles", "frontdoor", "visitor",
        "pantry", "sink", "fliptendo", "tree", "car",
        "door_kid", "door_bath", "door_parents", "door_kitchen", "door_living", "door_kl", "sheddoor",
        "win_bath", "win_parents", "win_kitchen", "win_living", "win_living2", "beast", "swing", "spruce"};
    return open.count(id) == 0;
}

bool Story::IsHideSpot(const std::string& id) {
    // v0.12: five more. The yard ones are cold
    return id == "wardrobe" || id == "bed" || id == "pbed" || id == "toolbox" || id == "couch" ||
           id == "pantry" || id == "tree" || id == "woodpile" || id == "doghouse" || id == "bins" || id == "crates" ||
           id == "oldcar" || id == "beast" || id == "spruce";   // v0.15: out back
}
bool Story::ShouldHide(const std::string& id) const {
    if (!chase) return false;
    return IsHideSpot(id);
}

// ============================================================ npcs
NpcState Story::Mom() const {
    NpcState n;
    int m = (int)(t / 60);
    if (momLocked) { n.x = 12; n.y = 39; n.visible = inBasement; return n; }
    if (momBathLocked) { n.x = 13; n.y = 5; return n; }   // v0.13: hinge A. She stays where you left her
    if (MomInBath()) { n.x = 13; n.y = 6; return n; }
    if (collection && dadOnFloor && stance == Stance::Befriended && !momOnFloor) { n.x = 19; n.y = 17; return n; }   // on her knees next to him
    if (collection) { n.x = 17; n.y = 15; return n; }
    int phone = Stage() >= 2 ? 21 : 22;
    if (t >= phone * 60 - 30 && t < (phone + 2) * 60) { n.x = 4; n.y = 15; return n; }
    if (t >= 28 * 60 && t < 30 * 60) { n.x = 27; n.y = 10; return n; }
    if (t >= 30 * 60 && t < 33 * 60) { n.x = 8; n.y = 40; n.visible = inBasement; return n; }
    if (Stage() >= 2 && ((m >= 12 && m < 18) || (m >= 36 && m < 40))) {
        n.x = 3; n.y = 19; n.facingWall = true; return n; // standing in the corner
    }
    n.x = 8; n.y = 16;
    return n;
}

NpcState Story::Dad() const {
    NpcState n;
    if (dadOnFloor) { n.x = stance == Stance::Befriended ? 20 : 18; n.y = stance == Stance::Befriended ? 17 : 17; return n; }
    if (dadDown) { n.visible = false; n.x = 20; n.y = 26; return n; }
    if (dadLocked) { n.x = 35; n.y = 8; return n; }
    if (collection) { n.x = 18; n.y = 17; return n; }
    if (t < 8 * 60) { n.x = 24; n.y = 16; return n; }
    if (t >= 36 * 60 && t < 38 * 60) { n.x = 28; n.y = 25; return n; }   // v0.16: an armful of firewood before the storm
    if (t < 40 * 60) { n.x = 35; n.y = 8; return n; }
    // v0.10: every Christmas Eve at 16:44 he checks that the key is still under the flagpole
    if (t >= 43 * 60 + 30 && t < 45 * 60 + 30 && dadAtt != Attitude::Broken && dadAtt != Attitude::Scared) { n.x = 11; n.y = 26; return n; }
    n.x = 10; n.y = 17;
    return n;
}

NpcState Story::Visitor() const {
    NpcState n;
    n.visible = CarHere();
    if (collection) {
        if (frontLocked) { n.x = 25; n.y = 18; }
        else if (doorGreeted) { n.x = 20; n.y = 21; }   // v0.14: hinge D. On the step, you in the doorway
        else { n.x = 20; n.y = 19; }
        return n;
    }
    // v0.9.2: his last minutes, counted back from the knock
    int left = visitorTime - (int)t;
    if (left > VISIT_WALK) { n.x = 20; n.y = 30; }                                   // getting out of the car
    else if (left > VISIT_BACK) { n.x = 25; n.y = 21; n.facingWall = true; }         // at the living room window
    else if (left > VISIT_IN_CAR) { n.x = 20; n.y = 30; }                            // back to the car
    else if (left > VISIT_OUT) { n.x = 20; n.y = 30; n.visible = false; }            // inside it
    else { n.x = 20; n.y = 21; }                                                     // the front step
    return n;
}

// v0.9.2: he has seen you. No reason to wait any longer
void Story::VisitorNoticesYou() {
    if (collection || visitorTime - (int)t <= VISIT_BACK) return;
    visitorTime = (int)t + VISIT_BACK;
    Say("", "(He looks at you a moment longer. Then he checks his watch and walks back to his car.)");
}

// ============================================================ v0.10 queries
bool Story::KnowsEvent(const std::string& id) const {
    if (seenEvents.count(id)) return true;
    for (auto& e : AllTimeline()) if (e.id == id) return !e.memory.empty() && Knows(e.memory);
    return false;
}
bool Story::CarEmpty() const {
    int left = visitorTime - (int)t;
    return !collection && left <= VISIT_WALK - 20 && left > VISIT_BACK;
}
bool Story::CanGreet() const {
    // v0.14: hinge D. From the moment he sits back down in his car until he knocks
    return !collection && !Tutorial() && Knows("visitor_time") && !frontLocked && visitorTime - (int)t <= VISIT_IN_CAR;
}
bool Story::Spared() const {
    // He follows the last orders he got. Without the 16:22 call that is "pickup". Unless you give him a reason
    return phoneUnplugged && !visitorInjured && threats == 0 && darkThisLoop == 0 && !dadDown && !doggoDead &&
           !kidShot && !momLocked && !dadLocked;
}
bool Story::MomHums() const {
    if (collection || momLocked || tainted || momRemorse || momAtt != Attitude::Normal) return false;
    int phone = Stage() >= 2 ? 21 : 22;
    if (t >= phone * 60 - 30 && t < (phone + 2) * 60) return false;
    if (t >= 28 * 60 && t < 33 * 60) return false;
    if (MomInBath()) return false;
    return !Mom().facingWall;
}

// v0.13: "in the hallway", for door choices that say where you end up
static std::string RoomPhrase(Room r) {
    switch (r) {
        case Room::KidsRoom: return "in my room";
        case Room::Bathroom: return "in the bathroom";
        case Room::ParentsRoom: return "in Mom and Dad's room";
        case Room::Hallway: return "in the hallway";
        case Room::Kitchen: return "in the kitchen";
        case Room::LivingRoom: return "in the living room";
        case Room::Shed: return "in the shed";
        case Room::Basement: return "in the basement";
        default: return "out in the yard";
    }
}
static std::string IntoPhrase(Room r) {
    switch (r) {
        case Room::KidsRoom: return "into my room";
        case Room::Bathroom: return "into the bathroom";
        case Room::ParentsRoom: return "into Mom and Dad's room";
        case Room::Hallway: return "out into the hallway";
        case Room::Kitchen: return "into the kitchen";
        case Room::LivingRoom: return "into the living room";
        case Room::Shed: return "into the shed";
        default: return "out into the yard";
    }
}

// v0.13: hinge A. Every loop at 16:46 she goes to the bathroom for two and a half minutes
bool Story::MomInBath() const {
    if (collection || momLocked || shift == Shift::Sister || Tutorial()) return false;
    return t >= 46 * 60 && t < 48 * 60 + 30;
}
bool Story::BathLockIsDark() const {
    return tainted || momAtt == Attitude::Scared || momAtt == Attitude::Broken || held == Item::Knife || knifeTakenLoop;
}
// v0.13: which side of a door you are on. Doors sit in a wall, so one room is on each side of it
bool Story::DoorSides(int dx, int dy, int px, int py, Room& here, Room& there, int& ox, int& oy) const {
    // a door in a wall that runs left to right connects the rooms above and below it
    bool vertical = world.tiles[dy][dx - 1] == Tile::Wall || world.tiles[dy][dx + 1] == Tile::Wall;
    int ax, ay, bx, by;
    if (vertical) { ax = dx; ay = dy - 1; bx = dx; by = dy + 1; } else { ax = dx - 1; ay = dy; bx = dx + 1; by = dy; }
    bool onA = vertical ? py < dy : px < dx;
    here = world.RoomAt(onA ? ax : bx, onA ? ay : by);
    there = world.RoomAt(onA ? bx : ax, onA ? by : ay);
    ox = onA ? bx : ax; oy = onA ? by : ay;
    return world.Walkable(ox, oy, false);
}

// The narrator picks up where the last loop left off. Otherwise a line for the stage he is in
void Story::LoopOpener(const std::string& how) {
    const char* r = nullptr;
    bool alt = loop % 2 == 0;
    if (how == "cold") r = alt ? "I could still feel the snow in my fingers. It wasn't there. It had been." : "A warm bed. I didn't trust warm any more.";
    else if (how == "toilet") r = alt ? "We are not talking about the last one." : "Moving on.";
    else if (how == "switch") r = alt ? "The place behind my ear still felt hot. I didn't touch it." : "I had killed a man an hour ago. An hour from now. It was getting hard to keep straight.";
    else if (how == "vidda") r = "My legs remembered the snow. All 31 kilometres I never got through.";
    else if (how == "gone") r = alt ? "He saw me in his mirror and let me sit there. I still don't know which of us that was for." : "Under a blanket in his car, the whole hour. It still ended. It always ends.";
    else if (how == "doggo" || how == "ending:doggo") r = DoggoExists() ? "Doggo was asleep on my feet. Breathing. I lay still for a while and let him." : "I listened for Doggo. Nothing. Not in this one.";
    else if (how == "spared") r = "They were alive when he carried me out. First time. I held on to that.";
    else if (how == "express") r = "I had climbed into his trunk by myself. I'm not proud of it. It was a little bit funny.";
    else if (how == "dadran") r = "Dad had gone at him with a lamp. For me. He wouldn't remember it. I would.";
    else if (how == "name") r = "She had said my name. I lay there trying to get it back. It was like trying to remember a smell.";
    else if (how == "ending:escape") r = "31 kilometres. I had counted every one of them from the back seat.";
    else if (how == "ending:bells") r = "I could still hear the bells. From inside the house. That one time.";
    else if (how == "ending:monster" || how == "ending:program") r = "A new 16:00. The same hands.";
    else if (how == "ending:screen") r = "I had spent a whole hour of my life on a paper plane. Well. The same hour.";
    else if (how == "ending:paid") r = "I had let them pay for me. The house didn't know. I did.";
    else if (how == "tied") r = alt ? "My fingers still remembered the knots. They hadn't held. Nothing holds past 17:00." : "I had tied him up. He told me riddles. I could still hear one of them and I didn't like it.";
    else if (how == "ending:samehouse") r = alt ? "16:00. I checked my hands first. Small. For now." : "My bed. My watch. Somewhere a coat that would fit me one day.";
    else if (how == "ending:after") r = alt ? "Whatever he remembered, he would have to forget it again by 16:43." : "I looked at the gate first thing. No car yet. He didn't know me yet. Again.";
    // v0.20 knife
    else if (how == "brakes") r = alt ? "His car wasn't at the gate yet. Its brakes still worked. I lay there knowing that." : "I could still feel the snow under his car, through my sweater.";
    else if (how == "mirror") r = "16:00. The watch. The bed. Ribbe downstairs. I lay there a long time. Then Mom called my name, and I answered.";
    if (r) { Narrate(r); return; }
    static const char* s1[] = {
        "16:00. Same bed. Same watch. Same me. Mostly.",
        "I stopped counting the loops. The watch didn't.",
        "Again. Fine. Let's see what I missed.",
        "16:00. I knew that ceiling better than my own face.",
        "One hour. I had all the time in the world, one hour at a time.",
        "The watch beeped. Downstairs, ribbe. Outside, a car that wasn't here yet.",
    };
    static const char* s2[] = {
        "They were never my parents. I just didn't know it yet. Well. Now I did.",
        "Same hour. Same lies. The house was starting to forget its lines.",
        "I knew the script better than they did now. That should have scared me more.",
        "Handler one. Handler two. Unit zero six. I said it under my breath until it stopped sounding like us.",
        "They would smile at me in a minute. I would smile back. We were all very good at it.",
        "16:00. Fifty minutes of pretending. I was getting better at it than they were.",
    };
    static const char* s3[] = {
        "I didn't correct myself anymore. Why would I.",
        "16:00. They'd be scared of me again by 16:05. Fine.",
        "The watch beeped. I didn't flinch. I used to flinch.",
        "I got up. I didn't bother being quiet on the stairs any more.",
        "The house knew what I was now. Only the people in it were behind.",
        "16:00. I checked my hands. Clean. That never lasted.",
    };
    static const char* s4[] = {
        "They locked me in a chair for what I'd become. So I became it. Now we're even.",
        "I knew what was in the basement. I knew what was in me. Same thing.",
        "Every loop the house got quieter around me. Like it was holding its breath.",
        "I knew where the chair was. I knew whose voice was on the tape. I got up anyway.",
        "There was nothing left in that house I hadn't opened. Including them.",
        "16:00. The basement hummed under my bed like it was glad to see me.",
    };
    const char** pool = Stage() >= 4 ? s4 : Stage() >= 3 ? s3 : Stage() >= 2 ? s2 : s1;
    Narrate(pool[(loop * 5 + (int)memory.size()) % 6]);
}

// Three loops with nothing new: the narrator points at the next thing worth finding
void Story::Nudge() {
    static const char* order[] = { "calendar", "no_car", "no_baby", "book_notes", "presents", "phone_mom", "reports", "dad_radio",
        "basement_open", "basement_screens", "phone_answer", "dad_goodbye", "prints", "his_car", "trunk", "doggo_basement", "doggo_out",
        "hiding", "he_runs_fast", "told_mom", "told_dad", "doggo_gate", "echo", "off_hook", "dad_key", "basement_files" };
    const MemoryEntry* pick = nullptr;
    for (const char* id : order) if (!Knows(id)) { pick = FindMemory(id); break; }
    if (!pick) for (auto& m : AllMemories()) if (!Knows(m.id) && !QuietMemory(m.id)) { pick = &m; break; }   // v0.20 knife: never that one
    if (!pick) return;
    Narrate("I was going in circles. Small ones, inside the big one.");
    Say("You", "(A thought that won't go away. " + pick->hint + ")");
    dryLoops = 0;
}

// ============================================================ loop
void Story::BeginLoop() {
    loop += 1;
    t = 0;
    mood = 0;
    fed = hasBag = trashDone = roomClean = pooped = false;
    allowanceChecked = grounded = collection = false;
    phoneAnswered = helpedDad = false;
    hasKnife = hasKey = frontLocked = momLocked = dadLocked = false;
    held = Item::None; hasStick = hasFood = hasLight = false;
    dishesDone = firewoodDone = earnedPresent = zuldaOpened = knifeTakenLoop = lightUsed = false;
    playStreak = 0; visitorStun = 0;
    hasCoat = openedEarly = false; cold = 0;
    lastLoopNew = loopNew; loopNew.clear();
    doggoGone = doggoOut = dadDown = inBasement = false;
    traps = 0; trapsSet.clear(); flagTouches = 0;
    chase = visitorInjured = visitorDowned = kidShot = switchTaken = false;
    closedDoors.clear(); lockedDoors.clear(); brokenDoors.clear();
    basementUnlocked = drawerOpen = keyInDrawer = false;
    doggoStay = doggoDead = false; doggoDeadX = doggoDeadY = -1;
    visitorLooked = false;
    darkThisLoop = 0; parentsAllied = false; stance = Stance::Normal; threats = 0;
    dadOnFloor = momOnFloor = kidHidden = doggoWithKid = false;
    momAtt = dadAtt = Attitude::Normal;
    pendingKnifeAct.clear(); knifeChecks = 0;
    hideSpot.clear(); pebbles = 0; decoys = 0;
    phoneUnplugged = momRemorse = sisterNoticed = echoAsked = false;
    momBathLocked = bathLockDark = false; huntStart.clear(); momRoomNow = Room::Kitchen;
    doorGreeted = carHidden = false;
    windowBanged = tongueStuck = backyardCold = false;
    asked.clear(); nagMom = nagDad = 0; dadJokes = 0;
    toldPieces.clear(); remembered = visitorTied = false; tiedTalk = 0;
    momTurned = dadTurned = momFought = dadFought = hasTaser = visitorDead = handlers = false;
    dropItem.clear(); dropX = dropY = -1;
    barricades.clear(); innerTraps.clear(); carKey.clear();
    falseTracksAt = 0; falseTracks = falseTrailWalked = false;
    // v0.20 story: the TV wants to know how the last one ended
    prevEnd = lastEnd; predMom = predDad = 0; fourthNoticed = unitNoticed = false;
    tapeCut = doggoTangled = gapCut = phoneCut = brakesCut = false;   // v0.20 knife
    std::string how = lastEnd; lastEnd.clear();
    visitorTime = 50 * 60;
    firedEvents.clear();
    rng = std::fmod(0.37f + loop * 0.1618f, 1.0f);

    // Reality shifts
    shift = RollShift();
    if (shift == Shift::None) ++cleanStreak; else cleanStreak = 0;

    Sound("beep");
    if (loop == 1 && !tainted) {
        Narrate("I was twelve. It was Christmas Eve, up on Hardangervidda.");
        Narrate("I remember the watch beeping. I remember everything after that, too. That's the problem.");
        Say("Mom", "Kiddo! Get down here, I need you!");
        Say("You", "(Arrows or WASD to walk. E or Space to use things. Hold T to wait.)");
        Say("You", "(Christmas Eve chores. If I do all of them, I get to open a present before dinner. Please let it be Zulda: Flute of Time!)");
    } else if (loop == 1 && tainted) {
        Narrate("New game. Cute.");
        Narrate("I was twelve. It was Christmas Eve. And I remember what I did. Every version of it.");
        Say("Mom", "(from downstairs, unsure) ...Kiddo? You up?");
    } else if (loop == 2 && !tainted) {
        Narrate("The watch beeped. 16:00. Christmas Eve. Again.");
        Narrate("I was in my bed. I remembered all of it. The knock. The trunk. Nobody else did.");
        Say("You", "(Everything is open now. J opens your memory log. Things you remember unlock new choices.)");
        Say("You", "(Knowing stuff lets you skip lines. Press Tab during talking.)");
    } else {
        LoopOpener(how);
    }
    if (tainted && loop >= 2) Say("Mom", "(from the kitchen) ...Is that you? You're up. Good. Good.");

    if (shift == Shift::Toilet45) Say("You", "(My stomach just made a noise like a dying whale.)");
    if (shift == Shift::NoDoggo) Say("You", "(It's quiet. Too quiet. Something's missing.)");
    if (shift == Shift::EarlyStorm) Say("You", "(The wind is already up. That's not how today goes.)");
    if (shift == Shift::Sister) {
        // v0.10: somebody is in the bathroom. She stays in there all hour
        closedDoors.insert(9 * World::W + 13); lockedDoors.insert(9 * World::W + 13);
        Say("You", "(Somebody is singing in the bathroom. Nobody in this house sings.)");
    }
    ShiftOpener();   // v0.20 story: the photos, oh-seven
    if (loop >= 4 && dryLoops >= 3 && !FullMemory()) Nudge();
}

void Story::EndLoop() {
    beats.clear();
    AddClue("loop");
    if (!tainted && darkPoints >= DARK_TAINT) {
        // crossed the line during the collection
        tainted = true;
        AddClue("first_dark");
        Achieve("Point of No Return");
    }
    if (loopNew.empty()) ++dryLoops; else dryLoops = 0;
    BeginLoop();
}

// ============================================================ time
void Story::Tick(float realDt, bool waiting, int px, int py) {
    kidX = px; kidY = py;
    if (collection) {
        // v0.6: the clock keeps going while he hunts you. At 17:00 you can't run any more.
        if (chase) {
            float before = t;
            t = std::min<float>((float)HOUR, t + realDt * gShiftCfg.huntScale * gShiftCfg.timeMul);   // v0.15.2: the hunt runs slower than the hour
            // v0.7: he runs out of patience in the last three minutes
            (void)before;
            // v0.15.2 (Noah): more hunt before he starts on them, and more time between them
            static const int kThreatAt[3] = { 57 * 60 + 30, 58 * 60 + 40, 59 * 60 + 40 };
            for (int n = 1; n <= 3; ++n) if (t >= kThreatAt[n - 1] && threats < n && !visitorChasing && !visitorDowned && !handlers) { Threat(n); break; }
            // v0.8: the storm gets you if you stay out in it. A jacket buys time. Indoors you warm up again
            Room here = world.RoomAt(px, py);
            // v0.15 (Noah): without the jacket you don't make it across the yard. With it you can stay out
            // as long as you like, you just light up in his torch. Out of the wind is slower, not safe
            if (Warm()) {
                cold = std::max(0.0f, cold - realDt / 6.0f);   // the snow beast is warm inside
                if (Once("beast_warm")) { Bubble("You", "(Warmer on the inside. Smells worse, though.)"); Achieve("Smells Worse Inside"); }
            } else if (IsYard(here) && !hasCoat) {
                float full = 8.0f;   // real seconds out in the open. Front door to the shed is about five
                if (kidHidden) full *= hideSpot == "oldcar" ? 2.5f : 1.8f;
                cold = std::min(1.0f, cold + realDt / full);
            } else if (IsYard(here)) cold = std::max(0.0f, cold - realDt / 30.0f);
            else cold = std::max(0.0f, cold - realDt / 12.0f);
            if (cold >= 1.0f && Once("cold")) { ColdOut(); return; }
            if (t >= HOUR && Once("tired")) { if (handlers) HandlersEnding(false); else if (HoldDown()) TiedBells(); else if (NobodyLeft()) EmptyHouseEnding(); else if (kidHidden) PaidEnding(); else TiredOut(); }
        }
        return;
    }
    float prev = t;
    // v0.5: waiting is faster and speeds up the longer you hold it (2 to 5 game minutes per second)
    waitHeld = waiting ? waitHeld + realDt : 0;
    float scale = waiting ? WAIT_SCALE + std::min(waitHeld, 2.0f) * 90.0f : TIME_SCALE * gShiftCfg.timeMul;
    t = std::min<float>((float)HOUR, t + realDt * scale);
    CheckEvents(prev, t, px, py);

    // v0.9.2: he walked up to the window. His tracks are there until the storm takes them
    if (!Tutorial() && !collection && visitorLooked && !Storm() && world.RoomAt(px, py) == Room::Outside &&
        py >= 21 && py <= 29 && px >= 19 && px <= 26 && Once("prints")) {
        Say("You", "(Footprints in the new snow. Big ones. They come from the gate and stop right under the living room window.)");
        Say("You", "(Dad's boots aren't that big.)");
        AddClue("prints");
    }

    // v0.15: the first time out back without the jacket
    if (!collection && !Tutorial() && !hasCoat && !backyardCold && world.RoomAt(px, py) == Room::Backyard) {
        backyardCold = true;
        Bubble("You", "(Minus twenty-five back here. Without my jacket I wouldn't last long out here.)");
    }

    // Somebody unlocks the front door again
    if (frontLocked && !collection) {
        bool momCan = MomHere() && !(t >= 28 * 60 && t < 33 * 60);
        bool dadCan = DadFree() && (t < 8 * 60 || t >= 40 * 60);
        if ((momCan || dadCan) && t - frontLockedAt > 90) {
            frontLocked = false;
            Say(momCan ? "Mom" : "Dad", "(click) Why is the front door locked? We're not expecting anyone.");
        }
    }

    // Running onto the vidda. v0.15.1 (Noah): never in loop 1, nothing bad happens in the first round
    if (!collection && !Tutorial() && world.RoomAt(px, py) == Room::Vidda && Once("vidda")) {
        Narrate("The nearest neighbour was 31 kilometres away. I knew that. I ran anyway.");
        FadeFor(0, "Running");
        Black("You run until your lungs burn. The snow comes up to your knees.");
        Black("Then it's all white. No house. No fence. No sky.");
        Black("You walk for a long time. You stop feeling your feet.");
        Black("Footprints in front of you. Small ones. Fresh.");
        Black("They're yours. You walked in a circle.");
        Black("Headlights, right there at the gate. He's been waiting.");
        Sound("thud");
        Black("He isn't even out of breath. He carries you in like a sack of potatoes.");
        AddClue("run_fail");
        Achieve("Nowhere to Run");
        lastEnd = "vidda";
        t = (float)visitorTime;
        StartCollection("run");
    }
}

void Story::AdvanceMinutes(int minutes, int px, int py) {
    if (collection) return;
    float prev = t;
    t = std::min<float>((float)HOUR, t + minutes * 60.0f);
    CheckEvents(prev, t, px, py);
}

static bool Crossed(float prev, float now, int minute, int second = 0) {
    float at = minute * 60.0f + second;
    return prev < at && now >= at;
}

void Story::CheckEvents(float prev, float now, int px, int py) {
    Room here = world.RoomAt(px, py);
    bool inKitchen = here == Room::Kitchen;
    int phoneMin = Stage() >= 2 ? 21 : 22; // events start firing early

    // v0.5: flavour bubbles only show when you're actually there for the moment, not after a big time skip
    auto fresh = [&](int minute) { return now - minute * 60 < 90; };
    KnifeEvents(prev, now, px, py);   // v0.20 knife: Doggo and the ribbon
    if (Crossed(prev, now, 2) && Once("chores") && MomHere() && fresh(2)) {
        Bubble("Mom", "Chores! Doggo, trash, dishes, firewood, your room. ALL done by 16:40 and you can open a present before dinner!");
        See("chores");
        if (Tutorial()) Bubble("You", "(A present! Then Julenissen at ten to five! Best day of the year.)");
    }
    if (Crossed(prev, now, 8) && Once("dad_out") && fresh(8)) {
        Bubble("Dad", "Gonna fix the generator before the storm hits. Holler if you need me.");
        See("dad_out");
    }
    if (Crossed(prev, now, 10) && Once("radio_report")) {
        if (here == Room::Shed && DadFree()) {
            Say("Dad", "(into a radio) Unit 06 is stable. Yeah. I know what today is.");
            Say("Dad", "...Oh. Hey buddy. Didn't see you there.");
            AddClue("dad_radio");
        }
    }
    if (Crossed(prev, now, phoneMin) && Once("phone") && !phoneUnplugged) {
        Sound("ring");
        if (inKitchen || here == Room::Hallway || here == Room::LivingRoom) { Say("", "*RING RING* (the landline)"); See("phone"); }
    }
    if (Crossed(prev, now, phoneMin, 40) && phoneUnplugged && Once("phone_dead") && MomHere() && inKitchen)
        Bubble("Mom", phoneCut ? "(she pulls the bench out. She finds the cord. She holds the two cut ends and looks at the kitchen door.)"   // v0.20 knife
                               : "(she looks at the phone. Then at the clock. Then at the phone.)");
    if (Crossed(prev, now, phoneMin, 40) && !phoneUnplugged && Once("phone_mom")) {
        if (!phoneAnswered && MomHere()) {
            phoneAnswered = true;
            if (inKitchen) {
                Say("Mom", "(answering) Hello? ...Yes. Today. ...No, he doesn't suspect a thing.");
                Say("Mom", "(she sees you) Go play, sweetie.");
                AddClue("phone_mom");
            }
        }
    }
    if (Crossed(prev, now, 30) && Once("basement") && MomHere()) {
        if (here == Room::Hallway) {
            Say("", "(Mom unlocks the basement door. Blue light spills out. Something hums down there.)");
            AddClue("basement_open");
        }
        if (inBasement) {
            if (tainted) {
                Say("", "(Mom comes down the stairs. She sees you. She doesn't scream.)");
                Say("", "(She just goes back up. Backwards. Not taking her eyes off you.)");
            } else {
                Say("Mom", "(on the stairs) No. No no no. You can't be down here. You CAN'T be down here.");
                ChangeMood(-3);
            }
        }
    }
    if (Crossed(prev, now, 35) && Once("doggo_door")) {
        if (fresh(35) && DoggoExists() && (here == Room::LivingRoom || here == Room::Hallway)) {
            Bubble("Doggo", "*whines at the front door*");
            See("doggo_door");
            if (!Tutorial() && !Knows("doggo_out") && Once("doggo_door_think")) Bubble("You", "(Every day at 16:35. He wants out.)");
        }
    }
    if (Crossed(prev, now, 40) && Once("allowance")) {
        allowanceChecked = true;
        if (!grounded && MomHere()) See("present");
        if (!grounded && MomHere() && momAtt == Attitude::Scared) {
            Say("", "(16:40. Mom doesn't mention the chores. She doesn't mention anything.)");
        } else if (!grounded && MomHere()) {
            if (ChoresDone() == CHORES) {
                Say("Mom", "All five? Look at you. Go on, then. One present before dinner.");
                Say("You", Knows("presents") ? "(I know which one. It's the only one with anything in it.)" : Stage() >= 1 ? "(It won't matter. I'm opening it anyway.)" : "(ZULDA! Straight to the tree!)");
                earnedPresent = true;
                mood += 1;
            } else if (ChoresDone() > 0) {
                Say("Mom", "Half done is not done. The presents can wait.");
                ChangeMood(-1);
            } else {
                Say("Mom", "Nothing? Not one chore? Unbelievable.");
                ChangeMood(-2);
            }
            if (doggoOut) Say("Mom", "And where's the dog? ...Fine. He'll scratch at the door when he's cold.");
        }
    }
    // v0.16 (Noah): Dad has more of a day. Only when you're close enough to hear it
    {
        NpcState d = Dad();
        bool nearDad = DadFree() && d.visible && std::abs(d.x - px) + std::abs(d.y - py) <= 9;
        if (nearDad && Crossed(prev, now, 4) && Once("dad_paper") && fresh(4))
            Bubble("Dad", Tutorial() ? "Minus twenty-five, it says. Your nose'll fall off, buddy." : "(behind the paper) Minus twenty-five. Road closes at five. Same as always.");
        if (nearDad && Crossed(prev, now, 15) && Once("dad_gen") && fresh(15))
            Bubble("Dad", "(to the generator) Come on, old girl. Don't do this to me tonight.");
        if (nearDad && Crossed(prev, now, 25) && Once("dad_whistle") && fresh(25))
            Bubble("Dad", Stage() >= 2 ? "(he whistles. It's her song. He stops halfway.)" : "(whistling something off-key)");
        if (nearDad && Crossed(prev, now, 33) && Once("dad_copy") && fresh(33) && !Tutorial())
            Bubble("Dad", "(quietly, into the radio) ...Copy. No change. Out.");
        if (nearDad && Crossed(prev, now, 36) && Once("dad_wood") && fresh(36))
            Bubble("Dad", firewoodDone ? "Somebody already brought wood in? Huh. Good man." : "Wood for the stove. Somebody forgot their chore.");
        if (nearDad && Crossed(prev, now, 41) && Once("dad_kitchen") && fresh(41))
            Bubble("Dad", tainted ? "(to Mom, low) Has he said anything to you? ...He keeps looking at the door." : "(to Mom) Smells good, love. Ribbe again?");
        if (nearDad && Crossed(prev, now, 47) && Once("dad_storm") && fresh(47))
            Bubble("Dad", "There it is. Storm's here. Everybody inside, and nobody opens that door.");
    }
    if (Crossed(prev, now, 45) && Once("nisse_soon") && fresh(45) && DadFree() && !tainted)
        Bubble("Dad", "Julenissen comes at ten to five, buddy. Better be good.");
    if (shift == Shift::Toilet45 && !pooped && Crossed(prev, now, 45) && Once("accident")) {
        Say("You", "(Oh no.)");
        Say("You", "(Oh no no no no.)");
        FadeFor(0, "");
        Black("You didn't make it.");
        Black("In this reality, the toilet always wins.");
        Achieve("Should've Gone");
        lastEnd = "toilet";
        AddClue("poop45");
        seenToilet45 = true;
        Narrate("No. That's... that is NOT how it went. We're not telling it like that.");
        Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
        collection = true; // freeze the clock
        return;
    }
    if (shift == Shift::EarlyStorm && Crossed(prev, now, 37) && Once("storm")) {
        Say("", "(The snowstorm hits. Ten minutes early. It has never been early.)");
        if (!inBasement) See("storm");
    }
    if (shift == Shift::EarlyStorm && Crossed(prev, now, 40) && Once("early_power")) {
        Sound("thud");
        Say("", "(The power dies. It's only 16:40. Nobody is at the door. Not yet.)");
        Achieve("Weather Report");
    }
    if (Crossed(prev, now, 47) && Once("storm")) {
        Say("", "(The snowstorm hits. The lights flicker. You can't see the gate any more.)");
        if (!inBasement) See("storm");
    }
    // v0.13: hinge A. Two and a half minutes in the bathroom, every loop
    if (Crossed(prev, now, 46) && Once("mom_bath") && MomInBath() && fresh(46)) {
        if (here == Room::Kitchen || here == Room::Hallway || here == Room::Bathroom) { Bubble("Mom", "Two minutes. Nobody touch the stove."); See("mom_bath"); }
    }
    if (momBathLocked && Crossed(prev, now, 48, 30) && Once("bath_late") && fresh(48))
        Bubble("Mom", bathLockDark ? "(not a sound from the bathroom)" : "Kiddo! Joke's over! He'll be here any... Dinner! Dinner's soon!");
    // v0.10: Dad and the box under the flagpole. You have to be out there to see it
    if (Crossed(prev, now, 44, 40) && Once("flag44") && !Tutorial() && DadFree() && Dad().x == 11 && here == Room::Outside &&
        std::abs(px - 10) + std::abs(py - 26) <= 16 && now - (44 * 60 + 40) < 90) {
        Say("", "(Dad crouches at the bottom of the flagpole. A tiny box, painted the same colour as the pole. He opens it, looks, closes it.)");
        int left = visitorTime - (int)now;
        if (left <= VISIT_LOOK && left > VISIT_BACK) {
            Say("", "(Then he looks up, across the yard, at the big man by the living room window. Dad nods. The man nods back.)");
            Say("You", "(They know each other.)");
            Achieve("A Nod Across the Yard");
        }
        Say("You", "(Flag. Forty-four. That's what he mutters in the shed. He's checking that something is still there.)");
        See("flag44");
    }
    if (now >= visitorTime - VISIT_ARRIVE && Once("car")) {
        if (here == Room::Outside || here == Room::KidsRoom) {
            Say("", EngineOn() ? "(Headlights at the gate. A black car. No plates.)" : "(A black car at the gate. No plates. Dark.)");
            See("car");
        }
    }
    // v0.9.2: he has a look at the family first, then fetches what he came with
    if (!collection) {
        int left = visitorTime - (int)now;
        if (left <= VISIT_LOOK && left > VISIT_BACK) {
            visitorLooked = true;
            if (here == Room::LivingRoom && Once("face")) {
                Say("", Tutorial() ? "(A big shape outside the window. Julenissen is early!)"
                                   : "(A face at the window, close to the glass. It's looking right at you. Then it's gone.)");
                See("window");
            }
        }
        // v0.19: he gets back to a car with no key in it. He doesn't sit and wait for his minute
        if (!carKey.empty() && left <= VISIT_IN_CAR && Once("key_gone")) {
            Once("car_in"); Once("car_out");
            if (IsYard(here)) {
                Sound("thud");
                Say("", "(He opens his car door. He stops. He pats his coat, then the seat, then his coat again.)");
                Say("", "(He straightens up and looks at the house for a long time. Then he starts walking.)");
                See("car_in");
            }
            visitorTime = std::min(visitorTime, (int)now + 40);
        }
        if (left <= VISIT_IN_CAR && left > VISIT_OUT && Once("car_in") && here == Room::Outside) {
            Sound("thud");
            Say("", "(He gets back into the car. The door shuts. The engine keeps running.)");
            See("car_in");
        }
        if (left <= VISIT_OUT && Once("car_out") && here == Room::Outside) {
            Sound("thud");
            Say("", "(The car door opens. There's something under his coat that wasn't there before.)");
            See("car_out");
        }
    }
    if (now >= visitorTime && !collection) {
        t = (float)visitorTime;
        StartCollection("normal");
    }
}

// ============================================================ the collection
void Story::StartCollection(const std::string& how) {
    if (collection) return;
    stance = CurrentStance();   // v0.7: decided before anything else changes
    collection = true;
    grounded = false;
    if (how == "run") { CollectionScript(); return; }
    // v0.13: hinge C. Down in the basement when he knocks, you stay down there
    bool downstairs = inBasement && !Tutorial();
    if (inBasement && !downstairs) {
        inBasement = false;
        Say("", "(Up in the house, the power dies. You come up the stairs into the dark.)");
        FadeFor(0, "", 27, 11);
    }

    if (Tutorial()) { frontLocked = false; TutorialCollection(); return; }   // v0.8: loop 1 has no chase

    // v0.13: where he starts the hunt. The shed wins, it's the last place he goes
    huntStart = downstairs ? "" : dadLocked ? "shed" : momBathLocked ? "bath" : how == "door" ? "door" : "";

    // v0.19: your fake trail out the back. Ten minutes of storm and it's gone
    if (falseTracksAt > 0) {
        int hold = shift == Shift::EarlyStorm ? 3 * 60 : 6 * 60;
        if (visitorTime - falseTracksAt <= hold) falseTracks = true;
        else Narrate("My tracks out back were gone by then. The storm had been filling them in for " + std::to_string(std::max(1, (visitorTime - (int)falseTracksAt) / 60)) + " minutes.");
    }
    if (how == "door") HingeDoor();   // v0.14: hinge D. No knock. You opened it
    else {
    Sound("knock");
    if (downstairs) HingeBasement();
    else Say("", "(The power dies. Then: three slow knocks.)");
    if (frontLocked) {
        if (downstairs) Say("", "(Above you, glass explodes. Heavy boots on the living room floor.)");
        else {
            Say("", "(Nobody opens. The handle turns. Locked.)");
            Sound("glass");
            Say("", "(Glass explodes in the living room. He climbs in through the window like it's a door.)");
        }
        AddClue("fortress");
        Achieve("Fortress");
    } else if (downstairs) {
        Say("", "(Through the floor: the front door. Mom's voice, too bright. His, very calm.)");
    } else if (MomHere()) {
        if (stance == Stance::Dark) Say("Mom", "(opening the door) Thank God. Take him. Please. Just take him.");
        else if (stance == Stance::Befriended) Say("Mom", "(opening the door, her voice too bright) ...You're early.");
        else Say("Mom", "(opening the door) ...You're early.");
    } else {
        Say("", "(Nobody answers. The door opens anyway. He has a key.)");
    }

    if (traps > 0 && !frontLocked) {
        Sound("crack");
        if (downstairs) Say("", "(A crash right above your head. Something in the hallway cracks. Not wood.)");
        else {
            if (trapsSet.count("marbles")) Say("", "(His boot hits the marbles on the iced step. He goes down hard. Something in his wrist cracks.)");
            if (trapsSet.count("tripwire")) Say("", "(The tripwire catches his shin. He falls through the doorway face first. The sound is wrong. Like a branch snapping.)");
            if (trapsSet.count("paint")) Say("", "(The paint can swings down into his face. In the movie it's funny. His nose is just... flat now.)");
        }
        AddClue("traps");
        Achieve("Home Alone (Realistic Edition)");
        visitorInjured = true;
        if (traps >= 2) {
            visitorDowned = true;
            if (!downstairs) Say("", "(He's on the floor. Breathing through his teeth. He's not getting up. Yet.)");
        }
        // v0.7: you hurt him. Doggo is the first thing he hurts back.
        if (DoggoExists() && !Tutorial()) { DoggoShot(!downstairs); doggoDeadX = 19; doggoDeadY = 19; }
    }

    if (downstairs) {
        // v0.13: you hear the rest through the floor. You see none of it
        Say("You", "(The stairs are the only way out of here. He doesn't know I'm down here. Yet.)");
        SceneBeat("chase");
        return;
    }

    Say("The visitor", visitorDowned ? "(from the floor, still calm) ...Evening." : "Evening.");
    // v0.19: he got back to a dead car
    if (!carKey.empty() && !visitorDowned) {
        Say("The visitor", "(he holds up one empty glove) Somebody turned my engine off.");
        if (MomHere() && !frontLocked) Say("Mom", "(she looks at you. She can't help it.)");
        Say("The visitor", "(to you, almost friendly) We'll talk about it in the car.");
    }
    // v0.19: you pushed something against your door, and you're behind it
    if (!visitorDowned && !frontLocked && MomHere() && BarricadeRoom(6, 9) == world.RoomAt(kidX, kidY) && Barricaded(6, 9)) {
        Say("Mom", "(calling down the hall) Kiddo? Kiddo, come out and say... (the handle rattles) He's pushed something against his door.");
        Say("The visitor", "(he smiles, a little) Has he, now.");
    }

    // v0.8: it's the question Julenissen always asks
    if (!visitorDowned && !frontLocked && MomHere()) {
        Say("The visitor", "Has he been good?");
        if (stance == Stance::Dark) Say("Mom", "(she won't look at you) Just take him.");
        else if (stance == Stance::Befriended) Say("Mom", "(her voice cracks) He's been so good.");
        else Say("Mom", "(very quietly) ...He's been good.");
        if (phoneUnplugged) {
            // v0.10: nobody confirmed anything at 16:22
            Say("The visitor", "(to her) You didn't pick up.");
            Say("Mom", "It never rang.");
            Say("The visitor", "(he looks at the phone. Then at you, one second too long.) ...No. I suppose it didn't.");
            if (phoneCut) { Say("Mom", "(very quietly) Somebody cut the cord."); Say("The visitor", "(still looking at you) Somebody did."); }   // v0.20 knife
        }
        if (momRemorse && stance != Stance::Dark) Say("Mom", "(she isn't looking at him. She's looking at you. She's been crying.)");
    }
    }   // v0.14: end of the knock (hinge D skips it)

    // v0.13: hinges A and B. He goes to the parent you shut away, and talks through the door
    if (momBathLocked && !visitorDowned) HingeBath();
    if (dadLocked && !visitorDowned) HingeShed();

    // v0.7: they believe you, and they know what clean-up means now
    if (parentsAllied && stance == Stance::Befriended && MomHere() && DadFree() && !visitorDowned) {
        Say("Mom", "(she grabs your hand, so hard it hurts) Car. NOW.");
        std::vector<ChoiceOpt> o = {{"e_run", "Run for his car with Mom and Dad."}, {"e_stay", "No. Hide. Dad, don't!"}};
        // v0.10: you have seen where the road ends
        if (Knows("leash")) o.insert(o.begin() + 1, {"e_dinner", "\"We can't outrun it. Sit down. Please.\"", true});
        Choice("Mom is already pulling you toward the door.", o);
        return;
    }
    // v0.18 (section 3): only one of them knows. That one stays close, and waits for him to get close to you
    std::string def = Defender();
    if (!def.empty() && !visitorDowned) {
        if (def == "Mom") {
            Say("Mom", "(behind her back, one hand finds your shoulder and squeezes. Once.)");
            Say("Mom", "(barely moving her lips) Hide. I'm right behind you.");
            if (DadFree()) Say("", "(Dad looks from you to Mom to the man. He doesn't understand what he's looking at.)");
        } else {
            Say("Dad", "(he puts himself between you and the door, very casually, like he's just standing there)");
            Say("Dad", "(without turning round) Go hide, buddy. Let him come to me.");
            if (MomHere()) Say("Mom", "(very quietly) ...Go with him, sweetie. It's okay.");
        }
        SceneBeat("chase");
        return;
    }
    if (DadFree() && dadAtt == Attitude::Broken) {
        Say("", "(Dad doesn't get up. He just looks at the floor.)");
    } else if (DadFree() && stance == Stance::Befriended && !visitorDowned) {
        Say("Dad", "(grabbing a lamp) Buddy. Run. RUN!");
        SceneBeat("dad_runs");   // v0.7: he doesn't run away. He runs AT him
        return;
    } else if (DadFree() && parentsAllied && visitorDowned && !dadOnFloor) {
        // v0.18: they're with you, and he's on the floor. Nobody planned for this part
        Say("Dad", "(he looks at the man on the floor, then at you) ...Tell us what to do, buddy.");
    } else if (DadFree() && stance == Stance::Dark) {
        Say("Dad", "(he looks at you, then at him) ...He's all yours.");
        if (!Tutorial() && !visitorDowned && !dadTurned) Say("Dad", "(to him) I'll take the door.");
    } else if (DadFree()) {
        // v0.7: normal day. They hand you over. They think they get to retire after
        Say("Dad", "(quietly, not looking at you) Go with him, buddy. It's okay. It's... it's okay.");
        if (MomHere()) Say("Mom", "(she's holding the door frame like it's the only thing keeping her up)");
    }
    // v0.18 (section 3): Dark. They want him gone, so they help him look
    if (stance == Stance::Dark && !Tutorial() && !visitorDowned && ((MomHere() && !momTurned) || (DadFree() && !dadTurned))) {
        if (MomHere() && !momTurned) Say("Mom", "(to him, not to you) Wardrobe. Under the beds. He likes small places. I'll show you.");
        AddClue("they_hunt");
        Achieve("Search Party");
    }
    if (!Knows("he_runs_fast") && !Knows("hiding") && !doorGreeted)
        Say("You", "(He doesn't know where I am yet. Hide. Sneak past him. Or walk up to him and press E.)");
    SceneBeat("chase");
}

// ============================================================ v0.13: the hinges
// Each one is a door you shut on somebody. He has to talk to them through it, and you get to listen.
// The three pieces are Claude's proposal (Noah left them open). Together they say: he was you, and you will be him.
void Story::PieceCheck() {
    int n = HingePieces();
    if (n >= 3 && !achievements.count("Three of Five")) {
        Achieve("Three of Five");
        Narrate("Three doors. Three pieces. I could feel where the other doors were. I couldn't see them yet.");
    }
    if (n >= 5 && !achievements.count("Five of Five")) {
        Achieve("Five of Five");
        Narrate("Five doors. Five pieces. I knew what I could say to him now. I didn't know yet if I wanted to.");
    }
}

void Story::HingeBath() {
    // A: Mom is in the bathroom. He stops at her door before he starts on yours
    Sound("thud");
    Say("", "(Down the hall: Mom, pounding on the bathroom door.)");
    if (bathLockDark) Say("Mom", "(through the door) Is that you? Is it him? Which one of you IS it?");
    else Say("Mom", "(through the door) Kiddo? Is that him? Kiddo, let me OUT. Not today, sweetie. Not TODAY.");
    Say("", "(His steps go down the hallway. They stop at the bathroom door. You hold very still and listen.)");
    { Beat f; f.type = BeatType::Fade; f.id = "hinge_bath"; beats.push_back(f); }   // the presentation moves him there in the black
    Say("The visitor", "(quietly, to the door) Evening, ma'am.");
    Say("Mom", "...You. Of course it's you.");
    Say("The visitor", "Do I know you?");
    if (bathLockDark) {
        Say("Mom", "He locked me in. He's got the knife. Look at him. LOOK at him. He's turning into you.");
        Say("The visitor", "(a long pause) Into me.");
    } else {
        Say("Mom", "Tell them I did it right this time. He never saw the chair awake. Not once. Not like with you.");
        Say("The visitor", "(a long pause) With me.");
    }
    Say("Mom", "You used to hum it too. Under the duvet. Week 612, you hummed it every single night.");
    if (momRemorse) Say("Mom", "He hummed it back to me today. Just like you.");
    Say("The visitor", "(he puts his glove flat on the door) I never lived here, ma'am.");
    Say("Mom", "No. Of course you didn't. Of course not.");
    Say("You", "(Week 612. That's this week. That's me.)");
    AddClue("piece_mom");
    PieceCheck();
    Say("The visitor", "(louder, to the whole house) Zero-six. Let's not drag it out.");
}

void Story::HingeShed() {
    // B: Dad is barred in the shed. The man goes out to him first, with his torch
    Say("The visitor", "Where's the father?");
    if (MomHere()) Say("Mom", stance == Stance::Dark ? "(she points at the yard) The shed. The kid put a plank on the door. Like an animal." : "(she looks at the yard) The shed. He... the door's barred.");
    else Say("The visitor", "(he looks through the house, out at the shed, at the plank across its door) Huh.");
    Say("", frontLocked ? "(He climbs back out through the broken window. The storm comes in, and with it every word.)"
                        : "(He walks back out into the storm and leaves the front door wide open. The wind carries every word.)");
    { Beat f; f.type = BeatType::Fade; f.id = "hinge_shed"; beats.push_back(f); }
    Say("", "(His torch finds the shed. A beam like a lighthouse. He doesn't lift the plank.)");
    Say("Dad", "(shouting, from inside) Let me out and I'll find him for you! I know every spot! Twelve years on the screens!");
    Say("The visitor", "You'd sell him for a plank.");
    Say("Dad", "...Hold your torch up. Your watch. Read me the back of your watch.");
    Say("The visitor", "Why.");
    Say("Dad", "Because I scratched it. With a nail. Four days after he was born. ANCHOR 06. Crooked, because he wouldn't stop kicking.");
    Say("", "(The beam doesn't move for a long time.)");
    Say("The visitor", "They all say something.");
    Say("Dad", "Not crooked like that.");
    Say("The visitor", "(he lets the beam drop) Stay in the shed. I know where you are.");
    if (Knows("baby_watch")) Say("You", "(The tiny watch in Dad's cabinet. Crooked letters. Scratched with a nail.)");
    else Say("You", "(I turn my watch over. Nothing on the back. Just scratches where something used to be.)");
    AddClue("piece_dad");
    PieceCheck();
}

void Story::HingeBasement() {
    // C: you are downstairs when the power dies. The console runs on a battery, and the channel is open
    Say("", "(The hum stops. Every screen goes black. Somewhere above you: three slow knocks.)");
    Say("", "(One small screen flickers back on. BATTERY. The radio console clicks and hisses.)");
    Say("Voice", "(on the console) Finse to collector. Confirm on site, unit 06.");
    Say("The visitor", "(on the radio, from somewhere above you) On site. Power's down.");
    Say("Voice", "Our cameras went with it. You'll have to find him the old way.");
    Say("Voice", "Process as before. Wipe. Age. Reissue. Same house, next cycle.");
    Say("The visitor", "(a long burst of static) ...Copy. Same house.");
    Say("Voice", "Your anchor reads drift again, collector. Dose before or after?");
    Say("The visitor", "After.");
    Say("You", "(Reissue. Like a library book. Same house. Next cycle.)");
    if (Knows("piece_mom") || Knows("piece_dad")) Say("You", "(Next cycle there's a kid in my bed and a man at the door. Which one of them do they reissue?)");
    AddClue("piece_reissue");
    PieceCheck();
}

// v0.14: hinge D. You open the door before he knocks. Nobody ever has, so for a minute he is off his script
void Story::HingeDoor() {
    if (EngineOn()) Say("", "(Down at the gate the engine stops. He gets out and walks up the path. Slower than he ever has.)");
    else Say("", "(Down at the gate his car sits dark and dead. He walks up the path with his hands in his pockets. Slower than he ever has.)");   // v0.19
    Say("", "(He stops on the step and looks at you. Snow on his shoulders. Nobody has ever opened this door for him. You can tell.)");
    if (traps > 0) Say("", "(He looks at what you rigged on the step. Then at you. He steps over all of it.)");
    Say("The visitor", "...You're supposed to wait for the knock.");
    Say("You", "I know.");
    Say("", "(He crouches, so his face is level with yours. His eyes keep sliding past you, into the house.)");
    Say("The visitor", "Your window sticks when it's this cold. You have to hit it with the heel of your hand first.");
    Say("You", "...How do you know that?");
    Say("The visitor", "(he looks down the hall toward your room for a long time) Don't know.");
    Sound("beep");
    Say("", "(His watch beeps. Two short, one long. He looks at it like a dog that just bit him.)");
    Say("The visitor", "Drift.");
    if (!Knows("w_drift")) { Say("You", "(Two short, one long. Mine does that every day at 16:00.)"); AddClue("w_drift"); }   // v0.20 story
    Say("", "(He takes a small pen out of his coat. A needle at the end. He holds it over his wrist. Then he puts it back.)");
    Say("The visitor", "After. They want it after. Always after.");
    Say("You", "What does it do?");
    Say("The visitor", "Keeps the house out of my head.");
    Say("The visitor", "(he stands up) Most nights there's a kid in a red sweater. Standing in a door. Just like that.");
    Say("You", "(I'm wearing my red sweater.)");
    AddClue("piece_door");
    Achieve("Open House");
    PieceCheck();
    if (MomHere()) {
        if (stance == Stance::Dark) Say("Mom", "(behind you, almost relieved) He opened it for you. Of course he did.");
        else if (stance == Stance::Befriended) Say("Mom", "(behind you, much too loud) Kiddo! Get AWAY from the door!");
        else Say("Mom", "(behind you) Kiddo, what are you... Oh. You're early.");
        Say("", "(He straightens up. The script comes back into his face like a light switching on.)");
        Say("The visitor", "Evening, ma'am. Has he been good?");
        if (stance == Stance::Dark) Say("Mom", "(she won't look at you) Just take him.");
        else if (stance == Stance::Befriended) Say("Mom", "(her voice cracks) He's been so good.");
        else Say("Mom", "(very quietly) ...He's been good.");
    } else {
        Say("", "(He straightens up. The script comes back into his face like a light switching on.)");
        Say("The visitor", "Evening.");
    }
    Say("", "(He doesn't grab you. He looks past you, into the house, like he's counting the rooms.)");
}

// v0.14: hinge E. Under the blanket on the floor of his car. He gets back in and sits there with you
void Story::HingeCar() {
    carHidden = true;
    Say("", "(The back door isn't locked either. You climb in, curl up on the floor behind the seats and pull a grey blanket over you.)");
    Say("You", "(It smells like oil. And other kids.)");
    FadeFor(0, "");
    Sound("hush_on");
    t = std::max(t, (float)(visitorTime - VISIT_IN_CAR));
    Once("car_in"); Once("car_out");   // you were there for both, under the blanket. No yard lines afterwards
    See("car_in");
    Sound("thud");
    Black("The driver's door. The whole car dips when he sits down.");
    Black("He breathes. In through the nose, out through the mouth. Like he's counting.");
    Black("The radio on the dashboard clicks.");
    Black("\"Finse to collector. Status.\"");
    Black("\"At the gate. Family's inside. He's in there somewhere.\"");
    Black("\"This is your last pickup, collector. After 06 you come in.\"");
    Black("He doesn't answer for a long time. \"...Come in for what.\"");
    Black("\"Your anchor drifts past the dose now. You know what that means. The new collector is in that house.\"");
    Black("\"...Copy.\"");
    Black("\"Say it back.\"");
    Black("A breath. \"The new collector is in the house.\"");
    Black("He sits very still. Then he opens the glovebox, takes something out and puts it under his coat.");
    t = std::max(t, (float)(visitorTime - VISIT_OUT));
    See("car_out");
    Sound("thud");
    Black("The door. His boots in the snow, going away from you. Up toward the house.");
    Sound("hush_off");
    AddClue("piece_car");
    Achieve("Stowaway");
    PieceCheck();
    Choice("Under the blanket. He's walking up to the house.",
           {{"cr_out", "Slip out and get back to the house."}, {"cr_stay", "Stay where you are."}});
}

// v0.14: hinge E, the rest of it. You are not in the house when he knocks. It ends anyway
void Story::GoneEnding() {
    stance = CurrentStance();
    t = (float)visitorTime;
    collection = true; grounded = false; chase = false;
    FadeFor(0, "");
    Sound("hush_on");
    Sound("knock");
    Black("Far away: three slow knocks. A door. Voices. Then nothing for a long time.");
    Black("The blanket gets warm. You count your breaths, the way he counts his.");
    if (Spared()) Black("No shouting. No bangs. Nothing at all.");
    else {
        Black("Faint, from the house: \"Zero-six! Three minutes!\"");
        if (MomHere() || momLocked || momBathLocked) { Sound("bang"); Black("BANG."); }
        if (!dadDown) { Sound("bang"); Black("BANG."); }
        if (DoggoExists()) { Sound("bang"); Black("Doggo stops barking."); }
    }
    Sound("thud");
    Black("The driver's door. He sits down. He doesn't start the engine.");
    Black("\"Collector to Finse. Subject not on site.\"");
    Black("\"Then he's out in the snow. Log a void cycle. They always come back to the house.\"");
    Black("He turns the mirror. In it, under the edge of the blanket, your eyes meet his.");
    Black("He looks at you for a long time. He doesn't say anything. He turns the mirror back.");
    Black("You sit in the dark car together until the bells.");
    Sound("bells");
    Black("Far away, through the storm: bells.");
    See("bells");
    lastEnd = "gone";
    Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
}

void Story::TutorialCollection() {
    // v0.8: loop 1 is the happiest day of his year. He runs to the door himself.
    Sound("knock");
    Say("", "(The lights go out. Then three slow knocks on the front door.)");
    Say("You", "(JULENISSEN!)");
    if (MomHere()) Say("Mom", "(calling, her voice too bright) Kiddo! There's someone at the door for you!");
    FadeFor(0, "You run to the door", 22, 18);
    Say("", "(He's very big. No beard. No sack. Just a black coat with snow on the shoulders.)");
    Say("The visitor", "Evening.");
    Say("The visitor", "(he crouches down to look at you) Have you been good?");
    Say("You", ChoresDone() > 0 ? "...I did my chores." : "...I think so.");
    Say("The visitor", "I know.");
    if (DadFree()) Say("Dad", "(quietly, not looking at you) Go with him, buddy. It's okay. It's... it's okay.");
    if (MomHere()) Say("Mom", "(she's holding the door frame like it's the only thing keeping her up)");
    CollectionScript();
}

void Story::ColdOut() {
    // v0.8: the snowstorm gets you
    chase = false;
    Say("", "(You can't feel your hands any more. The snow has stopped being cold.)");
    Say("", "(That's nice. You sit down. Just for a second.)");
    FadeFor(0, "");
    Black("It's warm now.");
    Black("White.");
    AddClue("cold");
    Achieve("Snow Angel");
    lastEnd = "cold";
    Narrate("No. That's not how it went. The snow didn't get me. Not that time.");
    Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
}

void Story::SceneDone(const std::string& id) {
    if (id == "dad_runs") {
        // v0.7: Dad went at him with a lamp. He's on the floor now, not dead. Not yet.
        dadDown = true; dadOnFloor = true;
        Say("", "(A sound like a plank snapping. Dad folds up on the floor and doesn't get up.)");
        Bubble("Mom", "No no no. Get up. Please get up.");
        AddClue("dad_ran");
        Achieve("Ten Seconds");
        lastEnd = "dadran";
        Bubble("The visitor", "Now. Where were we, 06?");
        SceneBeat("chase");
    }
}

void Story::VisitorChoice() {
    std::vector<ChoiceOpt> o;
    // v0.17: the true ending. All five pieces, and him where he can't walk away from them
    auto told = [&](const char* id) { return std::find(toldPieces.begin(), toldPieces.end(), id) != toldPieces.end(); };
    if (visitorDowned && HingePieces() >= 5 && !remembered) {
        if (!told("t_hum")) o.push_back({"t_hum", "(Hum Mom's song.)", true});
        if (!told("t_watch")) o.push_back({"t_watch", "\"Read the back of your watch.\"", true});
        if (!told("t_reissue")) o.push_back({"t_reissue", "\"Do you know what reissue means?\"", true});
        if (!told("t_dose")) o.push_back({"t_dose", "\"You haven't had your dose.\"", true});
        if (!told("t_last")) o.push_back({"t_last", "\"This is your last pickup.\"", true});
    }
    if (visitorTied) {
        // Noah's answer 10: tied up he only talks in riddles
        o.push_back({"t_talk", "Talk to him."});
        o.push_back({"t_wait", "(Sit down by the door and wait for the bells.)"});
        o.push_back({"none", "(Leave him.)"});
        Choice(toldPieces.empty() ? "His hands are tied behind him. He's watching you." : "He's still tied up. He's listening now.", o);
        return;
    }
    if (!toldPieces.empty()) {
        // you started. He isn't going anywhere, and you aren't either
        o.push_back({"none", "(Not yet. Back away.)"});
        Choice("He's still on the floor. He's listening now.", o);
        return;
    }
    // v0.18: answer 7. Both of them on your side and him on the floor: three of you, one of him
    if (visitorDowned && parentsAllied && TurnedStanding()) o.push_back({"c_help", MomHere() && !momOnFloor && DadFree() && !dadOnFloor ? "\"Mom! Dad! Help me hold him!\"" : (MomHere() && !momOnFloor ? "\"Mom! Help me hold him!\"" : "\"Dad! Help me hold him!\""), true});
    if (visitorDowned && traps >= 3 && trapsSet.count("tripwire")) o.push_back({"t_tie", "Tie his hands with the tripwire."});
    if (visitorDowned && hasKnife && tainted) o.push_back({"c_finish", "Finish it.", false, true});
    if (visitorDowned && !hasKnife) o.push_back({"c_kick", "Kick him while he's down."});
    if (visitorDowned && !Knows("collector")) o.push_back({"c_wrist", "Look at his wrist."});
    o.push_back({"c_go", threats > 0 ? "\"Stop! I'm here. Leave them alone.\"" : "Go with him."});
    if (!visitorDowned) {
        if (hasKnife) o.push_back({"c_knife", "Pull the knife.", false, true});
        else if (hasStick) o.push_back({"c_stick", "Swing the stick at him.", false, true});
        else o.push_back({"c_fight", "Fight him."});
        if (hasLight && !lightUsed && PowerOut()) o.push_back({"c_light", "Shine the flashlight in his eyes."});
        if (hasTaser) o.insert(o.begin(), {"c_tase", "His own taser. Right in the chest.", false, false});
    }
    if (Knows("visitor_time")) o.push_back({"c_know", "\"I know what you are.\"", true});
    if (Knows("handlers") && (MomHere() || DadFree())) o.push_back({"c_parents", "\"Mom. Dad. Did you know?\"", true});
    o.push_back({"none", "(Back away.)"});
    Choice(visitorDowned ? "He's on the floor. He's still bigger than you." : "He's huge. He's calm. He's right in front of you.", o);
}

void Story::OnCaught() {
    if (!chase) return;
    if (doggoWithKid && DoggoExists() && !Tutorial()) { DoggoSacrifice(); return; }
    chase = false;
    Say("", "(A hand like a vice closes on your collar.)");
    Say("The visitor", "Gotcha.");
    AddClue("he_runs_fast");
    Thirty();   // v0.19: if he walked your fake trail first
    CollectionScript();
}

void Story::OnFound() {
    if (!chase) return;
    if (doggoWithKid && DoggoExists() && !Tutorial()) { DoggoSacrifice(); return; }
    chase = false;
    // v0.12: he only says it once it is true. A spot he has found you in is a spot he checks first
    int before = CaughtIn(hideSpot);
    Say("", before > 0 ? "(No searching. The footsteps come straight here. Light. A big hand.)" : "(Light. A big hand.)");
    if (before == 0 && hideSpot == "beast") { Say("", "(The zip. Slowly, all the way down. Cold air and torchlight.)"); Say("The visitor", "Cosy in there, 06?"); }
    else if (before == 0 && hideSpot == "oldcar") { Say("", "(The back door of the old car opens with a crunch of ice.)"); Say("The visitor", "This old thing. I used to hide in here too."); }
    else if (before == 0 && hideSpot == "spruce") { Say("", "(The branches lift. Snow comes down on your neck. Then a torch.)"); Say("The visitor", "Under the tree. Like a present."); }
    else if (before == 0) Say("The visitor", "Found you.");
    else if (before == 1) Say("The visitor", "Found you. You always pick that one.");
    else Say("The visitor", "This one. Every time, 06. I don't know how I know. I just walk here.");
    if (!hideSpot.empty()) caughtSpots[hideSpot] = before + 1;
    AddClue("hiding");
    Achieve("Hide and Seek");
    if (before > 0) { AddClue("he_learns"); Achieve("Creature of Habit"); }
    Thirty();   // v0.19: if he walked your fake trail first
    CollectionScript();
}

// v0.12: the hunt. The presentation decides when these happen, the story decides what they mean
void Story::OnHeard(const std::string& how) {
    if (!chase) return;
    if (how == "run") {
        if (Once("heard_run")) Bubble("The visitor", "Little feet. Fast ones.");
        AddClue("he_hears");
    } else if (how == "gasp") {
        Bubble("The visitor", "There it is.");
        AddClue("breath");
    } else {
        Bubble("The visitor", "I can hear you breathing.");
        AddClue("breath");
    }
}
void Story::OnTracked() {
    if (!chase) return;
    if (Once("tracked_in")) Bubble("The visitor", "Wet boots, 06.");
    AddClue("wet_prints");
}
bool Story::OnDecoy() {
    if (!chase) return false;
    ++decoys;
    if (decoys > 2) { Bubble("The visitor", decoys == 3 ? "(he smiles) Marbles. That's three." : "No."); return false; }
    Bubble("The visitor", decoys == 1 ? "...What was that?" : "...Again?");
    AddClue("decoy");
    Achieve("Lost Your Marbles");
    return true;
}

void Story::OnLeftPremises() {
    if (!chase) return;
    chase = false;
    Say("", "(Past the fence. Snow to your knees. You're actually doing it.)");
    Say("", "(Then it's all white. No house. No fence. Just footprints ahead of you. Small ones. Yours.)");
    Sound("thud");
    Say("The visitor", "(out of the white, carrying you back like a sack of potatoes) 31 kilometres, 06. In this? You walked in a circle.");
    AddClue("run_fail");
    Achieve("Nowhere to Run");
    lastEnd = "vidda";
    CollectionScript();
}

void Story::OnVisitorUp() {
    if (!visitorDowned || HoldDown()) return;
    visitorDowned = false;
    Say("The visitor", "(getting up, slow, one arm hanging wrong) ...Okay. That one hurt.");
}

// v0.10: what he does to the handlers and to Doggo, as black screen lines. True if they lived
bool Story::CleanUp() {
    bool momLeft = MomHere() || momLocked || momBathLocked;
    bool dadLeft = !dadDown;
    if (Spared()) {
        Black("He stands in the doorway with you over his shoulder and looks at them for a long time.");
        Black("\"No confirmation came through.\" He says it to nobody. \"Pickup only, then.\"");
        Black("He nods at them. \"God jul.\"");
        if (stance == Stance::Dark) Black("They stand in the door and watch him carry you to the car. Nobody waves.");
        else Black("They stand in the door and watch him carry you to the car. Mom lifts her hand. Like you're going to camp.");
        if (DoggoExists()) Black("Doggo follows you all the way to the gate. He lets him.");
        if (shift == Shift::Sister) Black("Upstairs, behind the bathroom door, somebody is still singing.");
        AddClue("off_hook");
        Achieve("Clerical Error");
        lastEnd = "spared";
        return true;
    }
    bool named = false;
    if (momLocked) Black("He goes down to the basement. He knows exactly where she is.");
    if (dadLocked) Black("He goes out to the shed. He knows exactly where Dad is.");
    if (momBathLocked) Black(bathLockDark ? "He doesn't bother with the key. Two kicks and the bathroom door is just a hole."
                                          : "He opens the bathroom door last. She is sitting on the edge of the bath. She stands up for him.");
    if (!momLocked && MomHere()) {
        if (momRemorse) {
            Black("He walks Mom into the kitchen. In the doorway she turns around and says a name.");
            Black("Not Kiddo. Not sweetie. Not 06. A name. Yours.");
            named = true;
        }
        else if (tainted) Black("He walks Mom into the kitchen. She doesn't look at you on the way.");
        else Black("He walks Mom into the kitchen. She goes quietly. Like she doesn't understand yet.");
    }
    if (!dadLocked && dadLeft && dadOnFloor) Black("He drags Dad in after her. Dad is trying to say your name.");
    else if (!dadLocked && dadLeft) Black("Dad goes too. He tries to say something to you. The man shakes his head.");
    if (DoggoExists()) Black("Doggo follows them. Wagging.");
    if (doggoDead) Black("You look for Doggo. Then you remember.");
    bool doggoSafe = doggoOut && shift != Shift::NoDoggo && !doggoGone && !doggoDead;
    if (shift == Shift::Sister) {
        Black("He counts on his clipboard. Two handlers. One subject. He looks up at the bathroom door for a long time.");
        Black("He doesn't open it. He writes something down.");
    }
    if (momLeft) { Sound("bang"); Black(named ? "The bang comes before you can keep it." : "BANG."); }
    if (dadLeft) { Sound("bang"); Black("BANG."); }
    if (DoggoExists()) { Sound("bang"); Black("Doggo stops barking."); }
    if (doggoSafe) {
        Black("Somewhere out past the fence, barking. Getting further away. He isn't coming back for this.");
        AddClue("doggo_out");
        Achieve("Run, Boy");
    }
    if (named) { AddClue("my_name"); Achieve("Say It Again"); lastEnd = "name"; }
    if (stance == Stance::Normal && (momLeft || dadLeft)) Black("They never knew they were part of the clean-up. Somehow that's worse.");
    return false;
}

void Story::CollectionScript() {
    chase = false;
    FadeFor(0, "");
    Sound("hush_on");   // v0.9: no music for this part
    Black("Rope. The couch. He ties knots like he's done it a thousand times.");
    if (!CleanUp()) Black("He carries you out into the storm.");
    // v0.19: what you did to his car
    if (carKey == "snow") { AddClue("visitor_time"); NoKeyEnding(); return; }
    if (carKey == "pocket") Black("At the car he holds out his hand. You give him the key. He doesn't even look angry.");
    if (carKey == "woodpile") {
        Black("On the way down he walks straight to the woodpile and puts his arm in, third log from the top. He doesn't look.");
        Black("\"I always picked the woodpile too.\"");
    }
    Black("The trunk smells like oil and wet dog.");
    AddClue("visitor_time");
    if (BrakesGo("trunk")) return;   // v0.20 knife: you cut his brakes before he knocked
    if (FullMemory()) {
        Black("The lid comes down. A hand comes down too. Stay awake. STAY AWAKE.");
        Beat q; q.type = BeatType::QTE; q.text = "STAY AWAKE"; beats.push_back(q);
        return;
    }
    TrunkScript();
}

// v0.10: the joke one. You save him the trouble
void Story::ExpressDelivery() {
    stance = CurrentStance();
    t = (float)visitorTime;
    collection = true; grounded = false; chase = false;
    FadeFor(0, "");
    Sound("hush_on");
    Black("You climb in. You pull the lid down yourself. It's easier than you thought.");
    Black("Dark. Oil. A blanket that smells like other people. You wait.");
    Sound("knock");
    Black("Far away: three slow knocks. Voices. Then nothing for a while.");
    if (Spared()) {
        Black("You wait for the bangs. They don't come.");
        AddClue("off_hook");
        Achieve("Clerical Error");
    } else {
        if (MomHere() || momLocked || momBathLocked) { Sound("bang"); Black("BANG."); }
        Sound("bang"); Black("BANG.");
        if (DoggoExists()) { Sound("bang"); Black("Doggo stops barking."); }
    }
    Black("Footsteps in the snow. The lid opens.");
    Black("He looks at you. He looks at his clipboard. He looks at you again.");
    Black("\"...Huh. That's a first.\"");
    Black("He starts to say something else and stops. He frowns, like a man who has lost his place in a book.");
    Black("He closes the lid. Gently, this time.");
    AddClue("visitor_time");
    Achieve("Express Delivery");
    lastEnd = "express";
    if (BrakesGo("trunk")) return;   // v0.20 knife
    TrunkScript(true);
}

// v0.10: the clean ending. Nobody is saved. That was never on the table
void Story::DinnerEnding() {
    chase = false;
    solemn = true;
    Say("You", "No. We don't run. I've seen where that road ends. Thirty-one kilometres. For all three of us.");
    Say("Dad", "(his hand goes to the place behind his ear. Then he puts the lamp down.)");
    Say("Mom", "...Then what do we do?");
    Say("You", "It's Christmas Eve. We sit down.");
    solemn = false;
    FadeFor(0, "");
    Sound("hush_on");
    Black("Mom lights the candles, because the power is out. Dad carves the ribbe. His hands only shake a little.");
    Sound("knock");
    Black("He knocks again. Nobody gets up. After a while he lets himself in.");
    Black("He stands in the kitchen doorway with snow on his shoulders and looks at the table. Four chairs. Three plates.");
    Black("\"Has he been good?\"");
    Black("\"He's the best thing we ever did,\" Mom says. \"Sit down or get out.\"");
    Black("He looks at his watch. Then he pulls out the fourth chair and sits. He keeps his coat on.");
    Black("Nobody talks. Dad passes him the potatoes. He takes two. He holds his fork the way Dad does.");   // v0.20 story: the last sentence
    if (DoggoExists()) Black("Doggo lies on your feet under the table. He growls the whole time, very quietly, and nobody tells him to stop.");
    Sound("hum_on");
    Black(Knows("the_tape") ? "Mom hums. You let her. Just this once." : "Mom hums. The carol she always hums.");
    Sound("hum_off");
    Sound("bells");
    Black("Then, far away, through the storm: bells. Every church in the country.");
    Black("You have never heard them from inside the house. Not once, in all those hours.");
    Black("\"God jul,\" says Dad. His voice breaks on it.");
    Black("The man puts his fork down. \"Time.\"");
    Black("You get up by yourself. You put your jacket on by yourself. Mom zips it to your chin.");
    Sound("beep");
    Black("BEEP BEEP.");
    Narrate("It didn't save anyone. It wasn't for that.");
    Narrate("Out of all those hours, that is the one I would keep.");
    AddClue("three_plates");   // v0.20 story
    endings.insert("bells");
    Achieve("One Good Hour");
    lastEnd = "ending:bells";
    Beat e; e.type = BeatType::Ending; e.id = "bells"; e.speaker = "ENDING";
    e.text = "ONE GOOD HOUR\n\nThey sat down. He heard the bells.";
    beats.push_back(e);
}

void Story::ShowWrist(bool downed) {
    if (downed) Say("", "(His glove came off when he fell. On his wrist: an Origin. Scratched grey. The strap has been replaced so many times it doesn't match.)");
    else {
        Say("The visitor", "(he looks at the tiny watch for a long time) They kept that? Sentimental.");
        Say("", "(He pulls his glove back. An Origin. Scratched grey. The strap has been replaced so many times it doesn't match.)");
    }
    Say("The visitor", downed ? "(through his teeth) They don't come off, 06. You grow into them." : "They don't come off, 06. You grow into them.");
    Say("You", "(He was one of us. He was a kid in a house like this one.)");
    AddClue("collector");
}

void Story::TrunkScript(bool quiet) {
    if (lastEnd.empty()) lastEnd = "trunk";
    if (!quiet) {
        Sound("thud");
        Black("The lid comes down. Something hits your head.");
    }
    Sound("bells");
    Black("Far away, through the storm, church bells. Christmas is being rung in.");
    static const char* lines1[] = {
        "No. That's not how it went. Let me start again.",
        "No, wait. I'm telling it wrong. From the top.",
        "That's not... no. Again.",
    };
    static const char* lines2[] = {
        "Again.",
        "I knew how it ended. I just kept letting it.",
        "They always went quietly. Every single time.",
    };
    static const char* lines3[] = {
        "Yeah. That's how it went. I'm not fixing it.",
        "He took me. Again. Let him think he's winning.",
        "I let it stand. They had it coming. Some of them.",
    };
    int st = Stage();
    Narrate((st >= 3 ? lines3 : st >= 2 ? lines2 : lines1)[loop % 3]);
    Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
}

void Story::QTEResult(bool stayedAwake) {
    if (stayedAwake) { SecretEnding(); return; }
    Black("You almost made it.");
    TrunkScript();
}

void Story::SecretEnding() {
    Black("You don't black out. You pretend to.");
    Black("Two hours of snow chains. Then asphalt. Then a gate. A voice: \"Subject 06, delivered.\"");
    Black("Finse. A building that isn't on any map.");
    Black("You've never been here. You know every hallway anyway. You've had a lot of practice dying.");
    Black("Rooms with kids in them. Twelve years old, all of them. All wearing the same watch.");
    Black("You find the machine. It hums like the basement.");
    Sound("glass");
    Black("You break it. Then you break everything else. You don't stop for a long time.");
    Black("You open every door. They run out into the snow. Nobody stops them. Nobody's left to.");
    Black("Outside the sky is pink. You did it. You actually");
    Black("A red dot on your chest.");
    Sound("bang");
    Flash();
    Sound("beep");
    Black("BEEP BEEP.");
    Black("16:00. Your bed. Your room. Your watch.");
    Black("You scream until there's no sound left. Downstairs, Mom calls your name.");
    Narrate("Whatever was still soft in me stayed in that building. Only the dark came back.");
    Narrate("The longest surviving version of me is still in that bed. He always will be.");
    darkPoints = DARK_MAX;   // v0.7: the darkest ending. You wake up as dark as it gets
    tainted = true;
    endings.insert("secret");
    lastEnd = "ending:secret";
    Achieve("The Longest Survivor");
    Beat e; e.type = BeatType::Ending; e.id = "secret"; e.speaker = "SECRET ENDING";
    e.text = "THE LONGEST SURVIVOR\n\nHe set them free. He never got to leave.";
    beats.push_back(e);
}

void Story::MonsterEnding() {
    bool endProgram = Knows("basement_files") && Knows("program_hq");
    darkPoints = DARK_MAX;
    FadeFor(0, "");
    Black("You don't remember deciding. Just the sound.");
    Black("He doesn't get up.");
    if (MomHere()) {
        Black("Mom comes out of the kitchen. She looks at you like you're the thing that came through the door.");
        Black("\"What ARE you?\"");
    }
    if (!endProgram) {
        Black("The storm blows itself out by morning. You walk out into the white. Nobody follows. Nobody calls your name.");
        if (Knows("graduation")) {
            // v0.10: you read Appendix C. You know what this was
            Black("His car is still at the gate. On the seat, his radio crackles. \"Collector, confirm.\"");
            Black("You look at it for a long time. Then you press the button.");
            Black("\"Confirmed.\" Your voice doesn't shake at all.");
            Narrate("I survived. They had a job waiting for the one who did.");
        } else {
            Black("His car is still at the gate. On the seat, his radio crackles. \"Collector, confirm.\" You don't touch it. It asks all night.");   // v0.20 story
            Narrate("I survived. That's all I did.");
        }
        AddClue("confirmed");   // v0.20 story: the ending teaches the word
        endings.insert("monster");
        lastEnd = "ending:monster";
        Achieve("Monster");
        Beat e; e.type = BeatType::Ending; e.id = "monster"; e.speaker = "ENDING";
        e.text = Knows("graduation") ? "MONSTER\n\nHe lived. That was the test." : "MONSTER\n\nHe lived. Nobody wanted him to.";
        beats.push_back(e);
        return;
    }
    Black("His keys. His radio. The files said Finse.");
    Black("You can't drive. You learn. It takes nine loops.");
    Black("Loop after loop. The gate code. The guard rota. The hallway that creaks.");
    // v0.20 story: what the nine loops taught
    Black("The radio on the seat asks for one word. \"Collector, confirm.\" You learn to say it. After that every gate opens.");
    Black("The gate code is 0612. Week 612. They used your week.");
    AddClue("confirmed");
    AddClue("finse_gate");
    Black("You find the machine. It hums like the basement.");
    Sound("glass");
    Black("You break it. You keep breaking it after it stops humming.");
    if (Knows("graduation")) Black("A speaker in the ceiling, over the alarms. A calm voice. \"Outstanding, 06.\"");
    Narrate("I thought that would end it. The loop. Mine.");
    Sound("beep");
    Black("BEEP BEEP. 16:00.");
    Narrate("It didn't. It only ended theirs.");
    endings.insert("program");
    lastEnd = "ending:program";
    Achieve("End of the Program");
    Beat e; e.type = BeatType::Ending; e.id = "program"; e.speaker = "ENDING";
    e.text = "END OF THE PROGRAM\n\nThe machine is gone. The loop isn't.";
    beats.push_back(e);
}

void Story::MinigameDone(int score) {
    FadeFor(10, "Playing Flappy Plane");
    if (score >= 10) Say("You", "(" + std::to_string(score) + " towers. New personal best. Nobody will ever know.)");
    else if (score > 0) Say("You", "(" + std::to_string(score) + (score == 1 ? " tower" : " towers") + ". The jet is harder than it looks.)");
    else Say("You", "(Zero. The paper plane hates me.)");
    FliptendoPlayed("flappy");
}

void Story::FliptendoPlayed(const std::string& what) {
    (void)what;
    playStreak++;
    if (playStreak == 2) Say("You", "(Okay. One more. Then I'll do something.)");
    if (playStreak >= 3 && !collection) ScreenTimeEnding();
}

void Story::ScreenTimeEnding() {
    FadeFor(0, "");
    Black("One more. Then one more. Then one more.");
    Black("Somewhere far away, three slow knocks.");
    Black("Mom's voice. \"...You're early.\"");
    Black("A big hand on your shoulder. \"Come on, 06.\"");
    Black("\"Can I bring my Fliptendo?\"");
    Black("He shrugs. \"Sure.\"");
    Black("You play the whole drive. You never look up. You never find out where you went.");
    Black("At the third dungeon he leans over without looking. \"Under the well.\" He's right. Nobody knows that.");   // v0.20 story
    AddClue("under_well");
    Narrate("The longest loop I ever had was the one where I didn't notice.");
    endings.insert("screen");
    lastEnd = "ending:screen";
    Achieve("Screen Time");
    Beat e; e.type = BeatType::Ending; e.id = "screen"; e.speaker = "JOKE ENDING";
    e.text = "SCREEN TIME\n\nHe reached the last dungeon. He didn't notice anything else.";
    beats.push_back(e);
}

void Story::TiredOut() {
    // v0.6: the clock keeps going during the chase. At 17:00 the kid just runs out.
    chase = false;
    Sound("bells");
    Say("", "(17:00. Far away, bells. You've run. You've hidden. You've run again.)");
    Say("", "(Your legs just stop. You curl up in the snow behind the woodpile for one second.)");
    Say("The visitor", "(much later, gently) There you are. Long day, huh?");
    AddClue("outlasted");
    Achieve("Tired Out");
    CollectionScript();
}

void Story::Threat(int n) {
    // v0.7: he can't find you, so he makes them pay. A blink of black hides him walking back to them.
    threats = n;
    See("threat");
    bool dad = DadFree() || dadOnFloor, mom = MomHere() && !momLocked;
    bool bath = momBathLocked;   // v0.13: she is behind a door he can kick
    FadeFor(0, "");
    if (n == 1) {
        Sound("knock");
        Say("The visitor", std::string("(loud, from the living room) Zero-six! I'm done looking. Come out, or I start on your ") + (dadOnFloor ? "dad again." : dad ? "dad." : (mom || bath) ? "mom." : "family."));
        Bubble("Doggo", "*whimpers*");
    } else if (n == 2) {
        Sound("crack");
        if (dad) { Say("", "(A sound from the living room. Dad screams. Then he stops screaming.)"); dadOnFloor = true; dadDown = true; }
        else if (mom) { Say("", "(Mom's voice. Begging. Then a crack. Then just crying.)"); momOnFloor = true; }
        else if (bath) { Say("", "(The bathroom door. One kick. Two. Mom screams your name through the splinters. Then a crack.)"); momOnFloor = true; }
        else Say("", "(Something heavy breaks downstairs. He's not looking anymore. He's just breaking things.)");
        DarkAct(1, "", true);
        Say("You", darkPoints >= DARK_REVEL ? "(I could come out. I don't want to.)" : "(I could come out. I could. I'm still hiding.)");
        Say("The visitor", (mom || bath) && dad ? "Come out, 06. Mom's next." : "Come out, 06.");
    } else {
        Sound("crack");
        if (bath && !momOnFloor) { Say("", "(The bathroom door gives. Mom. Begging. Then a crack.)"); momOnFloor = true; }
        else if (mom && !momOnFloor) { Say("", "(Mom. Begging. Then a crack. Then just crying, very quietly.)"); momOnFloor = true; }
        else if (dad) Say("", "(Dad again. You put your hands over your ears. It doesn't help.)");
        else Say("", "(Glass. Wood. Your room, probably. He knows where you sleep.)");
        DarkAct(1, "", true);
        Say("You", darkPoints >= DARK_REVEL ? "(Let him.)" : "(Hands over my ears. It doesn't help. Nothing helps.)");
        Say("The visitor", "(quiet now) I'm not asking anymore.");
        AddClue("they_paid");
    }
}

void Story::PaidEnding() {
    chase = false;
    Sound("bells");
    Say("", "(17:00. Far away, the bells start. The house is completely quiet.)");
    Say("", "(Footsteps. Slow. He doesn't search anymore. He walks straight to your hiding spot. He always knew.)");
    AddClue("outlasted");
    Say("The visitor", "(crouching down) You let them pay for you. Huh.");
    Say("The visitor", "They said you were different. You're not. You're just like us.");
    FadeFor(0, "");
    Black("He carries you out past them.");
    if (MomHere() || momOnFloor) Black("Mom is still crying. She doesn't look up.");
    if (dadOnFloor) Black("Dad isn't moving.");
    Black("You don't cry. You notice that you don't cry.");
    AddClue("just_like_us");   // v0.20 story
    DarkAct(2, "", true);
    endings.insert("paid");
    lastEnd = "ending:paid";
    Achieve("They Paid");
    Beat e; e.type = BeatType::Ending; e.id = "paid"; e.speaker = "ENDING";
    e.text = "THEY PAID\n\nHe stayed hidden. Someone always pays.";
    beats.push_back(e);
}

void Story::EmptyHouseEnding() {
    // v0.15.2 (Noah): the special ending for lasting the whole hunt with nobody for him to hurt
    chase = false;
    Sound("bells");
    Say("", "(17:00. Far away, the bells start.)");
    Say("", "(Downstairs, his footsteps stop. They don't come any closer. For the first time all hour he isn't looking for you.)");
    Say("", "(The radio on his belt clicks.)");
    Say("Finse", "(on the radio) Collector. Status.");
    Say("The visitor", "Subject not recovered. Handlers secured, out of reach. The dog is gone. No leverage.");
    Say("Finse", "(a long pause) ...Hold position. The cycle closes on its own.");
    Say("", "(A chair creaks. Dad's chair, by the wood stove. He has sat down.)");
    Say("The visitor", "(to the empty living room, not loud) You did it right, 06. Nobody to make pay. No dog to follow. Just you and the cold.");
    Say("The visitor", "I never got this far. I always came out for them.");
    if (HingePieces() >= 3) Say("The visitor", "(very quiet) Mom used to hum under the duvet. I don't know why I remember that.");
    Say("The visitor", "See you at four.");
    FadeFor(0, "");
    Black("For one second the whole house is quiet. Nobody in it is afraid of anything.");
    Black("Mom isn't crying in the basement. Dad isn't banging on the shed door. They're listening too.");
    Black("You stay where you are. You don't come out. You don't need to.");
    Sound("beep");
    Narrate("I won. Nobody told me it would feel exactly like losing.");
    AddClue("nobody_left");
    endings.insert("empty");
    lastEnd = "ending:empty";
    Achieve("Nobody Left");
    Beat e; e.type = BeatType::Ending; e.id = "empty"; e.speaker = "ENDING";
    e.text = "THE EMPTY HOUSE\n\nNobody to make pay. Nobody to follow. He sat down and waited with you.";
    beats.push_back(e);
}

void Story::EscapeEnding() {
    chase = false;
    FadeFor(0, "");
    Black("Dad swings the lamp before the man is all the way through the door. It doesn't knock him out. It doesn't have to.");
    if (carKey == "pocket") Black("You already have his key. You press it into Mom's hand. You're already running.");
    else if (carKey == "woodpile") Black("Dad grabs the key out of the woodpile on the way past. You told him where. You're already running.");
    else if (carKey == "snow") Black("His key is somewhere in the snow. Mom finds a spare under the sun visor. You're already running.");
    else Black("Mom already has his keys. You're already running.");
    Black("The car smells like oil and wet dog. Mom drives into the storm. Dad holds his ribs in the back seat and laughs.");
    Black("You've never heard him laugh like that. Like it isn't for you.");
    if (DoggoExists()) Black("Doggo is in your lap. His heart is going as fast as yours.");
    else if (doggoOut && shift != Shift::NoDoggo && !doggoGone) Black("At the gate a shape comes out of the snow, running flat out beside the car. Dad opens the door. Doggo lands in your lap, soaked.");
    if (BrakesGo("escape")) return;   // v0.20 knife
    Black("His tracks are already filling in. The road is closing behind you. Nobody follows. Nobody could.");
    Black("Mom keeps checking the mirror. After a while she stops checking.");
    Black("\"Where do we go?\" \"Anywhere. Anywhere at all.\"");
    Black("A road sign in the headlights. 31.");
    Sound("beep");
    Black("Beep. From behind your ear. From behind Mom's ear. From behind Dad's.");
    Black("Mom touches the little scar behind her own ear. She didn't know. None of them did.");
    Flash();
    Sound("bang");
    Black("White.");
    Narrate("31 kilometres. Exactly as far as the leash goes. For all of us.");
    AddClue("leash");   // v0.10: the ending teaches you something you can use
    lastEnd = "ending:escape";
    endings.insert("escape");
    Achieve("False Hope");
    Beat e; e.type = BeatType::Ending; e.id = "escape"; e.speaker = "ENDING";
    e.text = "FALSE HOPE\n\nThey ran together. The leash was exactly 31 kilometres long.";
    beats.push_back(e);
}

void Story::DoggoShot(bool onScreen) {
    // v0.7: hurt him and he takes Doggo first. Off screen. Then you see what's left.
    doggoDead = true;
    if (onScreen) { doggoDeadX = 19; doggoDeadY = 19; }
    Say("Doggo", "*goes for him. Barking. Teeth. Everything he has.*");
    Sound("bang");
    FadeFor(0, "");
    SceneBeat("doggo_dies");   // the presentation swaps in the body during the black
    Black("BANG.");
    Black("A yelp. Short. Then nothing.");
    if (onScreen) {
        Say("", "(Doggo is lying by the door. He isn't moving. A thin dark line is running out from under him.)");
        Say("You", "(Get up. Get up, boy. Please.)");
        Say("The visitor", "(stepping over him) That was on you, 06.");
    }
}

void Story::DoggoSacrifice() {
    // v0.7: he was in the room. He always is, unless you tell him to stay.
    // v0.9: the first time the loop carries on, so you learn it before it costs you an ending
    bool first = !Knows("doggo_stay");
    chase = false;
    doggoDead = true;
    Say("", "(His hand is already reaching for you.)");
    Say("Doggo", "*comes out of nowhere. Between you and him. Teeth.*");
    Sound("bang");
    FadeFor(0, "");
    SceneBeat("doggo_dies");   // the presentation swaps in the body during the black
    Black("BANG.");
    Black("A yelp. Short. Then nothing.");
    Say("", "(Doggo is lying at your feet. He isn't moving. A thin dark line is running out from under him.)");
    Say("You", "(No. No no no. Get up. GET UP.)");
    Say("The visitor", "(wiping his sleeve) ...Good dog. Shame.");
    AddClue("doggo_stay");
    if (first) {
        Say("You", "(He got in front of me. I never told him to stay.)");
        lastEnd = "doggo";
        CollectionScript();
        return;
    }
    lastEnd = "ending:doggo";
    Achieve("Good Boy");
    FadeFor(0, "");
    Black("You don't fight when he picks you up. You can't stop looking back.");
    Black("He looks back too. Just once. \"I had one like him,\" he says. \"Same spot by the door.\"");   // v0.20 story
    AddClue("his_dog");
    Narrate("He was the only one in that house who never lied to me.");
    endings.insert("doggo");
    Beat e; e.type = BeatType::Ending; e.id = "doggo"; e.speaker = "REALLY BAD ENDING";
    e.text = "GOOD BOY\n\nHe did the only thing he knew how to do.\n\nNext time, tell him to stay. Or shut a door.";
    beats.push_back(e);
}

void Story::ConvinceParents(const std::string& who) {
    // v0.7: befriended + proof. They didn't know they were part of the clean-up.
    bool proofPhone = Knows("phone_answer");
    bool proofFiles = Knows("basement_files"), proofCar = Knows("his_car");
    solemn = true;
    Say("You", "I need you to listen. Really listen. The man at 16:50. The clean-up. It's not just me. It's you too.");
    if (tainted) Narrate("I said it the way they taught me to say things. Soft. Scared. Every word exactly where it would hurt. It worked.");
    Say(who, who == "Mom" ? "(she laughs, then stops) Clean-up is... no. We're being reassigned. They promised." : "(he rubs his face) Buddy, no. We're done after today. We retire. They promised.");
    if (proofPhone) Say("You", "\"Handler two, confirm pickup. Clean-up included.\" That's what they said on the phone. Clean-up is you.");
    else if (proofFiles) Say("You", "The files in the basement. Handlers are not to be informed. They think they're retiring. That's you.");
    else if (proofCar) Say("You", "His clipboard. I've seen it. 06: collect one, clean two. I'm the one. You're the two.");
    else {
        Say("You", "Feel behind your ear. Go on. The little scar. I've got one too. So has " + std::string(who == "Mom" ? "Dad" : "Mom") + ".");
        Say(who, "(a hand goes up. It stops there.)");
        Say("You", "We ran once. All three of us. We got 31 kilometres. Then all three of them beeped.");
    }
    Say(who, proofPhone || proofFiles ? "(very quietly) ...How do you know those words?" : "(very quietly) ...How do you know that?");
    // v0.18 (section 3): one at a time. Each of them has to hear it from you
    bool other = who == "Mom" ? dadTurned : momTurned;
    if (who == "Mom") momTurned = true; else dadTurned = true;
    AddClue("allies");
    if (!other) {
        if (who == "Mom") {
            Say("Mom", "(she sits down on the kitchen floor. Just sits.)");
            Say("Mom", "...Don't tell your father. Not yet. I don't know which side he'd pick.");
        } else {
            Say("Dad", "(he puts the wrench down very carefully)");
            Say("Dad", "...Not a word to your mother. Not yet. She'd tell them. She wouldn't mean to.");
        }
        Say("You", "(One of them. That's one more than I had this morning.)");
        solemn = false;
        return;
    }
    if (who == "Mom") { Say("Mom", "(she sits down on the kitchen floor. Just sits.) ...Does your father know?"); Say("You", "I told him first."); Say("Mom", "...Get him."); }
    else { Say("Dad", "(he puts the wrench down very carefully) ...Does your mother know?"); Say("You", "I told her first."); Say("Dad", "...Get her."); }
    FadeFor(5, "At the kitchen table. All three of you.");
    Say("Dad", "His car. He always leaves the keys in it.");
    Say("Mom", "When he knocks, we go. All three of us. We don't stop for anything.");
    if (Knows("the_tape")) Say("You", "(I'm not doing this because I forgive them. I will never forgive them. I just need them.)");
    else Say("You", "(For one second it feels like having parents.)");
    parentsAllied = true;
    Achieve("Not Alone");
    solemn = false;
}

// ============================================================ v0.18: whose side are they on (branches map, section 3)
// The presentation moves them through the house. The story says what it means when they find you.
std::string Story::Defender() const {
    if (!OneTurned() || stance != Stance::Befriended || !collection || handlers) return "";
    if (momTurned && !momFought && MomHere() && !momOnFloor) return "Mom";
    if (dadTurned && !dadFought && DadFree() && !dadOnFloor) return "Dad";
    return "";
}
static std::string RoomShout(Room r) {
    switch (r) {
        case Room::KidsRoom: return "his room";
        case Room::Bathroom: return "the bathroom";
        case Room::ParentsRoom: return "our room";
        case Room::Hallway: return "the hallway";
        case Room::Kitchen: return "the kitchen";
        case Room::LivingRoom: return "the living room";
        case Room::Shed: return "the shed";
        case Room::Basement: return "the basement";
        case Room::Backyard: return "out back";
        default: return "the yard";
    }
}
void Story::OnParentCall(const std::string& who) {
    static std::map<std::string, int> n;
    int k = n[who]++;
    auto pick = [&](std::vector<const char*> l) { Bubble(who, l[k % l.size()]); };
    if (handlers) {
        if (who == "Mom") pick({"Kiddo. It's just us now. Come here.", "Nobody's going to hurt you. We just have to drive somewhere.", "Please come out. You don't know what they do if we don't bring you."});
        else pick({"I know every spot in this house, buddy. I built half of them.", "Come on out. We're doing this by the book now.", "Finse is a long drive. Let's not make it longer."});
    } else if (ParentsHunt()) {
        if (who == "Mom") pick({"Kiddo? Come out. It's easier.", "Sweetie, please. Don't make him look.", "You're not in trouble. Just come out."});
        else pick({"Not this way, buddy.", "Come on out. Make it easy on everybody.", "I'm not moving, kiddo."});
    } else if (who == "Mom") pick({"(very quietly) Kiddo? It's okay. Come out.", "He's not going to hurt you, sweetie.", "(to herself) Please just come out."});
}
void Story::OnParentSpots(const std::string& who, Room r) {
    if (!chase) return;
    if (handlers) Bubble(who, who == "Mom" ? "Here! He's here!" : "There you are.");
    else Bubble(who, who == "Mom" ? "He's in " + RoomShout(r) + "! In " + RoomShout(r) + "!" : "Buddy's in " + RoomShout(r) + "!");
    AddClue("they_hunt");
}
void Story::OnParentFinds(const std::string& who) {
    if (!chase) return;
    AddClue("they_hunt");
    if (handlers) {
        chase = false;
        Say("", who == "Mom" ? "(Light. Mom's face. She's been crying. She doesn't stop now.)" : "(Light. Dad's big hand. He doesn't grab. He just holds it out.)");
        Say(who, who == "Mom" ? "There you are. Come on. Come on, sweetie." : "Up you get, buddy.");
        HandlersEnding(true);
        return;
    }
    Bubble(who, who == "Mom" ? "(she lifts the edge and looks right at you) ...Oh, sweetie." : "(he looks right at you) ...Found him.");
    Bubble(who, who == "Mom" ? "He's in here! In here!" : "Over here!");
}
void Story::OnParentGrabs(const std::string& who) {
    if (!chase) return;
    chase = false;
    AddClue("they_hunt");
    if (who == "Dad") Say("", "(Dad's arms close around you from behind. Not hard. Just enough.)");
    else Say("", "(Mom's hand closes on your sleeve. She's shaking. She doesn't let go.)");
    if (handlers) { Say(who, who == "Dad" ? "Got him. ...I've got him." : "I've got him. I've got him."); HandlersEnding(true); return; }
    Say(who, who == "Dad" ? "(over his shoulder) Got him. ...I've got him." : "(very quietly) I'm sorry. I'm so sorry. Over here.");
    Say("The visitor", "(walking up, not hurrying) Thank you, handler " + std::string(who == "Dad" ? "one." : "two."));
    if (who == "Dad") Say("You", "(He held the door. He held me. He never once looked at me.)");
    CollectionScript();
}
void Story::OnDefend(const std::string& who) {
    if (!chase) return;
    Bubble(who, who == "Mom" ? "GET AWAY FROM HIM!" : "Hey! HEY! Over here!");
}
void Story::OnDefendDone(const std::string& who, int x, int y) {
    if (!chase) return;
    Sound("crack");
    dropX = x; dropY = y;
    if (who == "Mom") {
        momFought = true; momOnFloor = true;
        dropItem = "box";
        Bubble("The visitor", "(breathing hard) ...Handler two. Really?");
        Bubble("You", "(Something black skids across the floor. The box from his belt.)");
        AddClue("mom_fought");
        Achieve("Just Her");
    } else {
        dadFought = true; dadOnFloor = true; dadDown = true;
        dropItem = "taser";
        Bubble("The visitor", "(breathing hard) Should have stayed in the shed, handler one.");
        Bubble("You", "(Something yellow skids across the floor. His taser.)");
        AddClue("dad_fought");
        Achieve("Just Him");
    }
}

void Story::StartHandlers() {
    // Noah's answer 7: you beat him, and they were never on your side. They take over the hunt
    darkPoints = DARK_MAX; tainted = true;
    visitorDead = visitorDowned = true;
    FadeFor(0, "");
    Black("You don't remember deciding. Just the sound.");
    Black("He doesn't get up. The box in your hand keeps blinking green.");
    solemn = true;
    std::string who = DadFree() && !dadOnFloor && !dadTurned ? "Dad" : "Mom";
    if (MomHere() && !momOnFloor) Say("Mom", "(from the doorway, very quietly) ...What did you do.");
    Say("", "(The radio on his belt clicks.)");
    Say("Finse", "(on the radio) Collector. Status.");
    Say("", who == "Dad" ? "(Dad kneels and takes the radio off his belt. He looks at you the whole time.)" : "(Mom kneels and takes the radio off his belt. She looks at you the whole time.)");
    Say(who, "Collector is down. The subject did it.");
    Say("Finse", "(a long pause) ...Then recover the subject. Deliver him yourselves. You know the road.");
    Say(who, "...Copy.");
    if (Knows("graduation")) Say("You", "(Appendix C said whoever kills his collector replaces him. Nobody told them that part.)");
    Say(who, who == "Dad" ? "(he stands up, slowly) Buddy. Put that down. Come here." : "(she stands up, slowly) Sweetie. Put that down. Come here.");
    Say("You", "(No.)");
    solemn = false;
    handlers = true;
    AddClue("handlers_hunt");
    SceneBeat("chase");
}

void Story::HandlersEnding(bool caught) {
    chase = false;
    // whoever was still on their feet and still on his side. At least one of them, or this never started
    bool mom = MomHere() && !momOnFloor && !momTurned, dad = DadFree() && !dadOnFloor && !dadTurned;
    if (!mom && !dad) dad = true;
    std::string driver = dad ? "Dad" : "Mom";
    if (!caught) {
        Sound("bells");
        Say("", "(17:00. Far away, the bells start.)");
        Say("", "(A torch comes round the corner. His torch. " + driver + " is holding it.)");
        if (dad) Say("Dad", "(not unkindly) Every spot in this house, buddy. I built half of them.");
        else Say("Mom", "(gently) I know where you hide, sweetie. I put the blankets in there.");
    }
    FadeFor(0, "");
    Sound("hush_on");
    // v0.19: you did something to his car. They live here, so it doesn't matter
    if (carKey == "snow") Black(driver + " finds the spare key under the sun visor in less than a minute. Of course there's a spare.");
    else if (carKey == "woodpile") Black(driver + " goes straight to the woodpile for the key. Third log from the top. " + (dad ? "He" : "She") + " doesn't look.");
    else if (carKey == "pocket") Black(driver + " holds out a hand for the key. You give it. You don't know why.");
    if (mom && dad) {
        Black("They don't tie you. They don't have to. Dad carries you out to the man's car. Mom carries the radio.");
        Black(momRemorse ? "Mom sits in the back with you. Halfway there she starts humming. She stops. She starts again."
                         : "Mom sits in the back with you. She keeps a hand on your knee the whole way, like you might float off.");
    } else if (dad) {
        Black("He doesn't tie you. He doesn't have to. Dad carries you out to the man's car by himself.");
        Black("He drives with both hands on the wheel. He doesn't turn the radio on. Once, at a red light that isn't there, he says your name.");
    } else {
        Black("She doesn't tie you. She doesn't have to. Mom carries you out to the man's car. You're too big to carry. She does it anyway.");
        Black(momRemorse ? "She drives. Halfway there she starts humming. She stops. She starts again."
                         : "She drives. She keeps looking at you in the mirror, like you might float off.");
    }
    if (DoggoExists()) Black("Doggo runs after the car all the way to the gate. Then he just stands there in the snow.");
    else if (doggoDead) Black("Nobody says anything about Doggo. Not once, the whole way.");
    if (BrakesGo(mom && dad ? "both" : driver)) return;   // v0.20 knife: his car, their drive
    Black(std::string("Two hours of snow chains. Then a gate. A voice: \"") + (mom && dad ? "Handlers" : "Handler") + "? You're not on the list.\"");
    Black("\"We brought him ourselves,\" " + driver + " says. \"The collector is down.\" " + (dad ? "He says" : "She says") + " it like a thank you is coming.");
    Black("Two people in white coats lift you out of the car. They're very gentle with you.");
    Black(mom && dad ? "They walk Mom and Dad round the back of the building. Like there's something there they need to sign."
                     : "They walk " + driver + " round the back of the building. Like there's something there to sign.");
    Sound("bang");
    Black("BANG.");
    if (mom && dad) { Sound("bang"); Black("BANG."); }
    Black("Handlers don't deliver. Collectors deliver. Nobody ever told them that rule either.");
    Narrate(mom && dad ? "They hunted me for them. They drove me there for them. They still didn't get to go home."
                       : dad ? "He hunted me for them. He drove me there for them. He still didn't get to go home."
                             : "She hunted me for them. She drove me there for them. She still didn't get to go home.");
    endings.insert("handlers");
    lastEnd = "ending:handlers";
    Achieve("Family Business");
    Beat e; e.type = BeatType::Ending; e.id = "handlers"; e.speaker = "ENDING";
    e.text = "THE HANDLERS\n\nHe was gone, so they finished his job. They didn't follow the rules.";
    beats.push_back(e);
}

void Story::LastTollEnding(bool killed) {
    // Noah's answer 7: you convinced them and beat him. False hope. On the last toll you're still in the loop
    chase = false;
    visitorDowned = true;
    bool mom = MomHere() && !momOnFloor, dad = DadFree() && !dadOnFloor;
    solemn = true;
    if (killed) {
        darkPoints = DARK_MAX; tainted = true;
        FadeFor(0, "");
        Black("You don't remember deciding. Just the sound.");
        Black("He doesn't get up. The box in your hand stays green.");
        if (mom) Black("Mom takes the knife out of your hand, very gently, like it's scissors. She washes it in the kitchen sink and puts it back in the block.");
        Black(dad ? "Dad drags him out to the shed by his boots and bars the door. Out of habit." : "Mom drags him out to the shed by his boots. It takes her a long time. She won't let you help.");
    } else {
        Sound("thud");
        if (mom && dad) Say("", "(Dad lands on him with his whole weight. Mom has his wrist bent up behind him before he can push off the floor.)");
        else if (dad) Say("", "(Dad lands on him with his whole weight. You hold one arm with everything you've got. It's like holding a car.)");
        else Say("", "(Mom lands on his arm with both knees. You hold the other one. It's like holding a car.)");
        Say("The visitor", "(face in the floor, almost laughing) Three of you. That's new.");
        Say(dad ? "Dad" : "Mom", "Basement. Now.");
        FadeFor(0, "");
        Black("They bolt him in the basement, with the blue screens and the chair with the straps. Nobody says anything about that.");
        Black("You hear him through the floor for a while. Then you don't.");
    }
    Black("The power is still out. Mom lights the candles. Dad finds the ribbe in the cold oven and carves it anyway.");
    if (!mom) Black("Mom can't get up off the floor, so you all eat on the floor next to her. Dad says it's a picnic.");
    if (!dad) Black("Dad lies on the couch with a dish towel full of snow on his head. He keeps saying he's fine.");
    if (DoggoExists()) Black("Doggo gets a whole plate of his own. Nobody says a word about it.");
    else if (doggoDead) Black("Nobody looks at the hallway, where Doggo is.");
    Black("Riskrem. Dad gets the almond and acts surprised. He gets it every year. Mom always makes sure.");
    Sound("hum_on");
    Black(Knows("the_tape") ? "Mom hums. You know where that song comes from now. You let her anyway. Nobody looks at the clock." : "Mom hums. Dad laughs at nothing. You laugh too. Nobody looks at the clock.");
    Sound("hum_off");
    Black("Except you.");
    Sound("bells");
    Black("17:00. Far away, through the storm, the bells start.");
    Black("Mom counts them out loud, like a game. Dad joins in. You don't.");
    Black("They go on for a long time. Then they slow down. Then there's one more.");
    Black("The last one.");
    Sound("beep");
    Black("BEEP BEEP.");
    Black("16:00. Your bed. Your room. Your watch.");
    Black("Downstairs, Mom is humming. She doesn't remember any of it. Neither does Dad.");
    Narrate("We won. All three of us, alive, at the same table. It didn't count. The hour doesn't care who wins it.");
    if (HingePieces() >= 5 && !Knows("he_remembers")) Narrate("The only one who could have told me why was locked downstairs. I never asked him.");
    solemn = false;
    AddClue("last_toll");
    endings.insert("lasttoll");
    lastEnd = "ending:lasttoll";
    Achieve("Last Toll");
    Beat e; e.type = BeatType::Ending; e.id = "lasttoll"; e.speaker = "ENDING";
    e.text = "LAST TOLL\n\nThey were on your side. He was beaten. The bells rang anyway.";
    beats.push_back(e);
}

void Story::DeadSwitchLoop() {
    // Killing him isn't a way out. Not like this.
    darkPoints = DARK_MAX;
    tainted = true;
    FadeFor(0, "");
    Black("You don't remember deciding. Just the sound.");
    Black("He doesn't get up.");
    Black("Then the box on his belt starts to scream. One long tone.");
    Black("Behind your ear, where the little scar is, something gets hot. Then hotter.");
    Sound("beep");
    Flash();
    Black("White.");
    AddClue("dead_switch");
    lastEnd = "switch";
    Narrate("He took me with him. Of course he did. They'd never send him without a leash on me.");
    Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
}

// ============================================================ v0.17: the true ending
// Branches map, section 6. The final dark choice was already there in pieces: Appendix C, Noah's answer 10,
// the Monster ending's "Confirmed". With all five pieces you get to say them to him while he's on the floor.
// Every line here is Claude's, for Noah to react to.
void Story::TellPiece(const std::string& id) {
    solemn = true;
    toldPieces.push_back(id);
    if (id == "t_hum") {
        Say("You", "(You hum it. Mom's song, the one from the kitchen. You don't know all of it. You hum what you know.)");
        Say("", "(He stops breathing through his teeth. For a second he doesn't breathe at all.)");
        Say("The visitor", "...Under the duvet. She stood in the door until I stopped. She thought I was asleep.");
        Say("The visitor", "(very quiet) That's not yours. Where did you get that?");
    } else if (id == "t_watch") {
        Say("You", "Read the back of your watch.");
        Say("", "(He turns his wrist over. He doesn't need the torch. His thumb finds the letters by itself.)");
        Say("The visitor", "ANCHOR 06. Crooked.");
        Say("The visitor", "I always thought I scratched that myself. In the car, the first night. So I'd have something.");
        Say("You", "Dad did it. With a nail. Four days after. You wouldn't stop kicking.");
        Say("The visitor", "(his thumb keeps going over it) ...Dad.");
    } else if (id == "t_reissue") {
        Say("You", "Do you know what reissue means?");
        Say("The visitor", "(the radio voice, by itself) Wipe. Age. Reissue.");
        Say("You", "Same house. Next cycle.");
        Say("", "(He looks up past you, at the hall ceiling. At the crack in the light fitting.)");
        Say("The visitor", "There's a crack up there shaped like a Y. I know that crack. How do I know that crack?");
    } else if (id == "t_dose") {
        Say("You", "You haven't had your dose.");
        Say("", "(His hand goes to his coat. To the pen with the needle. It stays there.)");
        Say("The visitor", "After. They want it after.");
        Say("You", "That's why you know where I hide. The house is still in your head.");
        Say("The visitor", "(a long breath out) ...Every spot. I don't know how I know. I just walk there.");
    } else if (id == "t_last") {
        Say("You", "This is your last pickup. Finse said so. I was in your car.");
        Say("The visitor", "(it's almost a laugh) Under the blanket. Of course. I always was too.");
        Say("The visitor", "Then where is he? The new collector?");
        Say("", "(You don't say anything. You don't have to.)");
        Say("The visitor", "(he looks at you for a long time) ...Oh.");
    }
    solemn = false;
    if (toldPieces.size() >= 5) Remember();
    else VisitorChoice();
}

void Story::Remember() {
    solemn = true;
    remembered = true;
    Say("", "(Something goes out of his face. The calm. The script. What's left looks about twelve.)");
    Say("The visitor", "The third stair creaks. Mom hums when she thinks nobody's listening. Dad says copy, no change, into a radio in the shed.");
    Say("The visitor", "There was a dog. Not this one. A long time before this one.");
    Say("The visitor", "I lived here.");
    AddClue("he_remembers");
    Achieve("He Remembers");
    // answer 10: the line nobody else hears
    bool momSees = MomHere() && !momOnFloor, dadSees = DadFree() && !dadOnFloor;
    if (momSees || dadSees)
        Say("", std::string("(He takes your collar, gently, and pulls you down until his mouth is at your ear. ") +
                (momSees && dadSees ? "Mom and Dad are right there." : momSees ? "Mom is right there." : "Dad is right there.") + " They don't hear a word of it.)");
    else Say("", "(He takes your collar, gently, and pulls you down until his mouth is at your ear. There's nobody left in the house who could hear. He whispers anyway.)");
    Say("The visitor", "(whispering) I stood where you're standing. He was down here, where I am. Cut lip. Crooked watch.");
    Say("The visitor", "(whispering) He told me the song, the watch, reissue, the dose, the last pickup. The same five. I didn't believe him either.");
    Say("The visitor", switchTaken ? "(whispering) You took the box first. Good. So did I." : "(whispering) Always the box first. Always.");
    if (Knows("grave_05")) Say("The visitor", "(whispering) Zero-five didn't know about the box. That's her out back, by the stream. They had me dig it.");
    Say("The visitor", "(whispering) Next time you'll sit in a car at that gate for two minutes, breathing like you're counting. You'll look in that window and see a kid in a red sweater, and you'll think: that's not me. That was never me.");
    Say("The visitor", "(whispering) And one day he opens the door before you knock. One of us always does, in the end.");
    if (Knows("graduation")) Say("The visitor", "(whispering) They'll say outstanding. They said it to me.");
    Say("The visitor", "(out loud now, tired) Your move, 06. It was mine once.");
    if (!switchTaken) {
        switchTaken = true;
        Say("", "(He unclips the black box from his belt himself and puts it in your hand. The green light keeps blinking.)");
        Say("The visitor", "Don't drop it. That's the one thing I got right.");
    }
    solemn = false;
    std::vector<ChoiceOpt> o;
    // answer 10: the kill only happens with nobody else in the room. Sparing him can happen in front of anyone
    if (ParentsOut()) o.push_back({"t_kill", hasKnife ? "Finish it." : "Take the gun out of his coat.", false, true});
    else Say("You", momSees ? "(Not with Mom standing right there. Not in front of her.)" : "(Not with Dad watching. Not in front of him.)");
    o.push_back({"t_spare", hasKnife ? "Put the knife down." : "Sit down next to him."});
    Choice("He's waiting. So is the house.", o);
}

void Story::SameHouseEnding() {
    // finish it. You take the box, the coat and the job (Appendix C, D2, D4)
    chase = false;
    solemn = true;
    int darkBefore = darkPoints;
    bool knife = hasKnife;
    std::string first = toldPieces.empty() ? "" : toldPieces.front();
    darkPoints = DARK_MAX;
    tainted = true;
    FadeFor(0, "");
    if (knife) Black("This time you remember deciding.");
    else Black("You take the gun out of his coat. He lets you. He even shows you where the safety is.");
    Black("He doesn't close his eyes. He watches you the whole time, like he's checking you do it right.");
    Sound("bang");
    Black("He doesn't get up. The box in your hand keeps blinking green. Nothing burns behind your ear.");
    // where they are
    if (momLocked) Black("Down in the basement, behind the bolt, Mom has stopped knocking. She heard it. She knows which one of you it was.");
    else if (momBathLocked) Black(momRemorse ? "Behind the bathroom door Mom is humming. The song. Very quietly. She's guessed."
                                             : "Behind the bathroom door Mom asks who's out there. Nobody answers her. Both answers would be true.");
    else if (momOnFloor) Black("Mom is still on the floor. She'll wake up to this. Or she won't.");
    if (shift == Shift::Sister) Black("Upstairs, behind a door, somebody is still singing.");
    if (dadLocked) Black("Out in the shed Dad has stopped shouting. Through the gap in the boards his torch is pointed at the house. At the door. At you.");
    else if (dadOnFloor || dadDown) Black("Dad is still on the floor. He's breathing. He'll wake up to this.");
    Black("You take his coat. It comes down to your ankles. It smells like this house.");
    Black("You take his watch. The strap goes round your wrist twice. ANCHOR 06, crooked. Your thumb finds it by itself.");
    AddClue("w_wrist");   // v0.20 story
    Black("You take his radio. It clicks before you've worked out the button.");
    Black("\"Collector, confirm.\"");
    Black("\"Confirmed.\"");
    if (Knows("graduation")) Black("A pause. \"...Outstanding, 06. Come in.\"");
    else Black("A pause. \"Copy, collector. Come in.\" Nobody asks why your voice is so high.");
    // Doggo
    if (doggoDead) Black("Doggo is lying by the door where he fell. You step over him. You'll step over him a lot.");
    else if (DoggoExists()) Black("Doggo follows you all the way to the gate. You tell him to stay. He stays. He always listened to you.");
    else if (doggoOut) Black("Somewhere out in the storm a dog barks at the car. He doesn't come any closer.");
    // how dark you were before this
    if (darkBefore >= DARK_REVEL) Black("You can't drive. You learn. It takes nine loops. You don't mind loops any more.");
    else if (darkBefore >= DARK_TAINT) Black("You can't drive. The car can. It knows the road to Finse better than you know the house.");
    else Black("You sit in his car for a long time with your hands in your lap. Then it starts by itself. Somebody at Finse is driving.");
    if (brakesCut) Black("Halfway down you remember the brake line. The car takes every bend anyway, slow and careful, like somebody else is driving.");   // v0.20 knife
    Black("They wiped me. They aged me. They reissued me.");
    Black("Snow on your shoulders. A black car at a gate. Two minutes, breathing like you're counting.");
    Black(std::string("You stand at the living room window and look in. A tree. A woman in the kitchen") + (momRemorse ? ", humming" : "") + ". A man by the stove with a newspaper.");
    Black("You walk up the path. Before you can knock, the door opens.");
    Black("A small kid in a red sweater. Standing in the door. Just like that.");
    if (first == "t_hum") Black("He's humming something. You know the tune. You don't know how.");
    else if (first == "t_watch") Black("\"What's on the back of your watch?\" he asks.");
    else if (first == "t_reissue") Black("\"Same house?\" he asks. You don't know what that means. You used to.");
    else if (first == "t_dose") Black("\"Did you take your dose?\" he asks. You did. You took it after.");
    else if (first == "t_last") Black("\"Is this your last one?\" he asks.");
    Black("\"...You're supposed to wait for the knock,\" you say.");
    Narrate(darkBefore >= DARK_TAINT ? "I know every spot he'll hide in. I don't know how I know. I just walk there."
                                     : "He looks like nobody. He looks like me.");
    solemn = false;
    endings.insert("samehouse");
    lastEnd = "ending:samehouse";
    Achieve("Same House");
    Beat e; e.type = BeatType::Ending; e.id = "samehouse"; e.speaker = "TRUE ENDING";
    e.text = darkBefore >= DARK_TAINT ? "SAME HOUSE\n\nHe took the box, the coat and the job. Next cycle he knocks."
                                      : "SAME HOUSE\n\nHe took the coat. It fit him eventually.";
    beats.push_back(e);
}

void Story::AfterEnding() {
    // don't finish it. He doesn't take the dose. Now two of you remember
    chase = false;
    solemn = true;
    std::string first = toldPieces.empty() ? "" : toldPieces.front();
    if (hasKnife) Say("", "(You put the knife down on the floor between you. Nobody picks it up.)");
    else Say("", "(You sit down on the wet floor next to him. He's shaking. So are you.)");
    Say("The visitor", "...That's not how it goes.");
    Say("You", "I know.");
    Say("", "(He takes the pen out of his coat. The needle. He holds it up between you.)");
    Say("The visitor", "If I take it, I forget you again. If I don't, I keep the house.");
    Sound("crack");
    Say("", "(He snaps it in half.)");
    Say("The visitor", "After. It's after now.");
    Say("", "(The radio on his belt clicks.)");
    Say("Finse", "(on the radio) Collector, confirm.");
    Say("The visitor", "Negative. Subject not on site.");
    Say("Finse", "...Collector. Your anchor.");
    Say("The visitor", "(he looks at you) Anchor's fine. Anchor's right here.");
    Say("Finse", "(a long pause) ...Hold position. The cycle closes on its own.");
    if (Knows("graduation")) Say("The visitor", "(to you) Appendix C doesn't have a page for this. I'd have read it.");
    FadeFor(0, "");
    // the parents, however you left them
    if (momLocked) Black("You pull the bolt on the basement door. Mom comes up the stairs, sees him on the floor and stands there for a long time.");
    else if (momBathLocked) Black("You unlock the bathroom. Mom comes out with her hands up, ready for anything. Not for this.");
    if (dadLocked) Black("You lift the plank off the shed door. Dad comes in out of the storm and stops in the doorway, snow on his eyebrows.");
    bool momThere = !momOnFloor, dadThere = !dadDown && !dadOnFloor;
    if (momThere) {
        if (momRemorse) Black("Mom kneels down next to him. She says a name. Not yours. Not 06. He closes his eyes when he hears it.");
        else if (stance == Stance::Dark) Black("Mom looks at him like a coat she gave away a long time ago.");
        else Black("Mom kneels down next to him. She doesn't say anything. She puts her hand on his hair, the way she does with you.");
    }
    if (dadThere) {
        if (stance == Stance::Befriended || parentsAllied) Black("Dad brings him a chair from the kitchen. Mom puts the kettle on. Like he's been away a long time. Like he's family.");
        else Black("Dad crouches and turns the man's wrist over. Reads the back of the watch with his thumb. He doesn't say anything. He doesn't have to.");
    }
    if (stance == Stance::Dark) Black("Nobody says sorry. Nobody in this house has ever known how.");
    if (doggoDead) Black("Doggo is lying by the door. The man looks at him for a long time. \"Sorry, boy,\" he says. \"I always am.\"");
    else if (DoggoExists()) Black("Doggo comes over and lies down against his leg, like he has always done it.");
    else if (doggoOut) Black("Out in the storm, a dog is barking. Nobody goes to let him in. Not yet.");
    Sound("bells");
    Black("17:00. Far away, the bells. Every church in the country.");
    Black("Your watch beeps. So does his. Two short, one long, at the same time.");
    Sound("beep");
    Black("BEEP BEEP.");
    Black("16:00. Your bed. Your room. Your watch.");
    Black("Down at the gate there's already a black car. It's early. It's never early.");
    Black("He's sitting in it with the engine off. He doesn't come in. He watches the house, like he's keeping it.");
    if (first == "t_hum") Black("Through the snow you can just hear it. He's humming.");
    else if (first == "t_watch") Black("He has his glove off. His thumb is going over the back of his watch, again and again.");
    else if (first == "t_reissue") Black("He's looking at your window. Same house. Same cycle. He isn't letting it be the next one.");
    else if (first == "t_dose") Black("There's a new pen on the dashboard. He doesn't touch it.");
    else if (first == "t_last") Black("The radio on the dashboard says something. He turns it off.");
    Black("At 16:22 the phone rings. Mom answers after forty seconds. \"Yes,\" she says. \"Both of them.\"");
    Narrate(darkPoints >= DARK_REVEL ? "I still know where the knife is. So does he. Neither of us goes to get it."
            : tainted ? "I'm still what I turned into. He sits out there anyway."
                      : "Nobody won. Nobody had to.");
    Narrate("Every hour after that, the car was at the gate at four. He never knocked. There were two of us now who remembered.");
    AddClue("two_remember");   // v0.20 story
    solemn = false;
    endings.insert("after");
    lastEnd = "ending:after";
    Achieve("After");
    Beat e; e.type = BeatType::Ending; e.id = "after"; e.speaker = "TRUE ENDING";
    e.text = tainted ? "AFTER\n\nHe didn't take the dose. Neither of them went for the knife."
                     : "AFTER\n\nHe didn't take the dose. Now two of them remember.";
    beats.push_back(e);
}

void Story::TiedBells() {
    // 17:00 and he never got up. You beat him. The hour doesn't care (Noah's answer 5)
    chase = false;
    Sound("bells");
    if (visitorTied) {
        Say("", "(17:00. Far away, the bells start. He's still on the floor with his hands behind him.)");
        Say("The visitor", "(almost kind) There they go. You did it. Nobody gets anywhere, but you did it.");
    } else {
        Say("", "(17:00. Far away, the bells start. He never got up.)");
        Say("The visitor", "(from the floor) Time's up. For both of us.");
    }
    Say("", "(The radio on his belt clicks.)");
    Say("Finse", "(on the radio) Collector. Status.");
    Say("The visitor", visitorTied ? "Restrained. By the subject." : "Down. By the subject.");
    Say("Finse", "(a pause) ...Noted. Same time next cycle.");
    if (!toldPieces.empty()) Say("The visitor", "(to you, quietly) I'll have forgotten all of this by four. Tell me again. All of it, next time.");
    Sound("beep");
    lastEnd = "tied";
    Narrate("I beat him. The hour didn't care.");
    Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
}

// ============================================================ v0.10 things to ask because you remember
// The menus got too long for the screen, so everything a memory unlocks sits one step down
std::vector<ChoiceOpt> Story::MomQuestions() const {
    std::vector<ChoiceOpt> o;
    if (Knows("loop")) o.push_back({"m_loop", "Mom, this day keeps repeating.", true});
    if (Knows("no_car")) o.push_back({"m_car", "Why don't we have a car?", true});
    if (Knows("no_baby")) o.push_back({"m_baby", "Why are there no baby photos?", true});
    if (Knows("calendar")) o.push_back({"m_pickup", "What's the pickup?", true});
    if (Knows("phone_mom") && t < 22 * 60) o.push_back({"m_phone", "Who's calling at 16:22?", true});
    if (Knows("reports")) o.push_back({"m_weeks", "What's week 612?", true});
    if (Knows("book_notes")) o.push_back({"m_chair", "What chair did I ask about?", true});
    if (Knows("juice")) o.push_back({"m_juice", "What's in my juice?", true});
    if (Knows("leash")) o.push_back({"m_scar", "What's that behind your ear?", true});
    if (Knows("handlers")) o.push_back({"m_real", "Are you even my mom?", true});
    if (shift == Shift::Sister && sisterNoticed) o.push_back({"m_sister", "Who's in the bathroom?"});
    MoreMomQuestions(o);   // v0.20 story
    return o;
}
std::vector<ChoiceOpt> Story::DadQuestions() const {
    std::vector<ChoiceOpt> o;
    if (Knows("loop")) o.push_back({"d_loop", "Dad, I'm stuck in a loop.", true});
    if (Knows("calendar")) o.push_back({"d_pickup", "What's the pickup?", true});
    if (Knows("dad_radio")) o.push_back({"d_report", "Who do you report to?", true});
    if (Knows("dad_goodbye") && !Knows("dad_key") && !SawEvent("flag44")) o.push_back({"d_flag", "What happens at forty-four?", true});
    if (Knows("basement_files")) o.push_back({"d_retire", "What will you do when you retire?", true});
    if (Knows("leash")) o.push_back({"d_scar", "What's that behind your ear?", true});
    if (Knows("the_tape")) o.push_back({"d_tape", "\"Subject is resisting.\"", true});
    // v0.16: more for Dad
    if (Knows("no_car")) o.push_back({"d_car", "Why don't we have a car?", true});
    if (Knows("handlers")) o.push_back({"d_real", "Are you even my dad?", true});
    if (Knows("grave_05")) o.push_back({"d_05", "Who was zero-five?", true});
    if (Knows("small_prints")) o.push_back({"d_prints", "Who made the small footprints out back?", true});
    if (Knows("collector")) o.push_back({"d_man", "The man at the door wears a watch like mine.", true});
    if (shift == Shift::Sister && sisterNoticed) o.push_back({"d_sister", "Who's in the bathroom?"});
    MoreDadQuestions(o);   // v0.20 story
    return o;
}

// ============================================================ knife
void Story::KnifeCheck() {
    int needed = KnifeNeeded();   // v0.20 knife: 2 once you know a job for it that isn't a person
    std::vector<ChoiceOpt> o = {{"k_take", "Take it.", false, true}, {"k_no", "Put it back."}};
    if (tainted) {
        if (knifeChecks == 0) Narrate("You again. Fine.");
        else Say("You", "(It fits my hand now.)");
    } else if (KnowsKnifeJob()) {
        // v0.20 knife: it's for more than hurting people
        if (knifeChecks == 0) Say("You", "(The kitchen knife. For twine. For tape. For things, not people.)");
        else Say("You", "(Blade down. Walk, don't run. Like Dad showed me.)");
    } else {
        switch (knifeChecks) {
        case 0: Say("You", "(A kitchen knife. It's heavier than it looks.)"); break;
        case 1:
            if (DoggoExists()) Say("Doggo", "*stops wagging. Watches you.*");
            else Say("You", "(The house is very quiet.)");
            break;
        case 2: Say("Mom", "(from somewhere) Sweetie? What are you doing in the drawers?"); break;
        case 3: Narrate("I remember the weight of it. I remember thinking: just to hold it."); break;
        default: Narrate("Last chance. Put it back. Please."); break;
        }
    }
    (void)needed;
    Choice(std::string("Take the knife? (") + std::to_string(knifeChecks + 1) + "/" + std::to_string(needed) + ")", o);
}

// ============================================================ choices
void Story::Choose(const std::string& id, int px, int py) {
    (void)px; (void)py;
    if (Nag(id)) return;   // v0.16: you asked that already
    if (id.rfind("ft_", 0) != 0 && id != "none") playStreak = 0;   // anything else breaks the gaming streak
    if (MoreChoose(id, px, py)) return;   // v0.20 story
    if (KnifeChoose(id)) return;   // v0.20 knife
    // ---- v0.7 doors
    if (id.rfind("dr_", 0) == 0) {
        size_t c = id.find(':');
        const DoorInfo* d = c == std::string::npos ? nullptr : DoorById(id.substr(c + 1));
        if (!d) return;
        int k = d->y * World::W + d->x;
        std::string act = id.substr(3, c - 3);
        if (act == "closeto" || act == "lockto") {
            // v0.13: through first, then the door
            Room here, there; int ox, oy;
            if (DoorSides(d->x, d->y, px, py, here, there, ox, oy)) {
                FadeFor(0, "", ox, oy);
                if (there == Room::Basement) inBasement = true;
            }
            act = act == "closeto" ? "close" : "lockc";
        }
        if (act == "open") { OpenDoor(d->x, d->y); Sound("door"); }
        else if (act == "close") { closedDoors.insert(k); Sound("door"); }
        else if (act == "lock" || act == "lockc") {
            closedDoors.insert(k); lockedDoors.insert(k); Sound("door");
            Say("You", "(Click.)");
            if (Once("master_key") && !Knows("dad_key")) Say("You", "(The basement key fits. It fits every lock in this house.)");
        } else if (act == "unlock") { lockedDoors.erase(k); Say("You", "(Click. Unlocked.)"); }
        return;
    }
    // ---- v0.19: furniture against a door. From the room side, before he comes
    if (id.rfind("bar_", 0) == 0) {
        size_t c = id.find(':');
        const DoorInfo* d = c == std::string::npos ? nullptr : DoorById(id.substr(c + 1));
        if (!d) return;
        int k = d->y * World::W + d->x;
        Room r = BarricadeRoom(d->x, d->y);
        if (id.rfind("bar_push", 0) == 0) {
            closedDoors.insert(k); barricades.insert(k);
            FadeFor(5, "Pushing furniture against the door");
            Sound("thud");
            if (r == Room::KidsRoom) Say("You", "(My chest of drawers, all the way across the room. It screams on the floorboards. Mom will hear. Let her.)");
            else if (r == Room::Bathroom) Say("You", "(The laundry cabinet tips, then slides. Towels everywhere. It's wedged right under the handle.)");
            else Say("You", "(Dad's dresser. It's heavier than me. It takes forever. But it's against the door.)");
            if (Once("bar_think")) Say("You", "(His key won't help him now. Nobody gets in. Nobody gets out either. Except through the window.)");
        } else {
            barricades.erase(k);
            FadeFor(1, "Pulling it away from the door");
            Say("You", "(Scrape. The door is free again.)");
        }
        return;
    }
    if (id == "tk_walk") {
        // v0.19: a trail for him to follow. The storm eats it if you're early
        FadeFor(3, "Out into the white, and back in your own steps", 29, -24);
        falseTracksAt = t + 180;
        Say("You", "(Twenty steps out. Then backwards, boot in boot, all the way back. From the house it looks like I ran.)");
        return;
    }
    if (id == "e_run") { EscapeEnding(); return; }
    if (id == "bath_lock") {
        // v0.13: hinge A. Noah: a trick when she's happy, a dark act when she's scared or you hold the knife
        int k = 9 * World::W + 13;
        bool dark = BathLockIsDark();
        bool scared = tainted || momAtt == Attitude::Scared || momAtt == Attitude::Broken;
        closedDoors.insert(k); lockedDoors.insert(k);
        momBathLocked = true;
        Sound("door");
        Say("You", "(Click.)");
        Achieve("Occupied");
        if (dark) {
            bathLockDark = true;
            if (scared) Say("Mom", "(the handle rattles once. Then nothing. She's holding her breath in there.)");
            else Say("Mom", "(through the door) Kiddo? ...Sweetie. Put the knife down. Then open the door.");
            DarkAct(scared ? 3 : 2, scared ? "(She doesn't scream. She's too scared of me to scream.)" : "");
            momAtt = Attitude::Scared;
        } else {
            Say("Mom", "(through the door) Hey! Kiddo! ...Okay. Very funny. Ha. Ha.");
            Say("Mom", "The key's on your side, sweetie. Let me out before he... before dinner.");
            if (momAtt == Attitude::Cold) ChangeMood(-1);
            if (Once("bath_think")) Say("You", "(Before he. She almost said it.)");
        }
        return;
    }
    if (id == "bath_free") {
        int k = 9 * World::W + 13;
        momBathLocked = false;
        lockedDoors.erase(k); closedDoors.erase(k);
        Sound("door");
        if (bathLockDark) { Say("Mom", "(she comes out sideways, along the wall, never turning her back on you)"); ChangeMood(-2); }
        else Say("Mom", "(she ruffles your hair, a little too hard) You little gremlin. Go wash your hands.");
        return;
    }
    if (id == "m_pocket") {
        pebbles = 3;
        Say("You", "(Three marbles. They click in my pocket.)");
        if (!Knows("decoy")) Say("You", "(Something to throw. If it lands far away, he looks far away.)");
        return;
    }
    if (id == "e_dinner") { DinnerEnding(); return; }
    if (id == "c_wrist") { ShowWrist(true); return; }
    if (id == "e_stay") {
        Say("Mom", "(she lets go of your hand) ...Okay. Okay. Hide. HIDE.");
        parentsAllied = false;
        Say("Dad", "(grabbing a lamp) Buddy. Run. RUN!");
        SceneBeat("dad_runs");
        return;
    }
    if (id == "b_unlock") {
        basementUnlocked = true;
        Say("You", "(Click. The basement door is unlocked now. It stays that way until 17:00.)");
        return;
    }
    if (id == "cab_tape" || id == "cab_tape2") {
        if (id == "cab_tape") keyInDrawer = true;   // the key stays in the cabinet lock
        Hold(Item::Tape, true);
        if (Once("tape_take")) Say("You", "(You leave the key in the lock and take the tape. It's heavier than a tape should be.)");
        else Say("You", "(You take the tape again.)");
        return;
    }
    if (id == "cab_watch") {
        // v0.10: the old watch was never just set dressing
        bool hadKey = hasKey;
        if (hadKey) keyInDrawer = true;
        Hold(Item::BabyWatch, hadKey);
        if (hadKey) keyInDrawer = true;
        if (Once("watch_take")) {
            Say("", "(Not an old watch. A small one. An Origin like yours, but tiny. The strap would fit around two of your fingers.)");
            Say("", "(Scratched into the back: ANCHOR 06. DO NOT REMOVE.)");
            Say("You", "(Zero six. That's me. I wore this before I could walk.)");
            if (Knows("no_baby")) Say("You", "(No baby photos. But they kept this.)");
            AddClue("baby_watch");
        } else Say("You", "(You take the tiny watch again.)");
        return;
    }
    if (id == "cab_key") { Hold(Item::Key, true); keyInDrawer = false; Say("You", "(You take the key back out of the lock.)"); return; }
    if (id == "vt_play") {
        Sound("hush_on");   // v0.9: nothing under the tape but the tape
        FadeFor(0, "");
        Black("The tape crackles. The basement. This basement. The same chair.");
        Black("A boy in the chair. Six, maybe. A watch that's much too big for his wrist.");
        Black("You know the watch. You know the boy.");
        Black("Dad's voice, off camera, bored. \"Session one, week three hundred and twelve. Subject is resisting.\"");
        Black("You don't watch the next part. You can't. You hear it anyway.");
        Black("You hear yourself. You didn't know you could make that sound.");
        Sound("hum_on");   // v0.10: the tune from the kitchen, alone
        Black("You hear Mom. She's humming. The same carol she hums in the kitchen every Christmas Eve.");
        Black("It goes on for a long time. The clock in the corner of the tape says forty-one minutes.");
        Black("Then Mom walks into the picture with a glass of juice. A straw in it. She smiles at the camera.");
        Black("\"There we go, sweetie. Drink up. You won't remember any of this.\"");
        Black("The boy drinks. He stops crying. He smiles back at her.");
        Black("That's the worst part. You're smiling.");
        Sound("hum_off");
        Sound("hush_off");
        FadeFor(5, "The tape hisses. Snow on the screen.");
        Say("You", "(I'm shaking. I can't make it stop.)");
        Say("You", "(The juice at 15:55. Every day. Every single day.)");
        Narrate("I never called her Mom again. Not out loud. Not in my head. Not even when it would have helped.");
        AddClue("the_tape");
        Achieve("Session One");
        return;
    }
    if (id == "g_come") { doggoStay = false; Say("Doggo", "*bounces up like he's been waiting his whole life for this*"); return; }
    // ---- v0.6 items, chores and the Fliptendo
    if (id == "it_back") {
        static const char* line[] = { "", "(You put the knife back in the block.)", "(You put the key back in the little box.)",
            "(You drop the stick on the woodpile.)", "(You put the trash bag back in the bin.)", "(You put the dog food back.)", "(You put the flashlight back.)",
            "(You put the tape back and take the key out of the lock.)", "(You put the tiny watch back in Dad's cabinet.)" };
        if (held == Item::Tape) { Say("You", line[(int)held]); Hold(Item::Key, true); keyInDrawer = false; return; }
        Say("You", line[(int)held]);
        Hold(Item::None);
        return;
    }
    if (id == "pa_food") { Hold(Item::DogFood); Say("You", "(A scoop of dog food. Doggo can hear it from anywhere in the house.)"); return; }
    if (id == "pa_light") { Hold(Item::Flashlight); Say("You", "(Dad's big flashlight. Heavy. It'll be dark tonight.)"); return; }
    if (id == "wp_stick") { Hold(Item::Stick); Say("You", "(A good stick. A really good stick. Doggo is already looking at it.)"); return; }
    if (id == "wp_stack") {
        FadeFor(Dur(20), "Stacking firewood. Log. After. Log.");
        firewoodDone = true;
        Say("You", "(Twenty minutes of logs. My arms are noodles. This had better be worth it.)");
        return;
    }
    if (id == "tr_zulda") {
        zuldaOpened = true;
        Say("You", "(Wrapping paper everywhere. A download code. ZULDA: FLUTE OF TIME!)");
        Say("You", "(Best. Day. Ever.)");
        if (Stage() >= 1) Say("You", "(It's the same day as always. It's still the best one.)");
        return;
    }
    if (id == "tr_tags") {
        Say("", "(To Kiddo, from Mom and Dad. To Kiddo, from Julenissen.)");
        Say("You", "(Julenissen has the same handwriting as Mom. That's normal. Right?)");
        if (Knows("handlers")) Say("", "(Small print on the back of every tag: S06.)");
        return;
    }
    if (id == "tr_shake") {
        Say("You", Stage() >= 1 ? "(No rattle. Nothing moves. It never does.)" : "(No rattle. Must be socks. Please don't be socks.)");
        return;
    }
    if (id == "tr_open") {
        openedEarly = true;
        Say("You", "(Just one. Just a corner.)");
        Say("", "(Paper. A box. A bow. You lift the lid.)");
        Say("", "(There's nothing in it. The tape on the inside has gone yellow. Somebody wrapped this years ago.)");
        Say("You", "(I close it again. I put it back exactly where it was.)");
        AddClue("presents");
        ChangeMood(-1);
        return;
    }
    // ---- v0.15 outside
    if (id.rfind("win_out:", 0) == 0 || id.rfind("win_in:", 0) == 0) {
        std::string w = id.substr(id.find(':') + 1);
        if (WindowSticks(w) && id.rfind("win_out:", 0) == 0) Say("You", "(Frozen shut. You hit the frame with the heel of your hand. Once. Twice.)");
        SceneBeat("climb:" + w);
        return;
    }
    if (id == "bst_in") {
        Say("", "(The zip is stiff with frost. You get it halfway and squeeze in.)");
        FadeFor(Dur(2), "Inside the snow beast");
        Say("You", "(Warmer on the inside. Smells worse, though.)");
        Achieve("Smells Worse Inside");
        return;
    }
    if (id == "oc_look") {
        Say("You", "(A child seat in the back. Small. The straps have been cut through, clean, like somebody was in a hurry.)");
        if (Stage() >= 2) Say("You", "(Not a family car. A transport.)");
        return;
    }
    if (id == "sw_swing") {
        FadeFor(Dur(2), "Swinging");
        Say("You", Stage() >= 2 ? "(On the way down I see small prints in the snow. On the way up they're gone. Down, there. Up, gone.)"
                                : "(Squeak. Squeak. Squeak. For two minutes I'm just a kid on a swing.)");
        return;
    }
    if (id == "ih_rope") {
        Say("You", "(It comes up easy. Too easy. The end is frayed. Whatever was on it is still down there.)");
        if (Stage() >= 3) Say("You", "(Every Christmas Eve somebody cuts this hole open again. Somebody who knows exactly where.)");
        return;
    }
    if (id == "fl_lick") {
        tongueStuck = true;
        Say("You", "(Everybody knows you don't. That's exactly why.)");
        Say("You", "(It sticks. Instantly. Completely.)");
        Say("You", "Mmf. MMFF.");
        FadeFor(Dur(3), "Stuck to the flagpole");
        if (MomHere() && momAtt != Attitude::Scared && momAtt != Attitude::Broken) Say("Mom", "(pouring a glass of warm water over the pole) ...I'm not even going to ask.");
        else if (DadFree()) Say("Dad", "(with a kettle) Hold still, buddy. Hold STILL.");
        else Say("You", "(You pull. You leave a little bit of tongue on the pole. It's fine. It's FINE.)");
        Achieve("Triple Dare");
        return;
    }
    if (id == "c_coat") {
        hasCoat = true;
        Say("You", "(Zipped to the chin. Warm. For now.)");
        return;
    }
    if (id == "ft_zulda") {
        FadeFor(10, "Playing Zulda: Flute of Time");
        Say("You", "(A boy in green plays a song that sends him back in time. Weird. Relatable.)");
        FliptendoPlayed("zulda");
        return;
    }
    if (id == "ft_flappy") { SceneBeat("minigame"); return; }
    if (id == "c_stick") {
        chase = false;
        AddDark(1);
        if (DoggoExists() && !Tutorial()) DoggoShot(false);
        Say("", "(You swing the stick at his head. It snaps across his forearm.)");
        Say("The visitor", "(looking at the two halves) ...Cute.");
        Sound("bang");
        FadeFor(0, "");
        Black("BANG.");
        Black("Your leg stops working.");
        AddClue("visitor_armed");
        Hold(Item::None, true);
        CollectionScript();
        return;
    }
    if (id == "c_light") {
        lightUsed = true;
        visitorStun = 3.5f;
        Say("", "(You shine the flashlight straight into his eyes. He swears and turns his face away.)");
        Say("You", "(Run. RUN.)");
        return;   // the chase goes on
    }
    // ---- v0.17: the true ending
    if (id == "t_hum" || id == "t_watch" || id == "t_reissue" || id == "t_dose" || id == "t_last") { TellPiece(id); return; }
    if (id == "t_tie") {
        visitorTied = true;
        solemn = true;
        Say("", "(The tripwire is still hanging off the door frame. You pull it loose.)");
        Say("", "(He doesn't fight. You go round his wrists three times and pull as hard as you can. Then once more, to be sure.)");
        Say("The visitor", "(into the floor) Reef knot. Not bad. Somebody taught you that.");
        Say("You", "(Nobody taught me that.)");
        AddClue("tied_up");
        Achieve("Knots");
        solemn = false;
        return;   // the hunt goes on without him. The clock doesn't care
    }
    if (id == "t_talk") {
        // vague and cryptic (answer 10). One at a time, in turn
        std::vector<std::string> l = {
            "Don't untie me before the bells. Don't untie me after them either.",
            "There's a hole in the ice out back. Don't look down it. I looked.",
            "It isn't me you have to get past, 06. It's four o'clock.",
            "Funny. I always thought this part went the other way round.",
            "Ask your mother what she hums. ...No. Don't.",
            Knows("grave_05") ? "Zero-five tied me up too. Better knots." : "Somebody tied me up once. Better knots.",
            "Same house. You'll see.",
        };
        Say("The visitor", l[tiedTalk++ % l.size()]);
        if (tiedTalk == 3 && HingePieces() < 5) Say("You", "(He knows things. I don't know enough of the right things to ask.)");
        return;
    }
    if (id == "t_wait") {
        Say("", "(You sit down on the floor by the door, out of his reach. He watches you. You watch him. The storm does the rest.)");
        t = std::max(t, (float)HOUR - 1.0f);
        return;   // Tick rings the bells
    }
    if (id == "t_kill") { SameHouseEnding(); return; }
    if (id == "t_spare") { AfterEnding(); return; }
    // ---- collection
    if (id == "c_go") {
        chase = false;
        Say("You", "...Okay.");
        Say("The visitor", "Good boy.");
        CollectionScript();
    } else if (id == "c_fight" || id == "c_kick") {
        chase = false;
        if (id == "c_kick") Say("", "(He catches your ankle without even looking.)");
        else Say("", "(You swing at him with everything you've got. He doesn't even blink.)");
        if (DoggoExists() && !Tutorial()) DoggoShot(false);
        Sound("bang");
        FadeFor(0, "");
        Black("BANG.");
        Black("Your leg stops working. The floor comes up fast.");
        Black("He sighs. Like you made extra paperwork.");
        kidShot = true;
        AddClue("visitor_armed");
        CollectionScript();
    } else if (id == "c_knife") {
        chase = false;
        AddDark(2);
        if (DoggoExists() && !Tutorial()) DoggoShot(false);
        Say("", "(He catches your wrist like he's been expecting it. Twists. The knife clatters.)");
        Say("The visitor", "There it is. They said you'd get there eventually.");
        Sound("bang");
        FadeFor(0, "");
        Black("BANG.");
        Black("Your leg stops working.");
        AddClue("visitor_armed");
        CollectionScript();
    } else if (id == "c_finish") {
        Say("", "(He looks up at you. He doesn't beg. He just looks.)");
        std::vector<ChoiceOpt> o;
        if (Knows("dead_switch") && !switchTaken) o.push_back({"c_switch", "Take the box off his belt first.", true});
        if (Knows("dead_switch") && !switchTaken && hasKnife) o.push_back({"kn_wire", "Cut the wire to the box on his belt.", true, true});   // v0.20 knife
        o.push_back({"c_finish2", "Do it.", false, true});
        o.push_back({"none", "(Back away.)"});
        Choice("Are you sure?", o);
    } else if (id == "c_switch") {
        switchTaken = true;
        Say("", "(A black box on his belt. A wire runs up under his shirt. You unclip it. A green light keeps blinking.)");
        Say("The visitor", "(very quietly) ...Who told you about that?");
        Choice("Are you sure?", {{"c_finish2", "Do it.", false, true}, {"none", "(Back away.)"}});
    } else if (id == "c_finish2") {
        chase = false;
        // v0.18: answer 7. Who was standing there when you did it decides what happens next
        if (switchTaken && parentsAllied && TurnedStanding()) LastTollEnding(true);
        else if (switchTaken && UntouchedParent() && !TurnedStanding()) StartHandlers();
        else if (switchTaken) MonsterEnding();
        else DeadSwitchLoop();
    } else if (id == "c_help") {
        chase = false;
        LastTollEnding(false);
    } else if (id == "c_tase") {
        // v0.18: Dad left it on the floor for you. One shot
        hasTaser = false;
        Sound("crack");
        Say("", "(You wait until he's close enough to touch. Then you push it into his coat and squeeze.)");
        Say("", "(Every muscle he has locks at once. He goes over like a tree. The floor shakes.)");
        Say("The visitor", "(through his teeth, from the floor) ...Hhh. That's... mine.");
        visitorDowned = visitorInjured = true;
        AddClue("tased_him");
        Achieve("His Own Medicine");
        SceneBeat("downed");
    } else if (id == "c_know") {
        chase = false;
        Say("You", "I know what you are.");
        Say("The visitor", "(he smiles) Good. They said you might.");
        if (MomHere()) Say("Mom", "...What? What does that mean?");
        AddClue("visitor_knows");
        CollectionScript();
    } else if (id == "c_parents") {
        chase = false;
        Say("You", "Mom. Dad. Did you know?");
        if (MomHere()) Say("Mom", "(she looks at the floor)");
        if (DadFree()) Say("Dad", "Know what? Buddy, he's taking you to the next place. That's all. And we get to... we get to retire.");
        Say("The visitor", "(he smiles at Dad) Sure you do. Everyone gets told something.");
        CollectionScript();
    }
    // ---- knife
    else if (id == "k_take") {
        knifeChecks++;
        if (knifeChecks >= KnifeNeeded()) {   // v0.20 knife
            Hold(Item::Knife);
            Say("You", tainted ? "(Got it.)" : KnowsKnifeJob() ? "(Point down, by my side. Careful.)" : "(You take it. You hide it in your sleeve. Just in case. Just to hold.)");
        } else {
            KnifeCheck();
        }
    } else if (id == "k_no") {
        knifeChecks = 0;
        Say("You", "(You put it back. Your hand is sweating.)");
    }
    // ---- v0.10 asking about what you remember
    else if (id == "m_ask") {
        std::vector<ChoiceOpt> o = MomQuestions();
        o.push_back({"none", "Never mind."});
        Choice("What do I ask her?", o);
    } else if (id == "d_ask") {
        std::vector<ChoiceOpt> o = DadQuestions();
        o.push_back({"none", "Never mind."});
        Choice("What do I ask him?", o);
    } else if (id == "m_weeks") {
        Say("You", "What's week 612?");
        Say("Mom", "(the glass in her hand stops halfway to the shelf) ...Where did you see that?");
        Say("You", "On your desk.");
        Say("Mom", "It's how long we've lived up here. I count. It's silly.");
        Say("You", "(612 weeks is eleven years and nine months. That's not the house. That's me.)");
        ChangeMood(-1);
    } else if (id == "m_chair") {
        Say("You", "What chair did I ask about?");
        Say("Mom", "(too fast) Your high chair. You loved that high chair.");
        Say("You", "(She wrote that note at week 400. I was almost eight.)");
    } else if (id == "m_juice") {
        Say("You", "What's in my juice?");
        Say("Mom", "Apple. (she smiles) It's always apple, sweetie.");
        Say("You", "(She didn't ask why I'm asking.)");
        if (tainted) Say("Mom", "(the smile stays on a second too long) ...You did drink it today. Didn't you?");
        ChangeMood(-1);
    } else if (id == "m_scar") {
        Say("You", "What's that behind your ear?");
        Say("Mom", "(her hand goes to it) Chicken pox. From when I was little.");
        Say("You", "Mine is in the same place. So is Dad's.");
        Say("Mom", "(she's still touching it) ...Is it?");
        Say("You", "(She really doesn't know. They did it to them too.)");
    } else if (id == "m_sister") {
        Say("You", "Who's in the bathroom?");
        Say("Mom", "(she laughs) Your sister. Who else? She'll be an hour, you know how she is.");
        Say("You", "(I don't. I really don't.)");
        Say("Mom", "Set the table when you've got a minute. Four, like always.");
        Achieve("Table for Four");
    } else if (id == "d_sister") {
        Say("You", "Who's in the bathroom?");
        Say("Dad", "Your sister, buddy. Don't start with her today. It's Christmas.");
        Say("You", "(He said it like he's said it a hundred times. He's never said it before.)");
        Achieve("Table for Four");
    } else if (id == "m_watch") {
        Say("Mom", "(her whole face changes) Oh. Look at that.");
        Say("Mom", "You were so small. It went around your arm twice. You screamed the whole first night.");
        Say("", "(She's smiling. A real one. You've never seen that one before.)");
        Say("Mom", "(then she remembers who she's talking to) ...Put that back where you found it. Before your father sees.");
        if (momAtt == Attitude::Normal && Once("mom_watch_mood")) ChangeMood(-1);
    } else if (id == "m_hum") {
        // v0.10: the tape pays off on her, not just on you
        solemn = true;
        Say("You", "(You hum it. The carol. All the way through, the way she does.)");
        Say("Mom", "(she smiles) You know that one? I didn't think you ever...");
        Say("Mom", "(the smile goes) Where did you hear that?");
        Say("You", "From you. Week three hundred and twelve. You hummed it the whole time.");
        Sound("glass");
        Say("", "(The plate slips out of her hands. She doesn't look at the pieces.)");
        Say("Mom", "(very quietly) You drank the juice. I watched you drink it.");
        Say("You", "Every time.");
        Say("Mom", "(she sits down on the kitchen floor, in the pieces) I hummed so I wouldn't hear it. That's all it was. So I wouldn't hear.");
        Say("You", tainted ? "(I watch her cry. I notice that I'm taking notes.)" : "(I wait to feel better. I don't.)");
        momRemorse = true;
        solemn = false;
    } else if (id == "d_flag") {
        Say("You", "What happens at forty-four?");
        Say("Dad", "(he drops the wrench) ...What?");
        Say("You", "You said it. Flag. Forty-four.");
        Say("Dad", "(he picks the wrench up very slowly) Just a thing I check. Before the storm. Go on in, buddy.");
        Say("You", "(16:44. The flagpole. He goes out there and checks something. I could be out there too.)");
    } else if (id == "d_retire") {
        Say("You", "What will you do when you retire?");
        Say("Dad", "(he actually smiles) A boat. Nothing fancy. Hardangerfjord, somewhere the phone doesn't reach.");
        Say("Dad", "Your mother wants chickens. I said fine. Chickens on a boat.");
        Say("Dad", "(he looks at you) ...You'd have liked the boat, buddy.");
        Say("You", "(Handlers are not to be informed. I don't inform him.)");
    } else if (id == "d_scar") {
        Say("You", "What's that behind your ear?");
        Say("Dad", "Old shrapnel. From before.");
        Say("You", "(He doesn't say before what. I don't think he knows.)");
    } else if (id == "d_tape") {
        Say("You", "\"Subject is resisting.\"");
        Say("Dad", "(all the colour goes out of him)");
        Say("Dad", "That was... we had orders. Buddy. We had orders.");
        Say("You", "(He sounded bored on the tape. He doesn't sound bored now.)");
        dadAtt = Attitude::Scared;
        Once("dad_sorry_scared");
    } else if (id == "d_car") {
        Say("You", "Why don't we have a car?");
        Say("Dad", "Car's a liability up here. Freezes, slides off the road, eats money.");
        Say("You", "(The man has one. It doesn't seem to freeze.)");
    } else if (id == "d_real") {
        Say("You", "Are you even my dad?");
        Say("Dad", "(a long pause) I taught you to ride a bike. In the hallway, because of the snow. You hit every wall.");
        Say("Dad", "(quieter) I remember that. Whatever else. I remember that.");
    } else if (id == "d_05") {
        solemn = true;
        Say("You", "Who was zero-five?");
        Say("", "(The wrench stops.)");
        Say("Dad", "Where did you hear that number?");
        Say("You", "Behind the chain fence. There's a cross.");
        Say("Dad", "(he puts the wrench down very carefully) You don't go behind the chain fence. Not ever. Do you understand me?");
        Say("Dad", "(then, almost too low to hear) ...He was a good kid too.");
        solemn = false;
        if (dadAtt == Attitude::Normal) dadAtt = Attitude::Cold;
    } else if (id == "d_prints") {
        Say("You", "Who made the small footprints out back?");
        Say("Dad", "Hares. Big ones this winter.");
        Say("You", "They have boots.");
        Say("Dad", "(he doesn't look up) ...Big hares.");
    } else if (id == "d_man") {
        Say("You", "The man at the door wears a watch like mine.");
        Say("Dad", "(he goes grey) Lots of people have watches, buddy.");
        Say("You", "Scratched. The strap replaced. Like somebody wore it his whole life.");
        Say("Dad", "(he checks his own wrist without meaning to) ...Go inside. Stay with your mother.");
    } else if (id == "d_joke") {
        static const char* jokes[] = {
            "Why don't skeletons go out in the snow? They've got no body to go with. (he laughs at his own joke for a long time)",
            "What do you call a reindeer with no eyes? No idea. (he waits) NO-EYE-DEER. Come on, that's a good one.",
            "How does Julenissen take pictures? With his North Pole-aroid. (he looks very pleased with himself)",
        };
        if (dadJokes < 3) {
            Say("You", "Tell me a joke.");
            Say("Dad", jokes[dadJokes]);
            if (++dadJokes == 3) Achieve("Dad Jokes");
            if (Stage() >= 2 && dadJokes == 3) Say("You", "(Same three jokes. Same order. Every loop. I don't think he knows that either.)");
        } else {
            Say("Dad", "I'm out, buddy. Fresh out. That was the whole act.");
            Say("Dad", "(he thinks) ...Okay, one more. Knock knock.");
            Say("You", "(I don't say who's there. Not today.)");
        }
    } else if (id == "d_storm") {
        Say("You", "Is the storm bad?");
        Say("Dad", Storm() ? "Listen to it. Worst one in years. Nobody's getting up that road tonight." : "Worst one in years, they're saying. Road closes at five. Nobody in, nobody out.");
        if (Stage() >= 2) Say("You", "(Somebody gets in.)");
    } else if (id == "d_army") {
        Say("You", "Were you in the army?");
        Say("Dad", "Something like that. A long time ago.");
        Say("Dad", "(he holds out his hand) Pass me that. The ten. Thanks.");
        Say("You", "(He changed the subject with a wrench. He always does.)");
    } else if (id == "d_mom") {
        Say("You", "How did you meet Mom?");
        Say("Dad", "(he smiles at the generator) At work. She was the only one who laughed at my jokes.");
        Say("Dad", "Still is, some days.");
        if (Stage() >= 2) Say("You", "(He says work like it's a place I've been.)");
    } else if (id == "d_present") {
        Say("You", "What did you get me?");
        Say("Dad", zuldaOpened ? "You already peeked, didn't you. I can tell. You've got the face." : "Socks. Obviously. Wool ones. You'll thank me in January.");
        if (Knows("presents")) Say("You", "(The boxes are empty. Every one of them. He doesn't know I know.)");
    } else if (id == "d_back") {
        Say("You", "That generator is junk. You've been fixing it for weeks.");
        Say("Dad", tainted ? "(he doesn't answer. He keeps the workbench between you.)" : "Hey. Easy. It's older than you and it still works harder.");
        ChangeMood(-1);
    } else if (id == "d_watch") {
        Say("Dad", "Where did you... (you can watch him check the cabinet key in his head)");
        Say("Dad", "Give me that.");
        Say("You", "(I don't.)");
        Say("Dad", "(he looks at it in your hand for a long time) ...Fine. It's yours anyway. Always was.");
        // v0.20 story: the watch topic
        Say("Dad", "(quieter) You got a new one every time the strap got too small. Same face. Same number on the back.");
        Say("Dad", "You don't take them off, buddy. You grow into the next one.");
        AddClue("w_grow");
        if (dadAtt == Attitude::Normal) dadAtt = Attitude::Cold;
    }
    // ---- bed
    else if (id == "m_nisse") {
        Say("You", "Is Julenissen really coming?");
        Say("Mom", tainted ? "(she doesn't smile) Ten to five." : "Ten to five, sweetie. He's never late.");
        if (Knows("visitor_time")) Say("You", "(No. He never is.)");
    }
    else if (id == "bed_sleep") {
        FadeFor(Dur(10), "Napping");
        Say("You", Stage() >= 1 ? "(Sleeping doesn't skip anything. I checked.)" : "(That was nice.)");
    }
    // ---- mom
    else if (id == "m_dinner") {
        Say("Mom", tainted ? "(she watches your hands) ...Ribbe." : "Ribbe.");
        Say("You", "(Again.)");
    } else if (id == "m_allow") {
        Say("Mom", "When the chores are done. You know the deal.");
    } else if (id == "m_snack") {
        FadeFor(Dur(5), "Snack");
        Say("You", "(Pepperkaker. Still warm. Classic.)");
    } else if (id == "m_born") {
        Say("You", "Where was I born?");
        Say("Mom", "Haukeland. In Bergen. It was raining.");
        Say("You", "(She says it like a line she practised.)");
    } else if (id == "m_baby") {
        Say("You", "Why are there no baby photos of me?");
        Say("Mom", "The camera broke.");
        Say("You", "For four years?");
        Say("Mom", "(she starts wiping a clean counter)");
    } else if (id == "m_pickup") {
        Say("You", "Why does it say PICKUP behind door 24?");
        Say("Mom", "(a tiny pause) The presents. Julenissen picks them up at the post office.");
        Say("You", "(Julenissen doesn't use the post office.)");
    } else if (id == "m_back") {
        Say("You", "Chores are stupid.");
        Say("Mom", "Excuse me? Try that again and see what happens.");
        ChangeMood(-1);
    } else if (id == "m_loop" && momTurned) {
        Say("Mom", "(very quietly, not looking up) I know. When he knocks, you hide. I'll be right there.");
    } else if (id == "m_loop" && Befriended()) {
        if (HasProof()) ConvinceParents("Mom");
        else {
            Say("You", "Mom, this day keeps happening. A man comes at 16:50 and...");
            Say("Mom", "(she looks at you for a long time) ...I believe that you believe it, sweetie.");
            Say("You", "(She didn't send me to my room. She just needs proof. Something she'd recognise.)");
        }
    } else if (id == "m_loop") {
        Say("You", "Mom, this day keeps happening. Over and over. A man comes at 16:50 and...");
        Say("Mom", "(her face goes very still)");
        Say("Mom", "That's enough. Your room. Twenty minutes. Go.");
        FadeFor(Dur(20), "Twenty minutes in your room", 5, 6);
        inBasement = false;
        Say("You", "(Okay. Telling Mom was a mistake. Noted.)");
        AddClue("told_mom");
        if (momAtt == Attitude::Normal) momAtt = Attitude::Cold;
    } else if (id == "m_phone") {
        Say("You", "Who's calling at 16:22?");
        Say("Mom", "(a tiny pause) Nobody, sweetie. Wrong number.");
        Say("You", "(It isn't 16:22 yet.)");
        ChangeMood(-1);
    } else if (id == "m_real") {
        Say("You", "Are you even my mom?");
        Say("Mom", "(she looks at you for a long time)");
        Say("Mom", "I raised you. Isn't that the same thing?");
    } else if (id == "m_car") {
        Say("You", "Why don't we have a car?");
        Say("Mom", "We don't need one. Everything we need is right here.");
    } else if (id == "m_knife") {
        Say("You", "(You let her see the knife.)");
        Say("Mom", "(she backs into the corner) ...Okay. Okay, sweetie. What do you want?");
        Say("You", "(Nothing. I just wanted to see her face.)");
        DarkAct(1, "");
        momAtt = Attitude::Scared;
    }
    else if (id == "m_sorry") {
        if (momAtt == Attitude::Scared) {
            Say("You", "I'm sorry. I don't know why I did that.");
            Say("Mom", "(she nods too fast) Okay. Okay. Just... put it back where it was. Please.");
            Say("You", "(She doesn't come out from behind the table.)");
            Once("mom_sorry_scared");
        } else {
            Say("You", "Sorry, Mom.");
            Say("Mom", "(a long breath) ...Fine. But no more stories today. Okay?");
            momAtt = Attitude::Normal;
            if (grounded) Say("You", "(The door is still locked, though.)");
        }
    }
    // ---- dad
    else if (id == "d_sorry") {
        Say("You", "Dad, I'm sorry. I didn't mean it.");
        Say("Dad", "(he doesn't move) ...Yeah. Okay, buddy. Why don't you go inside for a bit.");
        Once("dad_sorry_scared");
    } else if (id == "d_help") {
        helpedDad = true;
        FadeFor(Dur(15), "Helping Dad with the generator");
        Say("Dad", "Hand me the 10. No, the other 10. There you go.");
        Say("Dad", "(muttering to himself) Flag. Forty-four. Don't forget.");
        Say("Dad", "Hey. Whatever happens today... you were a good kid. Remember that.");
        Say("You", "(Whatever happens today?)");
        if (!Tutorial()) AddClue("dad_goodbye");
    } else if (id == "d_what") {
        Say("Dad", "Generator. Storm's coming. Power always goes when it storms up here.");
    } else if (id == "d_cards") {
        FadeFor(Dur(10), "Playing cards with Dad");
        Say("You", "(He lets you win. He always lets you win.)");
        if (Stage() >= 2) Say("You", "(I wonder if that's in the reports too.)");
    } else if (id == "d_pickup") {
        Say("You", "Why does it say PICKUP behind door 24?");
        Say("Dad", "...Presents. Mom's thing.");
        Say("You", "(He doesn't look at me when he says it.)");
    } else if (id == "d_loop" && dadTurned) {
        Say("Dad", "(he doesn't stop working) I heard you, buddy. When he knocks, you hide. Leave him to me.");
    } else if (id == "d_loop" && Befriended() && HasProof()) {
        ConvinceParents("Dad");
    } else if (id == "d_loop" && Befriended()) {
        Say("You", "Dad. I'm stuck. The same hour keeps repeating and a man is coming.");
        Say("Dad", "(he looks at you for a long time) ...You're scaring me a little, buddy.");
        Say("You", "(He didn't reach for the radio. He just needs proof. Something he'd recognise.)");
    } else if (id == "d_loop") {
        Say("You", "Dad. I'm stuck. The same hour keeps repeating and a man is coming.");
        Say("Dad", "(he laughs, then stops laughing)");
        Say("Dad", "...Okay, buddy. Okay. Let me make a call.");
        visitorTime = 40 * 60;
        if (t >= visitorTime - 60) visitorTime = (int)t + 120;
        Say("You", "(That felt like a mistake.)");
        AddClue("told_dad");
        if (dadAtt == Attitude::Normal) dadAtt = Attitude::Cold;
    } else if (id == "d_report") {
        Say("You", "Who do you report to on that radio?");
        Say("Dad", "(he goes pale) Go inside. Please. Just... go inside.");
    } else if (id == "d_knife") {
        Say("You", "(You let him see the knife.)");
        Say("Dad", "(he goes very still) Put that down. Please. You don't want to do this.");
        DarkAct(1, "");
        dadAtt = Attitude::Scared;
    } else if (id == "d_talk") {
        Say("Dad", "Buddy. Look at me. Whatever you think you know. This isn't you.");
        Say("Dad", "Please. PLEASE.");
        Choice("He's begging.", {{"d_talk2", "Make him tell you.", false, true}, {"none", "(Stop.)"}});
    } else if (id == "d_talk2") {
        FadeFor(11, "");
        Black("It took eleven minutes.");
        if (darkPoints >= DARK_REVEL) {
            Black("He told you after four.");
            Black("You kept going anyway.");
        } else {
            Black("He told you after four. You stopped.");
            Black("Your hands didn't stop shaking for a long time.");
        }
        Say("Dad", "(barely) ...The flagpole. The little box at the bottom. Please. Please.");
        AddClue("dad_key");
        dadAtt = Attitude::Broken;
        Achieve("Interrogation");
        DarkAct(4, "");
    }
    // ---- doggo
    else if (id == "g_pet") {
        Say("Doggo", "*happy wiggling*");
        if (Stage() >= 2) Say("Doggo", "*stops wiggling. Stares past you at nothing.*");
    } else if (id == "g_sit") {
        doggoStay = true;   // v0.7: he stays put until you call him
        if (chase) { Say("Doggo", "*lies down flat. Ears back. He stays.*"); return; }
        if (tainted) {
            Say("Doggo", "*sits. Instantly. He doesn't take his eyes off you.*");
            Say("You", "(He's never done that before. Like he's scared of what happens if he doesn't.)");
        } else if (Stage() >= 2) {
            Say("Doggo", "*sits... then slowly turns his head toward the hallway. Ears flat.*");
        } else {
            Say("Doggo", "*sits. Lifts a paw. Gets confused and lies down instead. Very proud of himself.*");
        }
        if (!Tutorial()) Say("You", "(Stay, boy. Stay. He'll stay right here until I call him.)");
    } else if (id == "g_ball") {
        FadeFor(Dur(3), "Playing ball with Doggo");
        if (Stage() >= 2) {
            Say("Doggo", "*comes back without the ball. Sits in the hallway. Staring at the basement door.*");
        } else {
            Say("Doggo", "*chases the ball under the couch and comes back with the ball AND a sock. Best day ever.*");
            ChangeMood(1);
        }
    } else if (id == "g_search") {
        FadeFor(Dur(5), "Doggo is searching");
        if (tainted && !hasKey) {
            // the dog knows where Dad hides it. He just never wanted you to have it
            Say("", "(Doggo sniffs his way across the yard and starts digging at the bottom of the flagpole.)");
            Say("", "(A tiny box, painted the same colour as the pole. He paws it open. A key.)");
            Say("Doggo", "*drops it at your feet and backs away. He doesn't want to be near it. Or you.*");
            Hold(Item::Key);
            AddClue("dad_key");
        } else if (darkPoints >= DARK_ARSENAL && !Knows("syringe")) {
            Say("", "(Doggo goes straight for the trash bins and roots around until he finds something.)");
            Say("", "(A syringe. Empty. A strip of tape on it: 06 - WEEKLY. Mom's handwriting.)");
            Say("You", "(Weekly. What am I getting every week?)");
            AddClue("syringe");
        } else {
            static const char* finds[] = {
                "*comes back with one of Dad's work gloves. Soggy. Very pleased.*",
                "*comes back with a pine cone and a look that says you're welcome.*",
                "*comes back with the TV remote. Mom has been looking for that since Tuesday.*",
            };
            Say("Doggo", finds[(int)(Rand() * 3) % 3]);
        }
    } else if (id == "g_tail") {
        Say("Doggo", "*YELP*");
        if (MomHere()) Say("Mom", "HEY! Leave him alone! What is wrong with you?");
        ChangeMood(-2);
    } else if (id == "g_stick") {
        FadeFor(Dur(5), "Throwing sticks for Doggo");
        Say("Doggo", "*brings it back. Every time. Like it's the best thing that ever happened.*");
    } else if (id == "g_shed") {
        Say("Doggo", "*looks up at you. Tail wagging. He'd follow you anywhere.*");
        Choice("Are you sure?", {{"g_shed2", "Take him to the shed.", false, true}, {"none", "No. Never."}});
    } else if (id == "g_shed2") {
        FadeFor(10, "", 34, 11);
        Black("You walk Doggo out to the shed.");
        Black("He's wagging the whole way.");
        Black("Only you walk back out.");
        doggoGone = true;
        DarkAct(10, "");
    }
    // ---- phone
    else if (id == "p_113") {
        Say("You", "(1. 1. 3.)");
        Say("", "(Nothing. Not even a dial tone anymore.)");
        Say("You", "(The line only goes one place. Not the police.)");
    }
    else if (id == "p_unplug") {
        phoneUnplugged = true;
        Say("You", "(Behind the bench, where nobody looks. One tug.)");
        Say("You", "(No call. No \"confirm pickup\". Let's see what he does without his orders.)");
    } else if (id == "p_plug") {
        phoneUnplugged = false;
        Say("You", "(Back in. Dial tone.)");
    }
    // ---- v0.10 his car
    // ---- v0.19: his car key
    else if (id == "car_key") {
        Sound("thud");
        Say("", "(You reach across the driver's seat and turn the key. The engine coughs once and dies. The headlights go out.)");
        Say("You", "(It's so quiet out here without it. Just the wind.)");
        Achieve("No Ride Home");
        Choice("His car key. Cold and heavy. The tag on the ring is worn smooth.",
               {{"ck_snow", "Throw it as far as I can into the snow."}, {"ck_wood", "Hide it in the woodpile."}, {"ck_pocket", "Keep it in my pocket."}});
    } else if (id == "ck_snow") {
        carKey = "snow";
        Say("You", "(You throw it as hard as you can. It's gone before it lands. The snow just takes it.)");
    } else if (id == "ck_wood") {
        carKey = "woodpile";
        FadeFor(1, "Hiding the key in the woodpile");
        Say("You", "(Third log from the top. Nobody would ever look there.)");
    } else if (id == "ck_pocket") {
        carKey = "pocket";
        Say("You", "(In my pocket. I keep touching it to check it's still there.)");
    }
    else if (id == "car_look") {
        Say("", "(A clipboard on the passenger seat. Three lines. 04, ticked. 05, ticked. 06 - 16:50 - COLLECT 1, CLEAN 2.)");
        Say("", "(A thermos. A paper bag from the bakery in Geilo. He stopped for a bun on his way to do this.)");
        Say("You", Knows("handlers") ? "(Collect one. That's me. Clean two. That's them. They have no idea.)" : "(Collect one. Clean two. There are three of us in the house. Four with Doggo.)");
        AddClue("his_car");
        SameNumber();   // v0.20 story
    } else if (id == "car_trunk") {
        Say("", "(It isn't locked. Oil. Wet dog. A grey blanket.)");
        Say("", "(Two juice boxes, empty, the straws chewed flat. A small mitten. Dog hair on the blanket. Not Doggo's colour.)");
        Say("You", "(The mitten isn't mine. 04 and 05 were in here today. Somebody had a dog.)");
        AddClue("trunk");
    } else if (id == "car_in") {
        ExpressDelivery();
    } else if (id == "car_hide") {
        HingeCar();
    } else if (id == "cr_out") {
        Say("", "(You slide out of the far door and crawl along the fence, low, in his blind side. He doesn't turn around.)");
        FadeFor(0, "", 27, 27);
    } else if (id == "cr_stay") {
        GoneEnding();
    }
    // ---- v0.10 Doggo wants out
    else if (id == "g_out") {
        doggoOut = true; doggoStay = false;
        closedDoors.erase(20 * World::W + 20);
        Sound("door");
        Say("Doggo", "*shoots out into the snow like he's been waiting all day. Straight through the gate. He doesn't look back.*");
        Say("You", Knows("visitor_time") ? "(Go on. Run. Don't come back before five.)" : "(He'll scratch at the door when he's cold.)");
    }
    // ---- v0.10 the filing cabinet
    else if (id == "fi_order") {
        FadeFor(5, "Reading the files");
        Say("", "SUBJECT 06. Loop-aware candidate. Suppression: daily. Sessions: weekly. Handlers: 2.");
        Say("", "IF SUBJECT SHOWS VIOLENCE: COLLECT IMMEDIATELY.");
        Say("", "Clean-up of both handlers at collection. Handlers are NOT to be informed. Compliance is better when they believe they are retiring.");
        Say("You", "(They don't know. They think they get to leave after today.)");
        if (tainted) Say("", "(The page on top is new. Today's date. SUBJECT HAS SHOWN VIOLENCE. The ink is still wet.)");
        Say("", "(A letterhead: FINSE FACILITY. Under it, THE MACHINE, underlined three times.)");
        bool first = !Knows("basement_files");
        AddClue("basement_files");
        AddClue("program_hq");
        if (first) {
            if (tainted) Narrate("They were right about me. They were right to be scared. I made sure of it.");
            else Narrate("If I never hurt anyone, they'd never have a reason. And they took me anyway.");
        }
        if (!Knows("protocol")) Say("You", "(There's a thinner folder behind this one. No label on it.)");
    } else if (id == "fi_protocol") {
        FadeFor(5, "Reading the thin folder");
        Say("", "ORIGIN PROTOCOL. Internal. Do not copy.");
        Say("", "A subject cannot experience his own death. In every branch where he dies there is nobody left to notice. What he experiences is the branch where he did not.");
        Say("", "From the inside he reports it as waking up. Same bed. Same hour.");
        Say("", "The watch fixes the anchor. For collection day the anchor is 16:00.");
        Say("", "We cannot see the branches. We only see drift: a child who knows things nobody told him.");
        Say("", "Daily suppression keeps the count at zero. On collection day the dose is a placebo. We need him to keep this one.");
        Say("You", "(Today's juice was just juice. They wanted me to remember this hour. All of it. On purpose.)");
        bool first = !Knows("protocol");
        AddClue("protocol");
        if (first) Narrate("I wasn't stuck in that hour. I was being kept in it. There's a difference. It took me years to feel it.");
        if (tainted || Knows("dead_switch")) {
            Say("", "(One more page, clipped to the back. APPENDIX C: GRADUATION.)");
            Say("", "The collector carries a paired switch. A subject who kills his collector dies with him.");
            Say("", "A subject who learns this across branches, and removes the switch first, has shown everything we select for: memory, planning, and the will to use both.");
            Say("", "Such a subject is not a failed collection. He is the result. He replaces the collector.");
            Say("You", "(\"They said you'd get there eventually.\")");
            bool firstG = !Knows("graduation");
            AddClue("graduation");
            if (firstG) { Achieve("Entrance Exam"); Narrate("It was never a way out. It was the entrance exam."); }
        } else Say("You", "(A page has been torn off the back. APPENDIX C. Just the staple is left.)");
    }
    // ---- front door
    else if (id == "f_greet") {
        // v0.14: hinge D. You don't wait for the knock
        doorGreeted = true;
        closedDoors.erase(20 * World::W + 20);
        Say("", "(You open the door and stand in it. The cold comes in over your socks. You wait.)");
        FadeFor(0, "", 20, 19);
        t = (float)visitorTime;
        StartCollection("door");
    } else if (id == "f_lock") {
        frontLocked = true;
        closedDoors.insert(20 * World::W + 20);
        frontLockedAt = t;
        Say("You", "(Click. Locked.)");
    } else if (id == "f_unlock") {
        frontLocked = false;
        Say("You", "(Unlocked.)");
    }
    // ---- shed door
    else if (id == "s_bar") {
        dadLocked = true;
        Say("Dad", "Buddy? Hey. HEY. Open the door. This isn't funny.");
        DarkAct(1, "(You drop the plank into the brackets. It fits perfectly. Like it was made for this.)");
    } else if (id == "s_free") {
        dadLocked = false;
        Say("Dad", "(stumbling out) What is WRONG with you?!");
        ChangeMood(-3);
    }
    // ---- basement door
    else if (id == "b_peek") {
        FadeFor(1, "Peeking down the stairs");
        Say("", "(Screens. Dozens of them. One shows your room. One shows the yard.)");
        Say("", "(One shows a boy on the stairs, looking at screens. That's you.)");
        Say("Mom", "(from below) GET OUT! Get out, NOW!");
        AddClue("basement_screens");
        ChangeMood(-2);
    } else if (id == "b_bolt") {
        momLocked = true;
        Say("", "(There's a bolt on the outside of the basement door. Why is there a bolt on the OUTSIDE?)");
        Say("Mom", "(from below) Sweetie? The door. Open the door. OPEN THE DOOR.");
        DarkAct(2, "(Clack.)");
    } else if (id == "b_free") {
        momLocked = false;
        Say("Mom", "(climbing out, shaking) Don't. Don't you ever...");
        ChangeMood(-3);
    } else if (id == "b_down") {
        basementUnlocked = true;
        inBasement = true;
        FadeFor(1, "Unlocking the basement", 6, 38);
        if (Once("first_down") && !achievements.count("Down the Stairs")) {
            Achieve("Down the Stairs");
            if (tainted) Narrate("They built a cage for what I'd become. I became it to find the cage.");
            else Narrate("I'd wanted to see the basement my whole life. I should have kept wanting.");
        }
    }
    // ---- v0.19: the inside traps. One door each, and they wait for him in the hunt
    else if (id == "a_marbles_in" || id == "a_tripwire_in" || id == "a_paint_in") {
        std::string trap = id == "a_marbles_in" ? "marbles" : id == "a_tripwire_in" ? "tripwire" : "paint";
        if (trapsSet.count(trap) || innerTraps.count(trap)) return;
        innerTraps.insert(trap);
        int x, y; InnerTrapDoor(trap, x, y);
        if (trap == "marbles") FadeFor(3, "Marbles in your doorway", x, y + 1);
        if (trap == "tripwire") FadeFor(5, "Tripwire across the living room door", x, y - 1);
        if (trap == "paint") FadeFor(5, "Paint can over the shed door", x, y + 1);
        Say("You", trap == "marbles" ? "(Right where the hall turns into my room. I step over them without looking. He won't know to.)"
                 : trap == "tripwire" ? "(Ankle high, across the living room door. Dad's knot, the one that doesn't slip. He comes through here, every time.)"
                 : "(The paint can, balanced on top of the shed door. He always checks the shed. Late, but he does.)");
        DarkAct(1, "");
    }
    // ---- arsenal
    else if (id == "a_marbles" || id == "a_tripwire" || id == "a_paint") {
        std::string trap = id == "a_marbles" ? "marbles" : id == "a_tripwire" ? "tripwire" : "paint";
        if (trapsSet.count(trap) || innerTraps.count(trap)) return;
        trapsSet.insert(trap);
        traps++;
        if (trap == "marbles") FadeFor(5, "Marbles on the front step", 20, 21);
        if (trap == "tripwire") FadeFor(10, "Tripwire across the porch", 20, 21);
        if (trap == "paint") FadeFor(10, "Paint can over the front door", 20, 19);
        Say("You", std::string("(Trap ") + std::to_string(traps) + " of 3. He comes through the front door. He always does.)");
        DarkAct(1, "");
    }
    // ---- basement console
    else if (id == "r_listen") {
        Say("Voice", "(static) ...unit 04 collected... unit 05 collected... unit 06, 16:50...");
    } else if (id == "r_cancel") {
        Say("You", "(into the mic) This is handler two. Cancel the pickup.");
        Say("Voice", "...Handler two?");
        Say("Voice", "That's not handler two. Stay where you are, 06. We're coming now.");
        AddClue("they_listen");
        visitorTime = t < 37 * 60 ? 40 * 60 : (int)t + 180;
    }
    // ---- visitor at the gate
    else if (id == "v_who") {
        Say("You", "Who are you?");
        Say("The visitor", "Just a man whose car broke down. Go inside, kid. It's cold.");
        VisitorNoticesYou();
    } else if (id == "v_program") {
        Say("You", "You're from the program.");
        Say("The visitor", "(he smiles) Go inside, 06. I'll be right there.");
        VisitorNoticesYou();
    } else if (id == "v_leave") {
        VisitorNoticesYou();
    } else if (id == "v_watch") {
        ShowWrist(false);
        VisitorNoticesYou();
    } else if (id == "v_every") {
        echoAsked = true;
        Say("You", "\"We do this every time.\" What did you mean?");
        Say("The visitor", "(he tilts his head) Did I say that?");
        Say("You", "Not today.");
        Say("The visitor", "(a long pause. Snow settles on his shoulders.) ...Huh.");
        Say("", "(For one second he looks at you the way you'd look at a person. Then it's gone.)");
        Say("The visitor", "Go inside, 06.");
        AddClue("echo");
        VisitorNoticesYou();
    } else if (id == "v_stab") {
        Say("", "(You go for him. He catches your wrist like he's been expecting it.)");
        AddDark(1);
        if (DoggoExists() && !Tutorial()) DoggoShot(false);
        t = (float)visitorTime;
        collection = true;
        Sound("bang");
        FadeFor(0, "");
        Black("BANG.");
        Black("Your leg stops working.");
        AddClue("visitor_armed");
        CollectionScript();
    }
    // ---- flagpole
    else if (id == "fl_key") {
        Hold(Item::Key);
        Say("You", "(The little box. The key. Right where he said.)");
    }
}

// ============================================================ interactions
void Story::Interact(const std::string& id, int px, int py) {
    (void)px; (void)py;
    if (id != "fliptendo") playStreak = 0;
    if (KnifeInteract(id, px, py)) return;   // v0.20 knife
    if (id == "visitor" && chase && visitorDead) { Say("", "(He doesn't move. He isn't going to.)"); return; }
    if (id == "visitor" && chase) { VisitorChoice(); return; }
    if (id == "drop" && !dropItem.empty()) {
        // v0.18: what the fight left on the floor
        if (dropItem == "box") {
            switchTaken = true;
            Say("", "(The black box from his belt. A wire hangs off it, torn. The green light keeps blinking.)");
            Say("You", Knows("dead_switch") ? "(Without the box he's just a man. Mom knew that.)" : "(Mom didn't go for his face. Or his gun. She went for this.)");
        } else {
            hasTaser = true;
            Say("", "(His taser, half under the couch. Heavier than it looks. The little screen says 1.)");
            Say("You", "(One shot. I'd have to let him get really close.)");
        }
        dropItem.clear(); dropX = dropY = -1;
        return;
    }
    if (id == "door_bath" && !collection && (momBathLocked || (MomInBath() && momRoomNow == Room::Bathroom))) {
        // v0.13: hinge A. Mom is in there and you are out here
        if (momBathLocked) {
            if (bathLockDark) Say("Mom", "(through the door, very quiet) Please. Whatever I did. Please.");
            else Say("Mom", "(through the door) Kiddo. Ha ha. Very funny. Open up.");
            std::vector<ChoiceOpt> o;
            if (hasKey) o.push_back({"bath_free", "Let her out."});
            o.push_back({"none", "Leave it."});
            Choice("Mom is locked in the bathroom.", o);
            return;
        }
        See("mom_bath");
        if (!hasKey) { Say("You", "(Mom's in the bathroom. The lock is on her side. Dad's master key turns it from out here, though.)"); return; }
        Choice("Mom is in the bathroom. The master key fits this lock too.", {{"bath_lock", "Lock her in.", false, BathLockIsDark()}, {"none", "Leave her alone."}});
        return;
    }
    // v0.7: doors and Doggo still work while he's hunting you
    if (const DoorInfo* d = DoorById(id); d && id != "frontdoor" && id != "sheddoor" && (!collection || chase)) {
        int k = d->y * World::W + d->x;
        // v0.19: furniture against it. From the inside you can move it. From the outside, nothing moves
        Room barRoom = BarricadeRoom(d->x, d->y);
        bool inside = barRoom != Room::Outside && world.RoomAt(px, py) == barRoom;
        if (barricades.count(k)) {
            if (!inside) Say("You", "(It won't budge. Something heavy is against it on the other side.)");
            else if (chase) Say("You", "(No time to drag it away. The window.)");
            else Choice("Furniture against the door. You are " + RoomPhrase(barRoom) + ".", {{"bar_pull:" + id, "Pull it away. (1 min)"}, {"none", "Leave it."}});
            return;
        }
        if (id == "door_kid" && Knows("new_hinges") && Once("hinges_look")) Bubble("You", "(New hinges. Every other door in this house has old brass ones.)");
        bool canBar = inside && !collection && !Tutorial() && Knows("visitor_time");
        ChoiceOpt barOpt{"bar_push:" + id, barRoom == Room::KidsRoom ? "Push my chest of drawers against it. (5 min)" : barRoom == Room::Bathroom ? "Push the laundry cabinet against it. (5 min)" : "Push Dad's dresser against it. (5 min)"};
        if (canBar && ((id == "door_kid" && grounded) || (lockedDoors.count(k) && !hasKey))) {
            Choice(id == "door_kid" && grounded ? "Locked from the outside. Mom has the key." : "Locked. Only the key opens it.", {barOpt, {"none", "Leave it."}});
            return;
        }
        if (id == "door_kid" && grounded && !collection) { Say("You", "(Locked from the outside. Mom has the key.)"); return; }
        if (brokenDoors.count(k)) { Say("You", "(Splinters. He didn't bother with the handle.)"); return; }
        if (lockedDoors.count(k)) {
            if (hasKey) {
                std::vector<ChoiceOpt> o = {{std::string("dr_unlock:") + id, "Unlock it."}};
                if (canBar) o.push_back(barOpt);
                o.push_back({"none", "Leave it locked."});
                Choice("Locked with the master key.", o);
            }
            else Say("You", "(Locked. Only the key opens it.)");
            return;
        }
        bool closed = closedDoors.count(k) > 0;
        if (!closed && px == d->x && py == d->y) { Say("You", "(I'm standing in the doorway.)"); return; }
        // v0.13: say which side you end up on. Noah: it was way too hard to tell
        Room here, there; int ox, oy;
        bool canStep = DoorSides(d->x, d->y, px, py, here, there, ox, oy);
        std::string stay = RoomPhrase(here);
        if (!hasKey && canBar) {
            Choice(std::string(closed ? "A closed door." : "An open door.") + " You are " + stay + ".",
                   {{std::string(closed ? "dr_open:" : "dr_close:") + id, closed ? "Open it." : "Close it. (I stay " + stay + ".)"}, barOpt, {"none", "Leave it."}});
            return;
        }
        if (!hasKey) {
            if (closed) closedDoors.erase(k); else { closedDoors.insert(k); Bubble("You", "(Shut. I'm " + stay + ".)"); }
            Sound("door"); return;
        }
        if (closed) {
            std::vector<ChoiceOpt> o = {{std::string("dr_open:") + id, "Open it."}, {std::string("dr_lock:") + id, "Lock it. (I stay " + stay + ".)"}};
            if (canBar) o.push_back(barOpt);
            o.push_back({"none", "Leave it."});
            Choice("A closed door. You are " + stay + ".", o);
        }
        else {
            std::vector<ChoiceOpt> o = {{std::string("dr_close:") + id, "Close it. (I stay " + stay + ".)"}};
            if (canBar) o.push_back(barOpt);
            if (canStep) o.push_back({std::string("dr_closeto:") + id, "Step " + IntoPhrase(there) + " and close it behind me."});
            o.push_back({std::string("dr_lockc:") + id, "Close it and lock it. (I stay " + stay + ".)"});
            if (canStep) o.push_back({std::string("dr_lockto:") + id, "Step " + IntoPhrase(there) + " and lock it behind me."});
            o.push_back({"none", "Leave it."});
            Choice("An open door. You are " + stay + ".", o);
        }
        return;
    }
    if (id == "door_bath" && shift == Shift::Sister && !collection && !brokenDoors.count(9 * World::W + 13)) {
        // v0.10: a reality where somebody else lives here
        Say("", "(Occupied. Running water. Somebody singing a carol, badly.)");
        Say("A girl's voice", "Go AWAY. I'm not done!");
        Say("You", "(I don't have a sister.)");
        if (hasKey) Say("You", "(The key turns. The door doesn't move. Something is holding it from the other side.)");
        sisterNoticed = true;
        Achieve("Table for Four");
        return;
    }
    if (id == "doggo" && chase) {
        if (doggoStay) Choice("Doggo is flat on the floor, staying.", {{"g_come", "\"Come on, boy!\""}, {"none", "(Let him stay.)"}});
        else Choice("Doggo is pressed against your legs, growling at the dark.", {{"g_sit", "\"Stay!\""}, {"none", "(Keep him close.)"}});
        return;
    }
    if (id == "coats" && chase && !hasCoat) { Choose("c_coat", px, py); return; }
    // v0.13: the basement stairs work during the hunt. A dead end with one way out
    if (chase && id == "stairs") { inBasement = false; FadeFor(0, "", 27, 11); return; }
    if (chase && id == "basement" && basementUnlocked) { inBasement = true; FadeFor(0, "", 6, 38); return; }
    if (chase && id == "screens") { Say("", "(Black. All of them. Only the little one in the corner, on its battery: a watch, counting. " + ClockText() + ".)"); return; }
    // v0.15: every window is a way through, both ways. In the hunt there's no time to think about it
    if (const WindowInfo* w = WindowById(id); w && (!collection || chase)) {
        bool inside = w->wy <= 2 ? py > w->wy : py < w->wy;
        if (chase) { SceneBeat("climb:" + id); return; }
        if (Tutorial() && id != "window") { Say("You", "(Minus twenty-five out there. Mom would kill me.)"); return; }
        std::vector<ChoiceOpt> o = {{"none", inside ? "Stay in." : "Leave it."}};
        if (inside && id == "window") {
            if (CarHere()) { Say("You", "(Headlights at the gate. A black car. No plates.)"); See("car"); }
            else Say("You", Stage() >= 2 ? "(The backyard. And past it the vidda. White and endless. Like it's waiting.)"
                                         : "(The backyard. Blue dusk. Snow on everything. Nothing out there for kilometres.)");
            if (Tutorial()) return;
        }
        o.push_back({std::string(inside ? "win_out:" : "win_in:") + id, inside ? (WindowSticks(id) ? "Climb out. (It sticks.)" : "Climb out.") : "Climb in."});
        if (inside && hasKnife && WindowSticks(id)) o.push_back({"kn_window:" + id, "Run the knife round the frozen frame. Then climb out."});   // v0.20 knife
        Choice(inside ? "A window. Outside it's minus twenty-five." : "A window into the house.", o);
        return;
    }
    if (id == "vane" && chase) { Interact("vane_look", px, py); return; }
    if (collection) return;
    if (TutorialLocked(id)) {
        Say("You", "(Not today. Maybe some other time.)");
        return;
    }
    if (MoreInteract(id, px, py)) return;   // v0.20 story: the TV, the watch, the stranger shifts, the bed
    bool dadInShed = world.RoomAt(Dad().x, Dad().y) == Room::Shed && DadFree();
    bool dadAtShed = world.RoomAt(Dad().x, Dad().y) == Room::Shed && !dadDown;

    if (id == "car") {
        // v0.10: his car stands empty at the gate while he is at the window
        if (Tutorial()) { Say("You", "(A black car at our gate. Julenissen has a CAR? Where are the reindeer?)"); return; }
        if (!CarEmpty()) { Say("You", "(He's right there. Not now.)"); return; }
        See("car");
        if (!carKey.empty()) { Say("You", "(The engine's off. The car is already getting cold. His key is " + std::string(carKey == "pocket" ? "in my pocket" : carKey == "snow" ? "somewhere in the snow" : "in the woodpile") + ".)"); return; }
        std::vector<ChoiceOpt> o = {{"car_look", "Look in through the window."}, {"car_trunk", "Open the trunk."}};
        if (Knows("visitor_time")) o.push_back({"car_in", "Get in the trunk and pull the lid down.", true});
        if (Knows("his_car") || Knows("trunk")) o.push_back({"car_hide", "Get in the back. Hide on the floor under a blanket.", true});   // v0.14: hinge E
        o.push_back({"car_key", "Reach in and turn the engine off. Take the key."});   // v0.19: section 4
        if (hasKnife && darkPoints >= DARK_INTERROGATE && !brakesCut) o.push_back({"kn_brake", "Crawl under it. Cut the brake line.", false, true});   // v0.20 knife
        o.push_back({"none", "Get away from it."});
        Choice("His car. The engine is running. Nobody is in it.", o);
        return;
    }
    if (id == "bed") {
        Choice("Lie down for a bit? (10 min)", {{"bed_sleep", "Nap."}, {"none", "Nah."}});
    } else if (id == "toys") {
        if (roomClean) { Say("You", "(Clean enough. Mom won't check under the bed. She never does.)"); return; }
        FadeFor(Dur(6), "Cleaning your room");
        roomClean = true;
        Say("You", "(Room: clean. Ish.)");
    } else if (id == "toilet") {
        if (pooped) { Say("You", "(Already went. I'm good.)"); return; }
        int dur = shift == Shift::Toilet45 ? 45 : 15;
        FadeFor(dur, dur == 45 ? "Toilet. This is taking forever." : "Toilet");
        pooped = true;
        if (dur == 45) {
            Say("You", "(Forty. Five. Minutes. WHY.)");
            Achieve("Constipated by Fate");
            AddClue("poop45");
            seenToilet45 = true;
        }
    } else if (id == "mirror") {
        if (shift == Shift::Sister) { Say("You", "(The mirror is fogged up. Somebody drew a smiley face in it. The tap is still warm.)"); return; }
        if (held == Item::BabyWatch && Once("mirror_watch")) Say("You", "(I hold the tiny watch up next to mine. Same watch. One of us grew.)");
        if (Stage() == 0) Say("You", "(That's me. Messy hair. Origin watch. Looking cool.)");
        else if (Stage() == 1) Say("You", "(That's me. Right? Yeah. That's me.)");
        else if (Stage() == 2) Say("You", "(My reflection blinks a little late. I'm sure it does.)");
        else {
            Say("You", "(My reflection is older. Grown up. It's smiling. I'm not.)");
            if (Stage() >= 4) Say("You", "(It mouths something. \"Welcome back.\")");
            if (!Knows("mirror_lag")) { Achieve("Who's There?"); AddClue("mirror_lag"); }
        }
    } else if (id == "desk") {
        FadeFor(10, "Reading Mom's papers");
        Say("", "WEEKLY REPORT. SUBJECT 06. Stable. Unaware. No signs of drift. Week 612.");
        if (tainted) Say("", "(A newer page underneath: SUBJECT 06. Drift confirmed. Request early collection.)");
        Say("You", "(Subject 06? 612 weeks is... that's my whole life.)");
        AddClue("reports");
        if (t >= 28 * 60 && t < 33 * 60 && MomHere()) {
            Say("Mom", "(from the hallway) What are you doing in our room?!");
            ChangeMood(-2);
        }
    } else if (id == "pbed") {
        Say("You", Stage() >= 2 ? "(Two pillows. No dents in either. I don't think they sleep in here. I think they wait.)"
                                : "(Two pillows. Perfectly made. It doesn't look slept in.)");
    } else if (id == "drawer") {
        // v0.7: Dad's cabinet. Same lock as the basement.
        if (drawerOpen) {
            std::vector<ChoiceOpt> o;
            if (held == Item::Tape) o.push_back({"it_back", "Put the tape back and take the key."});
            else if (held == Item::BabyWatch) o.push_back({"it_back", "Put the tiny watch back."});
            else {
                if (keyInDrawer) o.push_back({"cab_key", "Take the key out of the lock."});
                if (keyInDrawer || hasKey) o.push_back({keyInDrawer ? "cab_tape2" : "cab_tape", keyInDrawer ? "Take the tape." : "Take the tape. (leave the key in the lock)"});
            }
            if (held != Item::BabyWatch) o.push_back({"cab_watch", "Take the old watch."});
            o.push_back({"none", "Leave it."});
            Choice("Dad's cabinet is open. Papers, an old watch, a VHS tape: S06 - SESSION 1 - WEEK 312.", o);
            return;
        }
        if (hasKey) {
            drawerOpen = true;
            Say("You", "(The basement key fits Dad's cabinet too. Of course it does.)");
            Say("", "(Papers. An old watch. A VHS tape with a label in Dad's handwriting: S06 - SESSION 1 - WEEK 312.)");
            Choice("Dad's cabinet.", {{"cab_tape", "Take the tape. (leave the key in the lock)"}, {"cab_watch", "Take the old watch. (leave the key in the lock)"}, {"none", "Leave it."}});
            return;
        }
        Say("You", "(Dad's cabinet. Locked. Dad says he keeps the basement key on him. Always. He says it a lot.)");
    } else if (id == "coats") {
        Say("You", "(Dad's jacket. Mom's jacket. My jacket. No car keys anywhere.)");
        Say("You", "(We don't even have a car. 31 km to the nearest neighbour and we don't have a car.)");
        AddClue("no_car");
        if (!hasCoat) Choice("Your jacket is on the hook.", {{"c_coat", "Put it on."}, {"none", "Leave it."}});
    } else if (id == "basement") {
        if (momLocked) {
            Say("", "(Muffled pounding from below. Mom's voice, hoarse now.)");
            Choice("The bolt is still closed.", {{"b_free", "Let her out."}, {"none", "Leave it."}});
            return;
        }
        if (BasementOpen()) {
            std::vector<ChoiceOpt> o = {{"b_peek", "Peek down the stairs."}};
            if (!Tutorial()) o.push_back({"b_bolt", "Slam the door and bolt it.", false, true});
            o.push_back({"none", "Leave."});
            Choice("Mom is down there. The door is open.", o);
            return;
        }
        if (basementUnlocked) { Choice("The basement door. Unlocked. Blue light under it.", {{"b_down", "Go down."}, {"none", "Not yet."}}); return; }
        if (hasKey) {
            Choice("The basement key fits. Of course it fits.", {{"b_down", "Unlock it and go down."}, {"b_unlock", "Just unlock it. (keep it unlocked)"}, {"none", "Not yet."}});
            return;
        }
        Say("You", "(The basement. Locked. It's always locked.)");
        if (DoggoExists() && !Tutorial()) {
            Say("Doggo", "*growls at the door, low and steady*");
            AddClue("doggo_basement");
        }
    } else if (id == "phone") {
        int pm = Stage() >= 2 ? 21 : 22;
        bool ringing = t >= pm * 60 && t < pm * 60 + 40;
        if (ringing && !phoneAnswered) {
            phoneAnswered = true;
            if (Knows("phone_mom")) {
                Say("You", "Hello?");
                Say("Voice", "Handler two, confirm pickup at 16:50. Clean-up is included.");
                Say("You", "...Clean-up?");
                Say("", "*click*");
                if (MomHere()) Say("Mom", "Who was that? Give me that. Who was it?!");
                AddClue("phone_answer");
                ChangeMood(-1);
            } else if (MomHere()) {
                Say("Mom", "(snatching the phone) I've got it, sweetie.");
                Say("Mom", "Hello? ...Yes. Today. ...No, he doesn't suspect a thing.");
                AddClue("phone_mom");
            } else {
                // v0.10: this used to do nothing at all when Mom wasn't around
                Say("You", "Hello?");
                Say("Voice", "Handler two? ...Who is this?");
                Say("", "*click*");
            }
            return;
        }
        if (phoneUnplugged) {
            Say("You", "(Dead. The plug is behind the bench, where I left it.)");
            Choice("The landline.", {{"p_plug", "Plug it back in."}, {"none", "Leave it."}});
            return;
        }
        std::vector<ChoiceOpt> o = {{"none", "Hang up."}};
        if (Knows("visitor_time")) o.insert(o.begin(), {"p_113", "Call 113 (emergency).", true});
        if (Knows("phone_answer") && t < pm * 60) o.insert(o.begin(), {"p_unplug", "Pull the plug out of the wall.", true});
        if (hasKnife && darkPoints >= DARK_ARSENAL && t < pm * 60 && (Knows("phone_answer") || Knows("phone_mom"))) o.insert(o.begin(), {"kn_cord", "Cut the cord behind the bench.", false, true});   // v0.20 knife
        Say("You", "(Dial tone.)");
        Choice("Who would you even call?", o);
    } else if (id == "fridge") {
        Say("", "CHORES: Feed Doggo. Trash out. Dishes. Stack the firewood. Clean your room. One present before dinner if ALL done by 16:40!");
        Say("You", "(Done: " + std::to_string(ChoresDone()) + " of 5.)" + std::string(earnedPresent ? "" : " (One present before dinner. So close.)"));
    } else if (id == "stove") {
        Say("You", Stage() >= 1 ? "(Ribbe. Again. It's always ribbe.)" : "(Ribbe in the oven. It smells like Christmas.)");
    } else if (id == "trashcan") {
        if (trashDone) { Say("You", "(Already took it out.)"); return; }
        if (hasBag) { Choice("The bag is dripping on your sock.", {{"it_back", "Put it back."}, {"none", "Keep it."}}); return; }
        Hold(Item::Bag);
        Say("You", "(You grab the trash bag. It's leaking. Bins are out in the yard.)");
    } else if (id == "bowl") {
        if (doggoOut) { Say("You", "(His bowl. He's out there somewhere, running. Good.)"); return; }
        if (!DoggoExists()) {
            Say("You", "(An empty dog bowl. Whose is this? We don't have a dog. Do we?)");
            if (shift == Shift::NoDoggo) Achieve("Who's a Good Boy?");
            return;
        }
        if (fed) { Say("You", "(He's fed. He's acting like he isn't.)"); return; }
        if (!hasFood) { Say("You", "(Empty. The dog food is in the pantry, next to the fridge.)"); return; }
        FadeFor(Dur(3), "Feeding Doggo");
        fed = true; Hold(Item::None);
        Say("Doggo", "*inhales the entire bowl in two seconds*");
    } else if (id == "tv") {
        if (PowerOut()) { Say("You", "(No power. Just my own face in the glass.)"); return; }
        FadeFor(Dur(20), "Watching TV");
        if (Knows("handlers") && Knows("static_tv") && loop % 3 == 1) {
            // v0.10: it isn't always the same wrong episode
            Say("", "(A cooking show. The chef is making ribbe. He looks into the camera.)");
            Say("", "(\"Twelve years on a low heat. Then you take it out.\")");
        } else if (Knows("handlers") && Knows("static_tv") && loop % 3 == 2) {
            Say("", "(The weather. The map only has one house on it.)");
            Say("", "(\"Clear skies everywhere else,\" says the weather lady. She sounds sorry about it.)");
        } else if (Knows("handlers")) {
            Say("", "(Same channel. Different episode. A boy in a box. The audience laughs.)");
            Say("", "(The host looks straight at you. \"See you at ten to five, Subject 06!\")");
            if (!Knows("static_tv")) {
                Achieve("Previously On");
                AddClue("static_tv");
            }
        } else {
            Say("", "(The Kaptein Fjord Christmas special. You've seen it. Twice. Three times. Ten times.)");
            if (loop >= 6) Say("You", "(\"See you next year, kids!\" He has no idea.)");
            else if (loop >= 3) Say("You", "(I can do his lines with him now. I'm a little bit ahead of him.)");
        }
    } else if (id == "couch") {
        FadeFor(Dur(10), "Rotting on the couch");
        Say("You", Stage() >= 2 ? "(Ten minutes of nothing. I count the seconds anyway. All 600.)" : "(Ten minutes of nothing. Worth it.)");
    } else if (id == "radio") {
        // v0.10: the radio follows the hour
        if (PowerOut()) { Say("You", "(Dead. No power.)"); return; }
        if (t < 15 * 60) Say("", "(Radio) ...storm warning for Hardangervidda. The road closes at 16:50 and stays closed until the 27th. Do not travel tonight. God jul.");
        else if (t < 30 * 60) {
            Say("", "(Radio) ...and now the boys' choir. (Singing. A carol. You know this one. Mom hums it in the kitchen.)");
            if (Knows("the_tape")) Say("You", "(I turn it off.)");
        } else if (t < 45 * 60) {
            Say("", "(Radio) ...and at five o'clock the bells will ring Christmas in from every church in the country, as they have for...");
            if (Stage() >= 1) Say("You", "(I have never heard them from inside the house. Not once.)");
        } else {
            Say("", "(Radio) (Static. The storm is in the antenna.)");
            if (Stage() >= 2) Say("", "(Under the static, very faint: ...zero six... ten to five...)");
        }
    } else if (id == "woodstove") {
        Say("You", Stage() >= 3 ? "(Warm. I hold my hand close until it hurts. Just checking that it still does.)"
                                : "(Warm. The only thing in this house that doesn't feel weird.)");
    } else if (id == "bins") {
        if (trashDone) { Say("You", "(Trash is out. Chore done.)"); return; }
        if (!hasBag) { Say("You", "(The bins. I need the bag from the kitchen first.)"); return; }
        FadeFor(Dur(4), "Taking out the trash");
        trashDone = true; Hold(Item::None);
        Say("You", "(Trash: out. Sock: ruined.)");
    } else if (id == "doghouse") {
        if (doggoOut) Say("You", "(Empty. His tracks go straight past it and out through the gate.)");
        else if (!DoggoExists()) {
            Say("You", "(A doghouse. Empty. Nobody remembers anyone living here. Nobody but me.)");
            if (shift == Shift::NoDoggo) Achieve("Who's a Good Boy?");
        } else {
            Say("You", "(Doggo's house. It smells like Doggo. Strongly.)");
        }
    } else if (id == "gap") {
        // v0.19: the way out the back. Or the way to make him think you took it
        if (!Knows("visitor_time")) { Say("You", "(The back fence is down here. Past it there's nothing but white, all the way to Finse.)"); return; }
        Choice("The gap in the back fence. Past it, the vidda.",
               {{"tk_walk", "Walk tracks out into the snow. Come back in my own steps. (3 min)"}, {"none", "Not now."}});
    } else if (id == "gate") {
        Say("You", "(The gravel road goes on forever. 31 km to anyone.)");
        if (Stage() >= 1) Say("You", "(I could run. He'd catch me. I could still run.)");
    } else if (id == "flag") {
        if (Knows("dad_key") && keyInDrawer) { Say("You", "(The little box is empty. The key is still in the lock of Dad's cabinet.)"); return; }
        if (Knows("dad_key")) {
            if (hasKey) { Choice("The little box at the bottom of the flagpole.", {{"it_back", "Put the key back."}, {"none", "Keep it."}}); return; }
            Choice("The little box at the bottom of the flagpole.", {{"fl_key", "Take the key."}, {"none", "Leave it."}});
            return;
        }
        if (SawEvent("flag44")) {
            // v0.10: the clean route you can actually find. You watched him check it
            Say("You", "(The tiny box. Painted the same colour as the pole. I'd never have seen it if I hadn't watched him.)");
            Say("You", "(A key. THE key.)");
            Hold(Item::Key);
            AddClue("dad_key");
            Narrate("Every Christmas Eve at 16:44 he checked that it was still where he'd hidden it from me. That's how I found it.");
            return;
        }
        flagTouches++;
        // The clean route. Deliberately obscure: third look, at exactly 16:44.
        if (flagTouches >= 3 && t >= 44 * 60 && t < 45 * 60 && Knows("dad_goodbye")) {
            Say("You", "(Wait. There's a tiny box bolted to the bottom of the pole. Painted the same colour.)");
            Say("You", "(A key. THE key.)");
            Hold(Item::Key);
            AddClue("dad_key");
            if (darkPoints < DARK_INTERROGATE) {
                Narrate("Hold on. You didn't figure that out. Nobody figures that out. Someone told you.");
                Narrate("Fine. Keep it. I'm watching you.");
                Achieve("Looked It Up");
            }
            return;
        }
        Say("You", "(The flag is snapping in the wind. Dad raised it this morning, in this weather. For who?)");
        // v0.15: a nod. Everybody knows you don't
        if (!Tutorial() && !tongueStuck) Choice("The flagpole. Steel. Minus twenty-five.", {{"none", "Leave it."}, {"fl_lick", "Lick it."}});
    } else if (id == "generator") {
        Say("You", Knows("dad_radio") ? "(The generator. Nothing wrong with it. It's what he stands next to while he talks on the radio.)"
                                      : "(The generator. Dad's been 'fixing' it for weeks.)");
    } else if (id == "bench") {
        if (dadInShed) {
            Say("Dad", "Hands off the bench, buddy. Sharp stuff.");
            return;
        }
        FadeFor(5, "Snooping on Dad's bench");
        Say("You", "(A radio under a rag. A sticker on the back:)");
        Say("", "PROPERTY OF THE PROGRAM. UNIT 06 HANDLER.");
        AddClue("dad_radio");
    }
    // ---- v0.6 objects
    else if (id == "pantry") {
        std::vector<ChoiceOpt> o;
        if (hasFood || hasLight) o.push_back({"it_back", hasFood ? "Put the dog food back." : "Put the flashlight back."});
        if (!hasFood && !fed) o.push_back({"pa_food", "Take the dog food."});
        if (!hasLight) o.push_back({"pa_light", "Take Dad's big flashlight."});
        o.push_back({"none", "Close it."});
        Choice("The pantry. Knekkebrød, brunost, dog food, a big flashlight.", o);
    } else if (id == "sink") {
        if (dishesDone) { Say("You", "(Dishes: done. My fingers are prunes.)"); return; }
        if (shift == Shift::Sister) { Say("You", "(Four plates in the rack. Four glasses. Four.)"); sisterNoticed = true; }
        FadeFor(Dur(3), "Doing the dishes");
        dishesDone = true;
        Say("You", "(Ribbe fat. The worst kind of fat.)");
    } else if (id == "fliptendo") {
        std::vector<ChoiceOpt> o;
        o.push_back({"ft_flappy", "Play Flappy Plane. (10 min)"});
        if (zuldaOpened) o.push_back({"ft_zulda", "Play Zulda: Flute of Time. (10 min)"});
        o.push_back({"none", "Later."});
        if (Tutorial() && Once("ft_intro")) {
            Say("You", "(My Fliptendo Fold 2. Best thing I own. Only one game on it though.)");
            Say("You", "(Zulda: Flute of Time just got a remake. It's the only thing on my wish list.)");
        }
        Choice(zuldaOpened ? "Fliptendo Fold 2. ZULDA IS INSTALLED." : "Fliptendo Fold 2. Battery: 64%.", o);
    }
    // ---- v0.8 the tree
    else if (id == "tree") {
        std::vector<ChoiceOpt> o;
        if (earnedPresent && !zuldaOpened) o.push_back({"tr_zulda", "Open your present."});
        o.push_back({"tr_tags", "Read the tags."});
        o.push_back({"tr_shake", "Shake the big one."});
        if (!Tutorial() && !openedEarly) o.push_back({"tr_open", "Open one early."});
        if (!Tutorial() && hasKnife && !tapeCut) o.push_back({"kn_tape", "Slit the tape on the big one. Don't tear the paper."});   // v0.20 knife
        o.push_back({"none", "Leave them."});
        Choice(zuldaOpened ? "The tree. Two presents left." : "The tree. Three presents with your name on them.", o);
    }
    // ---- v0.2 objects
    else if (id == "wardrobe") {
        if (shift == Shift::Sister) { Say("You", "(My clothes on the left. On the right, dresses. A size bigger than me. They smell like somebody.)"); sisterNoticed = true; return; }
        Say("You", Knows("reports") ? "(My clothes. S06 on every label. It was never a size.)" : "(My clothes. Every label says S06. I always thought that was a size.)");
    } else if (id == "calendar") {
        Say("", "(The advent calendar. Door 24 is open. No chocolate. One word in Mom's handwriting: PICKUP.)");
        AddClue("calendar");
    } else if (id == "bookshelf") {
        Say("", "(\"Raising Exceptional Children.\" Mom's handwriting in the margins.)");
        Say("", "(\"dose\", \"drift\", \"week 400: stable\", \"he asked about the chair again\".)");
        AddClue("book_notes");
    } else if (id == "album") {
        if (shift == Shift::Sister) {
            Say("You", "(Me at 4. Me at 6, and a girl holding my hand. Me at 9 with a fish. She's holding the fish.)");
            Say("You", "(She's in every picture. She's taller than me. I have never seen her before in my life.)");
            Say("You", "(Still no baby photos. Of either of us.)");
            sisterNoticed = true;
            Achieve("Table for Four");
        } else {
            Say("You", "(Me at 4. Me at 6. Me at 9 with a fish.)");
            Say("You", "(No baby photos. Not one. The album just... starts.)");
        }
        AddClue("no_baby");
    } else if (id == "crates") {
        Say("You", "(Crates. PROPERTY OF THE PROGRAM, stencilled on every one. The big one is empty. I'd fit.)");
    } else if (id == "woodpile") {
        std::vector<ChoiceOpt> o;
        if (!firewoodDone) o.push_back({"wp_stack", hasKnife ? "Cut the twine and stack the firewood. (12 min)" : "Stack the firewood. (20 min)"});   // v0.20 knife
        if (hasStick) o.push_back({"it_back", "Drop the stick."});
        else o.push_back({"wp_stick", "Take a good stick."});
        o.push_back({"none", "Leave it."});
        Choice(tainted ? "The woodpile. An axe in the chopping block. Later, maybe." : "The woodpile. An axe in the chopping block. No. Not that.", o);
    } else if (id == "knives") {
        if (hasKnife) { Choice("The knife block.", {{"it_back", "Put the knife back."}, {"none", "Keep it."}}); return; }
        if (knifeTakenLoop) { Hold(Item::Knife); Say("You", "(You take it again. Easier the second time.)"); return; }
        knifeChecks = 0;
        KnifeCheck();
    } else if (id == "marbles") {
        // v0.12: a handful for your pocket. Something to throw when he is looking for you
        bool trap = darkPoints >= DARK_ARSENAL && !Tutorial() && !trapsSet.count("marbles") && !innerTraps.count("marbles");
        bool pocket = !Tutorial() && pebbles < 3 && Knows("visitor_time");
        if (trap || pocket) {
            std::vector<ChoiceOpt> o;
            if (pocket) o.push_back({"m_pocket", "Put a handful in your pocket."});
            if (trap) o.push_back({"a_marbles", "Pour them on the iced front step. (5 min)", false, true});
            if (trap) o.push_back({"a_marbles_in", "Pour them in my own doorway instead. (3 min)", false, true});   // v0.19
            o.push_back({"none", "Leave them."});
            Choice("A jar of marbles.", o);
        } else if (pebbles >= 3) Say("You", "(My pocket is full of them already.)");
        else if (trapsSet.count("marbles")) Say("You", "(The step is already covered.)");
        else if (innerTraps.count("marbles")) Say("You", "(They're all in my doorway already. Mind your step.)");
        else Say("You", "(Marbles. I used to love these. I still kind of do.)");
    } else if (id == "rope") {
        if (darkPoints >= DARK_ARSENAL) {
            if (trapsSet.count("tripwire") || innerTraps.count("tripwire")) { Say("You", "(The tripwire's already up.)"); return; }
            Choice("A coil of rope.", {{"a_tripwire", "Tripwire across the porch. (10 min)", false, true},
                                       {"a_tripwire_in", "Tripwire across the living room door. (5 min)", false, true}, {"none", "Leave it."}});
        } else Say("You", Knows("visitor_time") ? "(Rope. The man ties good knots. So does Dad.)" : "(Rope. Dad's good with knots.)");
    } else if (id == "toolbox") {
        if (darkPoints >= DARK_ARSENAL) {
            if (trapsSet.count("paint") || innerTraps.count("paint")) { Say("You", "(The paint can's already up there.)"); return; }
            Choice("Hammers, nails, a full can of paint.", {{"a_paint", "Rig the paint can over the front door. (10 min)", false, true},
                                                           {"a_paint_in", "Rig it over the shed door. (5 min)", false, true}, {"none", "Leave it."}});
        } else Say("You", "(Hammers, nails, a can of paint. Dad's stuff.)");
    } else if (id == "frontdoor") {
        bool closed = closedDoors.count(20 * World::W + 20) > 0;
        if (frontLocked) Choice("The front door is locked.", {{"f_unlock", "Unlock it."}, {"none", "Leave it locked."}});
        else {
            bool outside = world.RoomAt(px, py) == Room::Outside;   // v0.13: say which side you lock it from
            std::vector<ChoiceOpt> o = {{closed ? "dr_open:frontdoor" : "dr_close:frontdoor", closed ? "Open it." : "Close it."},
                                        {"f_lock", outside ? "Lock it. (I'm out here. Locked out.)" : "Lock it. (I stay inside.)"}, {"none", "Leave it."}};
            if (DoggoAtDoor() && !Tutorial()) o.insert(o.begin(), {"g_out", "Let Doggo out."});
            if (CanGreet() && !outside) o.insert(o.begin(), {"f_greet", "Open it and wait for him in the doorway.", true});   // v0.14: hinge D
            Choice(CanGreet() && !outside ? "The front door. His car is at the gate." : DoggoAtDoor() ? "The front door. Doggo is whining at it." : "The front door.", o);
        }
    } else if (id == "sheddoor") {
        if (dadLocked) {
            Say("Dad", "(through the door) Please. Whatever you think is happening...");
            Choice("The plank is still in place.", {{"s_free", "Let him out."}, {"none", "Leave it."}});
        } else {
            int k = 10 * World::W + 34;
            bool closed = closedDoors.count(k) > 0, locked = lockedDoors.count(k) > 0;
            std::vector<ChoiceOpt> o;
            Room here, there; int ox, oy;
            bool canStep = DoorSides(34, 10, px, py, here, there, ox, oy) && !closed;
            std::string stay = RoomPhrase(here);
            if (locked) { if (hasKey) o.push_back({"dr_unlock:sheddoor", "Unlock it."}); }
            else o.push_back({closed ? "dr_open:sheddoor" : "dr_close:sheddoor", closed ? "Open it." : "Close it. (I stay " + stay + ".)"});
            if (!locked && hasKey) o.push_back({"dr_lockc:sheddoor", "Close it and lock it. (I stay " + stay + ".)"});
            if (!locked && hasKey && canStep) o.push_back({"dr_lockto:sheddoor", "Step " + IntoPhrase(there) + " and lock it behind me."});
            if (dadInShed && !Tutorial()) o.push_back({"s_bar", "Bar the door with the plank.", false, true});
            o.push_back({"none", "Leave it."});
            Choice(dadInShed ? "Dad's in the shed. There's a plank leaning by the door." : locked ? "The shed door. Locked." : "The shed door. There's a plank leaning next to it. And brackets. Why are there brackets?", o);
        }
    }
    // ---- basement
    else if (id == "stairs") {
        inBasement = false;
        FadeFor(1, "Back up the stairs", 27, 11);
    } else if (id == "screens") {
        Say("", "(Every room. Every angle. Your bed. The toilet. The yard. The shed.)");
        Say("", "(One small screen in the corner just shows a watch. Counting. " + ClockText() + ".)");
        AddClue("basement_screens");
    } else if (id == "files") {
        if (!Knows("basement_files")) { Choose("fi_order", px, py); return; }
        Choice("The filing cabinet. Two folders.", {{"fi_protocol", Knows("protocol") ? "The thin folder at the back. (5 min)" : "The thin folder at the back. No label. (5 min)"},
               {"fi_order", "SUBJECT 06: the collection order. (5 min)"}, {"none", "Leave it."}});
    } else if (id == "chair") {
        Say("", "(A chair with straps. Kid-sized. Worn smooth where wrists go.)");
        Say("You", "(My wrists itch. I don't know why. I do know why.)");
        AddClue("treatment");
    } else if (id == "console") {
        std::vector<ChoiceOpt> o = {{"r_listen", "Listen."}};
        if (Knows("phone_answer")) o.push_back({"r_cancel", "\"This is handler two. Cancel the pickup.\"", true});
        if (Knows("confirmed")) o.push_back({"r_confirm", "Hold down PICKUP CONFIRM. \"Confirmed.\"", true});   // v0.20 story: what Monster taught
        o.push_back({"none", "Leave it."});
        Choice("The program's radio. A red button: PICKUP CONFIRM.", o);
    } else if (id == "medfridge") {
        Say("", "(Little bottles in neat rows. S06. RESET. AFTER EACH SESSION. IN JUICE.)");
        Say("", "(Syringes on the shelf below. S06. WEEKLY. Somebody drew a smiley face on the label.)");
        Say("You", "(Juice at 15:55. Every day. I always get sleepy. I never remember yesterday. I thought that was just how days were.)");
        AddClue("juice");
    } else if (id == "vcr") {
        if (held == Item::Tape) { Choice("An old TV on a cart, facing the chair. A VCR underneath.", {{"vt_play", "Play the tape."}, {"none", "Not yet."}}); return; }
        Say("You", "(An old TV on a cart. It's turned to face the chair. The VCR under it is empty.)");
        if (Knows("the_tape")) Say("You", "(I don't need to see it again. I can still hear the humming.)");
    }
    // ---- NPCs
    else if (id == "mom" && momAtt == Attitude::Scared) {
        // v0.5: she remembers the knife. No small talk.
        if (Knows("mom_sorry_scared") || firedEvents.count("mom_sorry_scared")) {
            Say("Mom", "(she flinches when you come close)");
            Say("You", "(Leave her alone. I did enough.)");
        } else {
            Say("Mom", "(she keeps the kitchen table between you) ...What do you want?");
            Choice(MomName(), {{"m_sorry", "\"I'm sorry.\""}, {"none", "(Leave her alone.)"}});
        }
    } else if (id == "mom" && momAtt == Attitude::Cold) {
        if (grounded) Say("Mom", "(through the door) I'm not discussing it. You stay in there.");
        else Say("Mom", "(not looking up) Did I say you could come out of your room?");
        Choice(MomName(), {{"m_sorry", "\"Sorry, Mom.\""}, {"none", "(Go.)"}});
    } else if (id == "mom") {
        if (momRemorse) {
            Say("Mom", "(she starts a sentence. She stops. She starts the same sentence again.)");
            Say("You", "(She can't look at me. Good. No. Not good. I don't know.)");
            return;
        }
        if (Knows("the_tape") && Once("tape_mom")) Say("You", "(She smiles at me. I can hear her humming. I smile back. I learned that from her.)");
        if (hasKnife && !tainted && Once("mom_sees_knife")) {
            Say("Mom", "Why are you holding that? Put. It. Down.");
            ChangeMood(-1);
        }
        if (tainted && Once("mom_distrust"))
            Say("Mom", "What's wrong with you lately? You look at us like we're strangers.");
        std::vector<ChoiceOpt> o = {
            {"m_dinner", "What's for dinner?"},
            {"m_allow", "Can I open a present?"},
        };
        if (!Tutorial()) o.push_back({"m_snack", "Can I have a snack? (5 min)"});
        if (Stage() >= 1) o.push_back({"m_born", "Where was I born?"});
        if (!Tutorial()) o.push_back({"m_back", "[Talk back] Chores are stupid."});
        if (size_t n = MomQuestions().size()) o.push_back({"m_ask", "Ask about something I remember. (" + std::to_string(n) + ")", true});
        if (Knows("the_tape") && !momRemorse) o.push_back({"m_hum", "(Hum her song back to her.)", true});
        if (held == Item::BabyWatch) o.push_back({"m_watch", "Show her the tiny watch."});
        if (hasKnife) o.push_back({"m_knife", "Show her the knife.", false, true});
        if (!Tutorial()) o.push_back({"m_shove", "[Shove her] Out of my way.", false, true});   // v0.20 story: early lessons
        o.push_back({"none", "Never mind."});
        o.insert(o.begin() + 1, {"m_nisse", "Is Julenissen really coming?"});
        if (Mom().facingWall) {
            Say("Mom", "...");
            Say("You", "(She's just standing there. Facing the corner. Breathing.)");
            Say("Mom", "(she turns around, smiling) Oh! Hi, sweetie.");
        }
        Choice(MomName(), o);
    } else if (id == "dad" && dadAtt == Attitude::Broken) {
        Say("Dad", "(he won't look at you. He's holding his hand against his chest.)");
        Say("You", "(There's nothing left to say to him. Not this loop.)");
    } else if (id == "dad" && dadAtt == Attitude::Scared) {
        if (firedEvents.count("dad_sorry_scared")) {
            Say("Dad", "(he keeps his eyes on your hands) ...Go inside, buddy.");
        } else {
            Say("Dad", "(he takes a step back) Hey. Hey. Easy.");
            Choice("Dad", {{"d_sorry", "\"I'm sorry.\""}, {"none", "(Leave.)"}});
        }
    } else if (id == "dad" && dadAtt == Attitude::Cold) {
        Say("Dad", "(checking his watch) Buddy, go inside for a bit, okay? I've got some things to sort out.");
        std::vector<ChoiceOpt> o;
        if (Knows("dad_radio")) o.push_back({"d_report", "Who do you report to?", true});
        o.push_back({"none", "(Go.)"});
        Choice("Dad", o);
    } else if (id == "dad") {
        std::vector<ChoiceOpt> o = {{"d_what", "What are you doing?"}};
        if (dadInShed && !helpedDad) o.insert(o.begin(), {"d_help", "Need help? (15 min)"});
        if (!dadInShed && !Tutorial()) o.push_back({"d_cards", "Wanna play cards? (10 min)"});
        // v0.16 (Noah): Dad gets as much to say as Mom
        o.push_back({"d_joke", "Tell me a joke."});
        o.push_back({"d_storm", "Is the storm bad?"});
        if (Stage() >= 1) o.push_back({"d_army", "Were you in the army?"});
        if (!Tutorial()) o.push_back({"d_mom", "How did you meet Mom?"});
        if (!Tutorial()) o.push_back({"d_present", "What did you get me?"});
        if (!Tutorial()) o.push_back({"d_back", "[Talk back] That generator is junk."});
        if (size_t n = DadQuestions().size()) o.push_back({"d_ask", "Ask about something I remember. (" + std::to_string(n) + ")", true});
        if (held == Item::BabyWatch) o.push_back({"d_watch", "Show him the tiny watch."});
        if (hasKnife && dadAtShed && darkPoints >= DARK_INTERROGATE && !Knows("dad_key"))
            o.push_back({"d_talk", "Make him tell you where the basement key is.", false, true});
        else if (hasKnife) o.push_back({"d_knife", "Show him the knife.", false, true});
        if (!Tutorial()) o.push_back({"d_hit", "[Hit him] As hard as I can.", false, true});   // v0.20 story: early lessons
        o.push_back({"none", "Never mind."});
        if (tainted && Once("dad_distrust")) Say("Dad", "(he keeps the workbench between you) Hey, buddy. ...You okay?");
        if (Knows("the_tape") && Once("tape_dad")) Say("You", "(\"Subject is resisting.\" He sounded bored. He sounds bored now too.)");
        Choice("Dad", o);
    } else if (id == "doggo") {
        std::vector<ChoiceOpt> o = {{"g_pet", "Pet Doggo."}};
        if (doggoStay) o.push_back({"g_come", "\"Come on, boy!\" (stop staying)"});
        else o.push_back({"g_sit", Tutorial() ? "\"Sit!\"" : "\"Sit! Stay!\""});
        if (hasStick && IsYard(world.RoomAt(px, py))) o.push_back({"g_stick", "Throw the stick. (5 min)"});
        else if (!Tutorial()) o.push_back({"g_ball", "Play with his ball. (3 min)"});
        if (!Tutorial()) o.push_back({"g_search", "\"Find it, boy!\" (5 min)"});
        if (!Tutorial()) o.push_back({"g_tail", "Pull his tail."});
        if (hasKnife) o.push_back({"g_shed", "Take him out to the shed.", false, true});
        if (DoggoAtDoor() && !Tutorial() && !frontLocked) o.insert(o.begin() + 1, {"g_out", "Open the front door for him."});
        o.push_back({"none", "Leave him."});
        Choice(DoggoAtDoor() ? "Doggo. He keeps looking at the front door." : "Doggo", o);
    } else if (id == "visitor") {
        if (DoggoExists()) {
            Say("Doggo", "*hides behind your legs. Shaking.*");
            AddClue("doggo_gate");
        }
        std::vector<ChoiceOpt> o = {{"v_who", "Who are you?"}};
        if (Stage() >= 2) o.push_back({"v_program", "You're from the program.", true});
        if (held == Item::BabyWatch && !Knows("collector")) o.push_back({"v_watch", "(Hold up the tiny watch.)"});
        if ((Knows("hiding") || Knows("he_runs_fast")) && !Knows("echo")) o.push_back({"v_every", "\"We do this every time.\" What does that mean?", true});
        if (hasKnife) o.push_back({"v_stab", "Go for him. Now.", false, true});
        if (size_t n = VisitorQuestions().size()) o.push_back({"v_ask", "Say something I remember. (" + std::to_string(n) + ")", true});   // v0.20 story
        o.push_back({"v_leave", "(Back away.)"});
        Choice("A big man in a black coat.", o);
    } else if (id == "beast") {
        // v0.15: a nod. A forgotten present the size of a pony
        Say("You", "(A ride-on snow beast, the size of a pony. Grey-white plush, three horns, two button eyes. Somebody's present, years ago. Somebody forgot it out here.)");
        if (Stage() >= 2) Say("You", "(Three horns. It's supposed to have two. I don't know how I know that.)");
        if (Tutorial()) return;
        Choice("The snow beast. There's a zip down its belly.", {{"none", "Leave it."}, {"bst_in", "Unzip it and climb inside."}});
    } else if (id == "oldcar") {
        Egg("oldcar");
        Say("You", "(An old car, snowed in to the door handles. No wheels. It sits on blocks of wood.)");
        if (Knows("no_car")) Say("You", "(We don't have a car. We never had a car. Mom said.)");
        Choice("The old car.", {{"none", "Leave it."}, {"oc_look", "Look in through the back window."}});
    } else if (id == "swing") {
        Egg("swing");
        Say("You", "(A swing set. One seat and one empty chain. The seat is too low for me. It was built for somebody smaller.)");
        if (Tutorial()) return;
        Choice("The swing.", {{"none", "Leave it."}, {"sw_swing", "Swing for a bit. (2 min)"}});
    } else if (id == "smallprints") {
        Egg("smallprints");
        Say("You", "(Small boot prints. Smaller than mine. They come from the swing and walk this way.)");
        Say("You", "(Then they stop. No turning around. No jump. They just stop.)");
        Say("You", Storm() ? "(It's snowing sideways and they don't fill in.)" : "(The new snow doesn't stay in them.)");
        AddClue("small_prints");
        if (sisterNoticed || SawEvent("sister")) Say("You", "(Small enough for a girl. Somebody sang in our bathroom once.)");
    } else if (id == "spruce") {
        Say("You", "(The big spruce. The lowest branches lie on the snow like a tent. It's dry under there.)");
        if (Stage() >= 1) Say("You", "(A good place to hide. Not a good place to be warm.)");
    } else if (id == "grave") {
        Egg("grave");
        Say("You", "(A little wooden cross, half under the snow. No name.)");
        Say("You", "(Somebody carved 05 into it. Carefully, with a knife.)");
        if (Knows("dad_radio")) Say("You", "(Unit 06 is stable, Dad told the radio. I'm zero-six. So who was zero-five?)");
        else Say("You", "(Zero-five. A number, not a name.)");
        AddClue("grave_05");
        if (tainted) Say("You", "(There's room next to it.)");
        SameNumber();   // v0.20 story
    } else if (id == "icehole") {
        Egg("icehole");
        Say("You", "(A hole in the ice. Round and neat. Somebody cut it. It's already freezing over at the edges.)");
        Choice("A rope is tied to a stake. It goes down into the black.", {{"none", "Leave it."}, {"ih_rope", "Pull up the rope."}});
    } else if (id == "dish") {
        Egg("dish");
        Say("You", "(A satellite dish on a pole. We don't have satellite TV. We have one channel and it's always the weather.)");
        Say("You", "(The cable doesn't go into the house. It goes straight down into the snow.)");
        if (Knows("basement_screens") || Knows("they_listen") || Knows("basement_files")) Say("You", "(Down. Toward the basement. That's how Finse hears us.)");
    } else if (id == "lights") {
        // v0.15: a nod. Lights over painted letters. On a dark night they spell
        if (BoardSpells()) {
            Say("", "(The bulbs come on one at a time, by themselves. B. E. L. O. W.)");
            Say("You", "(They aren't plugged in to anything.)");
            Achieve("Spelled Out");
            return;
        }
        Say("You", "(A sheet of plywood behind the shed. Somebody painted the alphabet on it in house paint and nailed Christmas lights over the letters. One bulb each.)");
        Say("You", PowerOut() ? "(The power's out. Every bulb is dark.)" : Stage() >= 2 ? "(Like somebody wanted to talk without a phone.)" : "(Weird thing to keep behind a shed.)");
    } else if (id == "vane" || id == "vane_look") {
        // v0.15: a nod. Not a rooster
        Say("You", "(The weathervane on the shed roof. Not a rooster. A fat little rodent with its paws up, pointing into the wind.)");
        if (t >= 45 * 60 && loop >= 2) { Say("You", "(Same day again, little guy?)"); Achieve("Same Day Again"); }
        else if (Storm()) Say("You", "(It can't make up its mind. Neither can I.)");
    } else {
        Say("You", "(Huh.)");
    }
}

// ============================================================ v0.16: parents get tired of you
bool Story::Nag(const std::string& id) {
    // the plain talk topics. Actions, memories that change things and menus are left alone
    static const std::set<std::string> momTalk = {"m_dinner", "m_allow", "m_nisse", "m_born", "m_baby", "m_pickup", "m_phone", "m_real", "m_car",
                                                  "m_weeks", "m_chair", "m_juice", "m_scar", "m_back", "m_snack"};
    static const std::set<std::string> dadTalk = {"d_what", "d_pickup", "d_report", "d_scar", "d_retire", "d_flag", "d_storm", "d_army", "d_mom",
                                                  "d_present", "d_back", "d_car", "d_real", "d_05", "d_prints", "d_man"};
    bool mom = momTalk.count(id) > 0, dad = dadTalk.count(id) > 0;
    if (!mom && !dad) return false;
    if (asked[id]++ == 0) return false;   // the first time they answer properly
    int& n = mom ? nagMom : nagDad;
    ++n;
    if (mom) {
        if (n == 1) Say("Mom", tainted ? "(she doesn't turn around) I answered that." : id == "m_snack" ? "You just had one. Nice try." : "I already told you, sweetie.");
        else if (n == 2) { Say("Mom", "Kiddo. What did I just say? Go find something to do."); ChangeMood(-1); }
        else if (n == 3) { Say("Mom", "(she puts the dish down hard) One more time. I dare you."); ChangeMood(-1); }
        else {
            Say("Mom", "That's it. I'm not answering the same question all day.");
            Achieve("Asked and Answered");
            Ground();
        }
    } else {
        if (n == 1) Say("Dad", tainted ? "(he keeps his eyes on your hands) ...Same answer as before, buddy." : "Asked and answered, buddy.");
        else if (n == 2) Say("Dad", "(he sighs through his nose) Go bother your mother for a bit.");
        else if (n == 3) { Say("Dad", "Buddy. I'm working. Out."); ChangeMood(-1); }
        else {
            Say("Dad", "Right. Your room. Ten minutes. Go on, march.");
            Achieve("Asked and Answered");
            FadeFor(Dur(10), "Sent to your room", 5, 6);
            inBasement = false;
            if (dadAtt == Attitude::Normal) dadAtt = Attitude::Cold;
            Say("You", "(He didn't lock the door. Dad never locks the door.)");
        }
    }
    return true;
}

// ============================================================ v0.16: Doggo at the door
void Story::OnDoggoAtDoor() {
    if (!chase || !DoggoExists()) return;
    if (Once("doggo_door_bark")) Bubble("The visitor", "(to the dog, quiet) Hey, boy. Easy. Where's the kid?");
}
void Story::OnDoggoCalled() {
    if (!chase) return;
    Bubble("You", "Doggo! HERE!");
    Bubble("The visitor", "Go on, then. Good dog.");
    AddClue("doggo_door");
    Achieve("Called Back");
}
void Story::OnDoggoPutDown(int x, int y) {
    if (!chase || !DoggoExists()) return;
    // no fade, no cut: it happens while you're still hiding
    doggoDead = true; doggoDeadX = x; doggoDeadY = y;
    Bubble("The visitor", "Sorry, boy.");
    Sound("bang");
    SceneBeat("doggo_dies");
    Bubble("You", "(No. NO.)");
    AddClue("doggo_door");
}

// ============================================================ v0.15: outside
int Story::OutbackEggs(const std::set<std::string>& seen) {
    int n = 0;
    for (const char* e : {"egg_oldcar", "egg_smallprints", "egg_grave", "egg_dish", "egg_swing", "egg_icehole"}) n += seen.count(e) ? 1 : 0;
    return n;
}
void Story::Egg(const std::string& id) {
    See("egg_" + id);
    if (EggsFound() == 6 && !achievements.count("Out Back")) {
        Achieve("Out Back");
        Narrate("Six things out back. A car, a swing, a cross, a dish, a hole in the ice, and prints that stop. Somebody small lived here before me. I don't think they ever left.");
    }
}
void Story::OnClimbed(const std::string& win, bool out, bool doggoLeft) {
    // Noah: neutral, even while grounded. No mood, no dark
    if (win == "window") windowBanged = true;
    if (!Knows("out_window")) {
        Say("You", out ? "(Out. Snow down my collar. Nobody saw.)" : "(In. Snow all over the floor. Nobody saw.)");
        AddClue("out_window");
    }
    Achieve("Out the Window");
    if (out && grounded && !collection && Once("grounded_window")) Bubble("You", "(Still grounded. Just grounded outside.)");
    if (doggoLeft) Bubble("Doggo", "*whines at the glass*");
}
void Story::OnLitUp() {
    if (!chase) return;
    if (Once("lit_up")) Bubble("The visitor", hasCoat ? "Stripes on your jacket, 06. You shine." : "Red as a postbox. There you are.");
    AddClue("lit_up");
}
void Story::OnTased() {
    // v0.15.2 (Noah): the counter to kiting him through windows and fences
    if (!chase) return;
    chase = false;
    Sound("crack");
    Say("", "(Two little hooks bite into your back. Then every muscle you have locks at once. The snow comes up and hits you.)");
    Say("The visitor", Knows("taser") ? "Same as last time, 06. I don't run anymore." : "Enough games, 06. I'm too old to run after you.");
    AddClue("taser");
    CollectionScript();
}
void Story::TooColdToLeave() {
    // v0.15.1 (Noah): in the first round the kid just turns around
    static const char* lines[] = {
        "(Brrr. NOPE. It's way too cold out there. Zulda: Flute of Time is waiting inside.)",
        "(Minus twenty-five and Julenissen comes at ten to five. I'm not missing that for a walk in the snow.)",
        "(My nose is already freezing. Back inside. Maybe Mom made cocoa.)",
    };
    int n = 0;
    for (auto& e : firedEvents) if (e.rfind("tooCold", 0) == 0) ++n;
    firedEvents.insert("tooCold" + std::to_string(n));
    Say("You", lines[n % 3]);
}
void Story::OnHusky() {
    if (collection) return;
    if (Once("husky")) Bubble("You", "(Nobody there. No tracks either.)");
    See("husky");
    Achieve("Not a Dog");
}

// v0.20 story: endings that teach, the watch, more questions, the TV, stranger shifts, early lessons, predictions
#include "StoryMore.inc"
#include "Knife.inc"   // v0.20 knife

} // namespace sixhundred
