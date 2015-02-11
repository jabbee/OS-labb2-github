/********************************************************************\
* server.c                                                           *
*                                                                    *
* Author: mle@mdh.se                                                 *
* Desc: example of the server-side of an application                 *
* Date: 990310                                                       *
* Revised: 011018 Dag Nyström & Jukka Mäki-Turja                     *
*                                                                    *
* Based on generic.c from Microsoft.                                 *
*                                                                    *
*  Functions:                                                        *
*     WinMain      - Application entry point                         *
*     MainWndProc  - main window procedure                           *
*                                                                    *
* NOTE: this program uses some graphic primitives provided by Win32, *
* therefore there are probably a lot of things that are unfamiliar   *
* to you. There are comments in this file that indicates where it is *
* appropriate to place your code.                                    *
* NOTE: you will stumble into some problems if you run this program  *
*       in Windows 95/98. don't do that, use NT instead.             *
\********************************************************************/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "wrapper.h"

/* the server uses a timer to periodically update the presentation window */
/* here is the timer id and timer period defined                          */

#define UPDATE_FREQ     100	/* update frequency (in ms) for the timer */

/* (the server uses a mailslot for incoming client requests) */



/*********************  Prototypes  ***************************/
/* NOTE: Windows has defined its own set of types. When the   */
/*       types are of importance to you we will write comments*/ 
/*       to indicate that. (Ignore them for now.)             */
/**************************************************************/

LRESULT WINAPI MainWndProc( HWND, UINT, WPARAM, LPARAM );
DWORD WINAPI mailThread(LPVOID);
DWORD WINAPI threadFunction(LPVOID arg);
DWORD Threads[10] = { NULL };
planet_type *root = NULL;
planet_type *planetPointer=NULL;
CRITICAL_SECTION critical_section;

HDC hDC;		/* Handle to Device Context, gets set 1st time in MainWndProc */
/* we need it to access the window for printing and drawin */

/********************************************************************\
*  Function: int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)    *
*                                                                    *
*   Purpose: Initializes Application                                 *
*                                                                    *
*  Comments: Register window class, create and display the main      *
*            window, and enter message loop.                         *
*                                                                    *
*                                                                    *
\********************************************************************/

/* NOTE: This function is not too important to you, it only */
/*       initializes a bunch of things.                     */
/* NOTE: In windows WinMain is the start function, not main */

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow ) {

	HWND hWnd;
	DWORD threadID;
	MSG msg;
	if (!InitializeCriticalSectionAndSpinCount(&critical_section,
		0x00000400))
		return;


	//root = malloc(sizeof(planet_type));

	/* Create the window, 3 last parameters important */
	/* The tile of the window, the callback function */
	/* and the backgrond color */

	hWnd = windowCreate (hPrevInstance, hInstance, nCmdShow, "Himmel", MainWndProc, COLOR_WINDOW+1);

	/* start the timer for the periodic update of the window    */
	/* (this is a one-shot timer, which means that it has to be */
	/* re-set after each time-out) */
	/* NOTE: When this timer expires a message will be sent to  */
	/*       our callback function (MainWndProc).               */

	windowRefreshTimer (hWnd, UPDATE_FREQ);


	/* create a thread that can handle incoming client requests */
	/* (the thread starts executing in the function mailThread) */
	/* NOTE: See online help for details, you need to know how  */ 
	/*       this function does and what its parameters mean.   */
	/* We have no parameters to pass, hence NULL				*/

	//EnterCriticalSection(&critical_section);
	threadID = threadCreate (mailThread, NULL); 


	/* (the message processing loop that all windows applications must have) */
	/* NOTE: just leave it as it is. */
	while( GetMessage( &msg, NULL, 0, 0 ) ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	DeleteCriticalSection(&critical_section);
	return msg.wParam;
}


/********************************************************************\
* Function: mailThread                                               *
* Purpose: Handle incoming requests from clients                     *
* NOTE: This function is important to you.                           *
/********************************************************************/
DWORD WINAPI mailThread(LPVOID arg) {

	char buffer[1024];
	static int i;
	DWORD bytesRead;
	static int posY = 0;
	HANDLE mailbox;
	planet_type ListPt = { NULL };

	//planetPointer = malloc(sizeof(planet_type));

	/* create a mailslot that clients can use to pass requests through   */
	/* (the clients use the name below to get contact with the mailslot) */
	/* NOTE: The name of a mailslot must start with "\\\\.\\mailslot\\"  */


	mailbox = mailslotCreate ("\\\\.\\mailslot\\serverSlot");
	MessageBox(NULL, "Mailslot create OK", "ops", MB_OK);


	for(;;) {				
		/* (ordinary file manipulating functions are used to read from mailslots) */
		/* in this example the server receives strings from the client side and   */
		/* displays them in the presentation window                               */
		/* NOTE: binary data can also be sent and received, e.g. planet structures*/
		//EnterCriticalSection(&critical_section);
		bytesRead = mailslotRead(mailbox, &ListPt, sizeof(planet_type)); 


		if(bytesRead!= 0) {
			EnterCriticalSection(&critical_section);
			/* NOTE: It is appropriate to replace this code with something */
			if(i==0)
			{
				root = &ListPt;
			}






			planetPointer = malloc(sizeof(planet_type));

			planetPointer = &ListPt;

			Threads[i] = threadCreate(threadFunction,planetPointer);
			planetPointer->next = NULL;
			planetPointer = planetPointer->next;

			i++;

			Sleep(10);
			LeaveCriticalSection(&critical_section);
		}

		else {
			/* failed reading from mailslot                              */
			/* (in this example we ignore this, and happily continue...) */
		}

	}



	return 0;
}

DWORD WINAPI threadFunction(LPVOID arg)
{
	int i=0;
	planet_type *ptr = arg;
	while(1)
	{



		while(ptr!=NULL)
		{

		ptr->sx = ptr->sx + ptr->vx;
		ptr->sy = ptr->sy + ptr->vy;
		Sleep(10);
		}

		/*

		ptr = root;
		Sleep(10);*/

	}

}


/********************************************************************\
* Function: LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM) *
*                                                                    *
* Purpose: Processes Application Messages (received by the window)   *
* Comments: The following messages are processed                     *
*                                                                    *
*           WM_PAINT                                                 *
*           WM_COMMAND                                               *
*           WM_DESTROY                                               *
*           WM_TIMER                                                 *
*                                                                    *
\********************************************************************/
/* NOTE: This function is called by Windows when something happens to our window */

LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	PAINTSTRUCT ps;
	static int posX = 10;
	int posY;
	HANDLE context;
	static DWORD color = 0;

	planet_type* increaser ;

	increaser=root;

	switch( msg ) {
		/**************************************************************/
		/*    WM_CREATE:        (received on window creation)
		/**************************************************************/
	case WM_CREATE:       
		hDC = GetDC(hWnd);  
		break;   
		/**************************************************************/
		/*    WM_TIMER:         (received when our timer expires)
		/**************************************************************/
	case WM_TIMER:

		/* NOTE: replace code below for periodic update of the window */
		/*       e.g. draw a planet system)                           */
		/* NOTE: this is referred to as the 'graphics' thread in the lab spec. */

		/* here we draw a simple sinus curve in the window    */
		/* just to show how pixels are drawn                  */
		//EnterCriticalSection(&critical_section);

		while (increaser != NULL)
		{

			SetPixel(hDC, increaser->sx, increaser->sy, (COLORREF)12);

			increaser = increaser->next;
			Sleep(10);
		}

		increaser = root;

		windowRefreshTimer(hWnd, UPDATE_FREQ);
		//LeaveCriticalSection(&critical_section);


		//Sleep(2);

		//posX += 4;
		//posY = (int) (10 * sin(posX / (double) 30) + 20);
		//SetPixel(hDC, posX % 547, posY, (COLORREF) color);
		//color += 12;
		//windowRefreshTimer (hWnd, UPDATE_FREQ);
		break;
		/****************************************************************\
		*     WM_PAINT: (received when the window needs to be repainted, *
		*               e.g. when maximizing the window)                 *
		\****************************************************************/

	case WM_PAINT:
		/* NOTE: The code for this message can be removed. It's just */
		/*       for showing something in the window.                */
		context = BeginPaint( hWnd, &ps ); /* (you can safely remove the following line of code) */
		/*TextOut( context, 10, 10, "Hello, World!", 13 ); // 13 is the string length 
		EndPaint( hWnd, &ps );*/
		break;
		/**************************************************************\
		*     WM_DESTROY: PostQuitMessage() is called                  *
		*     (received when the user presses the "quit" button in the *
		*      window)                                                 *
		\**************************************************************/
	case WM_DESTROY:
		PostQuitMessage( 0 );
		/* NOTE: Windows will automatically release most resources this */
		/*       process is using, e.g. memory and mailslots.           */
		/*       (So even though we don't free the memory which has been*/     
		/*       allocated by us, there will not be memory leaks.)      */

		ReleaseDC(hWnd, hDC); /* Some housekeeping */
		break;

		/**************************************************************\
		*     Let the default window proc handle all other messages    *
		\**************************************************************/
	default:
		return( DefWindowProc( hWnd, msg, wParam, lParam )); 

	}
	return 0;
}




