#ifndef tp_maps_PostSSRLayer_h
#define tp_maps_PostSSRLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostSSRLayer: public PostLayer
{
public:
  //################################################################################################
  PostSSRLayer(Map* map, RenderPass customRenderPass);

protected:
  //################################################################################################
  PostShader* makeShader() override;
};

}

#endif
