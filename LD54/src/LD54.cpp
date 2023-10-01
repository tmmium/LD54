// LD54.cpp

#include "LD54.hpp"

template <size_t N>
static void 
play_random_sound(sound_t(&sounds)[N], const float volume)
{
   int index = random_t::range_int(0, int(N - 1));
   sounds[index].play(volume);
}

application_t::application_t()
   : m_runtime(*runtime_t::ptr)
   , m_window(m_runtime.window())
   , m_graphics(m_runtime.graphics())
   , m_mouse(m_runtime.input().mouse())
   , m_keyboard(m_runtime.input().keyboard())
   , m_canvas_size({ 1920, 1080 })//{ 1280, 720 })
   , m_overlay(m_font)
{
   m_window.set_title("LD54: Limited Space");

   // todo: check desktop resolution and generate world from there.
   m_window.set_size(m_canvas_size);
   m_window.fullscreen();
   m_window_size = m_window.get_size();
   //m_canvas_size = m_window_size;
}

void application_t::run()
{
   if (!startup()) {
      return;
   }

   while (m_running) {
      pre_update();
      update();
      
      pre_render();
      render();
      post_render();
   }

   shutdown();
}

bool application_t::startup()
{
   mouse_t::hide_cursor();

   if (!m_sprite_tex.create_from_file("data/sprites.png", texture_t::filter_t::linear)) { 
      return false; 
   }

   m_font.set_texture(m_sprite_tex);
   bitmap_font_t::construct_monospaced_font(m_font, { 16, 6 }, { 8, 8 });

   { // note: splash screen settings and position
      m_splash_spr.set_texture(m_sprite_tex);
      m_splash_spr.set_source({ 0, 49, 288, 90 });
      m_splash_spr.set_destination({ 0, 0, 288, 90 });

      point_t splash_half_dims = point_t{ m_splash_spr.width() / 2, m_splash_spr.height() / 2 };
      vector2_t splash_canvas_position = (m_canvas_size / 2) - splash_half_dims;
      splash_canvas_position.y = float(m_canvas_size.y - m_splash_spr.height() * 5);
      m_splash_spr.set_position(splash_canvas_position);
   }

   { // note: tmmium sprite settings and position
      m_tmmium_spr.set_texture(m_sprite_tex);
      m_tmmium_spr.set_source({ 128, 0, 238, 14 });
      m_tmmium_spr.set_destination({ 0, 0, 238, 14 });

      point_t tmmium_dims = point_t{ m_tmmium_spr.width(), m_tmmium_spr.height() };
      vector2_t tmmium_canvas_position = { 0, m_canvas_size.y - tmmium_dims.y };
      m_tmmium_spr.set_position(tmmium_canvas_position);
   }

   m_starfield.randomize(m_canvas_size);
   m_solarsystem.randomize(m_canvas_size);
   m_spaceship.initialize(m_solarsystem.in_a_galaxy_far_far_away());

   return m_running;
}

void application_t::shutdown()
{
   m_sprite_tex.destroy();

   mouse_t::show_cursor();
}

void application_t::pre_update()
{
   m_runtime.input().update();
   if (!m_window.poll_events()) {
      m_running = false;
   }
}

void application_t::update()
{
   timespan_t now = timespan_t::time_since_start();
   m_frame_time = now - m_app_time;
   m_app_time = now;

   m_window_size = m_window.get_size();
   if (m_keyboard.released(keyboard_t::key_t::escape)) {
      m_running = false;
   }

   if (m_keyboard.pressed(keyboard_t::key_t::f1)) {
      m_overlay.toggle();
   }

   if (m_keyboard.pressed(keyboard_t::key_t::o)) {
      m_window.fullscreen();
   }

   m_solarsystem.update(m_frame_time);
   //if (m_keyboard.pressed(keyboard_t::key_t::space)) {
   //   m_solarsystem.randomize(m_canvas_size);
   //   m_spaceship.initialize(m_solarsystem.in_a_galaxy_far_far_away());
   //}

   vector2_t canvas_scale = vector2_t{ m_canvas_size } / vector2_t{ m_window_size };
   vector2_t scaled_mouse_position = m_mouse.scaled_position(canvas_scale);

   m_cursor.set_position(scaled_mouse_position);
   if (m_mouse.down(mouse_t::button_t::left)) {
      m_cursor.set_crosshair();
   }
   else {
      m_cursor.set_arrow();
   }

   //if (m_keyboard.pressed(keyboard_t::key_t::u)) {
   //   m_cursor.cycle();
   //}

   m_spaceship.direction(scaled_mouse_position - m_spaceship.m_position);
   m_spaceship.boost(m_keyboard.down(keyboard_t::key_t::left_shift));
   m_spaceship.accelerate(m_mouse.down(mouse_t::button_t::right));
   m_spaceship.m_gunturret.set_target(m_solarsystem.m_sun.m_position);
   m_spaceship.update(m_frame_time);
   m_spaceship.contain({ 0, 0, m_canvas_size });

   m_overlay.clear();
}

void application_t::pre_render()
{
   m_graphics.clear(space_background_color);
   m_graphics.projection(m_canvas_size);
}

void application_t::render()
{
   m_starfield.render(m_graphics);
   m_solarsystem.render(m_graphics);
   m_spaceship.render(m_graphics);
   m_cursor.render(m_graphics);

   m_tmmium_spr.render(m_graphics);
   m_splash_spr.render(m_graphics);

   m_spaceship.render(m_overlay);
   m_overlay.render(m_graphics);
}

void application_t::post_render()
{
   m_graphics.execute();
   m_window.swap_buffers();
}
