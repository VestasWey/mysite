// GENERATED FILE - DO NOT EDIT.
// Generated by gen_uniform_type_table.py.
//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Uniform type info table:
//   Metadata about a particular uniform format, indexed by GL type.

#include <array>
#include "common/utilities.h"

using namespace angle;

namespace gl
{

namespace
{
constexpr std::array<UniformTypeInfo, 77> kInfoTable = {
    {{GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum, 0, 0, 0, 0, 0 * 0,
      0 * 0, false, false, false, ""},
     {GL_BOOL, GL_BOOL, GL_NONE, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, false, ""},
     {GL_BOOL_VEC2, GL_BOOL, GL_NONE, GL_NONE, GL_BOOL_VEC2, SamplerFormat::InvalidEnum, 1, 2, 2,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 2, false, false, false, ""},
     {GL_BOOL_VEC3, GL_BOOL, GL_NONE, GL_NONE, GL_BOOL_VEC3, SamplerFormat::InvalidEnum, 1, 3, 3,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 3, false, false, false, ""},
     {GL_BOOL_VEC4, GL_BOOL, GL_NONE, GL_NONE, GL_BOOL_VEC4, SamplerFormat::InvalidEnum, 1, 4, 4,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 4, false, false, false, ""},
     {GL_FLOAT, GL_FLOAT, GL_NONE, GL_NONE, GL_BOOL, SamplerFormat::InvalidEnum, 1, 1, 1,
      sizeof(GLfloat), sizeof(GLfloat) * 4, sizeof(GLfloat) * 1, false, false, false, ""},
     {GL_FLOAT_MAT2, GL_FLOAT, GL_NONE, GL_FLOAT_MAT2, GL_NONE, SamplerFormat::InvalidEnum, 2, 2, 4,
      sizeof(GLfloat), sizeof(GLfloat) * 8, sizeof(GLfloat) * 4, false, true, false, ""},
     {GL_FLOAT_MAT2x3, GL_FLOAT, GL_NONE, GL_FLOAT_MAT3x2, GL_NONE, SamplerFormat::InvalidEnum, 3,
      2, 6, sizeof(GLfloat), sizeof(GLfloat) * 12, sizeof(GLfloat) * 6, false, true, false, ""},
     {GL_FLOAT_MAT2x4, GL_FLOAT, GL_NONE, GL_FLOAT_MAT4x2, GL_NONE, SamplerFormat::InvalidEnum, 4,
      2, 8, sizeof(GLfloat), sizeof(GLfloat) * 16, sizeof(GLfloat) * 8, false, true, false, ""},
     {GL_FLOAT_MAT3, GL_FLOAT, GL_NONE, GL_FLOAT_MAT3, GL_NONE, SamplerFormat::InvalidEnum, 3, 3, 9,
      sizeof(GLfloat), sizeof(GLfloat) * 12, sizeof(GLfloat) * 9, false, true, false, ""},
     {GL_FLOAT_MAT3x2, GL_FLOAT, GL_NONE, GL_FLOAT_MAT2x3, GL_NONE, SamplerFormat::InvalidEnum, 2,
      3, 6, sizeof(GLfloat), sizeof(GLfloat) * 8, sizeof(GLfloat) * 6, false, true, false, ""},
     {GL_FLOAT_MAT3x4, GL_FLOAT, GL_NONE, GL_FLOAT_MAT4x3, GL_NONE, SamplerFormat::InvalidEnum, 4,
      3, 12, sizeof(GLfloat), sizeof(GLfloat) * 16, sizeof(GLfloat) * 12, false, true, false, ""},
     {GL_FLOAT_MAT4, GL_FLOAT, GL_NONE, GL_FLOAT_MAT4, GL_NONE, SamplerFormat::InvalidEnum, 4, 4,
      16, sizeof(GLfloat), sizeof(GLfloat) * 16, sizeof(GLfloat) * 16, false, true, false, ""},
     {GL_FLOAT_MAT4x2, GL_FLOAT, GL_NONE, GL_FLOAT_MAT2x4, GL_NONE, SamplerFormat::InvalidEnum, 2,
      4, 8, sizeof(GLfloat), sizeof(GLfloat) * 8, sizeof(GLfloat) * 8, false, true, false, ""},
     {GL_FLOAT_MAT4x3, GL_FLOAT, GL_NONE, GL_FLOAT_MAT3x4, GL_NONE, SamplerFormat::InvalidEnum, 3,
      4, 12, sizeof(GLfloat), sizeof(GLfloat) * 12, sizeof(GLfloat) * 12, false, true, false, ""},
     {GL_FLOAT_VEC2, GL_FLOAT, GL_NONE, GL_NONE, GL_BOOL_VEC2, SamplerFormat::InvalidEnum, 1, 2, 2,
      sizeof(GLfloat), sizeof(GLfloat) * 4, sizeof(GLfloat) * 2, false, false, false, ""},
     {GL_FLOAT_VEC3, GL_FLOAT, GL_NONE, GL_NONE, GL_BOOL_VEC3, SamplerFormat::InvalidEnum, 1, 3, 3,
      sizeof(GLfloat), sizeof(GLfloat) * 4, sizeof(GLfloat) * 3, false, false, false, ""},
     {GL_FLOAT_VEC4, GL_FLOAT, GL_NONE, GL_NONE, GL_BOOL_VEC4, SamplerFormat::InvalidEnum, 1, 4, 4,
      sizeof(GLfloat), sizeof(GLfloat) * 4, sizeof(GLfloat) * 4, false, false, false, ""},
     {GL_IMAGE_2D, GL_INT, GL_TEXTURE_2D, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, true, "intBitsToFloat"},
     {GL_IMAGE_2D_ARRAY, GL_INT, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum,
      1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, true,
      "intBitsToFloat"},
     {GL_IMAGE_3D, GL_INT, GL_TEXTURE_3D, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, true, "intBitsToFloat"},
     {GL_IMAGE_CUBE, GL_INT, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum, 1,
      1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, true,
      "intBitsToFloat"},
     {GL_IMAGE_CUBE_MAP_ARRAY, GL_INT, GL_TEXTURE_CUBE_MAP_ARRAY, GL_NONE, GL_NONE,
      SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1,
      false, false, true, "intBitsToFloat"},
     {GL_IMAGE_BUFFER, GL_INT, GL_TEXTURE_BUFFER, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum, 1,
      1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, true,
      "intBitsToFloat"},
     {GL_INT, GL_INT, GL_NONE, GL_NONE, GL_BOOL, SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLint),
      sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, false, "intBitsToFloat"},
     {GL_INT_IMAGE_2D, GL_INT, GL_TEXTURE_2D, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, true, "intBitsToFloat"},
     {GL_INT_IMAGE_2D_ARRAY, GL_INT, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE,
      SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1,
      false, false, true, "intBitsToFloat"},
     {GL_INT_IMAGE_3D, GL_INT, GL_TEXTURE_3D, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, true, "intBitsToFloat"},
     {GL_INT_IMAGE_CUBE, GL_INT, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum,
      1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, true,
      "intBitsToFloat"},
     {GL_INT_IMAGE_CUBE_MAP_ARRAY, GL_INT, GL_TEXTURE_CUBE_MAP_ARRAY, GL_NONE, GL_NONE,
      SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1,
      false, false, true, "intBitsToFloat"},
     {GL_INT_IMAGE_BUFFER, GL_INT, GL_TEXTURE_BUFFER, GL_NONE, GL_NONE, SamplerFormat::InvalidEnum,
      1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, false, false, true,
      "intBitsToFloat"},
     {GL_INT_SAMPLER_2D, GL_INT, GL_TEXTURE_2D, GL_NONE, GL_NONE, SamplerFormat::Signed, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false, "intBitsToFloat"},
     {GL_INT_SAMPLER_2D_ARRAY, GL_INT, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE, SamplerFormat::Signed,
      1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false,
      "intBitsToFloat"},
     {GL_INT_SAMPLER_2D_MULTISAMPLE, GL_INT, GL_TEXTURE_2D_MULTISAMPLE, GL_NONE, GL_NONE,
      SamplerFormat::Signed, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true,
      false, false, "intBitsToFloat"},
     {GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_INT, GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_NONE,
      GL_NONE, SamplerFormat::Signed, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1,
      true, false, false, "intBitsToFloat"},
     {GL_INT_SAMPLER_3D, GL_INT, GL_TEXTURE_3D, GL_NONE, GL_NONE, SamplerFormat::Signed, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false, "intBitsToFloat"},
     {GL_INT_SAMPLER_CUBE, GL_INT, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_NONE, SamplerFormat::Signed, 1,
      1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false,
      "intBitsToFloat"},
     {GL_INT_SAMPLER_CUBE_MAP_ARRAY, GL_INT, GL_TEXTURE_CUBE_MAP_ARRAY, GL_NONE, GL_NONE,
      SamplerFormat::Signed, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true,
      false, false, "intBitsToFloat"},
     {GL_INT_SAMPLER_BUFFER, GL_INT, GL_TEXTURE_BUFFER, GL_NONE, GL_NONE, SamplerFormat::Signed, 1,
      1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false,
      "intBitsToFloat"},
     {GL_INT_VEC2, GL_INT, GL_NONE, GL_NONE, GL_BOOL_VEC2, SamplerFormat::InvalidEnum, 1, 2, 2,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 2, false, false, false, "intBitsToFloat"},
     {GL_INT_VEC3, GL_INT, GL_NONE, GL_NONE, GL_BOOL_VEC3, SamplerFormat::InvalidEnum, 1, 3, 3,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 3, false, false, false, "intBitsToFloat"},
     {GL_INT_VEC4, GL_INT, GL_NONE, GL_NONE, GL_BOOL_VEC4, SamplerFormat::InvalidEnum, 1, 4, 4,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 4, false, false, false, "intBitsToFloat"},
     {GL_SAMPLER_2D, GL_INT, GL_TEXTURE_2D, GL_NONE, GL_NONE, SamplerFormat::Float, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false, "intBitsToFloat"},
     {GL_SAMPLER_2D_ARRAY, GL_INT, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE, SamplerFormat::Float, 1,
      1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false,
      "intBitsToFloat"},
     {GL_SAMPLER_2D_ARRAY_SHADOW, GL_INT, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE,
      SamplerFormat::Shadow, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true,
      false, false, "intBitsToFloat"},
     {GL_SAMPLER_2D_MULTISAMPLE, GL_INT, GL_TEXTURE_2D_MULTISAMPLE, GL_NONE, GL_NONE,
      SamplerFormat::Float, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true,
      false, false, "intBitsToFloat"},
     {GL_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_INT, GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_NONE, GL_NONE,
      SamplerFormat::Float, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true,
      false, false, "intBitsToFloat"},
     {GL_SAMPLER_2D_RECT_ANGLE, GL_INT, GL_TEXTURE_2D, GL_NONE, GL_NONE, SamplerFormat::Float, 1, 1,
      1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false, "intBitsToFloat"},
     {GL_SAMPLER_2D_SHADOW, GL_INT, GL_TEXTURE_2D, GL_NONE, GL_NONE, SamplerFormat::Shadow, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false, "intBitsToFloat"},
     {GL_SAMPLER_3D, GL_INT, GL_TEXTURE_3D, GL_NONE, GL_NONE, SamplerFormat::Float, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false, "intBitsToFloat"},
     {GL_SAMPLER_CUBE, GL_INT, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_NONE, SamplerFormat::Float, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false, "intBitsToFloat"},
     {GL_SAMPLER_CUBE_MAP_ARRAY, GL_INT, GL_TEXTURE_CUBE_MAP_ARRAY, GL_NONE, GL_NONE,
      SamplerFormat::Float, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true,
      false, false, "intBitsToFloat"},
     {GL_SAMPLER_BUFFER, GL_INT, GL_TEXTURE_BUFFER, GL_NONE, GL_NONE, SamplerFormat::Float, 1, 1, 1,
      sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false, "intBitsToFloat"},
     {GL_SAMPLER_CUBE_SHADOW, GL_INT, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_NONE, SamplerFormat::Shadow,
      1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false,
      "intBitsToFloat"},
     {GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW, GL_INT, GL_NONE, GL_NONE, GL_NONE, SamplerFormat::Shadow, 1,
      1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false,
      "intBitsToFloat"},
     {GL_SAMPLER_EXTERNAL_OES, GL_INT, GL_TEXTURE_EXTERNAL_OES, GL_NONE, GL_NONE,
      SamplerFormat::Float, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true,
      false, false, "intBitsToFloat"},
     {GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_NONE, GL_NONE, GL_BOOL, SamplerFormat::InvalidEnum, 1, 1,
      1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1, false, false, false,
      "uintBitsToFloat"},
     {GL_UNSIGNED_INT_ATOMIC_COUNTER, GL_UNSIGNED_INT, GL_NONE, GL_NONE, GL_NONE,
      SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      false, false, false, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_IMAGE_2D, GL_UNSIGNED_INT, GL_TEXTURE_2D, GL_NONE, GL_NONE,
      SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      false, false, true, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_IMAGE_2D_ARRAY, GL_UNSIGNED_INT, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE,
      SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      false, false, true, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_IMAGE_3D, GL_UNSIGNED_INT, GL_TEXTURE_3D, GL_NONE, GL_NONE,
      SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      false, false, true, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_IMAGE_CUBE, GL_UNSIGNED_INT, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_NONE,
      SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      false, false, true, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY, GL_UNSIGNED_INT, GL_TEXTURE_CUBE_MAP_ARRAY, GL_NONE,
      GL_NONE, SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4,
      sizeof(GLuint) * 1, false, false, true, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_IMAGE_BUFFER, GL_UNSIGNED_INT, GL_TEXTURE_BUFFER, GL_NONE, GL_NONE,
      SamplerFormat::InvalidEnum, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      false, false, true, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_SAMPLER_2D, GL_UNSIGNED_INT, GL_TEXTURE_2D, GL_NONE, GL_NONE,
      SamplerFormat::Unsigned, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      true, false, false, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, GL_UNSIGNED_INT, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE,
      SamplerFormat::Unsigned, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      true, false, false, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, GL_UNSIGNED_INT, GL_TEXTURE_2D_MULTISAMPLE, GL_NONE,
      GL_NONE, SamplerFormat::Unsigned, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4,
      sizeof(GLuint) * 1, true, false, false, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_UNSIGNED_INT,
      GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_NONE, GL_NONE, SamplerFormat::Unsigned, 1, 1, 1,
      sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1, true, false, false,
      "uintBitsToFloat"},
     {GL_UNSIGNED_INT_SAMPLER_3D, GL_UNSIGNED_INT, GL_TEXTURE_3D, GL_NONE, GL_NONE,
      SamplerFormat::Unsigned, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      true, false, false, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_SAMPLER_CUBE, GL_UNSIGNED_INT, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_NONE,
      SamplerFormat::Unsigned, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      true, false, false, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY, GL_UNSIGNED_INT, GL_TEXTURE_CUBE_MAP_ARRAY, GL_NONE,
      GL_NONE, SamplerFormat::Unsigned, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4,
      sizeof(GLuint) * 1, true, false, false, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_SAMPLER_BUFFER, GL_UNSIGNED_INT, GL_TEXTURE_BUFFER, GL_NONE, GL_NONE,
      SamplerFormat::Unsigned, 1, 1, 1, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 1,
      true, false, false, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_VEC2, GL_UNSIGNED_INT, GL_NONE, GL_NONE, GL_BOOL_VEC2,
      SamplerFormat::InvalidEnum, 1, 2, 2, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 2,
      false, false, false, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_VEC3, GL_UNSIGNED_INT, GL_NONE, GL_NONE, GL_BOOL_VEC3,
      SamplerFormat::InvalidEnum, 1, 3, 3, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 3,
      false, false, false, "uintBitsToFloat"},
     {GL_UNSIGNED_INT_VEC4, GL_UNSIGNED_INT, GL_NONE, GL_NONE, GL_BOOL_VEC4,
      SamplerFormat::InvalidEnum, 1, 4, 4, sizeof(GLuint), sizeof(GLuint) * 4, sizeof(GLuint) * 4,
      false, false, false, "uintBitsToFloat"},
     {GL_SAMPLER_VIDEO_IMAGE_WEBGL, GL_INT, GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_NONE, GL_NONE,
      SamplerFormat::Float, 1, 1, 1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true,
      false, false, "intBitsToFloat"},
     {GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT, GL_INT, GL_NONE, GL_NONE, GL_NONE, SamplerFormat::Float, 1, 1,
      1, sizeof(GLint), sizeof(GLint) * 4, sizeof(GLint) * 1, true, false, false,
      "intBitsToFloat"}}};

size_t GetTypeInfoIndex(GLenum uniformType)
{
    switch (uniformType)
    {
        case GL_NONE:
            return 0;
        case GL_BOOL:
            return 1;
        case GL_BOOL_VEC2:
            return 2;
        case GL_BOOL_VEC3:
            return 3;
        case GL_BOOL_VEC4:
            return 4;
        case GL_FLOAT:
            return 5;
        case GL_FLOAT_MAT2:
            return 6;
        case GL_FLOAT_MAT2x3:
            return 7;
        case GL_FLOAT_MAT2x4:
            return 8;
        case GL_FLOAT_MAT3:
            return 9;
        case GL_FLOAT_MAT3x2:
            return 10;
        case GL_FLOAT_MAT3x4:
            return 11;
        case GL_FLOAT_MAT4:
            return 12;
        case GL_FLOAT_MAT4x2:
            return 13;
        case GL_FLOAT_MAT4x3:
            return 14;
        case GL_FLOAT_VEC2:
            return 15;
        case GL_FLOAT_VEC3:
            return 16;
        case GL_FLOAT_VEC4:
            return 17;
        case GL_IMAGE_2D:
            return 18;
        case GL_IMAGE_2D_ARRAY:
            return 19;
        case GL_IMAGE_3D:
            return 20;
        case GL_IMAGE_CUBE:
            return 21;
        case GL_IMAGE_CUBE_MAP_ARRAY:
            return 22;
        case GL_IMAGE_BUFFER:
            return 23;
        case GL_INT:
            return 24;
        case GL_INT_IMAGE_2D:
            return 25;
        case GL_INT_IMAGE_2D_ARRAY:
            return 26;
        case GL_INT_IMAGE_3D:
            return 27;
        case GL_INT_IMAGE_CUBE:
            return 28;
        case GL_INT_IMAGE_CUBE_MAP_ARRAY:
            return 29;
        case GL_INT_IMAGE_BUFFER:
            return 30;
        case GL_INT_SAMPLER_2D:
            return 31;
        case GL_INT_SAMPLER_2D_ARRAY:
            return 32;
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
            return 33;
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
            return 34;
        case GL_INT_SAMPLER_3D:
            return 35;
        case GL_INT_SAMPLER_CUBE:
            return 36;
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
            return 37;
        case GL_INT_SAMPLER_BUFFER:
            return 38;
        case GL_INT_VEC2:
            return 39;
        case GL_INT_VEC3:
            return 40;
        case GL_INT_VEC4:
            return 41;
        case GL_SAMPLER_2D:
            return 42;
        case GL_SAMPLER_2D_ARRAY:
            return 43;
        case GL_SAMPLER_2D_ARRAY_SHADOW:
            return 44;
        case GL_SAMPLER_2D_MULTISAMPLE:
            return 45;
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
            return 46;
        case GL_SAMPLER_2D_RECT_ANGLE:
            return 47;
        case GL_SAMPLER_2D_SHADOW:
            return 48;
        case GL_SAMPLER_3D:
            return 49;
        case GL_SAMPLER_CUBE:
            return 50;
        case GL_SAMPLER_CUBE_MAP_ARRAY:
            return 51;
        case GL_SAMPLER_BUFFER:
            return 52;
        case GL_SAMPLER_CUBE_SHADOW:
            return 53;
        case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
            return 54;
        case GL_SAMPLER_EXTERNAL_OES:
            return 55;
        case GL_UNSIGNED_INT:
            return 56;
        case GL_UNSIGNED_INT_ATOMIC_COUNTER:
            return 57;
        case GL_UNSIGNED_INT_IMAGE_2D:
            return 58;
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
            return 59;
        case GL_UNSIGNED_INT_IMAGE_3D:
            return 60;
        case GL_UNSIGNED_INT_IMAGE_CUBE:
            return 61;
        case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
            return 62;
        case GL_UNSIGNED_INT_IMAGE_BUFFER:
            return 63;
        case GL_UNSIGNED_INT_SAMPLER_2D:
            return 64;
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
            return 65;
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
            return 66;
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
            return 67;
        case GL_UNSIGNED_INT_SAMPLER_3D:
            return 68;
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
            return 69;
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
            return 70;
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
            return 71;
        case GL_UNSIGNED_INT_VEC2:
            return 72;
        case GL_UNSIGNED_INT_VEC3:
            return 73;
        case GL_UNSIGNED_INT_VEC4:
            return 74;
        case GL_SAMPLER_VIDEO_IMAGE_WEBGL:
            return 75;
        case GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT:
            return 76;
        default:
            UNREACHABLE();
            return 0;
    }
}
}  // anonymous namespace

const UniformTypeInfo &GetUniformTypeInfo(GLenum uniformType)
{
    ASSERT(kInfoTable[GetTypeInfoIndex(uniformType)].type == uniformType);
    return kInfoTable[GetTypeInfoIndex(uniformType)];
}

}  // namespace gl
