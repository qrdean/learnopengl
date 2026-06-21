#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cglm/cglm.h>

#include <mylib/shader_m.h>
#include <mylib/camera.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stddef.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void process_input(GLFWwindow *window);
void RenderCamera(Shader shader, struct CAMERA *Camera);
int gl_initialization();
GLFWwindow* gl_initialize_window();
uint32_t load_texture(const char *path);

const uint32_t SCR_WIDTH = 800;
const uint32_t SCR_HEIGHT = 600;

float last_x = (float)SCR_WIDTH / 2.0;
float last_y = (float)SCR_HEIGHT / 2.0;
uint8_t first_mouse = 1;

float delta_time = 0.0f;
float last_frame = 0.0f;

typedef struct vertex_holder
{
  uint32_t VAO, VBO;
} vertex_holder;

vertex_holder bind_vertices(float *vertices, int vert_size);

int main()
{
  GLFWwindow* window = gl_initialize_window();
  if (window == NULL)
    return -1;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  Shader shader = createShader("shaders/depth_test.vs", "shaders/depth_test.fs");

  float cubeVertices[] = {
      // positions          // texture Coords
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

  float planeVertices[] = {
      // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
       5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
      -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
      -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

       5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
      -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
       5.0f, -0.5f, -5.0f,  2.0f, 2.0f								
  };

  vertex_holder cube;// = bind_vertices(cubeVertices, 5);
                     //
  int vert_size = 5;
  glGenVertexArrays(1, &cube.VAO);
  glGenBuffers(1, &cube.VBO);
  glBindVertexArray(cube.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
  // Should figure out how to encapsulate this? cubeaybe it's just a size thing and have multiple functions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vert_size * sizeof(float), (void*)(3 * sizeof(float)));
  glBindVertexArray(0);

  vertex_holder plane; // = bind_vertices(planeVertices, 5);
  glGenVertexArrays(1, &plane.VAO);
  glGenBuffers(1, &plane.VBO);
  glBindVertexArray(plane.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, plane.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
  // Should figure out how to encapsulate this? planeaybe it's just a size thing and have multiple functions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vert_size * sizeof(float), (void*)(3 * sizeof(float)));
  glBindVertexArray(0);

  uint32_t cube_texture = load_texture("assets/marble.jpg");
  uint32_t floor_texture = load_texture("assets/metal.png");

  // load textures need to get
  glUseProgram(shader.ID);
  setInt(shader.ID, "texture1", 0);

  CreateCamera((vec3){0.5f, 0.0f, 5.0f});

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  while(!glfwWindowShouldClose(window))
  {
    float current_frame = (float)glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    process_input(window);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader.ID);
    // RenderCamera(shader, Camera);
    mat4 projection = GLM_MAT4_ZERO_INIT;
    glm_perspective(glm_rad(Camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f, projection);
    mat4 view = GLM_MAT4_ZERO_INIT;
    Camera_GetViewMatrix(view);
    setMat4(shader.ID, "projection", projection);
    setMat4(shader.ID, "view", view);

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    // bind vertex for cubes
    glBindVertexArray(cube.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cube_texture);
    // cube 1
    glm_translate(model, (vec3){-1.0f, 0.0f, -1.0f});
    setMat4(shader.ID, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // cube 2
    glm_mat4_identity(model);
    glm_translate(model, (vec3){2.0f, 0.0f, 0.0f});
    setMat4(shader.ID, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // floor
    glBindVertexArray(plane.VAO);
    glBindTexture(GL_TEXTURE_2D, floor_texture);
    glm_mat4_identity(model);
    setMat4(shader.ID, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);// unbind

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &cube.VAO);
  glDeleteVertexArrays(1, &plane.VAO);
  glDeleteBuffers(1, &cube.VAO);
  glDeleteBuffers(1, &plane.VAO);

  glfwTerminate();
  return 0;
}

GLFWwindow* gl_initialize_window()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Depth Buffering", NULL, NULL);
  if (window == NULL)
  {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return window; 
  }
  glfwMakeContextCurrent(window);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    printf("Failed to initialize GLAD\n");
    return window;
  }

  return window;
}

vertex_holder bind_vertices(float *vertices, int vert_size)
{
  vertex_holder m;
  glGenVertexArrays(1, &m.VAO);
  glGenBuffers(1, &m.VBO);
  glBindVertexArray(m.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
  // Should figure out how to encapsulate this? maybe it's just a size thing and have multiple functions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vert_size * sizeof(float), (void*)(3 * sizeof(float)));
  glBindVertexArray(0);
  return(m);
}

void RenderCamera(Shader shader, struct CAMERA *Camera)
{
  mat4 projection = GLM_MAT4_ZERO_INIT;
  glm_perspective(glm_rad(Camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f, projection);
  mat4 view = GLM_MAT4_ZERO_INIT;
  Camera_GetViewMatrix(view);
  setMat4(shader.ID, "projection", projection);
  setMat4(shader.ID, "view", view);
}

void process_input(GLFWwindow *window) 
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
  {
    glfwSetWindowShouldClose(window, 1);
  }
  if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    Camera_ProcessKeyboard(FORWARD, delta_time);
  if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    Camera_ProcessKeyboard(BACKWARD, delta_time);
  if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    Camera_ProcessKeyboard(LEFT, delta_time);
  if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    Camera_ProcessKeyboard(RIGHT, delta_time);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  Camera_ProcessMouseScroll();
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
  Camera_ProcessMouseMovement();
}

uint32_t load_texture(const char *filename)
{
  uint32_t textureId = 0;
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
