#pragma once
#include "PlatformDefines.h"

#if PLATFORM_NX
  #include <nn/util/util_MathTypes.h>
  #include <nn/util/util_Vector.h>
  #include <nn/util/util_VectorApi.h>
  #include <nn/util/util_Color.h>
  using namespace nn::util;

#else // PLATFORM_WIN64
  #include <cstring>
  #include <cmath>

  struct Float2 { float x, y; };
  struct Float3 { float x, y, z; };
  struct Float4 { float x, y, z, w; };
  struct Float4x4 { float m[4][4]; };

  inline Float2 MakeFloat2(float x, float y) {
    Float2 v; v.x = x; v.y = y; return v;
  }
  inline Float3 MakeFloat3(float x, float y, float z) {
    Float3 v; v.x = x; v.y = y; v.z = z; return v;
  }
  inline Float4 MakeFloat4(float x, float y, float z, float w) {
    Float4 v; v.x = x; v.y = y; v.z = z; v.w = w; return v;
  }

  struct Matrix4x4f {
    float m[4][4];

    // Orthographic projection (right-handed, same as nn::util)
    // Maps [0, width] x [0, |height|] to NDC [-1,1]
    // height < 0 means Y-down (screen coordinates)
    static Matrix4x4f OrthographicRightHanded(float width, float height, float nearZ, float farZ) {
      Matrix4x4f mat;
      memset(&mat, 0, sizeof(mat));
      mat.m[0][0] = 2.0f / width;
      mat.m[1][1] = 2.0f / height;
      mat.m[2][2] = -1.0f / (farZ - nearZ);
      mat.m[3][0] = -1.0f;
      mat.m[3][1] = (height < 0) ? 1.0f : -1.0f;
      mat.m[3][2] = -nearZ / (farZ - nearZ);
      mat.m[3][3] = 1.0f;
      return mat;
    }
  };

  inline void MatrixStore(Float4x4* dst, const Matrix4x4f& src) {
    memcpy(dst, &src, sizeof(Float4x4));
  }

#endif
