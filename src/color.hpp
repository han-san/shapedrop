#pragma once

#include "jint.h"
#include "positive.hpp"

namespace Color {
struct RGBA {
  static constexpr u8 maxChannelValue {0xFF};
  struct Alpha {
    static constexpr u8 opaque {maxChannelValue};
    static constexpr u8 transparent {0};
  };

  Positive<u8> r {0U};
  Positive<u8> g {0U};
  Positive<u8> b {0U};
  Positive<u8> a {Alpha::opaque};
};

static constexpr RGBA red {.r = RGBA::maxChannelValue, .g = 0U, .b = 0U};
static constexpr RGBA green {.r = 0U, .g = RGBA::maxChannelValue, .b = 0U};
static constexpr RGBA blue {.r = 0U, .g = 0U, .b = RGBA::maxChannelValue};
static constexpr RGBA cyan {.r = 0U, .g = RGBA::maxChannelValue, .b = RGBA::maxChannelValue};
static constexpr RGBA white {
    .r = RGBA::maxChannelValue, .g = RGBA::maxChannelValue, .b = RGBA::maxChannelValue};
static constexpr RGBA black {.r = 0U, .g = 0U, .b = 0U};
static constexpr RGBA transparent {.r = 0U, .g = 0U, .b = 0U, .a = RGBA::Alpha::transparent};

// An invalid color to give some visual feedback when a color hasn't been
// properly initialized. White isn't really used otherwise in the game, so
// hopefully it will be obvious that something is wrong.
static constexpr RGBA invalid {white};

struct Shape {
  static constexpr RGBA I {.r = 0U, .g = 0xF0U, .b = 0xF0U};
  static constexpr RGBA O {.r = 0xF0U, .g = 0xF0U, .b = 0U};
  static constexpr RGBA L {.r = 0xF0U, .g = 0xA0U, .b = 0U};
  static constexpr RGBA J {.r = 0U, .g = 0U, .b = 0xF0U};
  static constexpr RGBA S {.r = 0U, .g = 0xF0U, .b = 0U};
  static constexpr RGBA Z {.r = 0xF0U, .g = 0U, .b = 0U};
  static constexpr RGBA T {.r = 0xA0U, .g = 0U, .b = 0xF0U};
};
} // namespace Color
