// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/resources/ui_resource_bitmap.h"

#include <stdint.h>

#include <memory>

#include "base/check_op.h"
#include "base/notreached.h"
#include "base/numerics/checked_math.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkMallocPixelRef.h"
#include "third_party/skia/include/core/SkPixelRef.h"

namespace cc {
namespace {

UIResourceBitmap::UIResourceFormat SkColorTypeToUIResourceFormat(
    SkColorType sk_type) {
  UIResourceBitmap::UIResourceFormat format = UIResourceBitmap::RGBA8;
  switch (sk_type) {
    case kN32_SkColorType:
      format = UIResourceBitmap::RGBA8;
      break;
    case kAlpha_8_SkColorType:
      format = UIResourceBitmap::ALPHA_8;
      break;
    default:
      NOTREACHED() << "Invalid SkColorType for UIResourceBitmap: " << sk_type;
      break;
  }
  return format;
}

}  // namespace

void UIResourceBitmap::Create(sk_sp<SkPixelRef> pixel_ref,
                              const SkImageInfo& info,
                              UIResourceFormat format) {
  DCHECK(info.width());
  DCHECK(info.height());
  DCHECK(pixel_ref);
  DCHECK(pixel_ref->isImmutable());
  format_ = format;
  info_ = info;
  pixel_ref_ = std::move(pixel_ref);
}

void UIResourceBitmap::DrawToCanvas(SkCanvas* canvas, SkPaint* paint) {
  DCHECK_NE(info_.colorType(), kUnknown_SkColorType);

  SkBitmap bitmap;
  bitmap.setInfo(info_, pixel_ref_.get()->rowBytes());
  bitmap.setPixelRef(pixel_ref_, 0, 0);
  canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), paint);
  canvas->flush();
}

size_t UIResourceBitmap::SizeInBytes() const {
  if (!pixel_ref_)
    return 0u;
  base::CheckedNumeric<size_t> size_in_bytes = pixel_ref_->rowBytes();
  size_in_bytes *= info_.height();
  return size_in_bytes.ValueOrDie();
}

UIResourceBitmap::UIResourceBitmap(const SkBitmap& skbitmap) {
  DCHECK(skbitmap.isImmutable());

  sk_sp<SkPixelRef> pixel_ref = sk_ref_sp(skbitmap.pixelRef());
  Create(std::move(pixel_ref), skbitmap.info(),
         SkColorTypeToUIResourceFormat(skbitmap.colorType()));
}

UIResourceBitmap::UIResourceBitmap(const gfx::Size& size, bool is_opaque) {
  SkAlphaType alphaType = is_opaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
  SkImageInfo info =
      SkImageInfo::MakeN32(size.width(), size.height(), alphaType);
  sk_sp<SkPixelRef> pixel_ref(
      SkMallocPixelRef::MakeAllocate(info, info.minRowBytes()));
  pixel_ref->setImmutable();
  Create(std::move(pixel_ref), info, UIResourceBitmap::RGBA8);
}

UIResourceBitmap::UIResourceBitmap(sk_sp<SkPixelRef> pixel_ref,
                                   const gfx::Size& size) {
  // TODO(khushalsagar): It doesn't make sense to SkPixelRef to pass around
  // encoded data.
  SkImageInfo info = SkImageInfo::Make(
      size.width(), size.height(), kUnknown_SkColorType, kOpaque_SkAlphaType);
  Create(std::move(pixel_ref), info, UIResourceBitmap::ETC1);
}

UIResourceBitmap::UIResourceBitmap(const UIResourceBitmap& other) = default;

UIResourceBitmap::~UIResourceBitmap() = default;
}  // namespace cc
