#ifndef tp_maps_FPSController_h
#define tp_maps_FPSController_h

#include "tp_maps/Controller.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT FPSController : public Controller
{
public:
  //################################################################################################
  FPSController(Map* map, bool fullScreen);

  //################################################################################################
  glm::vec3 cameraOrigin() const;

  //################################################################################################
  void setCameraOrigin(const glm::vec3& cameraOrigin);

  //################################################################################################
  bool allowRotation() const;

  //################################################################################################
  void setAllowRotation(bool allowRotation);

  //################################################################################################
  bool variableViewAngle() const;

  //################################################################################################
  void setVariableViewAngle(bool variableViewAngle);

  //################################################################################################
  bool allowTranslation() const;

  //################################################################################################
  void setAllowTranslation(bool allowTranslation);

  //################################################################################################
  //! Rotation in degrees
  float rotationAngle() const;

  //################################################################################################
  void setRotationAngle(float rotationAngle);

  //################################################################################################
  float rotationFactor() const;

  //################################################################################################
  void setRotationFactor(float rotationFactor);

  //################################################################################################
  void setNearAndFar(float near, float far);

  //################################################################################################
  nlohmann::json saveState() const override;

  //################################################################################################
  void loadState(const nlohmann::json& j) override;

protected:
  //################################################################################################
  ~FPSController() override;

  //################################################################################################
   void mapResized(int w, int h) override;

  //################################################################################################
  void updateMatrices() override;

  //################################################################################################
  bool mouseEvent(const MouseEvent& event) override;

  //################################################################################################
  bool keyEvent(const KeyEvent& event) override;

  //################################################################################################
  void animate(double timestampMS) override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
