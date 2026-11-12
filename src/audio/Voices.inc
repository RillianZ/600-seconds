// 600 Seconds - src/audio/Voices.inc (v0.20.1 sound-look)
// Included by main.cpp after art/LookUI.inc. The synthesis is in audio/VoiceSynth.h (plain C++, checked offline).
//   Thunder: after every lightning flash (art/Look.inc), 0.5 to 2 s later. The shorter the wait, the closer it was:
//            close ones crack first and are louder, far ones are a low swell. From inside the house it's muffled.
//            With "Calm flashes" on it's still there, but softer and without the crack.
//   Speech blips: soft short syllables while a line types out in the dialogue box, and a few when a speech bubble pops up.
//            A pitch and a colour per speaker. Doggo gets one small huff, the narrator and Black text get nothing.
//            Through a wall or a closed door they are quieter and dull, the same way the humming is (VoiceThrough).
//            Pause menu: "Voice blips" on or off. They follow the Sound volume.

namespace vs = voicesynth;
static Sound sBlip[vs::SP_COUNT][vs::kVariants][vs::kMuffles], sHuff[vs::kMuffles], sThunder[3][2];
static bool v201Ready = false;

static Sound SoundFromBuf(const std::vector<float>& v) {
    Wave w{};
    w.frameCount = (unsigned)v.size(); w.sampleRate = vs::SR; w.sampleSize = 16; w.channels = 1;
    short* d = (short*)MemAlloc((unsigned)(v.size() * sizeof(short)));
    for (size_t i = 0; i < v.size(); ++i) d[i] = (short)(std::max(-1.0f, std::min(1.0f, v[i])) * 28000);
    w.data = d;
    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    return s;
}
static void InitSoundsV201() {
    if (!audioOk) return;
    for (int sp = 0; sp < vs::SP_COUNT; ++sp)
        for (int k = 0; k < vs::kVariants; ++k)
            for (int m = 0; m < vs::kMuffles; ++m) sBlip[sp][k][m] = SoundFromBuf(vs::RenderBlip(sp, k, m));
    for (int m = 0; m < vs::kMuffles; ++m) sHuff[m] = SoundFromBuf(vs::RenderHuff(m));
    for (int d = 0; d < 3; ++d) for (int m = 0; m < 2; ++m) sThunder[d][m] = SoundFromBuf(vs::RenderThunder(d, m != 0));
    v201Ready = true;
}

// the screenshot runner prints these with the other sound ids. Blips in a row from one speaker are counted, not listed
static void V201Log(const std::string& key, const std::string& tail) {
#if defined(SHOTS)
    if (!sfxLog.empty() && sfxLog.back().rfind(key + tail + " x", 0) == 0) {
        int n = std::atoi(sfxLog.back().c_str() + key.size() + tail.size() + 2);
        sfxLog.back() = key + tail + " x" + std::to_string(n + 1);
    } else sfxLog.push_back(key + tail + " x1");
#else
    (void)key; (void)tail;
#endif
}
static std::string V201Tail(float vol, float muffle) {
    std::string t;
    if (muffle > 0.7f) t += "(muffled)"; else if (muffle > 0.15f) t += "(dull)";
    if (vol < 0.99f) t += "(vol " + std::to_string((int)(vol * 100 + 0.5f)) + "%)";
    return t;
}
static int MuffleIdx(float muffle) { return muffle < 0.15f ? 0 : muffle < 0.7f ? 1 : 2; }

static void PlayBlip(int sp, float vol, float muffle) {
    V201Log("blip:" + std::string(sp == vs::SP_KID ? "kid" : sp == vs::SP_MOM ? "mom" : sp == vs::SP_DAD ? "dad" : sp == vs::SP_VISITOR ? "visitor" : sp == vs::SP_RADIO ? "radio" : "girl"), V201Tail(vol, muffle));
    if (!audioOk || !v201Ready || vol <= 0.01f) return;
    static int last[vs::SP_COUNT] = {};
    int k = GetRandomValue(0, vs::kVariants - 2);
    if (k >= last[sp]) ++k;   // never the same syllable twice in a row
    last[sp] = k;
    Sound& s = sBlip[sp][k][MuffleIdx(muffle)];
    SetSoundVolume(s, settings.sfx / 10.0f * vs::Params(sp).vol * vol);
    SetSoundPitch(s, 0.97f + GetRandomValue(0, 6) / 100.0f);
    PlaySound(s);
}
static void PlayHuff(float vol, float muffle) {
    V201Log("huff:doggo", V201Tail(vol, muffle));
    if (!audioOk || !v201Ready || vol <= 0.01f) return;
    Sound& s = sHuff[MuffleIdx(muffle)];
    SetSoundVolume(s, settings.sfx / 10.0f * 0.3f * vol);
    PlaySound(s);
}

// how a speaker reaches the kid. Talking to you in the box: VoiceThrough only (they are usually right there).
// A bubble: also quieter with distance, and nothing at all from far away or from somebody you can't see
static float V201VoiceGain(const std::string& who, bool bubble, float& muffle) {
    muffle = 0;
    Actor* a = ActorFor(who);
    if (!a) return bubble ? 0.0f : 1.0f;            // the radio, the console, the girl: no body in the house
    if (a == &player) return 1.0f;
    if (!a->visible) return bubble ? 0.0f : 1.0f;
    Room kr = story.world.RoomAt(TX(player.pos.x), TX(player.pos.y));
    float g = VoiceThrough(a->pos, kr, muffle);
    if (bubble) {
        float d = std::hypot(a->pos.x - player.pos.x, a->pos.y - player.pos.y) / TS;
        const float range = 14;
        if (d > range) return 0;
        g *= 0.35f + 0.65f * (1 - d / range);
    }
    return g;
}

// ---------------------------------------------------------------- bubbles
struct PendingBlip { float in; int sp; float vol, muffle; std::string who; };
static std::vector<PendingBlip> pendingBlips;
static void VoiceBubble(const std::string& who, const std::string& text) {
    for (size_t i = 0; i < pendingBlips.size();) { if (pendingBlips[i].who == who) pendingBlips.erase(pendingBlips.begin() + i); else ++i; }   // a new bubble replaces the old one
    if (!settings.blips || screen != Screen::Game) return;
    float muffle;
    float g = V201VoiceGain(who, true, muffle);
    if (g <= 0.01f) return;
    int spoken = vs::SpokenCount(text);
    if (who == "Doggo") { if (spoken > 0 && !IsBark(text)) PlayHuff(g, muffle); return; }
    int sp = vs::SpeakerFor(who);
    if (sp < 0 || spoken == 0 || vs::OnlyHumming(text)) return;
    int n = std::max(2, std::min(7, spoken / 4));
    float gap = vs::Params(sp).gap * 1.1f;
    for (int k = 0; k < n; ++k) pendingBlips.push_back({ k * gap, sp, g, muffle, who });
}

// ---------------------------------------------------------------- the dialogue box
static std::string blipKey;
static std::vector<bool> blipMask;
static size_t blipPrev = 0;
static float blipT = 0;
static void V201LineBlips(float dt) {
    bool on = settings.blips && screen == Screen::Game && beatActive && cur.type == BeatType::Say && !pauseOpen && !journalOpen && !recapOpen && !miniOpen;
    if (!on) { blipKey.clear(); return; }
    size_t total = cur.text.size();
    size_t now = typed >= 1e8f ? total : std::min(total, (size_t)std::max(0.0f, typed));
    std::string key = cur.speaker + "|" + cur.text;
    if (key != blipKey || now < blipPrev) {
        // a new line
        blipKey = key; blipMask = vs::SpokenMask(cur.text); blipPrev = 0; blipT = 0;
        if (cur.speaker == "Doggo" && !IsBark(cur.text) && vs::SpokenCount(cur.text) > 0 && typed < 1e8f) { float m; float g = V201VoiceGain("Doggo", false, m); PlayHuff(g, m); }
    }
    blipT -= dt;
    if (now > blipPrev) {
        bool skipped = typed >= 1e8f && blipPrev > 0;   // E pressed: the rest of the line at once, no blip for that
        bool spoken = false;
        for (size_t i = blipPrev; i < now && i < blipMask.size(); ++i) if (blipMask[i]) { spoken = true; break; }
        int sp = vs::SpeakerFor(cur.speaker);
        if (spoken && !skipped && sp >= 0 && blipT <= 0) {
            float m; float g = V201VoiceGain(cur.speaker, false, m);
            PlayBlip(sp, g, m);
            blipT = vs::Params(sp).gap;
        }
        blipPrev = now;
    }
}

// ---------------------------------------------------------------- thunder
struct PendingThunder { float in; int dist; float delay; };
static std::vector<PendingThunder> pendingThunder;
static float thunderPrevBolt = 99;
static void V201Thunder(float dt, bool paused) {
    // a new flash (art/Look.inc restarts lkBoltT at 0)
    if (lkBoltT < thunderPrevBolt && !paused && !viewBelow && screen == Screen::Game) {
        float delay = vs::ThunderDelay(Hash(lkBoltN + (int)(globalT * 10), 6067));
        pendingThunder.push_back({ delay, vs::ThunderDist(delay), delay });
    }
    thunderPrevBolt = lkBoltT;
    if (paused) return;
    for (size_t i = 0; i < pendingThunder.size();) {
        PendingThunder& p = pendingThunder[i];
        p.in -= dt;
        if (p.in > 0) { ++i; continue; }
        int dist = p.dist;
        float delay = p.delay;
        pendingThunder.erase(pendingThunder.begin() + i);
        bool card = beatActive && (cur.type == BeatType::Ending || cur.type == BeatType::Black || cur.type == BeatType::Card);
        if (card || hushMusic || screen != Screen::Game) continue;   // ending cards, black screens and the quiet scenes stay quiet
        if (settings.calmFlashes && dist == 0) dist = 1;              // calm: no crack
        Room r = story.world.RoomAt(TX(player.pos.x), TX(player.pos.y));
        bool out = OutdoorsAt(player.pos), below = story.inBasement || r == Room::Basement;
        float vol = (settings.calmFlashes ? 0.5f : 1.0f) * (out ? 1.0f : below ? 0.3f : 0.7f);
#if defined(SHOTS)
        char tag[48]; std::snprintf(tag, sizeof tag, "+%.1fs", delay);
        sfxLog.push_back(std::string(dist == 0 ? "thunder_close" : dist == 1 ? "thunder_mid" : "thunder_far") + tag + (out ? "" : "(muffled)") + (vol < 0.99f ? "(vol " + std::to_string((int)(vol * 100 + 0.5f)) + "%)" : ""));
#else
        (void)delay;
#endif
        if (!audioOk || !v201Ready) continue;
        Sound& s = sThunder[dist][out ? 0 : 1];
        SetSoundVolume(s, settings.sfx / 10.0f * vs::ThunderVol(dist) * vol);
        SetSoundPitch(s, 0.94f + GetRandomValue(0, 10) / 100.0f);
        PlaySound(s);
    }
}

// once a frame, after LookTick
static void VoicesTick(float dt) {
    bool paused = pauseOpen || journalOpen || miniOpen || recapOpen || screen != Screen::Game;
    V201Thunder(dt, paused);
    if (paused) return;
    V201LineBlips(dt);
    for (size_t i = 0; i < pendingBlips.size();) {
        PendingBlip& p = pendingBlips[i];
        p.in -= dt;
        if (p.in <= 0) { if (settings.blips) PlayBlip(p.sp, p.vol, p.muffle); pendingBlips.erase(pendingBlips.begin() + i); }
        else ++i;
    }
}
