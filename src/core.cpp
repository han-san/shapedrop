#include <array>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <iostream>
#include <numeric>
#include <optional>
#include <random>
#include <string>
#include <string_view>
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

using namespace std::string_literals;
using namespace std::string_view_literals;

auto run() -> void
{
    tests::run();

    ProgramState programState {};
    MenuState menuState {};
    GameState gameState {menuState.level};


    programState.frameStartClock = clock();

    while (programState.running) {
        auto newclock {clock()};
        auto frameclocktime {newclock - programState.frameStartClock};
        programState.frameStartClock = newclock;

        // delta = (double)frameclocktime / CLOCKS_PER_SEC;
        /* auto framemstime = 1000.0 * delta; */

        // TODO: sleep so cpu doesn't melt

        // input
        handle_input(programState, gameState);
        // sim
        simulate(programState, gameState, menuState);
        // draw border
        draw(programState, gameState);
    }
}
