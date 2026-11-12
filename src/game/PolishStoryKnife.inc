// 600 Seconds - v0.20.1 polish, lane story-knife. Included at the end of Story.cpp, inside the namespace. Engine-free.
// Oh-seven says 07 where you'd hear it, a last beat for the two new realities, four hunt payoffs for the v0.20 memories,
// the brake line in Screen Time, one knife job at DARK_REVEL (the door frame), and Mom when the knife is for a clean job.
// Every line here is a proposal for Noah.

// ============================================================ oh-seven: the number people say out loud
// Only what is heard: the visitor, the radio voices, Dad's radio, anybody talking, and quotes on the black screens.
// Never your own lines, never papers you read, never ANCHOR 06 (that's the watch, and a hinge piece), never memories.
static std::string OhSevenSpan(const std::string& t, bool words) {
    std::string out; out.reserve(t.size() + 8);
    for (size_t i = 0; i < t.size();) {
        if (words && (t.compare(i, 8, "Zero-six") == 0 || t.compare(i, 8, "zero-six") == 0)) {
            out += t[i] == 'Z' ? "Zero-seven" : "zero-seven"; i += 8; continue;
        }
        if (t.compare(i, 2, "06") == 0) {
            char p = i ? t[i - 1] : ' ';
            char n = i + 2 < t.size() ? t[i + 2] : ' ';
            bool lead = p == ' ' || p == '"' || p == '(';
            bool tail = !((n >= '0' && n <= '9') || (n >= 'A' && n <= 'Z') || (n >= 'a' && n <= 'z'));
            bool keep = (i >= 7 && t.compare(i - 7, 7, "ANCHOR ") == 0) ||
                        (i >= 6 && (t.compare(i - 6, 6, "model ") == 0 || t.compare(i - 6, 6, "MODEL ") == 0 || t.compare(i - 6, 6, "Model ") == 0));
            if (lead && tail && !keep) { out += "07"; i += 2; continue; }
        }
        out += t[i++];
    }
    return out;
}
std::string Story::UnitLine(const std::string& who, const std::string& text) const {
    if (!OhSeven() || (text.find("06") == std::string::npos && text.find("ero-six") == std::string::npos)) return text;
    if (who == "You" || who == "Doggo") return text;
    // the visitor and the radio call you by the word too. Dad's "Zero-six was before you" stays
    if (!who.empty()) return OhSevenSpan(text, who == "The visitor" || who == "Voice" || who == "Finse");
    // black screens and narration: only what somebody says, in quotes
    std::string out; size_t i = 0;
    for (;;) {
        size_t a = text.find('"', i);
        size_t b = a == std::string::npos ? a : text.find('"', a + 1);
        if (b == std::string::npos) { out += text.substr(i); break; }
        out += text.substr(i, a + 1 - i);
        out += OhSevenSpan(text.substr(a + 1, b - a - 1), true);
        out += '"';
        i = b + 1;
    }
    return out;
}

// ============================================================ the two new realities, on the way to the bells
void Story::ShiftTrunk() {
    if (shift == Shift::Fourth) {
        Black("Under the blanket in the trunk, something soft. A red sweater. Too small for a man. Still a bit too big for you.");
        if (fourthNoticed) Black("The boy in the photos wore it. Then he grew out of it. Then he grew into the coat.");
        Black("It smells like snow, and like your house.");
    } else if (shift == Shift::NextUnit) {
        Black("Through the gap behind the back seat: the dome light, and his clipboard. 06 has a line through it. Over it, in Mom's handwriting: 07.");
        Black("\"Seven,\" he says to nobody. \"Huh. They told me six.\"");
        if (unitNoticed) Black("Week one, Mom's report said. You lie there and wonder who gets your room for week two.");
    }
}

// ============================================================ hunt payoffs
// The watch: two short, one long. Out of marbles, you can throw that instead. He always goes to look. Once a loop
bool Story::CanThrowWatch() const {
    return chase && !watchThrown && !Tutorial() && !handlers && (Knows("w_beep") || Knows("w_drift"));
}
bool Story::OnWatchLure() {
    if (!chase || watchThrown) return false;
    watchThrown = true;
    Bubble("The visitor", "(two short, one long, from across the room) ...Drift?");
    return true;
}

// Small hands: hitting doesn't work. So the first time he grabs you, you don't fight. You go heavy
bool Story::SlipGrip() {
    if (slipped || !Knows("small_hands") || Tutorial() || handlers || visitorDowned) return false;
    slipped = true;
    Say("", "(A hand like a vice closes on your collar.)");
    Say("You", "(Hitting doesn't work. Not on him. So I don't.)");
    Say("", "(You go limp. Arms up, all of you at once, like a sack of flour. Your sweater comes off over your head.)");
    Bubble("The visitor", "(he's holding a red sweater) ...Huh. I used to do that.");
    visitorStun = 1.5f;
    SceneBeat("slip");   // the presentation goes back to the hunt
    return true;
}

// You told Dad about the knock. He didn't believe you enough to run. He believed you enough to stand in the door
bool Story::DadReadyAtDoor(const std::string& how) {
    if (!firedEvents.count("pd_knock") || how == "door" || Tutorial() || !DadFree()) return false;
    if (dadAtt == Attitude::Scared || dadAtt == Attitude::Broken || stance == Stance::Dark) return false;
    if (frontLocked || visitorDowned || nobodyHome || parentsAllied || Defender() == "Dad") return false;
    dadReady = true;
    Say("Dad", "(he's already there, wrench in his hand. He fills the doorway and doesn't step aside.)");
    Say("The visitor", "(he looks at the wrench. Then past Dad, at you.) ...Somebody told you I was coming.");
    Say("Dad", "(without turning round) Go to your room, buddy. Now.");
    Say("You", "(He believed me. Not enough to run. Enough to stand there.)");
    if (huntStart.empty()) huntStart = "door";   // he stands on the step a while, counting rooms
    return true;
}

// ============================================================ Mom and the knife
static bool NearObj(const World& w, const char* id, int px, int py) {
    for (auto& o : w.objs) if (o.id == id) return std::abs(o.x - px) + std::abs(o.y - py) <= 4;
    return false;
}
bool Story::KnifeLooksClean(int px, int py) const {
    if (darkThisLoop > 0 || tainted) return false;   // the dark path: she still scolds
    if (knifeCleanLoop || doggoTangled) return true;
    return (NearObj(world, "tree", px, py) && !tapeCut) || (NearObj(world, "woodpile", px, py) && !firewoodDone);
}
void Story::MomSeesKnife(int px, int py) {
    if (!KnifeLooksClean(px, py)) {
        Say("Mom", "Why are you holding that? Put. It. Down.");
        ChangeMood(-1);
        return;
    }
    Say("Mom", "(she sees the knife. She doesn't shout.) Careful with that. What are you cutting?");
    if (doggoTangled) {
        Say("You", "Doggo's all tangled up in the ribbon.");
        Say("Mom", "(she looks at the blade, then at you) Away from him. Not towards. Go on.");
    } else if (NearObj(world, "tree", px, py) && !tapeCut) {
        Say("You", "Just the tape on the big present. I won't tear the paper.");
        Say("Mom", "(something crosses her face) ...Leave the presents, sweetie. They're for after.");
    } else if (NearObj(world, "woodpile", px, py) && !firewoodDone) {
        Say("You", "The twine on the firewood. Like Dad does.");
        Say("Mom", "Like Dad does. Blade away from you, then.");
    } else {
        Say("You", "Just twine and stuff. I'm careful.");
        Say("Mom", "(she watches the blade, not you) Point down. Like your father showed you.");
    }
}

// ============================================================ DARK_REVEL: the door frame
bool Story::PolishInteract(const std::string& id, int px, int py) {
    (void)px; (void)py;
    if (id == "door_kid" && !carveSkip && hasKnife && !frameCarved && darkPoints >= DARK_REVEL && !collection && !Tutorial()) {
        Choice("Your door. Mom's pencil lines up the frame, a date by every one.",
               {{"kn_carve", "Carve 06 into the frame. Where she measures me.", false, true}, {"kn_doorplain", "Just the door."}, {"none", "Leave it."}});
        return true;
    }
    return false;
}
bool Story::PolishChoose(const std::string& id, int px, int py) {
    if (id == "kn_doorplain") { carveSkip = true; Interact("door_kid", px, py); carveSkip = false; return true; }
    if (id == "kn_carve") {
        frameCarved = true;
        Say("", "(Mom's pencil lines up the frame. 4. 6. 9. 12. A date by every one, in her writing.)");
        if (OhSeven()) Say("You", "(Everybody calls me oh-seven today. I carve 06 anyway.)");
        Say("", "(You put the point in above the last line and carve. 0. 6. Deep, so it won't paint over.)");
        Say("", "(The paint flakes off round it. Under the paint there are more lines. Older. Scratched, not pencil.)");
        Say("", "(They don't stop at twelve. They go on up the frame, past your head. The top one is a man's height.)");
        Say("", "(Next to it, scratched in, crooked: 06.)");
        DarkAct(2, "", true);
        Say("You", "(Somebody grew up in this doorway. All the way up.)");
        AddClue("under_paint");
        return true;
    }
    return false;
}

// ============================================================ Screen Time, with the brake line cut
// Same ending, same joke. The car just doesn't make it to Finse
void Story::ScreenTimeBrakes() {
    Black("Two bends later he brakes. Then he brakes harder.");
    Black("\"Huh,\" he says. Not to you.");
    Black("You don't look up. You're nearly at the boss.");
    Sound("thud");
    Black("GAME OVER, says the Fliptendo. For once it's right.");
    Black("The screen stays lit in the snow for a long time. Nobody presses continue.");
    AddClue("brake_line");
    Narrate("I cut his brakes. Then I played three games in a row and forgot. That's the kind of hour it was.");
    endings.insert("screen");
    lastEnd = "ending:screen";
    Achieve("Screen Time");
    Beat e; e.type = BeatType::Ending; e.id = "screen"; e.speaker = "JOKE ENDING";
    e.text = "SCREEN TIME\n\nHe reached the last dungeon. The car reached the last bend.";
    beats.push_back(e);
}
