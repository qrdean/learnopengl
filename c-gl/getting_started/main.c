#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

vec3 cameraPos = (vec3){0.0f, 0.0f, 3.0f};
vec3 cameraFront = (vec3){0.0f, 0.0f, -1.0f};
vec3 cameraUp = (vec3){0.0f, 1.0f, 0.0f};
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void pprint_vec2(vec2 v) 
{
  printf("vector: [%f %f]", v[0], v[1]);
}


void pprint_vec3(vec3 v, const char* msg) 
{
  printf("\n[%s] vector: \n[%f %f %f]\n", msg, v[0], v[1], v[2]);
}

void pprint_vec4(vec4 v) 
{
  printf("vector: [%f %f %f %f]", v[0], v[1], v[2], v[3]);
}

void pprint_mat2(mat2 m, const char* name) 
{
  printf("\nmatrix %s: \n[%f %f]\n[%f %f]\n", name, m[0][0], m[0][1], m[1][0], m[1][1]);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) 
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
  {
    glfwSetWindowShouldClose(window, 1);
  }

  float camSpeed = 0.05f * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
  {
    vec3 temp = {0.0f, 0.0f, 0.0f};
    glm_vec3_muladds(cameraFront, camSpeed, temp);
    glm_vec3_add(cameraPos, temp, cameraPos);
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
  {
    vec3 temp = {0.0f, 0.0f, 0.0f};
    glm_vec3_muladds(cameraFront, camSpeed, temp);
    glm_vec3_sub(cameraPos, temp, cameraPos);
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
  {
    vec3 temp = {0.0f, 0.0f, 0.0f};
    glm_vec3_crossn(cameraFront, cameraUp, temp);
    glm_vec3_muladds(temp, camSpeed, temp);
    glm_vec3_sub(cameraPos, temp, cameraPos);
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) 
  {
    vec3 temp = {0.0f, 0.0f, 0.0f};
    glm_vec3_crossn(cameraFront, cameraUp, temp);
    glm_vec3_muladds(temp, camSpeed, temp);
    glm_vec3_add(cameraPos, temp, cameraPos);
  }
} 

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
          printf("fileio text file loaded successfully %s", filename);
        } 
        else 
        {
          printf("FILEIO: failed to allocate memory for file %s ", filename);
        }
      }
      else 
      {
        printf("FILEIO: failed to read file %s", filename);
      }

      fclose(file);
    } else 
    {
      printf("FILEIO: failed to open file %s",  filename);
    }
  } 
  else 
  {
      printf("FILEIO: file name not valid");
  }
  return text;
}

struct Shader {
  int ID; 
};

static struct Shader createShader(const char* vertexPath, const char* fragmentPath)
{
  struct Shader s;
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

void setShaderBool(struct Shader s, const char* name, int value) 
{
  glUniform1i(glGetUniformLocation(s.ID, name), value);
}

void setShaderInt(struct Shader s, const char* name, int value) 
{
  glUniform1i(glGetUniformLocation(s.ID, name), value);
}

void setShaderFloat(struct Shader s, const char* name, float value) 
{
  glUniform1f(glGetUniformLocation(s.ID, name), value);
}

void loadTexture(const char* filename)
{
  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
  if (data) 
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else 
  {
    printf("failed to load texture\n");
  }
  stbi_image_free(data);
}


// const char* vertex_shader_source =
//   "#version 330 core\n"
//   "layout (location = 0) in vec3 aPos;\n"
//   "layout (location = 1) in vec3 aColor;\n"
//   "out vec3 ourColor;\n"
//   "void main() {\n"
//   "gl_Position = vec4(aPos, 1.0);\n"
//   "ourColor = aColor;\n"
//   "}\0";

// const char* fragment_shader_source =
//   "#version 330 core\n"
//   "out vec4 FragColor;\n"
//   "in vec3 ourColor;\n"
//   "void main() {\n"
//   "FragColor = vec4(ourColor, 1.0f);\n"
//   "}\0";

int main(void) 
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
  if (window == NULL) 
  {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
  {
    printf("Failed to init GLAD\n");
    return -1;
  }

  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  struct Shader s = createShader("vertex_shader.vert", "fragment_shader.frag");


  // float vertices[] = {
  //   0.5f, 0.5f, 0.0f,
  //   0.5f, -0.5f, 0.0f,
  //   -0.5f, -0.5f, 0.0f,
  //   -0.5f, 0.5f, 0.0f
  // };

  // float vertices[] = {
  //   0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
  //   -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
  //   0.0f, 0.5f, 0.0f,   0.0f, 0.0f, 1.0f
  // };

  // Textures
  // float vertices[] = {
  //   0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
  //   0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
  //  -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
  //  -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f
  // };

  float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
  };

  unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
  };

  float texCoords[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.5f, 1.0f
  };

  unsigned int texture1, texture2;
  glGenTextures(1, &texture1);
  glBindTexture(GL_TEXTURE_2D, texture1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  loadTexture("container.jpg");

  glGenTextures(1, &texture2);
  glBindTexture(GL_TEXTURE_2D, texture2);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  stbi_set_flip_vertically_on_load(1);
  loadTexture("awesomeface.png");
  // int width, height, nrChannels;
  // unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
  // if (data) 
  // {
  //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  //   glGenerateMipmap(GL_TEXTURE_2D);
  // }
  // else 
  // {
  //   printf("failed to load texture\n");
  // }

  // data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);
  // if (data)
  // {
  //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  //   glGenerateMipmap(GL_TEXTURE_2D);
  // }
  // stbi_image_free(data);

  // float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
  // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  unsigned int VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  // glEnableVertexAttribArray(2);

  // glBindBuffer(GL_ARRAY_BUFFER, 0);

  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // glBindVertexArray(0);

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  //

  glUseProgram(s.ID);
  // setup textures
  glUniform1i(glGetUniformLocation(s.ID, "texture1"), 0);
  glUniform1i(glGetUniformLocation(s.ID, "texture2"), 1);

  mat4 model = GLM_MAT4_IDENTITY_INIT;
  glm_rotate(model, glm_rad(-55.0f), (vec3){1.0f, 0.0f, 0.0f});

  mat4 view = GLM_MAT4_IDENTITY_INIT;
  // glm_translate(view, (vec3){0.0f, 0.0f, -3.0f});
  glm_lookat((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}, view);

  mat4 proj;
  glm_perspective(glm_rad(45.0f), 800.0f/600.0f, 0.1, 100.0f, proj);
  int projectionLoc = glGetUniformLocation(s.ID, "projection");
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &proj[0][0]);

  // Camera Setup
  // ----
  vec3 cameraTarget = {0.0f, 0.0f, 0.0f};
  vec3 cameraDirection = {0.0f, 0.0f, 0.0f}; 
  glm_vec3_subadd(cameraPos, cameraTarget, cameraDirection);
  glm_vec3_normalize(cameraDirection);
  // printf("[%f %f %f]", cameraDirection[0], cameraDirection[1], cameraDirection[2]);
  //
  // vec3 up = {0.0f, 1.0f, 0.0};
  // vec3 cameraRight = GLM_VEC3_ZERO_INIT;
  // glm_cross(up, cameraDirection, cameraRight);
  // printf("before norm: [%f %f %f]", cameraRight[0], cameraRight[1], cameraRight[2]);
  // glm_normalize(cameraRight);
  // printf("norm: [%f %f %f]", cameraRight[0], cameraRight[1], cameraRight[2]);
  // glm_cross(cameraDirection, cameraRight, cameraUp);
  // printf("cam up: [%f %f %f]", cameraUp[0], cameraUp[1], cameraUp[2]);

  // mat2 m2 = GLM_MAT2_IDENTITY_INIT;
  // pprint_mat2(m2, "m2");


  glEnable(GL_DEPTH_TEST);

  vec3 cubePositions[] = {
    { 0.0f, 0.0f, 0.0f},
    { 2.0f,  5.0f, -15.0f},
    {-1.5f, -2.2f, -2.5f},
    {-3.8f, -2.0f, -12.3f},
    { 2.4f, -0.4f, -3.5f},
    {-1.7f,  3.0f, -7.5f},
    { 1.3f, -2.0f, -2.5f},
    { 1.5f,  2.0f, -2.5f},
    { 1.5f,  0.2f, -1.5f},
    {-1.3f,  1.0f, -1.5f}
  };
  
  while(!glfwWindowShouldClose(window)) 
  {
    float currentFrame = (float)(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    // input
    processInput(window);

    // rendering
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // float timeValue = glfwGetTime();
    // float greenValue = sin(timeValue) / 2.0f + 0.5f;
    // int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
    // glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    // mat4 trans = GLM_MAT4_IDENTITY_INIT;
    // glm_translate(trans, (vec3){0.5f, -0.5f, 0.0f});
    // glm_rotate(trans, (float)glfwGetTime(), (vec3){0.0f, 0.0f, 1.0f});

    glUseProgram(s.ID);
    // unsigned int transformLoc = glGetUniformLocation(s.ID, "transform");
    // glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans[0]);
    // int viewLoc = glGetUniformLocation(s.ID, "view");
    // glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    // int projectionLoc = glGetUniformLocation(s.ID, "projection");
    // glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &proj[0][0]);
    // glm_rotate(model, (float)glfwGetTime() * glm_rad(1.0f), (vec3){0.5f, 1.0f, 0.0f});
    //
    // const float radius = 10.0f;
    // float cam_x = (float)(sin(glfwGetTime()) * radius);
    // float cam_z = cos(glfwGetTime()) * radius;
    // glm_lookat((vec3){cam_x, 0.0, cam_z}, (vec3){0.0f, 0.0f, 0.0f},(vec3){0.0f, 1.0f, 0.0f}, view);
    vec3 target = GLM_VEC3_ZERO_INIT;
    mat4 view = GLM_MAT4_ZERO_INIT;
    glm_vec3_add(cameraPos, cameraFront,target);
    glm_lookat(cameraPos, target, cameraUp, view);
    int viewLoc = glGetUniformLocation(s.ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    glBindVertexArray(VAO);
    for(unsigned int i = 0; i < 10; i++)
    {
      mat4 model = GLM_MAT4_IDENTITY_INIT;
      glm_translate(model, cubePositions[i]);
      float angle = 20.0f * i;
      glm_rotate(model, glm_rad(angle), (vec3){1.0f, 0.3f, 0.5f});
      int modelLoc = glGetUniformLocation(s.ID, "model");
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model[0]);

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // check and call events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(s.ID);

  glfwTerminate();
  printf("ran success\n");
  // printf("hello world\n");
  return 0;
}


