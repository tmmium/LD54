// sprite.hpp

struct sprite_t {
   sprite_t() = default;
   sprite_t(texture_t &texture)
      : m_texture(&texture)
      , m_source(0, 0, texture.m_size)
      , m_dest(0, 0, texture.m_size)
   {
   }

   void set_texture(texture_t &texture)
   {
      m_texture = &texture;
   }

   void set_source(const rectangle_t &source)
   {
      m_source = source;
   }

   void set_destination(const rectangle_t &dest)
   {
      m_dest = dest;
   }

   void set_color(const color_t &color)
   {
      m_color = color;
   }

   void set_position(const vector2_t &position)
   {
      m_dest = { position.as_point(), m_dest.width_height() };
   }

   void render(graphics_t &graphics)
   {
      graphics.draw(*m_texture, m_source, m_dest, m_color);
   }

   void render(graphics_t &graphics, const matrix3_t &transform)
   {
      graphics.draw(*m_texture, m_source, m_dest, transform, m_color);
   }

   texture_t  *m_texture = nullptr;
   rectangle_t m_source;
   rectangle_t m_dest;
   color_t     m_color;
};
