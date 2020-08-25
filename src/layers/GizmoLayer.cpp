#include "tp_maps/layers/GizmoLayer.h"
#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/Map.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/PickingResult.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/matrix_transform.hpp"

namespace tp_maps
{

namespace
{
enum class Modify_lt
{
  None,
  RotateX,
  RotateY,
  RotateZ
};
}

//##################################################################################################
struct GizmoLayer::Private
{
  Geometry3DLayer* geometryLayer{nullptr};

  Modify_lt activeModification{Modify_lt::None};
  glm::ivec2 previousPos;
};

//##################################################################################################
GizmoLayer::GizmoLayer():
  d(new Private())
{
  d->geometryLayer = new Geometry3DLayer();
  addChildLayer(d->geometryLayer);

  std::vector<Geometry3D> geometry;

  auto makeCircle = [&](const std::function<glm::vec3(const glm::vec3&)>& transform, const glm::vec3& color)
  {
    auto& circle = geometry.emplace_back();

    circle.material.ambient = color;
    circle.material.diffuse = color;

    circle.geometry.triangleFan   = GL_TRIANGLE_FAN;
    circle.geometry.triangleStrip = GL_TRIANGLE_STRIP;
    circle.geometry.triangles     = GL_TRIANGLES;

    float outerRadius=1.0f;
    float innerRadius=0.95f;
    float spikeRadius=0.90f;

    circle.geometry.indexes.resize(4);
    auto& top = circle.geometry.indexes.at(0);
    auto& btm = circle.geometry.indexes.at(1);
    auto& out = circle.geometry.indexes.at(2);
    auto& mid = circle.geometry.indexes.at(3);

    top.type = circle.geometry.triangleStrip;
    btm.type = circle.geometry.triangleStrip;
    out.type = circle.geometry.triangleStrip;
    mid.type = circle.geometry.triangleStrip;

    for(size_t a=0; a<=360; a+=2)
    {
      float x = std::sin(glm::radians(float(a)));
      float y = std::cos(glm::radians(float(a)));

      glm::vec2 v{x, y};

      tp_math_utils::Vertex3D vert;

      vert.vert = transform(glm::vec3(v*outerRadius, 0.01));
      circle.geometry.verts.push_back(vert);
      vert.vert = transform(glm::vec3(v*outerRadius, -0.01));
      circle.geometry.verts.push_back(vert);

      auto iRad = (a%20)?innerRadius:spikeRadius;

      vert.vert = transform(glm::vec3(v*iRad, 0.01));
      circle.geometry.verts.push_back(vert);
      vert.vert = transform(glm::vec3(v*iRad, -0.01));
      circle.geometry.verts.push_back(vert);

      int i = int(circle.geometry.verts.size());

      top.indexes.push_back(i-4);
      top.indexes.push_back(i-2);

      btm.indexes.push_back(i-1);
      btm.indexes.push_back(i-3);

      out.indexes.push_back(i-3);
      out.indexes.push_back(i-4);

      mid.indexes.push_back(i-2);
      mid.indexes.push_back(i-1);
    }

    circle.geometry.calculateFaceNormals();
  };

  makeCircle([&](const auto& c){return glm::vec3(c.z, c.x, c.y);}, glm::vec3(1,0,0));
  makeCircle([&](const auto& c){return glm::vec3(c.y, c.z, c.x);}, glm::vec3(0,1,0));
  makeCircle([&](const auto& c){return glm::vec3(c.x, c.y, c.z);}, glm::vec3(0,0,1));

  d->geometryLayer->setGeometry(geometry);
}

//##################################################################################################
GizmoLayer::~GizmoLayer()
{
  delete d;
}

//##################################################################################################
const glm::mat4& GizmoLayer::objectMatrix()const
{
  return d->geometryLayer->objectMatrix();
}

//##################################################################################################
void GizmoLayer::setObjectMatrix(const glm::mat4& objectMatrix)
{
  d->geometryLayer->setObjectMatrix(objectMatrix);
}

//##################################################################################################
bool GizmoLayer::mouseEvent(const MouseEvent& event)
{
  switch(event.type)
  {
  case MouseEventType::Press:
  {
    PickingResult* result = map()->performPicking(gizmoLayerSID(), event.pos);
    TP_CLEANUP([&]{delete result;});

    if(result)
    {
      if(result->layer == d->geometryLayer)
      {
        d->previousPos = event.pos;
        switch(result->details.index)
        {
        case 0:
          d->activeModification = Modify_lt::RotateX;
          return true;

        case 1:
          d->activeModification = Modify_lt::RotateY;
          return true;

        case 2:
          d->activeModification = Modify_lt::RotateZ;
          return true;
        };
      }
    }
    break;
  }

  case MouseEventType::Release:
  {
    if(d->activeModification != Modify_lt::None)
    {
      d->activeModification = Modify_lt::None;
      return true;
    }
    break;
  }

  case MouseEventType::Move:
  {
    if(d->activeModification == Modify_lt::None)
      break;

    glm::ivec2 delta = event.pos - d->previousPos;
    d->previousPos = event.pos;

    glm::vec3 axis{1,0,0};
    switch(d->activeModification)
    {
    case Modify_lt::RotateX: axis = {1,0,0}; break;
    case Modify_lt::RotateY: axis = {0,1,0}; break;
    case Modify_lt::RotateZ: axis = {0,0,1}; break;
    default: break;
    }

    auto mat = d->geometryLayer->objectMatrix();

    float angle = float((std::abs(delta.y)>std::abs(delta.x))?delta.y:delta.x) / 3.0f;
    mat = glm::rotate(mat, glm::radians(angle), axis);

    mat[0] = glm::vec4(glm::normalize(glm::vec3(mat[0])), 0.0f);
    mat[1] = glm::vec4(glm::normalize(glm::vec3(mat[1])), 0.0f);

    mat[2] = glm::vec4(glm::cross(glm::vec3(mat[0]), glm::vec3(mat[1])), 0.0f);
    mat[1] = glm::vec4(glm::cross(glm::vec3(mat[2]), glm::vec3(mat[0])), 0.0f);

    d->geometryLayer->setObjectMatrix(mat);
    changed();

    return true;
  }

  default:
    break;
  }

  return Layer::mouseEvent(event);
}

}