#ifndef tp_maps_PostShader_h
#define tp_maps_PostShader_h

#include "tp_maps/shaders/FullScreenShader.h"

namespace tp_maps
{

//##################################################################################################
//! The base class for post processing shaders.
class TP_MAPS_SHARED_EXPORT PostShader: public FullScreenShader
{
public:
  //################################################################################################
  PostShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const char* vertexShader, const char* fragmentShader);

  //################################################################################################
  ~PostShader();

  //################################################################################################
  void setReadFBO(const FBO& readFBO);

  //################################################################################################
  void setProjectionMatrix(const glm::mat4& projectionMatrix);

private:
  struct Private;
  Private* d;
};

}

#endif
