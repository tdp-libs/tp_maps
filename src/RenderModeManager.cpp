#include "tp_maps/RenderModeManager.h"
#include "tp_maps/Map.h"

#include "tp_utils/TimeUtils.h"

namespace tp_maps
{

//##################################################################################################
struct RenderModeManager::Private
{
  Q* q;
  Map* map;

  bool isConnected{false};

  RenderMode renderMode{RenderMode::Fast};
  RenderMode defaultRenderMode{RenderMode::Fast};
  int64_t nextModeAfter{0};

  int64_t switchToIntermediateDelay{300};
  int64_t switchToFullDelay{1000};

  size_t shadowSamplesFast{0};
  size_t shadowSamplesIntermediate{0};
  size_t shadowSamplesFull{0};

  size_t shadowSamples{0};

  //################################################################################################
  Private(Q* q_, Map* map_):
    q(q_),
    map(map_)
  {

  }

  //################################################################################################
  void checkConnect(bool connect)
  {
    if(isConnected == connect)
      return;

    isConnected = connect;

    if(connect)
      animateCallback.connect(map->animateCallbacks);
    else
      animateCallback.disconnect();
  }

  //################################################################################################
  tp_utils::Callback<void(double)> animateCallback = [&](double)
  {
    if(nextModeAfter == 0)
    {
      checkConnect(false);
      return;
    }

    if(renderMode == defaultRenderMode)
    {
      nextModeAfter = 0;
      checkConnect(false);
      return;
    }

    if(renderMode > defaultRenderMode)
    {
      q->setRenderMode(defaultRenderMode);
      checkConnect(false);
      return;
    }

    auto now = tp_utils::currentTimeMS();
    if(nextModeAfter>now)
      return;

    if(renderMode == RenderMode::Fast)
    {
      q->setRenderMode(RenderMode::Intermediate);
      if(renderMode != defaultRenderMode)
        nextModeAfter = now + switchToFullDelay;
      else
        checkConnect(false);

      return;
    }

    if(renderMode == RenderMode::Intermediate)
    {
      q->setRenderMode(RenderMode::Fast);
      checkConnect(false);
      return;
    }

    nextModeAfter = 0;
    checkConnect(false);
  };
};

//##################################################################################################
RenderModeManager::RenderModeManager(Map* map):
  d(new Private(this, map))
{

}

//##################################################################################################
RenderModeManager::~RenderModeManager()
{
  delete d;
}



//##################################################################################################
void RenderModeManager::setShadowSamples(RenderMode renderMode, size_t shadowSamples)
{
//#warning this seems inefficient
//  switch(renderMode)
//  {
//    case RenderMode::Full:
//    {
//      if(d->shadowSamplesFull != shadowSamples)
//      {
//        d->shadowSamplesFull = shadowSamples;
//        d->deleteShaders();
//      }
//    }
//    break;

//    case RenderMode::Intermediate:
//    {
//      if(d->shadowSamplesIntermediate != shadowSamples)
//      {
//        d->shadowSamplesIntermediate = shadowSamples;
//        d->deleteShaders();
//      }
//    }
//    break;

//    case RenderMode::Fast:
//    {
//      if(d->shadowSamplesFast != shadowSamples)
//        d->shadowSamplesFast = shadowSamples;
//    }
//    break;
//  }
}

//##################################################################################################
size_t RenderModeManager::shadowSamples(RenderMode renderMode) const
{
//#warning possibly move this into the manager
//  if(renderMode == RenderMode::Full)
//    return d->shadowSamplesFull;
//  else if(renderMode == RenderMode::Intermediate)
//    return d->shadowSamplesIntermediate;
//  else // renderMode == RenderMode::Fast
//    return d->shadowSamplesFast;
  return 0;
}


//##################################################################################################
void RenderModeManager::setRenderMode(RenderMode renderMode)
{
  d->renderMode = renderMode;
  d->nextModeAfter = 0;
  d->checkConnect(false);

  switch(renderMode)
  {
    case RenderMode::Fast         : d->shadowSamples = d->shadowSamplesFast        ; break;
    case RenderMode::Intermediate : d->shadowSamples = d->shadowSamplesIntermediate; break;
    case RenderMode::Full         : d->shadowSamples = d->shadowSamplesFull        ; break;
  }
}

//##################################################################################################
RenderMode RenderModeManager::renderMode() const
{
  return d->renderMode;
}

//##################################################################################################
size_t RenderModeManager::shadowSamples() const
{
#ifdef TP_LINUX
#warning implement
#warning make this return 0 for picking or just use 0 for picking renders.
#endif
  return d->shadowSamples;
}

//##################################################################################################
void RenderModeManager::enterFastRenderMode()
{
  setRenderMode(RenderMode::Fast);
  d->nextModeAfter = tp_utils::currentTimeMS() + d->switchToIntermediateDelay;
  d->checkConnect(true);
}

}
