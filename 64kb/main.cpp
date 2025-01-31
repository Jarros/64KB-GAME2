#include "main.h"
#include "sound.h"
#include "bots.h"
#include "textures.h"
#include "input.h"
#include "hud.h"
#include "render.h"
#include "player.h"
#include "game.h"
#include "projectile.h"
#include "terrain.h"

Input* input = nullptr;

int rnd(int from, int to)
{
	return rand() % (1 + to - from) + from;
}

float rndf()
{
	return rand() % 64 / 64.0f;
}

void Message(char* message)
{
	MessageBox(NULL, (LPCSTR)message, (LPCSTR)"Error", MB_OK | MB_ICONSTOP);
}

float deltaTick = 1.0f;


BOOL	done = FALSE;
DWORD TICKWIN = 0;
DWORD TICKFRM = 0;
DWORD networkingTick = 0;

float Testx; float Testy; float Testz;
float ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
float FOV = 90.0f;
GLuint		texture[1];							// Storage For One Texture ( NEW )





float Dist2D(float x, float y, float x2, float y2)
{
	return sqrt(pow(x - x2, 2.0f) + pow(y - y2, 2.0f));
}
float Dist3D(float x, float y, float z, float x2, float y2, float z2)
{
	return sqrt(pow(x - x2, 2.0f) + pow(y - y2, 2.0f) + pow(z - z2, 2.0f));
}






bool	active = TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen = TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default










void LimitCoord(coord input, int min, int max)
{
	std::clamp((int)input, min, max);
}







Sound* sound;
BotManager* bots;
Textures* textures;
Terrain* terrain;
HUD* hud;
Game* game;
Render* render;
Player* player;
//Projectile* projectile;





MSG		msg;									// Windows Message Structure
static DWORD timer, DeltaT;
int t;

DWORD tickpre;
DWORD tickpost;





HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application
//bool	input->keys[256];			// Array Used For The Keyboard Routine

float	rtri;				// Angle For The Triangle ( NEW )
float	rquad;				// Angle For The Quad ( NEW )
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}
	glViewport(0, 0, width, height);						// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	// Calculate The Aspect Ratio Of The Window
	gluPerspective(FOV, ratio, nearclip, farclip);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}



void SwapBuffers()
{
	SwapBuffers(hDC);
}


//*************************
//INITGL START STARTGAME GO FIRST LABEL
//*************************
int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{

	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOV, ratio, nearclip, farclip);
	// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	return TRUE;										// Initialization Went OK
}

//GLubyte red[]    = { 255,   0,   0, 255 };
//GLubyte green[]  = {   0, 255,   0, 255 };
//GLubyte blue[]   = {   0,   0, 255, 255 };
//GLubyte white[]  = { 255, 255, 255, 255 };
//GLubyte yellow[] = {   0, 255, 255, 255 };
//GLubyte black[]  = {   0,   0,   0, 255 };
//GLubyte orange[] = { 255, 255,   0, 255 };
//GLubyte purple[] = { 255,   0, 255,   0 };







GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}
	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}
	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}
	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}
	if (!UnregisterClass("OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}
/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
*	title			- Title To Appear At The Top Of The Window				*
*	width			- Width Of The GL Window Or Fullscreen Mode				*
*	height			- Height Of The GL Window Or Fullscreen Mode			*
*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
*	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height
	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag
	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = "OpenGL";								// Set The Class Name

	input = new Input();
	
	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "JarGL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}
	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size
	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}
	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		4, 8, 16, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 1, 0,									// Accumulation Bits Ignored
		4,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 4										// Layer Masks Ignored
	};
	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}
	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}
	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}
	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}
	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}
	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen



	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}


	return TRUE;									// Success
}
LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
	case WM_ACTIVATE:							// Watch For Window Activate Message
	{
		// LoWord Can Be WA_INACTIVE, WA_ACTIVE, WA_CLICKACTIVE,
		// The High-Order Word Specifies The Minimized State Of The Window Being Activated Or Deactivated.
		// A NonZero Value Indicates The Window Is Minimized.
		if ((LOWORD(wParam) != WA_INACTIVE) && !((BOOL)HIWORD(wParam)))
		{
			active = TRUE;						// Program Is Active
			input->windowFocused = true;
		}
		else
		{
			active = FALSE;						// Program Is No Longer Active
			input->windowFocused = false;
		}
		return 0;								// Return To The Message Loop
	}
	case WM_SYSCOMMAND:							// Intercept System Commands
	{
		switch (wParam)							// Check System Calls
		{
		case SC_SCREENSAVE:					// Screensaver Trying To Start?
		case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
			return 0;							// Prevent From Happening
		}
		break;									// Exit
	}
	case WM_CLOSE:								// Did We Receive A Close Message?
	{
		PostQuitMessage(0);						// Send A Quit Message
		return 0;								// Jump Back
	}
	case WM_KEYDOWN:							// Is A Key Being Held Down?
	{
		input->keys[wParam].press = TRUE;					// If So, Mark It As TRUE
		input->keys[wParam].Hit = TRUE;
		return 0;								// Jump Back
	}
	case WM_LBUTTONDOWN:
	{
		input->LMB.press = TRUE;
		input->LMB.Hit = TRUE;
		return 0;
	}
	case WM_LBUTTONUP:
	{
		input->LMB.press = FALSE;
		input->LMB.Rel = TRUE;
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		input->RMB.press = TRUE;
		input->RMB.Hit = TRUE;
		return 0;
	}
	case WM_RBUTTONUP:
	{
		input->RMB.press = FALSE;
		input->RMB.Rel = TRUE;
		return 0;
	}
	//case MK_MBUTTON:
	//{
	//	 MMB.press = TRUE;
	//	 MMB.Hit = TRUE;
	//}
	//case MK_MBUTTON:
	//{
	//	 MMB.press = FALSE;
	//	 MMB.Rel = TRUE;
	//}
	case WM_KEYUP:								// Has A Key Been Released?
	{
		input->keys[wParam].press = FALSE;					// If So, Mark It As FALSE
		input->keys[wParam].Rel = TRUE;
		return 0;								// Jump Back
	}
	case WM_SIZE:								// Resize The OpenGL Window
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
		return 0;								// Jump Back
	}
	}
	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void init() {


	sound = new Sound();
	bots = new BotManager();
	textures = new Textures();
	terrain = new Terrain();
	hud = new HUD();
	game = new Game(hud, input, terrain);
	render = new Render();
	player = new Player();
	//projectile = new Projectile();
	terrain->bindReferences(bots, game, player);

	sound->InitWave();

	terrain->BuildScene(true, true);
	//terrain->Fractal();

	game->mode = GameModes::menu;//game;
	game->Menu = true;

	//player->xpos = landsizeh;
	//player->zpos = landsizeh;
	player->ypos = terrain->scene[(int)player->xpos][(int)player->zpos].h + 32;
	player->yrot = 0;

	render->DrawCube = true;
	//terrain->BuildScene(false, false);
	//	setup_opengl( width, height );
	textures->Build();

}

void mainLoop() {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		err = err;
	}


	TICKWIN = GetTickCount();
	TICKFRM--;
	if (GetTickCount() - timer > 1486 - DeltaT)
	{
		timer = GetTickCount();
		t++;

		sound->Proc();
	}
	DeltaT = GetTickCount() - timer;

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
	{
		if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
		{
			done = TRUE;							// If So done=TRUE
		}
		else									// If Not, Deal With Window Messages
		{
			TranslateMessage(&msg);				// Translate The Message
			DispatchMessage(&msg);				// Dispatch The Message
		}
	}
	else										// If There Are No Messages
	{


		if (input->keys[VK_ESCAPE].Hit)	// Active?  Was There A Quit Received?  && !DrawGLScene()     active
		{{

				if (game->Cheat)done = TRUE;							// ESC or DrawGLScene Signalled A Quit
				else
					hud->SwitchMenu(*game);
			}
		}

		tickpre = GetTickCount();

		render->Cycle(*terrain, *player, *textures, *game, *hud, *sound, *bots, *input);
		bots->Process(*terrain, *game, *player, *sound);
		//bots->Process(*terrain, *game, *player, *sound);






		input->Check(*bots, *sound, *terrain, *player, *hud, *game);
		input->ClearKeys();
		//}			
		//DWORD tickwhile2=GetTickCount();


		if (true && simulateFPSdrops)
			Sleep(rand() % 512);


		tickpost = GetTickCount();

		int rendertime = tickpost - tickpre;


		if (rendertime <= DELTA)
		{
			Sleep(DELTA - rendertime);
			deltaTick = 1.0f;
		}
		else
		{
			deltaTick = float(rendertime) / float(DELTA);

		}
	}
	if (input->keys[VK_F1].press)						// Is F1 Being Pressed?
	{
		input->keys[VK_F1].press = FALSE;					// If So Make Key FALSE
		KillGLWindow();						// Kill Our Current Window
		fullscreen = !fullscreen;				// Toggle Fullscreen / Windowed Mode
		// Recreate Our OpenGL Window
		if (!CreateGLWindow("NeHe's Solid Object Tutorial", SCREEN_W, SCREEN_H, 16, fullscreen))
		{
			return;						// Quit If Window Was Not Created
		}
	}

}



int WINAPI WinMain(HINSTANCE	hInstance,			// Instance
	HINSTANCE	hPrevInstance,		// Previous Instance
	LPSTR		lpCmdLine,			// Command PrintLine Parameters
	int			nCmdShow)			// Window Show State
{
	//BOOL	done=FALSE;								// Bool Variable To Exit Loop

	tickpre = GetTickCount();
	tickpost = GetTickCount();

	bool SKIPWAITING = false;
	// Ask The User Which Screen Mode They Prefer
	if (false)
		if (MessageBox(NULL, "Would You Like To Run In Fullscreen Mode?", "64kb KONKYRZ IGOR", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			fullscreen = FALSE;							// Windowed Mode
		}
	//fullscreen = FALSE;
	// Create Our OpenGL Window
	if (!CreateGLWindow("64kb", fullscreen ? SCREEN_W : SCREEN_W/2, fullscreen ? SCREEN_H : SCREEN_H/2, 32, fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	//DWORD tickwhile;

	init();

	while (!done)									// Loop That Runs While done=FALSE
	{
		mainLoop();


	}
	sound->Close();
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}
