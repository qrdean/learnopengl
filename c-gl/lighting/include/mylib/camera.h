#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#include <glad/glad.h>
#include <stdlib.h>

#define YAW -90.0f;
#define PITCH 0.0f;
#define SPEED 2.5f;
#define SENS 0.1f;
#define ZOOM 45.0f;

typedef enum { FORWARD, BACKWARD, LEFT, RIGHT } Camera_Movement;

struct CAMERA {
  vec3 Position;
  vec3 Front;
  vec3 Up;
  vec3 Right;
  vec3 WorldUp;

  float Yaw;
  float Pitch;

  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;
};
struct CAMERA *Camera;

void updateCameraVectors() {
  vec3 vfront = {0.0f, 0.0f, 0.0f};
  vfront[0] = cos(glm_rad(Camera->Yaw) * cos(glm_rad(Camera->Pitch)));
  vfront[1] = sin(glm_rad(Camera->Pitch));
  vfront[2] = sin(glm_rad(Camera->Yaw) * cos(glm_rad(Camera->Pitch)));
  glm_normalize_to(vfront, Camera->Front);

  vec3 tempVec = {0.0f, 0.0f, 0.0f};
  glm_vec3_cross(Camera->Front, Camera->WorldUp, tempVec);
  glm_normalize_to(tempVec, Camera->Right);
  glm_vec3_cross(Camera->Right, Camera->Front, tempVec);
  glm_normalize_to(tempVec, Camera->Up);
}

void CreateCamera(vec3 position) {
  Camera = (struct CAMERA *)malloc(sizeof(struct CAMERA));

  Camera->Front[0] = 0.0f;
  Camera->Front[1] = 0.0f;
  Camera->Front[2] = -1.0f;
  Camera->MovementSpeed = SPEED;
  Camera->MouseSensitivity = SENS;
  Camera->Zoom = ZOOM;
  Camera->Position[0] = position[0];
  Camera->Position[1] = position[1];
  Camera->Position[2] = position[2];
  Camera->WorldUp[0] = 0.0f;
  Camera->WorldUp[1] = 1.0f;
  Camera->WorldUp[2] = 0.0f;
  Camera->Yaw = YAW;
  Camera->Pitch = PITCH;
  updateCameraVectors();
}

void CreateCameraScale();

// returns the view matrix calc using Euler and Lookat
void Camera_GetViewMatrix(mat4 mat) {
  vec3 tempVec = {0.0f, 0.0f, 0.0f};
  glm_vec3_add(Camera->Position, Camera->Front, tempVec);
  glm_lookat(Camera->Position, tempVec, Camera->Up, mat);
}

void Camera_ProcessKeyboard(Camera_Movement direction, float deltaTime) {
  float velocity = Camera->MovementSpeed * deltaTime;
  if (direction == FORWARD) {
    vec3 tempVec = {0, 0, 0};
    glm_vec3_muladds(Camera->Front, velocity, tempVec);
    glm_vec3_add(Camera->Position, tempVec, Camera->Position);
  }

  if (direction == BACKWARD) {
    vec3 tempVec = {0, 0, 0};
    glm_vec3_muladds(Camera->Front, velocity, tempVec);
    glm_vec3_sub(Camera->Position, tempVec, Camera->Position);
  }

  if (direction == LEFT) {
    vec3 tempVec = {0, 0, 0};
    glm_vec3_muladds(Camera->Right, velocity, tempVec);
    glm_vec3_sub(Camera->Position, tempVec, Camera->Position);
  }

  if (direction == RIGHT) {
    vec3 tempVec = {0, 0, 0};
    glm_vec3_muladds(Camera->Right, velocity, tempVec);
    glm_vec3_add(Camera->Position, tempVec, Camera->Position);
  }
}

void Camera_ProcessMouseMovement()
{}

void Camera_ProcessMouseScroll()
{}

void FreeCamera() { free(Camera); }

#endif
