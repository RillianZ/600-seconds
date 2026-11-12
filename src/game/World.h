// 600 Seconds - World (map, rooms, objects)
// Engine-free: no raylib in here. Ports straight to Unreal later.
#pragma once
#include <string>
#include <vector>

namespace sixhundred {

enum class Tile : unsigned char {
    Vidda,      // open plateau outside the fence. Walking here = running away
    Grass,
    Gravel,
    Floor,
    Wall,
    Door,
    Fence,
    BasementDoor, // solid unless open (16:30-16:32)
    ShedFloor,
    Void,         // underground filler around the basement
    // v0.15: the backyard
    Bush,         // a line of snowy bushes. Slow to push through, his torch shines straight through
    Chain,        // the old chain-link fence. Blocks you, not his light
    ChainGap,     // a torn bit of the chain fence. A kid squeezes through, slowly. He doesn't fit
    Ice,          // the frozen stream
};

enum class Room : unsigned char {
    Outside, KidsRoom, Bathroom, ParentsRoom, Hallway, Kitchen, LivingRoom, Shed, Vidda, Basement,
    Backyard,     // v0.15: behind the house (north). The map grows up into negative rows, so nothing else moved
};
inline bool IsYard(Room r) { return r == Room::Outside || r == Room::Backyard; }

inline const char* RoomName(Room r) {
    switch (r) {
        case Room::KidsRoom: return "Your room";
        case Room::Bathroom: return "Bathroom";
        case Room::ParentsRoom: return "Mom and Dad's room";
        case Room::Hallway: return "Hallway";
        case Room::Kitchen: return "Kitchen";
        case Room::LivingRoom: return "Living room";
        case Room::Shed: return "Shed";
        case Room::Vidda: return "Hardangervidda";
        case Room::Basement: return "The basement";
        case Room::Backyard: return "Backyard";
        default: return "Yard";
    }
}

struct Obj {
    std::string id;
    std::string name;
    int x, y;          // tile
    bool solid = true;
};

// v0.15: a grid whose rows run from World::TOP (negative, the backyard) to World::H - 1.
// grid[y][x] works for every row, so the house, the yard and the basement kept their old numbers
template <class T, int ROWS, int COLS, int FIRST>
struct Rows {
    T d[ROWS][COLS];
    T* operator[](int y) { return d[y - FIRST]; }
    const T* operator[](int y) const { return d[y - FIRST]; }
};

// v0.15: every window in the house is a way through, both ways. The window itself is a flat object
// on the floor tile inside (id = the object), the wall is at (wx, wy), outside is one tile further out
struct WindowInfo { const char* id; int wx, wy, inX, inY, outX, outY; };
inline const std::vector<WindowInfo>& AllWindows() {
    static const std::vector<WindowInfo> w = {
        {"window", 6, 2, 6, 3, 6, 1},            // your room. It sticks in the cold
        {"win_bath", 13, 2, 13, 3, 13, 1},
        {"win_parents", 20, 2, 20, 3, 20, 1},
        {"win_kitchen", 8, 20, 8, 19, 8, 21},
        {"win_living", 15, 20, 15, 19, 15, 21},
        {"win_living2", 25, 20, 25, 19, 25, 21}, // the one he looks in through, and climbs in through
    };
    return w;
}
inline const WindowInfo* WindowById(const std::string& id) { for (auto& w : AllWindows()) if (id == w.id) return &w; return nullptr; }

struct World {
    static constexpr int W = 40;
    static constexpr int H = 46;
    static constexpr int TOP = -26;          // v0.15: the first row. Row TOP is open vidda, TOP + 1 the back fence
    static constexpr int ROWS = H - TOP;
    Rows<Tile, ROWS, W, TOP> tiles;
    Rows<Room, ROWS, W, TOP> rooms;
    std::vector<Obj> objs;

    // Key spots
    static constexpr int BASEMENT_X = 29, BASEMENT_Y = 10;

    World() { Build(); }

    void Fill(int x0, int y0, int x1, int y1, Tile t) {
        for (int y = y0; y <= y1; ++y) for (int x = x0; x <= x1; ++x) tiles[y][x] = t;
    }
    void SetRoom(int x0, int y0, int x1, int y1, Room r) {
        for (int y = y0; y <= y1; ++y) for (int x = x0; x <= x1; ++x) rooms[y][x] = r;
    }
    void Outline(int x0, int y0, int x1, int y1, Tile t) {
        for (int x = x0; x <= x1; ++x) { tiles[y0][x] = t; tiles[y1][x] = t; }
        for (int y = y0; y <= y1; ++y) { tiles[y][x0] = t; tiles[y][x1] = t; }
    }

    void Build() {
        Fill(0, TOP, W - 1, H - 1, Tile::Vidda);
        SetRoom(0, TOP, W - 1, H - 1, Room::Vidda);
        // Yard inside the fence. v0.15: it goes on behind the house now, up to row TOP + 1
        Fill(1, TOP + 1, W - 2, 30, Tile::Grass);
        SetRoom(1, TOP + 1, W - 2, 30, Room::Outside);
        SetRoom(1, TOP + 1, W - 2, 1, Room::Backyard);
        Outline(1, TOP + 1, W - 2, 30, Tile::Fence);
        BuildBackyard();
        // Gate + gravel road out to the vidda
        Fill(19, 21, 21, 33, Tile::Gravel);
        tiles[30][19] = tiles[30][20] = tiles[30][21] = Tile::Gravel; // open gate

        // Underground: the basement lives down here, only reachable by the stairs
        Fill(0, 34, W - 1, H - 1, Tile::Void);
        SetRoom(0, 34, W - 1, H - 1, Room::Basement);
        Fill(4, 36, 20, 44, Tile::ShedFloor);
        Outline(4, 36, 20, 44, Tile::Wall);

        // House shell
        Fill(2, 2, 29, 20, Tile::Floor);
        Outline(2, 2, 29, 20, Tile::Wall);
        // Inner walls
        for (int x = 2; x <= 29; ++x) { tiles[9][x] = Tile::Wall; tiles[12][x] = Tile::Wall; }
        for (int y = 2; y <= 9; ++y) { tiles[y][10] = Tile::Wall; tiles[y][16] = Tile::Wall; }
        for (int y = 12; y <= 20; ++y) tiles[y][13] = Tile::Wall;
        // Doors
        tiles[9][6] = Tile::Door;    // kid's room
        tiles[9][13] = Tile::Door;   // bathroom
        tiles[9][22] = Tile::Door;   // parents
        tiles[12][7] = Tile::Door;   // kitchen
        tiles[12][20] = Tile::Door;  // living room
        tiles[16][13] = Tile::Door;  // kitchen <-> living
        tiles[20][20] = Tile::Door;  // front door
        tiles[BASEMENT_Y][BASEMENT_X] = Tile::BasementDoor;

        SetRoom(3, 3, 9, 8, Room::KidsRoom);
        SetRoom(11, 3, 15, 8, Room::Bathroom);
        SetRoom(17, 3, 28, 8, Room::ParentsRoom);
        SetRoom(3, 10, 28, 11, Room::Hallway);
        SetRoom(3, 13, 12, 19, Room::Kitchen);
        SetRoom(14, 13, 28, 19, Room::LivingRoom);
        // Doors belong to the room "below" them for simplicity
        rooms[9][6] = Room::Hallway; rooms[9][13] = Room::Hallway; rooms[9][22] = Room::Hallway;
        rooms[12][7] = Room::Hallway; rooms[12][20] = Room::Hallway;

        // Shed
        Fill(31, 4, 37, 10, Tile::ShedFloor);
        Outline(31, 4, 37, 10, Tile::Wall);
        tiles[10][34] = Tile::Door;
        SetRoom(32, 5, 36, 9, Room::Shed);

        // Objects (id, name, x, y)
        objs = {
            {"bed", "Your bed", 4, 4},
            {"toys", "Mess on the floor", 8, 6, false},
            {"window", "Your window", 6, 3, false},   // v0.15: you can stand under it and climb through
            {"toilet", "Toilet", 12, 4},
            {"mirror", "Mirror", 14, 3},
            {"desk", "Mom's desk", 18, 4},
            {"pbed", "Mom and Dad's bed", 23, 4},
            {"drawer", "Dad's cabinet", 27, 4},
            {"coats", "Coat rack", 4, 10},
            {"basement", "Basement door", BASEMENT_X, BASEMENT_Y},
            {"phone", "Landline", 3, 14},
            {"fridge", "Fridge", 9, 13},
            {"stove", "Stove", 11, 13},
            {"trashcan", "Kitchen trash", 11, 19},
            {"bowl", "Doggo's bowl", 5, 19},
            {"tv", "TV", 21, 13},
            {"couch", "Couch", 21, 16},
            {"radio", "Radio", 26, 13},
            {"tree", "Christmas tree", 26, 15},
            {"woodstove", "Wood stove", 27, 18},
            {"bins", "Trash bins", 5, 23},
            {"doghouse", "Doggo's house", 25, 24},
            {"gate", "Gate", 22, 30},
            {"flag", "Flagpole", 10, 26},
            {"generator", "Generator", 33, 5},
            {"bench", "Dad's workbench", 36, 6},
            // v0.2
            {"wardrobe", "Wardrobe", 9, 4},
            {"pantry", "Pantry", 4, 13},
            {"sink", "Sink", 8, 13},
            {"fliptendo", "Fliptendo Fold 2", 8, 8, false},
            {"marbles", "Marble jar", 3, 7},
            {"calendar", "Calendar", 6, 13},
            {"knives", "Knife block", 12, 14},
            {"bookshelf", "Bookshelf", 16, 13},
            {"album", "Photo album", 17, 18},
            {"frontdoor", "Front door", 20, 20, false},
            {"sheddoor", "Shed door", 34, 10, false},
            {"rope", "Rope", 32, 9},
            {"toolbox", "Toolbox", 36, 9},
            {"woodpile", "Woodpile", 29, 24},
            // basement
            {"stairs", "Stairs up", 5, 37, false},
            {"screens", "Wall of screens", 9, 37},
            {"files", "Filing cabinet", 14, 37},
            {"console", "Radio console", 18, 38},
            {"chair", "Chair with straps", 11, 41},
            {"medfridge", "Medicine fridge", 18, 43},
            {"vcr", "Old TV", 7, 41},
            {"crates", "Storage crates", 15, 43},   // v0.13: the one hiding spot down here
            // v0.7: every door can be closed (and locked with the master key)
            {"door_kid", "Your door", 6, 9, false},
            {"door_bath", "Bathroom door", 13, 9, false},
            {"door_parents", "Mom and Dad's door", 22, 9, false},
            {"door_kitchen", "Kitchen door", 7, 12, false},
            {"door_living", "Living room door", 20, 12, false},
            {"door_kl", "Kitchen door", 13, 16, false},
            // v0.15: the other windows you can climb through
            {"win_bath", "Bathroom window", 13, 3, false},
            {"win_parents", "Window", 20, 3, false},
            {"win_kitchen", "Kitchen window", 8, 19, false},
            {"win_living", "Living room window", 15, 19, false},
            {"win_living2", "Living room window", 25, 19, false},
            // v0.15: the backyard. Six things somebody left out here, and a few nods
            {"dish", "Satellite dish", 4, -1},
            {"beast", "Snow beast", 15, -7},           // a ride-on toy, the size of a pony. You can climb inside
            {"oldcar", "Old car", 9, -8},
            {"swing", "Swing set", 21, -7},
            {"smallprints", "Small footprints", 23, -10, false},
            {"spruce", "Big spruce", 4, -17},
            {"grave", "Little cross", 34, -16},
            {"icehole", "Hole in the ice", 26, -21},
            {"lights", "Painted board", 34, 0},
            {"vane", "Weathervane", 32, 3, false},
            {"gap", "Gap in the fence", 29, -24, false},   // v0.19: where the back fence is down. Tracks out, and back in your own steps
            {"chaingap", "Torn fence", 6, -12, false},     // v0.20 knife: laced shut with old twine. Cut it and it's a door your size
        };
    }

    // v0.15: the backyard, rows TOP + 1 to 1. Twice the house, give or take
    void BuildBackyard() {
        // the back fence is down in one place: that's the way out onto the vidda
        tiles[TOP + 1][29] = tiles[TOP + 1][30] = Tile::Grass;
        // bushes, two lines with gaps you have to find
        for (int x = 2; x <= 36; ++x) if (x != 9 && x != 10 && x != 24 && x != 25) tiles[-3][x] = Tile::Bush;
        for (int x = 2; x <= 37; ++x) if (x < 15 || x > 18) tiles[-19][x] = Tile::Bush;
        tiles[-4][31] = tiles[-4][32] = tiles[-18][3] = tiles[-18][27] = tiles[-18][28] = Tile::Bush;
        // the old chain fence across the middle: one gate, one torn gap
        for (int x = 2; x <= 37; ++x) tiles[-12][x] = Tile::Chain;
        tiles[-12][30] = tiles[-12][31] = Tile::Grass;     // the gate stands open
        tiles[-12][6] = Tile::ChainGap;                    // torn. A kid fits. He doesn't
        // the stream, frozen, along the far end. The tiles are a straight band, the art bends it inside them
        for (int x = 2; x <= 37; ++x) for (int y = -23; y <= -21; ++y) tiles[y][x] = Tile::Ice;
    }

    bool InBounds(int x, int y) const { return x >= 0 && y >= TOP && x < W && y < H; }

    const Obj* ObjAt(int x, int y) const {
        for (auto& o : objs) if (o.x == x && o.y == y) return &o;
        return nullptr;
    }

    bool Walkable(int x, int y, bool basementOpen) const {
        if (!InBounds(x, y)) return false;
        Tile t = tiles[y][x];
        if (t == Tile::Wall || t == Tile::Fence || t == Tile::Void || t == Tile::Chain) return false;
        if (t == Tile::BasementDoor && !basementOpen) return false;
        const Obj* o = ObjAt(x, y);
        if (o && o->solid) return false;
        return true;
    }

    Room RoomAt(int x, int y) const {
        if (!InBounds(x, y)) return Room::Vidda;
        return rooms[y][x];
    }
};

} // namespace sixhundred
