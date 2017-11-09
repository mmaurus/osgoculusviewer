#include "OculusTextureBuffer.h"

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <osg/Geometry>
#include <osgViewer/Renderer>
#include <osgViewer/GraphicsWindow>

#include <iostream>
#include <cstdio>

/* Public functions */
OculusTextureBuffer::OculusTextureBuffer(const ovrSession& session, osg::ref_ptr<osg::State> state, const ovrSizei& size, int msaaSamples) : m_session(session),
	m_textureSwapChain(nullptr),
	m_colorBuffer(nullptr),
	m_depthBuffer(nullptr),
	m_textureSize(osg::Vec2i(size.w, size.h)),
	m_Oculus_FBO(0),
	m_MSAA_FBO(0),
	m_MSAA_ColorTex(0),
	m_MSAA_DepthTex(0),
	m_samples(msaaSamples)
{
	if (msaaSamples == 0)
	{
		setup(*state);
	}
	else
	{
		setupMSAA(*state);
	}

}

void OculusTextureBuffer::setup(osg::State& state)
{

	ovrTextureSwapChainDesc desc = {};
	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Width = m_textureSize.x();
	desc.Height = m_textureSize.y();
	desc.MipLevels = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;

	ovrResult result = ovr_CreateTextureSwapChainGL(m_session, &desc, &m_textureSwapChain);

	int length = 0;
	ovr_GetTextureSwapChainLength(m_session, m_textureSwapChain, &length);

	if (OVR_SUCCESS(result))
	{
		for (int i = 0; i < length; ++i)
		{
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(m_session, m_textureSwapChain, i, &chainTexId);

			osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();

			osg::ref_ptr<osg::Texture::TextureObject> textureObject = new osg::Texture::TextureObject(texture.get(), chainTexId, GL_TEXTURE_2D);

			textureObject->setAllocated(true);

			texture->setTextureObject(state.getContextID(), textureObject.get());

			texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
			texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
			texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
			texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

			texture->setTextureSize(m_textureSize.x(), m_textureSize.y());
			texture->setSourceFormat(GL_SRGB8_ALPHA8);

			// Set the color buffer to be the first texture generated
			if (i == 0)
			{
				m_colorBuffer = texture;
			}
		}

		osg::notify(osg::DEBUG_INFO) << "Successfully created the swap texture set!" << std::endl;
	}
	else
	{
		osg::notify(osg::WARN) << "Warning: Unable to create swap texture set! " << std::endl;
		return;
	}

	m_depthBuffer = new osg::Texture2D();
	m_depthBuffer->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	m_depthBuffer->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	m_depthBuffer->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_depthBuffer->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_depthBuffer->setSourceFormat(GL_DEPTH_COMPONENT);
	m_depthBuffer->setSourceType(GL_UNSIGNED_INT);
	m_depthBuffer->setInternalFormat(GL_DEPTH_COMPONENT24);
	m_depthBuffer->setTextureWidth(textureWidth());
	m_depthBuffer->setTextureHeight(textureHeight());
	m_depthBuffer->setBorderWidth(0);

}


void OculusTextureBuffer::setupMSAA(osg::State& state)
{
	
	const OSG_GLExtensions* fbo_ext = getGLExtensions(state);

	
	ovrTextureSwapChainDesc desc = {};
	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Width = m_textureSize.x();
	desc.Height = m_textureSize.y();
	desc.MipLevels = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;

	ovrResult result = ovr_CreateTextureSwapChainGL(m_session, &desc, &m_textureSwapChain);

	int length = 0;
	ovr_GetTextureSwapChainLength(m_session, m_textureSwapChain, &length);

	if (OVR_SUCCESS(result))
	{
		for (int i = 0; i < length; ++i)
		{
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(m_session, m_textureSwapChain, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		osg::notify(osg::DEBUG_INFO) << "Successfully created the swap texture set!" << std::endl;
	}

	// Create an FBO for output to Oculus swap texture set.
	fbo_ext->glGenFramebuffers(1, &m_Oculus_FBO);

	// Create an FBO for primary render target.
	fbo_ext->glGenFramebuffers(1, &m_MSAA_FBO);

	// We don't want to support MIPMAP so, ensure only level 0 is allowed.
	const int maxTextureLevel = 0;

	const OSG_Texture_Extensions* extensions = getTextureExtensions(state);

	// Create MSAA color buffer
	glGenTextures(1, &m_MSAA_ColorTex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_ColorTex);
	extensions->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_RGBA, m_textureSize.x(), m_textureSize.y(), false);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, maxTextureLevel);

	// Create MSAA depth buffer
	glGenTextures(1, &m_MSAA_DepthTex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_DepthTex);
	extensions->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_DEPTH_COMPONENT, m_textureSize.x(), m_textureSize.y(), false);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, maxTextureLevel);
}

void OculusTextureBuffer::onPreRender(osg::RenderInfo& renderInfo)
{
	GLuint curTexId = 0;
	if (m_textureSwapChain)
	{
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(m_session, m_textureSwapChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(m_session, m_textureSwapChain, curIndex, &curTexId);
	}

	osg::State& state = *renderInfo.getState();
	const OSG_GLExtensions* fbo_ext = getGLExtensions(state);

	if (m_samples == 0)
	{
		osg::FrameBufferObject* fbo = getFrameBufferObject(renderInfo);

		if (fbo == nullptr)
		{
			return;
		}

		const unsigned int ctx = state.getContextID();
		fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo->getHandle(ctx));
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, curTexId, 0);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_depthBuffer->getTextureObject(state.getContextID())->id(), 0);
	}
	else
	{
		fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_MSAA_FBO);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_ColorTex, 0);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_DepthTex, 0);
	}

}
void OculusTextureBuffer::onPostRender(osg::RenderInfo& renderInfo)
{

	if (m_textureSwapChain) {
		ovr_CommitTextureSwapChain(m_session, m_textureSwapChain);
	}

	if (m_samples == 0)
	{
		// Nothing to do here if MSAA not being used.
		return;
	}

	// Get texture id
	GLuint curTexId = 0;
	if (m_textureSwapChain)
	{
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(m_session, m_textureSwapChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(m_session, m_textureSwapChain, curIndex, &curTexId);
	}

	osg::State& state = *renderInfo.getState();
	const OSG_GLExtensions* fbo_ext = getGLExtensions(state);

	fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_MSAA_FBO);
	fbo_ext->glFramebufferTexture2D(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_ColorTex, 0);
	fbo_ext->glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);

	fbo_ext->glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, m_Oculus_FBO);
	fbo_ext->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, curTexId, 0);
	fbo_ext->glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);

	int w = m_textureSize.x();
	int h = m_textureSize.y();
	fbo_ext->glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

}

void OculusTextureBuffer::destroy()
{
	ovr_DestroyTextureSwapChain(m_session, m_textureSwapChain);
}
