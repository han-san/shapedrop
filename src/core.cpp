#include <array>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <ctime>
#include <iostream>
#include <numeric>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "fmt/core.h"

#include "jint.h"

#include "board.hpp"
#include "simulate.hpp"
#include "draw.hpp"
#include "platform.hpp"
#include "shape.hpp"
#include "tests.hpp"
#include "ui.hpp"
#include "util.hpp"
#include "input.hpp"

#include "core.hpp"

auto run() -> void
{
    tests::run();

    ProgramState programState {};
    MenuState menuState {};
    GameState gameState {menuState.level};

    while (programState.running) {
        auto const newFrameStartClock {std::chrono::high_resolution_clock::now()};
        programState.frameTime = newFrameStartClock - programState.frameStartClock;
        programState.frameStartClock = newFrameStartClock;

        // input
        handle_input(programState, gameState);
        // sim
        simulate(programState, gameState, menuState);
        // draw border
        draw(programState, gameState);
    }
}
