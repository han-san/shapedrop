#include "input.hpp"

#include "core.hpp"
#include "platform.hpp"
#include "ui.hpp"

auto handle_input(ProgramState& programState, GameState& gameState) -> void {
  Event event;
  while ((event = get_event()).type != Event::Type::None) {
    UI::update_state(event);

    // First check events independent of whether in menu or game
    if (event.type == Event::Type::Quit) {
      programState.running = false;
    } else if (event.type == Event::Type::Reset) {
      gameState.reset();
    } else if (event.type == Event::Type::Increase_window_size) {
      change_window_scale(get_window_scale() + 1);
    } else if (event.type == Event::Type::Decrease_window_size) {
      change_window_scale(get_window_scale() - 1);
    } else if (programState.levelType == ProgramState::LevelType::Game) {
      auto update_shadow_and_clocks = [&](bool isGrounded) {
        gameState.currentShapeShadow =
            gameState.board.get_shadow(gameState.currentShape);
        gameState.lockClock = programState.frameStartClock;
        if (isGrounded) {
          gameState.dropClock = programState.frameStartClock;
        }
      };

      enum class HorDir { Left, Right };

      auto move_horizontal = [&](HorDir const dir) {
        // if currentShape is on top of a block before move,
        // the drop clock needs to be reset
        auto const isGrounded =
            !gameState.board.is_valid_move(gameState.currentShape, V2::down());
        auto const dirVec = dir == HorDir::Right ? V2::right() : V2::left();
        if (gameState.board.try_move(gameState.currentShape, dirVec)) {
          update_shadow_and_clocks(isGrounded);
          // if you move the piece you cancel the drop
          gameState.droppedRows = 0;
          if (isGrounded) {
            gameState.softDropRowCount = 0;
          }
        }
      };

      auto rotate_current_shape = [&](Shape::RotationDirection rot) {
        // if currentShape is on top of a block before rotation,
        // the drop clock needs to be reset
        auto const isGrounded =
            !gameState.board.is_valid_move(gameState.currentShape, V2::down());
        if (auto const rotation =
                gameState.board.rotate_shape(gameState.currentShape, rot)) {
          update_shadow_and_clocks(isGrounded);
          gameState.currentRotationType = rotation;
          // if you rotate the piece you cancel the drop
          gameState.droppedRows = 0;
          if ((rotation == Shape::RotationType::Wallkick) && isGrounded) {
            gameState.softDropRowCount = 0;
          }
        }
      };

      if (event.type == Event::Type::Move_right) {
        move_horizontal(HorDir::Right);
      } else if (event.type == Event::Type::Move_left) {
        move_horizontal(HorDir::Left);
      } else if (event.type == Event::Type::Increase_speed) {
        // TODO: How does this work if you e.g. press
        // left/right/rotate while holding button down?
        // is isSoftDropping still true at that time?

        // This event currently gets spammed when you hold down
        // the button, so resetting the soft drop count directly
        // will continue resetting it while the button is pressed.
        // In order to avoid that we check if isSoftDropping has
        // been set, which only happens during spam.
        if (!gameState.isSoftDropping) {
          gameState.softDropRowCount = 0;
        }
        gameState.isSoftDropping = true;
      } else if (event.type == Event::Type::Reset_speed) {
        gameState.isSoftDropping = false;

        // softdrops only get reset if the piece can currently fall
        if (gameState.board.is_valid_move(gameState.currentShape, V2::down())) {
          gameState.softDropRowCount = 0;
        }
      } else if (event.type == Event::Type::Drop) {
        auto droppedRows = 0;
        while (gameState.board.try_move(gameState.currentShape, V2::down())) {
          gameState.lockClock = programState.frameStartClock;
          gameState.currentRotationType = std::nullopt;
          ++droppedRows;
        }
        gameState.droppedRows = droppedRows;

        // hard drop overrides soft drop
        if (droppedRows) {
          gameState.softDropRowCount = 0;
        }
      } else if (event.type == Event::Type::Rotate_left) {
        rotate_current_shape(Shape::RotationDirection::Left);
      } else if (event.type == Event::Type::Rotate_right) {
        rotate_current_shape(Shape::RotationDirection::Right);
      } else if (event.type == Event::Type::Hold) {
        if (!gameState.hasHeld) {
          gameState.hasHeld = true;
          gameState.currentRotationType = std::nullopt;
          if (gameState.holdShapeType) {
            auto const holdType = *gameState.holdShapeType;

            gameState.holdShapeType = gameState.currentShape.type;
            gameState.currentShape = Shape {holdType};
          } else {
            gameState.holdShapeType = gameState.currentShape.type;
            gameState.currentShape = gameState.shapePool.next_shape();
          }

          gameState.softDropRowCount = 0;
          gameState.droppedRows = 0;

          auto const isGrounded = !gameState.board.is_valid_move(
              gameState.currentShape, V2::down());
          update_shadow_and_clocks(isGrounded);
        }
      } else if (event.type == Event::Type::Pause) {
        gameState.paused = !gameState.paused;
        // TODO: maybe save the amount of clocks left when the game was paused
        // and set them again here.
        gameState.dropClock = programState.frameStartClock;
        gameState.lockClock = programState.frameStartClock;
      }
    }
  }
}
