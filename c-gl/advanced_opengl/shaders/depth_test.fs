#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

in vec3 Normal;
in vec3 Position;

uniform sampler2D texture1;

uniform vec3 cameraPos;
uniform samplerCube skybox;


float near = 0.1;
float far = 100.0;

float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0;
  return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
  // FragColor = texture(texture1, TexCoords);
  // float depth = LinearizeDepth(gl_FragCoord.z) / far;
  // FragColor = vec4(vec3(depth), 1.0);
  // vec4 texColor = texture(texture1, TexCoords);
  // if (texColor.a < 0.1)
  //  discard;
  // FragColor = texColor;

  // FragColor = texture(texture1, TexCoords);

  vec3 I = normalize(Position - cameraPos);
  vec3 R = reflect(I, normalize(Normal));
  FragColor = vec4(texture(skybox, R).rgb, 1.0);
}
