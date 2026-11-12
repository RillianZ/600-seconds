// 600 Seconds - v0.20 systems: the rumor map (T-092). Engine-free data.
// Memories as cards on a grid, with a line between two that belong together.
// The memory log draws a card once you have the memory, and a "?" card for an unknown one
// that is linked to one you have. Everything else stays hidden.
// Layout: 5 columns (0 to 4), rows from 0 down. Keep linked cards next to each other
// (sideways, up/down or diagonal) so the lines don't run under other cards.
#pragma once
#include <vector>

namespace sixhundred {

struct RumorNode { const char* id; int col, row; };
struct RumorLink { const char* a; const char* b; };

inline const std::vector<RumorNode>& AllRumorNodes() {
    static const std::vector<RumorNode> n = {
        // who they are
        {"phone_mom", 0, 0}, {"phone_answer", 1, 0}, {"reports", 2, 0}, {"dad_radio", 3, 0},
        {"calendar", 0, 1}, {"no_baby", 1, 1}, {"handlers", 2, 1}, {"static_tv", 3, 1},
        {"visitor_time", 0, 2}, {"my_name", 1, 2}, {"the_tape", 2, 2}, {"allies", 3, 2}, {"last_toll", 4, 2},
        {"treatment", 2, 3}, {"juice", 3, 3}, {"book_notes", 4, 3},
        // he was me
        {"out_window", 0, 4}, {"piece_mom", 1, 4}, {"piece_dad", 2, 4}, {"piece_reissue", 3, 4}, {"trunk", 4, 4},
        {"nobody_home", 0, 5}, {"piece_door", 1, 5}, {"he_remembers", 2, 5}, {"piece_car", 3, 5}, {"his_car", 4, 5},
        {"fortress", 0, 6}, {"tied_up", 1, 6}, {"collector", 2, 6}, {"echo", 3, 6}, {"hand_me_down", 4, 6},
        {"his_knot", 0, 7}, {"traps", 1, 7}, {"thirty", 2, 7}, {"new_hinges", 3, 7}, {"no_car", 4, 7},
        // nowhere to go
        {"run_fail", 1, 8}, {"leash", 2, 8}, {"grave_05", 3, 8}, {"small_prints", 4, 8},
        {"told_dad", 3, 9}, {"they_listen", 4, 9},
        // downstairs
        {"doggo_basement", 0, 10}, {"basement_open", 1, 10}, {"basement_screens", 2, 10}, {"basement_files", 3, 10}, {"program_hq", 4, 10},
        {"dad_key", 0, 11}, {"baby_watch", 2, 11}, {"protocol", 3, 11}, {"graduation", 4, 11},
        {"taser", 0, 12}, {"dad_fought", 1, 12}, {"tased_him", 2, 12}, {"mom_fought", 3, 12}, {"dead_switch", 4, 12},
        // Doggo, and who looks for you
        {"doggo_out", 0, 13}, {"doggo_door", 1, 13}, {"doggo_stay", 2, 13}, {"dad_ran", 3, 13},
        {"doggo_gate", 1, 14}, {"they_hunt", 3, 14}, {"handlers_hunt", 4, 14},
        // v0.20 merge: the watch, the program, and the story and knife lanes' memories ("no_way_out" stays off the map)
        {"w_beep", 0, 15}, {"w_screen", 1, 15}, {"w_drift", 2, 15}, {"w_grow", 3, 15}, {"w_wrist", 4, 15},
        {"confirmed", 0, 16}, {"finse_gate", 1, 16}, {"unit_04", 2, 16}, {"same_number", 3, 16}, {"packing_slip", 4, 16},
        {"just_like_us", 0, 17}, {"small_hands", 1, 17}, {"brake_line", 2, 17}, {"kid_door", 3, 17}, {"two_remember", 4, 17},
        {"three_plates", 0, 18}, {"his_dog", 1, 18}, {"under_well", 2, 18},
    };
    return n;
}

inline const std::vector<RumorLink>& AllRumorLinks() {
    static const std::vector<RumorLink> l = {
        {"phone_mom", "phone_answer"}, {"phone_mom", "calendar"}, {"calendar", "visitor_time"},
        {"phone_answer", "handlers"}, {"reports", "handlers"}, {"dad_radio", "handlers"},
        {"no_baby", "handlers"}, {"static_tv", "handlers"}, {"the_tape", "handlers"}, {"allies", "handlers"},
        {"my_name", "the_tape"}, {"allies", "last_toll"},
        {"the_tape", "treatment"}, {"treatment", "juice"}, {"juice", "book_notes"},
        {"my_name", "piece_mom"},
        {"piece_mom", "he_remembers"}, {"piece_dad", "he_remembers"}, {"piece_reissue", "he_remembers"},
        {"piece_door", "he_remembers"}, {"piece_car", "he_remembers"},
        {"he_remembers", "tied_up"}, {"he_remembers", "collector"},
        {"out_window", "nobody_home"}, {"nobody_home", "piece_door"}, {"nobody_home", "fortress"},
        {"piece_car", "his_car"}, {"his_car", "trunk"}, {"his_car", "hand_me_down"}, {"hand_me_down", "no_car"},
        {"collector", "echo"}, {"collector", "thirty"}, {"collector", "new_hinges"},
        {"tied_up", "traps"}, {"tied_up", "his_knot"},
        {"thirty", "leash"}, {"leash", "run_fail"}, {"thirty", "grave_05"}, {"grave_05", "small_prints"},
        {"told_dad", "they_listen"}, {"they_listen", "program_hq"},
        {"basement_open", "doggo_basement"}, {"basement_open", "basement_screens"}, {"basement_screens", "basement_files"},
        {"basement_files", "program_hq"}, {"dad_key", "basement_open"}, {"basement_files", "protocol"},
        {"protocol", "baby_watch"}, {"protocol", "graduation"}, {"graduation", "dead_switch"}, {"dead_switch", "mom_fought"},
        {"taser", "dad_fought"}, {"dad_fought", "tased_him"},
        {"doggo_out", "doggo_door"}, {"doggo_door", "doggo_stay"}, {"doggo_stay", "dad_ran"}, {"doggo_door", "doggo_gate"},
        {"they_hunt", "handlers_hunt"},
        // v0.20 merge
        {"w_beep", "w_screen"}, {"w_screen", "w_drift"}, {"w_drift", "w_grow"}, {"w_grow", "w_wrist"}, {"w_drift", "finse_gate"},
        {"confirmed", "finse_gate"}, {"finse_gate", "unit_04"}, {"unit_04", "same_number"}, {"same_number", "packing_slip"},
        {"just_like_us", "small_hands"}, {"just_like_us", "three_plates"}, {"three_plates", "his_dog"},
    };
    return l;
}

} // namespace sixhundred
