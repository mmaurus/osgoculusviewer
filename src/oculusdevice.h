/*
 * oculusdevice.h
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSDEVICE_H_
#define _OSG_OCULUSDEVICE_H_

// Include the OculusVR SDK
#include <OVR_CAPI_GL.h>

#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/Version>
#include <osg/FrameBufferObject>

#include "OculusTextureBuffer.h"
#include "OculusMirrorTexture.h"

class OculusPreDrawCallback : public osg::Camera::DrawCallback
{
public:
	OculusPreDrawCallback(osg::Camera* camera, OculusTextureBuffer* textureBuffer)
		: m_camera(camera)
		, m_textureBuffer(textureBuffer)
	{
	}

	virtual void operator()(osg::RenderInfo& renderInfo) const;
protected:
	osg::observer_ptr<osg::Camera> m_camera;
	osg::observer_ptr<OculusTextureBuffer> m_textureBuffer;

};

class OculusPostDrawCallback : public osg::Camera::DrawCallback
{
public:
	OculusPostDrawCallback(osg::Camera* camera, OculusTextureBuffer* textureBuffer)
		: m_camera(camera)
		, m_textureBuffer(textureBuffer)
	{
	}

	virtual void operator()(osg::RenderInfo& renderInfo) const;
protected:
	osg::observer_ptr<osg::Camera> m_camera;
	osg::observer_ptr<OculusTextureBuffer> m_textureBuffer;

};


class OculusDevice : public osg::Referenced
{

public:
	typedef enum Eye_
	{
		LEFT = 0,
		RIGHT = 1,
		COUNT = 2
	} Eye;
	OculusDevice(float nearClip, float farClip, const float pixelsPerDisplayPixel = 1.0f, const float worldUnitsPerMetre = 1.0f, const int samples = 0, unsigned int mirrorTextureWidth = 960);
	void createRenderBuffers(osg::ref_ptr<osg::State> state);
	void init();

	bool hmdPresent() const;

	unsigned int screenResolutionWidth() const;
	unsigned int screenResolutionHeight() const;

	osg::Matrixf projectionMatrixLeft() const { return m_leftEyeProjectionMatrix; }
	osg::Matrixf projectionMatrixRight() const {	return m_rightEyeProjectionMatrix; }

	osg::Matrixf viewMatrixLeft() const { return m_leftEyeViewMatrix; }
	osg::Matrixf viewMatrixRight() const { return m_rightEyeViewMatrix; }
	osg::Matrixf viewMatrixCenter() const {
		osg::Matrixf viewMatrixCenter;
		viewMatrixCenter = m_leftEyeViewMatrix.operator*(0.5) + m_rightEyeViewMatrix.operator*(0.5);
		return viewMatrixCenter;
	}

	float nearClip() const { return m_nearClip;	}
	float farClip() const { return m_farClip; }

	void resetSensorOrientation() const;
	void updatePose(unsigned int frameIndex = 0);

	osg::Vec3 position() const { return m_position; }
	osg::Quat orientation() const { return m_orientation;  }

	osg::Camera* createRTTCamera(OculusDevice::Eye eye, osg::Transform::ReferenceFrame referenceFrame, const osg::Vec4& clearColor, osg::GraphicsContext* gc = 0) const;

	bool submitFrame(unsigned int frameIndex = 0);
	void blitMirrorTexture(osg::GraphicsContext* gc);

	void setPerfHudMode(int mode);

	osg::GraphicsContext::Traits* graphicsContextTraits() const;
protected:
	~OculusDevice(); // Since we inherit from osg::Referenced we must make destructor protected

	void printHMDDebugInfo();

	void initializeEyeRenderDesc();
	// Note: this function requires you to run the previous function first.
	void calculateViewMatrices();
	// Note: this function requires you to run the previous function first.
	void calculateProjectionMatrices();

	void setupLayers();

	void trySetProcessAsHighPriority() const;

	ovrSession m_session;
	ovrHmdDesc m_hmdDesc;

	const float m_pixelsPerDisplayPixel;
	const float m_worldUnitsPerMetre;

	osg::ref_ptr<OculusTextureBuffer> m_textureBuffer[2];
	osg::ref_ptr<OculusMirrorTexture> m_mirrorTexture;

	unsigned int m_mirrorTextureWidth;

	ovrEyeRenderDesc m_eyeRenderDesc[2];
	ovrVector2f m_UVScaleOffset[2][2];
	double m_frameTiming;
	ovrPosef m_headPose[2];
	ovrPosef m_eyeRenderPose[2];
	ovrLayerEyeFov m_layerEyeFov;
	ovrPosef m_viewOffset[2];
	osg::Matrixf m_leftEyeProjectionMatrix;
	osg::Matrixf m_rightEyeProjectionMatrix;
	osg::Matrixf m_leftEyeViewMatrix;
	osg::Matrixf m_rightEyeViewMatrix;

	osg::Vec3 m_position;
	osg::Quat m_orientation;

	float m_nearClip;
	float m_farClip;
	int m_samples;

	bool displayMirrorTexture;
private:
	OculusDevice(const OculusDevice&); // Do not allow copy
	OculusDevice& operator=(const OculusDevice&); // Do not allow assignment operator.
};


class OculusRealizeOperation : public osg::GraphicsOperation
{
public:
	explicit OculusRealizeOperation(osg::ref_ptr<OculusDevice> device) :
		osg::GraphicsOperation("OculusRealizeOperation", false), m_device(device), m_realized(false) {}
	virtual void operator () (osg::GraphicsContext* gc);
	bool realized() const { return m_realized; }
protected:
	OpenThreads::Mutex  _mutex;
	osg::observer_ptr<OculusDevice> m_device;
	bool m_realized;
};


class OculusSwapCallback : public osg::GraphicsContext::SwapCallback
{
public:
	explicit OculusSwapCallback(osg::ref_ptr<OculusDevice> device) : m_device(device), m_frameIndex(0) {}
	void swapBuffersImplementation(osg::GraphicsContext* gc);
	int frameIndex() const { return m_frameIndex; }
private:
	osg::observer_ptr<OculusDevice> m_device;
	int m_frameIndex;
};

class OculusInitialDrawCallback : public osg::Camera::DrawCallback
{
public:
   virtual void operator()(osg::RenderInfo& renderInfo) const;
};

#endif /* _OSG_OCULUSDEVICE_H_ */
