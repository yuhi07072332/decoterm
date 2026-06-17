// ╔╦╗┌─┐┌─┐┌─┐╔╦╗┌─┐┬─┐┌┬┐
//  ║║├┤ │  │ │ ║ ├┤ ├┬┘│││
// ═╩╝└─┘└─┘└─┘ ╩ └─┘┴└─┴ ┴
//
// MIT Licence
//
// Copyright (c) 2026 Yuhi0707
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


// TODO:
// - Detect terminal color support info & provide fallback system color for true color
// - compile time to_escape() cache for style that only contains system color fg/bg
// - Windows API fallback for Windows8 or lower versions

#ifndef DECOTERM_HPP
#define DECOTERM_HPP

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>

namespace deco {

namespace detail {

// clang-format off
inline constexpr std::array<std::string_view, 16> SGR_PARAM_FG {
    "30", "31", "32",
    "33", "34", "35",
    "36", "37", "90",
    "91", "92", "93",
    "94", "95", "96",
    "97"
};

inline constexpr std::array<std::string_view, 16> SGR_PARAM_BG {
    "40", "41", "42",
    "43", "44", "45",
    "46", "47", "100",
    "101", "102", "103",
    "104", "105", "106",
    "107"
};

inline constexpr std::array<std::string_view, 8> SGR_PARAMS_STYLE {
    "1" /*bold*/,           "2"  /*dim*/,           "3" /*italic*/,
    "4" /*underline*/,      "5"  /*blink*/,         "7" /*invert*/,
    "9" /*strikethrough*/,  "21" /*double underline*/
};

// clang-format on

}   // namespace detail

// ╔═════════════════════════════════════════════════════════╗
// ║                          Color                          ║
// ╚═════════════════════════════════════════════════════════╝

enum class Colors : uint8_t;

struct Color {
    enum class Type : uint8_t { None = 0, Default, Colors, TrueColor };
    
    enum SpecialColor : uint8_t {
        None = 0,
        Default = 1
    };

    // ----- constructors -----

    /// @brief Create Color::None
    constexpr Color();

    constexpr Color(SpecialColor sp) : data_({0, 0, 0}) {
        if (sp == None) type_ = Type::None;
        else if (sp == Default) type_ = Type::Default;
        else 
            throw std::invalid_argument("Color(): invalid SpecialColor value");
    }

    constexpr Color(Colors col)
        : type_(Type::Colors), data_({static_cast<uint8_t>(col), 0, 0}) {}

    constexpr Color(uint8_t r, uint8_t g, uint8_t b)
        : type_(Type::TrueColor), data_({r, g, b}) {}

    // ----- operators -----

    constexpr auto operator==(const Color&) const -> bool = default;
    constexpr auto operator!=(const Color&) const -> bool = default;

    constexpr explicit operator bool() const { return !empty(); }

    auto empty() const -> bool { return type_ == Type::None; }

    // ----- output -----

    /// @brief generate SGR parameters separated by ';'
    [[nodiscard]]
    auto to_escape_params(bool is_bg) const -> std::string {
        switch (type_) {
            case Type::None :
                return "";
            case Type::Default :
                return is_bg ? "49" : "39";
            case Type::Colors : {
                std::string esc;
                esc.reserve(8);
                if (is_system_color()) {
                return std::string(is_bg
                                   ? detail::SGR_PARAM_BG[data_[0]]
                                   : detail::SGR_PARAM_FG[data_[0]]);
                } else return esc
                        + (is_bg ? "48;5;" : "38;5;")
                        + std::to_string(data_[0]);
            }
            case Type::TrueColor : {
                const auto [r, g, b] = data_;
                std::string esc;
                esc.reserve(19);
                return esc 
                    + (is_bg ? "48;2;" : "38;2;")
                    + std::to_string(r) + ';'
                    + std::to_string(g) + ';'
                    + std::to_string(b);
            }
        }
        return "";
    }

    /// @brief generate full escape code
    [[nodiscard]]
    auto to_escape(bool is_bg) const -> std::string {
        return std::string("\x1b[")
            + to_escape_params(is_bg)
            + 'm';
    }

private:
    constexpr auto is_system_color() const -> bool {
        return data_[0] < 16;
    }

    Type type_;
    std::array<uint8_t, 3> data_;
};

/// @brief Create a color by RGB
inline constexpr auto rgb(uint8_t r, uint8_t g, uint8_t b) -> Color {
    return Color(r, g, b);
}

/// @brief Create a color by 0xRRGGBB
/// @pre rgb <= 0xFFFFFF
inline constexpr auto rgb(uint32_t hex) -> Color {
    // clang-format off
    if (hex > 0xffffff) throw std::invalid_argument("Color::rgb(): rgb > 0xffffff");
    return Color((hex >> 16) & 0xFF,
                      (hex >> 8) & 0xFF,
                      hex & 0xFF);
    // clang-format on
}

/// @brief Create a color by HSV
/// @param h [0, 360): Hue of the color
/// @param s [0, 255]: Saturation of the color
/// @param v [0, 255]: Value (brightness) of the color
inline constexpr auto hsv(int h, uint8_t s, uint8_t v) -> Color {
    // clang-format off
    if (h < 0 || h >= 360) throw std::invalid_argument(
        "Color::hsv(): h is not in range [0, 360)");

    float hp = h / 60.f;
    float sp = s / 255.0f;
    float vp = v / 255.0f;

    float f = hp - std::floor(hp);
    uint8_t p = std::round(vp * (1 - sp) * 255);
    uint8_t q = std::round(vp * (1 - f * sp) * 255);
    uint8_t t = std::round(vp * (1 - (1 - f) * sp) * 255);

    switch (h / 60) {
        case 0 : return Color(v, t, p);
        case 1 : return Color(q, v, p);
        case 2 : return Color(p, v, t);
        case 3 : return Color(p, q, v);
        case 4 : return Color(t, p, v);
        case 5 : return Color(v, p, q);
        default: assert(false);
    }
    // clang-format on
}


// ╔═════════════════════════════════════════════════════════╗
// ║                          Style                          ║
// ╚═════════════════════════════════════════════════════════╝

struct Style {
    enum AttributeFlags : uint8_t {
        None            = 0,
        Bold            = 1 << 0,
        Dim             = 1 << 1,
        Italic          = 1 << 2,
        Underline       = 1 << 3,
        Blink           = 1 << 4,
        Invert          = 1 << 5,
        Strikethrough   = 1 << 6,
        UnderlineDouble = 1 << 7,
    };

    uint8_t flags   = None;
    Color fg        = Color::None;
    Color bg        = Color::None;

    // ----- operators -----

    constexpr auto operator|(Style rhs) const -> Style {
        return Style(
            flags | rhs.flags,
            rhs.fg ? rhs.fg : fg,
            rhs.bg ? rhs.bg : bg
        );
    }

    constexpr auto operator==(const Style&) const -> bool = default;
    constexpr auto operator!=(const Style&) const -> bool = default;

    constexpr explicit operator bool() const { return !empty(); }

    auto empty() const -> bool {
        return flags == None && fg.empty() && bg.empty();
    }

    // ----- output -----

    [[nodiscard]]
    auto to_escape_params() const -> std::string {
        if (empty()) return "";

        std::string esc;
        if (fg) esc += fg.to_escape_params(false) + ';';
        if (bg) esc += bg.to_escape_params(true) + ';';

        int count = 0;
        uint8_t current_flag = flags;
        while (current_flag) {
            if (current_flag & 0x1) 
                esc.append(detail::SGR_PARAMS_STYLE[count]).push_back(';');
            count++;
            current_flag >>= 1;
        }

        if (!esc.empty() && esc.back() == ';') esc.pop_back();

        return esc;
    }

    auto to_escape() const -> std::string {
        return "\x1b[" + to_escape_params() + "m";
    }
};

// ----- color -----

inline constexpr auto color(Color fg, Color bg) -> Style {
    return Style{ .fg = fg, .bg = bg };
}

inline constexpr auto fg(Color fg) -> Style { return Style{ .fg = fg }; }
inline constexpr auto bg(Color bg) -> Style { return Style{ .bg = bg }; }

// ----- styles -----

inline constexpr Style bold              = Style(Style::Bold);
inline constexpr Style dim               = Style(Style::Dim);
inline constexpr Style italic            = Style(Style::Italic);
inline constexpr Style underline         = Style(Style::Underline);
inline constexpr Style blink             = Style(Style::Blink);
inline constexpr Style invert            = Style(Style::Invert);
inline constexpr Style strikethrough     = Style(Style::Strikethrough);
inline constexpr Style underline_double  = Style(Style::UnderlineDouble);

// ----- reset -----

struct StyleReset {
    Style reset_to = Style();

    constexpr auto operator|(Style rhs) const -> Style {
        return reset_to | rhs;
    }
};

inline constexpr auto reset_to(Style style) -> StyleReset {
    return StyleReset(style);
}

inline constexpr StyleReset reset = StyleReset();

// ----- style output -----

inline auto operator<<(std::ostream& os, Style rhs) -> std::ostream& {
    os << rhs.to_escape();
    return os;
}

inline auto operator<<(std::ostream& os, StyleReset rhs) -> std::ostream& {
    os << "\x1b[;" << rhs.reset_to.to_escape_params() << "m";
    return os;
}


// ╔═════════════════════════════════════════════════════════╗
// ║                         Colors                          ║
// ╚═════════════════════════════════════════════════════════╝

enum class Colors : uint8_t {
    // system colors
    Black             = 0,
    Red               = 1,
    Green             = 2,
    Yellow            = 3,
    Blue              = 4,
    Magenta           = 5,
    Cyan              = 6,
    White             = 7,
    BlackLight        = 8,
    RedLight          = 9,
    GreenLight        = 10,
    YellowLight       = 11,
    BlueLight         = 12,
    MagentaLight      = 13,
    CyanLight         = 14,
    WhiteLight        = 15,

    // non-system colors
    // color names are from https://www.ditig.com/256-colors-cheat-sheet#google_vignette
    Grey0             = 16,
    NavyBlue          = 17,
    DarkBlue          = 18,
    Blue3             = 19,
    Blue3_1           = 20,
    Blue1             = 21,
    DarkGreen         = 22,
    DeepSkyBlue4      = 23,
    DeepSkyBlue4_1    = 24,
    DeepSkyBlue4_2    = 25,
    DodgerBlue3       = 26,
    DodgerBlue2       = 27,
    Green4            = 28,
    SpringGreen4      = 29,
    Turquoise4        = 30,
    DeepSkyBlue3      = 31,
    DeepSkyBlue3_1    = 32,
    DodgerBlue1       = 33,
    Green3            = 34,
    SpringGreen3      = 35,
    DarkCyan          = 36,
    LightSeaGreen     = 37,
    DeepSkyBlue2      = 38,
    DeepSkyBlue1      = 39,
    Green3_1          = 40,
    SpringGreen3_1    = 41,
    SpringGreen2      = 42,
    Cyan3             = 43,
    DarkTurquoise     = 44,
    Turquoise2        = 45,
    Green1            = 46,
    SpringGreen2_1    = 47,
    SpringGreen1      = 48,
    MediumSpringGreen = 49,
    Cyan2             = 50,
    Cyan1             = 51,
    DarkRed           = 52,
    DeepPink4         = 53,
    Purple4           = 54,
    Purple4_1         = 55,
    Purple3           = 56,
    BlueViolet        = 57,
    Orange4           = 58,
    Grey37            = 59,
    MediumPurple4     = 60,
    SlateBlue3        = 61,
    SlateBlue3_1      = 62,
    RoyalBlue1        = 63,
    Chartreuse4       = 64,
    DarkSeaGreen4     = 65,
    PaleTurquoise4    = 66,
    SteelBlue         = 67,
    SteelBlue3        = 68,
    CornflowerBlue    = 69,
    Chartreuse3       = 70,
    DarkSeaGreen4_1   = 71,
    CadetBlue         = 72,
    CadetBlue_1       = 73,
    SkyBlue3          = 74,
    SteelBlue1        = 75,
    Chartreuse3_1     = 76,
    PaleGreen3        = 77,
    SeaGreen3         = 78,
    Aquamarine3       = 79,
    MediumTurquoise   = 80,
    SteelBlue1_1      = 81,
    Chartreuse2       = 82,
    SeaGreen2         = 83,
    SeaGreen1         = 84,
    SeaGreen1_1       = 85,
    Aquamarine1       = 86,
    DarkSlateGray2    = 87,
    DarkRed_1         = 88,
    DeepPink4_1       = 89,
    DarkMagenta       = 90,
    DarkMagenta_1     = 91,
    DarkViolet        = 92,
    Purple            = 93,
    Orange4_1         = 94,
    LightPink4        = 95,
    Plum4             = 96,
    MediumPurple3     = 97,
    MediumPurple3_1   = 98,
    SlateBlue1        = 99,
    Yellow4           = 100,
    Wheat4            = 101,
    Grey53            = 102,
    LightSlateGrey    = 103,
    MediumPurple      = 104,
    LightSlateBlue    = 105,
    Yellow4_1         = 106,
    DarkOliveGreen3   = 107,
    DarkSeaGreen      = 108,
    LightSkyBlue3     = 109,
    LightSkyBlue3_1   = 110,
    SkyBlue2          = 111,
    Chartreuse2_1     = 112,
    DarkOliveGreen3_1 = 113,
    PaleGreen3_1      = 114,
    DarkSeaGreen3     = 115,
    DarkSlateGray3    = 116,
    SkyBlue1          = 117,
    Chartreuse1       = 118,
    LightGreen        = 119,
    LightGreen_1      = 120,
    PaleGreen1        = 121,
    Aquamarine1_1     = 122,
    DarkSlateGray1    = 123,
    Red3              = 124,
    DeepPink4_2       = 125,
    MediumVioletRed   = 126,
    Magenta3          = 127,
    DarkViolet_1      = 128,
    Purple_1          = 129,
    DarkOrange3       = 130,
    IndianRed         = 131,
    HotPink3          = 132,
    MediumOrchid3     = 133,
    MediumOrchid      = 134,
    MediumPurple2     = 135,
    DarkGoldenrod     = 136,
    LightSalmon3      = 137,
    RosyBrown         = 138,
    Grey63            = 139,
    MediumPurple2_1   = 140,
    MediumPurple1     = 141,
    Gold3             = 142,
    DarkKhaki         = 143,
    NavajoWhite3      = 144,
    Grey69            = 145,
    LightSteelBlue3   = 146,
    LightSteelBlue    = 147,
    Yellow3           = 148,
    DarkOliveGreen3_2 = 149,
    DarkSeaGreen3_1   = 150,
    DarkSeaGreen2     = 151,
    LightCyan3        = 152,
    LightSkyBlue1     = 153,
    GreenYellow       = 154,
    DarkOliveGreen2   = 155,
    PaleGreen1_1      = 156,
    DarkSeaGreen2_1   = 157,
    DarkSeaGreen1     = 158,
    PaleTurquoise1    = 159,
    Red3_1            = 160,
    DeepPink3         = 161,
    DeepPink3_1       = 162,
    Magenta3_1        = 163,
    Magenta3_2        = 164,
    Magenta2          = 165,
    DarkOrange3_1     = 166,
    IndianRed_1       = 167,
    HotPink3_1        = 168,
    HotPink2          = 169,
    Orchid            = 170,
    MediumOrchid1     = 171,
    Orange3           = 172,
    LightSalmon3_1    = 173,
    LightPink3        = 174,
    Pink3             = 175,
    Plum3             = 176,
    Violet            = 177,
    Gold3_1           = 178,
    LightGoldenrod3   = 179,
    Tan               = 180,
    MistyRose3        = 181,
    Thistle3          = 182,
    Plum2             = 183,
    Yellow3_1         = 184,
    Khaki3            = 185,
    LightGoldenrod2   = 186,
    LightYellow3      = 187,
    Grey84            = 188,
    LightSteelBlue1   = 189,
    Yellow2           = 190,
    DarkOliveGreen1   = 191,
    DarkOliveGreen1_1 = 192,
    DarkSeaGreen1_1   = 193,
    Honeydew2         = 194,
    LightCyan1        = 195,
    Red1              = 196,
    DeepPink2         = 197,
    DeepPink1         = 198,
    DeepPink1_1       = 199,
    Magenta2_1        = 200,
    Magenta1          = 201,
    OrangeRed1        = 202,
    IndianRed1        = 203,
    IndianRed1_1      = 204,
    HotPink           = 205,
    HotPink_1         = 206,
    MediumOrchid1_1   = 207,
    DarkOrange        = 208,
    Salmon1           = 209,
    LightCoral        = 210,
    PaleVioletRed1    = 211,
    Orchid2           = 212,
    Orchid1           = 213,
    Orange1           = 214,
    SandyBrown        = 215,
    LightSalmon1      = 216,
    LightPink1        = 217,
    Pink1             = 218,
    Plum1             = 219,
    Gold1             = 220,
    LightGoldenrod2_1 = 221,
    LightGoldenrod2_2 = 222,
    NavajoWhite1      = 223,
    MistyRose1        = 224,
    Thistle1          = 225,
    Yellow1           = 226,
    LightGoldenrod1   = 227,
    Khaki1            = 228,
    Wheat1            = 229,
    Cornsilk1         = 230,
    Grey100           = 231,
    Grey3             = 232,
    Grey7             = 233,
    Grey11            = 234,
    Grey15            = 235,
    Grey19            = 236,
    Grey23            = 237,
    Grey27            = 238,
    Grey30            = 239,
    Grey35            = 240,
    Grey39            = 241,
    Grey42            = 242,
    Grey46            = 243,
    Grey50            = 244,
    Grey54            = 245,
    Grey58            = 246,
    Grey62            = 247,
    Grey66            = 248,
    Grey70            = 249,
    Grey74            = 250,
    Grey78            = 251,
    Grey82            = 252,
    Grey85            = 253,
    Grey89            = 254,
    Grey93            = 255,
};

inline constexpr Color black        = Color(Colors::Black);
inline constexpr Color red          = Color(Colors::Red);
inline constexpr Color green        = Color(Colors::Green);
inline constexpr Color yellow       = Color(Colors::Yellow);
inline constexpr Color blue         = Color(Colors::Blue);
inline constexpr Color magenta      = Color(Colors::Magenta);
inline constexpr Color cyan         = Color(Colors::Cyan);
inline constexpr Color white        = Color(Colors::White);
inline constexpr Color blacklight   = Color(Colors::BlackLight);
inline constexpr Color redlight     = Color(Colors::RedLight);
inline constexpr Color greenlight   = Color(Colors::GreenLight);
inline constexpr Color yellowlight  = Color(Colors::YellowLight);
inline constexpr Color bluelight    = Color(Colors::BlueLight);
inline constexpr Color magentalight = Color(Colors::MagentaLight);
inline constexpr Color cyanlight    = Color(Colors::CyanLight);
inline constexpr Color whitelight   = Color(Colors::WhiteLight);

}   // namespace deco

#ifndef DECOTERM_NO_FORMAT

#include <format>

// ╔═════════════════════════════════════════════════════════╗
// ║                        formatter                        ║
// ╚═════════════════════════════════════════════════════════╝

namespace std {

template<>
struct formatter<deco::Style>{
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(deco::Style style, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", style.to_escape());
    }
};

};

#endif // !DECOTERM_NO_FORMAT


#endif  // !DECOTERM_HPP
