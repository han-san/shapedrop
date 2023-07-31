#include "core.hpp"

#include "draw.hpp"
#include "input.hpp"
#include "simulate.hpp"
#include "tests.hpp"

#include <chrono>
#include <thread>

auto run() -> void {
  tests::run();

  ProgramState programState {};
  MenuState menuState {};
  GameState gameState {menuState.level};

  while (programState.running) {
    const auto newFrameStartClock = std::chrono::high_resolution_clock::now();
    programState.frameTime = newFrameStartClock - programState.frameStartClock;
    programState.frameStartClock = newFrameStartClock;
    if (programState.frameTime < ProgramState::targetFrameTime) {
      const auto sleepTime = ProgramState::targetFrameTime - programState.frameTime;
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
