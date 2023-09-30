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
   , m_canvas_size(m_runtime.get_desktop_size())
   , m_overlay(m_font)
{
   m_window.set_title("LD54: Limited Space");
   m_window.set_size(m_canvas_size);
   m_window.fullscreen();
   m_window_size = m_window.get_size();
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
   if (!m_font8_tex.create_from_file("data/font8x8.png")) { 
      return false; 
   }
   bitmap_font_t::construct_monospaced_font(m_font, m_font8_tex, { 16, 14 }, { 8, 8 });

   m_starfield.randomize(m_canvas_size);
   m_solarsystem.randomize(m_canvas_size);

   return m_running;
}

void application_t::shutdown()
{
   m_font8_tex.destroy();
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
   if (m_keyboard.pressed(keyboard_t::key_t::space)) {
      m_solarsystem.randomize(m_canvas_size);
   }

   m_overlay.clear();
}

void application_t::pre_render()
{
   m_graphics.clear(color_t{ 0xcd, 0xcd, 0xcd, 0xff });
   m_graphics.projection(m_canvas_size);
}

void application_t::render()
{
   m_starfield.render(m_graphics);
   m_solarsystem.render(m_graphics);
   m_overlay.render(m_graphics);
}

void application_t::post_render()
{
   m_graphics.execute();
   m_window.swap_buffers();
}