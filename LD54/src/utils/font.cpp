// font.cpp

#include "../awry/awry.h"
#include "font.hpp"

// static 
void bitmap_font_t::construct_monospaced_font(bitmap_font_t &font, texture_t &texture, 
                                              const point_t &character_count, 
                                              const point_t &character_size,
                                              const int first_codepoint)
{
   font.set_texture(texture);
   font.set_newline_spacing(character_size.y + 2);

   int count = character_count.x * character_count.y;
   for (int codepoint = first_codepoint; codepoint < count; codepoint++) {
      const int x = ((codepoint - first_codepoint) % character_count.x);
      const int y = ((codepoint - first_codepoint) / character_count.x);

      bitmap_font_t::glyph_t glyph;
      glyph.m_codepoint = codepoint;
      glyph.m_advance_x = character_size.x;
      glyph.m_source = { point_t{ x * character_size.x, y * character_size.y }, character_size };
      font.add_glyph(glyph);
   }

   font.sort();
}
