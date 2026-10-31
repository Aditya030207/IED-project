# Coin Catcher — C++

A basic standalone desktop game. All game code is in `main.cpp`, written in C++17.
It does not use Unity, C#, a browser, or a scripting language.
The open-source raylib library provides the window, keyboard/mouse input, and drawing.

## Play

Run `./build.command` in this folder, then open **Coin Catcher.app** on macOS.
The app is generated locally and is not committed to the repository.

- Move with **A / D** or the **left / right arrow keys**.
- Catch yellow coins to earn one point each.
- Avoid red-rimmed bombs. Catching **3 bombs** ends the round.
- Each round lasts **60 seconds**. Running out of time also ends the round.
- Click the single **Restart** button to reset the timer, score, bombs, and basket.
- Press Escape or close the window to quit.

## Source code

`main.cpp` contains the complete game, including:

- `Drop`: a falling coin or bomb.
- `Game`: movement, spawning, catch detection, timer, score, bomb limit, and reset.
- `drawGame`: colors, objects, text, and the Restart button.
- `main`: opens the window and runs the game loop.
- `runTests`: checks game rules without opening a window.

Change `ROUND_SECONDS`, `MOVE_SPEED`, `FALL_SPEED`, or `SPAWN_INTERVAL` at the top
of `main.cpp` to adjust the game.

## Build on macOS

Install Apple's Command Line Tools if they are not already installed:

```sh
xcode-select --install
```

Then, in this folder:

```sh
./build.command
```

The build script uses the included raylib headers and static library and creates
`Coin Catcher.app`. The app requires no separate raylib installation.

Run rule checks after building:

```sh
"Coin Catcher.app/Contents/MacOS/CoinCatcher" --test
```

The included library and executable target macOS. To build for another operating
system, use raylib's matching release and compile the same `main.cpp` with C++17.

## Third-party library

raylib 6.2: https://github.com/raysan5/raylib/releases/tag/6.2

raylib is distributed under its zlib/libpng license; see `third_party/raylib/LICENSE`.
The library is third-party infrastructure; your game's source code is C++.
