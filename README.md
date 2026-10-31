# Coin Catcher — C++

The standalone C++ game is in [CoinCatcherCpp](CoinCatcherCpp/).
All game logic, rendering calls, input handling, and interface code are written in C++17.
The included raylib 6.2 library provides the desktop window, graphics, and input APIs.

## Rules

- Move with **A / D** or **left / right arrow keys**.
- Catch yellow coins to earn points.
- Catching **3 bombs** ends the game.
- Each round lasts **60 seconds**.
- The single **Restart** button resets the entire round.

## Run on macOS

With Apple Command Line Tools installed:

```sh
cd CoinCatcherCpp
./build.command
```

Then open `Coin Catcher.app` in that folder.
The build includes automated checks for scoring, bombs, timer, movement, and restart.
The included static library supports Apple Silicon and Intel Macs.

- [Complete game source](CoinCatcherCpp/main.cpp)
- [Detailed build instructions and source walkthrough](CoinCatcherCpp/README.md)
- [raylib license](CoinCatcherCpp/third_party/raylib/LICENSE)

The repository also retains the earlier Unity project. The standalone C++ game does
not use or require Unity or C#.
