#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>
#include<GL/glew.h>
#include<GL/glx.h>

#include<SOIL/SOIL.h>

#include"include/fpslock.h"
#include"include/render.h"

bool doDisplayFPS = false;
struct winParam winSize;

void ToggleFullscreen(Display* gpDisplay, XVisualInfo* gpXVisualInfo, Window gWindow) {
	static bool dl_bFullscreen = false;
	
	Atom dl_wm_state;
	Atom dl_fullscreen;
	XEvent dl_xev = {0};

	dl_wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
	memset(&dl_xev, 0, sizeof(dl_xev));

	dl_xev.type = ClientMessage;
	dl_xev.xclient.window = gWindow;
	dl_xev.xclient.message_type = dl_wm_state;
	dl_xev.xclient.format = 32;
	dl_xev.xclient.data.l[0] = dl_bFullscreen ? 0 : 1;
	dl_fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	dl_xev.xclient.data.l[1] = dl_fullscreen;
	XSendEvent(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen), False, StructureNotifyMask, &dl_xev);

	dl_bFullscreen = !dl_bFullscreen;
}

int main(void) {
	bool dl_bDone = false;

		static int dl_frameBufferAttrib[] = {
		GLX_DOUBLEBUFFER, True,
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_STENCIL_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		None 
	};
	XSetWindowAttributes dl_winAttribs;
	GLXFBConfig *pGlxFBConfig;
	XVisualInfo *pTempVisInfo = NULL;
	int numFBConfigs;
	int dl_defaultScreen;
	int dl_styleMask;
	
	Display* dl_gpDisplay = XOpenDisplay(NULL);
	dl_defaultScreen = XDefaultScreen(dl_gpDisplay);
	XVisualInfo* dl_gpXVisualInfo = (XVisualInfo *)malloc(sizeof(XVisualInfo));

	pGlxFBConfig = glXChooseFBConfig(dl_gpDisplay, dl_defaultScreen, dl_frameBufferAttrib, &numFBConfigs);
	int bestFrameBufferConfig = -1, worstFrameBufferConfig = -1, bestNumOfSamples = -1, worstNumOfSamples = 999;
	for(int i = 0; i < numFBConfigs; i++) {
		pTempVisInfo = glXGetVisualFromFBConfig(dl_gpDisplay, pGlxFBConfig[i]);
		if(pTempVisInfo != NULL) {
			int samplesBuffer, samples;
			glXGetFBConfigAttrib(dl_gpDisplay, pGlxFBConfig[i], GLX_SAMPLE_BUFFERS, &samplesBuffer);
			glXGetFBConfigAttrib(dl_gpDisplay, pGlxFBConfig[i], GLX_SAMPLES, &samples);
			if(bestFrameBufferConfig < 0 || samplesBuffer && samples > bestNumOfSamples) {
				bestFrameBufferConfig = i;
				bestNumOfSamples = samples;
			} else if(worstFrameBufferConfig < 0 || !samplesBuffer || samplesBuffer < worstNumOfSamples) {
				worstFrameBufferConfig = i;
				worstNumOfSamples = samples;
			}
		}
		XFree(pTempVisInfo);
	}

	GLXFBConfig gGLXFBconfig = pGlxFBConfig[bestFrameBufferConfig];
	XFree(pGlxFBConfig);

	dl_gpXVisualInfo = glXGetVisualFromFBConfig(dl_gpDisplay, gGLXFBconfig);

	dl_winAttribs.border_pixel = 0;
	dl_winAttribs.background_pixmap = 0;
	dl_winAttribs.colormap = XCreateColormap(dl_gpDisplay, RootWindow(dl_gpDisplay, dl_gpXVisualInfo->screen), dl_gpXVisualInfo->visual, AllocNone);
	Colormap dl_gColormap = dl_winAttribs.colormap;
	dl_winAttribs.background_pixel = BlackPixel(dl_gpDisplay, dl_defaultScreen);
	dl_winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | Button1MotionMask | StructureNotifyMask;
	dl_styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

	Window dl_gWindow = XCreateWindow(dl_gpDisplay, RootWindow(dl_gpDisplay, dl_gpXVisualInfo->screen), 0, 0, 800, 600, 0, dl_gpXVisualInfo->depth, InputOutput, dl_gpXVisualInfo->visual, dl_styleMask, &dl_winAttribs);

	XStoreName(dl_gpDisplay, dl_gWindow, "OpenGL Programmable Pipeline");

	Atom dl_windowManagerDelete = XInternAtom(dl_gpDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(dl_gpDisplay, dl_gWindow, &dl_windowManagerDelete, 1);

	XMapWindow(dl_gpDisplay, dl_gWindow);
	
	typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	glXCreateContextAttribsARBProc glXCreateContextAttribARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
	
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 6,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	GLXContext dl_gGlxContex =  glXCreateContextAttribARB(dl_gpDisplay, gGLXFBconfig, 0, True, attribs);

	if(!dl_gGlxContex) {
		const int attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		};
		dl_gGlxContex =  glXCreateContextAttribARB(dl_gpDisplay, gGLXFBconfig, 0, True, attribs);
	}
	glXMakeCurrent(dl_gpDisplay, dl_gWindow, dl_gGlxContex);
	glewInit();
	initFPSLock();
	setDesiredFPS(125);
	initGL();
	
	XEvent dl_event;
	KeySym dl_keysym;
	XEvent ev;

	memset(&ev, 0, sizeof (ev));

	ev.xclient.type = ClientMessage;
	ev.xclient.window = dl_gWindow;
	ev.xclient.message_type = XInternAtom(dl_gpDisplay, "WM_PROTOCOLS", true);
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom(dl_gpDisplay, "WM_DELETE_WINDOW", false);
	ev.xclient.data.l[1] = CurrentTime;
	
	ToggleFullscreen(dl_gpDisplay, dl_gpXVisualInfo, dl_gWindow);

	while(!dl_bDone) {
		while(XPending(dl_gpDisplay)) {
			XNextEvent(dl_gpDisplay, &dl_event);
			switch(dl_event.type) {
			case KeyPress:
				dl_keysym = XkbKeycodeToKeysym(dl_gpDisplay, dl_event.xkey.keycode, 0, 0);
				keyboardGL(dl_keysym);
				switch(dl_keysym) {
				case XK_Escape:
					XSendEvent(dl_gpDisplay, dl_gWindow, False, NoEventMask, &ev);
					break;
				case XK_F: case XK_f:
					ToggleFullscreen(dl_gpDisplay, dl_gpXVisualInfo, dl_gWindow);
					break;
				default:
					break;
				}
				break;
			case ButtonPress:
				if(dl_event.xbutton.button == Button1) {
					mousemoveGL(dl_event.xmotion.x, dl_event.xmotion.y, false);
				}
				break;
			case ConfigureNotify:
				winSize.w = (float)dl_event.xconfigure.width;
				winSize.h = (float)dl_event.xconfigure.height;
				break;
			case MotionNotify:
				mousemoveGL(dl_event.xmotion.x, dl_event.xmotion.y, true);
				break;
			case ClientMessage:
				uninitGL();
				if(dl_gGlxContex) {
					glXDestroyContext(dl_gpDisplay, dl_gGlxContex);
				}

				if(dl_gWindow) {
					XDestroyWindow(dl_gpDisplay, dl_gWindow);
				}

				if(dl_gColormap) {
					XFreeColormap(dl_gpDisplay, dl_gColormap);
				}

				if(dl_gpXVisualInfo) {
					free(dl_gpXVisualInfo);
					dl_gpXVisualInfo = NULL;
				}

				if(dl_gpDisplay) {
					XCloseDisplay(dl_gpDisplay);
					dl_gpDisplay = NULL;
				}
				return 0;
				break;
			default:
				break;
			}
		}
		renderWithFPSLocked(renderGL, doDisplayFPS);
		glXSwapBuffers(dl_gpDisplay, dl_gWindow);
	}
	return 0;
}