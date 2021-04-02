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

  auto operator=(FontCharacter const&) -> FontCharacter& = delete;
  FontCharacter(FontCharacter const&) = delete;
};

using BakedChars = std::array<stbtt_bakedchar, 96>;
[[nodiscard]] auto get_baked_chars() -> BakedChars const&;

struct BakedCharsBitmap {
  auto constexpr static w = 512;
  auto constexpr static h = 512;
  std::array<uchar, w * h> bitmap;
};
[[nodiscard]] auto get_baked_chars_bitmap() -> BakedCharsBitmap const&;

class FontString {
public:
  std::vector<FontCharacter> data;
  Rect<double>::Size normalizedDimensions;

  [[nodiscard]] auto static from_width(std::string_view string,
                                       double desiredPixelWidth) -> FontString;
  [[nodiscard]] auto static from_width_normalized(std::string_view string,
                                                  double desiredWidth)
      -> FontString;
  [[nodiscard]] auto static from_height(std::string_view string,
                                        double desiredPixelHeight)
      -> FontString;
  [[nodiscard]] auto static from_height_normalized(std::string_view string,
                                                   double desiredHeight)
      -> FontString;
  [[nodiscard]] auto static get_text_width(std::string_view text,
                                           double fontHeight) -> double;
  [[nodiscard]] auto static get_text_width_normalized(
      std::string_view text, double fontHeightNormalized) -> double;

private:
  FontString(std::string_view string, double pixelHeight);
};

auto init_font(std::string const& fontName) -> bool;
