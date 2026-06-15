#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  gl_Position = projection * view * vec4(aPos, 1.0);
  FragPos = vec3(model * vec4(aPos, 1.0));
  // NOTE: inefficient should calculate on the CPU and pass this along as a uniform
  // This handles scaling by creating the "normal matrix" which is transpose of the inverse of the upper-left 3x3 of the model mat4
  // Normal = mat3(transpose(inverse(model))) * aNormal;
  Normal = aNormal;
  TexCoords = aTexCoords;
}
