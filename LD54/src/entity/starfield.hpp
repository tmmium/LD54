// startfield.hpp

struct starfield_t {
   static constexpr int   max_star_count = 512;
   static constexpr float star_alpha_variance = 0.8f;

   starfield_t() = default;

   void render(graphics_t &graphics)
   {
      for (star_t &star : m_stars) {
         graphics.draw_rect_filled(star.m_rect, star.m_color);
      }
   }

   void randomize(const point_t &size)
   {
      const float base_alpha = 1.0f - star_alpha_variance;
      for (star_t &star : m_stars) {
         star.m_color = color_t{}.fade(base_alpha + random_t::range01() * star_alpha_variance);
         star.m_rect = { { random_t::range_int(3, size.x - 3), random_t::range_int(3, size.y - 3)},  { 3, 3 } };
      }
   }

   struct star_t {
      color_t     m_color;
      rectangle_t m_rect;
   } m_stars[max_star_count];
};
