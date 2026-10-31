#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

#if defined(_WIN32)
#define CC_API extern "C" __declspec(dllexport)
#else
#define CC_API extern "C" __attribute__((visibility("default")))
#endif

struct CCState {
    int score;
    int bombs;
    float seconds;
    float basketX;
    int gameOver;
    int reason; // 0: playing, 1: three bombs, 2: time up
};

struct CCDrop {
    int id;
    float x;
    float y;
    int bomb;
};

class Game {
public:
    CCState state{};
    std::vector<CCDrop> drops;
    std::mt19937 random;
    float spawnTimer = .7f;
    int nextId = 0;

    explicit Game(unsigned seed) : random(seed) {
        drops.reserve(128);
        restart();
    }

    void restart() {
        state = {0, 0, 60.f, 0.f, 0, 0};
        spawnTimer = .7f;
        nextId = 0;
        drops.clear();
    }

    void finish(int reason) {
        state.gameOver = 1;
        state.reason = reason;
        drops.clear();
    }

    void update(float dt, float direction) {
        if (state.gameOver || !std::isfinite(dt) || dt <= 0) return;
        state.seconds = std::max(0.f, state.seconds - dt);
        if (state.seconds <= 0) { finish(2); return; }
        if (!std::isfinite(direction)) direction = 0;
        state.basketX = std::clamp(state.basketX + std::clamp(direction, -1.f, 1.f) * 500.f * dt,
                                    -500.f, 500.f);
        spawnTimer -= dt;
        while (spawnTimer <= 0) {
            std::uniform_real_distribution<float> x(-500.f, 500.f);
            std::uniform_int_distribution<int> bombChance(0, 3);
            drops.push_back({nextId++, x(random), 290.f, bombChance(random) == 0 ? 1 : 0});
            spawnTimer += .7f;
        }
        for (int i = static_cast<int>(drops.size()) - 1; i >= 0; --i) {
            auto &drop = drops[static_cast<std::size_t>(i)];
            float radius = drop.bomb ? 17.f : 13.f;
            float previousBottom = drop.y - radius;
            drop.y -= 220.f * dt;
            bool reachedBasket = previousBottom >= -287.5f && drop.y - radius <= -287.5f;
            bool overlaps = std::abs(drop.x - state.basketX) <= 55.f + radius;
            bool caught = reachedBasket && overlaps;
            bool missed = drop.y + radius < -400.f;
            if (caught) {
                if (drop.bomb) ++state.bombs;
                else ++state.score;
            }
            if (caught || missed) drops.erase(drops.begin() + i);
            if (state.bombs >= 3) { finish(1); return; }
        }
    }
};

static_assert(sizeof(CCState) == 24, "C# bridge expects a 24-byte state");
static_assert(sizeof(CCDrop) == 16, "C# bridge expects a 16-byte drop");

CC_API void *cc_create() {
    try { return new Game(std::random_device{}()); }
    catch (...) { return nullptr; }
}
CC_API void cc_destroy(void *handle) { delete static_cast<Game *>(handle); }
CC_API void cc_restart(void *handle) { if (handle) static_cast<Game *>(handle)->restart(); }
CC_API void cc_update(void *handle, float dt, float direction) {
    if (handle) static_cast<Game *>(handle)->update(dt, direction);
}
CC_API void cc_get_state(void *handle, CCState *out) {
    if (handle && out) *out = static_cast<Game *>(handle)->state;
}
CC_API int cc_drop_count(void *handle) {
    return handle ? static_cast<int>(static_cast<Game *>(handle)->drops.size()) : 0;
}
CC_API int cc_get_drop(void *handle, int index, CCDrop *out) {
    if (!handle || !out) return 0;
    const auto &drops = static_cast<Game *>(handle)->drops;
    if (index < 0 || index >= static_cast<int>(drops.size())) return 0;
    *out = drops[static_cast<std::size_t>(index)];
    return 1;
}
