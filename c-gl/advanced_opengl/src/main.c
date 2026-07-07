// #include <glad/glad.h>
// #include "glad.c"
#include "renderer.c"
#include <GLFW/glfw3.h>
#include <stdlib.h>

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
void sort_float_array(float array[], int array_size, float next_distance);
int gl_initialization();
GLFWwindow* gl_initialize_window();
uint32_t load_texture(const char *path);
uint32_t load_cubemap(const char *faces[]);

const uint32_t SCR_WIDTH = 800;
const uint32_t SCR_HEIGHT = 600;

float last_x = (float)SCR_WIDTH / 2.0;
float last_y = (float)SCR_HEIGHT / 2.0;
uint8_t first_mouse = 1;

float delta_time = 0.0f;
float last_frame = 0.0f;


typedef struct {
  vec3 position;
  float distance_from_camera;
} PositionFromCamera; 

void sort_position_cam(PositionFromCamera array[], int array_size);

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
  // glEnable(GL_STENCIL_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  Shader shader = createShader("shaders/depth_test.vs", "shaders/depth_test.fs");
  Shader colorShader = createShader("shaders/depth_test.vs", "shaders/stencil_shader.fs");
  Shader framebufferShader = createShader("shaders/framebuffer_test.vs", "shaders/framebuffer_test.fs");
  Shader skyboxShader = createShader("shaders/skybox_shader.vs", "shaders/skybox_shader.fs");

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

  float transparent_verts[] = {
    // positions        //texture coords
    0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
    0.0f,-0.5f, 0.0f, 0.0f, 1.0f,
    1.0f,-0.5f, 0.0f, 1.0f, 1.0f,

    0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
    1.0f,-0.5f, 0.0f, 1.0f, 1.0f,
    1.0f, 0.5f, 0.0f, 1.0f, 0.0f,
  };

  vec3 vegetation_positions[] = {
    {-1.5f, 0.0f, -0.48f},
    {1.5f, 0.0f, 0.51f},
    {0.0f, 0.0f, 0.70f},
    {-0.3f, 0.0f, -2.3f},
    {0.5f, 0.0f, -0.6f},
  };

  float quadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
  };	

  float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
  };

  PositionFromCamera vegetation_position_structs[] = {
    {{vegetation_positions[0][0], vegetation_positions[0][1], vegetation_positions[0][2]}, 0.0f},
    {{vegetation_positions[1][0], vegetation_positions[1][1], vegetation_positions[1][2]}, 0.0f},
    {{vegetation_positions[2][0], vegetation_positions[2][1], vegetation_positions[2][2]}, 0.0f},
    {{vegetation_positions[3][0], vegetation_positions[3][1], vegetation_positions[3][2]}, 0.0f},
    {{vegetation_positions[4][0], vegetation_positions[4][1], vegetation_positions[4][2]}, 0.0f},
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

  vertex_holder vegetation;
  glGenVertexArrays(1, &vegetation.VAO);
  glGenBuffers(1, &vegetation.VBO);
  glBindVertexArray(vegetation.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, vegetation.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(transparent_verts), &transparent_verts, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, vert_size * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vert_size * sizeof(float), (void*)(3 * sizeof(float)));
  glBindVertexArray(0);

  vertex_holder quad;
  glGenVertexArrays(1, &quad.VAO);
  glGenBuffers(1, &quad.VBO);
  glBindVertexArray(quad.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, quad.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  // TODO: setup the skybox VAO and VBO;
  vertex_holder skybox;
  glGenVertexArrays(1, &skybox.VAO);
  glGenBuffers(1, &skybox.VBO);
  glBindVertexArray(skybox.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, skybox.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glBindVertexArray(0);

  uint32_t cube_texture = load_texture("assets/marble.jpg");
  uint32_t floor_texture = load_texture("assets/metal.png");
  uint32_t grass_texture = load_texture("assets/grass.png");
  uint32_t glass_texture = load_texture("assets/blending_transparent_window.png");
  uint32_t container_texture = load_texture("assets/container.jpg");

  const char *faces[] = {
    "assets/right.jpg", 
    "assets/left.jpg", 
    "assets/top.jpg",
    "assets/bottom.jpg",
    "assets/front.jpg",
    "assets/back.jpg"
  };
  uint32_t cube_map_texture_id = load_cubemap(faces);

  // load textures need to get
  glUseProgram(shader.ID);
  setInt(shader.ID, "texture1", 0);

  glUseProgram(framebufferShader.ID);
  setInt(framebufferShader.ID, "screenTexture", 0);

  glUseProgram(skyboxShader.ID);
  setInt(skyboxShader.ID, "skybox", 0);

  uint32_t fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  uint32_t fbo_texture;
  glGenTextures(1, &fbo_texture);
  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // attach texture to framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);

  uint32_t rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  // attach
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    // glGetError();
    printf("[OPENGL_ERROR]: FrameBuffer %i could not be completed\n", fbo);
  } else {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  CreateCamera((vec3){0.5f, 0.0f, 5.0f});

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  while(!glfwWindowShouldClose(window))
  {
    float current_frame = (float)glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    process_input(window);


    for(uint32_t i = 0; i < 5; i++)
    {
      vegetation_position_structs[i].distance_from_camera = glm_vec3_distance(Camera->Position, vegetation_position_structs[i].position);
      sort_position_cam(vegetation_position_structs, 5);
    }

    // glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // glDepthMask(GL_FALSE);
    // glUseProgram(skyboxShader.ID);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glUseProgram(shader.ID);
    // RenderCamera(shader, Camera);
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    Camera_GetViewMatrix(view);
    glm_perspective(glm_rad(Camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f, projection);
    setMat4(colorShader.ID, "view", view);
    setMat4(colorShader.ID, "projection", projection);

    // mat3 temp_x = mat3(temp_v); 


    // glUseProgram(shader.ID);
    // setMat4(shader.ID, "view", view);
    // setMat4(shader.ID, "projection", projection);

    glUseProgram(shader.ID);
    // glStencilMask(0x00);
    //
    // // floor
    glBindVertexArray(plane.VAO);
    glBindTexture(GL_TEXTURE_2D, floor_texture);
    glm_mat4_identity(model);
    setMat4(shader.ID, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // glBindVertexArray(vegetation.VAO);
    // glBindTexture(GL_TEXTURE_2D, glass_texture);
    // for(uint32_t i = 0; i < 5; i++) {
    //   glm_mat4_identity(model);
    //   glm_translate(model, vegetation_position_structs[i].position);
    //   setMat4(shader.ID, "model", model);    
    //   glDrawArrays(GL_TRIANGLES, 0, 6);
    // }
    // glBindVertexArray(0);
    //
    // glStencilFunc(GL_ALWAYS, 1, 0xFF);
    // glStencilMask(0xFF);
    // // pass 1
    glBindVertexArray(cube.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, container_texture);
    glm_translate(model, (vec3){-1.0f, 0.0f, -1.0f});
    setMat4(shader.ID, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glm_mat4_identity(model);
    glm_translate(model, (vec3){2.0f, 0.0f, 0.0f});
    setMat4(shader.ID, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // skybox
    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxShader.ID);


    mat4 new_view = GLM_MAT4_IDENTITY_INIT;
    Camera_GetViewMatrix(new_view);
    mat4 temp_v = GLM_MAT4_IDENTITY_INIT;
    mat3 temp_x = GLM_MAT3_IDENTITY_INIT;
    Camera_GetViewMatrix(temp_v);
    glm_mat4_pick3(temp_v, temp_x);
    glm_mat4_identity(temp_v);
    glm_mat4_ins3(temp_x, temp_v);
    glm_mat4_copy(temp_v, new_view);

    setMat4(skyboxShader.ID, "view", new_view);
    setMat4(skyboxShader.ID, "projection", projection);
    glBindVertexArray(skybox.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glDisable(GL_DEPTH_TEST);
    // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);
    //
    // glUseProgram(framebufferShader.ID);
    // glBindVertexArray(quad.VAO);
    // glBindTexture(GL_TEXTURE_2D, fbo_texture);
    // glDrawArrays(GL_TRIANGLES, 0, 6);

    //                       
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    // glStencilMask(0x00);
    // glDisable(GL_DEPTH_TEST);
    // glUseProgram(colorShader.ID);
    // float scale = 1.1f;
    //
    // // bind vertex for cubes
    // glBindVertexArray(cube.VAO);
    // // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, cube_texture);
    // glm_mat4_identity(model);
    // // cube 1
    // glm_translate(model, (vec3){-1.0f, 0.0f, -1.0f});
    // glm_scale(model, (vec3){scale, scale, scale});
    // setMat4(colorShader.ID, "model", model);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    // // cube 2
    // glm_mat4_identity(model);
    // glm_translate(model, (vec3){2.0f, 0.0f, 0.0f});
    // glm_scale(model, (vec3){scale, scale, scale});
    // setMat4(colorShader.ID, "model", model);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    // glBindVertexArray(0);// unbind
    //                      
    // glStencilMask(0xFF);
    // glStencilFunc(GL_ALWAYS, 0, 0xFF);
    // glEnable(GL_DEPTH_TEST);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &cube.VAO);
  glDeleteVertexArrays(1, &plane.VAO);
  glDeleteBuffers(1, &cube.VAO);
  glDeleteBuffers(1, &plane.VAO);
  glDeleteBuffers(1, &fbo);
  glDeleteBuffers(1, &skybox.VAO);
  glDeleteBuffers(1, &skybox.VBO);

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

int comp(const void* a, const void* b)
{
  return (*(int*)a - *(int*)b);
}

void sort_float_array(float array[], int array_size, float next_distance)
{
  qsort(array, array_size, sizeof(float), comp);
}

int comp_position_from_camera(const void* a, const void* b)
{
  return (((PositionFromCamera*)b)->distance_from_camera - ((PositionFromCamera*)a)->distance_from_camera);
}

void sort_position_cam(PositionFromCamera array[], int array_size)
{
  qsort(array, array_size, sizeof(PositionFromCamera), comp_position_from_camera);
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

uint32_t load_cubemap(const char *faces[])
{
  uint32_t cubemap_texture_id;
  glGenTextures(1, &cubemap_texture_id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture_id);
  // setup the cubemap faces
  int width, height, nrChannels;
  const int texture_faces_size = 6;
  for (uint32_t i = 0; i < texture_faces_size; i++)
  {
    unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      printf("CUBEMAP TEXTURE FAILED TO LOAD AT %s\n", faces[i]);
      stbi_image_free(data);
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return cubemap_texture_id;
}
