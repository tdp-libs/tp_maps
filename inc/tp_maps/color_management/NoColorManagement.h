#ifndef tp_maps_NoColorManagement_h
#define tp_maps_NoColorManagement_h

#include "tp_maps/ColorManagement.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT NoColorManagement final : public ColorManagement
{
public:
  //################################################################################################
  ~NoColorManagement() override;

  //################################################################################################
  glm::vec4 toLinear(const glm::vec4& color) const override;

  //################################################################################################
  glm::vec3 toLinear(const glm::vec3& color) const override;

  //################################################################################################
  glm::vec4 fromLinear(const glm::vec4& color) const override;

  //################################################################################################
  glm::vec3 fromLinear(const glm::vec3& color) const override;

  //################################################################################################
  std::string glsl() const override;
};

}

#endif