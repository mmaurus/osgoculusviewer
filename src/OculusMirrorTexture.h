#pragma once

#include <OVR_CAPI_GL.h>

#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/Texture2DArray>
#include <osg/Version>
#include <osg/FrameBufferObject>

#include "helpers.h"

class OculusMirrorTexture : public osg::Referenced
{
public:
	OculusMirrorTexture(ovrSession& session, osg::ref_ptr<osg::State> state, int width, int height);
	void destroy(const OSG_GLExtensions* fbo_ext = 0);
	//GLuint id() const { return m_texture->OGL.TexId; }
	GLint width() const { return m_width; }
	GLint height() const { return m_height; }
	void blitTexture(osg::GraphicsContext* gc);
protected:
	~OculusMirrorTexture() {}

	const ovrSession m_session;
	ovrMirrorTexture m_texture;
	GLint m_width;
	GLint m_height;
	GLuint m_mirrorFBO;
};