// spaceship.hpp

struct cargohold_t {
   static constexpr int slot_count = 2;

   cargohold_t() = default;

};

struct spawnicator_t {
   static constexpr int        wave_count = 3;
   static constexpr float      wave_radius = 50.0f;
   static constexpr timespan_t wave_lifetime = timespan_t::from_seconds(1.5f);

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

struct spacetrail_t {
   static constexpr int        trail_count = 32;
   static constexpr timespan_t trail_lifetime = timespan_t::from_seconds(0.5);
   static constexpr timespan_t trail_spawn_timer = timespan_t::from_seconds(0.033);
   static constexpr float      trail_base_size = 2.0f;
   static constexpr float      trail_grow_size = 4.0f;

   struct node_t {
      vector2_t  m_position;
      vector2_t  m_velocity;
      timespan_t m_lifetime;
      float      m_fade;
   };

   spacetrail_t() = default;

   bool active() const
   {
      return m_count > 0;
   }

   void push(const vector2_t &position, const vector2_t &velocity)
   {
      if (m_timer > timespan_t::zero()) {
         return;
      }

      m_timer = trail_spawn_timer;
      if (m_count == array_size(m_nodes)) {
         return;
      }

      if (m_count >= 1) {
         for (int index = m_count - 1; index >= 0; index--) {
            m_nodes[index + 1] = m_nodes[index];
         }
      }

      m_nodes[0].m_position = position;
      m_nodes[0].m_velocity = velocity;
      m_nodes[0].m_lifetime = trail_lifetime;
      m_count++;
   }

   void update(const timespan_t &deltatime)
   {
      constexpr float trail_drag_factor = 0.98f;

      m_timer -= deltatime;
      if (!active()) {
         return;
      }

      for (auto &node : m_nodes) {
         if (node.m_lifetime <= timespan_t::zero()) {
            continue;
         }

         node.m_position += node.m_velocity * deltatime.elapsed_seconds();
         node.m_velocity *= trail_drag_factor;
         node.m_lifetime -= deltatime;
         node.m_fade = node.m_lifetime.elapsed_seconds() / trail_lifetime.elapsed_seconds();
         if (node.m_lifetime <= timespan_t::zero()) {
            m_count--;
            assert(m_count >= 0);
         }
      }
   }

   void render(graphics_t &graphics, const vector2_t &center, const vector2_t &offset)
   {
      if (!active()) {
         return;
      }

      for (size_t index = 0; index < m_count; index++) {
         const node_t &node = m_nodes[index];
         const vector2_t position = (node.m_position - offset);
         const float size = trail_base_size + trail_grow_size * (1.0f - node.m_fade);
         graphics.draw_circle_filled(position, size, 5, color_t{}.fade(node.m_fade));
      }
   }

   timespan_t m_timer;
   int        m_count = 0;
   float      m_thickness = 3.0f;
   node_t     m_nodes[trail_count];
};

struct gunturret_t {
   gunturret_t() = default;

   void position(const vector2_t &position)
   {
      m_position = position;
   }

   void direction(const vector2_t &direction)
   {
      m_direction = direction;
   }

   void set_target(const vector2_t &position)
   {
      m_direction = (position - m_position).normalized();
   }

   void update(const timespan_t &deltatime)
   {
   }

   void render(graphics_t &graphics)
   {
      //graphics.draw_circle_outlined(m_position, 6.0f, 12, 2.0f, spaceship_outline_color);
      graphics.draw_line(m_position, m_position + m_direction * 7.0f, 2.0f, spaceship_turret_color);
   }

   vector2_t m_position;
   vector2_t m_direction;
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
      m_spacetrail.push(m_position, m_velocity);
      m_spacetrail.update(deltatime);
      //m_gunturret.position(m_position);
      //m_gunturret.update(deltatime);
   }

   void render(graphics_t &graphics)
   {
      const vector2_t center = m_position;
      const vector2_t normal = m_direction;
      const vector2_t tangent = normal.perp();

      // body
      const vector2_t b0 = center + normal * 10.0f;
      const vector2_t b1 = center - normal * 5.0f + tangent * 5.0f;
      const vector2_t b2 = center - normal * 5.0f - tangent * 5.0f;

      // right wing
      const vector2_t r0 = center;
      const vector2_t r1 = b1 - normal * 3.0f + tangent * 4.0f;
      const vector2_t r2 = b1 - normal * 3.0f - tangent * 4.0f;
       
      // left wing
      const vector2_t l0 = center;
      const vector2_t l1 = b2 - normal * 3.0f + tangent * 4.0f;
      const vector2_t l2 = b2 - normal * 3.0f - tangent * 4.0f;

      const vector2_t vertices[] =
      {
         r0, r1, r2,
         l0, l1, l2,
         b0, b1, b2,
      };

      m_spawnicator.render(graphics);
      m_spacetrail.render(graphics, m_position, m_direction * 5.0f);

      graphics.draw_triangles_filled(vertices, spaceship_fill_color);
      for (int index = 0; index < 3; index++) {
         auto span = std::span{ vertices + index * 3, 3 };
         graphics.draw_line_strip(span, 2.0f, spaceship_outline_color);
      }

      //m_gunturret.render(graphics);
   }

   void render(overlay_t &overlay)
   {
      overlay.draw_text_va(color_t{},
                           "%d,%d (vel: %1.3f acc: %2.3f drag: %2.3f)",
                           int(m_position.x),
                           int(m_position.y),
                           m_velocity.length(),
                           m_acceleration.length(),
                           m_drag.length());

      overlay.draw_text_va(color_t{},
                           "trail: %d",
                           m_spacetrail.m_count);
   }

   bool      m_boosting = false;
   vector2_t m_position;
   vector2_t m_direction;
   vector2_t m_acceleration;
   vector2_t m_velocity;
   vector2_t m_drag;

   gunturret_t   m_gunturret;
   spacetrail_t  m_spacetrail;
   cargohold_t   m_cargohold;
   spawnicator_t m_spawnicator;
};

