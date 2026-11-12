// 600 Seconds - v0.20 systems: where Mom and Dad are on a normal Christmas Eve, and what they're doing.
// Engine-free. Story::Mom() and Story::Dad() check the fixed beats first (the phone, the basement,
// the bathroom, the firewood, the flagpole, the corner), then fall back to this table.
// Each spot can have one idle line, a bubble that only shows if you're close when it happens.
#pragma once

namespace sixhundred {

struct DaySpot {
    int from, to;        // game seconds since 16:00
    int x, y;            // tile
    int face;            // -1 = whatever way they walked in. 0 down, 1 up, 2 left, 3 right
    int lineAt;          // game seconds since 16:00 for the idle line (0 = no line)
    const char* line;    // the bubble
};

// Mom. The phone (16:21/16:22), the basement (16:28 to 16:33), the bathroom (16:46 to 16:48:30)
// and the corner (later loops) are handled in Story::Mom() and win over this table.
inline const DaySpot* MomDay(int& n) {
    static const DaySpot d[] = {
        {0,             2 * 60 + 30,  8, 16, -1, 0, nullptr},
        {2 * 60 + 30,   6 * 60 + 30,  8, 14,  1, 3 * 60,           "(she peels potatoes into the sink. The peel comes off in one long curl.)"},
        {6 * 60 + 30,  11 * 60,      11, 14,  1, 7 * 60,           "(she opens the oven and pokes the ribbe. The crackling isn't crackling yet.)"},
        {11 * 60,      14 * 60 + 30, 25, 16,  1, 12 * 60,          "(she moves one bauble on the tree two centimetres to the left. Then back.)"},
        {14 * 60 + 30, 20 * 60,      20, 16,  1, 15 * 60,          "Shh! It's starting. Kaptein Fjord!"},
        {20 * 60,      24 * 60,       8, 16, -1, 0, nullptr},
        {24 * 60,      28 * 60,      11, 14,  1, 24 * 60 + 30,     "(she bastes the ribbe and burns her thumb. She doesn't make a sound.)"},
        {28 * 60,      33 * 60,       8, 16, -1, 0, nullptr},
        {33 * 60,      36 * 60,      24, 16,  1, 33 * 60 + 30,     "(she lines up the presents under the tree. Biggest at the back. She shakes one of yours, and listens.)"},
        {36 * 60,      40 * 60,       8, 16, -1, 0, nullptr},
        {40 * 60,      43 * 60,       8, 16, -1, 40 * 60 + 40,     "Lefse with butter and sugar. One each. Dinner's at five."},
        {43 * 60,      46 * 60,      11, 14,  1, 0, nullptr},
        {46 * 60,      60 * 60,       8, 16, -1, 0, nullptr},
    };
    n = (int)(sizeof d / sizeof d[0]);
    return d;
}

// Dad. The firewood (16:36), the flagpole (16:43:30) and everything after the knock are in Story::Dad().
inline const DaySpot* DadDay(int& n) {
    static const DaySpot d[] = {
        {0,             8 * 60,      24, 16, -1, 0, nullptr},                                  // the paper (bubble at 16:04)
        {8 * 60,       14 * 60 + 30, 35,  8, -1, 0, nullptr},                                  // the generator, the radio at 16:10
        {14 * 60 + 30, 20 * 60,      22, 16,  1, 16 * 60,          "(he laughs at the joke a second before Kaptein Fjord says it) Every year!"},
        {20 * 60,      27 * 60,      35,  8, -1, 0, nullptr},                                  // whistling at 16:25
        {27 * 60,      31 * 60,      21, 23, -1, 27 * 60 + 30,     "(shovelling the path down to the gate) There. Nice and clear for... for Julenissen."},
        {31 * 60,      38 * 60,      35,  8, -1, 0, nullptr},                                  // "Copy. No change." at 16:33. Firewood 16:36 to 16:38 wins
        {38 * 60,      40 * 60,      35,  8, -1, 0, nullptr},
        {40 * 60,      45 * 60 + 30, 10, 17, -1, 42 * 60,          "(he takes a second lefse while Mom isn't looking, and winks at you)"},
        {45 * 60 + 30, 47 * 60 + 30, 26, 18,  3, 46 * 60 + 10,     "(he feeds the stove another log and watches it catch. He doesn't blink much.)"},
        {47 * 60 + 30, 60 * 60,      10, 17, -1, 0, nullptr},
    };
    n = (int)(sizeof d / sizeof d[0]);
    return d;
}

// T-086: past this many darkness points (and tainted) he comes EARLY_KNOCK seconds early. The car too
constexpr int DARK_EARLY = 10;
constexpr int EARLY_KNOCK = 120;
inline bool gEarlyKnock = true;   // the story test's older loops are timed for 16:50 and switch this off

inline const DaySpot* DaySpotAt(const DaySpot* d, int n, float t) {
    for (int i = 0; i < n; ++i) if (t >= d[i].from && t < d[i].to) return &d[i];
    return &d[n - 1];
}

} // namespace sixhundred
