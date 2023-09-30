// overlay.hpp

#include <cstdarg>

struct overlay_t {
   struct line_t {
      point_t position;
      color_t color;
      std::string text;
   };

   overlay_t(bitmap_font_t &font)
      : m_font(font)
   {
   }

   void render(graphics_t &graphics)
   {
      if (!m_active) {
         return;
      }

      for (auto &line : m_lines) {
         m_font.render(graphics, line.position, line.text, line.color, m_scale);
      }
   }

   void add(const color_t &color, const char *format, ...)
   {
      char text[512] = {};
      va_list args;
      va_start(args, format);
      vsprintf_s(text, format, args);
      va_end(args);

      m_lines.emplace_back(m_position, color, std::string(text));
      m_position.y += m_font.m_newline_spacing * m_scale;
   }

   void toggle()
   {
      m_active = !m_active;
   }

   void set_scale(int scale)
   {
      m_scale = scale;
   }

   void set_origin(const point_t &origin)
   {
      m_origin = origin;
   }

   void clear()
   {
      m_lines.clear();
      m_position = m_origin;
   }

   bitmap_font_t      &m_font;
   bool                m_active = true;
   int                 m_scale = 1;
   point_t             m_origin = { 2, 2 };
   point_t             m_position = { 2, 2 };
   std::vector<line_t> m_lines;
};
