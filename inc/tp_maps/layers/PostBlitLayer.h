#ifndef tp_maps_PostBlitLayer_h
#define tp_maps_PostBlitLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostBlitLayer: public PostLayer
{
public:
  //################################################################################################
  PostBlitLayer(Map* map, RenderPass customRenderPass);

protected:
  //################################################################################################
  PostShader* makeShader() override;
};

}

#endif
