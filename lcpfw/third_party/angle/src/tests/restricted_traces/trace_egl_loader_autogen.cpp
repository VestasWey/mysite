// GENERATED FILE - DO NOT EDIT.
// Generated by generate_loader.py using data from egl.xml and egl_angle_ext.xml.
//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// egl_loader_autogen.cpp:
//   Simple EGL function loader.

#include "trace_egl_loader_autogen.h"

ANGLE_TRACE_LOADER_EXPORT PFNEGLCHOOSECONFIGPROC t_eglChooseConfig;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCOPYBUFFERSPROC t_eglCopyBuffers;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATECONTEXTPROC t_eglCreateContext;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEPBUFFERSURFACEPROC t_eglCreatePbufferSurface;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEPIXMAPSURFACEPROC t_eglCreatePixmapSurface;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEWINDOWSURFACEPROC t_eglCreateWindowSurface;
ANGLE_TRACE_LOADER_EXPORT PFNEGLDESTROYCONTEXTPROC t_eglDestroyContext;
ANGLE_TRACE_LOADER_EXPORT PFNEGLDESTROYSURFACEPROC t_eglDestroySurface;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETCONFIGATTRIBPROC t_eglGetConfigAttrib;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETCONFIGSPROC t_eglGetConfigs;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETCURRENTDISPLAYPROC t_eglGetCurrentDisplay;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETCURRENTSURFACEPROC t_eglGetCurrentSurface;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETDISPLAYPROC t_eglGetDisplay;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETERRORPROC t_eglGetError;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETPROCADDRESSPROC t_eglGetProcAddress;
ANGLE_TRACE_LOADER_EXPORT PFNEGLINITIALIZEPROC t_eglInitialize;
ANGLE_TRACE_LOADER_EXPORT PFNEGLMAKECURRENTPROC t_eglMakeCurrent;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYCONTEXTPROC t_eglQueryContext;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYSTRINGPROC t_eglQueryString;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYSURFACEPROC t_eglQuerySurface;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSWAPBUFFERSPROC t_eglSwapBuffers;
ANGLE_TRACE_LOADER_EXPORT PFNEGLTERMINATEPROC t_eglTerminate;
ANGLE_TRACE_LOADER_EXPORT PFNEGLWAITGLPROC t_eglWaitGL;
ANGLE_TRACE_LOADER_EXPORT PFNEGLWAITNATIVEPROC t_eglWaitNative;
ANGLE_TRACE_LOADER_EXPORT PFNEGLBINDTEXIMAGEPROC t_eglBindTexImage;
ANGLE_TRACE_LOADER_EXPORT PFNEGLRELEASETEXIMAGEPROC t_eglReleaseTexImage;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSURFACEATTRIBPROC t_eglSurfaceAttrib;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSWAPINTERVALPROC t_eglSwapInterval;
ANGLE_TRACE_LOADER_EXPORT PFNEGLBINDAPIPROC t_eglBindAPI;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYAPIPROC t_eglQueryAPI;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC
    t_eglCreatePbufferFromClientBuffer;
ANGLE_TRACE_LOADER_EXPORT PFNEGLRELEASETHREADPROC t_eglReleaseThread;
ANGLE_TRACE_LOADER_EXPORT PFNEGLWAITCLIENTPROC t_eglWaitClient;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETCURRENTCONTEXTPROC t_eglGetCurrentContext;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATESYNCPROC t_eglCreateSync;
ANGLE_TRACE_LOADER_EXPORT PFNEGLDESTROYSYNCPROC t_eglDestroySync;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCLIENTWAITSYNCPROC t_eglClientWaitSync;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETSYNCATTRIBPROC t_eglGetSyncAttrib;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEIMAGEPROC t_eglCreateImage;
ANGLE_TRACE_LOADER_EXPORT PFNEGLDESTROYIMAGEPROC t_eglDestroyImage;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETPLATFORMDISPLAYPROC t_eglGetPlatformDisplay;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEPLATFORMWINDOWSURFACEPROC t_eglCreatePlatformWindowSurface;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC t_eglCreatePlatformPixmapSurface;
ANGLE_TRACE_LOADER_EXPORT PFNEGLWAITSYNCPROC t_eglWaitSync;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSETBLOBCACHEFUNCSANDROIDPROC t_eglSetBlobCacheFuncsANDROID;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATENATIVECLIENTBUFFERANDROIDPROC
    t_eglCreateNativeClientBufferANDROID;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETCOMPOSITORTIMINGANDROIDPROC t_eglGetCompositorTimingANDROID;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC
    t_eglGetCompositorTimingSupportedANDROID;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC
    t_eglGetFrameTimestampSupportedANDROID;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETFRAMETIMESTAMPSANDROIDPROC t_eglGetFrameTimestampsANDROID;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETNEXTFRAMEIDANDROIDPROC t_eglGetNextFrameIdANDROID;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC t_eglGetNativeClientBufferANDROID;
ANGLE_TRACE_LOADER_EXPORT PFNEGLDUPNATIVEFENCEFDANDROIDPROC t_eglDupNativeFenceFDANDROID;
ANGLE_TRACE_LOADER_EXPORT PFNEGLPRESENTATIONTIMEANDROIDPROC t_eglPresentationTimeANDROID;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEDEVICEANGLEPROC t_eglCreateDeviceANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLRELEASEDEVICEANGLEPROC t_eglReleaseDeviceANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYDISPLAYATTRIBANGLEPROC t_eglQueryDisplayAttribANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYSTRINGIANGLEPROC t_eglQueryStringiANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLHANDLEGPUSWITCHANGLEPROC t_eglHandleGPUSwitchANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLREACQUIREHIGHPOWERGPUANGLEPROC t_eglReacquireHighPowerGPUANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLRELEASEHIGHPOWERGPUANGLEPROC t_eglReleaseHighPowerGPUANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLPROGRAMCACHEGETATTRIBANGLEPROC t_eglProgramCacheGetAttribANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLPROGRAMCACHEPOPULATEANGLEPROC t_eglProgramCachePopulateANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLPROGRAMCACHEQUERYANGLEPROC t_eglProgramCacheQueryANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLPROGRAMCACHERESIZEANGLEPROC t_eglProgramCacheResizeANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYSURFACEPOINTERANGLEPROC t_eglQuerySurfacePointerANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATESTREAMPRODUCERD3DTEXTUREANGLEPROC
    t_eglCreateStreamProducerD3DTextureANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSTREAMPOSTD3DTEXTUREANGLEPROC t_eglStreamPostD3DTextureANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSWAPBUFFERSWITHFRAMETOKENANGLEPROC
    t_eglSwapBuffersWithFrameTokenANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETMSCRATEANGLEPROC t_eglGetMscRateANGLE;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETSYNCVALUESCHROMIUMPROC t_eglGetSyncValuesCHROMIUM;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYDEVICEATTRIBEXTPROC t_eglQueryDeviceAttribEXT;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYDEVICESTRINGEXTPROC t_eglQueryDeviceStringEXT;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYDISPLAYATTRIBEXTPROC t_eglQueryDisplayAttribEXT;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC
    t_eglCreatePlatformPixmapSurfaceEXT;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC
    t_eglCreatePlatformWindowSurfaceEXT;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETPLATFORMDISPLAYEXTPROC t_eglGetPlatformDisplayEXT;
ANGLE_TRACE_LOADER_EXPORT PFNEGLDEBUGMESSAGECONTROLKHRPROC t_eglDebugMessageControlKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLLABELOBJECTKHRPROC t_eglLabelObjectKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYDEBUGKHRPROC t_eglQueryDebugKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCLIENTWAITSYNCKHRPROC t_eglClientWaitSyncKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATESYNCKHRPROC t_eglCreateSyncKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLDESTROYSYNCKHRPROC t_eglDestroySyncKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLGETSYNCATTRIBKHRPROC t_eglGetSyncAttribKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATEIMAGEKHRPROC t_eglCreateImageKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLDESTROYIMAGEKHRPROC t_eglDestroyImageKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSIGNALSYNCKHRPROC t_eglSignalSyncKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLCREATESTREAMKHRPROC t_eglCreateStreamKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLDESTROYSTREAMKHRPROC t_eglDestroyStreamKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYSTREAMKHRPROC t_eglQueryStreamKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLQUERYSTREAMU64KHRPROC t_eglQueryStreamu64KHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSTREAMATTRIBKHRPROC t_eglStreamAttribKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSTREAMCONSUMERACQUIREKHRPROC t_eglStreamConsumerAcquireKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC
    t_eglStreamConsumerGLTextureExternalKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSTREAMCONSUMERRELEASEKHRPROC t_eglStreamConsumerReleaseKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC t_eglSwapBuffersWithDamageKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLWAITSYNCKHRPROC t_eglWaitSyncKHR;
ANGLE_TRACE_LOADER_EXPORT PFNEGLPOSTSUBBUFFERNVPROC t_eglPostSubBufferNV;
ANGLE_TRACE_LOADER_EXPORT PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALATTRIBSNVPROC
    t_eglStreamConsumerGLTextureExternalAttribsNV;

namespace trace_angle
{
void LoadEGL(LoadProc loadProc)
{
    t_eglChooseConfig  = reinterpret_cast<PFNEGLCHOOSECONFIGPROC>(loadProc("eglChooseConfig"));
    t_eglCopyBuffers   = reinterpret_cast<PFNEGLCOPYBUFFERSPROC>(loadProc("eglCopyBuffers"));
    t_eglCreateContext = reinterpret_cast<PFNEGLCREATECONTEXTPROC>(loadProc("eglCreateContext"));
    t_eglCreatePbufferSurface =
        reinterpret_cast<PFNEGLCREATEPBUFFERSURFACEPROC>(loadProc("eglCreatePbufferSurface"));
    t_eglCreatePixmapSurface =
        reinterpret_cast<PFNEGLCREATEPIXMAPSURFACEPROC>(loadProc("eglCreatePixmapSurface"));
    t_eglCreateWindowSurface =
        reinterpret_cast<PFNEGLCREATEWINDOWSURFACEPROC>(loadProc("eglCreateWindowSurface"));
    t_eglDestroyContext = reinterpret_cast<PFNEGLDESTROYCONTEXTPROC>(loadProc("eglDestroyContext"));
    t_eglDestroySurface = reinterpret_cast<PFNEGLDESTROYSURFACEPROC>(loadProc("eglDestroySurface"));
    t_eglGetConfigAttrib =
        reinterpret_cast<PFNEGLGETCONFIGATTRIBPROC>(loadProc("eglGetConfigAttrib"));
    t_eglGetConfigs = reinterpret_cast<PFNEGLGETCONFIGSPROC>(loadProc("eglGetConfigs"));
    t_eglGetCurrentDisplay =
        reinterpret_cast<PFNEGLGETCURRENTDISPLAYPROC>(loadProc("eglGetCurrentDisplay"));
    t_eglGetCurrentSurface =
        reinterpret_cast<PFNEGLGETCURRENTSURFACEPROC>(loadProc("eglGetCurrentSurface"));
    t_eglGetDisplay     = reinterpret_cast<PFNEGLGETDISPLAYPROC>(loadProc("eglGetDisplay"));
    t_eglGetError       = reinterpret_cast<PFNEGLGETERRORPROC>(loadProc("eglGetError"));
    t_eglGetProcAddress = reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(loadProc("eglGetProcAddress"));
    t_eglInitialize     = reinterpret_cast<PFNEGLINITIALIZEPROC>(loadProc("eglInitialize"));
    t_eglMakeCurrent    = reinterpret_cast<PFNEGLMAKECURRENTPROC>(loadProc("eglMakeCurrent"));
    t_eglQueryContext   = reinterpret_cast<PFNEGLQUERYCONTEXTPROC>(loadProc("eglQueryContext"));
    t_eglQueryString    = reinterpret_cast<PFNEGLQUERYSTRINGPROC>(loadProc("eglQueryString"));
    t_eglQuerySurface   = reinterpret_cast<PFNEGLQUERYSURFACEPROC>(loadProc("eglQuerySurface"));
    t_eglSwapBuffers    = reinterpret_cast<PFNEGLSWAPBUFFERSPROC>(loadProc("eglSwapBuffers"));
    t_eglTerminate      = reinterpret_cast<PFNEGLTERMINATEPROC>(loadProc("eglTerminate"));
    t_eglWaitGL         = reinterpret_cast<PFNEGLWAITGLPROC>(loadProc("eglWaitGL"));
    t_eglWaitNative     = reinterpret_cast<PFNEGLWAITNATIVEPROC>(loadProc("eglWaitNative"));
    t_eglBindTexImage   = reinterpret_cast<PFNEGLBINDTEXIMAGEPROC>(loadProc("eglBindTexImage"));
    t_eglReleaseTexImage =
        reinterpret_cast<PFNEGLRELEASETEXIMAGEPROC>(loadProc("eglReleaseTexImage"));
    t_eglSurfaceAttrib = reinterpret_cast<PFNEGLSURFACEATTRIBPROC>(loadProc("eglSurfaceAttrib"));
    t_eglSwapInterval  = reinterpret_cast<PFNEGLSWAPINTERVALPROC>(loadProc("eglSwapInterval"));
    t_eglBindAPI       = reinterpret_cast<PFNEGLBINDAPIPROC>(loadProc("eglBindAPI"));
    t_eglQueryAPI      = reinterpret_cast<PFNEGLQUERYAPIPROC>(loadProc("eglQueryAPI"));
    t_eglCreatePbufferFromClientBuffer = reinterpret_cast<PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC>(
        loadProc("eglCreatePbufferFromClientBuffer"));
    t_eglReleaseThread = reinterpret_cast<PFNEGLRELEASETHREADPROC>(loadProc("eglReleaseThread"));
    t_eglWaitClient    = reinterpret_cast<PFNEGLWAITCLIENTPROC>(loadProc("eglWaitClient"));
    t_eglGetCurrentContext =
        reinterpret_cast<PFNEGLGETCURRENTCONTEXTPROC>(loadProc("eglGetCurrentContext"));
    t_eglCreateSync     = reinterpret_cast<PFNEGLCREATESYNCPROC>(loadProc("eglCreateSync"));
    t_eglDestroySync    = reinterpret_cast<PFNEGLDESTROYSYNCPROC>(loadProc("eglDestroySync"));
    t_eglClientWaitSync = reinterpret_cast<PFNEGLCLIENTWAITSYNCPROC>(loadProc("eglClientWaitSync"));
    t_eglGetSyncAttrib  = reinterpret_cast<PFNEGLGETSYNCATTRIBPROC>(loadProc("eglGetSyncAttrib"));
    t_eglCreateImage    = reinterpret_cast<PFNEGLCREATEIMAGEPROC>(loadProc("eglCreateImage"));
    t_eglDestroyImage   = reinterpret_cast<PFNEGLDESTROYIMAGEPROC>(loadProc("eglDestroyImage"));
    t_eglGetPlatformDisplay =
        reinterpret_cast<PFNEGLGETPLATFORMDISPLAYPROC>(loadProc("eglGetPlatformDisplay"));
    t_eglCreatePlatformWindowSurface = reinterpret_cast<PFNEGLCREATEPLATFORMWINDOWSURFACEPROC>(
        loadProc("eglCreatePlatformWindowSurface"));
    t_eglCreatePlatformPixmapSurface = reinterpret_cast<PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC>(
        loadProc("eglCreatePlatformPixmapSurface"));
    t_eglWaitSync                 = reinterpret_cast<PFNEGLWAITSYNCPROC>(loadProc("eglWaitSync"));
    t_eglSetBlobCacheFuncsANDROID = reinterpret_cast<PFNEGLSETBLOBCACHEFUNCSANDROIDPROC>(
        loadProc("eglSetBlobCacheFuncsANDROID"));
    t_eglCreateNativeClientBufferANDROID =
        reinterpret_cast<PFNEGLCREATENATIVECLIENTBUFFERANDROIDPROC>(
            loadProc("eglCreateNativeClientBufferANDROID"));
    t_eglGetCompositorTimingANDROID = reinterpret_cast<PFNEGLGETCOMPOSITORTIMINGANDROIDPROC>(
        loadProc("eglGetCompositorTimingANDROID"));
    t_eglGetCompositorTimingSupportedANDROID =
        reinterpret_cast<PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC>(
            loadProc("eglGetCompositorTimingSupportedANDROID"));
    t_eglGetFrameTimestampSupportedANDROID =
        reinterpret_cast<PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC>(
            loadProc("eglGetFrameTimestampSupportedANDROID"));
    t_eglGetFrameTimestampsANDROID = reinterpret_cast<PFNEGLGETFRAMETIMESTAMPSANDROIDPROC>(
        loadProc("eglGetFrameTimestampsANDROID"));
    t_eglGetNextFrameIdANDROID =
        reinterpret_cast<PFNEGLGETNEXTFRAMEIDANDROIDPROC>(loadProc("eglGetNextFrameIdANDROID"));
    t_eglGetNativeClientBufferANDROID = reinterpret_cast<PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC>(
        loadProc("eglGetNativeClientBufferANDROID"));
    t_eglDupNativeFenceFDANDROID =
        reinterpret_cast<PFNEGLDUPNATIVEFENCEFDANDROIDPROC>(loadProc("eglDupNativeFenceFDANDROID"));
    t_eglPresentationTimeANDROID =
        reinterpret_cast<PFNEGLPRESENTATIONTIMEANDROIDPROC>(loadProc("eglPresentationTimeANDROID"));
    t_eglCreateDeviceANGLE =
        reinterpret_cast<PFNEGLCREATEDEVICEANGLEPROC>(loadProc("eglCreateDeviceANGLE"));
    t_eglReleaseDeviceANGLE =
        reinterpret_cast<PFNEGLRELEASEDEVICEANGLEPROC>(loadProc("eglReleaseDeviceANGLE"));
    t_eglQueryDisplayAttribANGLE =
        reinterpret_cast<PFNEGLQUERYDISPLAYATTRIBANGLEPROC>(loadProc("eglQueryDisplayAttribANGLE"));
    t_eglQueryStringiANGLE =
        reinterpret_cast<PFNEGLQUERYSTRINGIANGLEPROC>(loadProc("eglQueryStringiANGLE"));
    t_eglHandleGPUSwitchANGLE =
        reinterpret_cast<PFNEGLHANDLEGPUSWITCHANGLEPROC>(loadProc("eglHandleGPUSwitchANGLE"));
    t_eglReacquireHighPowerGPUANGLE = reinterpret_cast<PFNEGLREACQUIREHIGHPOWERGPUANGLEPROC>(
        loadProc("eglReacquireHighPowerGPUANGLE"));
    t_eglReleaseHighPowerGPUANGLE = reinterpret_cast<PFNEGLRELEASEHIGHPOWERGPUANGLEPROC>(
        loadProc("eglReleaseHighPowerGPUANGLE"));
    t_eglProgramCacheGetAttribANGLE = reinterpret_cast<PFNEGLPROGRAMCACHEGETATTRIBANGLEPROC>(
        loadProc("eglProgramCacheGetAttribANGLE"));
    t_eglProgramCachePopulateANGLE = reinterpret_cast<PFNEGLPROGRAMCACHEPOPULATEANGLEPROC>(
        loadProc("eglProgramCachePopulateANGLE"));
    t_eglProgramCacheQueryANGLE =
        reinterpret_cast<PFNEGLPROGRAMCACHEQUERYANGLEPROC>(loadProc("eglProgramCacheQueryANGLE"));
    t_eglProgramCacheResizeANGLE =
        reinterpret_cast<PFNEGLPROGRAMCACHERESIZEANGLEPROC>(loadProc("eglProgramCacheResizeANGLE"));
    t_eglQuerySurfacePointerANGLE = reinterpret_cast<PFNEGLQUERYSURFACEPOINTERANGLEPROC>(
        loadProc("eglQuerySurfacePointerANGLE"));
    t_eglCreateStreamProducerD3DTextureANGLE =
        reinterpret_cast<PFNEGLCREATESTREAMPRODUCERD3DTEXTUREANGLEPROC>(
            loadProc("eglCreateStreamProducerD3DTextureANGLE"));
    t_eglStreamPostD3DTextureANGLE = reinterpret_cast<PFNEGLSTREAMPOSTD3DTEXTUREANGLEPROC>(
        loadProc("eglStreamPostD3DTextureANGLE"));
    t_eglSwapBuffersWithFrameTokenANGLE =
        reinterpret_cast<PFNEGLSWAPBUFFERSWITHFRAMETOKENANGLEPROC>(
            loadProc("eglSwapBuffersWithFrameTokenANGLE"));
    t_eglGetMscRateANGLE =
        reinterpret_cast<PFNEGLGETMSCRATEANGLEPROC>(loadProc("eglGetMscRateANGLE"));
    t_eglGetSyncValuesCHROMIUM =
        reinterpret_cast<PFNEGLGETSYNCVALUESCHROMIUMPROC>(loadProc("eglGetSyncValuesCHROMIUM"));
    t_eglQueryDeviceAttribEXT =
        reinterpret_cast<PFNEGLQUERYDEVICEATTRIBEXTPROC>(loadProc("eglQueryDeviceAttribEXT"));
    t_eglQueryDeviceStringEXT =
        reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(loadProc("eglQueryDeviceStringEXT"));
    t_eglQueryDisplayAttribEXT =
        reinterpret_cast<PFNEGLQUERYDISPLAYATTRIBEXTPROC>(loadProc("eglQueryDisplayAttribEXT"));
    t_eglCreatePlatformPixmapSurfaceEXT =
        reinterpret_cast<PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC>(
            loadProc("eglCreatePlatformPixmapSurfaceEXT"));
    t_eglCreatePlatformWindowSurfaceEXT =
        reinterpret_cast<PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC>(
            loadProc("eglCreatePlatformWindowSurfaceEXT"));
    t_eglGetPlatformDisplayEXT =
        reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(loadProc("eglGetPlatformDisplayEXT"));
    t_eglDebugMessageControlKHR =
        reinterpret_cast<PFNEGLDEBUGMESSAGECONTROLKHRPROC>(loadProc("eglDebugMessageControlKHR"));
    t_eglLabelObjectKHR = reinterpret_cast<PFNEGLLABELOBJECTKHRPROC>(loadProc("eglLabelObjectKHR"));
    t_eglQueryDebugKHR  = reinterpret_cast<PFNEGLQUERYDEBUGKHRPROC>(loadProc("eglQueryDebugKHR"));
    t_eglClientWaitSyncKHR =
        reinterpret_cast<PFNEGLCLIENTWAITSYNCKHRPROC>(loadProc("eglClientWaitSyncKHR"));
    t_eglCreateSyncKHR  = reinterpret_cast<PFNEGLCREATESYNCKHRPROC>(loadProc("eglCreateSyncKHR"));
    t_eglDestroySyncKHR = reinterpret_cast<PFNEGLDESTROYSYNCKHRPROC>(loadProc("eglDestroySyncKHR"));
    t_eglGetSyncAttribKHR =
        reinterpret_cast<PFNEGLGETSYNCATTRIBKHRPROC>(loadProc("eglGetSyncAttribKHR"));
    t_eglCreateImageKHR = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(loadProc("eglCreateImageKHR"));
    t_eglDestroyImageKHR =
        reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(loadProc("eglDestroyImageKHR"));
    t_eglSignalSyncKHR = reinterpret_cast<PFNEGLSIGNALSYNCKHRPROC>(loadProc("eglSignalSyncKHR"));
    t_eglCreateStreamKHR =
        reinterpret_cast<PFNEGLCREATESTREAMKHRPROC>(loadProc("eglCreateStreamKHR"));
    t_eglDestroyStreamKHR =
        reinterpret_cast<PFNEGLDESTROYSTREAMKHRPROC>(loadProc("eglDestroyStreamKHR"));
    t_eglQueryStreamKHR = reinterpret_cast<PFNEGLQUERYSTREAMKHRPROC>(loadProc("eglQueryStreamKHR"));
    t_eglQueryStreamu64KHR =
        reinterpret_cast<PFNEGLQUERYSTREAMU64KHRPROC>(loadProc("eglQueryStreamu64KHR"));
    t_eglStreamAttribKHR =
        reinterpret_cast<PFNEGLSTREAMATTRIBKHRPROC>(loadProc("eglStreamAttribKHR"));
    t_eglStreamConsumerAcquireKHR = reinterpret_cast<PFNEGLSTREAMCONSUMERACQUIREKHRPROC>(
        loadProc("eglStreamConsumerAcquireKHR"));
    t_eglStreamConsumerGLTextureExternalKHR =
        reinterpret_cast<PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC>(
            loadProc("eglStreamConsumerGLTextureExternalKHR"));
    t_eglStreamConsumerReleaseKHR = reinterpret_cast<PFNEGLSTREAMCONSUMERRELEASEKHRPROC>(
        loadProc("eglStreamConsumerReleaseKHR"));
    t_eglSwapBuffersWithDamageKHR = reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC>(
        loadProc("eglSwapBuffersWithDamageKHR"));
    t_eglWaitSyncKHR = reinterpret_cast<PFNEGLWAITSYNCKHRPROC>(loadProc("eglWaitSyncKHR"));
    t_eglPostSubBufferNV =
        reinterpret_cast<PFNEGLPOSTSUBBUFFERNVPROC>(loadProc("eglPostSubBufferNV"));
    t_eglStreamConsumerGLTextureExternalAttribsNV =
        reinterpret_cast<PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALATTRIBSNVPROC>(
            loadProc("eglStreamConsumerGLTextureExternalAttribsNV"));
}
}  // namespace trace_angle
