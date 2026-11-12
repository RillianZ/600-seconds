// 600 Seconds - the kitchen knife (v0.20, lane knife). Engine-free.
// Included at the end of Story.cpp, inside the namespace, so the story test still builds from one file.
//
// Clean jobs first, no darkness for any of them: the tape on the big present, the twine on the firewood,
// Doggo in the ribbon at 16:15, the frozen window in your room, and the twine in the torn fence out back
// (before the hunt, or during it). The knife can come out of the block during the hunt now too.
// Darker jobs open as you darken: the phone cord (3), his brake line (8), the wire to the box on his belt (tainted).
// And Noah's request at 15: the bathroom mirror. Off screen, and never a way out.

// ---------------------------------------------------------------- queries
bool Story::KnowsKnifeJob() const {
    for (auto& e : seenEvents) if (e.rfind("kn_", 0) == 0) return true;   // kn_hint: seen where one is. The rest: done one
    return false;
}
bool Story::MirrorReady() const {
    return hasKnife && tainted && darkPoints >= DARK_MAX && !collection && shift != Shift::Sister && Knows("mirror_lag");
}
int Story::MemoryTotal() const {
    int n = 0;
    for (auto& m : AllMemories()) if (!QuietMemory(m.id) || Knows(m.id)) ++n;
    return n;
}
void Story::BladeCheck() {
    int n = 0;
    for (const char* e : {"kn_tape", "kn_twine", "kn_ribbon", "kn_gap", "kn_window"}) n += SawEvent(e) ? 1 : 0;
    if (n >= 3) Achieve("Blade Down");
}

// ---------------------------------------------------------------- 16:15, Doggo and the ribbon
void Story::KnifeEvents(float prev, float now, int px, int py) {
    if (collection || Tutorial()) return;
    Room here = world.RoomAt(px, py);
    auto crossed = [&](int m) { return prev < m * 60.0f && now >= m * 60.0f; };
    // he goes where you go. In the living room he goes under the tree, after the smell of the presents
    if (crossed(15) && now - 15 * 60 < 90 && here == Room::LivingRoom && DoggoExists() && !doggoStay && Once("ribbon")) {
        doggoTangled = true;
        See("ribbon");
        Bubble("Doggo", "*noses in under the tree. Comes out wearing the ribbon off the big present. Spins. It gets worse.*");
    }
    if (crossed(21) && doggoTangled) {
        doggoTangled = false;
        Sound("glass");
        if (here == Room::LivingRoom || here == Room::Hallway || here == Room::Kitchen)
            Bubble("Doggo", "*yanks free. A bauble goes. He shoots out from under the tree with a bow on one leg.*");
        if (MomHere() && (here == Room::LivingRoom || here == Room::Kitchen)) Bubble("Mom", "What was THAT? ...Doggo. Oh, Doggo.");
    }
}

// ---------------------------------------------------------------- objects
bool Story::KnifeInteract(const std::string& id, int px, int py) {
    if (chase) {
        // the hunt: the block is right there. Two quick ones, or none if you had it earlier this loop
        if (id == "knives") {
            if (hasKnife) { Bubble("You", "(Already got it.)"); return true; }
            if (knifeTakenLoop) { Hold(Item::Knife, true); Bubble("You", "(Back in my hand. Easier every time.)"); return true; }
            Choice("The knife block. He's in the house. Take it? (1/2)", {{"kn_hunt", "Take it.", false, true}, {"none", "Leave it."}});
            return true;
        }
        if (id == "chaingap") {
            if (gapCut) { Bubble("You", "(My size. Not his.)"); return true; }
            if (!hasKnife) { Bubble("You", "(Twine all through it. With the kitchen knife I'd be through in a second.)"); See("kn_hint"); return true; }
            gapCut = true;
            Bubble("You", "(Three cuts through the old twine. A door my size now. Not his.)");
            AddClue("kid_door"); See("kn_gap"); BladeCheck();
            return true;
        }
        if (const WindowInfo* w = WindowById(id); w && hasKnife && WindowSticks(id)) {
            bool inside = w->wy <= 2 ? py > w->wy : py < w->wy;
            if (!inside) return false;
            windowBanged = true;   // no bang, no thuds: the presentation reads WindowSticks when the climb starts
            Bubble("You", "(Blade round the frozen frame. It gives without a sound.)");
            See("kn_window"); BladeCheck();
            SceneBeat("climb:" + id);
            return true;
        }
        return false;
    }
    if (collection) return false;
    if (id == "chaingap") {
        if (gapCut) { Say("You", "(A door my size. I checked twice. He won't fit.)"); return true; }
        if (!hasKnife) {
            Say("You", "(The torn bit of the old fence. Somebody laced it shut with baler twine, years ago. I can just about squeeze through.)");
            Say("You", "(The kitchen knife would go through that twine like nothing.)");
            See("kn_hint");
            return true;
        }
        Choice("The torn bit of the old fence, laced shut with baler twine.", {{"kn_gap", "Cut the twine out. Make it a door my size. (2 min)"}, {"none", "Leave it."}});
        return true;
    }
    if (id == "doggo" && doggoTangled) {
        std::vector<ChoiceOpt> o;
        if (hasKnife) o.push_back({"kn_ribbon", "Cut the ribbon. Blade away from him."});
        o.push_back({"kn_untangle", "Untangle him. (3 min)"});
        o.push_back({"none", "Leave him."});
        if (!hasKnife) See("kn_hint");
        Choice(hasKnife ? "Doggo, wrapped in ribbon. The more he spins, the worse it gets."
                        : "Doggo, wrapped in ribbon. The more he spins, the worse it gets. A knife would do it in one go.", o);
        return true;
    }
    if (id == "mirror" && MirrorReady()) {
        // Noah's request. The quiet option comes first, so a stray key press never picks the other one
        Choice("The one in the mirror is older. He isn't smiling. The knife is still in your hand.", {{"none", "Look away."}, {"kn_mirror", "End it.", false, true}});
        return true;
    }
    if (id == "phone" && phoneCut) { Say("You", "(Dead. Behind the bench the cord just stops. Nobody's plugging that back in.)"); return true; }
    if (id == "phone" && phoneUnplugged && hasKnife && darkPoints >= DARK_ARSENAL) {
        Say("You", "(Dead. The plug is behind the bench, where I left it.)");
        Choice("The landline.", {{"p_plug", "Plug it back in."}, {"kn_cord", "Cut the cord. Then nobody can.", false, true}, {"none", "Leave it."}});
        return true;
    }
    // where a clean job is, before you've done one. Once a loop
    if (id == "tree" && !hasKnife && !tapeCut && !Tutorial() && Stage() >= 1 && !SawEvent("kn_tape") && Once("tape_hint")) {
        Say("You", "(The tape on the big one is the clear kind. Something thin and sharp would slide right under it. The paper wouldn't even tear.)");
        See("kn_hint");
        return false;   // and then the tree as usual
    }
    if (id == "woodpile" && !hasKnife && !firewoodDone && !Tutorial() && Stage() >= 1 && Once("twine_hint")) {
        Say("You", "(Every bundle is tied with twine. Dad does it with the kitchen knife. One cut each. I use my teeth.)");
        See("kn_hint");
        return false;
    }
    // his car with the engine already off: the brake line is still there
    if (id == "car" && !Tutorial() && CarEmpty() && !carKey.empty() && hasKnife && darkPoints >= DARK_INTERROGATE && !brakesCut) {
        See("car");
        Say("You", "(The engine's off. The car is already getting cold.)");
        Choice("His car. Dark. Nobody in it.", {{"kn_brake", "Crawl under it. Cut the brake line.", false, true}, {"none", "Get away from it."}});
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- choices
bool Story::KnifeChoose(const std::string& id) {
    // ---- clean
    if (id == "wp_stack" && hasKnife) {
        FadeFor(Dur(12), "Stacking firewood. One cut per bundle.");
        firewoodDone = true;
        Say("You", "(One cut per bundle, blade away from me. Twelve minutes instead of twenty. Dad's trick.)");
        See("kn_twine"); BladeCheck();
        return true;
    }
    if (id == "kn_tape") {
        tapeCut = true;
        Say("", "(You turn the big one over and slide the blade under the clear tape. Slowly. The paper doesn't tear.)");
        Say("", "(Tissue paper, and nothing. Under the tissue paper, a packing slip.)");
        Say("", "FINSE SUPPLY. UNIT 06. 3 x GIFT, DECORATIVE. RETURN TO STORES AFTER COLLECTION.");
        Say("You", "(Somebody orders my Christmas. And takes it back after.)");
        Say("", "(You fold it all back the way it was. Nobody could tell.)");
        AddClue("presents");
        AddClue("packing_slip");
        See("kn_tape"); BladeCheck();
        return true;
    }
    if (id == "kn_ribbon") {
        doggoTangled = false;
        Say("", "(You slide the blade under the ribbon, edge away from him, and cut once.)");
        Say("Doggo", "*sneezes. Shakes. Leans his whole weight on your legs like you saved his life.*");
        See("kn_ribbon"); BladeCheck();
        return true;
    }
    if (id == "kn_untangle") {
        FadeFor(Dur(3), "Untangling Doggo");
        doggoTangled = false;
        Say("You", "(Three minutes, one very patient dog, and a ribbon that's never going back on that present.)");
        return true;
    }
    if (id == "kn_gap") {
        FadeFor(Dur(2), "Cutting the twine out of the fence");
        gapCut = true;
        Say("You", "(Three cuts through the old twine, and the loose ends fold back. A door my size.)");
        Say("You", "(Not his. He'd have to go round by the gate.)");
        AddClue("kid_door"); See("kn_gap"); BladeCheck();
        return true;
    }
    if (id.rfind("kn_window:", 0) == 0) {
        windowBanged = true;
        Say("You", "(Blade round the frozen frame, slow, all the way round. It gives without a bang.)");
        See("kn_window"); BladeCheck();
        SceneBeat("climb:" + id.substr(10));
        return true;
    }
    if (id == "kn_hunt") {
        Say("You", tainted ? "(It fits my hand.)" : "(Blade down. Like Dad showed me. Even now.)");
        Choice("Take the knife? (2/2)", {{"kn_hunt2", "Take it.", false, true}, {"none", "Leave it."}});
        return true;
    }
    if (id == "kn_hunt2") {
        Hold(Item::Knife, true);
        Bubble("You", "(In my sleeve. Point down.)");
        return true;
    }
    // ---- dark
    if (id == "kn_cord") {
        phoneUnplugged = phoneCut = true;
        Say("You", "(Behind the bench, where nobody looks. Two cuts, so there's a piece missing.)");
        DarkAct(1, "", true);
        Say("You", "(No call. No \"confirm pickup\". And nobody plugs that back in.)");
        return true;
    }
    if (id == "kn_brake") {
        brakesCut = true;
        FadeFor(1, "Under his car");
        Say("", "(On your back in the snow under his car. It's warm under here. Something ticks.)");
        Say("", "(Dad showed you once, under the old car out back. The thin pipe that runs to the wheel. You cut it.)");
        Say("", "(It drips into the snow. The snow covers it.)");
        DarkAct(3, "", true);
        Say("You", "(He'll be in a hurry. The road down off the vidda is all bends.)");
        return true;
    }
    if (id == "kn_wire") {
        switchTaken = true;
        Say("", "(The black box on his belt. A wire runs from it up under his shirt. You don't unclip anything. You cut the wire.)");
        Say("", "(The green light doesn't go red. It just goes out.)");
        Say("The visitor", "(very quietly) ...Huh. Nobody ever did it like that.");
        DarkAct(1, "", true);
        Choice("Are you sure?", {{"c_finish2", "Do it.", false, true}, {"none", "(Back away.)"}});
        return true;
    }
    if (id == "kn_mirror") {
        Choice("Are you sure?", {{"none", "No."}, {"kn_mirror2", "Yes.", false, true}});
        return true;
    }
    if (id == "kn_mirror2") { MirrorEnd(); return true; }
    return false;
}

// ---------------------------------------------------------------- his car, after the brake line
// who: "trunk" (he drives you), "Dad" / "Mom" / "both" (The Handlers), "escape" (False Hope). All off screen
bool Story::BrakesGo(const std::string& who) {
    if (!brakesCut) return false;
    solemn = true;
    Sound("hush_on");
    if (who == "trunk") {
        Black("He drives fast for a man with a kid in his trunk. The road down off the vidda is all bends.");
        Black("The first bend, he brakes. The second, he brakes harder. Through the lid you hear him say one word.");
        Sound("thud");
        Black("The third bend, the car doesn't.");
        Black("Then it's very quiet. Snow ticking on the lid.");
        if (!switchTaken) Black("The place behind your ear goes hot. Somewhere in front of you his heart has stopped, and the box on his belt doesn't wait for anybody.");
        else Black("Nobody comes. The storm does the rest.");
        Narrate("I cut his brakes. I was in his trunk. I knew both of those things. I did it anyway.");
        AddClue("brake_line");
        lastEnd = "brakes";
        solemn = false;
        Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
        return true;
    }
    bool both = who == "both", escape = who == "escape";
    std::string driver = escape ? "Mom" : both ? "Dad" : who;
    std::string he = driver == "Dad" ? "He" : "She";
    if (escape) Black("At the first steep bend Mom brakes, and nothing happens. You know why before she does.");
    else {
        Black("The road down off the vidda is all bends. " + driver + " takes the first one slowly. " + he + " was always careful.");
        Black("At the second bend " + driver + " puts a foot on the brake, and nothing happens.");
        Black("\"Oh,\" " + driver + " says. Just that.");
    }
    if (both) Black("Mom finds your hand in the back seat.");
    else if (escape) Black("Dad finds your hand in the back seat.");
    Sound("thud");
    Black("Then the snow.");
    Narrate(escape ? "I cut those brakes for him. We were the ones in his car."
                   : both ? "I cut his brakes. They were the ones who drove." : "I cut his brakes. " + driver + " was the one who drove.");
    AddClue("brake_line");
    solemn = false;
    Beat e; e.type = BeatType::Ending; e.speaker = "ENDING";
    if (escape) {
        endings.insert("escape"); lastEnd = "ending:escape"; Achieve("False Hope");
        e.id = "escape"; e.text = "FALSE HOPE\n\nThey ran together, in his car. You had already seen to his car.";
    } else {
        endings.insert("handlers"); lastEnd = "ending:handlers"; Achieve("Family Business");
        e.id = "handlers"; e.text = "THE HANDLERS\n\nThey finished his job. His car finished theirs.";
    }
    beats.push_back(e);
    return true;
}

// ---------------------------------------------------------------- the mirror
// Noah's request (his D3: the loop is quantum immortality). Off screen: it cuts to black before anything happens.
// No sound, no popup, no achievement. The memory goes in quietly. Then 16:00 and the watch, like every loop
void Story::MirrorEnd() {
    solemn = true;
    FadeFor(0, "");
    Sound("hush_on");
    Black("Nothing. For no time at all.");
    Narrate("It wasn't a way out. There isn't one in here. The hour doesn't let go. It just starts again, and I'm still in it.");
    if (!memory.count("no_way_out")) { memory.insert("no_way_out"); loopNew.push_back("no_way_out"); }
    lastEnd = "mirror";
    solemn = false;
    Beat e; e.type = BeatType::EndLoop; beats.push_back(e);
}
