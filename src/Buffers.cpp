#include "tp_maps/Buffers.h"
#include "tp_maps/Errors.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#ifdef TP_MAPS_DEBUG
#  define DEBUG_printOpenGLError(A) Errors::printOpenGLError(A)
#else
#  define DEBUG_printOpenGLError(A) do{}while(false)
#endif

namespace tp_maps
{

//##################################################################################################
struct Buffers::Private
{
  Map* map;

  bool updateSamplesRequired{true};
  size_t maxSamples{1};
  size_t samples{1};

  //################################################################################################
  Private(Map* map_):
    map(map_)
  {

  }

  //################################################################################################
  GLint colorFormatF(Alpha alpha)
  {
#ifdef TP_GLES2
    return (alpha==Alpha::Yes)?GL_RGBA32F_EXT:GL_RGB16F_EXT;
#else
    switch(map->openGLProfile())
    {
    case OpenGLProfile::VERSION_100_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_320_ES:
      return (alpha==Alpha::Yes)?GL_RGBA32F:GL_RGB16F;
    default:
      return (alpha==Alpha::Yes)?GL_RGBA32F:GL_RGB32F;
    }
#endif
  }

  //################################################################################################
  GLint depthFormatF()
  {
#ifdef TP_GLES2
    return GL_R32F_EXT;
#else
    switch(map->openGLProfile())
    {
    case OpenGLProfile::VERSION_100_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_320_ES:
      return GL_R32F;

    default:
      return GL_R32F;
    }
#endif
  }

  //################################################################################################
  void create2DColorTexture(GLuint& textureID, size_t width, size_t height, HDR hdr, Alpha alpha)
  {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    DEBUG_printOpenGLError("create2DColorTexture A");

    if(hdr == HDR::No)
    {
      if(alpha == Alpha::No)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TPGLsizei(width), TPGLsizei(height), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
      else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TPGLsizei(width), TPGLsizei(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      DEBUG_printOpenGLError("create2DColorTexture B");
    }
    else
    {
      // On ES force alpha as GL_RGB32F is not an option but GL_RGBA32F is.
      if(alpha == Alpha::No)
      {
        switch (map->openGLProfile())
        {
        case OpenGLProfile::VERSION_100_ES: [[fallthrough]];
        case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
        case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
        case OpenGLProfile::VERSION_320_ES:
          alpha = Alpha::Yes;
          break;

        default:
          break;
        }
      }

      if(alpha == Alpha::No)
        glTexImage2D(GL_TEXTURE_2D, 0, colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height), 0, GL_RGB, GL_FLOAT, nullptr);
      else
        glTexImage2D(GL_TEXTURE_2D, 0, colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height), 0, GL_RGBA, GL_FLOAT, nullptr);
      DEBUG_printOpenGLError("create2DColorTexture C");
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    DEBUG_printOpenGLError("create2DColorTexture generate 2D texture for color buffer");
  }

#ifdef TP_ENABLE_MULTISAMPLE_FBO
  //################################################################################################
  void createMultisampleTexture(GLuint& multisampleTextureID, size_t width, size_t height, HDR hdr, Alpha alpha, GLenum attachment)
  {
    switch(map->openGLProfile())
    {
    case OpenGLProfile::VERSION_100_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_320_ES:
      break;

    default:
      glGenTextures(1, &multisampleTextureID);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleTextureID);

      if(hdr == HDR::No)
      {
        if(alpha == Alpha::No)
          glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, TPGLsizei(samples), GL_RGB, TPGLsizei(width), TPGLsizei(height), GL_TRUE);
        else
          glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, TPGLsizei(samples), GL_RGBA, TPGLsizei(width), TPGLsizei(height), GL_TRUE);
      }
      else
      {
        if(alpha == Alpha::No)
          glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, TPGLsizei(samples), colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height), GL_TRUE);
        else
          glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, TPGLsizei(samples), colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height), GL_TRUE);
      }

      glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D_MULTISAMPLE, multisampleTextureID, 0);
      break;
    }
    DEBUG_printOpenGLError("createMultisampleTexture generate 2D texture for multisample FBO");
  }
#endif

  //################################################################################################
  void create2DDepthTexture(GLuint& depthID, size_t width, size_t height)
  {
    glGenTextures(1, &depthID);

    glBindTexture(GL_TEXTURE_2D, depthID);

    switch(map->openGLProfile())
    {
    case OpenGLProfile::VERSION_100_ES:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TPGLsizei(width), TPGLsizei(height), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
      break;

    case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_320_ES:

      //          (GLenum target, GLint level,    GLint internalformat,    GLsizei width,    GLsizei height, GLint border,      GLenum format,  GLenum type, const void *pixels);
      glTexImage2D(GL_TEXTURE_2D,           0, TP_GL_DEPTH_COMPONENT32, TPGLsizei(width), TPGLsizei(height),            0, GL_DEPTH_COMPONENT,     GL_FLOAT,            nullptr);
      break;

    default:
      glTexImage2D(GL_TEXTURE_2D, 0, TP_GL_DEPTH_COMPONENT24, TPGLsizei(width), TPGLsizei(height), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
      break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    DEBUG_printOpenGLError("create2DDepthTexture generate 2D texture for depth buffer");
  }

  //################################################################################################
  void create3DDepthTexture(GLuint& depthID, size_t width, size_t height, size_t levels)
  {
    glGenTextures(1, &depthID);

    glBindTexture(GL_TEXTURE_3D, depthID);

    switch(map->openGLProfile())
    {
    case OpenGLProfile::VERSION_100_ES:
      glTexImage3D(GL_TEXTURE_3D, 0, GL_DEPTH_COMPONENT, TPGLsizei(width), TPGLsizei(height), TPGLsizei(levels), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
      break;

    case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_320_ES:
      glTexImage3D(GL_TEXTURE_3D, 0, depthFormatF(), TPGLsizei(width), TPGLsizei(height), TPGLsizei(levels), 0, GL_RED, GL_FLOAT, nullptr);
      break;

    default:
      glTexImage3D(GL_TEXTURE_3D, 0, depthFormatF(), TPGLsizei(width), TPGLsizei(height), TPGLsizei(levels), 0, GL_RED, GL_UNSIGNED_SHORT, nullptr);
      break;
    }

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    DEBUG_printOpenGLError("create3DDepthTexture generate 3D texture for depth buffer");
  }

  //################################################################################################
  void createColorRBO(GLuint& rboID, size_t width, size_t height, HDR hdr, Alpha alpha, GLenum attachment)
  {
    glGenRenderbuffers(1, &rboID);
    glBindRenderbuffer(GL_RENDERBUFFER, rboID);

    if(hdr == HDR::No)
    {
      if(alpha == Alpha::No)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), GL_RGB8, TPGLsizei(width), TPGLsizei(height));
      else
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), GL_RGBA8, TPGLsizei(width), TPGLsizei(height));
    }
    else
    {
      if(alpha == Alpha::No)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height));
      else
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height));
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboID);
    DEBUG_printOpenGLError("createColorRBO multisample RBO for attachment " + std::to_string(attachment));
  }

  //################################################################################################
  void createDepthRBO(GLuint& rboID, size_t width, size_t height)
  {
    glGenRenderbuffers(1, &rboID);
    glBindRenderbuffer(GL_RENDERBUFFER, rboID);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), TP_GL_DEPTH_COMPONENT24, TPGLsizei(width), TPGLsizei(height));
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);
    DEBUG_printOpenGLError("createDepthRBO multisample RBO for depth");
  }

  //################################################################################################
  void setDrawBuffers(const std::vector<GLenum>& buffers)
  {
    switch(map->openGLProfile())
    {
    default:
      glDrawBuffers(TPGLsizei(buffers.size()), buffers.data());
      break;

    case OpenGLProfile::VERSION_100_ES:
      break;
    }

  }

  //################################################################################################
  //! Create and bind an FBO.
  /*!
  This will delete and recreate the buffer if required.

  The rules for buffer formats are quite complicated and vary between versions.
  Section: 4.4.4
  https://www.khronos.org/registry/OpenGL/specs/es/3.0/es_spec_3.0.pdf

  \param buffer holds the resource ID's generated by this function.
  \param width of the buffer pixels.
  \param height of the buffer in pixels.
  \param createColorBuffer should be true if we need to create a color buffer, false for shadows.
  \param multisample should be true to enable antialiasing (MSAA).
  \param hdr are we using 8 bit or HDR buffers.
  \param extendedFBO are we creating extra buffers for normals and specular.
  \param levels controls the number of textures to generate for a shadow texture, else 1.
  \param level index to bind when rendering lights.

  \return true if we managed to create a functional FBO.
  */
  bool prepareBuffer(FBO& buffer,
                     size_t width,
                     size_t height,
                     CreateColorBuffer createColorBuffer,
                     Multisample multisample,
                     HDR hdr,
                     ExtendedFBO extendedFBO,
                     size_t levels,
                     size_t level,
                     bool clear)
  {
    DEBUG_printOpenGLError("prepareBuffer Start");



#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if(updateSamplesRequired)
    {
      updateSamplesRequired = false;

      GLint max=1;
      glGetIntegerv(GL_MAX_SAMPLES, &max);
      samples = tpMin(size_t(max), maxSamples);

      if(samples != maxSamples)
        tpWarning() << "Max samples set to: " << samples;
    }
#endif

#ifndef TP_ENABLE_3D_TEXTURE
    levels = 1;
#endif

    multisample = (multisample==Multisample::Yes && (samples>1))?Multisample::Yes:Multisample::No;

    if(buffer.width!=width || buffer.height!=height || buffer.levels!=levels || buffer.samples!=samples || buffer.hdr != hdr || buffer.extendedFBO != extendedFBO || buffer.multisample != multisample)
      deleteBuffer(buffer);

    buffer.level = level;

    if(!buffer.frameBuffer)
    {
      glGenFramebuffers(1, &buffer.frameBuffer);
      buffer.width       = width;
      buffer.height      = height;
      buffer.levels      = levels;
      buffer.samples     = samples;
      buffer.hdr         = hdr;
      buffer.extendedFBO = extendedFBO;
      buffer.multisample = multisample;
    }

    DEBUG_printOpenGLError("prepareBuffer Init");

    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    DEBUG_printOpenGLError("prepareBuffer glBindFramebuffer first");

    // Some versions of OpenGL must have a color buffer even if we are not going to use it.
    if(createColorBuffer == CreateColorBuffer::No)
    {
      if(map->openGLProfile() == OpenGLProfile::VERSION_100_ES)
        createColorBuffer = CreateColorBuffer::Yes;
    }

#ifdef TP_ENABLE_3D_TEXTURE
    if(levels != 1)
      createColorBuffer = CreateColorBuffer::No;
#endif

    if(createColorBuffer == CreateColorBuffer::Yes)
    {
      if(!buffer.textureID)
        create2DColorTexture(buffer.textureID, width, height, hdr, Alpha::No);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.textureID, 0);
      DEBUG_printOpenGLError("prepareBuffer bind 2D texture to FBO");
    }

#ifdef TP_ENABLE_3D_TEXTURE
    if(levels != 1)
    {
      //It's not possible to bind a 3D texture as a depth buffer, so we bind it as the color buffer
      //and copy the depth values to that color buffer.
      //The 3D depth buffer is bound as GL_COLOR_ATTACHMENT0.
      if(!buffer.depthID)
        create3DDepthTexture(buffer.depthID, width, height, levels);

      if(!buffer.textureID)
      {
        // For most OpenGL versions, we do however still require an actual depth
        //buffer to perform depth tests against. So here textureID gets prepared as a 2D depth buffer.
        //The 2D depth buffer is bound as GL_DEPTH_ATTACHMENT.
        switch(map->openGLProfile())
        {
        default:
          create2DDepthTexture(buffer.textureID, width, height);
          break;

        case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
        case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
        case OpenGLProfile::VERSION_320_ES:
          break;
        }
      }

#ifdef TP_GLES3
      // glFramebufferTexture3D is not supported in GLES.
      glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer.depthID, 0, GLint(level));
#else
      glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, buffer.depthID, 0, GLint(level));
#endif
      switch(map->openGLProfile())
      {
      default:
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.textureID, 0);
        break;

      case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
      case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
      case OpenGLProfile::VERSION_320_ES:
        break;
      }
      DEBUG_printOpenGLError("prepareBuffer bind 3D texture to FBO as color but to store depth");
    }
    else
#endif
    {
      if(!buffer.depthID)
        create2DDepthTexture(buffer.depthID, width, height);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthID, 0);
      DEBUG_printOpenGLError("prepareBuffer bind 2D texture to FBO to store depth");
    }

    if(extendedFBO == ExtendedFBO::Yes)
    {
      if(!buffer.normalsID)
        create2DColorTexture(buffer.normalsID, width, height, HDR::Yes, Alpha::No);

      if(!buffer.specularID)
        create2DColorTexture(buffer.specularID, width, height, HDR::Yes, Alpha::Yes);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer.normalsID, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, buffer.specularID, 0);
      DEBUG_printOpenGLError("prepareBuffer bind 2D normal and specular textures to FBO");
      setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
      setDrawBuffers({GL_COLOR_ATTACHMENT0});
    }
    else
      setDrawBuffers({GL_COLOR_ATTACHMENT0});

    if(Errors::printFBOError(buffer, "Error Map::Private::prepareBuffer frame buffer not complete!"))
      return false;

#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if(multisample == Multisample::Yes)
    {
      glEnable(GL_MULTISAMPLE);

      if(!buffer.multisampleFrameBuffer)
        glGenFramebuffers(1, &buffer.multisampleFrameBuffer);

      glBindFramebuffer(GL_FRAMEBUFFER, buffer.multisampleFrameBuffer);
      DEBUG_printOpenGLError("prepareBuffer glBindFramebuffer second");

      if(!buffer.multisampleDepthRBO)
        createDepthRBO(buffer.multisampleDepthRBO, width, height);

      if(!buffer.multisampleTextureID)
        createMultisampleTexture(buffer.multisampleTextureID, width, height, hdr, Alpha::No, GL_COLOR_ATTACHMENT0);

      if(!buffer.multisampleColorRBO)
        createColorRBO(buffer.multisampleColorRBO, width, height, hdr, Alpha::No, GL_COLOR_ATTACHMENT0);

      if(extendedFBO == ExtendedFBO::Yes)
      {
        if(!buffer.multisampleNormalsTextureID)
          createMultisampleTexture(buffer.multisampleNormalsTextureID, width, height, HDR::Yes, Alpha::No, GL_COLOR_ATTACHMENT1);

        if(!buffer.multisampleSpecularTextureID)
          createMultisampleTexture(buffer.multisampleSpecularTextureID, width, height, HDR::Yes, Alpha::Yes, GL_COLOR_ATTACHMENT2);

        if(!buffer.multisampleNormalsRBO)
          createColorRBO(buffer.multisampleNormalsRBO, width, height, HDR::Yes, Alpha::No, GL_COLOR_ATTACHMENT1);

        if(!buffer.multisampleSpecularRBO)
          createColorRBO(buffer.multisampleSpecularRBO, width, height, HDR::Yes, Alpha::Yes, GL_COLOR_ATTACHMENT2);

        setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
      }
      else
        setDrawBuffers({GL_COLOR_ATTACHMENT0});

      if(Errors::printFBOError(buffer, "Error Map::Private::prepareBuffer multisample frame buffer not complete!"))
        return false;
    }
#endif

    glViewport(0, 0, TPGLsizei(width), TPGLsizei(height));

    glDepthMask(true);

    if(clear)
    {
      glClearDepthf(1.0f);

      if(levels!=1)
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      else
      {
        auto backgroundColor = map->backgroundColor();
        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
      }

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    DEBUG_printOpenGLError("prepareBuffer end");

    return true;
  }

  //################################################################################################
  //If we are rendering to a multisample FBO we need to blit the results to a non multisampled FBO
  //before we can actually use it. (thanks OpenGL)
  void swapMultisampledBuffer(FBO& buffer)
  {
#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if(buffer.multisample == Multisample::Yes)
    {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer.multisampleFrameBuffer);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer.frameBuffer);
      DEBUG_printOpenGLError("swapMultisampledBuffer bind FBOs");

      glReadBuffer(GL_COLOR_ATTACHMENT0);
      DEBUG_printOpenGLError("swapMultisampledBuffer a");
      setDrawBuffers({GL_COLOR_ATTACHMENT0});
      DEBUG_printOpenGLError("swapMultisampledBuffer b");
      glBlitFramebuffer(0, 0, GLint(buffer.width), GLint(buffer.height), 0, 0, GLint(buffer.width), GLint(buffer.height), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
      DEBUG_printOpenGLError("swapMultisampledBuffer blit color 0 and depth");

      if(buffer.extendedFBO == ExtendedFBO::Yes)
      {
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        setDrawBuffers({GL_COLOR_ATTACHMENT1});
        glBlitFramebuffer(0, 0, GLint(buffer.width), GLint(buffer.height), 0, 0, GLint(buffer.width), GLint(buffer.height), GL_COLOR_BUFFER_BIT, GL_NEAREST);
        DEBUG_printOpenGLError("swapMultisampledBuffer blit color 1");

        glReadBuffer(GL_COLOR_ATTACHMENT2);
        setDrawBuffers({GL_COLOR_ATTACHMENT2});
        glBlitFramebuffer(0, 0, GLint(buffer.width), GLint(buffer.height), 0, 0, GLint(buffer.width), GLint(buffer.height), GL_COLOR_BUFFER_BIT, GL_NEAREST);
        DEBUG_printOpenGLError("swapMultisampledBuffer blit color 2");

        setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
      }
      else
        setDrawBuffers({GL_COLOR_ATTACHMENT0});

      glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
      DEBUG_printOpenGLError("swapMultisampledBuffer bind FBO");
    }
#else
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    TP_UNUSED(buffer);
#endif
  }

  //################################################################################################
  void deleteBuffer(FBO& buffer)
  {
    glBindTexture(GL_TEXTURE_2D, 0);

    if(buffer.frameBuffer)
    {
      glDeleteFramebuffers(1, &buffer.frameBuffer);
      buffer.frameBuffer = 0;
    }

    if(buffer.textureID)
    {
      glDeleteTextures(1, &buffer.textureID);
      buffer.textureID = 0;
    }

    if(buffer.depthID)
    {
      glDeleteTextures(1, &buffer.depthID);
      buffer.depthID = 0;
    }

    if(buffer.normalsID)
    {
      glDeleteTextures(1, &buffer.normalsID);
      buffer.normalsID = 0;
    }

    if(buffer.specularID)
    {
      glDeleteTextures(1, &buffer.specularID);
      buffer.specularID = 0;
    }

#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if(buffer.multisampleFrameBuffer)
    {
      glDeleteFramebuffers(1, &buffer.multisampleFrameBuffer);
      buffer.multisampleFrameBuffer = 0;
    }

    if(buffer.multisampleTextureID)
    {
      glDeleteTextures(1, &buffer.multisampleTextureID);
      buffer.multisampleTextureID = 0;
    }

    if(buffer.multisampleColorRBO)
    {
      glDeleteRenderbuffers(1, &buffer.multisampleColorRBO);
      buffer.multisampleColorRBO = 0;
    }

    if(buffer.multisampleDepthRBO)
    {
      glDeleteRenderbuffers(1, &buffer.multisampleDepthRBO);
      buffer.multisampleDepthRBO = 0;
    }

    if(buffer.multisampleNormalsTextureID)
    {
      glDeleteTextures(1, &buffer.multisampleNormalsTextureID);
      buffer.multisampleNormalsTextureID = 0;
    }

    if(buffer.multisampleSpecularTextureID)
    {
      glDeleteTextures(1, &buffer.multisampleSpecularTextureID);
      buffer.multisampleSpecularTextureID = 0;
    }

    if(buffer.multisampleNormalsRBO)
    {
      glDeleteRenderbuffers(1, &buffer.multisampleNormalsRBO);
      buffer.multisampleNormalsRBO = 0;
    }

    if(buffer.multisampleSpecularRBO)
    {
      glDeleteRenderbuffers(1, &buffer.multisampleSpecularRBO);
      buffer.multisampleSpecularRBO = 0;
    }
#endif
  }

  //################################################################################################
  void invalidateBuffer(FBO& buffer)
  {
    buffer.frameBuffer = 0;
    buffer.textureID   = 0;
    buffer.depthID     = 0;
    buffer.normalsID   = 0;
    buffer.specularID  = 0;

#ifdef TP_ENABLE_MULTISAMPLE_FBO
    buffer.multisampleFrameBuffer = 0;
    buffer.multisampleTextureID   = 0;
    buffer.multisampleColorRBO    = 0;
    buffer.multisampleDepthRBO    = 0;

    buffer.multisampleNormalsTextureID  = 0;
    buffer.multisampleSpecularTextureID = 0;
    buffer.multisampleNormalsRBO        = 0;
    buffer.multisampleSpecularRBO       = 0;
#endif
  }
};

//##################################################################################################
Buffers::Buffers(Map* map):
  d(new Private(map))
{

}

//##################################################################################################
Buffers::~Buffers()
{
  delete d;
}

//##################################################################################################
bool Buffers::prepareBuffer(FBO& buffer,
                            size_t width,
                            size_t height,
                            CreateColorBuffer createColorBuffer,
                            Multisample multisample,
                            HDR hdr,
                            ExtendedFBO extendedFBO,
                            size_t levels,
                            size_t level,
                            bool clear) const
{
  return d->prepareBuffer( buffer,
                           width,
                           height,
                           createColorBuffer,
                           multisample,
                           hdr,
                           extendedFBO,
                           levels,
                           level,
                           clear);
}

//##################################################################################################
void Buffers::invalidateBuffer(FBO& fbo) const
{
  d->invalidateBuffer(fbo);
}

//##################################################################################################
void Buffers::deleteBuffer(FBO& fbo) const
{
  d->deleteBuffer(fbo);
}

//##################################################################################################
void Buffers::swapMultisampledBuffer(FBO& fbo) const
{
  d->swapMultisampledBuffer(fbo);
}

//##################################################################################################
void Buffers::setDrawBuffers(const std::vector<GLenum>& buffers) const
{
  d->setDrawBuffers(buffers);
}

//##################################################################################################
void Buffers::setMaxSamples(size_t maxSamples)
{
  d->updateSamplesRequired = true;
  d->maxSamples = maxSamples;
}

//##################################################################################################
size_t Buffers::maxSamples() const
{
  return d->maxSamples;
}

//##################################################################################################
void Buffers::initializeGL()
{
  d->updateSamplesRequired = true;
}

}
