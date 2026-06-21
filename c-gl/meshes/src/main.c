#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include <mylib/camera.h>
#include <mylib/shader_m.h>
#include <mylib/model.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

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
  struct Shader shader = createShader("shaders/colors_vert.vs", "shaders/colors_frag.fs");
  // struct Shader lightCubeS = createShader("shaders/light_cube_vert.vs", "shaders/light_cube_frag.fs");
  // Model ourModel = loadModel("assets/test_cube.glb");
  // Model ourModel = loadModel("assets/test_backpack.gltf");
  Model ourModel = loadModel("assets/mini_mech_base.gltf");

  // Clear color prior to loop. if changing then we would add to loop
  glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

  CreateCamera((vec3){0.5f, 0.0f, 5.0f});

  while (!glfwWindowShouldClose(window))
  {

    // per-frame time logic
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    processInput(window);

    // render
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader.ID);
    mat4 projection = GLM_MAT4_ZERO_INIT;
    glm_perspective(glm_rad(Camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f, projection);
    mat4 view =GLM_MAT4_ZERO_INIT;
    Camera_GetViewMatrix(view);
    setMat4(shader.ID, "projection", projection);
    setMat4(shader.ID, "view", view);

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
    glm_scale(model, (vec3){1.0f, 1.0f, 1.0f});
    setMat4(shader.ID, "model", model);

    modelDraw(ourModel, shader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  FreeShader(shader.ID);
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

