// awry.h

#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <numbers>
#include <span>

template <typename T, size_t N>
constexpr auto array_size(const T(&)[N]) { return N; }

template <typename T, size_t N>
constexpr auto max_index_of(const T(&)[N]) { return N - 1; }

struct math_t {
   static constexpr float kPI  = std::numbers::pi_v<float>;
   static constexpr float kPI2 = kPI * 2.0f;

   static float abs(float value);
   static float cosf(float value);
   static float sinf(float value);
   static float to_rad(float value);
   static float to_deg(float value);

   template <typename T>
   static constexpr T lerp(T a, T b, float t)
   {
      return T(a + (b - a) * t);
   }

   template <typename T>
   static constexpr T min(T lhs, T rhs) { return lhs < rhs ? lhs : rhs; }
   
   template <typename T>
   static constexpr T max(T lhs, T rhs) { return lhs > rhs ? lhs : rhs; }

   template <typename T>
   static constexpr T clamp(T value, T lo, T hi) { return min(hi, max(lo, value)); }
};

struct random_t {
   static float range01();
   static float range(float lo, float hi);
   static int   range_int(int lo, int hi);
};

struct point_t {
   constexpr point_t() = default;
   constexpr point_t(int x, int y) : x(x), y(y) {}
   template <typename U> constexpr point_t(U x, U y) : x(int(x)), y(int(y)) {}

   constexpr point_t operator+(const point_t &rhs) const { return { x + rhs.x, y + rhs.y }; }
   constexpr point_t operator-(const point_t &rhs) const { return { x - rhs.x, y - rhs.y }; }
   constexpr point_t operator*(const point_t &rhs) const { return { x * rhs.x, y * rhs.y }; }
   constexpr point_t operator/(const point_t &rhs) const { return { x / rhs.x, y / rhs.y }; }
   constexpr point_t operator*(const int rhs) const { return { x * rhs, y * rhs }; }
   constexpr point_t operator/(const int rhs) const { return { x / rhs, y / rhs }; }
   constexpr point_t operator*(const float rhs) const { return { int(x * rhs), int(y * rhs) }; }
   constexpr point_t operator/(const float rhs) const { return { int(x / rhs), int(y / rhs) }; }

   int x = 0;
   int y = 0;
};

struct rectangle_t {
   constexpr rectangle_t() = default;
   constexpr rectangle_t(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
   constexpr rectangle_t(int x, int y, point_t wh) : x(x), y(y), w(wh.x), h(wh.y) {}
   constexpr rectangle_t(point_t xy, point_t wh) : x(xy.x), y(xy.y), w(wh.x), h(wh.y) {}
   template <typename U> constexpr rectangle_t(U x, U y, U w, U h) : x(int(x)), y(int(y)), w(int(w)), h(int(h)) {}

   constexpr point_t top_left() const { return { x, y }; }
   constexpr point_t top_right() const { return { x, y + h }; }
   constexpr point_t bottom_right() const { return { x + w, y + h }; }
   constexpr point_t bottom_left() const { return { x + w, y }; }

   constexpr point_t width_height() const { return { w, h }; }

   constexpr bool overlap(const point_t &position)
   {
      bool x_overlap = (x <= position.x) && ((x + w) >= position.x);
      bool y_overlap = (y <= position.y) && ((y + h) >= position.y);
      return x_overlap && y_overlap;
   }

   int x = 0;
   int y = 0;
   int w = 0;
   int h = 0;
};

struct vector2_t {
   static constexpr vector2_t zero() { return vector2_t{ 0.0f, 0.0f }; }
   static constexpr vector2_t one() { return vector2_t{ 1.0f, 1.0f }; }
   static constexpr vector2_t left() { return vector2_t{ -1.0f, 0.0f }; }
   static constexpr vector2_t right() { return vector2_t{ 1.0f, 0.0f }; }
   static constexpr vector2_t up() { return vector2_t{ 0.0f, -1.0f }; }
   static constexpr vector2_t down() { return vector2_t{ 0.0f, 1.0f }; }

   static float distance(vector2_t &lhs, const vector2_t &rhs);
   static vector2_t random_direction();
   static vector2_t lerp(const vector2_t &a, const vector2_t &b, float t);

   constexpr vector2_t() = default;
   constexpr vector2_t(float x, float y) : x(x), y(y) {}
   constexpr vector2_t(const vector2_t &rhs) : x(rhs.x), y(rhs.y) {}
   constexpr vector2_t(const point_t &rhs) : x(float(rhs.x)), y(float(rhs.y)) {}
   template <typename U> constexpr vector2_t(U x, U y) : x(float(x)), y(float(y)) {}

   constexpr vector2_t operator=(const vector2_t &rhs) { x = rhs.x; y = rhs.y; return *this; }

   constexpr vector2_t operator-() const { return { -x, -y }; }
   constexpr vector2_t operator+(const vector2_t &rhs) const { return { x + rhs.x, y + rhs.y }; }
   constexpr vector2_t operator-(const vector2_t &rhs) const { return { x - rhs.x, y - rhs.y }; }
   constexpr vector2_t operator*(const vector2_t &rhs) const { return { x * rhs.x, y * rhs.y }; }
   constexpr vector2_t operator/(const vector2_t &rhs) const { return { x / rhs.x, y / rhs.y }; }
   constexpr vector2_t operator*(const float rhs) const { return { x * rhs, y * rhs }; }
   constexpr vector2_t operator/(const float rhs) const { return { x / rhs, y / rhs }; }
   constexpr vector2_t operator+(const point_t &rhs) const { return { x + rhs.x, y + rhs.y }; }
   constexpr vector2_t operator-(const point_t &rhs) const { return { x - rhs.x, y - rhs.y }; }
   constexpr vector2_t &operator+=(const vector2_t &rhs) { x += rhs.x; y += rhs.y; return *this; }
   constexpr vector2_t &operator-=(const vector2_t &rhs) { x -= rhs.x; y -= rhs.y; return *this; }
   constexpr vector2_t &operator*=(const vector2_t &rhs) { x *= rhs.x; y *= rhs.y; return *this; }
   constexpr vector2_t &operator/=(const vector2_t &rhs) { x /= rhs.x; y /= rhs.y; return *this; }
   constexpr vector2_t &operator*=(const float rhs) { x *= rhs; y *= rhs; return *this; }
   constexpr vector2_t &operator/=(const float rhs) { x /= rhs; y /= rhs; return *this; }

   constexpr vector2_t perp() const { return { y, -x }; }
   constexpr float length_squared() const { return x * x + y * y; }
   constexpr float dot(const vector2_t &rhs) const { return x * rhs.x + y * rhs.y; }

   constexpr point_t as_point() const { return point_t{ x, y }; }

   float length() const;
   float radians() const;
   vector2_t normalized() const;

   float x = 0.0f;
   float y = 0.0f;
};

struct vector3_t {
   constexpr vector3_t() = default;
   constexpr vector3_t(float x, float y, float z) : x(x), y(y), z(z) {}
   constexpr vector3_t(const vector2_t &rhs, float z) : x(rhs.x), y(rhs.y), z(z) {}
   template <typename U> constexpr vector3_t(U x, U y, U z) : x(float(x)), y(float(y)), z(float(z)) {}

   constexpr vector3_t operator-() const { return { -x, -y, -z }; }
   constexpr vector3_t operator+(const vector3_t &rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
   constexpr vector3_t operator-(const vector3_t &rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
   constexpr vector3_t operator*(const vector3_t &rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z }; }
   constexpr vector3_t operator/(const vector3_t &rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }
   constexpr vector3_t operator*(const float rhs) const { return { x * rhs, y * rhs, z * rhs }; }
   constexpr vector3_t operator/(const float rhs) const { return { x / rhs, y / rhs, z / rhs }; }

   constexpr float length_squared() const { return x * x + y * y + z * z; }
   constexpr float dot(const vector3_t &rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }
   constexpr vector2_t xy() const { return vector2_t{ x, y }; }

   float length() const;
   vector3_t normalized() const;

   float x = 0.0f;
   float y = 0.0f;
   float z = 0.0f;
};

struct matrix3_t {
   static matrix3_t from_transform(const vector2_t &position,
                                   const vector2_t &scale,
                                   const float rotation,
                                   const vector2_t &origin);

   static matrix3_t translate(const vector2_t &position);
   static matrix3_t rotate(const float radians);

   constexpr matrix3_t() = default;
   constexpr matrix3_t(vector3_t x, vector3_t y, vector3_t z) : x(x), y(y), z(z) {}

   constexpr vector2_t operator* (const vector2_t &rhs) const
   {
      const float xx = x.x * rhs.x + x.y * rhs.y + x.z;
      const float yy = y.x * rhs.x + y.y * rhs.y + y.z;

      return vector2_t{ xx, yy };
   }

   constexpr matrix3_t operator* (const matrix3_t &rhs) const
   {
      const float xx = x.x * rhs.x.x + x.y * rhs.y.x + x.z * rhs.z.x;
      const float xy = x.x * rhs.x.y + x.y * rhs.y.y + x.z * rhs.z.y;
      const float xz = x.x * rhs.x.z + x.y * rhs.y.z + x.z * rhs.z.z;

      const float yx = y.x * rhs.x.x + y.y * rhs.y.x + y.z * rhs.z.x;
      const float yy = y.x * rhs.x.y + y.y * rhs.y.y + y.z * rhs.z.y;
      const float yz = y.x * rhs.x.z + y.y * rhs.y.z + y.z * rhs.z.z;

      const float zx = z.x * rhs.x.x + z.y * rhs.y.x + z.z * rhs.z.x;
      const float zy = z.x * rhs.x.y + z.y * rhs.y.y + z.z * rhs.z.y;
      const float zz = z.x * rhs.x.z + z.y * rhs.y.z + z.z * rhs.z.z;

      return
      {
         { xx, xy, xz },
         { yx, yy, yz },
         { zx, zy, zz },
      };
   }

   vector3_t x = { 1.0f,0.0f,0.0f };
   vector3_t y = { 0.0f,1.0f,0.0f };
   vector3_t z = { 0.0f,0.0f,1.0f };
};

struct timespan_t {
   static constexpr timespan_t zero() { return timespan_t{ 0 }; }
   static constexpr timespan_t from_seconds(double seconds) { return timespan_t{ int64_t(seconds * 1000000.0) }; }
   static constexpr timespan_t from_milliseconds(double millis) { return timespan_t{ int64_t(millis * 1000.0) }; }

   static timespan_t time_since_start();

   constexpr timespan_t() = default;
   constexpr timespan_t(int64_t duration) : m_duration(duration) {}

   constexpr bool operator==(const timespan_t &rhs) const { return m_duration == rhs.m_duration; }
   constexpr bool operator!=(const timespan_t &rhs) const { return m_duration != rhs.m_duration; }
   constexpr bool operator< (const timespan_t &rhs) const { return m_duration < rhs.m_duration; }
   constexpr bool operator<=(const timespan_t &rhs) const { return m_duration <= rhs.m_duration; }
   constexpr bool operator> (const timespan_t &rhs) const { return m_duration > rhs.m_duration; }
   constexpr bool operator>=(const timespan_t &rhs) const { return m_duration >= rhs.m_duration; }

   constexpr timespan_t  operator*(const double rhs) const { return int64_t(m_duration * rhs); }
   constexpr timespan_t  operator+(const timespan_t &rhs) const { return { m_duration + rhs.m_duration }; }
   constexpr timespan_t  operator-(const timespan_t &rhs) const { return { m_duration - rhs.m_duration }; }
   constexpr timespan_t &operator+=(const timespan_t &rhs) { m_duration += rhs.m_duration; return *this; }
   constexpr timespan_t &operator-=(const timespan_t &rhs) { m_duration -= rhs.m_duration; return *this; }

   constexpr float elapsed_seconds() const { return float(m_duration / 1000000.0); }
   constexpr float elapsed_milliseconds() const { return float(m_duration / 1000.0); }
   constexpr int64_t elapsed_microseconds() const { return m_duration; }

   int64_t m_duration = 0;
};

struct sound_t {
   sound_t() = default;

   bool valid() const;
   bool create_from_file(const char *path);
   bool create_from_memory(const std::vector<uint8_t> &content);
   void destroy();

   void play(float volume);

   uint32_t m_id = 0;
};

struct color_t {
   constexpr color_t() = default;
   constexpr color_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

   constexpr color_t fade(const float value) const
   {
      const float clamp = (value > 1.0f ? 1.0f : value < 0.0f ? 0.0f : value);
      const uint8_t alpha = uint8_t(a * clamp);
      return color_t{ r, g, b, alpha };
   }

   uint8_t r = 0xff;
   uint8_t g = 0xff;
   uint8_t b = 0xff;
   uint8_t a = 0xff;
};

struct texture_t {
   enum class filter_t {
      nearest, linear,
   };

   enum class address_mode_t {
      clamp, wrap, mirror,
   };

   texture_t() = default;

   bool valid() const;
   bool create(const point_t &size, const void *data,
               const filter_t filter = filter_t::nearest,
               const address_mode_t address = address_mode_t::clamp);
   bool create_from_file(const char *path,
                         const filter_t filter = filter_t::nearest,
                         const address_mode_t address = address_mode_t::clamp);
   bool create_from_memory(const std::vector<uint8_t> &content,
                           const filter_t filter = filter_t::nearest,
                           const address_mode_t address = address_mode_t::clamp);
   void destroy();

   uint32_t m_id = 0;
   point_t  m_size;
};

struct zip_archive_t {
   struct zip_entry_t {
      std::string name;
      uint64_t hash = 0;
      uint64_t data_offset = 0;
      uint64_t size_compressed = 0;
      uint64_t size_uncompressed = 0;
   };

   zip_archive_t();
   ~zip_archive_t();

   bool valid() const;
   bool open(const std::string_view &path);
   void close();

   bool contains(const std::string_view &path) const;
   bool load_content(const std::string_view &path, std::vector<uint8_t> &content);

   void *m_handle;
   std::vector<zip_entry_t> m_entries;
};

struct native_window_t {
   virtual ~native_window_t() = default;
   virtual bool poll_events() = 0;
   virtual void swap_buffers() = 0;
   virtual void set_size(const point_t &size) = 0;
   virtual void set_title(const char *title) = 0;
   virtual void fullscreen() = 0;
   virtual point_t get_size() const = 0;
};

struct mouse_t {
   static void hide_cursor();
   static void show_cursor();

   enum class button_t {
      left,
      right,
      middle,
      count,
   };

   mouse_t() = default;

   void update();
   point_t position() const;
   point_t position_delta() const;
   point_t scroll_wheel_delta() const;
   point_t scaled_position(const vector2_t &scale);
   bool down(const button_t index) const;
   bool pressed(const button_t index) const;
   bool released(const button_t index) const;
   void on_mouse_move(const point_t &position);
   void on_mouse_wheel(const point_t &wheel_delta);
   void on_button_pressed(const button_t index);
   void on_button_released(const button_t index);

   point_t m_position;
   point_t m_position_delta;
   point_t m_scroll_wheel_delta;
   struct {
      uint8_t down : 1;
      uint8_t pressed : 1;
      uint8_t released : 1;
   } m_button[int(button_t::count)] = {};
};

struct keyboard_t {
   enum class key_t {
      backspace, tab, enter, caps_lock, escape, space, page_up, page_down,
      end, home, left, up, right, down, insert, delete_, digit0, digit1,
      digit2, digit3, digit4, digit5, digit6, digit7, digit8, digit9,
      a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v,
      w, x, y, z, left_os, right_os, f1, f2, f3, f4, f5, f6, f7, f8, f9,
      f10, f11, f12, num_lock, scroll_lock, left_shift, right_shift,
      left_control, right_control, left_menu, right_menu,
      count,
      unknown
   };

   keyboard_t() = default;

   void update();
   bool down(const key_t index) const;
   bool pressed(const key_t index) const;
   bool released(const key_t index) const;
   void on_key_pressed(const key_t index);
   void on_key_released(const key_t index);

   struct {
      uint8_t down : 1;
      uint8_t pressed : 1;
      uint8_t released : 1;
   } m_key[int(key_t::count)] = {};
};

struct input_context_t {
   input_context_t() = default;

   auto &mouse() { return m_mouse; }
   auto &keyboard() { return m_keyboard; }

   void update();
   void on_mouse_move(const point_t &position);
   void on_mouse_wheel(const point_t &wheel_delta);
   void on_button_pressed(const mouse_t::button_t index);
   void on_button_released(const mouse_t::button_t index);
   void on_key_pressed(const keyboard_t::key_t index);
   void on_key_released(const keyboard_t::key_t index);

   mouse_t m_mouse;
   keyboard_t m_keyboard;
};

struct graphics_t {
   virtual ~graphics_t() = default;
   virtual void clear(const color_t &color) = 0;
   virtual void projection(const vector2_t &projection) = 0;
   virtual void draw_rect_filled(const rectangle_t &dst, const color_t &color) = 0;
   virtual void draw_rect_filled(const rectangle_t &dst, const matrix3_t &transform, const color_t &color) = 0;
   virtual void draw_rect_outlined(const rectangle_t &dst, const float thickness, const color_t &color) = 0;
   virtual void draw_rect_outlined(const rectangle_t &dst, const float thickness, const matrix3_t &transform, const color_t &color) = 0;
   virtual void draw_circle_filled(const vector2_t &center, const float radius, const int steps, const color_t &color) = 0;
   virtual void draw_circle_filled(const vector2_t &center, const float radius, const int steps, const color_t &center_color, const color_t &outer_color) = 0;
   virtual void draw_circle_outlined(const vector2_t &center, const float radius, const int steps, const float thickness, const color_t &color) = 0;
   virtual void draw_circle_segment(const vector2_t &center, const float radius, const int steps, const float start_angle, const float end_angle, const color_t &color) = 0;
   virtual void draw_circle_segment(const vector2_t &center, const float radius, const int steps, const float thickness, const float start_angle, const float end_angle, const color_t &color) = 0;
   virtual void draw_line(const vector2_t &from, const vector2_t &to, const float thickness, const color_t &color) = 0;
   virtual void draw_line(const vector2_t &from, const vector2_t &to, const float thickness, const color_t &from_color, const color_t &to_color) = 0;
   virtual void draw_line_strip(const std::span<const vector2_t> positions, const float thickness, const color_t &color) = 0;
   virtual void draw_triangles_filled(const std::span<const vector2_t> positions, const color_t &color) = 0;
   virtual void draw(const texture_t &texture, const rectangle_t &src, const rectangle_t &dst, const color_t &color) = 0;
   virtual void draw(const texture_t &texture, const rectangle_t &src, const rectangle_t &dst, const matrix3_t &transform, const color_t &color) = 0;
   //virtual void draw_polygon_filled() = 0;
   //virtual void draw_polygon_outlined() = 0;
   virtual void execute() = 0;
};

struct runtime_t {
   static inline runtime_t *ptr = nullptr;

   runtime_t();
   ~runtime_t();

   auto &window()   { return *m_window; }
   auto &input()    { return *m_input; }
   auto &graphics() { return *m_graphics; }

   point_t get_desktop_size() const;

   native_window_t *m_window = nullptr;
   input_context_t *m_input = nullptr;
   graphics_t      *m_graphics = nullptr;
};
