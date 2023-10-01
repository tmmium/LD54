// font.hpp

#include <algorithm>

struct bitmap_font_t {
   static void construct_monospaced_font(bitmap_font_t &font, 
                                         const point_t &character_count, 
                                         const point_t &character_size, 
                                         const int first_codepoint = 32);

   static constexpr uint32_t invalid_character_codepoint = '?';
   static constexpr uint32_t newline_character_codepoint = '\n';
   static constexpr uint32_t tab_character_codepoint = '\t';

   struct glyph_t {
      uint32_t    m_codepoint = 0;
      int         m_advance_x = 0;
      rectangle_t m_source;
   };

   bitmap_font_t() = default;
   bitmap_font_t(texture_t &texture)
      : m_texture(&texture)
   {
   }

   void render(graphics_t &graphics, const point_t &position, const std::string &text, const color_t &color, const int scale = 1)
   {
      point_t character_position = position;
      for (auto &ch : text) {
         uint32_t character_codepoint = static_cast<uint32_t>(uint8_t(ch));

         if (character_codepoint == newline_character_codepoint) {
            character_position.x = position.x;
            character_position.y += m_newline_spacing * scale;
            continue;
         }

         if (character_codepoint < m_first_valid_character_codepoint || 
             character_codepoint > m_last_valid_character_codepoint)
         {
            character_codepoint = invalid_character_codepoint;
         }

         glyph_t glyph;
         if (contains(character_codepoint, glyph)) {
            rectangle_t src = glyph.m_source;
            rectangle_t dest{ character_position, src.width_height() * scale };
            graphics.draw(*m_texture, src, dest, color);

            character_position.x += glyph.m_advance_x * scale;
         }
      }
   }

   point_t calculate_bounds(const std::string &text, const int scale = 1)
   {
      point_t result;
      for (auto &ch : text) {
         uint32_t character_codepoint = static_cast<uint32_t>(uint8_t(ch));

         if (character_codepoint == newline_character_codepoint) {
            result.y += m_newline_spacing * scale;
            continue;
         }

         if (character_codepoint < m_first_valid_character_codepoint ||
             character_codepoint > m_last_valid_character_codepoint)
         {
            character_codepoint = invalid_character_codepoint;
         }

         glyph_t glyph;
         if (contains(character_codepoint, glyph)) {
            result.x += glyph.m_advance_x * scale;
         }
      }

      return result;
   }

   template <typename T1, typename T2, typename ...Ts>
   bitmap_font_t &add_glyph(T1 t1, T2 t2, Ts ...ts)
   {
      add_glyph(t1);
      add_glyph(t2, ts...);
      return *this;
   }

   void add_glyph(glyph_t glyph)
   {
      if (m_last_valid_character_codepoint < glyph.m_codepoint) {
         m_last_valid_character_codepoint = glyph.m_codepoint;
      }
      if (m_first_valid_character_codepoint > glyph.m_codepoint) {
         m_first_valid_character_codepoint = glyph.m_codepoint;
      }

      m_glyphs.emplace_back(std::move(glyph));
   }

   void sort()
   {
      std::sort(m_glyphs.begin(), m_glyphs.end(), [](auto &lhs, auto &rhs) {
         return lhs.m_codepoint < rhs.m_codepoint;
      });
   }

   bool contains(const uint32_t codepoint, glyph_t &glyph) const
   {
      auto it = std::find_if(m_glyphs.begin(), m_glyphs.end(), [&](auto &rhs) {
         return rhs.m_codepoint == codepoint;
      });

      if (it == m_glyphs.end()) {
         return false;
      }

      glyph = *it;

      return true;
   }

   void set_newline_spacing(const int spacing)
   {
      m_newline_spacing = spacing;
   }

   void set_texture(texture_t &texture)
   {
      m_texture = &texture;
   }

   texture_t           *m_texture = nullptr;
   uint32_t             m_first_valid_character_codepoint = ~0u;
   uint32_t             m_last_valid_character_codepoint = 0;
   int                  m_newline_spacing = 12;
   std::vector<glyph_t> m_glyphs;
};

#if 0
struct monospace_font_t {
   monospace_font_t() = default;
   monospace_font_t(texture_t &texture)
      : m_texture(&texture)
   {
   }

   void set_newline_spacing(const int spacing)
   {
      m_newline_spacing = spacing;
   }

   void set_tab_spacing_factor(const int factor)
   {
      m_tab_spacing_factor = factor;
   }

   void set_character_count(const point_t &count)
   {
      m_character_count = count;
   }

   void set_glyph_size(const point_t &size)
   {
      m_glyph_size = size;
   }

   void set_texture(texture_t &texture)
   {
      m_texture = &texture;
   }

   void render(graphics_t &graphics, const point_t &position, const std::string &text, const color_t &color)
   {
      // note: constants
      const int character_column_count = m_character_count.x;
      const int character_row_count = m_character_count.y;
      const int character_width = m_glyph_size.x;
      const int character_height = m_glyph_size.y;

      const int newline_character_index = '\n';
      const int newline_advance_y = character_height + m_newline_spacing;

      const int tab_character_index = '\t';
      const int tab_advance_x = character_width * m_tab_spacing_factor;

      const int first_valid_character_index = ' ';
      const int last_valid_character_index = '~';
      const int invalid_character_index = '?';

      // note: render text
      point_t character_position = position;
      for (auto &ch : text) {
         int character_index = static_cast<int>(ch);

         if (character_index == newline_character_index) {
            character_position.x = position.x;
            character_position.y += newline_advance_y;
            continue;
         }
         else if (character_index == tab_character_index) {
            character_position.x += tab_advance_x;
            continue;
         }

         if (character_index < first_valid_character_index ||
             character_index > last_valid_character_index)
         {
            character_index = invalid_character_index;
         }

         rectangle_t dest{ character_position, m_glyph_size };
         rectangle_t src{ 0, 0, m_glyph_size };
         src.x = (character_index % character_column_count) * m_glyph_size.x;
         src.y = (character_index / character_column_count) * m_glyph_size.y;
         graphics.draw(*m_texture, src, dest, color);

         character_position.x += character_width;
      }
   }

   texture_t  *m_texture = nullptr;
   int         m_newline_spacing = 2;
   int         m_tab_spacing_factor = 4;
   point_t     m_character_count = { 16, 16 };
   point_t     m_glyph_size = { 8, 8 };
};
#endif
