#include <stdint.h>
typedef struct {
  uint32_t vbo;
} VertexBuffer; 

VertexBuffer new_vertex_buffer(const void* data, uint32_t size);
void destroy_vertex_buffer(VertexBuffer vb);
void bind_vertex_buffer(VertexBuffer vb);
void unbind_vertex_buffer();

typedef struct {
  uint32_t ibo;
  uint32_t count;
} IndexBuffer;

IndexBuffer new_index_buffer(const void* data, uint32_t count);
void destroy_index_buffer(IndexBuffer ib);
void bind_index_buffer(IndexBuffer ib);
void unbind_index_buffer();

typedef struct {
  uint32_t fbo;
} FrameBuffer;

FrameBuffer new_frame_buffer(const void* data, uint32_t size);
