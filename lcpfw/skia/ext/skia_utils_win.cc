// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skia/ext/skia_utils_win.h"

#include <stddef.h>
#include <windows.h>

#include "base/check_op.h"
#include "base/debug/gdi_debug_util_win.h"
#include "base/numerics/checked_math.h"
#include "base/win/scoped_hdc.h"
#include "skia/ext/legacy_display_globals.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/core/SkTypes.h"

namespace {

static_assert(offsetof(RECT, left) == offsetof(SkIRect, fLeft), "o1");
static_assert(offsetof(RECT, top) == offsetof(SkIRect, fTop), "o2");
static_assert(offsetof(RECT, right) == offsetof(SkIRect, fRight), "o3");
static_assert(offsetof(RECT, bottom) == offsetof(SkIRect, fBottom), "o4");
static_assert(sizeof(RECT().left) == sizeof(SkIRect().fLeft), "o5");
static_assert(sizeof(RECT().top) == sizeof(SkIRect().fTop), "o6");
static_assert(sizeof(RECT().right) == sizeof(SkIRect().fRight), "o7");
static_assert(sizeof(RECT().bottom) == sizeof(SkIRect().fBottom), "o8");
static_assert(sizeof(RECT) == sizeof(SkIRect), "o9");

void CreateBitmapHeaderWithColorDepth(LONG width,
                                      LONG height,
                                      WORD color_depth,
                                      BITMAPINFOHEADER* hdr) {
  // These values are shared with gfx::PlatformDevice.
  hdr->biSize = sizeof(BITMAPINFOHEADER);
  hdr->biWidth = width;
  hdr->biHeight = -height;  // Minus means top-down bitmap.
  hdr->biPlanes = 1;
  hdr->biBitCount = color_depth;
  hdr->biCompression = BI_RGB;  // No compression.
  hdr->biSizeImage = 0;
  hdr->biXPelsPerMeter = 1;
  hdr->biYPelsPerMeter = 1;
  hdr->biClrUsed = 0;
  hdr->biClrImportant = 0;
}

}  // namespace

namespace skia {

POINT SkPointToPOINT(const SkPoint& point) {
  POINT win_point = {
      SkScalarRoundToInt(point.fX), SkScalarRoundToInt(point.fY)
  };
  return win_point;
}

SkRect RECTToSkRect(const RECT& rect) {
  SkRect sk_rect = { SkIntToScalar(rect.left), SkIntToScalar(rect.top),
                     SkIntToScalar(rect.right), SkIntToScalar(rect.bottom) };
  return sk_rect;
}

SkColor COLORREFToSkColor(COLORREF color) {
#ifndef _MSC_VER
  return SkColorSetRGB(GetRValue(color), GetGValue(color), GetBValue(color));
#else
  // ARGB = 0xFF000000 | ((0BGR -> RGB0) >> 8)
  return 0xFF000000u | (_byteswap_ulong(color) >> 8);
#endif
}

COLORREF SkColorToCOLORREF(SkColor color) {
#ifndef _MSC_VER
  return RGB(SkColorGetR(color), SkColorGetG(color), SkColorGetB(color));
#else
  // 0BGR = ((ARGB -> BGRA) >> 8)
  return (_byteswap_ulong(color) >> 8);
#endif
}

void InitializeDC(HDC context) {
  // Enables world transformation.
  // If the GM_ADVANCED graphics mode is set, GDI always draws arcs in the
  // counterclockwise direction in logical space. This is equivalent to the
  // statement that, in the GM_ADVANCED graphics mode, both arc control points
  // and arcs themselves fully respect the device context's world-to-device
  // transformation.
  BOOL res = SetGraphicsMode(context, GM_ADVANCED);
  SkASSERT(res != 0);

  // Enables dithering.
  res = SetStretchBltMode(context, HALFTONE);
  SkASSERT(res != 0);
  // As per SetStretchBltMode() documentation, SetBrushOrgEx() must be called
  // right after.
  res = SetBrushOrgEx(context, 0, 0, NULL);
  SkASSERT(res != 0);

  // Sets up default orientation.
  res = SetArcDirection(context, AD_CLOCKWISE);
  SkASSERT(res != 0);

  // Sets up default colors.
  res = SetBkColor(context, RGB(255, 255, 255));
  SkASSERT(res != CLR_INVALID);
  res = SetTextColor(context, RGB(0, 0, 0));
  SkASSERT(res != CLR_INVALID);
  res = SetDCBrushColor(context, RGB(255, 255, 255));
  SkASSERT(res != CLR_INVALID);
  res = SetDCPenColor(context, RGB(0, 0, 0));
  SkASSERT(res != CLR_INVALID);

  // Sets up default transparency.
  res = SetBkMode(context, OPAQUE);
  SkASSERT(res != 0);
  res = SetROP2(context, R2_COPYPEN);
  SkASSERT(res != 0);
}

void LoadTransformToDC(HDC dc, const SkMatrix& matrix) {
  XFORM xf;
  xf.eM11 = matrix[SkMatrix::kMScaleX];
  xf.eM21 = matrix[SkMatrix::kMSkewX];
  xf.eDx = matrix[SkMatrix::kMTransX];
  xf.eM12 = matrix[SkMatrix::kMSkewY];
  xf.eM22 = matrix[SkMatrix::kMScaleY];
  xf.eDy = matrix[SkMatrix::kMTransY];
  SetWorldTransform(dc, &xf);
}

void CopyHDC(HDC source, HDC destination, int x, int y, bool is_opaque,
             const RECT& src_rect, const SkMatrix& transform) {

  int copy_width = src_rect.right - src_rect.left;
  int copy_height = src_rect.bottom - src_rect.top;

  // We need to reset the translation for our bitmap or (0,0) won't be in the
  // upper left anymore
  SkMatrix identity;
  identity.reset();

  LoadTransformToDC(source, identity);
  if (is_opaque) {
    BitBlt(destination,
           x,
           y,
           copy_width,
           copy_height,
           source,
           src_rect.left,
           src_rect.top,
           SRCCOPY);
  } else {
    SkASSERT(copy_width != 0 && copy_height != 0);
    BLENDFUNCTION blend_function = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    GdiAlphaBlend(destination,
                  x,
                  y,
                  copy_width,
                  copy_height,
                  source,
                  src_rect.left,
                  src_rect.top,
                  copy_width,
                  copy_height,
                  blend_function);
  }
  LoadTransformToDC(source, transform);
}

SkImageInfo PrepareAllocation(HDC context, BITMAP* backing) {
  HBITMAP backing_handle =
      static_cast<HBITMAP>(GetCurrentObject(context, OBJ_BITMAP));
  const size_t backing_size = sizeof *backing;
  return (GetObject(backing_handle, backing_size, backing) == backing_size)
            ? SkImageInfo::MakeN32Premul(backing->bmWidth, backing->bmHeight)
            : SkImageInfo();
}

sk_sp<SkSurface> MapPlatformSurface(HDC context) {
  BITMAP backing;
  const SkImageInfo size(PrepareAllocation(context, &backing));
  SkSurfaceProps props = skia::LegacyDisplayGlobals::GetSkSurfaceProps();
  return size.isEmpty()
             ? nullptr
             : SkSurface::MakeRasterDirect(size, backing.bmBits,
                                           backing.bmWidthBytes, &props);
}

SkBitmap MapPlatformBitmap(HDC context) {
  BITMAP backing;
  const SkImageInfo size(PrepareAllocation(context, &backing));
  SkBitmap bitmap;
  if (!size.isEmpty())
    bitmap.installPixels(size, backing.bmBits, size.minRowBytes());
  return bitmap;
}

void CreateBitmapHeaderForN32SkBitmap(const SkBitmap& bitmap,
                                      BITMAPINFOHEADER* hdr) {
  // Native HBITMAPs are XRGB-backed, and we expect SkBitmaps that we will use
  // with them to also be of the same format.
  CHECK_EQ(bitmap.colorType(), kN32_SkColorType);
  // The header will be for an RGB bitmap with 32 bits-per-pixel. The SkBitmap
  // data to go into the bitmap should be of the same size. If the SkBitmap
  // SkColorType is for a larger number of bits-per-pixel, copying the SkBitmap
  // into the HBITMAP for this header would cause a write out-of-bounds.
  CHECK_EQ(4, bitmap.info().bytesPerPixel());
  // The HBITMAP's bytes will always be tightly packed so we expect the SkBitmap
  // to be also. Row padding would mean the number of bytes in the SkBitmap and
  // in the HBITMAP for this header would be different, which can cause out-of-
  // bound reads or writes.
  CHECK_EQ(bitmap.rowBytes(), bitmap.width() * static_cast<size_t>(4));

  CreateBitmapHeaderWithColorDepth(bitmap.width(), bitmap.height(), 32, hdr);
}

base::win::ScopedBitmap CreateHBitmapFromN32SkBitmap(const SkBitmap& bitmap) {
  BITMAPINFOHEADER header;
  CreateBitmapHeaderForN32SkBitmap(bitmap, &header);

  int width = bitmap.width();
  int height = bitmap.height();

  size_t bytes;
  // Native HBITMAPs store 32-bit RGB data, and the SkBitmap used with it must
  // also, as verified by CreateBitmapHeaderForN32SkBitmap(). A size_t type
  // causes a type change from int when multiplying.
  const size_t bpp = 4;
  if (!base::CheckMul(height, base::CheckMul(width, bpp)).AssignIfValid(&bytes))
    return {};

  void* bits;
  HBITMAP hbitmap;
  {
    base::win::ScopedGetDC screen_dc(nullptr);
    // By giving a null hSection, the |bits| will be destroyed when the
    // |hbitmap| is destroyed.
    hbitmap =
        CreateDIBSection(screen_dc, reinterpret_cast<BITMAPINFO*>(&header),
                         DIB_RGB_COLORS, &bits, nullptr, 0);
  }
  if (hbitmap) {
    memcpy(bits, bitmap.getPixels(), bytes);
  } else {
    // If CreateDIBSection() failed, try to get some useful information out
    // before we crash for post-mortem analysis.
    base::debug::CollectGDIUsageAndDie(&header, nullptr);
  }

  return base::win::ScopedBitmap(hbitmap);
}

void CreateBitmapHeaderForXRGB888(int width,
                                  int height,
                                  BITMAPINFOHEADER* hdr) {
  CreateBitmapHeaderWithColorDepth(width, height, 32, hdr);
}

base::win::ScopedBitmap CreateHBitmapXRGB8888(int width,
                                              int height,
                                              HANDLE shared_section,
                                              void** data) {
  // CreateDIBSection fails to allocate anything if we try to create an empty
  // bitmap, so just create a minimal bitmap.
  if ((width == 0) || (height == 0)) {
    width = 1;
    height = 1;
  }

  BITMAPINFOHEADER hdr = {0};
  CreateBitmapHeaderWithColorDepth(width, height, 32, &hdr);
  HBITMAP hbitmap = CreateDIBSection(NULL, reinterpret_cast<BITMAPINFO*>(&hdr),
                                     0, data, shared_section, 0);

  // If CreateDIBSection() failed, try to get some useful information out
  // before we crash for post-mortem analysis.
  if (!hbitmap)
    base::debug::CollectGDIUsageAndDie(&hdr, shared_section);

  return base::win::ScopedBitmap(hbitmap);
}

}  // namespace skia

