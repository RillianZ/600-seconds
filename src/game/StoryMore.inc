// 600 Seconds - v0.20 story lane. Included at the end of Story.cpp, inside the namespace. Engine-free.
// Endings that teach (T-062), the watch (T-064), more questions and tainted lines (T-068), the TV (T-069),
// two stranger shifts (T-089), early lessons (T-084) and predictions (T-085).
// Every line here is Claude's, for Noah to react to.

// ============================================================ the stranger shifts (T-089)
void Story::ShiftOpener() {
    if (shift == Shift::Fourth) Say("You", "(The house smells like somebody else's coat.)");
    if (shift == Shift::NextUnit) {
        Say("Dad", "(from downstairs) Oh-seven! Up and at 'em, buddy!");
        Say("You", "(Oh-seven? I'm zero-six. I've always been zero-six.)");
    }
}

// 05 on the cross out back, 05 ticked on his clipboard today
void Story::SameNumber() {
    if (!Knows("grave_05") || !Knows("his_car") || Knows("same_number")) return;
    Say("You", "(05 on the cross. 05 on his clipboard, ticked, today. That's two different kids.)");
    Say("You", "(They don't make new numbers. They hand them down.)");
    AddClue("same_number");
}

// ============================================================ predictions (T-085)
// You tell them what is about to happen. Every one shifts them a little: uneasy, then cold, then scared
void Story::Predict(const std::string& who, const std::string& what) {
    bool mom = who == "Mom";
    Once((mom ? "pm_" : "pd_") + what);
    Once("pred_" + what);
    int pm = Stage() >= 2 ? 21 : 22;
    std::string phone = "16:" + std::to_string(pm);
    solemn = true;
    if (what == "phone") {
        if (mom) {
            Say("You", "The phone's going to ring at " + phone + ". It's for you.");
            Say("Mom", "(she laughs) Are you expecting a call, mister?");
            Say("You", "No. You are. They'll ask if I suspect anything.");
            Say("Mom", "(the smile stays where it is. The rest of her face leaves.)");
        } else {
            Say("You", "At " + phone + " Mom gets a phone call. She says I don't suspect a thing.");
            Say("Dad", "(he stops working) ...Who told you that, buddy?");
            Say("You", "Nobody. It just always happens.");
        }
    } else if (what == "flag") {
        if (mom) {
            Say("You", "At 16:44 Dad goes out and checks the little box under the flagpole.");
            Say("Mom", "(her eyes go to the window, to the pole, and back) What box, sweetie?");
            Say("You", "(She knows what box.)");
        } else {
            Say("You", "At 16:44 you'll go out to the flagpole and check the box. You check it every year.");
            Say("Dad", "(very still) ...Have you been following me?");
            Say("You", "I don't need to.");
        }
    } else if (what == "storm") {
        if (mom) {
            Say("You", "The storm hits at 16:47. The lights flicker, and you can't see the gate.");
            Say("Mom", "(she looks out. Clear sky, nearly.) The radio said after five.");
            Say("You", "The radio's wrong. It's always wrong.");
        } else {
            Say("You", "The storm comes at 16:47. You'll say everybody inside, and nobody opens that door.");
            Say("Dad", "(he checks the sky, then you) ...That's what I'd say, alright.");
            Say("Dad", "(he doesn't laugh. He goes back to work and gets it wrong twice.)");
        }
    } else if (what == "knock") {
        if (mom) {
            Say("You", "At ten to five the power goes out. Then three slow knocks.");
            Say("Mom", "(she drops the spoon in the pot) ...Julenissen. You mean Julenissen.");
            Say("You", "He doesn't have a beard.");
        } else {
            Say("You", "At ten to five the power dies. Then a man knocks three times. You let him in.");
            Say("Dad", "(the wrench goes down on the bench, very carefully) Buddy. Who have you been talking to?");
        }
    }
    int& n = mom ? predMom : predDad;
    Attitude& a = mom ? momAtt : dadAtt;
    ++n;
    if (n == 1) Say(who, mom ? "(she goes back to the stove. She keeps looking at you sideways.)" : "(he goes back to work. He keeps one eye on you.)");
    else if (n == 2) {
        if (a == Attitude::Normal) a = Attitude::Cold;
        Say(who, mom ? "(quietly) I think you should go and play in your room for a while." : "(not looking up) Go inside for a bit, buddy. Please.");
    } else {
        a = Attitude::Scared;
        Say(who, mom ? "(she backs up until the counter stops her) How do you KNOW these things?" : "(he takes a step back from you. A whole step.)");
    }
    if (mom) ChangeMood(-1);
    solemn = false;
    bool all = true;
    for (const char* w : {"phone", "flag", "storm", "knock"}) if (!firedEvents.count(std::string("pred_") + w)) all = false;
    if (all) Achieve("Fortune Teller");
}

// ============================================================ the questions (T-068, T-062, T-085)
void Story::MoreMomQuestions(std::vector<ChoiceOpt>& o) const {
    int pm = Stage() >= 2 ? 21 : 22;
    // predictions: only before it happens, once each
    if ((Knows("phone_mom") || Knows("phone_answer")) && !phoneUnplugged && t < pm * 60 - 20 && !firedEvents.count("pm_phone"))
        o.push_back({"mp_phone", "\"The phone's going to ring at 16:" + std::to_string(pm) + ".\"", true});
    if ((Knows("dad_key") || SawEvent("flag44")) && t < 44 * 60 + 20 && !firedEvents.count("pm_flag"))
        o.push_back({"mp_flag", "\"At 16:44 Dad checks the flagpole.\"", true});
    if (KnowsEvent("storm") && !Storm() && !firedEvents.count("pm_storm"))
        o.push_back({"mp_storm", "\"The storm hits at 16:47.\"", true});
    if (Knows("visitor_time") && t < 46 * 60 && !firedEvents.count("pm_knock"))
        o.push_back({"mp_knock", "\"At ten to five the power goes. Then three knocks.\"", true});
    // the newer memories
    if (Knows("new_hinges")) o.push_back({"m_hinges", "Why does my door have new hinges?", true});
    if (Knows("hand_me_down")) o.push_back({"m_brother", "Did I ever have a big brother?", true});
    if (Knows("thirty")) o.push_back({"m_thirty", "What's thirty kilometres from here?", true});
    if (Knows("piece_mom")) o.push_back({"m_before", "Who did you hum to before me?", true});
    if (Knows("piece_door")) o.push_back({"m_door", "Did anybody ever open the door before he knocked?", true});
    if (Knows("he_remembers")) o.push_back({"m_lived", "If he didn't take his medicine, would he remember this house?", true});
    // what the endings taught
    if (Knows("three_plates")) o.push_back({"m_plate", "Can we set a fourth plate tonight?", true});
    if (Knows("handlers_hunt")) o.push_back({"m_deliver", "If he's gone, don't drive me there. Handlers don't deliver.", true});
    if (endings.count("secret")) o.push_back({"m_kids", "There are other kids at Finse. They all wear my watch.", true});
    // the stranger shifts
    if (shift == Shift::Fourth && fourthNoticed) o.push_back({"m_fourth", "Who's the man at the back of the photos?"});
    if (shift == Shift::NextUnit) o.push_back({"m_unit", "Why does everybody call me oh-seven?"});
}

void Story::MoreDadQuestions(std::vector<ChoiceOpt>& o) const {
    int pm = Stage() >= 2 ? 21 : 22;
    if ((Knows("phone_mom") || Knows("phone_answer")) && !phoneUnplugged && t < pm * 60 - 20 && !firedEvents.count("pd_phone"))
        o.push_back({"dp_phone", "\"At 16:" + std::to_string(pm) + " Mom gets a phone call.\"", true});
    if ((Knows("dad_key") || SawEvent("flag44")) && t < 44 * 60 + 20 && !firedEvents.count("pd_flag"))
        o.push_back({"dp_flag", "\"At 16:44 you'll go and check the flagpole.\"", true});
    if (KnowsEvent("storm") && !Storm() && !firedEvents.count("pd_storm"))
        o.push_back({"dp_storm", "\"The storm comes at 16:47.\"", true});
    if (Knows("visitor_time") && t < 46 * 60 && !firedEvents.count("pd_knock"))
        o.push_back({"dp_knock", "\"At ten to five a man knocks. You let him in.\"", true});
    if (Knows("his_knot")) o.push_back({"d_knot", "Who else did you teach your knot?", true});
    if (Knows("new_hinges")) o.push_back({"d_hinges", "Why did you put new hinges on my door?", true});
    if (Knows("piece_dad")) o.push_back({"d_nail", "Did you scratch my watch with a nail?", true});
    if (Knows("piece_reissue")) o.push_back({"d_reissue", "What does reissue mean?", true});
    if (Knows("hand_me_down")) o.push_back({"d_coat", "Whose coat did the man wear before him?", true});
    if (SawEvent("egg_icehole")) o.push_back({"d_ice", "What's on the rope in the ice out back?", true});
    if (Knows("finse_gate")) o.push_back({"d_0612", "What's 0612?", true});
    if (Knows("just_like_us")) o.push_back({"d_hide", "If he can't find me, he starts on you.", true});
    if (Knows("last_toll")) o.push_back({"d_almond", "Do you always get the almond?", true});
    if (shift == Shift::Fourth && fourthNoticed) o.push_back({"d_fourth", "Who's the man at the back of the photos?"});
    if (shift == Shift::NextUnit) o.push_back({"d_unit", "Why did you call me oh-seven?"});
}

std::vector<ChoiceOpt> Story::VisitorQuestions() const {
    std::vector<ChoiceOpt> o;
    if (Knows("under_well")) o.push_back({"v_well", "\"Under the well.\"", true});
    if (Knows("his_dog")) o.push_back({"v_dog", "Did you have a dog?", true});
    if (Knows("nobody_left")) o.push_back({"v_cameout", "\"You always came out for them.\"", true});
    if (Knows("w_wrist")) o.push_back({"v_twice", "\"Your watch goes round my wrist twice.\"", true});
    if (Knows("two_remember")) o.push_back({"v_after", "\"After. It's after now.\"", true});
    return o;
}

// ============================================================ objects
bool Story::MoreInteract(const std::string& id, int px, int py) {
    (void)px; (void)py;
    // ---- tainted runs: they can tell. Every other loop instead of the old line
    if (id == "mom" && tainted && momAtt == Attitude::Normal && !momRemorse && loop % 2 == 0 && Once("nosoul_mom")) {
        Once("mom_distrust");
        static const char* l[] = {
            "(she watches you come in) ...Your eyes do the smile, kiddo. The rest of you doesn't.",
            "(she looks at you a moment too long) You used to laugh at the TV. You don't any more.",
            "(quietly, to the pot on the stove) Something's gone out of him. I don't know when.",
        };
        Say("Mom", l[(loop / 2) % 3]);
        return false;   // the menu as usual
    }
    if (id == "dad" && tainted && dadAtt == Attitude::Normal && loop % 2 == 0 && Once("nosoul_dad")) {
        Once("dad_distrust");
        static const char* l[] = {
            "(he doesn't turn round) You used to cry at the end of films, buddy. Even the funny ones.",
            "(he keeps the bench between you) Where'd you go, buddy? You're right there and you're not.",
            "(he looks at you, then at the radio) ...They said this might happen. They didn't say what it'd look like.",
        };
        Say("Dad", l[(loop / 2) % 3]);
        return false;
    }
    // ---- the TV changes between loops, sometimes because of how the last one ended (T-069)
    if (id == "tv" && !PowerOut() && Stage() >= 2 && (Knows("static_tv") || !Knows("handlers"))) {
        const std::string& e = prevEnd;
        bool trunk = e == "trunk" || e == "express" || e == "spared" || e == "dadran" || e == "name" || e == "nokey";
        int ep = e == "vidda" || e == "ending:escape" ? 1 : e == "cold" ? 2 : (e == "doggo" || e == "ending:doggo") ? 3 : (trunk && loop % 2 == 0) ? 4 : 0;
        if (ep == 0) return false;
        FadeFor(Dur(20), "Watching TV");
        if (ep == 1) {
            Say("", "(A travel programme. A car drives through snow at night. A road sign in the headlights: 31.)");
            Say("", "(The car stops. The presenter doesn't get out. \"And that's as far as we go today,\" she says. \"That's as far as anyone goes.\")");
        } else if (ep == 2) {
            Say("", "(A nature programme. A fox curls up in the snow, nose under its tail, and goes to sleep.)");
            Say("", "(\"At minus twenty-five the cold stops hurting,\" says the narrator. \"That's how you know.\")");
        } else if (ep == 3) {
            Say("", "(An old film. A dog waits at a train station every day for a man who never comes back.)");
            Say("", "(You change the channel. It's the same film. The dog is still waiting.)");
        } else {
            Say("", "(A magic show. The magician puts a boy in a box and closes the lid. He taps it twice. He opens it. Empty.)");
            Say("", "(Applause. The magician checks his watch. \"Same time tomorrow,\" he says. \"Same boy.\")");
        }
        if (Once("tv_last")) Say("You", "(It's showing me what happened last time. Nobody else watches it like that.)");
        return true;
    }
    // ---- the watch (T-064): lie down and actually look at it
    if (id == "bed" && !Tutorial()) {
        std::vector<ChoiceOpt> o = {{"bed_sleep", "Nap. (10 min)"}, {"bed_watch", "Look at my watch."}, {"none", "Nah."}};
        Choice("Lie down for a bit?", o);
        return true;
    }
    // the little screen in the corner, the second time you really look
    if (id == "screens" && Knows("basement_screens") && !Knows("w_screen")) {
        Say("", "(Every room. Every angle. Your bed. The toilet. The yard. The shed.)");
        Say("", "(The small screen in the corner. A watch. Not yours. Grey, scratched, the strap doesn't match.)");
        int left = std::max(0, visitorTime - (int)t);
        char b[32]; std::snprintf(b, sizeof b, "%d:%02d", left / 60, left % 60);
        Say("", std::string("(It isn't telling the time. It's counting down. ") + b + ")");
        Say("You", "(Down to ten to five. Somebody wants to know exactly when he gets here. Or he wants to know exactly when to come.)");
        AddClue("w_screen");
        return true;
    }
    // ---- the photos: somebody else grows up in them (Fourth)
    if (id == "album" && shift == Shift::Fourth) {
        Say("You", "(Me at 4. Behind me, a boy of about sixteen in a red sweater. He isn't looking at the camera. He's looking at me.)");
        Say("You", "(Me at 6. The same boy. Older. The sweater is too small for him now.)");
        Say("You", "(Me at 9 with a fish. A man at the back in a black coat. Same face. Snow on his shoulders. Indoors.)");
        Say("You", "(Still no baby photos. Not of me. Not of him.)");
        fourthNoticed = true;
        AddClue("no_baby");
        Achieve("Family Portrait");
        return true;
    }
    if (id == "coats" && shift == Shift::Fourth) {
        Say("You", "(Dad's jacket. Mom's jacket. My jacket. And a black coat, much too big, with snow still on the shoulders. It's dry underneath.)");
        Say("You", "(Nobody else lives here. Nobody else has ever lived here.)");
        AddClue("no_car");
        if (!hasCoat) Choice("Your jacket is on the hook.", {{"c_coat", "Put it on."}, {"none", "Leave it."}});
        return true;
    }
    // ---- oh-seven (NextUnit)
    if (id == "wardrobe" && shift == Shift::NextUnit) {
        Say("You", "(My clothes. Every label says S07. Under it you can feel the stitches where an older label was cut out.)");
        unitNoticed = true;
        return true;
    }
    if (id == "desk" && shift == Shift::NextUnit) {
        FadeFor(10, "Reading Mom's papers");
        Say("", "WEEKLY REPORT. SUBJECT 07. Week 1. Settling in well. Unaware.");
        Say("", "(Under it, an older page with one line through it: SUBJECT 06. Week 612. REISSUED.)");
        Say("You", "(Week one. I've lived here my whole life. Haven't I?)");
        unitNoticed = true;
        AddClue("reports");
        return true;
    }
    return false;
}

// ============================================================ choices
bool Story::MoreChoose(const std::string& id, int px, int py) {
    (void)px; (void)py;
    // ---- predictions
    if (id.size() > 3 && (id.rfind("mp_", 0) == 0 || id.rfind("dp_", 0) == 0)) { Predict(id[0] == 'm' ? "Mom" : "Dad", id.substr(3)); return true; }
    // ---- the watch
    if (id == "bed_watch") {
        Say("You", "(ORIGIN. Model 06. A plain face. No date. No buttons, except the one that doesn't do anything.)");
        Say("You", "(At 16:00 it beeps. Two short, one long. I never set an alarm. There's no way to set one.)");
        if (held == Item::BabyWatch) Say("You", "(I hold the tiny one next to it. Same face. Same beep, probably. It just stopped beeping for somebody smaller.)");
        else if (Knows("collector")) Say("You", "(His is the same. Scratched grey. Older. Like mine will be.)");
        AddClue("w_beep");
        return true;
    }
    // ---- the console, once you've seen his trunk
    if (id == "r_listen" && Knows("trunk") && !Knows("unit_04")) {
        Say("Voice", "(static) ...unit 04, Geilo, collected 13:10. Anchor reset. Dog destroyed...");
        Say("Voice", "...unit 05 collected 14:40... unit 06, 16:50...");
        Say("You", "(Dog destroyed. The hair in his trunk wasn't Doggo's colour.)");
        AddClue("unit_04");
        return true;
    }
    if (id == "r_confirm") {
        // the word from the Monster ending, said too early
        Say("You", "(You press PICKUP CONFIRM and hold it down.) Confirmed.");
        Say("Voice", "(a pause) ...Collector? Our clock says you're still at the gate.");
        Say("Voice", "(another voice, further off) That isn't him. That's the house.");
        Say("", "(The line clicks dead. Upstairs, very faintly, the landline starts to ring. Nobody answers it.)");
        Say("You", "(They heard me. They know where the word came from.)");
        return true;
    }
    // ---- early lessons (T-084). Hitting barely works on them. It won't work on him
    if (id == "m_shove") {
        Say("You", "(You shove her. Both hands. Everything you've got.)");
        Say("Mom", "(she takes one step back. One. She looks down at you.)");
        Say("Mom", tainted ? "(she doesn't say anything. She just keeps her hands where you can see them.)" : "...Did you just PUSH me?");
        Say("You", "(She barely moved. My wrists hurt.)");
        DarkAct(1, "", true);
        momAtt = tainted ? Attitude::Scared : Attitude::Cold;
        See("shove_mom");
    } else if (id == "d_hit") {
        Say("You", "(You punch him in the stomach. As hard as you can.)");
        Say("Dad", "(it's like hitting the generator. He doesn't even step back.)");
        if (tainted) Say("Dad", "(he doesn't catch it. He lets it land. Then he looks at you like he's writing it down.)");
        else Say("Dad", "(he takes your wrist, gently) Hey. HEY. What's that for, buddy?");
        Say("You", "(My knuckles hurt. He's fine. He isn't even angry.)");
        DarkAct(1, "", true);
        dadAtt = tainted ? Attitude::Scared : Attitude::Cold;
        See("hit_dad");
    }
    if (id == "m_shove" || id == "d_hit") {
        if (SawEvent("shove_mom") && SawEvent("hit_dad") && !Knows("small_hands")) {
            Narrate("I was twelve. Hitting only works in films. If I wanted to beat anyone in that house, I'd have to be smarter than them. Or bigger.");
            AddClue("small_hands");
        }
        return true;
    }
    // ---- Mom: the newer memories
    if (id == "m_hinges") {
        Say("You", "Why does my door have new hinges?");
        Say("Mom", "(she doesn't look up) Your father did them. The old ones squeaked.");
        Say("You", "Every door in this house squeaks. Only mine got new ones.");
        Say("Mom", tainted ? "(she looks down the hall at your door. Then at you.) ...So we'd hear you." : "(she keeps stirring. She stirs for a long time.)");
    } else if (id == "m_brother") {
        Say("You", "Did I ever have a big brother?");
        Say("Mom", "(a long pause) No, sweetie. Just you. There's only ever been you.");
        Say("You", "(Only ever. She says it like it means something else.)");
    } else if (id == "m_thirty") {
        Say("You", "What's thirty kilometres from here?");
        Say("Mom", "Nothing. Snow. Why?");
        Say("You", "Somebody walked that far once.");
        Say("Mom", "(the spoon stops) ...Then he must have been very cold.");
        Say("You", "(I didn't say he.)");
    } else if (id == "m_before") {
        Say("You", "Who did you hum to before me?");
        Say("Mom", "(the plate stays in her hand) ...Nobody. You're the only one I ever hummed to.");
        Say("You", "(She told him the same thing through the bathroom door. Week 612. He didn't believe her either.)");
        if (momAtt == Attitude::Normal) momAtt = Attitude::Cold;
    } else if (id == "m_door") {
        Say("You", "Did anybody ever open the door before he knocked?");
        Say("Mom", "(she laughs, but not really) Once. A long time ago. He stood right there in his socks.");
        Say("You", "What happened to him?");
        Say("Mom", "(she turns back to the stove) He caught a cold.");
    } else if (id == "m_lived") {
        Say("You", "If he didn't take his medicine, would he remember this house?");
        Say("Mom", "(she goes very still) Who, sweetie?");
        Say("You", "The man. The pen in his coat.");
        Say("Mom", "...Go and wash your hands for dinner.");
        Say("You", "(She didn't say no.)");
    } else if (id == "m_plate") {
        Say("You", "Can we set a fourth plate tonight?");
        Say("Mom", "(she laughs) For who? Julenissen?");
        Say("You", "For whoever comes.");
        Say("", "(She looks at you for a long time. Then she takes a fourth plate down from the shelf and puts it on the table. She doesn't ask again.)");
    } else if (id == "m_deliver") {
        Say("You", "If he's gone, don't drive me there. Handlers don't deliver.");
        Say("Mom", "(the glass stops halfway to the shelf) ...Drive you where?");
        Say("You", "Finse. They walk you round the back of the building.");
        Say("Mom", "(she puts the glass down very carefully) We're retiring after today, sweetie. They promised.");
        Say("You", "(She says it like a prayer. It's the only one she knows.)");
    } else if (id == "m_kids") {
        solemn = true;
        Say("You", "There are other kids at Finse. Rooms full of them. They all wear my watch.");
        Say("Mom", "(she sits down at the kitchen table. Slowly, like it hurts.) ...How many?");
        Say("You", "All of them.");
        Say("Mom", "(very quietly) They told us you were the only one. They told us you were special.");
        solemn = false;
    } else if (id == "m_fourth") {
        Say("You", "Who's the man at the back of the photos?");
        Say("Mom", "(she looks over your shoulder at nothing, smiling) That's you, sweetie.");
        Say("You", "(She goes back to the stove like she said something normal. I'm in the front. I'm always in the front.)");
    } else if (id == "m_unit") {
        Say("You", "Why does everybody call me oh-seven?");
        Say("Mom", "(she smiles) Because that's you. Lucky number seven. They gave you a lucky one.");
        Say("You", "(Then who got six?)");
        Achieve("Next in Line");
    }
    // ---- Dad: the newer memories
    else if (id == "d_knot") {
        Say("You", "Who else did you teach your knot?");
        Say("Dad", "Nobody, buddy. Just you.");
        Say("Dad", "(he ties one in the rag without looking. Then he looks at it for a long time.)");
        Say("Dad", "...Just you.");
    } else if (id == "d_hinges") {
        Say("You", "Why did you put new hinges on my door?");
        Say("Dad", "Old ones broke.");
        Say("You", "How?");
        Say("Dad", "(he picks the wrench back up) Somebody pushed a chest of drawers against it. Long time ago. Before your time.");
        Say("You", "(Before my time. In my room.)");
    } else if (id == "d_nail") {
        Say("You", "Did you scratch my watch with a nail?");
        Say("Dad", "(he looks at his hands) ...Four days after. You kicked so hard I got the six wrong twice.");
        Say("Dad", "I wanted one thing on it that wasn't theirs.");
        Say("You", "(Crooked letters. On the tiny watch. And on his.)");
    } else if (id == "d_reissue") {
        Say("You", "What does reissue mean?");
        Say("Dad", "(he turns the radio face down without looking at it) It's when they print a book again. Same book. New cover.");
        Say("You", "Same house?");
        Say("Dad", "...Go inside, buddy.");
    } else if (id == "d_coat") {
        Say("You", "Whose coat did the man wear before him?");
        Say("Dad", "(a long time) I don't know his name. I knew the one before.");
        Say("You", "What was he like?");
        Say("Dad", "Quiet. Good with knots.");
    } else if (id == "d_ice") {
        Say("You", "What's on the rope in the ice out back?");
        Say("Dad", "Fish trap. I check it every Christmas.");
        Say("You", "The end's cut.");
        Say("Dad", "(he doesn't look up) ...Then something got away.");
    } else if (id == "d_0612") {
        Say("You", "What's 0612?");
        Say("Dad", "(the wrench stops) That's a gate code. From work. Where did you...");
        Say("Dad", "(he checks that the radio is off. Twice.) Don't say numbers out loud in this house, buddy. Somebody's always listening.");
    } else if (id == "d_hide") {
        solemn = true;
        Say("You", "If he can't find me, he starts on you.");
        Say("Dad", "(he puts the wrench down) Who's he?");
        Say("You", "You know who.");
        Say("Dad", "(a long breath) ...Then you let him find you, buddy. You hear me? Don't you dare stay hidden for us.");
        Say("You", "(He means it. He's the one who'd scream first.)");
        solemn = false;
    } else if (id == "d_almond") {
        Say("You", "Do you always get the almond?");
        Say("Dad", "(he grins) Every single year. Your mother's a terrible cheat.");
        Say("You", "(He doesn't know I watched her do it. He doesn't know there's a last bell.)");
    } else if (id == "d_fourth") {
        Say("You", "Who's the man at the back of the photos?");
        Say("Dad", "That's just you, buddy. At the back. You always stand at the back.");
        Say("You", "(I'm in the front. In every single one.)");
    } else if (id == "d_unit") {
        Say("You", "Why did you call me oh-seven?");
        Say("Dad", "(he frowns, like you're joking) Because that's your number, buddy.");
        Say("Dad", "Zero-six was before you. Good kid. You've got his room.");
        Say("You", "(Same house. Next cycle.)");
        Achieve("Next in Line");
    }
    // ---- the man at the gate: what the endings taught
    else if (id == "v_ask") {
        std::vector<ChoiceOpt> o = VisitorQuestions();
        o.push_back({"v_leave", "(Back away.)"});
        Choice("What do I say to him?", o);
    } else if (id == "v_well") {
        Say("You", "Under the well.");
        Say("The visitor", "(he almost smiles) Third dungeon. Everybody looks in the tower.");
        Say("The visitor", "(then it's gone) ...How do you know I know that?");
        VisitorNoticesYou();
    } else if (id == "v_dog") {
        Say("You", "Did you have a dog?");
        Say("The visitor", DoggoExists() ? "(he looks at Doggo behind your legs for a long time) ...Once. Same house." : "(he looks at the doghouse for a long time) ...Once. Same house.");
        Say("The visitor", "Go inside, 06.");
        VisitorNoticesYou();
    } else if (id == "v_cameout") {
        Say("You", "You always came out for them.");
        Say("The visitor", "(he goes very still. Snow settles on his shoulders.) ...Who told you that?");
        Say("You", "You did.");
        VisitorNoticesYou();
    } else if (id == "v_twice") {
        Say("You", "Your watch goes round my wrist twice.");
        Say("The visitor", "(he looks down at his own wrist) Twice. It did on me too. The first year.");
        Say("The visitor", "(he pulls his glove back over it) You don't know that. Nobody's worn this but me.");
        VisitorNoticesYou();
    } else if (id == "v_after") {
        Say("You", "After. It's after now.");
        Say("", "(His hand goes to his coat. To the pen. It stays there.)");
        Say("The visitor", "...Who says that?");
        Say("You", "You do. Next time.");
        VisitorNoticesYou();
    } else return false;
    return true;
}
