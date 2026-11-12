// 600 Seconds - src/audio/SfxV20.inc (v0.20 sound, T-034 and T-035)
// Included by main.cpp right before InitSounds. New sound effects, all made in code like the old ones,
// the tape hiss switch, a knock that waits for the power to die first, and sounds placed in the house.
//   power_cut      the hum of the house winds down, a relay clunks
//   lock           a key or a bolt: click
//   phone_pickup   the receiver comes off the cradle
//   phone_hangup   and goes back (*click*)
//   vhs_on/off     a tape goes in and the hiss starts, under her humming / the hiss stops
//   trunk          his trunk lid comes down
//   trunk_open     and opens
//   kick           a boot against a locked door (he kicks locked doors and furniture barricades)
//   door_break     the door gives
//   scrape         the furniture behind the door shifts a little

static Sound sPowerCut, sLock, sPhoneUp, sPhoneDown, sVhsIn, sTrunk, sTrunkOpen, sKick, sDoorBreak, sScrape;
static volatile float aVhsT = 0;   // the tape hiss, read by the music callback
static double powerCutAt = -10;
struct LaterSfx { double at; std::string id; float vol, pitch; };
static std::vector<LaterSfx> laterSfx;

// a resonant band-pass on noise, for scrapes and creaks (state lives in the caller)
struct Bp { float lo = 0, bp = 0; float Run(float x, float f, float q) { float k = 2 * std::sin((float)PI * f / SFX_SR); lo += k * bp; float hi = x - lo - q * bp; bp += k * hi; return bp; } };

static void InitSoundsV20() {
    sPowerCut = MakeSound(SFX_SR * 1.3f, [](float t, int i) {
        // the house hum (50 Hz and up) sags in pitch and dies over most of a second. A relay clunks at the start
        float fall = std::max(0.0f, 1 - t / 0.9f);
        float pitch = 0.55f + 0.45f * fall;
        static float ph = 0;
        if (i == 0) ph = 0;
        ph += 2 * (float)PI * 50 * pitch / SFX_SR;
        float hum = (std::sin(ph) * 0.5f + std::sin(2 * ph) * 0.35f + std::sin(3 * ph) * 0.2f) * fall * fall * 0.55f;
        float clunk = (std::sin(t * 2 * (float)PI * 70) * 0.8f + Noise(i) * 0.4f) * std::exp(-t * 30);
        float tick = t > 0.05f && t < 0.06f ? Noise(i * 7) * 0.5f : 0;
        return hum + clunk + tick;
    });
    sLock = MakeSound(SFX_SR * 0.2f, [](float t, int i) {
        // two small metal clicks and a little body
        float v = 0;
        for (int k = 0; k < 2; ++k) {
            float s = t - k * 0.045f;
            if (s > 0) v += (std::sin(s * 2 * (float)PI * (3100 - k * 900)) * 0.5f + Noise(i * 3 + k) * 0.5f) * std::exp(-s * (k ? 160.0f : 220.0f)) * (k ? 0.8f : 0.6f);
        }
        return v + std::sin(t * 2 * (float)PI * 180) * 0.25f * std::exp(-t * 40);
    });
    sPhoneUp = MakeSound(SFX_SR * 0.45f, [](float t, int i) {
        // plastic knocks against plastic, then the cradle switch lets go
        float v = (std::sin(t * 2 * (float)PI * 240) * 0.5f + Noise(i) * 0.35f) * std::exp(-t * 45);
        float s = t - 0.06f;
        if (s > 0) v += Noise(i * 5) * 0.25f * std::exp(-s * 60);
        s = t - 0.16f;
        if (s > 0) v += (std::sin(s * 2 * (float)PI * 2400) * 0.35f + Noise(i * 3) * 0.3f) * std::exp(-s * 180);
        return v;
    });
    sPhoneDown = MakeSound(SFX_SR * 0.3f, [](float t, int i) {
        // the switch clicks, the receiver lands
        float v = (std::sin(t * 2 * (float)PI * 2200) * 0.35f + Noise(i * 3) * 0.3f) * std::exp(-t * 200);
        float s = t - 0.03f;
        if (s > 0) v += (std::sin(s * 2 * (float)PI * 200) * 0.7f + Noise(i) * 0.3f) * std::exp(-s * 35);
        return v;
    });
    sVhsIn = MakeSound(SFX_SR * 1.1f, [](float t, int i) {
        // the tape goes in: a clunk, the mechanism, the motor finding its speed
        float v = (std::sin(t * 2 * (float)PI * 95) * 0.6f + Noise(i) * 0.3f) * std::exp(-t * 25);
        float s = t - 0.25f;
        if (s > 0) v += (std::sin(s * 2 * (float)PI * 140) * 0.4f + Noise(i * 3) * 0.4f) * std::exp(-s * 40);
        s = t - 0.4f;
        if (s > 0) {
            float spin = std::min(1.0f, s / 0.3f);
            v += (std::sin(s * 2 * (float)PI * (60 + 60 * spin)) * 0.12f + Noise(i / 3) * 0.06f) * std::min(1.0f, (0.7f - s) / 0.2f);
        }
        return v;
    });
    sTrunk = MakeSound(SFX_SR * 1.0f, [](float t, int i) {
        // a car trunk slammed: a deep boom, sheet metal ringing, the latch
        float v = (std::sin(t * 2 * (float)PI * 62) * 0.9f + Noise(i) * 0.5f * std::exp(-t * 25)) * std::exp(-t * 7);
        static const float part[3] = { 410, 1030, 1720 };
        for (int k = 0; k < 3; ++k) v += std::sin(t * 2 * (float)PI * part[k]) * 0.07f * std::exp(-t * (9 + k * 5));
        float s = t - 0.02f;
        if (s > 0) v += Noise(i * 7) * 0.4f * std::exp(-s * 250);
        return v;
    });
    sTrunkOpen = MakeSound(SFX_SR * 0.8f, [](float t, int i) {
        // the latch, then the lid on its springs
        float v = Noise(i * 7) * 0.4f * std::exp(-t * 220) + std::sin(t * 2 * (float)PI * 1800) * 0.2f * std::exp(-t * 120);
        float s = t - 0.08f;
        if (s > 0 && s < 0.5f) v += std::sin(s * 2 * (float)PI * (300 + 120 * s)) * 0.07f * std::sin(s * (float)PI / 0.5f);
        s = t - 0.6f;
        if (s > 0) v += std::sin(s * 2 * (float)PI * 90) * 0.4f * std::exp(-s * 30);
        return v;
    });
    sKick = MakeSound(SFX_SR * 0.75f, [](float t, int i) {
        // a boot against a door: the boom of the panel, the frame rattling after
        float v = (std::sin(t * 2 * (float)PI * 72) * 0.9f + std::sin(t * 2 * (float)PI * 150) * 0.4f) * std::exp(-t * 11);
        v += Noise(i) * 0.6f * std::exp(-t * 45);
        float rat = std::max(0.0f, std::sin(t * 2 * (float)PI * 24));
        v += Noise(i * 3) * 0.18f * rat * rat * std::exp(-t * 9) * (t > 0.04f ? 1 : 0);
        return v;
    });
    sDoorBreak = MakeSound(SFX_SR * 1.3f, [](float t, int i) {
        // the crack, splinters, the door hitting the wall, bits on the floor
        float v = (Noise(i * 3) * 0.9f + std::sin(t * 2 * (float)PI * 120) * 0.5f) * std::exp(-t * 18);
        for (int k = 0; k < 7; ++k) {
            float s = t - 0.05f - k * 0.07f - (k * k % 5) * 0.013f;
            if (s > 0) v += Noise(i * (11 + k)) * 0.3f * std::exp(-s * 70) * (1 - k / 8.0f);
        }
        float s = t - 0.3f;
        if (s > 0) v += (std::sin(s * 2 * (float)PI * 85) * 0.7f + Noise(i) * 0.2f) * std::exp(-s * 16);
        return v;
    });
    sScrape = MakeSound(SFX_SR * 0.6f, [](float t, int i) {
        // heavy wood dragged a few centimetres over the floorboards
        static Bp bp;
        if (i == 0) bp = Bp{};
        float judder = 0.6f + 0.4f * std::sin(t * 2 * (float)PI * 31);
        float x = bp.Run(Noise(i * 2 + 1), 260 + 80 * std::sin(t * 9), 0.25f);
        return x * 0.5f * judder * std::min(1.0f, t / 0.05f) * std::min(1.0f, (0.6f - t) / 0.2f);
    });
}

// vol and pitch only change the sounds played here. Returns true if the id was one of ours
#if defined(SHOTS)
static std::vector<std::string> sfxLog;   // the screenshot runner prints what would have been heard
#endif
static bool PlayV20(const std::string& id, float vol = 1.0f, float pitch = 1.0f) {
#if defined(SHOTS)
    sfxLog.push_back(id + (pitch < 0.99f ? "(muffled)" : "") + (vol < 0.99f ? "(vol " + std::to_string((int)(vol * 100)) + "%)" : ""));
#endif
    if (id == "vhs_on") { aVhsT = 1; if (audioOk) { SetSoundVolume(sVhsIn, settings.sfx / 10.0f * 0.8f); PlaySound(sVhsIn); } return true; }
    if (id == "vhs_off") { aVhsT = 0; return true; }
    Sound* s = nullptr;
    if (id == "power_cut") { s = &sPowerCut; powerCutAt = GetTime(); }
    else if (id == "lock") s = &sLock;
    else if (id == "phone_pickup") s = &sPhoneUp;
    else if (id == "phone_hangup") s = &sPhoneDown;
    else if (id == "trunk") s = &sTrunk;
    else if (id == "trunk_open") s = &sTrunkOpen;
    else if (id == "kick") s = &sKick;
    else if (id == "door_break") s = &sDoorBreak;
    else if (id == "scrape") s = &sScrape;
    else if (id == "knock" && GetTime() - powerCutAt < 0.3) {
        // the house goes dark first, then he knocks
        laterSfx.push_back({ powerCutAt + 1.0, "knock", vol, pitch });
        return true;
    }
    if (!s) return false;
    if (!audioOk) return true;
    SetSoundVolume(*s, settings.sfx / 10.0f * vol);
    SetSoundPitch(*s, pitch);
    PlaySound(*s);
    return true;
}

static void Play(const std::string& id);
static bool LineOfSight(Vector2 a, Vector2 b);

// sounds that wait (the knock after the power cut). Called once a frame
static void PlayV20Later() {
    double now = GetTime();
    for (size_t k = 0; k < laterSfx.size();) {
        if (now >= laterSfx[k].at) { std::string id = laterSfx[k].id; laterSfx.erase(laterSfx.begin() + k); Play(id); }
        else ++k;
    }
}

// T-035: a sound somewhere in the house. Quieter with distance, and duller (played lower) through a wall or a closed door
static void PlayAt(const std::string& id, Vector2 at, float range = TS * 30.0f) {
    float d = std::hypot(at.x - player.pos.x, at.y - player.pos.y);
    if (d > range) return;
    float k = 1 - d / range;
    bool los = LineOfSight(at, player.pos);
    float vol = (0.3f + 0.7f * k * k) * (los ? 1.0f : 0.6f);
    if (!PlayV20(id, vol, los ? 1.0f : 0.8f)) Play(id);
}
