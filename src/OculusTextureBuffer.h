#pragma once

#include <OVR_CAPI_GL.h>

#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/Texture2DArray>
#include <osg/Version>
#include <osg/FrameBufferObject>

#include "helpers.h"


class OculusTextureBuffer : public osg::Referenced
{
public:
	OculusTextureBuffer(const ovrSession& session, osg::ref_ptr<osg::State> state, const ovrSizei& size, int msaaSamples);
	void destroy();
	int textureWidth() const { return m_textureSize.x(); }
	int textureHeight() const { return m_textureSize.y(); }
	int samples() const { return m_samples; }
	ovrTextureSwapChain textureSwapChain() const { return m_textureSwapChain; }
	osg::ref_ptr<osg::Texture2D> colorBuffer() const { return m_colorBuffer; }
	osg::ref_ptr<osg::Texture2D> depthBuffer() const { return m_depthBuffer; }
	void onPreRender(osg::RenderInfo& renderInfo);
	void onPostRender(osg::RenderInfo& renderInfo);

protected:
	~OculusTextureBuffer() {}

	const ovrSession m_session;
	ovrTextureSwapChain m_textureSwapChain;
	osg::ref_ptr<osg::Texture2D> m_colorBuffer;
	osg::ref_ptr<osg::Texture2D> m_depthBuffer;
	osg::Vec2i m_textureSize;

	void setup(osg::State& state);
	void setupMSAA(osg::State& state);

	GLuint m_Oculus_FBO; // MSAA FBO is copied to this FBO after render.
	GLuint m_MSAA_FBO; // framebuffer for MSAA texture
	GLuint m_MSAA_ColorTex; // color texture for MSAA
	GLuint m_MSAA_DepthTex; // depth texture for MSAA
	int m_samples;  // sample width for MSAA

};