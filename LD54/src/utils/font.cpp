// font.cpp

#include "../awry/awry.h"
#include "font.hpp"

// static 
void bitmap_font_t::construct_monospaced_font(bitmap_font_t &font, 
                                              const point_t &character_count, 
                                              const point_t &character_size,
                                              const int first_codepoint)
{
   font.set_newline_spacing(character_size.y + 2);

   const int codepoint_count = character_count.x * character_count.y;
   for (int codepoint = 0; codepoint < codepoint_count; codepoint++) {
      const int x = (codepoint % character_count.x);
      const int y = (codepoint / character_count.x);

      bitmap_font_t::glyph_t glyph;
      glyph.m_codepoint = codepoint + first_codepoint;
      glyph.m_advance_x = character_size.x;
      glyph.m_source = { point_t{ x * character_size.x, y * character_size.y }, character_size };
      font.add_glyph(glyph);
   }

   font.sort();
}
