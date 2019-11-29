#include "tp_maps/layers/LinesLayer.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/shaders/LineShader.h"
#include "tp_maps/picking_results/LinesPickingResult.h"

#include "tp_utils/DebugUtils.h"

#include "glm/glm.hpp"

namespace tp_maps
{
namespace
{
struct LinesDetails_lt
{
  LineShader::VertexBuffer* vertexBuffer;
  glm::vec4 color;
  GLenum mode;
};
}

//##################################################################################################
struct LinesLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::LinesLayer::Private");
  TP_NONCOPYABLE(Private);

  LinesLayer* q;
  std::vector<Lines> lines;

  //Processed geometry ready for rendering
  bool updateVertexBuffer{true};
  std::vector<LinesDetails_lt> processedGeometry;
  float lineWidth{1.0f};

  //################################################################################################
  Private(LinesLayer* q_):
    q(q_)
  {

  }

  //################################################################################################
  ~Private()
  {
    deleteVertexBuffers();
  }

  //################################################################################################
  void deleteVertexBuffers()
  {
    for(const auto& details : processedGeometry)
      delete details.vertexBuffer;

    processedGeometry.clear();
  }
};

//##################################################################################################
LinesLayer::LinesLayer():
  d(new Private(this))
{

}

//##################################################################################################
LinesLayer::~LinesLayer()
{
  delete d;
}

//##################################################################################################
const std::vector<Lines>& LinesLayer::lines()const
{
  return d->lines;
}

//##################################################################################################
void LinesLayer::setLines(const std::vector<Lines>& lines)
{
  d->lines = lines;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
float LinesLayer::lineWidth()const
{
  return d->lineWidth;
}

//##################################################################################################
void LinesLayer::setLineWidth(float lineWidth)
{
  d->lineWidth = lineWidth;
  update();
}

//##################################################################################################
void LinesLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass() && renderInfo.pass != RenderPass::Picking)
    return;

  auto shader = map()->getShader<LineShader>();
  if(shader->error())
    return;

  if(d->updateVertexBuffer)
  {
    d->deleteVertexBuffers();
    d->updateVertexBuffer=false;

    for(const Lines& shape : d->lines)
    {
      LinesDetails_lt& details = d->processedGeometry.emplace_back();
      details.vertexBuffer = shader->generateVertexBuffer(map(), shape.lines);
      details.color = shape.color;
      details.mode = shape.mode;
    }
  }

  shader->use(renderInfo.pass==RenderPass::Picking?ShaderType::Picking:ShaderType::Render);
  shader->setMatrix(map()->controller()->matrix(coordinateSystem()));
  shader->setLineWidth(d->lineWidth);

  if(renderInfo.pass==RenderPass::Picking)
  {
    size_t i=0;
    for(const LinesDetails_lt& line : d->processedGeometry)
    {
      auto pickingID = renderInfo.pickingIDMat(PickingDetails(0, [&, i](const PickingResult& r) -> PickingResult*
      {
        return new LinesPickingResult(r.pickingType, r.details, r.renderInfo, this, i);
      }));

      shader->setColor(pickingID);
      shader->drawLines(line.mode, line.vertexBuffer);
      i++;
    }
  }
  else
  {
    for(const LinesDetails_lt& line : d->processedGeometry)
    {
      shader->setColor(line.color);
      shader->drawLines(line.mode, line.vertexBuffer);
    }
  }
}

//##################################################################################################
void LinesLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
}

}
