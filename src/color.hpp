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

static constexpr RGBA red {RGBA::maxChannelValue, 0U, 0U};
static constexpr RGBA green {0U, RGBA::maxChannelValue, 0U};
static constexpr RGBA blue {0U, 0U, RGBA::maxChannelValue};
static constexpr RGBA cyan {0U, RGBA::maxChannelValue, RGBA::maxChannelValue};
static constexpr RGBA white {RGBA::maxChannelValue, RGBA::maxChannelValue, RGBA::maxChannelValue};
static constexpr RGBA black {0U, 0U, 0U};
static constexpr RGBA transparent {0U, 0U, 0U, RGBA::Alpha::transparent};

// An invalid color to give some visual feedback when a color hasn't been
// properly initialized. White isn't really used otherwise in the game, so
// hopefully it will be obvious that something is wrong.
static constexpr RGBA invalid {white};

struct Shape {
  static constexpr RGBA I {0U, 0xF0U, 0xF0U};
  static constexpr RGBA O {0xF0U, 0xF0U, 0U};
  static constexpr RGBA L {0xF0U, 0xA0U, 0U};
  static constexpr RGBA J {0U, 0U, 0xF0U};
  static constexpr RGBA S {0U, 0xF0U, 0U};
  static constexpr RGBA Z {0xF0U, 0U, 0U};
  static constexpr RGBA T {0xA0U, 0U, 0xF0U};
};
} // namespace Color
