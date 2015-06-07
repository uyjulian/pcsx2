/*
 *	Copyright (C) 2007-2012 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

// TODO OSX diff this file against GSWndOGL and remove commented/not used junk

//#include <wx/wx.h>

#include "stdafx.h"

#define GL_CTX_MAJOR 3
#define GL_CTX_MINOR 3

#include <cstdio>
#include "GSWndSDLGL.h"

#if defined(__POSIX__) && !defined(ENABLE_GLES)
GSWndOGL::GSWndOGL()
    : m_NativeWindow(0), m_NativeDisplay(NULL), m_SDLInitialized(false)
{

}

static bool sdl_last_error(int line = -1)
{
    const char *error = SDL_GetError();
    if (*error != '\0')
    {
        fprintf(stderr, "SDL Error: %s\n", error);
        if (line != -1)
            fprintf(stderr, " - %s + line: %i\n", __FILE__, line);
        SDL_ClearError();
        return true;
    }
    return false;
}

void GSWndOGL::SDLInit()
{
    if (m_SDLInitialized)
    {
        // No need to redo
        return;
    }
    else if (SDL_Init(0) != 0)
    {
        sdl_last_error(__LINE__);
        throw GSDXRecoverableError();
    }
    //if (SDL_Init(SDL_INIT_VIDEO) < 0)
    if (SDL_VideoInit(NULL) != 0)
    {
        sdl_last_error(__LINE__);
        throw GSDXRecoverableError();
    }
    m_SDLInitialized = true;
}

void GSWndOGL::CreateContext(int major, int minor)
{
	if ( !m_NativeWindow )
	{
		fprintf( stderr, "Wrong X11 display/window\n" );
		throw GSDXRecoverableError();
	}

    /*PFNGLXCHOOSEFBCONFIGPROC glX_ChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC) glXGetProcAddress((GLubyte *) "glXChooseFBConfig");
	int fbcount = 0;
	GLXFBConfig *fbc = glX_ChooseFBConfig(m_NativeDisplay, DefaultScreen(m_NativeDisplay), attrListDbl, &fbcount);
	if (!fbc || fbcount < 1) {
		throw GSDXRecoverableError();
	}

	PFNGLXCREATECONTEXTATTRIBSARBPROC glX_CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte*) "glXCreateContextAttribsARB");
	if (!glX_CreateContextAttribsARB) {
		throw GSDXRecoverableError();
    }*/

	// Install a dummy handler to handle gracefully (aka not segfault) the support of GL version
    //int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);
	// Be sure the handler is installed
    //XSync( m_NativeDisplay, false);

	// Create a context
    /*int context_attribs[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, major,
		GLX_CONTEXT_MINOR_VERSION_ARB, minor,
#ifdef ENABLE_OGL_DEBUG
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
    };*/

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
    //SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, True);
    m_context = SDL_GL_CreateContext(m_NativeWindow);

    //m_context = glX_CreateContextAttribsARB(m_NativeDisplay, fbc[0], 0, true, context_attribs);
    //XFree(fbc);

	// Don't forget to reinstall the older Handler
    //XSetErrorHandler(oldHandler);

	// Get latest error
    //XSync( m_NativeDisplay, false);

    if (sdl_last_error(__LINE__)) {
		fprintf(stderr, "Failed to create the opengl context. Check your drivers support openGL %d.%d. Hint: opensource drivers don't\n", major, minor );
		throw GSDXRecoverableError();
	}
}

void GSWndOGL::AttachContext()
{
	if (!IsContextAttached()) {
		//fprintf(stderr, "Attach the context\n");
        SDL_GL_MakeCurrent(m_NativeWindow, m_context);
		m_ctx_attached = true;
	}
}

void GSWndOGL::DetachContext()
{
	if (IsContextAttached()) {
		//fprintf(stderr, "Detach the context\n");
        SDL_GL_MakeCurrent(m_NativeWindow, NULL);
		m_ctx_attached = false;
	}
}

void GSWndOGL::CheckContext()
{
    /*int glxMajorVersion, glxMinorVersion;
    glXQueryVersion(m_NativeDisplay, &glxMajorVersion, &glxMinorVersion);
    if (glXIsDirect(m_NativeDisplay, m_context))
        fprintf(stderr, "glX-Version %d.%d with Direct Rendering\n", glxMajorVersion, glxMinorVersion);
    else {
        fprintf(stderr, "glX-Version %d.%d with Indirect Rendering !!! It won't support properly opengl\n", glxMajorVersion, glxMinorVersion);
        throw GSDXRecoverableError();
    }*/

    const GLubyte* verstring = glGetString(GL_VERSION);
    int sdlGLMajorVersion=0, sdlGLMinorVersion=0;

    /*if (version != NULL)
            if ( sscanf( version, "%d.%d", major, minor ) == 2 )
                return GL_TRUE;

        printf( "<!>Invalid GL_VERSION format\n" );
        return GL_FALSE;*/

    if ((verstring == NULL)
            || (sscanf((const char*)verstring, "%d.%d", &sdlGLMajorVersion, &sdlGLMinorVersion) != 2)
            || (sdlGLMajorVersion <= GL_CTX_MAJOR && sdlGLMinorVersion < GL_CTX_MINOR)
            )
    {
        fprintf(stderr, "WRONG SDL GL (%s) Version %d.%d\n", verstring, sdlGLMajorVersion, sdlGLMinorVersion);
        //throw GSDXRecoverableError();
    }
    fprintf(stderr, "SDL GL (%s) Version %d.%d\n", verstring, sdlGLMajorVersion, sdlGLMinorVersion);
}

bool GSWndOGL::Attach(void* handle, bool managed)
{
    //m_NativeWindow = (SDL_Window*)handle;
    //m_NativeWindow = SDL_CreateWindowFrom(handle);

    //sdl_last_error(__LINE__);

	m_managed = managed;

    //m_NativeDisplay = XOpenDisplay(NULL);
    SDLInit();

    // TODO OSX lol I am a hack
    m_NativeWindow = SDL_CreateWindow("title.c_str()", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (m_NativeWindow == 0)
        throw GSDXRecoverableError();

    CreateContext(GL_CTX_MAJOR, GL_CTX_MINOR);

	AttachContext();

	CheckContext();

    //m_swapinterval = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*) "glXSwapIntervalEXT");

	PopulateGlFunction();

	return true;
}

void GSWndOGL::Detach()
{
	// Actually the destructor is not called when there is only a GSclose/GSshutdown
	// The window still need to be closed
	DetachContext();
    SDL_GL_DeleteContext(m_context);

	if (m_NativeDisplay) {
        delete m_NativeDisplay;
		m_NativeDisplay = NULL;
	}
}

bool GSWndOGL::Create(const string& title, int w, int h)
{
	if(m_NativeWindow)
		throw GSDXRecoverableError();

	if(w <= 0 || h <= 0) {
		w = theApp.GetConfig("ModeWidth", 640);
		h = theApp.GetConfig("ModeHeight", 480);
	}

	m_managed = true;

	// note this part must be only executed when replaying .gs debug file
    //m_NativeDisplay = XOpenDisplay(NULL);

    //m_NativeWindow = XCreateSimpleWindow(m_NativeDisplay, DefaultRootWindow(m_NativeDisplay), 0, 0, w, h, 0, 0, 0);
    //XMapWindow (m_NativeDisplay, m_NativeWindow);

    SDLInit();

	m_NativeWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (m_NativeWindow == 0)
		throw GSDXRecoverableError();

    CreateContext(GL_CTX_MAJOR, GL_CTX_MINOR);

	AttachContext();

	CheckContext();

	//m_swapinterval = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*) "glXSwapIntervalEXT");

	PopulateGlFunction();

	return true;
}

void* GSWndOGL::GetProcAddress(const char* name, bool opt)
{
    void* ptr = NULL; //(void*)glXGetProcAddress((const GLubyte*)name);
	if (!ptr) ptr = &gl_CopyImageSubData;
	if (ptr == NULL) {
		fprintf(stderr, "Failed to find %s\n", name);
		if (!opt)
			throw GSDXRecoverableError();
	}
	return ptr;
}

void* GSWndOGL::GetDisplay()
{
	// note this part must be only executed when replaying .gs debug file
	return (void*)m_NativeDisplay;
}

GSVector4i GSWndOGL::GetClientRect()
{
    int h = 480;
    int w = 640;

    /*unsigned int borderDummy;
	unsigned int depthDummy;
	Window winDummy;
    int xDummy;
    int yDummy;*/

    //if (!m_NativeDisplay) m_NativeDisplay = XOpenDisplay(NULL);
    //XGetGeometry(m_NativeDisplay, m_NativeWindow, &winDummy, &xDummy, &yDummy, &w, &h, &borderDummy, &depthDummy);
    SDL_GL_GetDrawableSize(m_NativeWindow, &w, &h);

	return GSVector4i(0, 0, (int)w, (int)h);
}

// Returns FALSE if the window has no title, or if th window title is under the strict
// management of the emulator.

bool GSWndOGL::SetWindowText(const char* title)
{
	if (!m_managed) return true;

    /*XTextProperty prop;

	memset(&prop, 0, sizeof(prop));

	char* ptitle = (char*)title;
	if (XStringListToTextProperty(&ptitle, 1, &prop)) {
		XSetWMName(m_NativeDisplay, m_NativeWindow, &prop);
	}

	XFree(prop.value);
    XFlush(m_NativeDisplay);*/

	return true;
}

void GSWndOGL::SetVSync(bool enable)
{
	// m_swapinterval uses an integer as parameter
	// 0 -> disable vsync
	// n -> wait n frame
    //if (m_swapinterval) m_swapinterval(m_NativeDisplay, m_NativeWindow, (int)enable);
    SDL_GL_SetSwapInterval((int)enable);
}

void GSWndOGL::Flip()
{
    SDL_GL_SwapWindow(m_NativeWindow);
}

void GSWndOGL::Show()
{
    //XMapRaised(m_NativeDisplay, m_NativeWindow);
    //XFlush(m_NativeDisplay);
}

void GSWndOGL::Hide()
{
    //XUnmapWindow(m_NativeDisplay, m_NativeWindow);
    //XFlush(m_NativeDisplay);
}

void GSWndOGL::HideFrame()
{
	// TODO
}

#endif
