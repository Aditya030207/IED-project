#include "CoinCatcher.cpp"
#include <iostream>
#include <stdexcept>
void check(bool ok, const char *message) { if (!ok) throw std::runtime_error(message); }
int main() {
    Game game(42);
    game.spawnTimer = 1000;
    game.drops.push_back({0, 0, -274, 0});
    game.update(.02f, 0);
    check(game.state.score == 1 && game.drops.empty(), "coin score");
    for (int i = 1; i <= 3; ++i) {
        game.drops.push_back({i, 0, -270, 1});
        game.update(.02f, 0);
        check(game.state.bombs == i, "bomb count");
        check(game.state.gameOver == (i == 3), "third bomb limit");
    }
    check(game.state.reason == 1, "bomb game-over reason");
    auto seconds = game.state.seconds;
    game.update(2, 1);
    check(game.state.seconds == seconds, "game-over freezes timer");
    game.restart();
    check(game.state.score == 0 && game.state.bombs == 0 && game.state.seconds == 60 &&
          game.state.basketX == 0 && !game.state.gameOver && game.drops.empty(), "restart");
    game.spawnTimer = 1000;
    game.drops.push_back({0, 400, -420, 1});
    game.update(.02f, 0);
    check(game.state.bombs == 0 && game.drops.empty(), "missed bomb does not count");
    game.update(2, -1);
    check(game.state.basketX == -500, "left boundary");
    game.update(3, 1);
    check(game.state.basketX == 500, "right boundary");
    game.restart();
    game.update(60, 0);
    check(game.state.gameOver && game.state.reason == 2 && game.state.seconds == 0, "timer");
    Game a(42), b(42);
    for (int i = 0; i < 30; ++i) a.update(1.f / 30, .1f);
    for (int i = 0; i < 120; ++i) b.update(1.f / 120, .1f);
    check(std::abs(a.state.basketX - b.state.basketX) < .01f &&
          std::abs(a.state.seconds - b.state.seconds) < .001f, "frame-rate independence");
    void *handle = cc_create();
    check(handle != nullptr, "native handle");
    CCState state; cc_get_state(handle, &state);
    check(state.seconds == 60 && state.score == 0, "C API state");
    cc_update(handle, 1, 1); cc_get_state(handle, &state);
    check(state.basketX == 500 && state.seconds == 59, "C API update");
    CCDrop drop;
    check(cc_drop_count(handle) == 1 && cc_get_drop(handle, 0, &drop) == 1, "C API drops");
    check(cc_get_drop(handle, -1, &drop) == 0, "invalid index");
    cc_restart(handle); cc_get_state(handle, &state);
    check(state.seconds == 60 && cc_drop_count(handle) == 0, "C API restart");
    cc_destroy(handle);
    std::cout << "PASS: C++ scoring, bombs, timer, movement, restart, frame timing, and Unity bridge API\n";
}
