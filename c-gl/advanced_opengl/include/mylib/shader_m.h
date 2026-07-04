#ifndef SHADER_M_H
#define SHADER_M_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "stdio.h"
#include <stdlib.h>
#include <stdbool.h>

// Probably should move this out somewhere
char* LoadFileText(const char *filename);

typedef struct Shader {
  int ID; 
} Shader;

static Shader createShader(const char* vertexPath, const char* fragmentPath);
void setBool(unsigned int x, const char* name, bool value);
void setInt(unsigned int x, const char* name, int value);
void setFloat(unsigned int x, const char* name, float value);
void setFloat(unsigned int x, const char* name, float value);


char *LoadFileText(const char *filename)
{
  char *text = NULL;
  if (filename != NULL) 
  {
    FILE *file = fopen(filename, "rt");

    if (file != NULL) 
    {
      fseek(file, 0, SEEK_END);
      unsigned int size = (unsigned int)ftell(file);
      fseek(file, 0, SEEK_SET);

      if (size > 0)
      {
        text = (char*)calloc(size + 1, sizeof(char));

        if (text != NULL)
        {
          unsigned int count = (unsigned int)fread(text, sizeof(char), size, file);

          if (count < size) text = (char*)realloc(text, count + 1);

          text[count] = '\0';
          printf("fileio text file loaded successfully %s\n", filename);
        } 
        else 
        {
          printf("FILEIO: failed to allocate memory for file %s\n", filename);
        }
      }
      else 
      {
        printf("FILEIO: failed to read file %s\n", filename);
      }

      fclose(file);
    } else 
    {
      printf("FILEIO: failed to open file %s\n",  filename);
    }
  } 
  else 
  {
      printf("FILEIO: file name not valid\n");
  }
  return text;
}

unsigned char *LoadFileData(const char *filename, int *dataSize)
{
   unsigned char *data = NULL;
    *dataSize = 0;

    if (filename != NULL)
    {
        FILE *file = fopen(filename, "rb");

        if (file != NULL)
        {
            // WARNING: On binary streams SEEK_END could not be found,
            // using fseek() and ftell() could not work in some (rare) cases
            fseek(file, 0, SEEK_END);
            int size = ftell(file);     // WARNING: ftell() returns 'long int', maximum size returned is INT_MAX (2147483647 bytes)
            fseek(file, 0, SEEK_SET);

            if (size > 0)
            {
                data = (unsigned char *)calloc(size, sizeof(unsigned char));

                if (data != NULL)
                {
                    // NOTE: fread() returns number of read elements instead of bytes, so reading [1 byte, size elements]
                    size_t count = fread(data, sizeof(unsigned char), size, file);

                    // WARNING: fread() returns a size_t value, usually 'unsigned int' (32bit compilation) and 'unsigned long long' (64bit compilation)
                    // dataSize is unified along raylib as a 'int' type, so, for file-sizes >INT_MAX (2147483647 bytes) there is a limitation
                    if (count > 2147483647)
                    {
                        free(data);
                        data = NULL;
                    }
                    else
                    {
                        *dataSize = (int)count;
                    }
                }
            }
            fclose(file);
        }
    }

    return data;
}

static Shader createShader(const char* vertexPath, const char* fragmentPath)
{
  Shader s;
  s.ID = -1;
  const char* vertex_source = LoadFileText(vertexPath);
  if (vertex_source == NULL) {
    return(s);
  }
  const char* fragment_source = LoadFileText(fragmentPath);
  if (fragment_source == NULL) {
    return(s);
  }
  unsigned int vertexShader, fragmentShader;
  int success;
  char infoLog[512];

  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertex_source, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

  if(!success) 
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_ERROR\n%s\n", infoLog);
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragment_source, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

  if(!success) 
  {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::FRAGMENT::COMPILATION_ERROR\n%s\n", infoLog);
  }

  s.ID = glCreateProgram();
  glAttachShader(s.ID, vertexShader);
  glAttachShader(s.ID, fragmentShader);
  glLinkProgram(s.ID);

  glGetProgramiv(s.ID, GL_LINK_STATUS, &success);
  if (!success) 
  {
    glGetProgramInfoLog(s.ID, 512, NULL, infoLog);
    printf("ERROR::SHADER::PROGRAM::COMPILATION_ERROR\n%s\n", infoLog);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return(s);
}

void setBool(unsigned int programID, const char* name, bool value)
{
    glUniform1i(glGetUniformLocation(programID, name), (int)value);
}

// ------------------------------------------------------------------------
void setInt(unsigned int programID, const char* name, int value)
{
    glUniform1i(glGetUniformLocation(programID, name), value);
}

// ------------------------------------------------------------------------
void setFloat(unsigned int programID, const char* name, float value)
{
    glUniform1f(glGetUniformLocation(programID, name), value);
}

// ------------------------------------------------------------------------
void setVec2(unsigned int programID, const char* name, const vec2 value)
{
    glUniform2fv(glGetUniformLocation(programID, name), 1, &value[0]);
}

void setVec2_XY(unsigned int programID, const char* name, float x, float y)
{
    glUniform2f(glGetUniformLocation(programID, name), x, y);
}

// ------------------------------------------------------------------------
void setVec3(unsigned int programID, const char* name, const vec3 value)
{
    glUniform3fv(glGetUniformLocation(programID, name), 1, &value[0]);
}

void setVec3_XYZ(unsigned int programID, const char* name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(programID, name), x, y, z);
}

// ------------------------------------------------------------------------
void setVec4(unsigned int programID, const char* name, const vec4 value)
{
    glUniform4fv(glGetUniformLocation(programID, name), 1, &value[0]);
}

void setVec4_XYZW(unsigned int programID, const char* name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(programID, name), x, y, z, w);
}

// ------------------------------------------------------------------------
void setMat2(unsigned int programID, const char* name, const mat2 mat)
{
    glUniformMatrix2fv(glGetUniformLocation(programID, name), 1, GL_FALSE, &mat[0][0]);
}

// ------------------------------------------------------------------------
void setMat3(unsigned int programID, const char* name, const mat3 mat)
{
    glUniformMatrix3fv(glGetUniformLocation(programID, name), 1, GL_FALSE, &mat[0][0]);
}

// ------------------------------------------------------------------------
void setMat4(unsigned int programID, const char* name, const mat4 mat)
{
    glUniformMatrix4fv(glGetUniformLocation(programID, name), 1, GL_FALSE, &mat[0][0]);
}

void FreeShader(unsigned int programID) 
{
  glDeleteShader(programID);
}

#endif // SHADER_M_H
