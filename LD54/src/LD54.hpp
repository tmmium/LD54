// LD54.hpp

#pragma once

#include "awry/awry.h"
#include "utils/font.hpp"
#include "utils/colors.hpp"
#include "utils/sprite.hpp"
#include "utils/overlay.hpp"
#include "entity/cursor.hpp"
#include "entity/warez.hpp"
#include "entity/starfield.hpp"
#include "entity/solarsystem.hpp"
#include "entity/spaceship.hpp"

class application_t final {
public:
   application_t();

   void run();

private:
   bool startup();
   void shutdown();

   void pre_update();
   void update();

   void pre_render();
   void render();
   void post_render();

private:
   bool             m_running = true;
   runtime_t       &m_runtime;
   native_window_t &m_window;
   graphics_t      &m_graphics;
   mouse_t         &m_mouse;
   keyboard_t      &m_keyboard;
   point_t          m_window_size;
   point_t          m_canvas_size;
   timespan_t       m_app_time;
   timespan_t       m_frame_time;

private:
   enum class state_t 
   {
      menu,
      play,
      over,
   };

   state_t          m_state = {};
   texture_t        m_sprite_tex;
   bitmap_font_t    m_font;
   overlay_t        m_overlay;
   float            m_splash_pulse = 0.0f;
   sprite_t         m_splash_spr;
   sprite_t         m_space_spr;
   sprite_t         m_tmmium_spr;
   sprite_t         m_instructions_spr;
   cursor_t         m_cursor;
   starfield_t      m_starfield;
   solarsystem_t    m_solarsystem;
   spaceship_t      m_spaceship;
};
