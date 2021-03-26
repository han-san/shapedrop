#include <chrono>
#include <thread>

#include "draw.hpp"
#include "input.hpp"
#include "simulate.hpp"
#include "tests.hpp"

#include "core.hpp"

auto run() -> void
{
    tests::run();

    ProgramState programState {};
    MenuState menuState {};
    GameState gameState {menuState.level};

    while (programState.running) {
        auto const newFrameStartClock = std::chrono::high_resolution_clock::now();
        programState.frameTime = newFrameStartClock - programState.frameStartClock;
        programState.frameStartClock = newFrameStartClock;
        if (programState.frameTime < ProgramState::targetFrameTime) {
            auto const sleepTime = ProgramState::targetFrameTime - programState.frameTime;
            std::this_thread::sleep_for(sleepTime);
        }

        // input
        handle_input(programState, gameState);
        // sim
        simulate(programState, gameState, menuState);
        // draw border
        draw(programState, gameState);
    }
}
