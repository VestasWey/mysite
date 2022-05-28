//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/gles_loader_autogen.h"
#include "util/random_utils.h"
#include "util/test_utils.h"

using namespace angle;

namespace
{

class TransformFeedbackTestBase : public ANGLETest
{
  protected:
    TransformFeedbackTestBase() : mProgram(0), mTransformFeedbackBuffer(0), mTransformFeedback(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        glGenBuffers(1, &mTransformFeedbackBuffer);
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBufferSize, nullptr,
                     GL_STATIC_DRAW);

        glGenTransformFeedbacks(1, &mTransformFeedback);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
            mProgram = 0;
        }

        if (mTransformFeedbackBuffer != 0)
        {
            glDeleteBuffers(1, &mTransformFeedbackBuffer);
            mTransformFeedbackBuffer = 0;
        }

        if (mTransformFeedback != 0)
        {
            glDeleteTransformFeedbacks(1, &mTransformFeedback);
            mTransformFeedback = 0;
        }
    }

    GLuint mProgram;

    static const size_t mTransformFeedbackBufferSize = 1 << 24;
    GLuint mTransformFeedbackBuffer;
    GLuint mTransformFeedback;
};

class TransformFeedbackTest : public TransformFeedbackTestBase
{
  protected:
    void compileDefaultProgram(const std::vector<std::string> &tfVaryings, GLenum bufferMode)
    {
        ASSERT_EQ(0u, mProgram);

        mProgram = CompileProgramWithTransformFeedback(
            essl1_shaders::vs::Simple(), essl1_shaders::fs::Red(), tfVaryings, bufferMode);
        ASSERT_NE(0u, mProgram);
    }

    void setupOverrunTest(const std::vector<GLfloat> &vertices);

    void midRecordOpDoesNotContributeTest(std::function<void()> op);
};

TEST_P(TransformFeedbackTest, ZeroSizedViewport)
{
    // http://anglebug.com/5154
    ANGLE_SKIP_TEST_IF(IsOSX() && IsOpenGL());

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glUseProgram(mProgram);

    // Bind the buffer for transform feedback output and start transform feedback
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glBeginTransformFeedback(GL_TRIANGLES);

    // Create a query to check how many primitives were written
    GLQuery primitivesWrittenQuery;
    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitivesWrittenQuery);

    // Set a viewport that would result in no pixels being written to the framebuffer and draw
    // a quad
    glViewport(0, 0, 0, 0);

    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);

    // End the query and transform feedback
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glEndTransformFeedback();

    glUseProgram(0);

    // Check how many primitives were written and verify that some were written even if
    // no pixels were rendered
    GLuint primitivesWritten = 0;
    glGetQueryObjectuiv(primitivesWrittenQuery, GL_QUERY_RESULT_EXT, &primitivesWritten);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(2u, primitivesWritten);
}

// Test that rebinding a buffer with the same offset resets the offset (no longer appending from the
// old position)
TEST_P(TransformFeedbackTest, BufferRebinding)
{
    // http://anglebug.com/5154
    ANGLE_SKIP_TEST_IF(IsOSX() && IsOpenGL());

    glDisable(GL_DEPTH_TEST);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glUseProgram(mProgram);

    // Make sure the buffer has zero'd data
    std::vector<float> data(mTransformFeedbackBufferSize / sizeof(float), 0.0f);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBufferSize, data.data(),
                 GL_STATIC_DRAW);

    // Create a query to check how many primitives were written
    GLQuery primitivesWrittenQuery;
    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitivesWrittenQuery);

    const float finalZ = 0.95f;

    RNG rng;

    const size_t loopCount = 64;
    for (size_t loopIdx = 0; loopIdx < loopCount; loopIdx++)
    {
        // Bind the buffer for transform feedback output and start transform feedback
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
        glBeginTransformFeedback(GL_TRIANGLES);

        float z = (loopIdx + 1 == loopCount) ? finalZ : rng.randomFloatBetween(0.1f, 0.5f);
        drawQuad(mProgram, essl1_shaders::PositionAttrib(), z);

        glEndTransformFeedback();
    }

    // End the query and transform feedback
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glUseProgram(0);

    // Check how many primitives were written and verify that some were written even if
    // no pixels were rendered
    GLuint primitivesWritten = 0;
    glGetQueryObjectuiv(primitivesWrittenQuery, GL_QUERY_RESULT_EXT, &primitivesWritten);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(loopCount * 2, primitivesWritten);

    // Check the buffer data
    const float *bufferData = static_cast<float *>(glMapBufferRange(
        GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBufferSize, GL_MAP_READ_BIT));

    for (size_t vertexIdx = 0; vertexIdx < 6; vertexIdx++)
    {
        // Check the third (Z) component of each vertex written and make sure it has the final
        // value
        EXPECT_NEAR(finalZ, bufferData[vertexIdx * 4 + 2], 0.0001);
    }

    for (size_t dataIdx = 24; dataIdx < mTransformFeedbackBufferSize / sizeof(float); dataIdx++)
    {
        EXPECT_EQ(data[dataIdx], bufferData[dataIdx]) << "Buffer overrun detected.";
    }

    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Test that XFB can write back vertices to a buffer and that we can draw from this buffer
// afterward.
TEST_P(TransformFeedbackTest, RecordAndDraw)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // Fails on Mac GL drivers. http://anglebug.com/4992
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsOSX());

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());

    // First pass: draw 6 points to the XFB buffer
    glEnable(GL_RASTERIZER_DISCARD);

    const GLfloat vertices[] = {
        -1.0f, 1.0f, 0.5f, -1.0f, -1.0f, 0.5f, 1.0f, -1.0f, 0.5f,
        -1.0f, 1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f,  0.5f,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    // Bind the buffer for transform feedback output and start transform feedback
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glBeginTransformFeedback(GL_POINTS);

    // Create a query to check how many primitives were written
    GLQuery primitivesWrittenQuery;
    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitivesWrittenQuery);

    glDrawArrays(GL_POINTS, 0, 6);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    // End the query and transform feedback
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glEndTransformFeedback();

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    // Check how many primitives were written and verify that some were written even if
    // no pixels were rendered
    GLuint primitivesWritten = 0;
    glGetQueryObjectuiv(primitivesWrittenQuery, GL_QUERY_RESULT_EXT, &primitivesWritten);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(6u, primitivesWritten);

    // Nothing should have been drawn to the framebuffer
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 0, 0);

    // Second pass: draw from the feedback buffer

    glBindBuffer(GL_ARRAY_BUFFER, mTransformFeedbackBuffer);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);
    EXPECT_GL_NO_ERROR();
}

// Test that transform feedback can cover multiple render passes.
TEST_P(TransformFeedbackTest, SpanMultipleRenderPasses)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // Fails on Mac GL drivers. http://anglebug.com/4992
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsOSX());

    // anglebug.com/5429
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());

    const GLfloat vertices[] = {
        -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f, 0.5f,  0.5f,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    // Bind the buffer for transform feedback output and start transform feedback
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glBeginTransformFeedback(GL_POINTS);

    // Create a query to check how many primitives were written
    GLQuery primitivesWrittenQuery;
    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitivesWrittenQuery);

    // Draw the first set of three points
    glDrawArrays(GL_POINTS, 0, 3);

    // Break the render pass
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Draw the second set of three points
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices + 9);
    glDrawArrays(GL_POINTS, 0, 3);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    // End the query and transform feedback
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glEndTransformFeedback();

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    // Verify the number of primitives written
    GLuint primitivesWritten = 0;
    glGetQueryObjectuiv(primitivesWrittenQuery, GL_QUERY_RESULT_EXT, &primitivesWritten);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(6u, primitivesWritten);

    // Verify the captured buffer.

    glBindBuffer(GL_ARRAY_BUFFER, mTransformFeedbackBuffer);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(w - 1, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(0, h - 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(w - 1, h - 1, GLColor::black);

    EXPECT_PIXEL_COLOR_EQ(w / 4 + 1, h / 4 + 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * w / 4 - 1, h / 4 + 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(w / 4 + 1, 3 * h / 4 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * w / 4 - 1, 3 * h / 4 - 1, GLColor::red);

    EXPECT_PIXEL_COLOR_EQ(w / 2, h / 2, GLColor::red);

    EXPECT_GL_NO_ERROR();
}

void TransformFeedbackTest::midRecordOpDoesNotContributeTest(std::function<void()> op)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());

    const GLfloat vertices[] = {
        -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f, 0.5f,  0.5f,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    // Bind the buffer for transform feedback output and start transform feedback
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glBeginTransformFeedback(GL_POINTS);

    // Create a query to check how many primitives were written
    GLQuery primitivesWrittenQuery;
    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitivesWrittenQuery);

    // Draw the first set of three points
    glDrawArrays(GL_POINTS, 0, 3);

    // Perform the operation in the middle of recording
    op();

    // Draw the second set of three points
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices + 9);
    glDrawArrays(GL_POINTS, 0, 3);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    // End the query and transform feedback
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glEndTransformFeedback();

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    // Verify the number of primitives written
    GLuint primitivesWritten = 0;
    glGetQueryObjectuiv(primitivesWrittenQuery, GL_QUERY_RESULT_EXT, &primitivesWritten);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(6u, primitivesWritten);

    // Verify the captured buffer.
    glBindBuffer(GL_ARRAY_BUFFER, mTransformFeedbackBuffer);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Test that draw-based clear between draws does not contribute to transform feedback.
TEST_P(TransformFeedbackTest, ClearWhileRecordingDoesNotContribute)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // Fails on Mac GL drivers. http://anglebug.com/4992
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsOSX());

    // anglebug.com/5434
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    auto clear = []() {
        glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);
    };

    midRecordOpDoesNotContributeTest(clear);

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(w - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, h - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(w - 1, h - 1, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(w / 4 + 1, h / 4 + 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(3 * w / 4 - 1, h / 4 + 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(w / 4 + 1, 3 * h / 4 - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(3 * w / 4 - 1, 3 * h / 4 - 1, GLColor::magenta);

    EXPECT_PIXEL_COLOR_EQ(w / 2, h / 2, GLColor::magenta);

    EXPECT_GL_NO_ERROR();
}

// Test that copy in the middle of rendering doesn't contribute to transform feedback.
TEST_P(TransformFeedbackTest, CopyWhileRecordingDoesNotContribute)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // Fails on Mac GL drivers. http://anglebug.com/4992
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsOSX());

    // anglebug.com/5434
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    auto copy = []() {
        GLTexture texture;
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
    };

    midRecordOpDoesNotContributeTest(copy);

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(w - 1, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(0, h - 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(w - 1, h - 1, GLColor::black);

    EXPECT_PIXEL_COLOR_EQ(w / 4 + 1, h / 4 + 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * w / 4 - 1, h / 4 + 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(w / 4 + 1, 3 * h / 4 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * w / 4 - 1, 3 * h / 4 - 1, GLColor::red);

    EXPECT_PIXEL_COLOR_EQ(w / 2, h / 2, GLColor::red);

    EXPECT_GL_NO_ERROR();
}

// Test that blit in the middle of rendering doesn't contribute to transform feedback.
TEST_P(TransformFeedbackTest, BlitWhileRecordingDoesNotContribute)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // Fails on Mac GL drivers. http://anglebug.com/4992
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsOSX());

    // anglebug.com/5434
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    auto blit = []() {
        GLFramebuffer dstFbo;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);

        GLTexture dstTex;
        glBindTexture(GL_TEXTURE_2D, dstTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTex, 0);

        glBlitFramebuffer(0, 0, 1, 1, 1, 1, 0, 0, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    };

    midRecordOpDoesNotContributeTest(blit);

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(w - 1, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(0, h - 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(w - 1, h - 1, GLColor::black);

    EXPECT_PIXEL_COLOR_EQ(w / 4 + 1, h / 4 + 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * w / 4 - 1, h / 4 + 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(w / 4 + 1, 3 * h / 4 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * w / 4 - 1, 3 * h / 4 - 1, GLColor::red);

    EXPECT_PIXEL_COLOR_EQ(w / 2, h / 2, GLColor::red);

    EXPECT_GL_NO_ERROR();
}

// Test that XFB does not allow writing more vertices than fit in the bound buffers.
// TODO(jmadill): Enable this test after fixing the last case where the buffer size changes after
// calling glBeginTransformFeedback.
TEST_P(TransformFeedbackTest, DISABLED_TooSmallBuffers)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_RASTERIZER_DISCARD);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);
    GLint positionLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());

    glUseProgram(mProgram);

    const GLfloat vertices[] = {
        -1.0f, 1.0f, 0.5f, -1.0f, -1.0f, 0.5f, 1.0f, -1.0f, 0.5f,

        -1.0f, 1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f,  0.5f,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    const size_t verticesToDraw = 6;
    const size_t stride         = sizeof(float) * 4;
    const size_t bytesNeeded    = stride * verticesToDraw;

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    // Set up the buffer to be the right size
    uint8_t tfData[stride * verticesToDraw] = {0};
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, bytesNeeded, &tfData, GL_STATIC_DRAW);

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, verticesToDraw);
    EXPECT_GL_NO_ERROR();
    glEndTransformFeedback();

    // Set up the buffer to be too small
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, bytesNeeded - 1, &tfData, GL_STATIC_DRAW);

    glBeginTransformFeedback(GL_POINTS);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, verticesToDraw);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glEndTransformFeedback();

    // Set up the buffer to be the right size but make it smaller after glBeginTransformFeedback
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, bytesNeeded, &tfData, GL_STATIC_DRAW);
    glBeginTransformFeedback(GL_POINTS);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, bytesNeeded - 1, &tfData, GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, verticesToDraw);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glEndTransformFeedback();
}

// Test that buffer binding happens only on the current transform feedback object
TEST_P(TransformFeedbackTest, BufferBinding)
{
    // http://anglebug.com/5154
    ANGLE_SKIP_TEST_IF(IsOSX() && IsOpenGL());

    // Reset any state
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    // Generate a new buffer
    GLuint scratchBuffer = 0;
    glGenBuffers(1, &scratchBuffer);

    EXPECT_GL_NO_ERROR();

    // Bind TF 0 and a buffer
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    EXPECT_GL_NO_ERROR();

    // Check that the buffer ID matches the one that was just bound
    GLint currentBufferBinding = 0;
    glGetIntegerv(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, &currentBufferBinding);
    EXPECT_EQ(static_cast<GLuint>(currentBufferBinding), mTransformFeedbackBuffer);

    glGetIntegeri_v(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, 0, &currentBufferBinding);
    EXPECT_EQ(static_cast<GLuint>(currentBufferBinding), mTransformFeedbackBuffer);

    EXPECT_GL_NO_ERROR();

    // Check that the buffer ID for the newly bound transform feedback is zero
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);

    glGetIntegeri_v(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, 0, &currentBufferBinding);
    EXPECT_EQ(0, currentBufferBinding);

    // But the generic bind point is unaffected by glBindTransformFeedback.
    glGetIntegerv(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, &currentBufferBinding);
    EXPECT_EQ(static_cast<GLuint>(currentBufferBinding), mTransformFeedbackBuffer);

    EXPECT_GL_NO_ERROR();

    // Bind a buffer to this TF
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, scratchBuffer, 0, 32);

    glGetIntegeri_v(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, 0, &currentBufferBinding);
    EXPECT_EQ(static_cast<GLuint>(currentBufferBinding), scratchBuffer);

    EXPECT_GL_NO_ERROR();

    // Rebind the original TF and check it's bindings
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    glGetIntegeri_v(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, 0, &currentBufferBinding);
    EXPECT_EQ(static_cast<GLuint>(currentBufferBinding), mTransformFeedbackBuffer);

    EXPECT_GL_NO_ERROR();

    // Clean up
    glDeleteBuffers(1, &scratchBuffer);
}

// Test that we can capture varyings only used in the vertex shader.
TEST_P(TransformFeedbackTest, VertexOnly)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec2 position;\n"
        "in float attrib;\n"
        "out float varyingAttrib;\n"
        "void main() {\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "  varyingAttrib = attrib;\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "out mediump vec4 color;\n"
        "void main() {\n"
        "  color = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "}";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("varyingAttrib");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    std::vector<float> attribData;
    for (unsigned int cnt = 0; cnt < 100; ++cnt)
    {
        attribData.push_back(static_cast<float>(cnt));
    }

    GLint attribLocation = glGetAttribLocation(mProgram, "attrib");
    ASSERT_NE(-1, attribLocation);

    glVertexAttribPointer(attribLocation, 1, GL_FLOAT, GL_FALSE, 4, &attribData[0]);
    glEnableVertexAttribArray(attribLocation);

    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(mProgram, "position", 0.5f);
    glEndTransformFeedback();
    ASSERT_GL_NO_ERROR();

    glUseProgram(0);

    void *mappedBuffer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(float) * 6, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mappedBuffer);

    float *mappedFloats = static_cast<float *>(mappedBuffer);
    for (unsigned int cnt = 0; cnt < 6; ++cnt)
    {
        EXPECT_EQ(attribData[cnt], mappedFloats[cnt]);
    }
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Test that multiple paused transform feedbacks do not generate errors or crash
TEST_P(TransformFeedbackTest, MultiplePaused)
{
    // Crashes on Mac Intel GL drivers. http://anglebug.com/4992
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsOSX());

    const size_t drawSize = 1024;
    std::vector<float> transformFeedbackData(drawSize);
    for (size_t i = 0; i < drawSize; i++)
    {
        transformFeedbackData[i] = static_cast<float>(i + 1);
    }

    // Initialize the buffers to zero
    size_t bufferSize = drawSize;
    std::vector<float> bufferInitialData(bufferSize, 0);

    const size_t transformFeedbackCount = 8;

    constexpr char kVS[] = R"(#version 300 es
in highp vec4 position;
in float transformFeedbackInput;
out float transformFeedbackOutput;
void main(void)
{
    gl_Position = position;
    transformFeedbackOutput = transformFeedbackInput;
})";

    constexpr char kFS[] = R"(#version 300 es
out mediump vec4 color;
void main(void)
{
    color = vec4(1.0, 1.0, 1.0, 1.0);
})";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("transformFeedbackOutput");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, mProgram);
    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    glDisableVertexAttribArray(positionLocation);
    glVertexAttrib4f(positionLocation, 0.0f, 0.0f, 0.0f, 1.0f);

    GLint tfInputLocation = glGetAttribLocation(mProgram, "transformFeedbackInput");
    glEnableVertexAttribArray(tfInputLocation);
    glVertexAttribPointer(tfInputLocation, 1, GL_FLOAT, false, 0, &transformFeedbackData[0]);

    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    ASSERT_GL_NO_ERROR();

    GLuint transformFeedbacks[transformFeedbackCount];
    glGenTransformFeedbacks(transformFeedbackCount, transformFeedbacks);

    GLuint buffers[transformFeedbackCount];
    glGenBuffers(transformFeedbackCount, buffers);

    for (size_t i = 0; i < transformFeedbackCount; i++)
    {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbacks[i]);

        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffers[i]);
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, bufferSize * sizeof(GLfloat),
                     &bufferInitialData[0], GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers[i]);
        ASSERT_GL_NO_ERROR();

        glBeginTransformFeedback(GL_POINTS);

        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(drawSize));

        glPauseTransformFeedback();

        EXPECT_GL_NO_ERROR();
    }

    for (size_t i = 0; i < transformFeedbackCount; i++)
    {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbacks[i]);
        glEndTransformFeedback();
        glDeleteTransformFeedbacks(1, &transformFeedbacks[i]);

        EXPECT_GL_NO_ERROR();
    }
}
// Test that running multiple simultaneous queries and transform feedbacks from multiple EGL
// contexts returns the correct results.  Helps expose bugs in ANGLE's virtual contexts.
TEST_P(TransformFeedbackTest, MultiContext)
{
    // These tests are flaky, do not lift these unless you find the root cause and the fix.
    ANGLE_SKIP_TEST_IF(IsOSX() && IsOpenGL());

    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsOpenGL());

    // Flaky on Win Intel Vulkan. http://anglebug.com/4497
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION_KHR,
        GetParam().majorVersion,
        EGL_CONTEXT_MINOR_VERSION_KHR,
        GetParam().minorVersion,
        EGL_NONE,
    };

    // Keep a fixed seed RNG so we are deterministic.
    RNG rng(0);
    EGLWindow *window = getEGLWindow();

    EGLDisplay display = window->getDisplay();
    EGLConfig config   = window->getConfig();
    EGLSurface surface = window->getSurface();

    const size_t passCount = 5;
    struct ContextInfo
    {
        EGLContext context;
        GLuint program;
        GLuint query;
        GLuint buffer;
        size_t primitiveCounts[passCount];
    };
    static constexpr uint32_t kContextCount = 32;
    ContextInfo contexts[kContextCount];

    const size_t maxDrawSize = 512;

    std::vector<float> transformFeedbackData(maxDrawSize);
    for (size_t i = 0; i < maxDrawSize; i++)
    {
        transformFeedbackData[i] = static_cast<float>(i + 1);
    }

    // Initialize the buffers to zero
    size_t bufferSize = maxDrawSize * passCount;
    std::vector<float> bufferInitialData(bufferSize, 0);

    constexpr char kVS[] = R"(#version 300 es
in highp vec4 position;
in float transformFeedbackInput;
out float transformFeedbackOutput;
void main(void)
{
    gl_Position = position;
    transformFeedbackOutput = transformFeedbackInput;
})";

    constexpr char kFS[] = R"(#version 300 es
out mediump vec4 color;
void main(void)
{
    color = vec4(1.0, 1.0, 1.0, 1.0);
})";

    for (ContextInfo &context : contexts)
    {
        context.context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
        ASSERT_NE(context.context, EGL_NO_CONTEXT);

        eglMakeCurrent(display, surface, surface, context.context);

        std::vector<std::string> tfVaryings;
        tfVaryings.push_back("transformFeedbackOutput");

        context.program =
            CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
        ASSERT_NE(context.program, 0u);
        glUseProgram(context.program);

        GLint positionLocation = glGetAttribLocation(context.program, "position");
        glDisableVertexAttribArray(positionLocation);
        glVertexAttrib4f(positionLocation, 0.0f, 0.0f, 0.0f, 1.0f);

        GLint tfInputLocation = glGetAttribLocation(context.program, "transformFeedbackInput");
        glEnableVertexAttribArray(tfInputLocation);
        glVertexAttribPointer(tfInputLocation, 1, GL_FLOAT, false, 0, &transformFeedbackData[0]);

        glDepthMask(GL_FALSE);
        glEnable(GL_DEPTH_TEST);
        glGenQueriesEXT(1, &context.query);
        glBeginQueryEXT(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, context.query);

        ASSERT_GL_NO_ERROR();

        glGenBuffers(1, &context.buffer);
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, context.buffer);
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, bufferSize * sizeof(GLfloat),
                     &bufferInitialData[0], GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, context.buffer);

        ASSERT_GL_NO_ERROR();

        // For each pass, draw between 0 and maxDrawSize primitives
        for (size_t &primCount : context.primitiveCounts)
        {
            primCount = rng.randomIntBetween(1, maxDrawSize);
        }

        glBeginTransformFeedback(GL_POINTS);
    }

    for (size_t pass = 0; pass < passCount; pass++)
    {
        for (const auto &context : contexts)
        {
            eglMakeCurrent(display, surface, surface, context.context);

            glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(context.primitiveCounts[pass]));
        }
    }

    for (const auto &context : contexts)
    {
        eglMakeCurrent(display, surface, surface, context.context);

        glEndTransformFeedback();

        glEndQueryEXT(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

        GLuint result = 0;
        glGetQueryObjectuivEXT(context.query, GL_QUERY_RESULT_EXT, &result);

        EXPECT_GL_NO_ERROR();

        size_t totalPrimCount = 0;
        for (const auto &primCount : context.primitiveCounts)
        {
            totalPrimCount += primCount;
        }
        EXPECT_EQ(static_cast<GLuint>(totalPrimCount), result);

        const float *bufferData = reinterpret_cast<float *>(glMapBufferRange(
            GL_TRANSFORM_FEEDBACK_BUFFER, 0, bufferSize * sizeof(GLfloat), GL_MAP_READ_BIT));

        size_t curBufferIndex = 0;
        unsigned int failures = 0;
        for (const auto &primCount : context.primitiveCounts)
        {
            for (size_t prim = 0; prim < primCount; prim++)
            {
                failures += (bufferData[curBufferIndex] != (prim + 1)) ? 1 : 0;
                curBufferIndex++;
            }
        }

        EXPECT_EQ(0u, failures);

        while (curBufferIndex < bufferSize)
        {
            EXPECT_EQ(bufferData[curBufferIndex], 0.0f);
            curBufferIndex++;
        }

        glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
    }

    eglMakeCurrent(display, surface, surface, window->getContext());

    for (auto &context : contexts)
    {
        eglDestroyContext(display, context.context);
        context.context = EGL_NO_CONTEXT;
    }
}

// Test that when two vec2s are packed into the same register, we can still capture both of them.
TEST_P(TransformFeedbackTest, PackingBug)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // TODO(anglebug.com/5360): Timing out on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsOSX() && IsARM64() && IsDesktopOpenGL());

    // TODO(jmadill): With points and rasterizer discard?
    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec2 inAttrib1;\n"
        "in vec2 inAttrib2;\n"
        "out vec2 outAttrib1;\n"
        "out vec2 outAttrib2;\n"
        "in vec2 position;\n"
        "void main() {"
        "  outAttrib1 = inAttrib1;\n"
        "  outAttrib2 = inAttrib2;\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "  color = vec4(0);\n"
        "}";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("outAttrib1");
    tfVaryings.push_back("outAttrib2");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, mProgram);

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(Vector2) * 2 * 6, nullptr, GL_STREAM_DRAW);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    GLint attrib1Loc = glGetAttribLocation(mProgram, "inAttrib1");
    GLint attrib2Loc = glGetAttribLocation(mProgram, "inAttrib2");

    std::vector<Vector2> attrib1Data;
    std::vector<Vector2> attrib2Data;
    int counter = 0;
    for (size_t i = 0; i < 6; i++)
    {
        attrib1Data.push_back(Vector2(counter + 0.0f, counter + 1.0f));
        attrib2Data.push_back(Vector2(counter + 2.0f, counter + 3.0f));
        counter += 4;
    }

    glEnableVertexAttribArray(attrib1Loc);
    glEnableVertexAttribArray(attrib2Loc);

    glVertexAttribPointer(attrib1Loc, 2, GL_FLOAT, GL_FALSE, 0, attrib1Data.data());
    glVertexAttribPointer(attrib2Loc, 2, GL_FLOAT, GL_FALSE, 0, attrib2Data.data());

    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(mProgram, "position", 0.5f);
    glEndTransformFeedback();
    glUseProgram(0);
    ASSERT_GL_NO_ERROR();

    const void *mapPointer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(Vector2) * 2 * 6, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPointer);

    const Vector2 *vecPointer = static_cast<const Vector2 *>(mapPointer);
    for (unsigned int vectorIndex = 0; vectorIndex < 3; ++vectorIndex)
    {
        unsigned int stream1Index = vectorIndex * 2;
        unsigned int stream2Index = vectorIndex * 2 + 1;
        EXPECT_EQ(attrib1Data[vectorIndex], vecPointer[stream1Index]);
        EXPECT_EQ(attrib2Data[vectorIndex], vecPointer[stream2Index]);
    }
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    ASSERT_GL_NO_ERROR();
}

// Test that transform feedback varyings that can be optimized out yet do not cause program
// compilation to fail
TEST_P(TransformFeedbackTest, OptimizedVaryings)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 a_vertex;\n"
        "in vec3 a_normal; \n"
        "\n"
        "uniform Transform\n"
        "{\n"
        "    mat4 u_modelViewMatrix;\n"
        "    mat4 u_projectionMatrix;\n"
        "    mat3 u_normalMatrix;\n"
        "};\n"
        "\n"
        "out vec3 normal;\n"
        "out vec4 ecPosition;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    normal = normalize(u_normalMatrix * a_normal);\n"
        "    ecPosition = u_modelViewMatrix * a_vertex;\n"
        "    gl_Position = u_projectionMatrix * ecPosition;\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "\n"
        "in vec3 normal;\n"
        "in vec4 ecPosition;\n"
        "\n"
        "out vec4 fragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    fragColor = vec4(normal/2.0+vec3(0.5), 1);\n"
        "}\n";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("normal");
    tfVaryings.push_back("ecPosition");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, mProgram);
}

// Test an edge case where two varyings are unreferenced in the frag shader.
TEST_P(TransformFeedbackTest, TwoUnreferencedInFragShader)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());
    // TODO(anglebug.com/5360): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsOSX() && IsARM64() && IsDesktopOpenGL());

    // TODO(jmadill): With points and rasterizer discard?
    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec3 position;\n"
        "out vec3 outAttrib1;\n"
        "out vec3 outAttrib2;\n"
        "void main() {"
        "  outAttrib1 = position;\n"
        "  outAttrib2 = position;\n"
        "  gl_Position = vec4(position, 1);\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "in vec3 outAttrib1;\n"
        "in vec3 outAttrib2;\n"
        "void main() {\n"
        "  color = vec4(0);\n"
        "}";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("outAttrib1");
    tfVaryings.push_back("outAttrib2");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, mProgram);

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(Vector3) * 2 * 6, nullptr, GL_STREAM_DRAW);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(mProgram, "position", 0.5f);
    glEndTransformFeedback();
    glUseProgram(0);
    ASSERT_GL_NO_ERROR();

    const void *mapPointer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(Vector3) * 2 * 6, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPointer);

    const auto &quadVertices = GetQuadVertices();

    const Vector3 *vecPointer = static_cast<const Vector3 *>(mapPointer);
    for (unsigned int vectorIndex = 0; vectorIndex < 3; ++vectorIndex)
    {
        unsigned int stream1Index = vectorIndex * 2;
        unsigned int stream2Index = vectorIndex * 2 + 1;
        EXPECT_EQ(quadVertices[vectorIndex], vecPointer[stream1Index]);
        EXPECT_EQ(quadVertices[vectorIndex], vecPointer[stream2Index]);
    }
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    ASSERT_GL_NO_ERROR();
}

// Test that the transform feedback write offset is reset to the buffer's offset when
// glBeginTransformFeedback is called
TEST_P(TransformFeedbackTest, OffsetResetOnBeginTransformFeedback)
{
    // http://anglebug.com/5069
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsOSX() && IsAMD());

    // http://anglebug.com/5069
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsOpenGLES());

    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 position;\n"
        "out vec4 outAttrib;\n"
        "void main() {"
        "  outAttrib = position;\n"
        "  gl_Position = vec4(0);\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "  color = vec4(0);\n"
        "}";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("outAttrib");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(Vector4) * 2, nullptr, GL_STREAM_DRAW);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    glUseProgram(mProgram);

    Vector4 drawVertex0(4, 3, 2, 1);
    Vector4 drawVertex1(8, 7, 6, 5);
    Vector4 drawVertex2(12, 11, 10, 9);

    glEnableVertexAttribArray(positionLocation);

    glBeginTransformFeedback(GL_POINTS);

    // Write vertex 0 at offset 0
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, false, 0, &drawVertex0);
    glDrawArrays(GL_POINTS, 0, 1);

    // Append vertex 1
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, false, 0, &drawVertex1);
    glDrawArrays(GL_POINTS, 0, 1);

    glEndTransformFeedback();
    glBeginTransformFeedback(GL_POINTS);

    // Write vertex 2 at offset 0
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, false, 0, &drawVertex2);
    glDrawArrays(GL_POINTS, 0, 1);

    glEndTransformFeedback();

    const void *mapPointer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(Vector4) * 2, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPointer);

    const Vector4 *vecPointer = static_cast<const Vector4 *>(mapPointer);
    ASSERT_EQ(drawVertex2, vecPointer[0]);
    ASSERT_EQ(drawVertex1, vecPointer[1]);

    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    ASSERT_GL_NO_ERROR();
}

// Test that the captured buffer can be copied to other buffers.
TEST_P(TransformFeedbackTest, CaptureAndCopy)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());

    glEnable(GL_RASTERIZER_DISCARD);

    const GLfloat vertices[] = {
        -1.0f, 1.0f, 0.5f, -1.0f, -1.0f, 0.5f, 1.0f, -1.0f, 0.5f,

        -1.0f, 1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f,  0.5f,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    // Bind the buffer for transform feedback output and start transform feedback
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glBeginTransformFeedback(GL_POINTS);

    glDrawArrays(GL_POINTS, 0, 6);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEndTransformFeedback();
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    glDisable(GL_RASTERIZER_DISCARD);

    // Allocate a buffer with one byte
    uint8_t singleByte[] = {0xaa};

    // Create a new buffer and copy the first byte of captured data to it
    GLBuffer copyBuffer;
    glBindBuffer(GL_COPY_WRITE_BUFFER, copyBuffer);
    glBufferData(GL_COPY_WRITE_BUFFER, 1, singleByte, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    glCopyBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, 1);

    EXPECT_GL_NO_ERROR();
}

class TransformFeedbackLifetimeTest : public TransformFeedbackTest
{
  protected:
    TransformFeedbackLifetimeTest() : mVertexArray(0) {}

    void testSetUp() override
    {
        glGenVertexArrays(1, &mVertexArray);
        glBindVertexArray(mVertexArray);

        std::vector<std::string> tfVaryings;
        tfVaryings.push_back("gl_Position");
        compileDefaultProgram(tfVaryings, GL_SEPARATE_ATTRIBS);

        glGenBuffers(1, &mTransformFeedbackBuffer);
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBufferSize, nullptr,
                     GL_DYNAMIC_DRAW);
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

        glGenTransformFeedbacks(1, &mTransformFeedback);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteVertexArrays(1, &mVertexArray);
        TransformFeedbackTest::testTearDown();
    }

    GLuint mVertexArray;
};

// Tests a bug with state syncing and deleted transform feedback buffers.
TEST_P(TransformFeedbackLifetimeTest, DeletedBuffer)
{
    // First stream vertex data to mTransformFeedbackBuffer.
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    glUseProgram(mProgram);

    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    glEndTransformFeedback();

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    // TODO(jmadill): Remove this when http://anglebug.com/1351 is fixed.
    glBindVertexArray(0);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    glBindVertexArray(1);

    // Next, draw vertices with mTransformFeedbackBuffer. This will link to mVertexArray.
    glBindBuffer(GL_ARRAY_BUFFER, mTransformFeedbackBuffer);
    GLint loc = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, loc);
    glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Delete resources, making a stranded pointer to mVertexArray in mTransformFeedbackBuffer.
    glDeleteBuffers(1, &mTransformFeedbackBuffer);
    mTransformFeedbackBuffer = 0;
    glDeleteVertexArrays(1, &mVertexArray);
    mVertexArray = 0;

    // Then draw again with transform feedback, dereferencing the stranded pointer.
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    glEndTransformFeedback();
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    ASSERT_GL_NO_ERROR();
}

class TransformFeedbackTestES31 : public TransformFeedbackTestBase
{};

// Test that program link fails in case that transform feedback names including same array element.
TEST_P(TransformFeedbackTestES31, SameArrayElementVaryings)
{
    constexpr char kVS[] =
        "#version 310 es\n"
        "in vec3 position;\n"
        "out vec3 outAttribs[3];\n"
        "void main() {"
        "  outAttribs[0] = position;\n"
        "  outAttribs[1] = vec3(0, 0, 0);\n"
        "  outAttribs[2] = position;\n"
        "  gl_Position = vec4(position, 1);\n"
        "}";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "in vec3 outAttribs[3];\n"
        "void main() {\n"
        "  color = vec4(0);\n"
        "}";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("outAttribs");
    tfVaryings.push_back("outAttribs[1]");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_EQ(0u, mProgram);
}

// Test that program link fails in case to capture array element on a non-array varying.
TEST_P(TransformFeedbackTestES31, ElementCaptureOnNonArrayVarying)
{
    constexpr char kVS[] =
        "#version 310 es\n"
        "in vec3 position;\n"
        "out vec3 outAttrib;\n"
        "void main() {"
        "  outAttrib = position;\n"
        "  gl_Position = vec4(position, 1);\n"
        "}";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "in vec3 outAttrib;\n"
        "void main() {\n"
        "  color = vec4(0);\n"
        "}";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("outAttrib[1]");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_EQ(0u, mProgram);
}

// Test that program link fails in case to capure an outbound array element.
TEST_P(TransformFeedbackTestES31, CaptureOutboundElement)
{
    constexpr char kVS[] =
        "#version 310 es\n"
        "in vec3 position;\n"
        "out vec3 outAttribs[3];\n"
        "void main() {"
        "  outAttribs[0] = position;\n"
        "  outAttribs[1] = vec3(0, 0, 0);\n"
        "  outAttribs[2] = position;\n"
        "  gl_Position = vec4(position, 1);\n"
        "}";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "in vec3 outAttribs[3];\n"
        "void main() {\n"
        "  color = vec4(0);\n"
        "}";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("outAttribs[3]");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_EQ(0u, mProgram);
}

// Test transform feedback names can be specified using array element.
TEST_P(TransformFeedbackTestES31, DifferentArrayElementVaryings)
{
    // Remove this when http://anglebug.com/4140 is fixed.
    ANGLE_SKIP_TEST_IF(IsVulkan());

    constexpr char kVS[] =
        "#version 310 es\n"
        "in vec3 position;\n"
        "out vec3 outAttribs[3];\n"
        "void main() {"
        "  outAttribs[0] = position;\n"
        "  outAttribs[1] = vec3(0, 0, 0);\n"
        "  outAttribs[2] = position;\n"
        "  gl_Position = vec4(position, 1);\n"
        "}";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "in vec3 outAttribs[3];\n"
        "void main() {\n"
        "  color = vec4(0);\n"
        "}";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("outAttribs[0]");
    tfVaryings.push_back("outAttribs[2]");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, mProgram);

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(Vector3) * 2 * 6, nullptr, GL_STREAM_DRAW);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(mProgram, "position", 0.5f);
    glEndTransformFeedback();
    glUseProgram(0);
    ASSERT_GL_NO_ERROR();

    const GLvoid *mapPointer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(Vector3) * 2 * 6, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPointer);

    const auto &quadVertices = GetQuadVertices();

    const Vector3 *vecPointer = static_cast<const Vector3 *>(mapPointer);
    for (unsigned int vectorIndex = 0; vectorIndex < 3; ++vectorIndex)
    {
        unsigned int stream1Index = vectorIndex * 2;
        unsigned int stream2Index = vectorIndex * 2 + 1;
        EXPECT_EQ(quadVertices[vectorIndex], vecPointer[stream1Index]);
        EXPECT_EQ(quadVertices[vectorIndex], vecPointer[stream2Index]);
    }
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    ASSERT_GL_NO_ERROR();
}

// Test transform feedback varying for base-level members of struct.
TEST_P(TransformFeedbackTestES31, StructMemberVaryings)
{
    // Remove this when http://anglebug.com/4140 is fixed.
    ANGLE_SKIP_TEST_IF(IsVulkan());

    constexpr char kVS[] = R"(#version 310 es
in vec3 position;
struct S {
    vec3 field0;
    vec3 field1;
    vec3 field2;
};
out S s;

void main() {
    s.field0 = position;
    s.field1 = vec3(0, 0, 0);
    s.field2 = position;
    gl_Position = vec4(position, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
struct S {
    vec3 field0;
    vec3 field1;
    vec3 field2;
};
out vec4 color;
in S s;

void main() {
    color = vec4(s.field1, 1);
})";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("s.field0");
    tfVaryings.push_back("s.field2");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, mProgram);

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(Vector3) * 2 * 6, nullptr, GL_STREAM_DRAW);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(mProgram, "position", 0.5f);
    glEndTransformFeedback();
    glUseProgram(0);
    ASSERT_GL_NO_ERROR();

    const GLvoid *mapPointer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(Vector3) * 2 * 6, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPointer);

    const auto &quadVertices = GetQuadVertices();

    const Vector3 *vecPointer = static_cast<const Vector3 *>(mapPointer);
    for (unsigned int vectorIndex = 0; vectorIndex < 3; ++vectorIndex)
    {
        unsigned int stream1Index = vectorIndex * 2;
        unsigned int stream2Index = vectorIndex * 2 + 1;
        EXPECT_EQ(quadVertices[vectorIndex], vecPointer[stream1Index]);
        EXPECT_EQ(quadVertices[vectorIndex], vecPointer[stream2Index]);
    }
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    ASSERT_GL_NO_ERROR();
}

// Test transform feedback varying for struct is not allowed.
TEST_P(TransformFeedbackTestES31, InvalidStructVaryings)
{
    constexpr char kVS[] = R"(#version 310 es
in vec3 position;
struct S {
    vec3 field0;
    vec3 field1;
};
out S s;

void main() {
    s.field0 = position;
    s.field1 = vec3(0, 0, 0);
    gl_Position = vec4(position, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
struct S {
    vec3 field0;
    vec3 field1;
};
out vec4 color;
in S s;

void main() {
    color = vec4(s.field1, 1);
})";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("s");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_EQ(0u, mProgram);
}

// Test transform feedback can capture the whole array
TEST_P(TransformFeedbackTestES31, CaptureArray)
{
    constexpr char kVS[] = R"(#version 310 es
        in vec4 a_position;
        in float a_varA;
        in float a_varB1;
        in float a_varB2;
        out float v_varA[1];
        out float v_varB[2];
        void main()
        {
            gl_Position = a_position;
            gl_PointSize = 1.0;
            v_varA[0] = a_varA;
            v_varB[0] = a_varB1;
            v_varB[1] = a_varB2;
        })";

    constexpr char kFS[] = R"(#version 310 es
        precision mediump float;
        in float v_varA[1];
        in float v_varB[2];
        out vec4 fragColor;
        void main()
        {
            vec4 res = vec4(0.0);
            res += vec4(v_varA[0]);
            res += vec4(v_varB[0]);
            res += vec4(v_varB[1]);
            fragColor = res;
        })";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("v_varA");
    tfVaryings.push_back("v_varB");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, mProgram);

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float) * 3 * 6, nullptr, GL_STREAM_DRAW);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    GLint varA = glGetAttribLocation(mProgram, "a_varA");
    ASSERT_NE(-1, varA);
    GLint varB1 = glGetAttribLocation(mProgram, "a_varB1");
    ASSERT_NE(-1, varB1);
    GLint varB2 = glGetAttribLocation(mProgram, "a_varB2");
    ASSERT_NE(-1, varB2);

    std::array<float, 6> data1 = {24.0f, 25.0f, 30.0f, 33.0f, 37.5f, 44.0f};
    std::array<float, 6> data2 = {48.0f, 5.0f, 55.0f, 3.1415f, 87.0f, 42.0f};
    std::array<float, 6> data3 = {128.0f, 1.0f, 0.0f, -1.0f, 16.0f, 1024.0f};

    glVertexAttribPointer(varA, 1, GL_FLOAT, GL_FALSE, 0, data1.data());
    glEnableVertexAttribArray(varA);
    glVertexAttribPointer(varB1, 1, GL_FLOAT, GL_FALSE, 0, data2.data());
    glEnableVertexAttribArray(varB1);
    glVertexAttribPointer(varB2, 1, GL_FLOAT, GL_FALSE, 0, data3.data());
    glEnableVertexAttribArray(varB2);

    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(mProgram, "a_position", 0.5f);
    glEndTransformFeedback();
    glUseProgram(0);
    ASSERT_GL_NO_ERROR();

    void *mappedBuffer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(float) * 3 * 6, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mappedBuffer);

    float *mappedFloats = static_cast<float *>(mappedBuffer);
    for (int i = 0; i < 6; i++)
    {
        std::array<float, 3> mappedData = {mappedFloats[i * 3], mappedFloats[i * 3 + 1],
                                           mappedFloats[i * 3 + 2]};
        std::array<float, 3> data       = {data1[i], data2[i], data3[i]};
        EXPECT_EQ(data, mappedData) << "iteration #" << i;
    }

    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    ASSERT_GL_NO_ERROR();
}

// Test that nonexistent transform feedback varyings don't assert when linking.
TEST_P(TransformFeedbackTest, NonExistentTransformFeedbackVarying)
{
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("bogus");

    mProgram = CompileProgramWithTransformFeedback(
        essl3_shaders::vs::Simple(), essl3_shaders::fs::Red(), tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_EQ(0u, mProgram);
}

// Test that nonexistent transform feedback varyings don't assert when linking. In this test the
// nonexistent varying is prefixed with "gl_".
TEST_P(TransformFeedbackTest, NonExistentTransformFeedbackVaryingWithGLPrefix)
{
    // TODO(anglebug.com/5360): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsOSX() && IsARM64() && IsDesktopOpenGL());

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Bogus");

    mProgram = CompileProgramWithTransformFeedback(
        essl3_shaders::vs::Simple(), essl3_shaders::fs::Red(), tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_EQ(0u, mProgram);
}

// Test transform feedback names can be reserved names in GLSL, as long as they're not reserved in
// GLSL ES.
TEST_P(TransformFeedbackTest, VaryingReservedOpenGLName)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec3 position;\n"
        "out vec3 buffer;\n"
        "void main() {\n"
        "  buffer = position;\n"
        "  gl_Position = vec4(position, 1);\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 color;\n"
        "in vec3 buffer;\n"
        "void main() {\n"
        "  color = vec4(0);\n"
        "}";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("buffer");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, mProgram);

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(Vector3) * 6, nullptr, GL_STREAM_DRAW);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(mProgram, "position", 0.5f);
    glEndTransformFeedback();
    glUseProgram(0);
    ASSERT_GL_NO_ERROR();

    const GLvoid *mapPointer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(Vector3) * 6, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPointer);

    const auto &quadVertices = GetQuadVertices();

    const Vector3 *vecPointer = static_cast<const Vector3 *>(mapPointer);
    for (unsigned int vectorIndex = 0; vectorIndex < 3; ++vectorIndex)
    {
        EXPECT_EQ(quadVertices[vectorIndex], vecPointer[vectorIndex]);
    }
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    ASSERT_GL_NO_ERROR();
}

// Test that calling BeginTransformFeedback when no program is currentwill generate an
// INVALID_OPERATION error.
TEST_P(TransformFeedbackTest, NoCurrentProgram)
{
    glUseProgram(0);
    glBeginTransformFeedback(GL_TRIANGLES);

    // GLES 3.0.5 section 2.15.2: "The error INVALID_OPERATION is also generated by
    // BeginTransformFeedback if no binding points would be used, either because no program object
    // is active or because the active program object has specified no output variables to record."
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that calling BeginTransformFeedback when no transform feedback varyings are in use will
// generate an INVALID_OPERATION error.
TEST_P(TransformFeedbackTest, NoTransformFeedbackVaryingsInUse)
{
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

    glUseProgram(program);
    glBeginTransformFeedback(GL_TRIANGLES);

    // GLES 3.0.5 section 2.15.2: "The error INVALID_OPERATION is also generated by
    // BeginTransformFeedback if no binding points would be used, either because no program object
    // is active or because the active program object has specified no output variables to record."

    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that you can pause transform feedback without drawing first.
TEST_P(TransformFeedbackTest, SwitchProgramBeforeDraw)
{
    // TODO(anglebug.com/5360): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsOSX() && IsARM64() && IsDesktopOpenGL());

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ANGLE_GL_PROGRAM(nonTFProgram, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

    // Set up transform feedback, but pause it.
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    glPauseTransformFeedback();

    // Switch programs and draw while transform feedback is paused.
    glUseProgram(nonTFProgram);
    GLint positionLocation = glGetAttribLocation(nonTFProgram, essl1_shaders::PositionAttrib());
    glDisableVertexAttribArray(positionLocation);
    glVertexAttrib4f(positionLocation, 0.0f, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glEndTransformFeedback();

    ASSERT_GL_NO_ERROR();
}

// Test that ending transform feedback with a different program bound does not cause internal
// errors.
TEST_P(TransformFeedbackTest, EndWithDifferentProgram)
{
    // AMD drivers fail because they perform transform feedback when it should be paused.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ANGLE_GL_PROGRAM(nonTFProgram, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

    // Set up transform feedback, but pause it.
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    // Make sure the buffer has zero'd data
    std::vector<float> data(mTransformFeedbackBufferSize / sizeof(float), 0.0f);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBufferSize, data.data(),
                 GL_STATIC_DRAW);
    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    glPauseTransformFeedback();
    // Transform feedback should not happen
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    // Draw using a different program.
    glUseProgram(nonTFProgram);
    GLint positionLocation = glGetAttribLocation(nonTFProgram, essl1_shaders::PositionAttrib());
    glDisableVertexAttribArray(positionLocation);
    glVertexAttrib4f(positionLocation, 0.0f, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // End transform feedback without unpausing and with a different program bound. This triggers
    // the bug.
    glEndTransformFeedback();

    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    // On a buggy driver without the workaround this will cause a GL error because the driver
    // thinks transform feedback is still paused, but rendering will still write to the transform
    // feedback buffers.
    glPauseTransformFeedback();
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    glEndTransformFeedback();

    // Make sure that transform feedback did not happen. We always paused transform feedback before
    // rendering, but a buggy driver will fail to pause.
    const void *mapPointer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(Vector4) * 4, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPointer);
    const Vector4 *vecPointer = static_cast<const Vector4 *>(mapPointer);
    ASSERT_EQ(vecPointer[0], Vector4(0, 0, 0, 0));
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
    ASSERT_GL_NO_ERROR();
}

// Test that switching contexts with paused transform feedback does not cause internal errors.
TEST_P(TransformFeedbackTest, EndWithDifferentProgramContextSwitch)
{
    // AMD drivers fail because they perform transform feedback when it should be paused.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    EGLWindow *window          = getEGLWindow();
    EGLDisplay display         = window->getDisplay();
    EGLConfig config           = window->getConfig();
    EGLSurface surface         = window->getSurface();
    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION_KHR,
        GetParam().majorVersion,
        EGL_CONTEXT_MINOR_VERSION_KHR,
        GetParam().minorVersion,
        EGL_NONE,
    };
    auto context1 = eglGetCurrentContext();
    auto context2 = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
    ASSERT_NE(context2, EGL_NO_CONTEXT);
    // Compile a program on the second context.
    eglMakeCurrent(display, surface, surface, context2);
    ANGLE_GL_PROGRAM(nonTFProgram, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    eglMakeCurrent(display, surface, surface, context1);

    // Set up transform feedback, but pause it.
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    // Make sure the buffer has zero'd data
    std::vector<float> data(mTransformFeedbackBufferSize / sizeof(float), 0.0f);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBufferSize, data.data(),
                 GL_STATIC_DRAW);
    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    glPauseTransformFeedback();
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    // Leave transform feedback active but paused while we switch to a second context and render
    // something.
    eglMakeCurrent(display, surface, surface, context2);
    glUseProgram(nonTFProgram);
    GLint positionLocation = glGetAttribLocation(nonTFProgram, essl1_shaders::PositionAttrib());
    glDisableVertexAttribArray(positionLocation);
    glVertexAttrib4f(positionLocation, 0.0f, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Switch back to the first context and end transform feedback. On a buggy driver, this will
    // cause the transform feedback object to enter an invalid "inactive, but paused" state unless
    // the workaround is applied.
    eglMakeCurrent(display, surface, surface, context1);
    glEndTransformFeedback();
    glBeginTransformFeedback(GL_TRIANGLES);
    // On a buggy driver without the workaround this will cause a GL error because the driver
    // thinks transform feedback is still paused, but rendering will still write to the transform
    // feedback buffers.
    glPauseTransformFeedback();
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    glEndTransformFeedback();

    // Make sure that transform feedback did not happen. We always paused transform feedback before
    // rendering, but a buggy driver will fail to pause.
    const void *mapPointer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(Vector4) * 4, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPointer);
    const Vector4 *vecPointer = static_cast<const Vector4 *>(mapPointer);
    ASSERT_EQ(vecPointer[0], Vector4(0, 0, 0, 0));
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
    eglDestroyContext(display, context2);
    ASSERT_GL_NO_ERROR();
}

// Test an out of memory event.
TEST_P(TransformFeedbackTest, BufferOutOfMemory)
{
    // The GL back-end throws an internal error that we can't deal with in this test.
    ANGLE_SKIP_TEST_IF(IsOpenGL());

    // TODO: http://anglebug.com/5345: fails consistently on Mac FYI GPU ASAN Release bot
    ANGLE_SKIP_TEST_IF(IsMetal() && (IsIntel() || IsAMD()));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    GLint positionLocation   = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());
    const GLfloat vertices[] = {-1.0f, -0.5f, 0.0f, 0.5f, 1.0f};

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    // Draw normally.
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 5);
    glEndTransformFeedback();
    ASSERT_GL_NO_ERROR();

    // Attempt to generate OOM and begin XFB.
    constexpr GLsizeiptr kLargeSize = std::numeric_limits<GLsizeiptr>::max();
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, kLargeSize, nullptr, GL_STATIC_DRAW);

    // It's not spec guaranteed to return OOM here.
    GLenum err = glGetError();
    EXPECT_TRUE(err == GL_NO_ERROR || err == GL_OUT_OF_MEMORY);

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 5);
    glEndTransformFeedback();
}

void TransformFeedbackTest::setupOverrunTest(const std::vector<GLfloat> &vertices)
{
    std::vector<uint8_t> zeroData(mTransformFeedbackBufferSize, 0);

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    glBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBufferSize, zeroData.data());

    // Draw a simple points XFB.
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);
    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());

    // First pass: draw 6 points to the XFB buffer
    glEnable(GL_RASTERIZER_DISCARD);

    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, vertices.data());
    glEnableVertexAttribArray(positionLocation);

    // Bind the buffer for transform feedback output and start transform feedback
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 6);
}

void VerifyVertexFloats(const GLfloat *mapPtrFloat,
                        const std::vector<GLfloat> &vertices,
                        size_t copies,
                        size_t numFloats)
{
    for (size_t floatIndex = 0; floatIndex < vertices.size() * copies; ++floatIndex)
    {
        size_t vertIndex = floatIndex % vertices.size();
        ASSERT_EQ(mapPtrFloat[floatIndex], vertices[vertIndex]) << "at float index " << floatIndex;
    }

    // The rest should be zero.
    for (size_t floatIndex = vertices.size() * copies; floatIndex < numFloats; ++floatIndex)
    {
        ASSERT_EQ(mapPtrFloat[floatIndex], 0) << "at float index " << floatIndex;
    }
}

// Tests that stopping XFB works as expected.
TEST_P(TransformFeedbackTest, Overrun)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    const std::vector<GLfloat> vertices = {
        -1.0f, 1.0f, 0.5f, 1.0f, -1.0f, -1.0f, 0.5f, 1.0f, 1.0f, -1.0f, 0.5f, 1.0f,
        -1.0f, 1.0f, 0.5f, 1.0f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f, 1.0f,  0.5f, 1.0f,
    };

    setupOverrunTest(vertices);

    glEndTransformFeedback();

    // Draw a second time without XFB.
    glDrawArrays(GL_POINTS, 0, 6);

    ASSERT_GL_NO_ERROR();

    // Verify only the first data was output.
    const void *mapPtr         = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
                                          mTransformFeedbackBufferSize, GL_MAP_READ_BIT);
    const GLfloat *mapPtrFloat = reinterpret_cast<const float *>(mapPtr);

    size_t numFloats = mTransformFeedbackBufferSize / sizeof(GLfloat);
    VerifyVertexFloats(mapPtrFloat, vertices, 1, numFloats);
}

// Similar to the overrun test but with Pause instead of End.
TEST_P(TransformFeedbackTest, OverrunWithPause)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // Fails on Mac Intel GL drivers. http://anglebug.com/4992
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsOSX());

    const std::vector<GLfloat> vertices = {
        -1.0f, 1.0f, 0.5f, 1.0f, -1.0f, -1.0f, 0.5f, 1.0f, 1.0f, -1.0f, 0.5f, 1.0f,
        -1.0f, 1.0f, 0.5f, 1.0f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f, 1.0f,  0.5f, 1.0f,
    };

    setupOverrunTest(vertices);

    glPauseTransformFeedback();

    // Draw a second time without XFB.
    glDrawArrays(GL_POINTS, 0, 6);

    glEndTransformFeedback();

    ASSERT_GL_NO_ERROR();

    // Verify only the first data was output.
    const void *mapPtr         = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
                                          mTransformFeedbackBufferSize, GL_MAP_READ_BIT);
    const GLfloat *mapPtrFloat = reinterpret_cast<const float *>(mapPtr);

    size_t numFloats = mTransformFeedbackBufferSize / sizeof(GLfloat);
    VerifyVertexFloats(mapPtrFloat, vertices, 1, numFloats);
}

// Similar to the overrun test but with Pause instead of End.
TEST_P(TransformFeedbackTest, OverrunWithPauseAndResume)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // Fails on Adreno Pixel 2 GL drivers. Not a supported configuration.
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsAdreno() && IsAndroid());

    // Fails on Windows Intel GL drivers. http://anglebug.com/4697
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsWindows());

    const std::vector<GLfloat> vertices = {
        -1.0f, 1.0f, 0.5f, 1.0f, -1.0f, -1.0f, 0.5f, 1.0f, 1.0f, -1.0f, 0.5f, 1.0f,
        -1.0f, 1.0f, 0.5f, 1.0f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f, 1.0f,  0.5f, 1.0f,
    };

    setupOverrunTest(vertices);

    glPauseTransformFeedback();

    // Draw a second time without XFB.
    glDrawArrays(GL_POINTS, 0, 6);

    // Draw a third time with XFB.
    glResumeTransformFeedback();
    glDrawArrays(GL_POINTS, 0, 6);

    glEndTransformFeedback();

    ASSERT_GL_NO_ERROR();

    // Verify only the first and third data was output.
    const void *mapPtr         = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
                                          mTransformFeedbackBufferSize, GL_MAP_READ_BIT);
    const GLfloat *mapPtrFloat = reinterpret_cast<const float *>(mapPtr);

    size_t numFloats = mTransformFeedbackBufferSize / sizeof(GLfloat);
    VerifyVertexFloats(mapPtrFloat, vertices, 2, numFloats);
}

// Similar to the overrun Pause/Resume test but with more than one Pause and Resume.
TEST_P(TransformFeedbackTest, OverrunWithMultiplePauseAndResume)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // Fails on Adreno Pixel 2 GL drivers. Not a supported configuration.
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsAdreno() && IsAndroid());

    // Fails on Windows Intel GL drivers. http://anglebug.com/4697
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsWindows());

    // Fails on Mac AMD GL drivers. http://anglebug.com/4775
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsAMD() && IsOSX());

    // Crashes on Mac Intel GL drivers. http://anglebug.com/4992
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsOSX());

    const std::vector<GLfloat> vertices = {
        -1.0f, 1.0f, 0.5f, 1.0f, -1.0f, -1.0f, 0.5f, 1.0f, 1.0f, -1.0f, 0.5f, 1.0f,
        -1.0f, 1.0f, 0.5f, 1.0f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f, 1.0f,  0.5f, 1.0f,
    };

    setupOverrunTest(vertices);

    for (int iteration = 0; iteration < 2; ++iteration)
    {
        // Draw without XFB.
        glPauseTransformFeedback();
        glDrawArrays(GL_POINTS, 0, 6);

        // Draw with XFB.
        glResumeTransformFeedback();
        glDrawArrays(GL_POINTS, 0, 6);
    }

    glEndTransformFeedback();

    ASSERT_GL_NO_ERROR();

    // Verify only the first and third data was output.
    const void *mapPtr         = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
                                          mTransformFeedbackBufferSize, GL_MAP_READ_BIT);
    const GLfloat *mapPtrFloat = reinterpret_cast<const float *>(mapPtr);

    size_t numFloats = mTransformFeedbackBufferSize / sizeof(GLfloat);
    VerifyVertexFloats(mapPtrFloat, vertices, 3, numFloats);
}

// Tests begin/draw/end/*bindBuffer*/begin/draw/end.
TEST_P(TransformFeedbackTest, EndThenBindNewBufferAndRestart)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);
    glEnableVertexAttribArray(positionLocation);

    GLBuffer secondBuffer;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, secondBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBufferSize, nullptr,
                 GL_STATIC_DRAW);

    std::vector<GLfloat> posData1 = {0.1f, 0.0f, 0.0f, 1.0f, 0.2f, 0.0f, 0.0f, 1.0f, 0.3f, 0.0f,
                                     0.0f, 1.0f, 0.4f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 0.0f, 1.0f};
    std::vector<GLfloat> posData2 = {0.6f, 0.0f, 0.0f, 1.0f, 0.7f, 0.0f, 0.0f, 1.0f, 0.8f, 0.0f,
                                     0.0f, 1.0f, 0.9f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    size_t posBytes = posData1.size() * sizeof(posData1[0]);
    ASSERT_EQ(posBytes, posData2.size() * sizeof(posData2[0]));

    GLBuffer posBuffer1;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer1);
    glBufferData(GL_ARRAY_BUFFER, posBytes, posData1.data(), GL_STATIC_DRAW);

    GLBuffer posBuffer2;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer2);
    glBufferData(GL_ARRAY_BUFFER, posBytes, posData2.data(), GL_STATIC_DRAW);

    // Draw a first time with first buffer.
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer1);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 5);
    glEndTransformFeedback();
    ASSERT_GL_NO_ERROR();

    // Bind second buffer and draw with new data.
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer2);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, secondBuffer);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 5);
    glEndTransformFeedback();
    ASSERT_GL_NO_ERROR();

    // Read back buffer datas.
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
    void *posMap1 = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBytes, GL_MAP_READ_BIT);
    ASSERT_NE(posMap1, nullptr);

    std::vector<GLfloat> actualData1(posData1.size());
    memcpy(actualData1.data(), posMap1, posBytes);

    EXPECT_EQ(posData1, actualData1);

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, secondBuffer);
    void *posMap2 = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBytes, GL_MAP_READ_BIT);
    ASSERT_NE(posMap2, nullptr);

    std::vector<GLfloat> actualData2(posData2.size());
    memcpy(actualData2.data(), posMap2, posBytes);

    EXPECT_EQ(posData2, actualData2);
}

// Draw without transform feedback, then with it.  In this test, there are no uniforms.  Regression
// test based on conformance2/transform_feedback/simultaneous_binding.html for the transform
// feedback emulation path in Vulkan that bundles default uniforms and transform feedback buffers
// in the same descriptor set.  A previous bug was that the first non-transform-feedback draw call
// didn't allocate this descriptor set as there were neither uniforms nor transform feedback to be
// updated.  A second bug was that the second draw call didn't attempt to update the transform
// feedback buffers, as they were not "dirty".
TEST_P(TransformFeedbackTest, DrawWithoutTransformFeedbackThenWith)
{
    // Fails on Mac Intel GL drivers. http://anglebug.com/4992
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsOSX());

    constexpr char kVS[] =
        R"(#version 300 es
in float in_value;
out float out_value;

void main() {
   out_value = in_value * 2.;
})";

    constexpr char kFS[] =
        R"(#version 300 es
precision mediump float;
out vec4 unused;
void main() {
  unused = vec4(0.5);
})";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("out_value");

    mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_SEPARATE_ATTRIBS);
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);

    GLBuffer vertexBuffer, indexBuffer, xfbBuffer;
    GLVertexArray vao;

    constexpr std::array<float, 4> kAttribInitData         = {1, 2, 3, 4};
    constexpr std::array<unsigned short, 4> kIndexInitData = {0, 1, 2, 3};
    constexpr std::array<float, 4> kXfbInitData            = {0, 0, 0, 0};

    // Initialize buffers.
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, kAttribInitData.size() * sizeof(kAttribInitData[0]),
                 kAttribInitData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, kIndexInitData.size() * sizeof(kIndexInitData[0]),
                 kIndexInitData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, xfbBuffer);
    glBufferData(GL_ARRAY_BUFFER, kXfbInitData.size() * sizeof(kXfbInitData[0]),
                 kXfbInitData.data(), GL_STATIC_DRAW);

    // This tests that having a transform feedback buffer bound in an unbound VAO
    // does not affect anything.
    GLVertexArray unboundVao;
    glBindVertexArray(unboundVao);
    glBindBuffer(GL_ARRAY_BUFFER, xfbBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, false, 0, nullptr);
    glBindVertexArray(0);

    // Create the real VAO used for the test.
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, false, 0, nullptr);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfbBuffer);

    // First, issue an indexed draw call without transform feedback.
    glDrawElements(GL_POINTS, 4, GL_UNSIGNED_SHORT, 0);

    // Then issue a draw call with transform feedback.
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 4);
    glEndTransformFeedback();

    // Verify transform feedback buffer.
    void *mappedBuffer = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
                                          kXfbInitData.size() * sizeof(float), GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mappedBuffer);

    float *xfbOutput = static_cast<float *>(mappedBuffer);
    for (size_t index = 0; index < kXfbInitData.size(); ++index)
    {
        EXPECT_EQ(xfbOutput[index], kAttribInitData[index] * 2);
    }
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Test that transform feedback with scissor test enabled works.
TEST_P(TransformFeedbackTest, RecordAndDrawWithScissorTest)
{
    // http://crbug.com/1135841
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOSX());

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glScissor(0, 0, getWindowWidth() / 2 + 1, getWindowHeight() / 2 + 1);
    glEnable(GL_SCISSOR_TEST);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());

    // First pass: draw 6 points to the XFB buffer
    glEnable(GL_RASTERIZER_DISCARD);

    const GLfloat vertices[] = {
        -1.0f, 1.0f, 0.5f, -1.0f, -1.0f, 0.5f, 1.0f, -1.0f, 0.5f,

        -1.0f, 1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f,  0.5f,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    // Bind the buffer for transform feedback output and start transform feedback
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glBeginTransformFeedback(GL_POINTS);

    // Create a query to check how many primitives were written
    GLQuery primitivesWrittenQuery;
    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitivesWrittenQuery);

    glDrawArrays(GL_POINTS, 0, 3);
    glDrawArrays(GL_POINTS, 3, 3);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    // End the query and transform feedback
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glEndTransformFeedback();

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    // Check how many primitives were written and verify that some were written even if
    // no pixels were rendered
    GLuint primitivesWritten = 0;
    glGetQueryObjectuiv(primitivesWrittenQuery, GL_QUERY_RESULT_EXT, &primitivesWritten);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(6u, primitivesWritten);

    // Second pass: draw from the feedback buffer

    glBindBuffer(GL_ARRAY_BUFFER, mTransformFeedbackBuffer);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2 + 1, getWindowHeight() / 2 + 1, 0, 0, 0, 255);
    EXPECT_GL_NO_ERROR();
}

// Test XFB with depth write enabled.
class TransformFeedbackWithDepthBufferTest : public TransformFeedbackTest
{
  public:
    TransformFeedbackWithDepthBufferTest() : TransformFeedbackTest() { setConfigDepthBits(24); }
};

TEST_P(TransformFeedbackWithDepthBufferTest, RecordAndDrawWithDepthWriteEnabled)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());

    // First pass: draw 6 points to the XFB buffer
    glEnable(GL_RASTERIZER_DISCARD);

    const GLfloat vertices[] = {
        -1.0f, 1.0f, 0.5f, -1.0f, -1.0f, 0.5f, 1.0f, -1.0f, 0.5f,

        -1.0f, 1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f,  0.5f,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    // Bind the buffer for transform feedback output and start transform feedback
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glBeginTransformFeedback(GL_POINTS);

    // Create a query to check how many primitives were written
    GLQuery primitivesWrittenQuery;
    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitivesWrittenQuery);

    glDrawArrays(GL_POINTS, 0, 3);
    glDrawArrays(GL_POINTS, 3, 3);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    // End the query and transform feedback
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glEndTransformFeedback();

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    // Check how many primitives were written and verify that some were written even if
    // no pixels were rendered
    GLuint primitivesWritten = 0;
    glGetQueryObjectuiv(primitivesWrittenQuery, GL_QUERY_RESULT_EXT, &primitivesWritten);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(6u, primitivesWritten);

    // Second pass: draw from the feedback buffer

    glBindBuffer(GL_ARRAY_BUFFER, mTransformFeedbackBuffer);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);
    EXPECT_GL_NO_ERROR();
}

class TransformFeedbackTestES32 : public TransformFeedbackTest
{};

// Test that simultaneous use of transform feedback primitives written and primitives generated
// queries works.
TEST_P(TransformFeedbackTestES32, PrimitivesWrittenAndGenerated)
{
    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // No ES3.2 support on out bots.  http://anglebug.com/5435
    ANGLE_SKIP_TEST_IF(IsPixel2() && IsVulkan());

    // No VK_EXT_transform_feedback support on the following configurations.
    // http://anglebug.com/5435
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsAMD() && IsWindows());
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsNVIDIA() && IsWindows7());

    // http://anglebug.com/5539
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsLinux());

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    compileDefaultProgram(tfVaryings, GL_INTERLEAVED_ATTRIBS);

    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());

    glEnable(GL_RASTERIZER_DISCARD);

    const GLfloat vertices[] = {
        -1.0f, 1.0f,  0.5f, -1.0f, -1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, -1.0f, 1.0f,  0.5f,
        1.0f,  -1.0f, 0.5f, 1.0f,  1.0f,  0.5f, -1.0f, 1.0f,  0.5f, -1.0f, -1.0f, 0.5f,
        1.0f,  -1.0f, 0.5f, -1.0f, 1.0f,  0.5f, 1.0f,  -1.0f, 0.5f, 1.0f,  1.0f,  0.5f,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    // Bind the buffer for transform feedback output and start transform feedback
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);
    glBeginTransformFeedback(GL_POINTS);
    EXPECT_GL_NO_ERROR();

    // Create a number of queries.  The test overview is as follows (PW = PrimitivesWritten, PG =
    // Primitives Generated):
    //
    //           PW0 begin
    // - Draw 3
    //                      PG0 begin
    // - Draw 4
    //           PW0 end
    // - Draw 5
    // - Copy
    // - Draw 6
    //                                 PW1 begin
    // - Draw 7
    // - Copy
    // - Draw 8
    //                      PG0 end
    //                                            PG1 begin
    // - Draw 9
    // - Copy
    //                                 PW1 end
    // - Draw 10
    // - Copy
    //                                            PG1 end
    //                                                        PW2 begin
    //                                                                   PG2 begin
    // - Draw 11
    // - Copy
    // - Draw 12
    //                                                                   PG2 end
    //                                                        PW2 end
    //
    // This tests a variety of scenarios where either of PW or PG is active or not when the other
    // begins or ends, as well as testing render pass restarts with the queries active and begin and
    // end of queries outside or mid render pass.
    constexpr size_t kQueryCount = 3;
    GLQuery primitivesWrittenQueries[kQueryCount];
    GLQuery primitivesGeneratedQueries[kQueryCount];

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    /* PG PW */
    /*     / */ glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitivesWrittenQueries[0]);
    /*    |  */ glDrawArrays(GL_POINTS, 0, 3);
    /*  / 0  */ glBeginQuery(GL_PRIMITIVES_GENERATED, primitivesGeneratedQueries[0]);
    /* |  |  */ glDrawArrays(GL_POINTS, 0, 4);
    /* |   \ */ glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    /* |     */ glDrawArrays(GL_POINTS, 0, 5);
    /* |     */ glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
    /* 0     */ glDrawArrays(GL_POINTS, 0, 6);
    /* |   / */ glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitivesWrittenQueries[1]);
    /* |  |  */ glDrawArrays(GL_POINTS, 0, 7);
    /* |  |  */ glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
    /* |  |  */ glDrawArrays(GL_POINTS, 0, 8);
    /*  \ 1  */ glEndQuery(GL_PRIMITIVES_GENERATED);
    /*  / |  */ glBeginQuery(GL_PRIMITIVES_GENERATED, primitivesGeneratedQueries[1]);
    /* |  |  */ glDrawArrays(GL_POINTS, 0, 9);
    /* |  |  */ glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
    /* 1   \ */ glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    /* |     */ glDrawArrays(GL_POINTS, 0, 10);
    /* |     */ glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
    /*  \    */ glEndQuery(GL_PRIMITIVES_GENERATED);
    /*     / */ glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, primitivesWrittenQueries[2]);
    /*  / |  */ glBeginQuery(GL_PRIMITIVES_GENERATED, primitivesGeneratedQueries[2]);
    /* |  |  */ glDrawArrays(GL_POINTS, 0, 11);
    /* 2  2  */ glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
    /* |  |  */ glDrawArrays(GL_POINTS, 0, 12);
    /*  \ |  */ glEndQuery(GL_PRIMITIVES_GENERATED);
    /*     \ */ glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glEndTransformFeedback();
    EXPECT_GL_NO_ERROR();

    // Check that the queries have correct results.  Verify the first of each query with
    // GL_QUERY_RESULT_AVAILABLE for no particular reason other than testing different paths.
    GLuint readyPW = GL_FALSE;
    GLuint readyPG = GL_FALSE;
    while (readyPW == GL_FALSE || readyPG == GL_FALSE)
    {
        glGetQueryObjectuiv(primitivesWrittenQueries[0], GL_QUERY_RESULT_AVAILABLE, &readyPW);
        glGetQueryObjectuiv(primitivesGeneratedQueries[0], GL_QUERY_RESULT_AVAILABLE, &readyPG);
    }
    EXPECT_GL_NO_ERROR();

    constexpr GLuint kPrimitivesWrittenExpected[kQueryCount] = {
        3 + 4,
        7 + 8 + 9,
        11 + 12,
    };
    constexpr GLuint kPrimitivesGeneratedExpected[kQueryCount] = {
        4 + 5 + 6 + 7 + 8,
        9 + 10,
        11 + 12,
    };

    for (size_t queryIndex = 0; queryIndex < kQueryCount; ++queryIndex)
    {
        GLuint primitivesWritten = 0;
        glGetQueryObjectuiv(primitivesWrittenQueries[queryIndex], GL_QUERY_RESULT,
                            &primitivesWritten);

        GLuint primitivesGenerated = 0;
        glGetQueryObjectuiv(primitivesGeneratedQueries[queryIndex], GL_QUERY_RESULT,
                            &primitivesGenerated);
        EXPECT_GL_NO_ERROR();

        EXPECT_EQ(primitivesWritten, kPrimitivesWrittenExpected[queryIndex]) << queryIndex;
        EXPECT_EQ(primitivesGenerated, kPrimitivesGeneratedExpected[queryIndex]) << queryIndex;
    }
}

// Verify that capture of I/O block fields works, both when the instance name is specified and when
// not.  This test uses interleaved components.
TEST_P(TransformFeedbackTestES31, IOBlocksInterleaved)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    // http://anglebug.com/5488
    ANGLE_SKIP_TEST_IF(IsQualcomm() && IsOpenGLES());
    // http://anglebug.com/5493
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsVulkan());

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require

out VSBlock1
{
    vec4 a;
    vec4 b[2];
} blockOut1;

out VSBlock2
{
    vec4 c;
    mat3 d;
    vec4 e;
};

out vec4 looseVarying;

void main()
{
    blockOut1.a = vec4(0.15, 0.18, 0.21, 0.24);
    blockOut1.b[0] = vec4(0.27, 0.30, 0.33, 0.36);
    blockOut1.b[1] = vec4(0.39, 0.42, 0.45, 0.48);
    c = vec4(0.51, 0.54, 0.57, 0.6);
    d = mat3(vec3(0.63, 0.66, 0.69), vec3(0.72, 0.75, 0.78), vec3(0.81, 0.84, 0.87));
    e = vec4(0.9, 0.93, 0.96, 0.99);
    looseVarying = vec4(0.25, 0.5, 0.75, 1.0);
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;

layout(location = 0) out mediump vec4 color;

in VSBlock2
{
    vec4 c;
    mat3 d;
    vec4 e;
};

void main()
{
    color = vec4(c.x, d[0].y, e.z, 1.0);
})";

    std::vector<std::string> tfVaryings     = {"VSBlock1.b", "d", "looseVarying"};
    constexpr size_t kCapturedVaryingsCount = 3;
    constexpr std::array<size_t, kCapturedVaryingsCount> kCaptureSizes = {8, 9, 4};
    const std::vector<float> kExpected[kCapturedVaryingsCount]         = {
        {0.27, 0.30, 0.33, 0.36, 0.39, 0.42, 0.45, 0.48},
        {0.63, 0.66, 0.69, 0.72, 0.75, 0.78, 0.81, 0.84, 0.87},
        {0.25, 0.5, 0.75, 1.0},
    };

    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(program, kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    EXPECT_GL_NO_ERROR();

    GLTransformFeedback xfb;
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, xfb);

    GLBuffer xfbBuffer;

    size_t totalSize = 0;
    for (size_t index = 0; index < kCapturedVaryingsCount; ++index)
    {
        totalSize += kCaptureSizes[index];
    }

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfbBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, totalSize * sizeof(float), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfbBuffer);

    glUseProgram(program);

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 1);
    glEndTransformFeedback();

    const float *bufferData = static_cast<float *>(glMapBufferRange(
        GL_TRANSFORM_FEEDBACK_BUFFER, 0, totalSize * sizeof(float), GL_MAP_READ_BIT));

    size_t currentOffset = 0;
    for (size_t index = 0; index < kCapturedVaryingsCount; ++index)
    {
        for (size_t component = 0; component < kCaptureSizes[index]; ++component)
        {
            EXPECT_NEAR(bufferData[currentOffset + component], kExpected[index][component], 0.001f)
                << index << " " << component;
        }
        currentOffset += kCaptureSizes[index];
    }

    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
}

// Verify that capture of I/O block fields works.  This test uses separate components.
TEST_P(TransformFeedbackTestES31, IOBlocksSeparate)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    // http://anglebug.com/5487
    ANGLE_SKIP_TEST_IF(IsLinux() && (IsIntel() || IsAMD()) && IsOpenGL());

    // http://anglebug.com/5488
    ANGLE_SKIP_TEST_IF(IsQualcomm() && IsOpenGLES());

    // http://anglebug.com/5493
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsVulkan());

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require

out VSBlock
{
    float a;
    vec2 b;
};

out float c;

void main()
{
    a = 0.25;
    b = vec2(0.5, 0.75);
    c = 1.0;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;

layout(location = 0) out mediump vec4 color;

in VSBlock
{
    float a;
    vec2 b;
};

void main()
{
    color = vec4(a, b, 1.0);
})";

    std::vector<std::string> tfVaryings                                = {"a", "b", "c"};
    constexpr size_t kCapturedVaryingsCount                            = 3;
    constexpr std::array<size_t, kCapturedVaryingsCount> kCaptureSizes = {1, 2, 1};
    const std::vector<float> kExpected[kCapturedVaryingsCount]         = {
        {0.25},
        {0.5, 0.75},
        {1.0},
    };

    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(program, kVS, kFS, tfVaryings, GL_SEPARATE_ATTRIBS);
    EXPECT_GL_NO_ERROR();

    GLTransformFeedback xfb;
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, xfb);

    std::array<GLBuffer, kCapturedVaryingsCount> xfbBuffers;

    for (size_t index = 0; index < kCapturedVaryingsCount; ++index)
    {
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfbBuffers[index]);
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, kCaptureSizes[index] * sizeof(float), nullptr,
                     GL_STATIC_DRAW);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, index, xfbBuffers[index]);
    }

    glUseProgram(program);

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 1);
    glEndTransformFeedback();

    for (size_t index = 0; index < kCapturedVaryingsCount; ++index)
    {
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfbBuffers[index]);

        const float *bufferData = static_cast<float *>(
            glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, kCaptureSizes[index] * sizeof(float),
                             GL_MAP_READ_BIT));

        for (size_t component = 0; component < kCaptureSizes[index]; ++component)
        {
            EXPECT_NEAR(bufferData[component], kExpected[index][component], 0.001f)
                << index << " " << component;
        }

        glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
    }
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES3(TransformFeedbackTest);
ANGLE_INSTANTIATE_TEST_ES3(TransformFeedbackLifetimeTest);
ANGLE_INSTANTIATE_TEST_ES31(TransformFeedbackTestES31);
ANGLE_INSTANTIATE_TEST_ES32(TransformFeedbackTestES32);

ANGLE_INSTANTIATE_TEST(TransformFeedbackWithDepthBufferTest, ES3_METAL());

// These test suites are not instantiated on some OSes.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TransformFeedbackTestES32);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TransformFeedbackWithDepthBufferTest);

}  // anonymous namespace