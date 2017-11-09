#pragma once

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <osg/Geometry>
#include <osgViewer/Renderer>
#include <osgViewer/GraphicsWindow>

#ifndef GL_TEXTURE_MAX_LEVEL
	#define GL_TEXTURE_MAX_LEVEL 0x813D
#endif

#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
	typedef osg::GLExtensions OSG_GLExtensions;
	typedef osg::GLExtensions OSG_Texture_Extensions;
#else
	typedef osg::FBOExtensions OSG_GLExtensions;
	typedef osg::Texture::Extensions OSG_Texture_Extensions;
#endif

static const OSG_GLExtensions* getGLExtensions(const osg::State& state)
{
#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
	return state.get<osg::GLExtensions>();
#else
	return osg::FBOExtensions::instance(state.getContextID(), true);
#endif
}

static const OSG_Texture_Extensions* getTextureExtensions(const osg::State& state)
{
#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
	return state.get<osg::GLExtensions>();
#else
	return osg::Texture::getExtensions(state.getContextID(), true);
#endif
}

static osg::FrameBufferObject* getFrameBufferObject(osg::RenderInfo& renderInfo)
{
	osg::Camera* camera = renderInfo.getCurrentCamera();
	osgViewer::Renderer* camRenderer = (dynamic_cast<osgViewer::Renderer*>(camera->getRenderer()));

	if (camRenderer != nullptr)
	{
		osgUtil::SceneView* sceneView = camRenderer->getSceneView(0);

		if (sceneView != nullptr)
		{
			osgUtil::RenderStage* renderStage = sceneView->getRenderStage();

			if (renderStage != nullptr)
			{
				return renderStage->getFrameBufferObject();
			}
		}
	}

	return nullptr;
}