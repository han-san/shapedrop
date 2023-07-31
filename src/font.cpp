#include "font.hpp"

#include "platform.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <string>
#include <string_view>

static uchar ttf_buffer[1 << 25];
stbtt_fontinfo font;
static BakedChars bakedChars;
BakedCharsBitmap bakedCharsBitmap {};

// FIXME: Temporary implementation. Should probably use std::path or something.
//        Also the path shouldn't be relative to the current working directory.
[[nodiscard]] static auto get_font_path() -> std::string { return "./"; }

auto init_font(const std::string& fontName) -> bool {
  const auto filePath = get_font_path() + fontName;
  auto* file {fopen(filePath.data(), "rb")};
  if (!file) {
    return false;
  }
  fread(ttf_buffer, 1, 1 << 25, file);
  stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(&(*ttf_buffer), 0));

  constexpr auto fontHeight = 32;
  constexpr auto offset = 0;
  constexpr auto firstChar = 32;
  constexpr auto charCount = 96;
  stbtt_BakeFontBitmap(ttf_buffer, offset, fontHeight, bakedCharsBitmap.bitmap.data(),
                       bakedCharsBitmap.w, bakedCharsBitmap.h, firstChar, charCount,
                       bakedChars.data());

  return true;
}

[[nodiscard]] static auto get_codepoint_kern_advance(const char codepoint, const char nextCodepoint,
                                                     const double scale) -> double {
  int advance {};
  int lsb {};
  stbtt_GetCodepointHMetrics(&font, codepoint, &advance, &lsb);
  return scale * static_cast<double>(
                     advance + stbtt_GetCodepointKernAdvance(&font, codepoint, nextCodepoint));
}

FontCharacter::FontCharacter(const char c, const double pixelHeight, const char nextChar)
    : character(c),
      scale(static_cast<double>(stbtt_ScaleForPixelHeight(&font, static_cast<float>(pixelHeight)))),
      advance(get_codepoint_kern_advance(c, nextChar, scale)) {
  bitmap = stbtt_GetCodepointBitmap(&font, 0, static_cast<float>(scale), c, &dimensions.w,
                                    &dimensions.h, &xoff, &yoff);
  stbtt_GetFontVMetrics(&font, &ascent, nullptr, nullptr);
}

FontCharacter::~FontCharacter() {
  stbtt_FreeBitmap(bitmap, font.userdata); // TODO: find out this actually does
}

auto get_baked_chars() -> const BakedChars& { return bakedChars; }

auto get_baked_chars_bitmap() -> const BakedCharsBitmap& { return bakedCharsBitmap; }

FontString::FontString(const std::string_view string, const double pixelHeight) {
  auto w = 0.;

  const auto size = string.size();
  data.reserve(size);
  for (std::size_t i {0}; i < size; ++i) {
    const auto c = string[i];
    const auto nextChar = i + 1 == size ? '\0' : string[i + 1];
    const auto& fontCharacter = data.emplace_back(c, pixelHeight, nextChar);
    w += fontCharacter.advance;
  }

  const auto windim = get_window_dimensions();
  normalizedDimensions = {
      w / windim.w,
      pixelHeight / windim.h,
  };
}

auto FontString::get_text_width(const std::string_view text, const double fontHeight) -> double {
  auto width = 0.;
  const auto scale =
      static_cast<double>(stbtt_ScaleForPixelHeight(&font, static_cast<float>(fontHeight)));
  const auto size = text.size();
  for (std::size_t i {0}; i < size; ++i) {
    const auto c = text[i];
    const auto nextChar = (i + 1 == size) ? '\0' : text[i + 1];
    const auto advance = get_codepoint_kern_advance(c, nextChar, scale);
    width += advance;
  }

  return width;
}

auto FontString::get_text_width_normalized(const std::string_view text,
                                           const double fontHeightNormalized) -> double {
  return get_text_width(text, get_window_dimensions().h * fontHeightNormalized);
}

auto FontString::from_width(const std::string_view string, const double desiredPixelWidth)
    -> FontString {
  // start with a reasonable pixelheight value
  auto pixelHeight = 12.;

  while (true) {
    auto width = get_text_width(string, pixelHeight);

    if (width > desiredPixelWidth) {
      pixelHeight -= 1.;
    } else if (width < desiredPixelWidth - 2.) {
      pixelHeight += 1.;
    } else {
      break;
    }
  }

  return FontString(string, pixelHeight);
}

auto FontString::from_width_normalized(const std::string_view string, const double desiredWidth)
    -> FontString {
  return from_width(string, get_window_dimensions().w * desiredWidth);
}

auto FontString::from_height(const std::string_view string, const double desiredPixelHeight)
    -> FontString {
  return FontString(string, desiredPixelHeight);
}
auto FontString::from_height_normalized(const std::string_view string, const double desiredHeight)
    -> FontString {
  return from_height(string, get_window_dimensions().h * desiredHeight);
}
