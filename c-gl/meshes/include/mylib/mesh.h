#ifndef MESH_H
#define MESH_H

#include "cglm/cglm.h"
#include "shader_m.h"

typedef struct Vertex {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
} Vertex;

typedef enum { DIFFUSE, SPECULAR } TextureType;

typedef struct Texture
{
  unsigned int id;
  TextureType type;
} Texture;

typedef struct Mesh
{
  Vertex *vertices;
  uint32_t numVertices;
  unsigned short *indices;
  uint32_t numIndices;
  Texture *textures;
  uint32_t numTextures;
  unsigned int VAO, VBO, EBO;
} Mesh;

Mesh createMesh(Vertex *vertices, unsigned short *indices, Texture *textures);
void Draw(Mesh *m, struct Shader shader);
void setupMesh(Mesh *m);
char* GetTextureTypeChar(TextureType type);

Mesh createMesh(Vertex *vertices, unsigned short *indices, Texture *textures){
  struct Mesh m = {0};
  m.vertices = vertices;
  m.indices = indices;
  m.textures = textures;
  setupMesh(&m);
  return m;
}

void setupMesh(Mesh *m)
{
  glGenVertexArrays(1, &m->VAO);
  glGenBuffers(1, &m->VBO);
  glGenBuffers(1, &m->EBO);
  
  glBindVertexArray(m->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m->VBO);

  glBufferData(GL_ARRAY_BUFFER, m->numVertices * sizeof(Vertex), &m->vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->numIndices * sizeof(unsigned int), m->indices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

  glBindVertexArray(0);
}

void Draw(Mesh *m, struct Shader shader)
{
  unsigned int diffuseNr = 1;
  unsigned int specularNr = 1;
  for (unsigned int i = 0; i < m->numTextures; i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);

    int number;
    Texture *texture = &m->textures[i];
    TextureType type = texture->type;
    if (type == DIFFUSE)
    {
      number = diffuseNr++;
    } 
    else if (type == SPECULAR) 
    {
      number = specularNr++;
    }

    char materialUniformName[256];
    sprintf(materialUniformName, "material.%s%i", GetTextureTypeChar(type), number);
    setInt(shader.ID, materialUniformName, i);
  }

  glBindVertexArray(m->VAO);
  glDrawElements(GL_TRIANGLES, m->numIndices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  glActiveTexture(GL_TEXTURE0);
}

char* GetTextureTypeChar(TextureType type) {
  if (type == DIFFUSE) {
    return "diffuse";
  } else if (type == SPECULAR)
    return "specular";
  return "";
}
#endif
