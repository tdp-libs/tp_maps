#pragma replace TP_VERT_SHADER_HEADER
#define TP_GLSL_IN_V
#define TP_GLSL_OUT_V

TP_GLSL_IN_V vec3 inVertex;
TP_GLSL_IN_V vec2 inTexture;

TP_GLSL_OUT_V vec3 vertex;
TP_GLSL_OUT_V vec2 texCoord;

uniform mat4 m;
uniform mat4 mvp;
uniform mat3 uvMatrix;

void main()
{
  gl_Position = mvp * vec4(inVertex, 1.0);
  vertex = vec4(m * vec4(inVertex, 1.0)).xyz;

  vec3 uv = uvMatrix * vec3(inTexture, 1.0f);
  texCoord = uv.xy;
}
