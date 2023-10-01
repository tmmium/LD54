// aw_math.cpp

#include "awry.h"
#include <cmath>
#include <random>
#include <numbers>

namespace
{
   std::random_device                    g_random_device;
   std::uniform_real_distribution<float> g_uniform_distribution(0.0f, 1.0f);
} // !anon

// static 
float vector2_t::distance(vector2_t &lhs, const vector2_t &rhs)
{
   return (lhs - rhs).length();
}

vector2_t vector2_t::random_direction()
{
   float value = random_t::range01() * math_t::kPI2;
   return vector2_t{ math_t::cosf(value), math_t::sinf(value) };
}

vector2_t vector2_t::lerp(const vector2_t &a, const vector2_t &b, float t)
{
   return a + (b - a) * math_t::clamp(t, 0.0f, 1.0f);
}

float vector2_t::length() const
{
   return std::sqrtf(length_squared());
}

float vector2_t::radians() const
{
   return std::atan2(y, x);
}

vector2_t vector2_t::normalized() const
{
   float len = length();
   if (len > 0.0) {
      return *this / len;
   }
   return { 1.0f, 0.0f };
}

float vector3_t::length() const
{
   return std::sqrtf(length_squared());
}

vector3_t vector3_t::normalized() const
{
   float len = length();
   if (len > 0.0) {
      return *this / len;
   }
   return { 1.0f, 0.0f, 0.0f };
}

matrix3_t matrix3_t::from_transform(const vector2_t &position,
                                    const vector2_t &scale,
                                    const float rotation,
                                    const vector2_t &origin)
{
   const float angle = math_t::to_rad(rotation);
   const float c = math_t::cosf(angle);
   const float s = math_t::sinf(angle);
   const float sxc = scale.x * c;
   const float syc = scale.y * c;
   const float sxs = scale.x * s;
   const float sys = scale.y * s;
   const float tx = -origin.x * sxc - origin.y * sys + position.x;
   const float ty =  origin.x * sxs - origin.y * syc + position.y;

   return matrix3_t
   {
      {  sxc,  sys,   tx },
      { -sxs,  syc,   ty },
      { 0.0f, 0.0f, 1.0f },
   };
}

matrix3_t matrix3_t::translate(const vector2_t &position)
{
   return matrix3_t({ 1.0f, 0.0f, position.x }, { 0.0f, 1.0f, position.y }, { 0.0f, 0.0f, 1.0f });
}

matrix3_t matrix3_t::rotate(const float radians)
{
   const float angle = math_t::to_rad(radians);
   const float c = math_t::cosf(angle);
   const float s = math_t::sinf(angle);

   return matrix3_t
   {
      {    c,    s, 0.0f },
      {   -s,    c, 0.0f },
      { 0.0f, 0.0f, 1.0f },
   };
}

// static 
float math_t::abs(float value)
{
   return std::fabsf(value);
}

float math_t::cosf(float value)
{
   return std::cosf(value);
}

float math_t::sinf(float value)
{
   return std::sinf(value);
}

float math_t::to_rad(float value)
{
   constexpr float factor = kPI / 180.0f;
   return value * factor;
}

float math_t::to_deg(float value)
{
   constexpr float factor = 180.0f / kPI;
   return value * factor;
}

// static 
float random_t::range01()
{
   return g_uniform_distribution(g_random_device);
}

float random_t::range(float lo, float hi)
{
   return lo + (hi - lo) * range01();
}

int random_t::range_int(int lo, int hi)
{
   return int(range(float(lo), float(hi)) + 0.5f);
}

void mouse_t::update()
{
   m_position_delta = {};
   m_scroll_wheel_delta = {};
   for (auto &button : m_button) {
      button.pressed = false;
      button.released = false;
   }
}

point_t mouse_t::position() const
{
   return m_position;
}

point_t mouse_t::position_delta() const
{
   return m_position_delta;
}

point_t mouse_t::scroll_wheel_delta() const
{
   return m_scroll_wheel_delta;
}

point_t mouse_t::scaled_position(const vector2_t &scale)
{
   float x = m_position.x * scale.x;
   float y = m_position.y * scale.y;
   return point_t{ x, y };
}

bool mouse_t::down(const button_t index) const
{
   return m_button[int(index)].down;
}

bool mouse_t::pressed(const button_t index) const
{
   return m_button[int(index)].pressed;
}

bool mouse_t::released(const button_t index) const
{
   return m_button[int(index)].released;
}

void mouse_t::on_mouse_move(const point_t &position)
{
   m_position_delta = m_position - position;
   m_position = position;
}

void mouse_t::on_mouse_wheel(const point_t &wheel_delta)
{
   m_scroll_wheel_delta = wheel_delta;
}

void mouse_t::on_button_pressed(const button_t index)
{
   m_button[int(index)].down = true;
   m_button[int(index)].pressed = true;
   m_button[int(index)].released = false;
}

void mouse_t::on_button_released(const button_t index)
{
   m_button[int(index)].down = false;
   m_button[int(index)].pressed = false;
   m_button[int(index)].released = true;
}

void keyboard_t::update()
{
   for (auto &key : m_key) {
      key.pressed = false;
      key.released = false;
   }
}

bool keyboard_t::down(const key_t index) const
{
   return m_key[int(index)].down;
}

bool keyboard_t::pressed(const key_t index) const
{
   return m_key[int(index)].pressed;
}

bool keyboard_t::released(const key_t index) const
{
   return m_key[int(index)].released;
}

void keyboard_t::on_key_pressed(const key_t index)
{
   m_key[int(index)].down = true;
   m_key[int(index)].pressed = true;
   m_key[int(index)].released = false;
}

void keyboard_t::on_key_released(const key_t index)
{
   m_key[int(index)].down = false;
   m_key[int(index)].pressed = false;
   m_key[int(index)].released = true;
}

void input_context_t::update()
{
   m_mouse.update();
   m_keyboard.update();
}

void input_context_t::on_mouse_move(const point_t &position)
{
   m_mouse.on_mouse_move(position);
}

void input_context_t::on_mouse_wheel(const point_t &wheel_delta)
{
   m_mouse.on_mouse_wheel(wheel_delta);
}

void input_context_t::on_button_pressed(const mouse_t::button_t index)
{
   m_mouse.on_button_pressed(index);
}

void input_context_t::on_button_released(const mouse_t::button_t index)
{
   m_mouse.on_button_released(index);
}

void input_context_t::on_key_pressed(const keyboard_t::key_t index)
{
   m_keyboard.on_key_pressed(index);
}

void input_context_t::on_key_released(const keyboard_t::key_t index)
{
   m_keyboard.on_key_released(index);
}
