// spaceship.hpp

struct spawnicator_t {
   static constexpr int        wave_count = 3;
   static constexpr float      wave_radius = 70.0f;
   static constexpr timespan_t wave_lifetime = timespan_t::from_seconds(1.0f);

   spawnicator_t() = default;

   void activate()
   {
      m_waves = wave_count;
   }

   void position(const vector2_t &position)
   {
      m_position = position;
   }

   void update(const timespan_t &deltatime)
   {
      if (m_waves == 0 && m_duration == timespan_t::zero()) {
         return;
      }

      m_duration += deltatime;
      if (m_duration >= wave_lifetime) {
         m_duration = timespan_t::zero();
         m_waves--;
      }
   }

   void render(graphics_t &graphics)
   {
      if (m_waves == 0) {
         return;
      }

      float alpha = m_duration.elapsed_seconds() / wave_lifetime.elapsed_seconds();
      float radius = alpha * wave_radius;
      graphics.draw_circle_outlined(m_position, radius, 32, 2.0f, spawnicator_outline_color.fade(1.0f - alpha));
   }

   vector2_t  m_position;
   timespan_t m_duration;
   int        m_waves = 0;
};

struct spaceship_t {
   // todo: move to propulsion
   static constexpr float drag_coefficient = 0.8f;
   static constexpr float propulsion_thrust = 150.0f;
   static constexpr float propulsion_boost_factor = 2.0f;
   static constexpr float propulsion_base_velocity = propulsion_thrust / drag_coefficient;
   static constexpr float propulsion_maximum_velocity = (propulsion_thrust * propulsion_boost_factor) / drag_coefficient;

   spaceship_t() = default;

   void initialize(const vector2_t &position, const vector2_t &direction = { 1.0f, 0.0f })
   {
      m_position = position;
      m_direction = direction;
      m_acceleration = {};
      m_velocity = {};
      m_spawnicator.activate();
   }

   void position(const vector2_t &position)
   {
      m_position = position;
   }

   void direction(const vector2_t &direction)
   {
      if (m_boosting) {
         return;
      }

      m_direction = direction.normalized();
   }

   void boost(const bool active)
   {
      m_boosting = active;
   }

   void accelerate(const bool active)
   {
      if (!m_boosting) {
         m_acceleration = m_direction * (active * spaceship_t::propulsion_thrust);
      }
      else {
         m_acceleration = m_direction * (active * spaceship_t::propulsion_thrust) * propulsion_boost_factor;
      }
   }

   void update(const timespan_t &deltatime)
   {
      const float dt = deltatime.elapsed_seconds();
      m_velocity += m_acceleration * dt - m_drag * dt;
      m_position += m_velocity * dt + m_acceleration * 0.5f * dt * dt;
      m_drag = m_velocity * spaceship_t::drag_coefficient;

      m_spawnicator.position(m_position);
      m_spawnicator.update(deltatime);
   }

   void render(graphics_t &graphics)
   {
      const vector2_t center = m_position;
      const vector2_t normal = m_direction;
      const vector2_t tangent = normal.perp();

      // body
      const vector2_t b0 = center + normal * 8.0f;
      const vector2_t b1 = center - normal * 4.0f + tangent * 4.0f;
      const vector2_t b2 = center - normal * 4.0f - tangent * 4.0f;

      const vector2_t vertices[] =
      {
         b0, b1, b2,
         //r0, r1, r2,
         //l0, l1, l2,
      };

      m_spawnicator.render(graphics);
      graphics.draw_triangles_filled(vertices, spaceship_fill_color);
      graphics.draw_line_strip(vertices, 2.0f, spaceship_outline_color);  
   }

   void render(overlay_t &overlay)
   {
      overlay.draw_text_va(color_black, 
                           "%d,%d (vel: %1.3f acc: %2.3f drag: %2.3f)",
                           int(m_position.x),
                           int(m_position.y),
                           m_velocity.length(),
                           m_acceleration.length(),
                           m_drag.length());
   }

   bool      m_boosting = false;
   vector2_t m_position;
   vector2_t m_direction;
   vector2_t m_acceleration;
   vector2_t m_velocity;
   vector2_t m_drag;

   spawnicator_t m_spawnicator;
};

