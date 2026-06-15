#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include <mylib/camera.h>
#include <mylib/shader_m.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void pprint_vec3(vec3 v, const char* msg) 
{
  printf("\n[%s] vector: \n[%f %f %f]\n", msg, v[0], v[1], v[2]);
}

unsigned int loadTexture(const char* filename)
{
  unsigned int textureId = 0;
  glGenTextures(1, &textureId);

  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
  if (data) 
  {
    GLenum format;
    if (nrChannels == 1)
      format = GL_RED;
    else if (nrChannels == 3)
      format = GL_RGB;
    else if (nrChannels == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else 
  {
    printf("failed to load texture\n");
  }
  stbi_image_free(data);

  return textureId;
}


int main()
{
  // glfw boilerplate should move to another header or function
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
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
  {
    printf("Failed to init GLAD\n");
    return -1;
  }

  glEnable(GL_DEPTH_TEST);
  glViewport(0, 0, 800, 600);

  // Start of loadingj
  struct Shader lightingS = createShader("shaders/colors_vert.vs", "shaders/colors_frag.fs");
  struct Shader lightCubeS = createShader("shaders/light_cube_vert.vs", "shaders/light_cube_frag.fs");

  // 36 point Cube Vert
  // float vertices[] = {
  //     -0.5f, -0.5f, -0.5f,
  //      0.5f, -0.5f, -0.5f,
  //      0.5f,  0.5f, -0.5f,
  //      0.5f,  0.5f, -0.5f,
  //     -0.5f,  0.5f, -0.5f,
  //     -0.5f, -0.5f, -0.5f,
  //
  //     -0.5f, -0.5f,  0.5f,
  //      0.5f, -0.5f,  0.5f,
  //      0.5f,  0.5f,  0.5f,
  //      0.5f,  0.5f,  0.5f,
  //     -0.5f,  0.5f,  0.5f,
  //     -0.5f, -0.5f,  0.5f,
  //
  //     -0.5f,  0.5f,  0.5f,
  //     -0.5f,  0.5f, -0.5f,
  //     -0.5f, -0.5f, -0.5f,
  //     -0.5f, -0.5f, -0.5f,
  //     -0.5f, -0.5f,  0.5f,
  //     -0.5f,  0.5f,  0.5f,
  //
  //      0.5f,  0.5f,  0.5f,
  //      0.5f,  0.5f, -0.5f,
  //      0.5f, -0.5f, -0.5f,
  //      0.5f, -0.5f, -0.5f,
  //      0.5f, -0.5f,  0.5f,
  //      0.5f,  0.5f,  0.5f,
  //
  //     -0.5f, -0.5f, -0.5f,
  //      0.5f, -0.5f, -0.5f,
  //      0.5f, -0.5f,  0.5f,
  //      0.5f, -0.5f,  0.5f,
  //     -0.5f, -0.5f,  0.5f,
  //     -0.5f, -0.5f, -0.5f,
  //
  //     -0.5f,  0.5f, -0.5f,
  //      0.5f,  0.5f, -0.5f,
  //      0.5f,  0.5f,  0.5f,
  //      0.5f,  0.5f,  0.5f,
  //     -0.5f,  0.5f,  0.5f,
  //     -0.5f,  0.5f, -0.5f,
  // };
  
  // 36 point cube verts with calculated normals
  // float vertices[] = {
  //   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
  //    0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
  //    0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
  //    0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
  //   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
  //   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
  //
  //   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
  //    0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
  //    0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
  //    0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
  //   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
  //   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
  //
  //   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
  //   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
  //   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
  //   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
  //   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
  //   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
  //
  //    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
  //    0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
  //    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
  //    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
  //    0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
  //    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
  //
  //   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
  //    0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
  //    0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
  //    0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
  //   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
  //   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
  //
  //   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
  //    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
  //    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
  //    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
  //   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
  //   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
  // };

  float vertices[] = {
      // positions          // normals           // texture coords
      -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
       0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
      -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

      -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
      -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
      -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

       0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
       0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
       0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
       0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
  };

  unsigned int cubeVAO, VBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(cubeVAO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  
  unsigned int lightCubeVAO;
  glGenVertexArrays(1, &lightCubeVAO);
  glBindVertexArray(lightCubeVAO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  unsigned int diffuseMap = loadTexture("assets/container2.png");
  unsigned int specularMap = loadTexture("assets/container2_specular.png");

  // Clear color prior to loop. if changing then we would add to loop
  glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

  // glUseProgram(lightingS.ID);
  // setVec3(lightingS.ID, "objectColor", (vec3){1.0f, 0.5f, 0.31f});
  // setVec3(lightingS.ID, "lightColor", (vec3){1.0f, 1.0f, 1.0f});

  // glUseProgram(lightCubeS.ID);
  // setVec3(lightingS.ID, "objectColor", (vec3){1.0f, 0.5f, 0.31f});
  // setVec3(lightingS.ID, "lightColor", (vec3){1.0f, 1.0f, 1.0f});

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

  vec3 pointLightPositions[] = {
      { 0.7f,  0.2f,  2.0f},
      { 2.3f, -3.3f, -4.0f},
      { -4.0f,  2.0f, -12.0f },
      { 0.0f,  0.0f, -3.0f}
  };

  CreateCamera((vec3){0.5f, 0.0f, 5.0f});

  vec3 lightPos = {1.2f, 1.0f, 2.0f};
  glUseProgram(lightingS.ID);
  setInt(lightingS.ID, "material.diffuse", 0);
  setInt(lightingS.ID, "material.specular", 1);
  // setVec3(lightingS.ID, "lightPos", lightPos);

  while (!glfwWindowShouldClose(window))
  {

    // per-frame time logic
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    processInput(window);

    // move the light
    // lightPos[0] = 1.0f + sin(glfwGetTime()) * 2.0f; // x
    // lightPos[1] = sin(glfwGetTime() / 2.0f) * 1.0f; // y

    // vec3 lightColor;
    // lightColor[0] = sin(glfwGetTime() * 2.0f);
    // lightColor[1] = sin(glfwGetTime() * 0.7f);
    // lightColor[2] = sin(glfwGetTime() * 1.3f);
    //
    // vec3 diffuseColor, ambientColor; 
    // glm_vec3_mul(lightColor, (vec3){0.5f, 0.5f, 0.5f}, diffuseColor);
    // glm_vec3_mul(diffuseColor, (vec3){0.5f, 0.5f, 0.5f}, ambientColor);

    // render
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw our objects
    glUseProgram(lightingS.ID);
    setVec3(lightingS.ID, "viewPos", Camera->Position);
    setFloat(lightingS.ID, "material.shininess", 32.0f);

    // Directional Lighting
    setVec3(lightingS.ID, "dirLight.direction", (vec3){-0.2f, -1.0f, -0.3f});
    setVec3(lightingS.ID, "dirLight.ambient", (vec3){0.05f, 0.05f, 0.05f});
    setVec3(lightingS.ID, "dirLight.diffuse", (vec3){0.4f, 0.4f, 0.4f});
    setVec3(lightingS.ID, "dirLight.specular", (vec3){0.5f,0.5f,0.5f});

    // point lights
    setVec3(lightingS.ID, "pointLights[0].position", pointLightPositions[0]);
    setVec3(lightingS.ID, "pointLights[0].ambient", (vec3){0.05f, 0.05f, 0.05f});
    setVec3(lightingS.ID, "pointLights[0].diffuse", (vec3){0.8f, 0.8f, 0.8f});
    setVec3(lightingS.ID, "pointLights[0].specular", (vec3){1.0f, 1.0f, 1.0f});
    setFloat(lightingS.ID, "pointLights[0].constant", 1.0f);
    setFloat(lightingS.ID, "pointLights[0].linear", 0.09f);
    setFloat(lightingS.ID, "pointLights[0].quadratic", 0.032f);

    setVec3(lightingS.ID, "pointLights[1].position", pointLightPositions[1]);
    setVec3(lightingS.ID, "pointLights[1].ambient", (vec3){0.05f, 0.05f, 0.05f});
    setVec3(lightingS.ID, "pointLights[1].diffuse", (vec3){0.8f, 0.8f, 0.8f});
    setVec3(lightingS.ID, "pointLights[1].specular", (vec3){1.0f, 1.0f, 1.0f});
    setFloat(lightingS.ID, "pointLights[1].constant", 1.0f);
    setFloat(lightingS.ID, "pointLights[1].linear", 0.09f);
    setFloat(lightingS.ID, "pointLights[1].quadratic", 0.032f);

    setVec3(lightingS.ID, "pointLights[2].position", pointLightPositions[2]);
    setVec3(lightingS.ID, "pointLights[2].ambient", (vec3){0.05f, 0.05f, 0.05f});
    setVec3(lightingS.ID, "pointLights[2].diffuse", (vec3){0.8f, 0.8f, 0.8f});
    setVec3(lightingS.ID, "pointLights[2].specular", (vec3){1.0f, 1.0f, 1.0f});
    setFloat(lightingS.ID, "pointLights[2].constant", 1.0f);
    setFloat(lightingS.ID, "pointLights[2].linear", 0.09f);
    setFloat(lightingS.ID, "pointLights[2].quadratic", 0.032f);

    setVec3(lightingS.ID, "pointLights[3].position", pointLightPositions[3]);
    setVec3(lightingS.ID, "pointLights[3].ambient", (vec3){0.05f, 0.05f, 0.05f});
    setVec3(lightingS.ID, "pointLights[3].diffuse", (vec3){0.8f, 0.8f, 0.8f});
    setVec3(lightingS.ID, "pointLights[3].specular", (vec3){1.0f, 1.0f, 1.0f});
    setFloat(lightingS.ID, "pointLights[3].constant", 1.0f);
    setFloat(lightingS.ID, "pointLights[3].linear", 0.09f);
    setFloat(lightingS.ID, "pointLights[3].quadratic", 0.032f);

    setVec3(lightingS.ID, "spotLight.position", Camera->Position);
    setVec3(lightingS.ID, "spotLight.direction", Camera->Front);
    setVec3(lightingS.ID, "spotLight.ambient", (vec3){0.0f, 0.0f, 0.0f});
    setVec3(lightingS.ID, "spotLight.diffuse", (vec3){1.0f, 1.0f, 1.0f});
    setVec3(lightingS.ID, "spotLight.specular", (vec3){1.0f, 1.0f, 1.0f});
    setFloat(lightingS.ID, "spotLight.constant", 1.0f);
    setFloat(lightingS.ID, "spotLight.linear", 0.09f);
    setFloat(lightingS.ID, "spotLight.quadratic", 0.032f);
    setFloat(lightingS.ID, "spotLight.cutOff", cos(glm_rad(12.5f)));
    setFloat(lightingS.ID, "spotLight.outerCutOff", cos(glm_rad(15.0f)));

    // NOTE: From previous chapters this is with all the individually different lighting solutions
    // setVec3(lightingS.ID, "objectColor", (vec3){1.0f, 0.5f, 0.31f});
    // setVec3(lightingS.ID, "light.ambient", (vec3){0.2f, 0.2f, 0.2f});
    // setVec3(lightingS.ID, "light.diffuse", (vec3){0.5f, 0.5f, 0.5f});
    // // setVec3(lightingS.ID, "light.ambient", ambientColor);
    // // setVec3(lightingS.ID, "light.diffuse", diffuseColor);
    // setVec3(lightingS.ID, "light.specular", (vec3){1.0f, 1.0f, 1.0f});
    // // setVec3(lightingS.ID, "light.position", lightPos);
    // setVec3(lightingS.ID, "light.position", Camera->Position);
    // setVec3(lightingS.ID, "light.direction", Camera->Front);
    // setFloat(lightingS.ID, "light.cutOff", cos(glm_rad(12.5f)));
    // setFloat(lightingS.ID, "light.outerCutOff", cos(glm_rad(17.5f)));
    // setFloat(lightingS.ID, "light.constant", 1.0f);
    // setFloat(lightingS.ID, "light.linear", 0.09f);
    // setFloat(lightingS.ID, "light.quadratic", 0.032f);
    // // setVec3(lightingS.ID, "light.direction", (vec3){-0.2f, -1.0f, -0.3f});
    // setVec3(lightingS.ID, "viewPos", Camera->Position);
    // setVec3(lightingS.ID, "material.ambient", (vec3){1.0f, 0.5f, 0.31f});
    // // setVec3(lightingS.ID, "material.diffuse", (vec3){1.0f, 0.5f, 0.31f});
    // setVec3(lightingS.ID, "material.specular", (vec3){0.5f, 0.5f, 0.5});
    // setFloat(lightingS.ID, "material.shininess", 32.0f);
    //


    // view/projection transformation
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    glm_perspective(glm_rad(Camera->Zoom), (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100.0f, projection);
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    Camera_GetViewMatrix(view);
    setMat4(lightingS.ID, "projection", projection);
    setMat4(lightingS.ID, "view", view);

    // world transformation
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    setMat4(lightingS.ID, "model", model);
    
    // bind diffuse map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);

    // bind specular map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);

    // render the cube
    // glBindVertexArray(cubeVAO);
    // glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(cubeVAO);
    for (unsigned int i = 0; i< 10; i++)
    {
      glm_mat4_identity(model);
      glm_translate(model, cubePositions[i]);
      float angle = 20.0f * i;
      glm_rotate(model, glm_rad(angle), (vec3){1.0f, 0.3f, 0.5f});
      setMat4(lightingS.ID, "model", model);

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }


    // draw the lamp
    glUseProgram(lightCubeS.ID);
    setMat4(lightCubeS.ID, "projection", projection);
    setMat4(lightCubeS.ID, "view", view);

    glBindVertexArray(lightCubeVAO);
    for (unsigned int i = 0; i < 4; i++) 
    {
      glm_mat4_identity(model);
      glm_translate(model, pointLightPositions[i]);
      glm_scale(model, (vec3){0.2f, 0.2f, 0.2f});
      setMat4(lightCubeS.ID, "model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteVertexArrays(1, &lightCubeVAO);
  glDeleteBuffers(1, &VBO);
  FreeShader(lightingS.ID);
  FreeShader(lightCubeS.ID);
  FreeCamera();

  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow* _window, int width, int height) 
{
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) 
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
  {
    glfwSetWindowShouldClose(window, 1);
  }
  if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    Camera_ProcessKeyboard(FORWARD, deltaTime);
  if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    Camera_ProcessKeyboard(BACKWARD, deltaTime);
  if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    Camera_ProcessKeyboard(LEFT, deltaTime);
  if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    Camera_ProcessKeyboard(RIGHT, deltaTime);
}

