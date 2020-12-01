/*TP_FRAG_SHADER_HEADER*/

struct Material
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  float alpha;

  float roughness;
  float metalness;

  float useDiffuse;
  float useNdotL;
  float useAttenuation;
  float useShadow;
  float useLightMask;
  float useReflection;

  float ambientScale;
  float diffuseScale;
  float specularScale;
};

struct Light
{
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float diffuseScale;

  float constant;
  float linear;
  float quadratic;

  vec2 spotLightUV;
  vec2 spotLightWH;
};

struct LightResult
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform sampler2D ambientTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D alphaTexture;
uniform sampler2D bumpTexture;
uniform sampler2D spotLightTexture;

uniform Material material;

uniform vec2 txlSize;
uniform float discardOpacity;

uniform mat4 m;
uniform mat4 mv;
uniform mat4 mvp;
uniform mat4 v;

uniform vec3 cameraOrigin_world;

/*TP_GLSL_IN_F*/vec3 fragPos_world;

/*TP_GLSL_IN_F*/vec3 outNormal;
/*TP_GLSL_IN_F*/vec3 outTangent;
/*TP_GLSL_IN_F*/vec3 outBitangent;

/*TP_GLSL_IN_F*/vec2 uv_tangent;
vec3 fragPos_tangent;
vec3 cameraOrigin_tangent;

vec3 F0;
vec3 surfaceToCamera;
vec3 albedo;

/*TP_GLSL_IN_F*/vec3 normal_view;


/*LIGHT_FRAG_VARS*/

const int shadowSamples=/*TP_SHADOW_SAMPLES*/;
const float totalSadowSamples=float(((shadowSamples*2)+1) * ((shadowSamples*2)+1));

const float pi = 3.14159265358979323846264338327950288419716939937510f;

/*TP_GLSL_GLFRAGCOLOR_DEF*/
/*TP_WRITE_FRAGMENT*/

//See MaterialShader.cpp for documentation.

//##################################################################################################
float sampleShadowMap2D(sampler2D shadowMap, vec2 coords, float compareLight, float compareDark)
{
  return smoothstep(compareLight, compareDark, /*TP_GLSL_TEXTURE_2D*/(shadowMap, coords.xy).r);
}

//##################################################################################################
float sampleShadowMapLinear2D(sampler2D shadowMap, vec2 coords, float compareLight, float compareDark)
{
  vec2 pixelPos = coords/txlSize + vec2(0.5);
  vec2 fracPart = fract(pixelPos);
  vec2 startTxl = (pixelPos - fracPart) * txlSize;

  float blTxl = sampleShadowMap2D(shadowMap, startTxl, compareLight, compareDark);
  float brTxl = sampleShadowMap2D(shadowMap, startTxl + vec2(txlSize.x, 0.0), compareLight, compareDark);
  float tlTxl = sampleShadowMap2D(shadowMap, startTxl + vec2(0.0, txlSize.y), compareLight, compareDark);
  float trTxl = sampleShadowMap2D(shadowMap, startTxl + txlSize, compareLight, compareDark);

  float mixA = mix(blTxl, tlTxl, fracPart.y);
  float mixB = mix(brTxl, trTxl, fracPart.y);

  return mix(mixA, mixB, fracPart.x);
}

//##################################################################################################
float sampleShadowMap3D(sampler3D shadowMap, vec2 coords, float compareLight, float compareDark, float level)
{
  return smoothstep(compareLight, compareDark, /*TP_GLSL_TEXTURE_3D*/(shadowMap, vec3(coords.xy, level)).r);
}

//##################################################################################################
float sampleShadowMapLinear3D(sampler3D shadowMap, vec2 coords, float compareLight, float compareDark, float level)
{
  vec2 pixelPos = coords/txlSize + vec2(0.5);
  vec2 fracPart = fract(pixelPos);
  vec2 startTxl = (pixelPos - fracPart) * txlSize;

  float blTxl = sampleShadowMap3D(shadowMap, startTxl, compareLight, compareDark, level);
  float brTxl = sampleShadowMap3D(shadowMap, startTxl + vec2(txlSize.x, 0.0), compareLight, compareDark, level);
  float tlTxl = sampleShadowMap3D(shadowMap, startTxl + vec2(0.0, txlSize.y), compareLight, compareDark, level);
  float trTxl = sampleShadowMap3D(shadowMap, startTxl + txlSize, compareLight, compareDark, level);

  float mixA = mix(blTxl, tlTxl, fracPart.y);
  float mixB = mix(brTxl, trTxl, fracPart.y);

  return mix(mixA, mixB, fracPart.x);
}

//##################################################################################################
vec3 lightPosToTexture(vec4 fragPos_light, vec4 offset, mat4 proj)
{
  vec4 fp = proj * (fragPos_light+offset);
  return vec3((vec3(0.5, 0.5, 0.5) * (fp.xyz / fp.w)) + vec3(0.5, 0.5, 0.5));
}

//##################################################################################################
float calcBiasedDepth(float bias, float z)
{
  bias = 1.0-bias;
  bias = clamp(bias, 0.4, 1.0);
  bias *= 0.0001;

  return min(z-bias,1.0);
}

//##################################################################################################
float calcGGXDist(vec3 norm, vec3 halfV, float roughness2)
{
  float normDotLight = clamp(dot(norm, halfV), 0.0, 1.0) ;
  float normDotLight2 = normDotLight * normDotLight;
  float fDen = normDotLight2 * roughness2 + (1.0-normDotLight2);

  return roughness2/(pi*fDen * fDen);
}

float chiGGX(float f)
{
  return f > 0.0 ? 1.0 : 0.0 ;
}

//##################################################################################################
float calcGGXGeom(vec3 surfaceToCamera, vec3 norm, vec3 lightViewHalfVector, float roughness2)
{
  float fViewerDotLightViewHalf = clamp(dot(surfaceToCamera, lightViewHalfVector), 0.0, 1.0) ;
  //float fChi = step(0.0, fViewerDotLightViewHalf / clamp(dot(surfaceToCamera, norm), 0.0, 1.0));
  float fChi = chiGGX(fViewerDotLightViewHalf / clamp(dot(surfaceToCamera, norm), 0.0, 1.0));
  fViewerDotLightViewHalf *= fViewerDotLightViewHalf;
  float fTan2 = (1.0 - fViewerDotLightViewHalf) / fViewerDotLightViewHalf;

  return (fChi * 2.0) / (1.0 + sqrt(1.0 + roughness2 * fTan2)) ;
}


float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

//##################################################################################################
vec3 calcFresnel(vec3 halfV, vec3 norm, vec3 F0)
{
  return F0 + (1.0 - F0) * pow(1.0-max(0.0, dot(halfV, norm)), 5.0);
}

//##################################################################################################
LightResult directionalLight(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec3 fragPos_light)
{
  LightResult r;

  // Ambient
  r.ambient = light.ambient;

  // Diffuse
  float cosTheta = clamp(dot(norm, -lightDirection_tangent), 0.0, 1.0);
  float diff = cosTheta*light.diffuseScale;
  r.diffuse = light.diffuse * diff;

  // Specular
  vec3 incidenceVector = lightDirection_tangent;
  vec3 reflectionVector = reflect(incidenceVector, norm);
  vec3 surfaceToCamera = normalize(cameraOrigin_tangent-fragPos_tangent);
  float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
  float specularCoefficient = pow(cosAngle, material.shininess);
  r.specular = specularCoefficient * light.specular;

  // Shadow
  float shadow = 0.0;
  float bias = dot(norm, -lightDirection_tangent);
  if(bias>0.0 && fragPos_light.z>0.0 && fragPos_light.z<1.0)
  {
    float biasedDepth = calcBiasedDepth(bias, fragPos_light.z);

    for(int x = -shadowSamples; x <= shadowSamples; ++x)
    {
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 coord = fragPos_light.xy + (vec2(x, y)*txlSize);
        if(coord.x<0.0 || coord.x>1.0 || coord.y<0.0 || coord.y>1.0)
        {
          shadow += 1.0;
        }
        else
        {
          shadow += sampleShadowMapLinear2D(lightTexture, coord, biasedDepth, fragPos_light.z);
        }
      }
    }
    shadow /= totalSadowSamples;
  }
  else
  {
    shadow=1.0;
  }

  r.diffuse *= shadow;
  r.specular *= shadow;

  return r;
}

////##################################################################################################
//float spotLightSampleShadow2D(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec3 fragPos_light)
//{
//  float shadow = 0.0;

//  float bias = dot(norm, -lightDirection_tangent);
//  if(bias>0.0 && fragPos_light.z>0.0 && fragPos_light.z<1.0)
//  {
//    float biasedDepth = calcBiasedDepth(bias, fragPos_light.z);

//    for(int x = -shadowSamples; x <= shadowSamples; ++x)
//    {
//      for(int y = -shadowSamples; y <= shadowSamples; ++y)
//      {
//        vec2 coord = fragPos_light.xy + (vec2(x, y)*txlSize);
//        if(coord.x<0.0 || coord.x>1.0 || coord.y<0.0 || coord.y>1.0)
//        {
//          shadow += 0.0;
//        }
//        else
//        {
//          shadow += sampleShadowMapLinear2D(lightTexture, coord, biasedDepth, fragPos_light.z);
//        }
//      }
//    }
//  }
//  return shadow;

//  vec2 spotTexCoord = (fragPos_light.xy*light.spotLightWH) + light.spotLightUV;
//  return mix(1.0, /*TP_GLSL_TEXTURE_2D*/(spotLightTexture, spotTexCoord).x, material.useLightMask) * shadow;
//}

//##################################################################################################
float maskLight(Light light, vec3 uv, float shadow)
{
  float mask = 0.0;
  if(uv.x>=0.0 && uv.x<=1.0 && uv.y>=0.0 && uv.y<=1.0)
    mask = /*TP_GLSL_TEXTURE_2D*/(spotLightTexture, (uv.xy*light.spotLightWH) + light.spotLightUV).x;

  return mix(1.0, mask, material.useLightMask) * shadow;
}

//##################################################################################################
float spotLightSampleShadow2D(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec3 uv)
{
  float shadow = totalSadowSamples;
  float bias = dot(norm, -lightDirection_tangent);
  if(bias>0.0 && uv.z>0.0 && uv.z<1.0)
  {
    float biasedDepth = calcBiasedDepth(bias, uv.z);

    for(int x = -shadowSamples; x <= shadowSamples; ++x)
    {
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 coord = uv.xy + (vec2(x, y)*txlSize);
        if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
          shadow -= 1.0-sampleShadowMapLinear2D(lightTexture, coord, biasedDepth, uv.z);
      }
    }
  }
  return maskLight(light, uv, shadow);
}

//##################################################################################################
float spotLightSampleShadow3D(vec3 norm, Light light, vec3 lightDirection_tangent, sampler3D lightTexture, vec3 uv, float level)
{
  float shadow = totalSadowSamples;
  float bias = dot(norm, -lightDirection_tangent);
  if(bias>0.0 && uv.z>0.0 && uv.z<1.0)
  {
    float biasedDepth = calcBiasedDepth(bias, uv.z);

    for(int x = -shadowSamples; x <= shadowSamples; ++x)
    {
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 coord = uv.xy + (vec2(x, y)*txlSize);
        if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
          shadow -= 1.0-sampleShadowMapLinear3D(lightTexture, coord, biasedDepth, uv.z, level);
      }
    }
  }
  return maskLight(light, uv, shadow);
}

//##################################################################################################
LightResult spotLight(vec3 norm, Light light, vec3 lightDirection_tangent, vec3 fragPos_light, float shadow)
{
  LightResult r;

  r.specular = vec3(0,0,0);
  r.ambient = vec3(0,0,0);

  float roughness = max(0.001, material.roughness);
  float roughness2 = roughness*roughness;

  vec3 surfaceToLight  = -lightDirection_tangent;
  vec3 halfV = normalize(surfaceToCamera + surfaceToLight);

  // Attenuation
  float distance    = length(light.position - fragPos_world);
  float attenuation = mix(1.0, 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)), material.useAttenuation);
  vec3  radiance    = light.diffuseScale * light.diffuse  * attenuation * 5.0;

  // Cook-Torrance BRDF
  float NDF = calcGGXDist(norm, halfV, roughness2);
  float G   = GeometrySmith(norm, surfaceToCamera, surfaceToLight, roughness);
  vec3  F   = calcFresnel(halfV, norm, F0);

  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - material.metalness;
  kD = mix(vec3(1.0,1.0,1.0), kD, material.useDiffuse);

  vec3  numerator   = NDF * G * F;
  float denominator = 4.0 * max(dot(norm, surfaceToCamera), 0.0) * max(dot(norm, surfaceToLight), 0.0);
  vec3  specular    = (material.specular*material.specularScale) * numerator / max(denominator, 0.001);

  // add to outgoing radiance Lo
  float NdotL = mix(1.0, max(dot(norm, surfaceToLight), 0.0), material.useNdotL);
  r.diffuse = (kD * albedo/* / pi*/ + specular) * radiance * NdotL * shadow;

  return r;
}

//##################################################################################################
mat3 transposeMat3(mat3 inMatrix)
{
  vec3 i0 = inMatrix[0];
  vec3 i1 = inMatrix[1];
  vec3 i2 = inMatrix[2];

  return mat3(vec3(i0.x, i1.x, i2.x), vec3(i0.y, i1.y, i2.y), vec3(i0.z, i1.z, i2.z));
}

//##################################################################################################
void main()
{
  vec3  ambientTex = /*TP_GLSL_TEXTURE_2D*/( ambientTexture, uv_tangent).xyz;
  vec3  diffuseTex = /*TP_GLSL_TEXTURE_2D*/( diffuseTexture, uv_tangent).xyz;
  vec3 specularTex = /*TP_GLSL_TEXTURE_2D*/(specularTexture, uv_tangent).xyz;
  float   alphaTex = /*TP_GLSL_TEXTURE_2D*/(   alphaTexture, uv_tangent).x;
  vec3     bumpTex = /*TP_GLSL_TEXTURE_2D*/(    bumpTexture, uv_tangent).xyz;

  vec3 norm = normalize(bumpTex*2.0-1.0);

  vec3 ambient  = vec3(0,0,0);
  vec3 diffuse  = vec3(0,0,0);
  vec3 specular = vec3(0,0,0);

  // Calculate the TBN matrix used to transform between world and tangent space.
  vec3 t = normalize(outTangent  );
  vec3 n = normalize(outNormal   );
  vec3 b = cross(n, t);
  t = cross(n, b);

  mat3 m3 = mat3(m);
  mat3 TBN = mat3(m3*t, m3*b, m3*n);
  mat3 invTBN = transposeMat3(TBN);
  mat3 TBNv = mat3(v) * TBN;

  cameraOrigin_tangent = invTBN * cameraOrigin_world;
  fragPos_tangent = invTBN * fragPos_world;

  F0 = mix(vec3(0.04), material.diffuse, material.metalness);
  surfaceToCamera = normalize(cameraOrigin_tangent-fragPos_tangent);
  albedo = (diffuseTex+material.diffuse) * material.diffuseScale;

  /*LIGHT_FRAG_CALC*/

  ambient  *= (ambientTex+material.ambient)   * material.ambientScale;
  diffuse  *= (diffuseTex+material.diffuse)   * material.diffuseScale;
  specular *= (specularTex+material.specular) * material.specularScale;

  float alpha = material.alpha*alphaTex;
  vec3 materialSpecular = material.specular;
  float shininess = material.roughness;

  //ambient = vec3(0.03) * albedo;
  //diffuse = ambient + diffuse;
  //diffuse = diffuse / (diffuse + vec3(1.0));
  //diffuse = pow(diffuse, vec3(1.0/2.2));

  //ambient = vec3(0,0,0);

  vec3 normal = TBNv*norm;

  writeFragment(ambient, diffuse, specular, normal, alpha, materialSpecular, shininess);
}
