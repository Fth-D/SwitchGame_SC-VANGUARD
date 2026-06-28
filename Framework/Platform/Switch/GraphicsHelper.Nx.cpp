/*--------------------------------------------------------------------------------*
  Copyright (C)Nintendo All rights reserved.

  These coded instructions, statements, and computer programs contain proprietary
  information of Nintendo and/or its licensed developers and are protected by
  national and international copyright laws. They may not be disclosed to third
  parties or copied or duplicated in any form, in whole or in part, without the
  prior written consent of Nintendo.

  The content herein is highly confidential and should be handled accordingly.
 *--------------------------------------------------------------------------------*/

#include <nn/nn_Assert.h>
#include <nn/vi.h>
#include <nv/nv_MemoryManagement.h>
#include <nn/gll.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <algorithm>
#include <cstdlib>

#include "GraphicsHelper.h"

namespace
{
    // 携帯モードのネイティブ解像度でレイヤーを作成する。
    // ドックモード対応する際はここを動的に切り替えるか、max を 1920×1080 にして
    // Resize() でクロップする方式に戻す。
    const int MaxWidth = 1280;
    const int MaxHeight = 720;

    EGLDisplay g_Display;
    EGLSurface g_Surface;
    EGLContext g_Context;

    nn::vi::NativeWindowHandle g_NativeWindowHandle;
    nn::vi::Display* g_pDisplay;
    nn::vi::Layer* g_pLayer;

    void* NvAllocateFunction( size_t size, size_t alignment, void* userPtr )
    {
        NN_UNUSED( userPtr );
        // According to specifications of aligned_alloc(), we need to coordinate the size parameter to become the integral multiple of alignment.
        return aligned_alloc( alignment, nn::util::align_up( size, alignment ) );
    }
    void NvFreeFunction( void* addr, void* userPtr )
    {
        NN_UNUSED( userPtr );
        free( addr );
    }
    void* NvReallocateFunction( void* addr, size_t newSize, void* userPtr )
    {
        NN_UNUSED( userPtr );
        return realloc( addr, newSize );
    }

    void* NvDevtoolsAllocateFunction( size_t size, size_t alignment, void* userPtr )
    {
        NN_UNUSED( userPtr );
        // According to specifications of aligned_alloc(), we need to coordinate the size parameter to become the integral multiple of alignment.
        return aligned_alloc( alignment, nn::util::align_up( size, alignment ) );
    }
    void NvDevtoolsFreeFunction( void* addr, void* userPtr )
    {
        NN_UNUSED( userPtr );
        free( addr );
    }
    void* NvDevtoolsReallocateFunction( void* addr, size_t newSize, void* userPtr )
    {
        NN_UNUSED( userPtr );
        return realloc( addr, newSize );
    }
}

void GraphicsHelper::Initialize()
{
    /*
    * Set memory allocator for graphics subsystem.
    * This function must be called before using any graphics API's.
    */
    nv::SetGraphicsAllocator( NvAllocateFunction, NvFreeFunction, NvReallocateFunction, NULL );

    /*
    * Set memory allocator for graphics developer tools and NVN debug layer.
    * This function must be called before using any graphics developer features.
    */
    nv::SetGraphicsDevtoolsAllocator( NvDevtoolsAllocateFunction, NvDevtoolsFreeFunction, NvDevtoolsReallocateFunction, NULL );
    /*
    * Donate memory for graphics driver to work in.
    * This function must be called before using any graphics API's.
    */
    const size_t GraphicsSystemMemorySize = 8 * 1024 * 1024;
    void* graphicsHeap = aligned_alloc( 4096, GraphicsSystemMemorySize );
    nv::InitializeGraphics( graphicsHeap, GraphicsSystemMemorySize );

    /*
    * Initialize Video Interface (VI) system to display
    * to the target's screen
    */
    nn::vi::Initialize();

    nn::Result result = nn::vi::OpenDefaultDisplay( &g_pDisplay );
    NN_ASSERT( result.IsSuccess() );

    result = nn::vi::CreateLayer( &g_pLayer, g_pDisplay, MaxWidth, MaxHeight );
    NN_ASSERT( result.IsSuccess() );

    result = nn::vi::GetNativeWindow( &g_NativeWindowHandle, g_pLayer );
    NN_ASSERT( result.IsSuccess() );

    /*
    * Initialize EGL
    */
    EGLBoolean eglResult;
    NN_UNUSED(eglResult);

    g_Display = ::eglGetDisplay( EGL_DEFAULT_DISPLAY );
    NN_ASSERT( g_Display != NULL, "eglGetDisplay failed." );

    eglResult = ::eglInitialize( g_Display, 0, 0 );
    NN_ASSERT( eglResult, "eglInitialize failed." );

    EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES, 4,  // This is for 4x MSAA.
        EGL_NONE
    };
    EGLint numConfigs = 0;
    EGLConfig config;
    eglResult = ::eglChooseConfig( g_Display, configAttribs, &config, 1, &numConfigs );
    NN_ASSERT( eglResult && numConfigs == 1, "eglChooseConfig failed." );

    g_Surface = ::eglCreateWindowSurface( g_Display, config,
        static_cast< NativeWindowType >( g_NativeWindowHandle ), 0 );
    NN_ASSERT( g_Surface != EGL_NO_SURFACE, "eglCreateWindowSurface failed." );

    /*
    * Set the current rendering API.
    */
    eglResult = eglBindAPI( EGL_OPENGL_API );
    NN_ASSERT( eglResult, "eglBindAPI failed." );

    /*
    * Create new context and set it as current.
    */
    EGLint contextAttribs[] = {
        /* Set target garaphics api version. */
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 5,
        EGL_NONE
    };
    g_Context = ::eglCreateContext( g_Display, config, EGL_NO_CONTEXT, contextAttribs );
    NN_ASSERT( g_Context != EGL_NO_CONTEXT, "eglCreateContext failed. %d", eglGetError() );

    eglResult = ::eglMakeCurrent( g_Display, g_Surface, g_Surface, g_Context );
    NN_ASSERT( eglResult, "eglMakeCurrent failed." );

    eglResult = ::eglSwapInterval( g_Display, 0 );   // VSync OFF。フレームレートは Game::Process の TARGET_FPS で制御する
    NN_ASSERT( eglResult, "eglSwapInterval failed." );
}

void GraphicsHelper::Finalize()
{
    EGLBoolean eglResult;
    NN_UNUSED(eglResult);

    eglResult = ::eglMakeCurrent( g_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    NN_ASSERT( eglResult, "eglMakeCurrent failed." );
    eglResult = ::eglTerminate( g_Display );
    NN_ASSERT( eglResult, "eglTerminate failed." );
    eglResult = ::eglReleaseThread();
    NN_ASSERT( eglResult, "eglReleaseThread failed." );

    nn::vi::DestroyLayer( g_pLayer );
    nn::vi::CloseDisplay( g_pDisplay );
    nn::vi::Finalize();

    nv::FinalizeGraphics();
}

void GraphicsHelper::SwapBuffers()
{
    ::eglSwapBuffers( g_Display, g_Surface );
}

void GraphicsHelper::Resize(int width, int height)
{
    // Don't exceed max dimensions
    width = std::min(width, MaxWidth);
    height = std::min(height, MaxHeight);

    // This sample creates a 1080p EGLSurface through the arguments provided to
    // nn::vi::CreateLayer(). The subset of the EGLSurface used by the display is
    // adjusted with nn::vi::SetLayerCrop() to avoid recreation.

    // Origin for OpenGL is bottom left, so need to invert the y-axis
    nn::vi::SetLayerCrop(g_pLayer, 0, MaxHeight - height, width, height);

    glViewport(0, 0, width, height);
    glScissor(0, 0, width, height);
}
