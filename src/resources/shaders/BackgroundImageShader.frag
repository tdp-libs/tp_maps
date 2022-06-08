/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 coord_tex;

uniform sampler2D textureSampler;

const float discardOpacity=0.8;

uniform mat4 matrix;

uniform float rotationFactor;

/*TP_GLSL_GLFRAGCOLOR_DEF*/
/*TP_WRITE_FRAGMENT*/

//##################################################################################################
void main()
{
  vec3 ambient = /*TP_GLSL_TEXTURE_2D*/(textureSampler, coord_tex).xyz;

  //Note: GammaCorrection
  ambient = pow(ambient, vec3(2.2));

  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);
  vec3 normal = vec3(0.0,0.0,1.0);
  float alpha = 1.0;
  vec3 materialSpecular = vec3(0.0);
  float shininess = 0.0;

  writeFragment(ambient, diffuse, specular, normal, alpha, materialSpecular, shininess);
}
