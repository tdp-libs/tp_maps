#ifndef tp_maps_GridLayer_h
#define tp_maps_GridLayer_h

#include "tp_maps/Layer.h"

namespace tp_maps
{
class FontRenderer;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT GridLayer: public Layer
{
public:
  //################################################################################################
  GridLayer(float scale = 1.0f, const glm::vec3& gridColor = {0.05f, 0.05f, 0.9f});

  //################################################################################################
  ~GridLayer() override;

  //################################################################################################
  //! Set a multiplier for the spacing between each graduation in the grid.
  /*!
  \param spacing Smaller is spacing, closer are the graduations to one another.
  */
  void setSpacing(float spacing);

  //################################################################################################
  float getSpacing() const;

  //################################################################################################
  //! Set the font that will be used to labels
  /*!
  This sets the font that will be used to draw the grid labels.
  \note This does not take ownership.
  \param font The font to use for drawing grid labels.
  */
  void setFont(FontRenderer* font);

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
