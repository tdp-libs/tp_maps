#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_GLFRAGCOLOR

uniform vec4 pickingID;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

void main()
{
  TP_GLSL_GLFRAGCOLOR = pickingID;
}
