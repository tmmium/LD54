// colors.hpp

constexpr color_t color_white                = color_t{ 0xff, 0xff, 0xff, 0xff };
constexpr color_t color_black                = color_t{ 0x00, 0x00, 0x00, 0xff };
constexpr color_t cursor_fill_color          = color_t{ 0xad, 0xff, 0x02, 0xff };
constexpr color_t cursor_outer_color         = color_t{ 0x00, 0x30, 0x49, 0xff };
constexpr color_t cursor_inner_color         = color_t{ 0x00, 0x30, 0x49, 0xff };
constexpr color_t cursor_disabled_color      = color_t{ 0xd6, 0x28, 0x28, 0xff };
constexpr color_t space_background_color     = color_t{ 0xea, 0xe2, 0xb7, 0xff };
constexpr color_t sun_fill_color             = color_t{ 0xfc, 0xbf, 0x49, 0xff };
constexpr color_t planet_fill_color          = color_t{ 0xff, 0xff, 0xff, 0xff };
constexpr color_t planet_orbit_color         = color_t{ 0x00, 0x30, 0x49, 0xff }.fade(0.05f);
constexpr color_t planet_outline_color       = color_t{ 0x00, 0x30, 0x49, 0xff }; //color_t{ 0x8a, 0x8a, 0x8a, 0xff };
constexpr color_t spaceship_fill_color       = color_t{ 0xad, 0xff, 0x02, 0xff };
constexpr color_t spaceship_outline_color    = color_t{ 0x00, 0x30, 0x49, 0xff };
constexpr color_t spaceship_turret_color     = color_t{ 0xd6, 0x28, 0x28, 0xff };
constexpr color_t indicator_outline_color    = color_t{ 0x00, 0x30, 0x49, 0xff };
constexpr color_t spawnicator_outline_color  = color_t{ 0x00, 0x30, 0x49, 0xff };
