// cursor.hpp

struct cursor_t {
   enum class type_t {
      arrow,
      crosshair,
      disabled,
      count,
   };

   cursor_t() = default;

   // note: testing
   void cycle()
   {
      m_type = (type_t)((int(m_type) + 1) % int(type_t::count));
   }

   void set_arrow()
   {
      m_type = type_t::arrow;
   }

   void set_crosshair()
   {
      m_type = type_t::crosshair;
   }

   void set_disabled()
   {
      m_type = type_t::disabled;
   }

   void set_position(const vector2_t &position)
   {
      m_position = position;
   }

   void render(graphics_t &graphics)
   {
      constexpr float thickness = 2.0f;
      constexpr float radius = 12.0f;
      constexpr float radius_outer = radius * 0.7f;
      constexpr float radius_inner = radius * 0.4f;

      if (m_type == type_t::crosshair) {
         graphics.draw_line(m_position + vector2_t::down() * radius_inner, m_position + vector2_t::down() * radius_outer, thickness, cursor_inner_color);
         graphics.draw_line(m_position + vector2_t::up() * radius_inner, m_position + vector2_t::up() * radius_outer, thickness, cursor_inner_color);
         graphics.draw_line(m_position + vector2_t::left() * radius_inner, m_position + vector2_t::left() * radius_outer, thickness, cursor_inner_color);
         graphics.draw_line(m_position + vector2_t::right() * radius_inner, m_position + vector2_t::right() * radius_outer, thickness, cursor_inner_color);
         graphics.draw_circle_outlined(m_position, radius, 24, thickness, cursor_outer_color);
      }
      else if (m_type == type_t::arrow) {
         constexpr float arrow_length = 9.0f;
         constexpr float arrow_base_width = 7.0f;

         vector2_t bottom = m_position + vector2_t{ 1.0f, 2.0f } * arrow_length;
         vector2_t direction = (m_position - bottom).normalized();
         vector2_t base = direction.perp();
         vector2_t left = bottom + base * arrow_base_width;
         vector2_t right = bottom + -base * arrow_base_width;
         vector2_t points[] = { m_position, left, right };

         graphics.draw_triangles_filled(points, cursor_fill_color);
         graphics.draw_line(m_position, left, 2.0f, cursor_outer_color);
         graphics.draw_line(m_position, right, 2.0f, cursor_outer_color);
         graphics.draw_line(left, right, 2.0f, cursor_outer_color);
      }
      else if (m_type == type_t::disabled) {
         const vector2_t top_left = m_position - vector2_t{ -1.0f, -1.0f } * radius_inner;
         const vector2_t top_right = m_position - vector2_t{ 1.0f, -1.0f } * radius_inner;
         const vector2_t bottom_left = m_position - vector2_t{ -1.0f, 1.0f } * radius_inner;
         const vector2_t bottom_right = m_position - vector2_t{ 1.0f, 1.0f } * radius_inner;
         
         graphics.draw_line(top_left, bottom_right, thickness, cursor_disabled_color);
         graphics.draw_line(top_right, bottom_left, thickness, cursor_disabled_color);
         graphics.draw_circle_outlined(m_position, radius, 24, thickness, cursor_outer_color);
      }
   }

   type_t      m_type = {};
   vector2_t   m_position;
};
