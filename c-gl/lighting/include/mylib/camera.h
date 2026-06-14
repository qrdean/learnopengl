#ifndef CAMERA_H
#define CAMERA_H

#include <stdlib.h>

enum Camera_Movement
{
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};

#define YAW  -90.0f;
#define PITCH  0.0f;
#define SPEED  2.5f;
#define SENS  0.1f;
#define ZOOM  45.0f;

struct CAMERA {
  int a;
};
struct CAMERA* Camera;

void CreateCamera()
{
  Camera = (struct CAMERA*)malloc(sizeof(struct CAMERA));
}

void FreeCamera()
{
  free(Camera);
}

#endif
