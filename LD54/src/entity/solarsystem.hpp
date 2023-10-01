// solarsystem.hpp



struct indicator_t {
   static constexpr timespan_t wave_lifetime = timespan_t::from_seconds(0.8f);
   static constexpr float      wave_radius_factor = 2.5f;
   static constexpr float      oscillator_speed_factor = 3.0f;

   indicator_t() = default;

   void activate(float radius, int wave_count = 2)
   {
      m_active = true;
      m_wave_count = wave_count;
      m_initial_radius = 0.0f;
      m_final_radius = radius * wave_radius_factor;
   }

   void deactivate()
   {
      m_active = false;
   }

   void position(const vector2_t &position)
   {
      m_position = position;
   }

   void update(const timespan_t &deltatime)
   {
      if (!m_active) {
         return;
      }

      m_duration += deltatime;
      //m_oscillator = math_t::abs(math_t::cosf(m_duration.elapsed_seconds() * oscillator_speed_factor));
      if (m_duration >= wave_lifetime) {
         m_duration = timespan_t::zero();
         m_wave_count--;
         if (m_wave_count == 0) {
            m_active = false;
         }
      }
   }

   void render(graphics_t &graphics)
   {
      if (!m_active) {
         return;
      }

      float alpha =  m_duration.elapsed_seconds() / wave_lifetime.elapsed_seconds();
      float radius = m_initial_radius + alpha * (m_final_radius - m_initial_radius);
      graphics.draw_circle_outlined(m_position, radius, 32, 2.0f, indicator_outline_color.fade(1.0f - alpha));
   }

   bool       m_active = false;
   int        m_wave_count = 0;
   vector2_t  m_position;
   timespan_t m_duration;
   float      m_oscillator = 0.0f;
   float      m_final_radius = 0.0f;
   float      m_initial_radius = 0.0f;
};

struct sun_t {
   static constexpr float radius = 45.0f;

   sun_t() = default;

   vector2_t m_position;
   float     m_radius = 0.0f;
};

struct planet_t {
   static constexpr float min_radius = 10.0f;
   static constexpr float max_radius = 20.0f;
   static constexpr float orbital_speed_factor = 10.0f;

   planet_t() = default;

   vector2_t m_position;
   float     m_radius = 0.0f;
   float     m_distance = 0.0f;
   float     m_speed = 0.0f;
   float     m_orbit = 0.0f;

   indicator_t m_indicator;
};

struct solarsystem_t {
   static constexpr int planet_count = 9;

   solarsystem_t() = default;

   void update(const timespan_t &deltatime)
   {
      const vector2_t center = m_sun.m_position;
      for (auto &planet : m_planets) {
         matrix3_t transform = matrix3_t::translate(center) * matrix3_t::rotate(planet.m_orbit);
         vector2_t position = transform * vector2_t{ planet.m_distance, 0.0f };

         planet.m_position = position;
         planet.m_orbit   += planet.m_speed * deltatime.elapsed_seconds();

         planet.m_indicator.position(planet.m_position);
         planet.m_indicator.update(deltatime);
      }
   }

   void render(graphics_t &graphics)
   {
      graphics.draw_circle_filled(m_sun.m_position, m_sun.m_radius * 1.5f, 48, sun_fill_color, sun_fill_color.fade(0.0f));
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
         planet.m_indicator.render(graphics);
         graphics.draw_circle_filled(planet.m_position, planet.m_radius, 32, planet_fill_color);
         graphics.draw_circle_outlined(planet.m_position, planet.m_radius, 32, 2.0f, planet_outline_color);
      }
   }

   void randomize(const point_t &size, const uint32_t seed = 1)
   {
      m_prng = { seed };

      const vector2_t center = size * 0.5f;
      const float system_radius = center.y * 0.99f;
      const float radius_step = system_radius / float(planet_count + 1);
      float radius = radius_step * 1.5f;

      m_sun.m_position = center;
      m_sun.m_radius = sun_t::radius;

      for (auto &planet : m_planets) {
         planet.m_orbit = m_prng.range01() * 360.0f;
         matrix3_t transform = matrix3_t::translate(center) * matrix3_t::rotate(planet.m_orbit);
         planet.m_position = transform * vector2_t{ radius, 0.0f };
         planet.m_radius = m_prng.range(planet_t::min_radius, planet_t::max_radius);
         planet.m_distance = vector2_t::distance(m_sun.m_position, planet.m_position);
         planet.m_speed = 1.0f + planet_t::orbital_speed_factor * (1.0f - (radius / system_radius));
         radius += radius_step;

         planet.m_indicator.activate(planet.m_radius);
      }
   }

   vector2_t in_a_galaxy_far_far_away() const 
   {
      //const int index_offset = 3;
      //vector2_t position = m_planets[planet_count - index_offset].m_position;
      //float     distance = m_planets[planet_count - index_offset].m_distance;
      //vector2_t direction = (m_sun.m_position - position).normalized();
      //return m_sun.m_position + direction * distance;
      return m_sun.m_position + m_sun.m_position * vector2_t::right() * 0.6f;
   }

   prng_t   m_prng;
   sun_t    m_sun;
   planet_t m_planets[planet_count];
};
