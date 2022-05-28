// GENERATED FILE - DO NOT EDIT.
// Generated by generate_entry_points.py using data from gl.xml.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// validationGL11_autogen.h:
//   Validation functions for the OpenGL 1.1 entry points.

#ifndef LIBANGLE_VALIDATION_GL11_AUTOGEN_H_
#define LIBANGLE_VALIDATION_GL11_AUTOGEN_H_

#include "common/PackedEnums.h"

namespace gl
{
class Context;

bool ValidateAreTexturesResident(const Context *context,
                                 GLsizei n,
                                 const GLuint *textures,
                                 const GLboolean *residences);
bool ValidateArrayElement(const Context *context, GLint i);
bool ValidateCopyTexImage1D(const Context *context,
                            GLenum target,
                            GLint level,
                            GLenum internalformat,
                            GLint x,
                            GLint y,
                            GLsizei width,
                            GLint border);
bool ValidateCopyTexSubImage1D(const Context *context,
                               GLenum target,
                               GLint level,
                               GLint xoffset,
                               GLint x,
                               GLint y,
                               GLsizei width);
bool ValidateEdgeFlagPointer(const Context *context, GLsizei stride, const void *pointer);
bool ValidateIndexPointer(const Context *context, GLenum type, GLsizei stride, const void *pointer);
bool ValidateIndexub(const Context *context, GLubyte c);
bool ValidateIndexubv(const Context *context, const GLubyte *c);
bool ValidateInterleavedArrays(const Context *context,
                               GLenum format,
                               GLsizei stride,
                               const void *pointer);
bool ValidatePopClientAttrib(const Context *context);
bool ValidatePrioritizeTextures(const Context *context,
                                GLsizei n,
                                const GLuint *textures,
                                const GLfloat *priorities);
bool ValidatePushClientAttrib(const Context *context, GLbitfield mask);
bool ValidateTexSubImage1D(const Context *context,
                           GLenum target,
                           GLint level,
                           GLint xoffset,
                           GLsizei width,
                           GLenum format,
                           GLenum type,
                           const void *pixels);
}  // namespace gl

#endif  // LIBANGLE_VALIDATION_GL11_AUTOGEN_H_
