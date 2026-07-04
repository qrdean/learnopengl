#include "glad.c"
#include "renderer.h"

VertexBuffer new_vertex_buffer(const void* data, uint32_t size)
{
  VertexBuffer vb;
  glGenBuffers(1, &vb.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vb.vbo);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  return vb;
}

void destroy_vertex_buffer(VertexBuffer vb) 
{
  glDeleteBuffers(1, &vb.vbo);
}

void bind_vertex_buffer(VertexBuffer vb)
{
  glBindBuffer(GL_ARRAY_BUFFER, vb.vbo);
}

void unbind_vertex_buffer()
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}


IndexBuffer new_index_buffer(const void* data, uint32_t count) {
  IndexBuffer ib;
  glGenBuffers(1, &ib.ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, GL_STATIC_DRAW);
  return ib;
}

void destroy_index_buffer(IndexBuffer ib) {
  glDeleteBuffers(1, &ib.ibo);
}

void bind_index_buffer(IndexBuffer ib) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.ibo);
}

void unbind_index_buffer() {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// FrameBuffer new_frame_buffer(const void* data, ) {
//
// }
