// solarsystem.hpp

struct sun_t {
   static constexpr float radius = 45.0f;

   sun_t() = default;

   vector2_t m_position;
   float     m_radius = 0.0f;
};

struct planet_t {
   static constexpr float min_radius = 4.0f;
   static constexpr float max_radius = 15.0f;
   static constexpr float orbital_speed_factor = 3.0f;

   planet_t() = default;

   vector2_t m_position;
   float     m_radius = 0.0f;
   float     m_distance = 0.0f;
   float     m_speed = 0.0f;
   float     m_orbit = 0.0f;
};

struct solarsystem_t {
   static constexpr int planet_count = 6;

   solarsystem_t() = default;

   void update(const timespan_t &frame_time)
   {
      const vector2_t center = m_sun.m_position;
      for (auto &planet : m_planets) {
         matrix3_t transform = matrix3_t::translate(center) * matrix3_t::rotate(planet.m_orbit);
         vector2_t position = transform * vector2_t{ planet.m_distance, 0.0f };

         planet.m_position = position;
         planet.m_orbit   += planet.m_speed * frame_time.elapsed_seconds();
      }
   }

   void render(graphics_t &graphics)
   {
      const color_t sun_fill_color = color_t{};
      const color_t planet_fill_color = color_t{};
      const color_t planet_orbit_color = color_t{}.fade(0.25f);
      const color_t planet_outline_color = color_t{ 0x8a, 0x8a, 0x8a, 0xff };

      graphics.draw_circle_filled(m_sun.m_position, m_sun.m_radius, 48, sun_fill_color);
      graphics.draw_circle_outlined(m_sun.m_position, m_sun.m_radius, 48, 2.0f, planet_outline_color);

      // orbit circles
      for (auto &planet : m_planets) {
         float distance = vector2_t::distance(m_sun.m_position, planet.m_position);
         graphics.draw_circle_outlined(m_sun.m_position, distance, 48, 1.0f, planet_orbit_color);
      }

      // planet orbits
      for (auto &planet : m_planets) {
         graphics.draw_circle_outlined(planet.m_position, planet.m_radius + planet.m_radius * 0.7f, 32, 1.0f, planet_orbit_color);
      }

      // planet bodies
      for (auto &planet : m_planets) {
         graphics.draw_circle_filled(planet.m_position, planet.m_radius, 32, planet_fill_color);
         graphics.draw_circle_outlined(planet.m_position, planet.m_radius, 32, 2.0f, planet_outline_color);
      }
   }

   void randomize(const point_t &size)
   {
      const vector2_t center = size * 0.5f;
      const float system_radius = center.y * 0.99f;
      const float radius_step = system_radius / float(planet_count + 1);
      float radius = radius_step * 1.5f;

      m_sun.m_position = center;
      m_sun.m_radius = sun_t::radius;

      for (auto &planet : m_planets) {
         planet.m_position = center + vector2_t::random_direction() * radius;
         planet.m_radius = random_t::range(planet_t::min_radius, planet_t::max_radius);
         planet.m_distance = vector2_t::distance(m_sun.m_position, planet.m_position);
         planet.m_speed = planet_t::orbital_speed_factor * (1.0f - (radius / system_radius));
         planet.m_orbit = random_t::range01() * 360.0f;
         radius += radius_step;
      }
   }

   sun_t    m_sun;
   planet_t m_planets[planet_count];
};
