// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is auto-generated from
// gpu/command_buffer/build_raster_cmd_buffer.py
// It's formatted by clang-format using chromium coding style:
//    clang-format -i -style=chromium filename
// DO NOT EDIT!

#ifndef GPU_COMMAND_BUFFER_COMMON_RASTER_CMD_IDS_AUTOGEN_H_
#define GPU_COMMAND_BUFFER_COMMON_RASTER_CMD_IDS_AUTOGEN_H_

#define RASTER_COMMAND_LIST(OP)                            \
  OP(Finish)                                     /* 256 */ \
  OP(Flush)                                      /* 257 */ \
  OP(GetError)                                   /* 258 */ \
  OP(GenQueriesEXTImmediate)                     /* 259 */ \
  OP(DeleteQueriesEXTImmediate)                  /* 260 */ \
  OP(QueryCounterEXT)                            /* 261 */ \
  OP(BeginQueryEXT)                              /* 262 */ \
  OP(EndQueryEXT)                                /* 263 */ \
  OP(LoseContextCHROMIUM)                        /* 264 */ \
  OP(BeginRasterCHROMIUMImmediate)               /* 265 */ \
  OP(RasterCHROMIUM)                             /* 266 */ \
  OP(EndRasterCHROMIUM)                          /* 267 */ \
  OP(CreateTransferCacheEntryINTERNAL)           /* 268 */ \
  OP(DeleteTransferCacheEntryINTERNAL)           /* 269 */ \
  OP(UnlockTransferCacheEntryINTERNAL)           /* 270 */ \
  OP(DeletePaintCacheTextBlobsINTERNALImmediate) /* 271 */ \
  OP(DeletePaintCachePathsINTERNALImmediate)     /* 272 */ \
  OP(ClearPaintCacheINTERNAL)                    /* 273 */ \
  OP(CopySubTextureINTERNALImmediate)            /* 274 */ \
  OP(WritePixelsINTERNALImmediate)               /* 275 */ \
  OP(ReadbackImagePixelsINTERNALImmediate)       /* 276 */ \
  OP(ConvertYUVAMailboxesToRGBINTERNALImmediate) /* 277 */ \
  OP(TraceBeginCHROMIUM)                         /* 278 */ \
  OP(TraceEndCHROMIUM)                           /* 279 */ \
  OP(SetActiveURLCHROMIUM)                       /* 280 */

enum CommandId {
  kOneBeforeStartPoint =
      cmd::kLastCommonId,  // All Raster commands start after this.
#define RASTER_CMD_OP(name) k##name,
  RASTER_COMMAND_LIST(RASTER_CMD_OP)
#undef RASTER_CMD_OP
      kNumCommands,
  kFirstRasterCommand = kOneBeforeStartPoint + 1
};

#endif  // GPU_COMMAND_BUFFER_COMMON_RASTER_CMD_IDS_AUTOGEN_H_
