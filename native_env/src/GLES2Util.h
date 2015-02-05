/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef GLES2UTIL_H_
#define GLES2UTIL_H_

#ifdef ANDROID
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#endif
#include "Base.h"
#include <Eigen/Dense>

namespace GLES2Util
{

bool CreateFragmentProgram(GLuint &programId, const char *vertexShader, const char *fragmentShader);
bool LoadFragmentProgram(GLuint &programId, const char *vertexShaderFileName, const char *fragmentShaderFileName);

class QuadRenderer
{
public:
    virtual ~QuadRenderer(void);

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    void setTransform(Eigen::Matrix4f const &matrix);
    virtual void draw(GLenum srcTextureTarget, GLuint srcTexture);

    /**
     * Requirements for vertex/fragment program pair
     *
     * example vertex shader:
     *
     * uniform mat4 uTransform;  // name of the transformation matrix uniform is fixed
     * attribute vec2 aPosition; // same goes for point and texture coordinates
     * attribute vec2 aTexCoord;
     * varying vec2 vTexCoord;
     *
     * void main(void)
     * {
     *   vTexCoord = aTexCoord;
     *   gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0) * uTransform;
     * }
     *
     * example fragment shader
     *
     * precision mediump float;
     * uniform sampler2D uSourceTex; // name of the source texture sampler is fixed
     * varying vec2 vTexCoord;
     *
     * void main(void)
     * {
     *   gl_FragColor = texture2D(uSourceTex, vTexCoord);
     * }
     */
    static QuadRenderer *Create(const char *fragmentShader);
    static QuadRenderer *Create(GLuint programId);

    GLuint getProgramId(void) const
    {
        return mOutputProgramId;
    }

    static char const DefaultFragmentShaderSampler2D[];
    static char const DefaultFragmentShaderSamplerExternalOES[];

protected:
    QuadRenderer(GLuint programId, bool programOwner, float const *textureCoords);

	Eigen::Matrix4f mTransformMatrix;

    bool const mOutputProgramOwner;
    GLuint mOutputProgramId;
    GLint mSourceTexUniform;
    GLint mTransformUniform;
    GLint mPositionAttrib;
    GLint mTexCoordAttrib;

    float mTextureCoords[8];

private:
    QuadRenderer(QuadRenderer const &);
    QuadRenderer &operator=(QuadRenderer const &);
};

class QuadToTextureRenderer: public QuadRenderer
{
public:
    ~QuadToTextureRenderer(void);

    void setDestination(GLuint dstTexture, uint width, uint height);
    void draw(GLenum srcTextureTarget, GLuint srcTexture);
    void drawAndReadBack(GLenum srcTextureTarget, GLuint srcTexture, void *dst, int pixelFormat, int channelType);

    static QuadToTextureRenderer *Create(const char *fragmentShader);
    static QuadToTextureRenderer *Create(GLuint programId);

private:
    QuadToTextureRenderer(QuadToTextureRenderer const &);
    QuadToTextureRenderer &operator=(QuadToTextureRenderer const &);

    QuadToTextureRenderer(GLuint programId, bool programOwner, float const *textureCoords);

    GLuint mFBO;
    GLuint mOutputTextureId;
    uint mOutputTextureWidth, mOutputTextureHeight;
};

}

#endif /* GLES2UTIL_H_ */
