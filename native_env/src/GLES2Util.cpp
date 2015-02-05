/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include <memory>
#include "System.h"
#include "SystemFile.h"
#include "GLES2Util.h"

static bool CheckShaderStatus(GLuint shader)
{
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        int messageLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &messageLength);
        if (messageLength > 1)
        {
            std::unique_ptr<char[]> message(new char[messageLength]);
            glGetShaderInfoLog(shader, messageLength, 0, message.get());
            LOG_DEBUG(LOG_SYS, "GLSL compiler message: %s\n", message.get());
        }
        return false;
    }

    return true;
}

static bool CheckFragmentProgramStatus(GLuint program, GLenum mode)
{
    GLint status;
    glGetProgramiv(program, mode, &status);
    if (status == 0)
    {
        int messageLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &messageLength);
        if (messageLength > 1)
        {
            std::unique_ptr<char[]> message(new char[messageLength]);
            glGetProgramInfoLog(program, messageLength, 0, message.get());
            LOG_DEBUG(LOG_SYS, "GLSL compiler message: %s\n", message.get());
        }
        return false;
    }

    return true;
}

static bool CreateFragmentProgramFromStrings(GLuint programId, GLuint vertexShaderId, GLuint fragmentShaderId,
                                             const char *vertexShader, const char *fragmentShader)
{
    glShaderSource(vertexShaderId, 1, &vertexShader, 0);
    glCompileShader(vertexShaderId);
    if (!CheckShaderStatus(vertexShaderId))
    {
        return false;
    }

    glShaderSource(fragmentShaderId, 1, &fragmentShader, 0);
    glCompileShader(fragmentShaderId);
    if (!CheckShaderStatus(fragmentShaderId))
    {
        return false;
    }

    // link the fragment program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    if (!CheckFragmentProgramStatus(programId, GL_LINK_STATUS))
    {
        return false;
    }

    // FIXME: move validation to a debug check in render call
//    glValidateProgram(programId);
//    if (!CheckFragmentProgramStatus(programId, GL_VALIDATE_STATUS))
//    {
//        return false;
//    }

    return true;
}

bool GLES2Util::CreateFragmentProgram(GLuint &destProgramId, const char *vertexShader, const char *fragmentShader)
{
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint programId = glCreateProgram();

    if (!CreateFragmentProgramFromStrings(programId, vertexShaderId, fragmentShaderId, vertexShader, fragmentShader))
    {
        glDeleteProgram(programId);
        glDeleteShader(fragmentShaderId);
        glDeleteShader(vertexShaderId);
        return false;
    }

    glDeleteShader(fragmentShaderId);
    glDeleteShader(vertexShaderId);

    destProgramId = programId;
    return true;
}

bool GLES2Util::LoadFragmentProgram(GLuint &programId, const char *vertexShaderFileName,
                                    const char *fragmentShaderFileName)
{
    System::File vertexShaderFile, fragmentShaderFile;

    if (!vertexShaderFile.open(vertexShaderFileName, System::File::FileRead))
    {
        LOG_DEBUG(LOG_SYS, "unable to open \"%s\"!", vertexShaderFileName);
        return false;
    }

    int vertexShaderStringSize = vertexShaderFile.getSize();
    std::unique_ptr<char[]> vertexShaderString(new char[vertexShaderStringSize + 1]);
    vertexShaderFile.readBytes(vertexShaderString.get(), vertexShaderStringSize);
    vertexShaderString[vertexShaderStringSize] = 0;
    vertexShaderFile.close();

    if (!fragmentShaderFile.open(fragmentShaderFileName, System::File::FileRead))
    {
        LOG_DEBUG(LOG_SYS, "unable to open \"%s\"!", fragmentShaderFileName);
        return false;
    }

    int fragmentShaderStringSize = fragmentShaderFile.getSize();
    std::unique_ptr<char[]> fragmentShaderString(new char[fragmentShaderStringSize + 1]);
    fragmentShaderFile.readBytes(fragmentShaderString.get(), fragmentShaderStringSize);
    fragmentShaderString[fragmentShaderStringSize] = 0;
    fragmentShaderFile.close();

    return CreateFragmentProgram(programId, vertexShaderString.get(), fragmentShaderString.get());
}

// =============================================================================
// QuadRenderer
// =============================================================================

static char const gDefaultVertexShader[] =
        "uniform mat4 uTransform;\n\
        attribute vec2 aPosition;\n\
        attribute vec2 aTexCoord;\n\
        varying vec2 vTexCoord;\n\
        void main(void)\n\
        {\n\
            vTexCoord = aTexCoord;\n\
            gl_Position = uTransform * vec4(aPosition.x, aPosition.y, 0.0, 1.0);\n\
        }\n";

char const GLES2Util::QuadRenderer::DefaultFragmentShaderSampler2D[] =
        "precision mediump float;\n\
        uniform sampler2D uSourceTex;\n\
        varying vec2 vTexCoord;\n\
        void main(void)\n\
        {\n\
            gl_FragColor = texture2D(uSourceTex, vTexCoord);\n\
        }\n";

char const GLES2Util::QuadRenderer::DefaultFragmentShaderSamplerExternalOES[] =
        "#extension GL_OES_EGL_image_external : require\n\
        precision mediump float;\n\
        uniform samplerExternalOES uSourceTex;\n\
        varying vec2 vTexCoord;\n\
        void main(void)\n\
        {\n\
            gl_FragColor = texture2D(uSourceTex, vTexCoord);\n\
        }\n";

GLES2Util::QuadRenderer::QuadRenderer(GLuint programId, bool programOwner, float const *textureCoords)
        : mOutputProgramOwner(programOwner)
{
    for (int i = 0; i < 8; i++)
    {
        mTextureCoords[i] = textureCoords[i];
    }
    mTransformMatrix.setIdentity();

    mOutputProgramId = programId;
    glUseProgram(programId);
    mSourceTexUniform = glGetUniformLocation(programId, "uSourceTex");
    mTransformUniform = glGetUniformLocation(programId, "uTransform");
    mPositionAttrib = glGetAttribLocation(programId, "aPosition");
    mTexCoordAttrib = glGetAttribLocation(programId, "aTexCoord");
}

GLES2Util::QuadRenderer::~QuadRenderer(void)
{
    if (mOutputProgramOwner)
    {
        glDeleteProgram(mOutputProgramId);
    }
}

GLES2Util::QuadRenderer *GLES2Util::QuadRenderer::Create(const char *fragmentShader)
{
    if (fragmentShader == 0)
    {
        return 0;
    }

    GLuint programId;
    if (!CreateFragmentProgram(programId, gDefaultVertexShader, fragmentShader))
    {
        LOG_DEBUG(LOG_SYS, "error compiling fragment program!");
        return 0;
    }

    static float const textureCoord[] =
    { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };

    return new QuadRenderer(programId, true, textureCoord);
}

GLES2Util::QuadRenderer *GLES2Util::QuadRenderer::Create(GLuint programId)
{
    static float const textureCoord[] =
    { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };

    return new QuadRenderer(programId, false, textureCoord);
}

void GLES2Util::QuadRenderer::setTransform(Eigen::Matrix4f const &matrix)
{
    mTransformMatrix = matrix;
}

void GLES2Util::QuadRenderer::draw(GLenum srcTextureTarget, GLuint srcTexture)
{
    static float const vertexPosition[] =
    { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f };

    glUseProgram(mOutputProgramId);

    // setup uniforms
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(srcTextureTarget, srcTexture);
    glUniform1i(mSourceTexUniform, 0);
    glUniformMatrix4fv(mTransformUniform, 1, GL_FALSE, mTransformMatrix.data());

    glVertexAttribPointer(mPositionAttrib, 2, GL_FLOAT, GL_FALSE, 0, vertexPosition);
    glVertexAttribPointer(mTexCoordAttrib, 2, GL_FLOAT, GL_FALSE, 0, mTextureCoords);
    glEnableVertexAttribArray(mPositionAttrib);
    glEnableVertexAttribArray(mTexCoordAttrib);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(mPositionAttrib);
    glDisableVertexAttribArray(mTexCoordAttrib);
}

// =============================================================================
// QuadToTextureRenderer
// =============================================================================

GLES2Util::QuadToTextureRenderer::QuadToTextureRenderer(GLuint programId, bool programOwner, float const *textureCoords)
        : QuadRenderer(programId, programOwner, textureCoords)
{
    glGenFramebuffers(1, &mFBO);

    mOutputTextureId = 0;
    mOutputTextureWidth = 0;
    mOutputTextureHeight = 0;
}

GLES2Util::QuadToTextureRenderer::~QuadToTextureRenderer(void)
{
    glDeleteFramebuffers(1, &mFBO);
}

GLES2Util::QuadToTextureRenderer *GLES2Util::QuadToTextureRenderer::Create(const char *fragmentShader)
{
    GLuint programId;
    if (!CreateFragmentProgram(programId, gDefaultVertexShader, fragmentShader))
    {
        LOG_DEBUG(LOG_SYS, "error compiling fragment program!");
        return 0;
    }

    static float const textureCoord[] =
    { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    return new QuadToTextureRenderer(programId, true, textureCoord);
}

GLES2Util::QuadToTextureRenderer *GLES2Util::QuadToTextureRenderer::Create(GLuint programId)
{
    static float const textureCoord[] =
    { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    return new QuadToTextureRenderer(programId, false, textureCoord);
}

void GLES2Util::QuadToTextureRenderer::setDestination(GLuint dstTexture, uint width, uint height)
{
    mOutputTextureId = dstTexture;
    mOutputTextureWidth = width;
    mOutputTextureHeight = height;
}

void GLES2Util::QuadToTextureRenderer::draw(GLenum srcTextureTarget, GLuint srcTexture)
{
    if (mOutputTextureId == 0)
    {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mOutputTextureId, 0);

    GLenum glstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (glstatus != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_DEBUG(LOG_SYS, "framebuffer incomplete!");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glViewport(0, 0, mOutputTextureWidth, mOutputTextureHeight);

    QuadRenderer::draw(srcTextureTarget, srcTexture);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLES2Util::QuadToTextureRenderer::drawAndReadBack(GLenum srcTextureTarget, GLuint srcTexture, void *dst, int pixelFormat, int channelType)
{
    if (mOutputTextureId == 0)
    {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mOutputTextureId, 0);

    GLenum glstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (glstatus != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_DEBUG(LOG_SYS, "framebuffer incomplete!");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glViewport(0, 0, mOutputTextureWidth, mOutputTextureHeight);

    QuadRenderer::draw(srcTextureTarget, srcTexture);
    glReadPixels(0, 0, mOutputTextureWidth, mOutputTextureHeight, pixelFormat, channelType, dst);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
