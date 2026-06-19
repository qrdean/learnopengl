#ifndef MESH_H
#define MESH_H

#include "cglm/cglm.h"
#include "shader_m.h"
#include <string.h>

typedef struct Vertex {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
} Vertex;

typedef enum { DIFFUSE, SPECULAR, NORMAL, NONE } TextureType;

typedef struct Texture
{
  unsigned int id;
  TextureType type;
} Texture;

typedef struct TextureDecode {
  unsigned char *pixels;
  int width, height, channels;
  TextureType type;
} TextureDecode;

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
unsigned int SetupTexture(TextureDecode texture_decode);

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
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->numIndices * sizeof(unsigned short), m->indices, GL_STATIC_DRAW);

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
    snprintf(materialUniformName, sizeof(materialUniformName), "%s%i", GetTextureTypeChar(type), number);
    setInt(shader.ID, materialUniformName, i);
    glBindTexture(GL_TEXTURE_2D, m->textures[i].id);
  }

  glBindVertexArray(m->VAO);
  glDrawElements(GL_TRIANGLES, m->numIndices, GL_UNSIGNED_SHORT, 0);
  glBindVertexArray(0);

  glActiveTexture(GL_TEXTURE0);
}

char* GetTextureTypeChar(TextureType type) {
  if (type == DIFFUSE) {
    return "texture_diffuse";
  } else if (type == SPECULAR) {
    return "texture_specular";
  } else if (type == NORMAL) {
    return "texture_normal";
  }
  return "";
}

TextureType GetTextureTypeFromChar(const char* name)
{
  if (strcmp(name, "specular") == 0) {
    printf("loading spec\n");
    return SPECULAR;
  } else if (strcmp(name, "diffuse") == 0) {
    printf("loading diffuse\n");
    return DIFFUSE;
  } else if (strcmp(name, "normal") == 0) {
    printf("loading norm\n");
    return NORMAL;
  }
  return NONE;
}              


unsigned int SetupTexture(TextureDecode texture_decode) 
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  if (texture_decode.pixels)  
  {
    GLenum format;
    if (texture_decode.channels == 1)
      format = GL_RED;
    else if (texture_decode.channels == 3)
      format = GL_RGB;
    else if (texture_decode.channels == 4) 
      format = GL_RGBA;
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, texture_decode.width, texture_decode.height, 0, format, GL_UNSIGNED_BYTE, texture_decode.pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);

    free(texture_decode.pixels);
  }

  return textureID;
}

#endif
