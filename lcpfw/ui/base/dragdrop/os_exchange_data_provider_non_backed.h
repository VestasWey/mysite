// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_DRAGDROP_OS_EXCHANGE_DATA_PROVIDER_NON_BACKED_H_
#define UI_BASE_DRAGDROP_OS_EXCHANGE_DATA_PROVIDER_NON_BACKED_H_

#include <map>
#include <memory>

#include "base/component_export.h"
#include "base/pickle.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "ui/base/clipboard/file_info.h"
#include "ui/base/dragdrop/os_exchange_data_provider.h"
#include "ui/gfx/geometry/vector2d.h"
#include "ui/gfx/image/image_skia.h"
#include "url/gurl.h"

namespace base {
class FilePath;
}  // namespace base

namespace ui {

class ClipboardFormatType;

// Simple OSExchangeDataProvider implementation for aura-based ports with no
// actual platform integration. So data managed by this class is exchangeable
// only among Chromium windows and is available only while it is alive.
class COMPONENT_EXPORT(UI_BASE) OSExchangeDataProviderNonBacked
    : public OSExchangeDataProvider {
 public:
  OSExchangeDataProviderNonBacked();
  OSExchangeDataProviderNonBacked(const OSExchangeDataProviderNonBacked&) =
      delete;
  OSExchangeDataProviderNonBacked& operator=(
      const OSExchangeDataProviderNonBacked&) = delete;
  ~OSExchangeDataProviderNonBacked() override;

  // Overridden from OSExchangeDataProvider:
  std::unique_ptr<OSExchangeDataProvider> Clone() const override;
  void MarkOriginatedFromRenderer() override;
  bool DidOriginateFromRenderer() const override;
  void SetString(const base::string16& data) override;
  void SetURL(const GURL& url, const base::string16& title) override;
  void SetFilename(const base::FilePath& path) override;
  void SetFilenames(const std::vector<FileInfo>& filenames) override;
  void SetPickledData(const ClipboardFormatType& format,
                      const base::Pickle& data) override;
  bool GetString(base::string16* data) const override;
  bool GetURLAndTitle(FilenameToURLPolicy policy,
                      GURL* url,
                      base::string16* title) const override;
  bool GetFilename(base::FilePath* path) const override;
  bool GetFilenames(std::vector<FileInfo>* filenames) const override;
  bool GetPickledData(const ClipboardFormatType& format,
                      base::Pickle* data) const override;
  bool HasString() const override;
  bool HasURL(FilenameToURLPolicy policy) const override;
  bool HasFile() const override;
  bool HasCustomFormat(const ClipboardFormatType& format) const override;
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
  void SetFileContents(const base::FilePath& filename,
                       const std::string& file_contents) override;
#endif

  void SetHtml(const base::string16& html, const GURL& base_url) override;
  bool GetHtml(base::string16* html, GURL* base_url) const override;
  bool HasHtml() const override;
  void SetDragImage(const gfx::ImageSkia& image,
                    const gfx::Vector2d& cursor_offset) override;
  gfx::ImageSkia GetDragImage() const override;
  gfx::Vector2d GetDragImageOffset() const override;

  void SetSource(std::unique_ptr<DataTransferEndpoint> data_source) override;
  DataTransferEndpoint* GetSource() const override;

 private:
  // Returns true if |formats_| contains a file format and the file name can be
  // parsed as a URL.
  bool GetFileURL(GURL* url) const;

  // Returns true if |formats_| contains a string format and the string can be
  // parsed as a URL.
  bool GetPlainTextURL(GURL* url) const;

  // Actual formats that have been set.
  // for details.
  int formats_ = 0;

  // String contents.
  base::string16 string_;

  // URL contents.
  GURL url_;
  base::string16 title_;

  // File name.
  std::vector<FileInfo> filenames_;

  // PICKLED_DATA contents.
  std::map<ClipboardFormatType, base::Pickle> pickle_data_;

  // Drag image and offset data.
  gfx::ImageSkia drag_image_;
  gfx::Vector2d drag_image_offset_;

  // For HTML format
  base::string16 html_;
  GURL base_url_;

#if !BUILDFLAG(IS_CHROMEOS_ASH)
  // For marking data originating from the renderer.
  bool originated_from_renderer_ = false;
#endif

  // Data source.
  std::unique_ptr<DataTransferEndpoint> source_;
};

}  // namespace ui

#endif  // UI_BASE_DRAGDROP_OS_EXCHANGE_DATA_PROVIDER_NON_BACKED_H_
