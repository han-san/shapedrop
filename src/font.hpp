#pragma once

#include "jint.h"
#include "util.hpp"

#include "stb_truetype.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>

class FontCharacter {
public:
  uchar* bitmap {};
  Rect<int>::Size dimensions {};
  int xoff {};
  int yoff {};
  int ascent {};
  char character {};
  // advance depends on scale and must be declared after
  double scale {};
  double advance {};

  FontCharacter(char c, double pixelHeight, char nextChar);
  ~FontCharacter();

  FontCharacter(FontCharacter&&) = default;
  auto operator=(FontCharacter&&) -> FontCharacter& = default;

  auto operator=(const FontCharacter&) -> FontCharacter& = delete;
  FontCharacter(const FontCharacter&) = delete;
};

using BakedChars = std::array<stbtt_bakedchar, 96>;
[[nodiscard]] auto get_baked_chars() -> const BakedChars&;

struct BakedCharsBitmap {
  static constexpr auto w = 512;
  static constexpr auto h = 512;
  std::array<uchar, w * h> bitmap;
};
[[nodiscard]] auto get_baked_chars_bitmap() -> const BakedCharsBitmap&;

class FontString {
public:
  std::vector<FontCharacter> data;
  Rect<double>::Size normalizedDimensions;

  [[nodiscard]] static auto from_width(std::string_view string, double desiredPixelWidth)
      -> FontString;
  [[nodiscard]] static auto from_width_normalized(std::string_view string, double desiredWidth)
      -> FontString;
  [[nodiscard]] static auto from_height(std::string_view string, double desiredPixelHeight)
      -> FontString;
  [[nodiscard]] static auto from_height_normalized(std::string_view string, double desiredHeight)
      -> FontString;
  [[nodiscard]] static auto get_text_width(std::string_view text, double fontHeight) -> double;
  [[nodiscard]] static auto get_text_width_normalized(std::string_view text,
                                                      double fontHeightNormalized) -> double;

private:
  FontString(std::string_view string, double pixelHeight);
};

auto init_font(const std::string& fontName) -> bool;
