// warez.hpp

struct warez_t {
   enum class type_t {
      metal,
      water,
   };

   warez_t() = default;

   void update(const timespan_t &deltatime)
   {
   }

   void render(graphics_t &graphics)
   {
   }

   type_t    m_type;
   vector2_t m_position;
};

struct warez_list_t {
};
