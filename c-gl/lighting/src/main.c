#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include <mylib/camera.h>
#include <mylib/shader_m.h>
#include <stdio.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

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
  float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
  };

  unsigned int cubeVAO, VBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(cubeVAO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  unsigned int lightCubeVAO;
  glGenVertexArrays(1, &lightCubeVAO);
  glBindVertexArray(lightCubeVAO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // Clear color prior to loop. if changing then we would add to loop
  glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

  // glUseProgram(lightingS.ID);
  // setVec3(lightingS.ID, "objectColor", (vec3){1.0f, 0.5f, 0.31f});
  // setVec3(lightingS.ID, "lightColor", (vec3){1.0f, 1.0f, 1.0f});

  // glUseProgram(lightCubeS.ID);
  // setVec3(lightingS.ID, "objectColor", (vec3){1.0f, 0.5f, 0.31f});
  // setVec3(lightingS.ID, "lightColor", (vec3){1.0f, 1.0f, 1.0f});

  CreateCamera((vec3){0.5f, 0.0f, 5.0f});

  vec3 lightPos = {1.2f, 1.0f, 2.0f};
  // glUseProgram(lightingS.ID);
  // setVec3(lightingS.ID, "lightPos", lightPos);

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

    glUseProgram(lightingS.ID);
    setVec3(lightingS.ID, "objectColor", (vec3){1.0f, 0.5f, 0.31f});
    setVec3(lightingS.ID, "lightColor", (vec3){1.0f, 1.0f, 1.0f});
    setVec3(lightingS.ID, "lightPos", lightPos);

    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    glm_perspective(glm_rad(Camera->Zoom), (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100.0f, projection);
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    Camera_GetViewMatrix(view);
    setMat4(lightingS.ID, "projection", projection);
    setMat4(lightingS.ID, "view", view);

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    setMat4(lightingS.ID, "model", model);

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glUseProgram(lightCubeS.ID);
    setMat4(lightCubeS.ID, "projection", projection);
    setMat4(lightCubeS.ID, "view", view);
    glm_mat4_identity(model);
    glm_translate(model, lightPos);
    glm_scale(model, (vec3){0.2f, 0.2f, 0.2f});
    setMat4(lightCubeS.ID, "model", model);

    glBindVertexArray(lightCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

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

