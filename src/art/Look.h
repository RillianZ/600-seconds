// 600 Seconds - art/Look.h (v0.20 look)
// Forward declarations, so Sprites.inc and WorldDraw.inc can ask the v0.20 art (art/Look.inc, art/LookUI.inc)
// without moving any of the old code around.
#pragma once
struct Look;
struct Actor;

// art/Look.inc: the world
static float RoomPower(sixhundred::Room r);                  // 1 = lights on in that room, 0 = dark. The 16:50 sequence goes room by room
static float RoomPowerAt(int tx, int ty);
static bool LkObjDark(const sixhundred::Obj& o);             // the power is out where this object stands
static bool LkAnyPower();                        // some light is still on somewhere (the sequence is running)
static void LookTick(float dt);                  // per frame: power sequence, lightning, headlights, idle timer
static void LookLightmap(float inH);             // inside the light map pass (additive): headlight sweep
static void LookGlows(float inH);                // in the glow pass (additive, after the light): the headlight window on the walls
static void LookWallProps();                     // before people: the darkness details and the stage 1 flux props
static bool LookDrawKid(const Actor& a, const Look& l);   // the kid, standing still a while. False = draw him the normal way
static void LookKidShadow(const Actor& a);       // under the kid, once he's very dark
static void LookStormFx(Camera2D cam);           // after the light: lightning and his silhouette
static float LookMugSortY();                     // Mom's mug sorts with the furniture
static void LookMug();
// art/LookUI.inc: the screen
static bool DrawBust(const std::string& who, float cx, float cy, float px);   // dialogue portraits with expressions
static void LookWatchFace(float bx, float by, float w, float bh, float lx, float ly, float lw, float lh);
static Color LookLcd();
static void LookOnInteract(const std::string& id);   // the presentation used something, or picked a choice: maybe a close-up
