// Coin Catcher: all game code is C++17. raylib provides window, input and drawing.
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

constexpr int WIDTH = 1000;
constexpr int HEIGHT = 700;
constexpr float ROUND_SECONDS = 60.0f;
constexpr float BASKET_Y = 600.0f;
constexpr float BASKET_WIDTH = 110.0f;
constexpr float MOVE_SPEED = 500.0f;
constexpr float FALL_SPEED = 220.0f;
constexpr float SPAWN_INTERVAL = 0.7f;

struct Drop {
    float x;
    float y;
    bool bomb;
};

enum class Ending { None, ThreeBombs, TimeUp };

class Game {
public:
    int score = 0;
    int bombsCaught = 0;
    float timeLeft = ROUND_SECONDS;
    float basketX = (WIDTH - BASKET_WIDTH) / 2.0f;
    float spawnTimer = SPAWN_INTERVAL;
    Ending ending = Ending::None;
    std::vector<Drop> drops;

    explicit Game(unsigned seed) : random(seed) {}

    void restart() {
        score = 0;
        bombsCaught = 0;
        timeLeft = ROUND_SECONDS;
        basketX = (WIDTH - BASKET_WIDTH) / 2.0f;
        spawnTimer = SPAWN_INTERVAL;
        ending = Ending::None;
        drops.clear();
    }

    bool gameOver() const { return ending != Ending::None; }

    void update(float seconds, float direction) {
        if (gameOver() || seconds <= 0.0f) return;
        timeLeft = std::max(0.0f, timeLeft - seconds);
        if (timeLeft <= 0.0f) {
            finish(Ending::TimeUp);
            return;
        }

        basketX = std::clamp(basketX + direction * MOVE_SPEED * seconds,
                             30.0f, WIDTH - BASKET_WIDTH - 30.0f);
        spawnTimer -= seconds;
        while (spawnTimer <= 0.0f) {
            std::uniform_real_distribution<float> xPosition(45.0f, WIDTH - 45.0f);
            std::uniform_int_distribution<int> bombChance(0, 3);
            drops.push_back({xPosition(random), 110.0f, bombChance(random) == 0});
            spawnTimer += SPAWN_INTERVAL;
        }

        for (int i = static_cast<int>(drops.size()) - 1; i >= 0; --i) {
            Drop &drop = drops[static_cast<std::size_t>(i)];
            float radius = drop.bomb ? 17.0f : 13.0f;
            float previousBottom = drop.y + radius;
            drop.y += FALL_SPEED * seconds;
            bool reachedBasket = previousBottom <= BASKET_Y && drop.y + radius >= BASKET_Y;
            bool overlapsBasket = drop.x + radius >= basketX &&
                                  drop.x - radius <= basketX + BASKET_WIDTH;
            bool caught = reachedBasket && overlapsBasket;
            bool missed = drop.y - radius > HEIGHT;
            if (caught) {
                if (drop.bomb) ++bombsCaught;
                else ++score;
            }
            if (caught || missed) drops.erase(drops.begin() + i);
            if (bombsCaught >= 3) {
                finish(Ending::ThreeBombs);
                return;
            }
        }
    }

private:
    std::mt19937 random;
    void finish(Ending reason) {
        ending = reason;
        drops.clear();
    }
};

void centeredText(const std::string &text, int y, int size, Color color) {
    DrawText(text.c_str(), (WIDTH - MeasureText(text.c_str(), size)) / 2, y, size, color);
}

void drawGame(const Game &game) {
    const Color background{23, 36, 57, 255};
    const Color gold{255, 204, 64, 255};
    const Color blue{75, 177, 255, 255};
    const Color red{244, 87, 87, 255};
    ClearBackground(background);

    DrawText(TextFormat("Score: %d", game.score), 35, 30, 28, gold);
    int seconds = static_cast<int>(std::ceil(game.timeLeft));
    centeredText(TextFormat("Time: %02d:%02d", seconds / 60, seconds % 60),
                 30, 28, game.timeLeft <= 10.0f ? red : WHITE);
    DrawText(TextFormat("Bombs: %d / 3", game.bombsCaught), WIDTH - 235, 30, 28, red);

    for (const Drop &drop : game.drops) {
        Vector2 center{drop.x, drop.y};
        if (drop.bomb) {
            DrawCircleV(center, 17.0f, red);
            DrawCircleV(center, 13.0f, Color{32, 35, 46, 255});
            DrawLineEx({drop.x + 6, drop.y - 13}, {drop.x + 12, drop.y - 26}, 4, ORANGE);
        } else {
            DrawCircleV(center, 13.0f, gold);
            DrawCircleLines(static_cast<int>(drop.x), static_cast<int>(drop.y), 9.0f,
                            Color{191, 139, 28, 255});
        }
    }

    DrawRectangleRec({game.basketX, BASKET_Y, BASKET_WIDTH, 23.0f}, blue);
    centeredText("A / D or arrow keys to move. Catch coins. Avoid bombs.", 660, 18,
                 Color{193, 208, 226, 255});

    if (game.gameOver()) {
        DrawRectangle(0, 0, WIDTH, HEIGHT, Color{0, 0, 0, 195});
        DrawRectangle(250, 205, 500, 290, Color{37, 56, 81, 255});
        centeredText("Game Over", 240, 38, WHITE);
        centeredText(game.ending == Ending::ThreeBombs ? "You caught 3 bombs!" : "Time is up!",
                     300, 24, WHITE);
        centeredText(TextFormat("Score: %d", game.score), 340, 24, gold);
        Rectangle button{385, 410, 230, 60};
        bool hover = CheckCollisionPointRec(GetMousePosition(), button);
        DrawRectangleRec(button, hover ? Color{255, 223, 115, 255} : gold);
        centeredText("Restart", 428, 26, background);
    }
}

void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}

int runTests() {
    Game game(123);
    game.spawnTimer = 1000;
    float center = game.basketX + BASKET_WIDTH / 2;
    game.drops = {{center, 586, false}};
    game.update(0.02f, 0);
    require(game.score == 1 && game.drops.empty(), "Coin catch must add one point");
    for (int i = 1; i <= 3; ++i) {
        game.drops.push_back({center, 582, true});
        game.update(0.02f, 0);
        require(game.bombsCaught == i, "Bomb catch must increase the counter");
        require(game.gameOver() == (i == 3), "Game must end only on the third bomb");
    }
    require(game.ending == Ending::ThreeBombs, "Third bomb must set correct game-over reason");
    float frozenTime = game.timeLeft;
    game.update(1, 1);
    require(game.timeLeft == frozenTime, "Game-over state must stop the timer");
    game.restart();
    require(game.score == 0 && game.bombsCaught == 0 && game.timeLeft == 60 &&
            !game.gameOver() && game.drops.empty() && game.basketX == 445,
            "Restart must reset the whole round");
    game.spawnTimer = 1000;
    game.drops.push_back({35, 725, true});
    game.update(0.02f, 0);
    require(game.bombsCaught == 0 && game.drops.empty(), "Missed bombs must not count");
    game.update(1, -1);
    require(game.basketX == 30, "Basket must stay within left edge");
    game.update(5, 1);
    require(game.basketX == 860, "Basket must stay within right edge");
    game.restart();
    game.update(60, 0);
    require(game.gameOver() && game.timeLeft == 0 && game.ending == Ending::TimeUp,
            "Timer must end at zero with the correct reason");
    Game slow(7), fast(7);
    for (int i = 0; i < 30; ++i) slow.update(1.0f / 30, 0.1f);
    for (int i = 0; i < 120; ++i) fast.update(1.0f / 120, 0.1f);
    require(std::abs(slow.timeLeft - fast.timeLeft) < 0.001f &&
            std::abs(slow.basketX - fast.basketX) < 0.01f,
            "Movement and timer must be frame-rate independent");
    std::cout << "PASS: coin scoring, 3-bomb game over, frozen end state, restart, missed bombs, "
                 "movement limits, timer expiration and frame-rate independence.\n";
    return 0;
}

int main(int argc, char **argv) {
    try {
        if (argc > 1 && std::string(argv[1]) == "--test") return runTests();
        SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_ALWAYS_RUN);
        InitWindow(WIDTH, HEIGHT, "Coin Catcher - C++");
        if (!IsWindowReady()) return 1;
        SetTargetFPS(60);
        Game game(std::random_device{}());
        const Rectangle restartButton{385, 410, 230, 60};
        while (!WindowShouldClose()) {
            if (game.gameOver()) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                    CheckCollisionPointRec(GetMousePosition(), restartButton)) game.restart();
            } else {
                float direction = 0;
                if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direction -= 1;
                if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction += 1;
                game.update(GetFrameTime(), direction);
            }
            BeginDrawing();
            drawGame(game);
            EndDrawing();
        }
        CloseWindow();
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Coin Catcher: " << error.what() << '\n';
        return 1;
    }
}
