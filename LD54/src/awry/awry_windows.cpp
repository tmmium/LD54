// awry_windows.cpp

#include "awry.h"
#include <vector>
#include <cmath>
#include <numbers>
#include <Windows.h>
#include <windowsx.h>
#include <shellscalingapi.h>
#include <gl/GL.h>
#include <xaudio2.h>

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.h>
#include <stb_image.h>

// GL_VERSION_1_4
#define GL_MIRRORED_REPEAT                0x8370

// GL_VERSION_1_5
typedef unsigned long long GLsizeiptr;
typedef long long GLintptr;
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define OPENGL_FUNCTIONS \
   GL_FUNC(void, glGenBuffers, GLsizei n, GLuint *buffers) \
   GL_FUNC(void, glBindBuffer, GLenum target, GLuint buffer) \
   GL_FUNC(void, glDeleteBuffers, GLsizei n, const GLuint *buffers) \
   GL_FUNC(void, glBufferData, GLenum target, GLsizeiptr size, const void *data, GLenum usage) \
   GL_FUNC(void, glBufferSubData, GLenum target, GLintptr offset, GLsizeiptr size, const void *data)

#define GL_FUNC(ret, name, ...)                 \
   typedef ret APIENTRY type_##name (__VA_ARGS__); \
   type_##name *name; 

OPENGL_FUNCTIONS;
#undef GL_FUNC

#define WGL_DRAW_TO_WINDOW_ARB                     0x2001
#define WGL_ACCELERATION_ARB                       0x2003
#define WGL_SUPPORT_OPENGL_ARB                     0x2010
#define WGL_DOUBLE_BUFFER_ARB                      0x2011
#define WGL_PIXEL_TYPE_ARB                         0x2013
#define WGL_COLOR_BITS_ARB                         0x2014
#define WGL_DEPTH_BITS_ARB                         0x2022
#define WGL_STENCIL_BITS_ARB                       0x2023
#define WGL_FULL_ACCELERATION_ARB                  0x2027
#define WGL_TYPE_RGBA_ARB                          0x202B

#define WGL_SAMPLE_BUFFERS_ARB               0x2041
#define WGL_SAMPLES_ARB                      0x2042

#define WGL_CONTEXT_MAJOR_VERSION_ARB              0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB              0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB                0x2093
#define WGL_CONTEXT_FLAGS_ARB                      0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB               0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB                  0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB     0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB           0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB  0x00000002

typedef const char *(WINAPI type_wglGetExtensionsStringARB)(HDC hdc);
typedef BOOL(WINAPI type_wglChoosePixelFormatARB)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList,
                                                  UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC(WINAPI type_wglCreateContextAttribsARB)(HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL(WINAPI type_wglSwapIntervalEXT)(int interval);

static type_wglGetExtensionsStringARB *wglGetExtensionsStringARB;
static type_wglChoosePixelFormatARB *wglChoosePixelFormatARB;
static type_wglCreateContextAttribsARB *wglCreateContextAttribsARB;
static type_wglSwapIntervalEXT *wglSwapIntervalEXT;

constexpr DWORD kWindowStyle = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU);

extern int main(int argc, char **argv);

static void
win_fatal_error(const char *message)
{
   MessageBoxA(nullptr, message, "Error!", MB_OK | MB_ICONERROR);
   ExitProcess(1);
}

struct fake_context_t {
   fake_context_t()
   {
      // note: dummy window and context
      HWND dummy_window = CreateWindowA("STATIC",
                                        NULL, WS_POPUP | WS_DISABLED,
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        NULL,
                                        NULL,
                                        GetModuleHandleA(NULL),
                                        NULL);

      PIXELFORMATDESCRIPTOR pfd = { 0 };
      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.cColorBits = 32;
      pfd.cDepthBits = 24;
      pfd.cStencilBits = 8;
      pfd.iLayerType = PFD_MAIN_PLANE;

      HDC dummy_device = GetDC(dummy_window);
      int pixel_format_index = ChoosePixelFormat(dummy_device, &pfd);
      DescribePixelFormat(dummy_device, pixel_format_index, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
      int pixel_format = SetPixelFormat(dummy_device, pixel_format_index, &pfd);
      HGLRC dummy_context = wglCreateContext(dummy_device);
      wglMakeCurrent(dummy_device, dummy_context);

      // note: load windows context extensions
      wglGetExtensionsStringARB = (type_wglGetExtensionsStringARB *)wglGetProcAddress("wglGetExtensionsStringARB");
      wglChoosePixelFormatARB = (type_wglChoosePixelFormatARB *)wglGetProcAddress("wglChoosePixelFormatARB");
      wglCreateContextAttribsARB = (type_wglCreateContextAttribsARB *)wglGetProcAddress("wglCreateContextAttribsARB");
      wglSwapIntervalEXT = (type_wglSwapIntervalEXT *)wglGetProcAddress("wglSwapIntervalEXT");
      if (wglCreateContextAttribsARB == NULL) {
         win_fatal_error("Could not load OpenGL extensions!");
      }

      // note: destroy dummy window and context
      wglMakeCurrent(NULL, NULL);
      wglDeleteContext(dummy_context);
      DestroyWindow(dummy_window);
   }
};

timespan_t timespan_t::time_since_start()
{
   static LARGE_INTEGER start = {};
   static long long factor = 0;
   if (factor == 0) {
      QueryPerformanceCounter(&start);

      LARGE_INTEGER freq = {};
      QueryPerformanceFrequency(&freq);
      factor = freq.QuadPart / 1000000;
   }

   LARGE_INTEGER current = {};
   QueryPerformanceCounter(&current);

   long long diff = (current.QuadPart - start.QuadPart);

   return timespan_t{ diff / factor };
}

struct window_t final : native_window_t {
   window_t(HWND hWnd)
      : m_hWnd(hWnd)
      , m_hDC(GetDC(hWnd))
   {
   }

   bool poll_events()
   {
      MSG msg = {};
      while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
         if (msg.message == WM_QUIT) {
            return false;
         }

         TranslateMessage(&msg);
         DispatchMessageA(&msg);
      }

      return true;
   }

   void swap_buffers()
   {
      SwapBuffers(m_hDC);
   }

   void set_size(const point_t &size)
   {
      RECT window_rect = { 0, 0, size.x, size.y };
      DWORD style = GetWindowLongA(m_hWnd, GWL_STYLE);
      AdjustWindowRect(&window_rect, style, 0);

      int window_w = window_rect.right - window_rect.left;
      int window_h = window_rect.bottom - window_rect.top;
      int window_x = (GetSystemMetrics(SM_CXSCREEN) - window_w) / 2;
      int window_y = (GetSystemMetrics(SM_CYSCREEN) - window_h) / 2;

      SetWindowPos(m_hWnd,
                   nullptr,
                   window_x,
                   window_y,
                   window_w,
                   window_h,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
      glViewport(0, 0, size.x, size.y);
   }

   void set_title(const char *title)
   {
      SetWindowTextA(m_hWnd, title);
   }

   void fullscreen()
   {
      DWORD style = GetWindowLongA(m_hWnd, GWL_STYLE);
      if (style & kWindowStyle) {
         MONITORINFO info = { sizeof(MONITORINFO) };
         if (GetWindowPlacement(m_hWnd, &m_placement) && GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &info))
         {
            m_fullscreen = true;
            SetWindowLong(m_hWnd, GWL_STYLE, style & ~kWindowStyle);
            SetWindowPos(m_hWnd, HWND_TOP,
                         info.rcMonitor.left,
                         info.rcMonitor.top,
                         info.rcMonitor.right - info.rcMonitor.left,
                         info.rcMonitor.bottom - info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
         }
      }
      else {
         m_fullscreen = false;
         SetWindowLong(m_hWnd, GWL_STYLE, style | kWindowStyle);
         SetWindowPlacement(m_hWnd, &m_placement);
         SetWindowPos(m_hWnd, 0, 0, 0, 0, 0,
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                      SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
      }
      
      point_t size = get_size();
      glViewport(0, 0, size.x, size.y);
   }

   point_t get_size() const
   {
      RECT rect = {};
      GetClientRect(m_hWnd, &rect);
      return { rect.right, rect.bottom };
   }

   HWND m_hWnd = nullptr;
   HDC  m_hDC = nullptr;
   bool m_fullscreen = false;
   WINDOWPLACEMENT m_placement = {};
};

struct indexer_t {
   indexer_t() = default;
   indexer_t(uint32_t i) : m_gen(uint16_t(i >> 16)), m_index(uint16_t(i)) {}

   bool operator==(const indexer_t &rhs) const { return m_gen == rhs.m_gen && m_index == rhs.m_index; }

   uint32_t id()   { return (uint32_t(m_gen) << 16 | uint32_t(m_index)); }
   void     next() { m_gen++; }

   uint16_t m_gen = 1;
   uint16_t m_index = 0;
};

struct audio_buffer_t {
   indexer_t          handle;
   uint32_t           size = 0;
   std::vector<short> samples;
};

struct audio_source_t {
   bool                 active = false;
   IXAudio2SourceVoice *voice = nullptr;
   XAUDIO2_BUFFER       buffer = {};
};

struct audio_callback_t final : IXAudio2VoiceCallback
{
   void DECLSPEC_NOTHROW OnVoiceProcessingPassStart(UINT32 BytesRequired) { }
   void DECLSPEC_NOTHROW OnVoiceProcessingPassEnd() { }
   void DECLSPEC_NOTHROW OnStreamEnd() { }
   void DECLSPEC_NOTHROW OnBufferStart(void *pBufferContext) { }
   void DECLSPEC_NOTHROW OnBufferEnd(void *pBufferContext) { (*(audio_source_t *)pBufferContext).active = false; }
   void DECLSPEC_NOTHROW OnLoopEnd(void *pBufferContext) { }
   void DECLSPEC_NOTHROW OnVoiceError(void *pBufferContext, HRESULT Error) { }
};

struct audio_device_t {
   static inline audio_device_t *ptr = nullptr;

   static constexpr int max_buffer_count = 256;
   static constexpr int max_source_count = 64;
   static constexpr int channel_count = 2;
   static constexpr int sample_rate = 44100;
   static constexpr int bits_per_sample = 16;

   audio_device_t()
   {
      HRESULT hr = S_OK;
      hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
      if (FAILED(hr)) {
         win_fatal_error("CoInitializeEx failed!");
      }

      hr = XAudio2Create(&m_audio_device);
      if (FAILED(hr)) {
         win_fatal_error("XAudio2Create failed!");
      }

      hr = m_audio_device->CreateMasteringVoice(&m_master_voice, channel_count, sample_rate);
      if (FAILED(hr)) {
         win_fatal_error("CreateMasteringVoice failed!");
      }

      for (uint16_t index = 0; auto &buffer : m_buffers) {
         buffer.handle.m_index = index++;
      }

      const WAVEFORMATEX waveform =
      {
         .wFormatTag = WAVE_FORMAT_PCM,
         .nChannels = channel_count,
         .nSamplesPerSec = sample_rate,
         .nAvgBytesPerSec = channel_count * sample_rate * (bits_per_sample / 8),
         .nBlockAlign = 4,
         .wBitsPerSample = bits_per_sample,
         .cbSize = 0,
      };

      for (auto &source : m_sources) {
         source.buffer.Flags = XAUDIO2_END_OF_STREAM;
         source.buffer.pContext = &source;
         hr = m_audio_device->CreateSourceVoice(&source.voice,
                                                &waveform,
                                                0,
                                                XAUDIO2_DEFAULT_FREQ_RATIO,
                                                &m_callbacks,
                                                nullptr,
                                                nullptr);
         if (FAILED(hr)) {
            win_fatal_error("CreateSourceVoice failed!");
         }
      }

      audio_device_t::ptr = this;
   }

   ~audio_device_t()
   {
      audio_device_t::ptr = nullptr;
      m_audio_device->Release();
      CoUninitialize();
   }

   void play(indexer_t handle, float volume)
   {
      if (m_audio_device == nullptr) {
         return;
      }

      if (m_buffers[handle.m_index].handle != handle) {
         return;
      }

      if (m_buffers[handle.m_index].samples.empty()) {
         return;
      }

      audio_buffer_t &buffer = m_buffers[handle.m_index];
      for (auto &source : m_sources) {
         if (source.active) {
            continue;
         }

         source.active = true;
         source.buffer.AudioBytes = (UINT32)buffer.size;
         source.buffer.pAudioData = (BYTE *)buffer.samples.data();
         source.buffer.LoopCount = 0;
         source.voice->SubmitSourceBuffer(&source.buffer, nullptr);
         source.voice->Start(0, XAUDIO2_COMMIT_NOW);
         source.voice->SetVolume(volume, XAUDIO2_COMMIT_NOW);
         break;
      }
   }

   indexer_t create(std::vector<short> &&data)
   {
      for (auto &buffer : m_buffers) {
         if (buffer.samples.empty()) {
            buffer.size    = uint32_t(data.size() * sizeof(short));
            buffer.samples = std::move(data);
            return buffer.handle;
         }
      }

      return indexer_t{ 0 };
   }

   void destroy(indexer_t handle)
   {
      if (m_buffers[handle.m_index].handle != handle) {
         return;
      }

      if (m_buffers[handle.m_index].samples.empty()) {
         return;
      }

      m_buffers[handle.m_index].handle.next();
      m_buffers[handle.m_index].samples.clear();
   }

   IXAudio2               *m_audio_device = nullptr;
   IXAudio2MasteringVoice *m_master_voice = nullptr;
   audio_callback_t        m_callbacks;
   audio_buffer_t          m_buffers[max_buffer_count];
   audio_source_t          m_sources[max_source_count];
};

bool sound_t::valid() const
{
   return m_id != 0;
}

bool sound_t::create_from_file(const char *path)
{
   int error_code = 0;
   stb_vorbis *vorbis = stb_vorbis_open_filename(path, &error_code, nullptr);
   if (vorbis == nullptr) {
      return false;
   }

   stb_vorbis_info info = stb_vorbis_get_info(vorbis);
   uint32_t num_samples = stb_vorbis_stream_length_in_samples(vorbis) * info.channels;

   std::vector<short> samples(num_samples);
   stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, samples.data(), (int)samples.size());
   stb_vorbis_close(vorbis);

   m_id = audio_device_t::ptr->create(std::move(samples)).id();
   
   return valid();
}

bool sound_t::create_from_memory(const std::vector<uint8_t> &content)
{
   int error_code = 0;
   stb_vorbis *vorbis = stb_vorbis_open_memory(content.data(), int(content.size()), &error_code, nullptr);
   if (vorbis == nullptr) {
      return false;
   }

   stb_vorbis_info info = stb_vorbis_get_info(vorbis);
   uint32_t num_samples = stb_vorbis_stream_length_in_samples(vorbis) * info.channels;

   std::vector<short> samples(num_samples);
   stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, samples.data(), (int)samples.size());
   stb_vorbis_close(vorbis);

   m_id = audio_device_t::ptr->create(std::move(samples)).id();

   return valid();
}

void sound_t::destroy()
{
   if (valid()) {
      audio_device_t::ptr->destroy(indexer_t{ m_id });
   }

   m_id = 0;
}

void sound_t::play(float volume)
{
   if (!valid()) {
      return;
   }

   audio_device_t::ptr->play(indexer_t{ m_id }, volume);
}

#pragma pack(push, 1)
struct zip_local_header_t {
   uint32_t signature;
   uint16_t version;
   uint16_t bit_flags;
   uint16_t method;
   uint16_t last_mod_time;
   uint16_t last_mod_date;
   uint32_t crc32;
   uint32_t size_compressed;
   uint32_t size_uncompressed;
   uint16_t filename_length;
   uint16_t extra_field_length;
};

// note: central directory file header
struct zip_cdir_file_header_t {
   uint32_t signature;
   uint16_t version;
   uint16_t minimum_version;
   uint16_t bit_flags;
   uint16_t method;
   uint16_t last_mod_time;
   uint16_t last_mod_date;
   uint32_t crc32;
   uint32_t size_compressed;
   uint32_t size_uncompressed;
   uint16_t filename_length;
   uint16_t extra_field_length;
   uint16_t comment_length;
   uint16_t disk_num_file_start;
   uint16_t internal_file_attribs;
   uint32_t external_file_attribs;
   uint32_t local_header_offset;
};

// note: end of central directory record
struct zip_eocdir_record_t {
   uint32_t signature;
   uint16_t disk_count;
   uint16_t disk_cdir_start;
   uint16_t local_cdir_count;
   uint16_t cdir_record_count;
   uint32_t cdir_size;
   uint32_t cdir_offset;
   uint16_t comment_length;
};
#pragma pack(pop)

static constexpr uint32_t kZipCentralDirSignature = 0x02014B50u;
static constexpr uint32_t kZipLocalSignature = 0x04034B50u;
static constexpr uint32_t kZipEocdSignature = 0x06054B50u;

static uint64_t
fnv1a64(const char *str)
{
   uint64_t h = 14695981039346656037;
   while (*str != '\0') {
      h ^= uint64_t(str[0]);
      h *= 1099511628211;
      str++;
   }
   return h;
}

zip_archive_t::zip_archive_t()
   : m_handle(INVALID_HANDLE_VALUE)
{
}

zip_archive_t::~zip_archive_t()
{
   close();
}

bool zip_archive_t::valid() const
{
   return m_handle != INVALID_HANDLE_VALUE;
}

bool zip_archive_t::open(const std::string_view &path)
{
   m_handle = CreateFileA(path.data(),
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               nullptr,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               nullptr);
   if (m_handle == INVALID_HANDLE_VALUE) {
      return false;
   }

   LARGE_INTEGER size = {};
   if (GetFileSizeEx(m_handle, &size) == FALSE) {
      close();
      return false;
   }

   zip_eocdir_record_t eocd = {};
   LARGE_INTEGER position = { .QuadPart = size.QuadPart - LONGLONG(sizeof(eocd)) };
   SetFilePointerEx(m_handle, position, nullptr, FILE_BEGIN);
   ReadFile(m_handle, &eocd, sizeof(eocd), nullptr, nullptr);
   if (eocd.signature != kZipEocdSignature) {
      close();
      return false;
   }

   position = { .QuadPart = eocd.cdir_offset };
   SetFilePointerEx(m_handle, position, nullptr, FILE_BEGIN);
   for (uint32_t index = 0; index < eocd.cdir_record_count; index++) {
      zip_cdir_file_header_t cdir = {};
      ReadFile(m_handle, &cdir, sizeof(cdir), nullptr, nullptr);
      if (cdir.signature != kZipCentralDirSignature) {
         close();
         return false;
      }

      bool might_be_ascii = (cdir.internal_file_attribs & 0x1);
      std::string name(cdir.filename_length, '\0');
      ReadFile(m_handle, name.data(), DWORD(name.size()), nullptr, nullptr);

      zip_entry_t entry = {};
      entry.name = name;
      entry.hash = fnv1a64(name.c_str());
      entry.data_offset = cdir.local_header_offset + sizeof(zip_local_header_t) + cdir.filename_length;
      entry.size_compressed = cdir.size_compressed + (might_be_ascii ? 1 : 0);
      entry.size_uncompressed = cdir.size_uncompressed;
      m_entries.push_back(entry);
   }

   return valid();
}

void zip_archive_t::close()
{
   if (valid()) {
      CloseHandle(m_handle);
   }

   m_handle = INVALID_HANDLE_VALUE;
}

bool zip_archive_t::contains(const std::string_view &path) const
{
   if (!valid()) {
      return false;
   }

   const uint64_t hash = fnv1a64(path.data());
   for (auto &entry : m_entries) {
      if (entry.hash == hash) {
         return true;
      }
   }

   return false;
}

bool zip_archive_t::load_content(const std::string_view &path, std::vector<uint8_t> &content)
{
   if (!valid()) {
      return false;
   }

   const uint64_t hash = fnv1a64(path.data());
   for (auto &entry : m_entries) {
      if (entry.hash == hash) {
         std::vector<uint8_t> compressed;
         compressed.resize(entry.size_compressed);

         LARGE_INTEGER position = { .QuadPart = (LONGLONG)entry.data_offset };
         SetFilePointerEx(m_handle, position, nullptr, FILE_BEGIN);
         ReadFile(m_handle, compressed.data(), DWORD(compressed.size()), nullptr, nullptr);

         content.resize(entry.size_uncompressed);
         return stbi_zlib_decode_noheader_buffer((char *)content.data(),
                                                 int(content.size()),
                                                 (char *)compressed.data(),
                                                 int(compressed.size())) > 0;
      }
   }

   return true;
}

namespace
{
   inline float angle_diff(const float lhs, const float rhs)
   {
      return math_t::kPI - std::fabsf(std::fmod(rhs - lhs, math_t::kPI2) - math_t::kPI);
   }

   inline vector2_t from_angle(const float angle, const float length)
   {
      return vector2_t{ std::cosf(angle) * length, std::sinf(angle) * length };
   }
} // !anon

struct vertex_t {
   vector2_t position;
   vector2_t texcoord;
   color_t   color;
};

struct vertex_buffer_t {
   vertex_buffer_t() = default;

   bool valid() const
   {
      return id != 0;
   }

   bool create(uint64_t sz, const void *data)
   {
      destroy();

      size = sz;
      glGenBuffers(1, &id);
      glBindBuffer(GL_ARRAY_BUFFER, id);
      glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      if (glGetError() != GL_NO_ERROR) {
         destroy();
         return false;
      }

      return true;
   }

   void update(uint64_t sz, const void *data)
   {
      if (!valid()) {
         return;
      }

      glBindBuffer(GL_ARRAY_BUFFER, id);
      if (sz < size) {
         glBufferSubData(GL_ARRAY_BUFFER, 0, sz, data);
      }
      else {
         glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
         glBufferData(GL_ARRAY_BUFFER, sz, data, GL_STATIC_DRAW);
         size = sz;
      }

      GLenum error_code = glGetError();
      assert(error_code == GL_NO_ERROR);
   }

   void destroy()
   {
      if (valid()) {
         glDeleteBuffers(1, &id);
      }

      id = 0;
      size = 0;
   }

   uint32_t id = 0;
   uint64_t  size = 0;
};

struct command_t {
   uint32_t         count = 0;
   const texture_t *texture = nullptr;
};

struct gl_graphics_t final : graphics_t {
   gl_graphics_t()
   {
      uint32_t color = 0xffffffff;
      m_texture.create({ 1,1 }, &color);
      m_vertex_buffer.create(sizeof(vertex_t) * 8192, nullptr);
   }

   ~gl_graphics_t()
   {
      m_texture.destroy();
   }

   void clear(const color_t &color) 
   { 
      m_clear_color = color; 
   }

   void projection(const vector2_t &projection)
   {
      m_projection = { projection.x, projection.y };
   }

   void draw_rect_filled(const rectangle_t &dst, const color_t &color)
   {
      const vector2_t p0{ dst.x        , dst.y };
      const vector2_t p1{ dst.x + dst.w, dst.y };
      const vector2_t p2{ dst.x + dst.w, dst.y + dst.h };
      const vector2_t p3{ dst.x        , dst.y + dst.h };

      const vector2_t uv = { 0.5f, 0.5f };

      const vertex_t v0 = { p0, uv, color };
      const vertex_t v1 = { p1, uv, color };
      const vertex_t v2 = { p2, uv, color };
      const vertex_t v3 = { p3, uv, color };

      push(m_texture);
      push(v0, v1, v2, v3);
   }

   void draw_rect_filled(const rectangle_t &dst, const matrix3_t &transform, const color_t &color)
   {
      const vector2_t p0 = transform * vector2_t{ dst.x        , dst.y         };
      const vector2_t p1 = transform * vector2_t{ dst.x + dst.w, dst.y         };
      const vector2_t p2 = transform * vector2_t{ dst.x + dst.w, dst.y + dst.h };
      const vector2_t p3 = transform * vector2_t{ dst.x        , dst.y + dst.h };

      const vector2_t uv = { 0.5f, 0.5f };

      const vertex_t v0 = { p0, uv, color };
      const vertex_t v1 = { p1, uv, color };
      const vertex_t v2 = { p2, uv, color };
      const vertex_t v3 = { p3, uv, color };

      push(m_texture);
      push(v0, v1, v2, v3);
   }

   void draw_rect_outlined(const rectangle_t &dst, const float thickness, const color_t &color)
   {
      const float pxy = thickness * 0.5f;
      const float px0 = float(dst.x);
      const float px1 = float(dst.x + dst.w);
      const float py0 = float(dst.y);
      const float py1 = float(dst.y + dst.h);

      draw_line(vector2_t{ px0 - pxy, py0 }, vector2_t{ px1 + pxy, py0 }, thickness, color);
      draw_line(vector2_t{ px1, py0 - pxy }, vector2_t{ px1, py1 + pxy }, thickness, color);
      draw_line(vector2_t{ px1 + pxy, py1 }, vector2_t{ px0 - pxy, py1 }, thickness, color);
      draw_line(vector2_t{ px0, py1 + pxy }, vector2_t{ px0, py0 - pxy }, thickness, color);
   }

   void draw_rect_outlined(const rectangle_t &dst, const float thickness, const matrix3_t &transform, const color_t &color)
   {
      const float pxy = thickness * 0.5f;
      const float px0 = float(dst.x);
      const float px1 = float(dst.x + dst.w);
      const float py0 = float(dst.y);
      const float py1 = float(dst.y + dst.h);

      const vector2_t p0 = transform * vector2_t{ dst.top_left() };
      const vector2_t p1 = transform * vector2_t{ dst.top_right() };
      const vector2_t p2 = transform * vector2_t{ dst.bottom_right() };
      const vector2_t p3 = transform * vector2_t{ dst.bottom_left() };

      draw_line(p0, p1, thickness, color);
      draw_line(p1, p2, thickness, color);
      draw_line(p2, p3, thickness, color);
      draw_line(p3, p0, thickness, color);
   }

   void draw_circle_filled(const vector2_t &center, const float radius, const int steps, const color_t &color)
   {
      const vector2_t uv = { 0.5f, 0.5f };
      const vector2_t p0 = center;
      vector2_t p1 = center + vector2_t{ radius, 0.0 };

      push(m_texture);
      for (int i = 1; i <= steps; i++) {
         const float theta = float(i) / float(steps) * math_t::kPI2;
         const vector2_t p2{ center.x + std::cosf(theta) * radius, center.y + std::sinf(theta) * radius };

         const vertex_t v0 = { p0, uv, color };
         const vertex_t v1 = { p1, uv, color };
         const vertex_t v2 = { p2, uv, color };
         push(v0, v1, v2);

         p1 = p2;
      }
   }

   void draw_circle_filled(const vector2_t &center, const float radius, const int steps, const color_t &center_color, const color_t &outer_color)
   {
      const vector2_t uv = { 0.5f, 0.5f };
      const vector2_t p0 = center;
      vector2_t p1 = center + vector2_t{ radius, 0.0f };

      push(m_texture);
      for (int i = 1; i <= steps; i++) {
         const float theta = float(i) / float(steps) * math_t::kPI2;
         const vector2_t p2{ center.x + std::cosf(theta) * radius, center.y + std::sinf(theta) * radius };

         const vertex_t v0 = { p0, uv, center_color };
         const vertex_t v1 = { p1, uv, outer_color };
         const vertex_t v2 = { p2, uv, outer_color };

         push(v0, v1, v2);

         p1 = p2;
      }
   }

   void draw_circle_outlined(const vector2_t &center, const float radius_outer, const int steps, const float thickness, const color_t &color)
   {
      const float radius_inner = radius_outer - thickness;
      vector2_t p1_outer = center + vector2_t{ radius_outer, 0.0 };
      vector2_t p1_inner = center + vector2_t{ radius_inner, 0.0 };

      const vector2_t uv = { 0.5f, 0.5f };

      push(m_texture);
      for (int i = 1; i <= steps; i++) {
         const float theta = float(i) / float(steps) * math_t::kPI2;
         const vector2_t normal{ std::cosf(theta), std::sinf(theta) };

         const vector2_t p2_outer{ center.x + normal.x * radius_outer, center.y + normal.y * radius_outer };
         const vector2_t p2_inner{ center.x + normal.x * radius_inner, center.y + normal.y * radius_inner };

         const vertex_t v0 = { p1_inner, uv, color };
         const vertex_t v1 = { p1_outer, uv, color };
         const vertex_t v2 = { p2_outer, uv, color };
         const vertex_t v3 = { p2_inner, uv, color };

         push(v0, v1, v2, v3);

         p1_outer = p2_outer;
         p1_inner = p2_inner;
      }
   }

   void draw_circle_segment(const vector2_t &center, const float radius, const int steps, const float start_angle,
                                        const float end_angle, const color_t &color)
   {
      const float theta = angle_diff(start_angle, end_angle);

      const vector2_t uv = { 0.5f, 0.5f };
      const vector2_t p0 = center;
      vector2_t p1 = center + from_angle(start_angle, radius);

      push(m_texture);
      for (int i = 1; i <= steps; i++) {
         const float step = i / float(steps);
         const float angle = start_angle + theta * step;

         vector2_t p2 = center + from_angle(angle, radius);

         const vertex_t v0 = { p0, uv, color };
         const vertex_t v1 = { p1, uv, color };
         const vertex_t v2 = { p2, uv, color };

         push(v0, v1, v2);

         p1 = p2;
      }
   }

   void draw_circle_segment(const vector2_t &center, const float radius_outer, const int steps, const float thickness,
                                        const float start_angle, const float end_angle, const color_t &color)
   {
      const float radius_inner = radius_outer - thickness;
      const float theta = angle_diff(start_angle, end_angle);

      const vector2_t uv = { 0.5f, 0.5f };
      vector2_t p1_outer = from_angle(start_angle, radius_outer);
      vector2_t p1_inner = from_angle(start_angle, radius_inner);

      push(m_texture);
      for (int i = 1; i <= steps; i++) {
         const float step = i / float(steps);
         const float angle = start_angle + theta * step;

         vector2_t p2_outer = from_angle(angle, radius_outer);
         vector2_t p2_inner = from_angle(angle, radius_inner);

         const vertex_t v0 = { center + p1_inner, uv, color };
         const vertex_t v1 = { center + p1_outer, uv, color };
         const vertex_t v2 = { center + p2_outer, uv, color };
         const vertex_t v3 = { center + p2_inner, uv, color };

         push(v0, v1, v2, v3);

         p1_outer = p2_outer;
         p1_inner = p2_inner;
      }
   }

   void draw_line(const vector2_t &from, const vector2_t &to, const float thickness, const color_t &color)
   {
      const vector2_t perp = (to - from).normalized().perp();
      const vector2_t disp = perp * thickness * 0.5f;
      const vector2_t p0 = from + disp;
      const vector2_t p1 = to + disp;
      const vector2_t p2 = to - disp;
      const vector2_t p3 = from - disp;

      const vector2_t uv = { 0.5f, 0.5f };

      const vertex_t v0 = { p0, uv, color };
      const vertex_t v1 = { p1, uv, color };
      const vertex_t v2 = { p2, uv, color };
      const vertex_t v3 = { p3, uv, color };

      push(m_texture);
      push(v0, v1, v2, v3);
   }

   void draw_line(const vector2_t &from, const vector2_t &to, const float thickness, const color_t &from_color,
                              const color_t &to_color)
   {
      const vector2_t perp = (to - from).normalized().perp();
      const vector2_t disp = perp * thickness * 0.5f;
      const vector2_t p0 = from + disp;
      const vector2_t p1 = to + disp;
      const vector2_t p2 = to - disp;
      const vector2_t p3 = from - disp;

      const vector2_t uv = { 0.5f, 0.5f };

      const vertex_t v0 = { p0, uv, from_color };
      const vertex_t v1 = { p1, uv, to_color };
      const vertex_t v2 = { p2, uv, to_color };
      const vertex_t v3 = { p3, uv, from_color };

      push(m_texture);
      push(v0, v1, v2, v3);
   }

   void draw_line_strip(const std::span<const vector2_t> positions, const float thickness, const color_t &color)
   {
      push(m_texture);
      for (size_t index = 0; index < positions.size(); index++) {
         auto from = positions[index];
         auto to = positions[(index + 1) % positions.size()];

         const vector2_t perp = (to - from).normalized().perp();
         const vector2_t disp = perp * thickness * 0.5f;
         const vector2_t p0 = from + disp;
         const vector2_t p1 = to + disp;
         const vector2_t p2 = to - disp;
         const vector2_t p3 = from - disp;

         const vector2_t uv = { 0.5f, 0.5f };

         const vertex_t v0 = { p0, uv, color };
         const vertex_t v1 = { p1, uv, color };
         const vertex_t v2 = { p2, uv, color };
         const vertex_t v3 = { p3, uv, color };

         push(v0, v1, v2, v3);
      }
   }

   void draw_triangles_filled(const std::span<const vector2_t> positions, const color_t &color)
   {
      assert(positions.size() % 3 == 0);

      const vector2_t uv = { 0.5f, 0.5f };

      push(m_texture);
      for (size_t index = 0; index < positions.size(); index += 3) {
         const vertex_t v0 = { positions[index + 0], uv, color };
         const vertex_t v1 = { positions[index + 1], uv, color };
         const vertex_t v2 = { positions[index + 2], uv, color };
         push(v0, v1, v2);
      }
   }

   void draw(const texture_t &texture, const rectangle_t &src, const rectangle_t &dst, const color_t &color)
   {
      const float iu = 1.0f / texture.m_size.x;
      const float iv = 1.0f / texture.m_size.y;

      const vector2_t p0{ dst.x        , dst.y };
      const vector2_t p1{ dst.x + dst.w, dst.y };
      const vector2_t p2{ dst.x + dst.w, dst.y + dst.h };
      const vector2_t p3{ dst.x        , dst.y + dst.h };

      const vector2_t t0{ (src.x) * iu        , (src.y) * iv };
      const vector2_t t1{ (src.x + src.w) * iu, (src.y) * iv };
      const vector2_t t2{ (src.x + src.w) * iu, (src.y + src.h) * iv };
      const vector2_t t3{ (src.x) * iu        , (src.y + src.h) * iv };

      const vertex_t v0 = { p0, t0, color };
      const vertex_t v1 = { p1, t1, color };
      const vertex_t v2 = { p2, t2, color };
      const vertex_t v3 = { p3, t3, color };

      push(texture);
      push(v0, v1, v2, v3);
   }

   void draw(const texture_t &texture, const rectangle_t &src, const rectangle_t &dst, const matrix3_t &transform,
                         const color_t &color)
   {

      const float iu = 1.0f / texture.m_size.x;
      const float iv = 1.0f / texture.m_size.y;

      const vector2_t p0 = transform * vector2_t{ 0    , 0 };
      const vector2_t p1 = transform * vector2_t{ dst.w, 0 };
      const vector2_t p2 = transform * vector2_t{ dst.w, dst.h };
      const vector2_t p3 = transform * vector2_t{ 0    , dst.h };

      const vector2_t t0{ (src.x) * iu        , (src.y) * iv };
      const vector2_t t1{ (src.x + src.w) * iu, (src.y) * iv };
      const vector2_t t2{ (src.x + src.w) * iu, (src.y + src.h) * iv };
      const vector2_t t3{ (src.x) * iu        , (src.y + src.h) * iv };

      const vertex_t v0 = { p0, t0, color };
      const vertex_t v1 = { p1, t1, color };
      const vertex_t v2 = { p2, t2, color };
      const vertex_t v3 = { p3, t3, color };

      push(texture);
      push(v0, v1, v2, v3);
   }

   void execute()
   {
      glClearColor(m_clear_color.r / 255.0f,
                   m_clear_color.g / 255.0f,
                   m_clear_color.b / 255.0f,
                   m_clear_color.a / 255.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      if (m_commands.empty()) {
         m_vertices.clear();
         return;
      }

      {
         const float xx = 2.0f / float(m_projection.x);
         const float yy = 2.0f / -float(m_projection.y);
         const float zz = 1.0f / 2.0f;
         const float wx = -1.0f;
         const float wy = 1.0f;
         const float wz = 0.5f;

         float orthographic[16] =
         {
              xx, 0.0f, 0.0f, 0.0f,
            0.0f,   yy, 0.0f, 0.0f,
            0.0f, 0.0f,   zz, 0.0f,
              wx,   wy,   wz, 1.0f,
         };

         glMatrixMode(GL_PROJECTION);
         glLoadIdentity();
         glLoadMatrixf(orthographic);
         glMatrixMode(GL_MODELVIEW);
         glLoadIdentity();
      }

      glDisable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);

      m_vertex_buffer.update(sizeof(vertex_t) * m_vertices.size(), m_vertices.data());
      glVertexPointer(2, GL_FLOAT, sizeof(vertex_t), (const GLvoid *)offsetof(vertex_t, position));
      glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (const GLvoid *)offsetof(vertex_t, texcoord));
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex_t), (const GLvoid *)offsetof(vertex_t, color));

      GLint offset = 0;
      for (auto &command : m_commands) {
         glBindTexture(GL_TEXTURE_2D, command.texture->m_id);
         glDrawArrays(GL_TRIANGLES, offset, command.count);
         offset += command.count;
      }

      assert(glGetError() == GL_NO_ERROR);

      m_vertices.clear();
      m_commands.clear();
   }

   void push(const texture_t &texture)
   {
      assert(texture.valid());
      if (m_commands.empty()) {
         m_commands.emplace_back(0, std::addressof(texture));
      }
      else {
         if (m_commands.back().texture != std::addressof(texture)) {
            m_commands.emplace_back(0, std::addressof(texture));
         }
      }
   }

   void push(vertex_t v0, vertex_t v1, vertex_t v2)
   {
      m_commands.back().count += 3;
      m_vertices.insert(m_vertices.end(), { v0, v1, v2 });
   }

   void push(vertex_t v0, vertex_t v1, vertex_t v2, vertex_t v3)
   {
      m_commands.back().count += 6;
      m_vertices.insert(m_vertices.end(), { v0, v1, v2, v2, v3, v0 });
   }

   texture_t m_texture;
   color_t m_clear_color;
   vector2_t m_projection;
   vertex_buffer_t m_vertex_buffer;
   std::vector<vertex_t> m_vertices;
   std::vector<command_t> m_commands;
};

bool texture_t::valid() const
{
   return m_id != 0;
}

bool texture_t::create(const point_t &dim, const void *data,
                       const filter_t filter,
                       const address_mode_t address)
{
   destroy();

   m_size = dim;

   GLenum gl_filter = filter == filter_t::nearest ? GL_NEAREST : GL_LINEAR;
   GLenum gl_address = GL_CLAMP;
   if (address == address_mode_t::wrap) {
      gl_address = GL_REPEAT;
   }
   else if (address == address_mode_t::mirror) {
      gl_address = GL_MIRRORED_REPEAT;
   }

   glGenTextures(1, &m_id);
   glBindTexture(GL_TEXTURE_2D, m_id);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_address);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_address);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
   if (glGetError() != GL_NO_ERROR) {
      destroy();
      return false;
   }

   return true;
}

bool texture_t::create_from_file(const char *path, const filter_t filter, const address_mode_t address)
{
   int x = 0, y = 0, c = 0;
   auto b = stbi_load(path, &x, &y, &c, STBI_rgb_alpha);
   if (b == nullptr) {
      return false;
   }

   create({ x, y }, b, filter, address);
   stbi_image_free(b);

   return valid();
}

bool texture_t::create_from_memory(const std::vector<uint8_t> &content,
                                   const filter_t filter,
                                   const address_mode_t address)
{
   int x = 0, y = 0, c = 0;
   auto b = stbi_load_from_memory(content.data(), int(content.size()), &x, &y, &c, STBI_rgb_alpha);
   if (b == nullptr) {
      return false;
   }

   create({ x, y }, b, filter, address);
   stbi_image_free(b);

   return valid();
}

void texture_t::destroy()
{
   if (valid()) {
      glDeleteTextures(1, &m_id);
   }

   m_id = 0;
   m_size = {};
}

//static 
void mouse_t::hide_cursor()
{
   ShowCursor(FALSE);
}

void mouse_t::show_cursor()
{
   ShowCursor(TRUE);
}

runtime_t::runtime_t()
{
   runtime_t::ptr = this;
}

runtime_t::~runtime_t()
{
   runtime_t::ptr = nullptr;
}

point_t runtime_t::get_desktop_size() const
{
   RECT rect = {};
   GetWindowRect(GetDesktopWindow(), &rect);
   return point_t{ rect.right, rect.bottom };
}

static keyboard_t::key_t 
translate_keycode(WPARAM wParam)
{
   switch (wParam)
   {
      case VK_BACK: return keyboard_t::key_t::backspace;
      case VK_TAB: return keyboard_t::key_t::tab;
      case VK_RETURN: return keyboard_t::key_t::enter;
      case VK_CAPITAL: return keyboard_t::key_t::caps_lock;
      case VK_ESCAPE: return keyboard_t::key_t::escape;
      case VK_SPACE: return keyboard_t::key_t::space;
      case VK_PRIOR: return keyboard_t::key_t::page_up;
      case VK_NEXT: return keyboard_t::key_t::page_down;
      case VK_END: return keyboard_t::key_t::end;
      case VK_HOME: return keyboard_t::key_t::home;
      case VK_LEFT: return keyboard_t::key_t::left;
      case VK_UP: return keyboard_t::key_t::up;
      case VK_RIGHT: return keyboard_t::key_t::right;
      case VK_DOWN: return keyboard_t::key_t::down;
      case VK_INSERT: return keyboard_t::key_t::insert;
      case VK_DELETE: return keyboard_t::key_t::delete_;
      case '0': return keyboard_t::key_t::digit0;
      case '1': return keyboard_t::key_t::digit1;
      case '2': return keyboard_t::key_t::digit2;
      case '3': return keyboard_t::key_t::digit3;
      case '4': return keyboard_t::key_t::digit4;
      case '5': return keyboard_t::key_t::digit5;
      case '6': return keyboard_t::key_t::digit6;
      case '7': return keyboard_t::key_t::digit7;
      case '8': return keyboard_t::key_t::digit8;
      case '9': return keyboard_t::key_t::digit9;
      case 'A': return keyboard_t::key_t::a;
      case 'B': return keyboard_t::key_t::b;
      case 'C': return keyboard_t::key_t::c;
      case 'D': return keyboard_t::key_t::d;
      case 'E': return keyboard_t::key_t::e;
      case 'F': return keyboard_t::key_t::f;
      case 'G': return keyboard_t::key_t::g;
      case 'H': return keyboard_t::key_t::h;
      case 'I': return keyboard_t::key_t::i;
      case 'J': return keyboard_t::key_t::j;
      case 'K': return keyboard_t::key_t::k;
      case 'L': return keyboard_t::key_t::l;
      case 'M': return keyboard_t::key_t::m;
      case 'N': return keyboard_t::key_t::n;
      case 'O': return keyboard_t::key_t::o;
      case 'P': return keyboard_t::key_t::p;
      case 'Q': return keyboard_t::key_t::q;
      case 'R': return keyboard_t::key_t::r;
      case 'S': return keyboard_t::key_t::s;
      case 'T': return keyboard_t::key_t::t;
      case 'U': return keyboard_t::key_t::u;
      case 'V': return keyboard_t::key_t::v;
      case 'W': return keyboard_t::key_t::w;
      case 'X': return keyboard_t::key_t::x;
      case 'Y': return keyboard_t::key_t::y;
      case 'Z': return keyboard_t::key_t::z;
      case VK_LWIN: return keyboard_t::key_t::left_os;
      case VK_RWIN: return keyboard_t::key_t::right_os;
      case VK_F1: return keyboard_t::key_t::f1;
      case VK_F2: return keyboard_t::key_t::f2;
      case VK_F3: return keyboard_t::key_t::f3;
      case VK_F4: return keyboard_t::key_t::f4;
      case VK_F5: return keyboard_t::key_t::f5;
      case VK_F6: return keyboard_t::key_t::f6;
      case VK_F7: return keyboard_t::key_t::f7;
      case VK_F8: return keyboard_t::key_t::f8;
      case VK_F9: return keyboard_t::key_t::f9;
      case VK_F10: return keyboard_t::key_t::f10;
      case VK_F11: return keyboard_t::key_t::f11;
      case VK_F12: return keyboard_t::key_t::f12;
      case VK_NUMLOCK: return keyboard_t::key_t::num_lock;
      case VK_SCROLL: return keyboard_t::key_t::scroll_lock;
      case VK_LSHIFT: return keyboard_t::key_t::left_shift;
      case VK_RSHIFT: return keyboard_t::key_t::right_shift;
      case VK_LCONTROL: return keyboard_t::key_t::left_control;
      case VK_RCONTROL: return keyboard_t::key_t::right_control;
      case VK_LMENU: return keyboard_t::key_t::left_menu;
      case VK_RMENU: return keyboard_t::key_t::right_menu;
   }

   return keyboard_t::key_t::unknown;
}

static WPARAM
win_convert_extended(WPARAM wParam, LPARAM lParam)
{
   WPARAM result = wParam;
   UINT scancode = (lParam & 0x00ff0000) >> 16;
   int  extended = (lParam & 0x01000000) != 0;

   switch (wParam) {
      case VK_SHIFT:
         result = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
         break;
      case VK_CONTROL:
         result = extended ? VK_RCONTROL : VK_LCONTROL;
         break;
      case VK_MENU:
         result = extended ? VK_RMENU : VK_LMENU;
         break;
   }

   return result;
}

static LRESULT CALLBACK
win_message_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   if (uMsg == WM_CLOSE) {
      PostQuitMessage(0);
      return 0;
   }

   input_context_t *input = (input_context_t *)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
   if (input == nullptr) {
      return DefWindowProcA(hWnd, uMsg, wParam, lParam);
   }

   if (uMsg == WM_MOUSEMOVE) {
      input->on_mouse_move({ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
      return 0;
   }
   else if (uMsg == WM_LBUTTONDOWN) {
      input->on_button_pressed(mouse_t::button_t::left);
      return 0;
   }
   else if (uMsg == WM_LBUTTONUP) {
      input->on_button_released(mouse_t::button_t::left);
      return 0;
   }
   else if (uMsg == WM_RBUTTONDOWN) {
      input->on_button_pressed(mouse_t::button_t::right);
      return 0;
   }
   else if (uMsg == WM_RBUTTONUP) {
      input->on_button_released(mouse_t::button_t::right);
      return 0;
   }
   else if (uMsg == WM_MBUTTONDOWN) {
      input->on_button_pressed(mouse_t::button_t::middle);
      return 0;
   }
   else if (uMsg == WM_MBUTTONUP) {
      input->on_button_released(mouse_t::button_t::middle);
      return 0;
   }

   if (uMsg == WM_KEYDOWN) {
      BOOL repeat = (lParam >> 30) & 0x1;
      if (repeat == FALSE) {
         auto key = translate_keycode(win_convert_extended(wParam, lParam));
         if (key != keyboard_t::key_t::unknown) {
            input->on_key_pressed(key);
         }
      }
      return 0;
   }
   else if (uMsg == WM_KEYUP) {
      auto key = translate_keycode(win_convert_extended(wParam, lParam));
      if (key != keyboard_t::key_t::unknown) {
         input->on_key_released(key);
      }
      return 0;
   }

   return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance,
                   _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine,
                   _In_ int nCmdShow)
{
   { fake_context_t{}; }
   SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);

   HWND hWnd = nullptr;
   {
      
      WNDCLASSA wc =
      {
         .style = (CS_OWNDC | CS_HREDRAW | CS_VREDRAW),
         .lpfnWndProc = win_message_proc,
         .hInstance = hInstance,
         .hCursor = LoadCursor(nullptr, IDC_ARROW),
         .lpszClassName = "ludumdareWindowClassName",
      };
      RegisterClassA(&wc);

      DWORD window_style_ex = 0;
      DWORD window_style = kWindowStyle;
      RECT  window_rect = { 0, 0, 1280, 720 };
      AdjustWindowRect(&window_rect, window_style, FALSE);

      int window_w = window_rect.right - window_rect.left;
      int window_h = window_rect.bottom - window_rect.top;
      int window_x = (GetSystemMetrics(SM_CXSCREEN) - window_w) / 2;
      int window_y = (GetSystemMetrics(SM_CYSCREEN) - window_h) / 2;
      hWnd = CreateWindowExA(window_style_ex,
                             wc.lpszClassName,
                             "LD52",
                             window_style,
                             window_x,
                             window_y,
                             window_w,
                             window_h,
                             nullptr,
                             nullptr,
                             wc.hInstance,
                             nullptr);
      if (hWnd == nullptr) {
         win_fatal_error("could not create window!");
      }
   }

   HDC hDC = GetDC(hWnd);
   HGLRC hRC = nullptr;
   {
#if 1
      // note: create a core context
      PIXELFORMATDESCRIPTOR pfd = { 0 };
      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.cColorBits = 32;
      pfd.cDepthBits = 24;
      pfd.cStencilBits = 8;
      pfd.iLayerType = PFD_MAIN_PLANE;

      // note: select pixel format
      int pixel_format_index = ChoosePixelFormat(hDC, &pfd);
      DescribePixelFormat(hDC, pixel_format_index, sizeof(pfd), &pfd);

      const int pixel_format_attribs[] = {
         WGL_DRAW_TO_WINDOW_ARB, GL_TRUE          ,
         WGL_SUPPORT_OPENGL_ARB, GL_TRUE          ,
         WGL_DOUBLE_BUFFER_ARB , GL_TRUE          ,
         WGL_PIXEL_TYPE_ARB    , WGL_TYPE_RGBA_ARB,
         WGL_COLOR_BITS_ARB    , 32               ,
         WGL_DEPTH_BITS_ARB    , 24               ,
         WGL_STENCIL_BITS_ARB  , 8                ,
         WGL_SAMPLE_BUFFERS_ARB, GL_TRUE          ,
         WGL_SAMPLES_ARB       , 4                ,
         0
      };
      UINT num_pixel_formats = 0;
      wglChoosePixelFormatARB(hDC, pixel_format_attribs, 0, 1, &pixel_format_index, &num_pixel_formats);
      DescribePixelFormat(hDC, pixel_format_index, sizeof(pfd), &pfd);
      SetPixelFormat(hDC, pixel_format_index, &pfd);

      const int context_attrib_list[] = {
         WGL_CONTEXT_MAJOR_VERSION_ARB, 2                               ,
         WGL_CONTEXT_MINOR_VERSION_ARB, 1                               ,
         //WGL_CONTEXT_PROFILE_MASK_ARB , WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
         WGL_CONTEXT_FLAGS_ARB        , 0                               ,
         0
      };

      hRC = wglCreateContextAttribsARB(hDC, NULL, context_attrib_list);
      if (wglMakeCurrent(hDC, hRC) == FALSE) {
         win_fatal_error("Could not create render context!");
      }

      OutputDebugStringA((const char *)glGetString(GL_VERSION));
      OutputDebugStringA("\n");
#else
      PIXELFORMATDESCRIPTOR pfd = {};
      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.cColorBits = 32;
      pfd.cDepthBits = 24;
      pfd.cStencilBits = 8;
      pfd.iLayerType = PFD_MAIN_PLANE;

      int pixel_format_index = ChoosePixelFormat(hDC, &pfd);
      DescribePixelFormat(hDC, pixel_format_index, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
      int pixel_format = SetPixelFormat(hDC, pixel_format_index, &pfd);
      hRC = wglCreateContext(hDC);
      if (hRC == nullptr || wglMakeCurrent(hDC, hRC) == FALSE) {
         win_fatal_error("Could not create render context!");
      }
#endif

#define GL_FUNC(ret, name, ...)  name = (type_##name *)wglGetProcAddress(#name); 
      OPENGL_FUNCTIONS;
#undef GL_FUNC

#define GL_FUNC(ret, name, ...)  if (name == nullptr) { win_fatal_error("Could not load '" #name "'!"); }
      OPENGL_FUNCTIONS;
#undef GL_FUNC
   }

   window_t window(hWnd);
   input_context_t input;
   gl_graphics_t graphics;
   audio_device_t audio_device;

   runtime_t runtime;
   runtime.m_window = &window;
   runtime.m_input = &input;
   runtime.m_graphics = &graphics;

   SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)&input);
   ShowWindow(hWnd, nCmdShow);

   int ret = main(__argc, __argv);

   wglMakeCurrent(nullptr, nullptr);
   wglDeleteContext(hRC);
   DestroyWindow(hWnd);

   return ret;
}

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#undef STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_PUSHDATA_API
#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4245)
#pragma warning(disable: 4456)
#pragma warning(disable: 4457)
#pragma warning(disable: 4701)
#include <stb_vorbis.h>
#pragma warning(pop)
