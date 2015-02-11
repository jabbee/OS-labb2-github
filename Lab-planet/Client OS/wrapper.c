#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <strsafe.h>
#include "wrapper.h"

#define TIMERID			100  /* id for timer that is used by the thread that manages the window where graphics is drawn */
#define DEFAULT_STACK_SIZE	1024
#define TIME_OUT			MAILSLOT_WAIT_FOREVER 
typedef	struct smessage{
	char msg[256];

}smessage;
/* ATTENTION!!! calls that require a time out, use TIME_OUT constant, specifies that calls are blocked forever */


DWORD threadCreate (LPTHREAD_START_ROUTINE threadFunc, LPVOID threadParams) {
	
	DWORD ID;
		
	HANDLE thread = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		threadFunc,       // thread function name
		threadParams,          // argument to thread function 
		0,                      // use default creation flags 
		&ID);   // returns the thread identifier
	return ID;
	/* Creates a thread running threadFunc */
	/* optional parameters (NULL otherwise)and returns its id! */
}


HANDLE mailslotCreate (LPTSTR lpszSlotName) {

	/* Creates a mailslot with the specified name and returns the handle */
	/* Should be able to handle a messages of any size */
	HANDLE hSlot = CreateMailslot(lpszSlotName,
		0,                             // no maximum message size
		MAILSLOT_WAIT_FOREVER,         // no time-out for operations
		(LPSECURITY_ATTRIBUTES)NULL); // default security

	if (hSlot == INVALID_HANDLE_VALUE)
	{
		printf("CreateMailslot() failed with error code %d\n", GetLastError());
		return FALSE;
	}
	printf("CreateMailslot() is OK!\n");
	return hSlot;
}

HANDLE mailslotConnect (LPTSTR lpszSlotName, planet_type *msg) {

	/* Connects to an existing mailslot for writing */
	/* and returns the handle upon success     */
	HANDLE hFile;
	DWORD bytesWritten;
	hFile = CreateFile(lpszSlotName, GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// GetLastError() code - http://msdn.microsoft.com/en-us/library/ms679360(VS.85).aspx
		printf("CreateFile() failed with error code %d.\n", GetLastError());
		return FALSE;
	}
	else
		printf("CreateFile() is OK!\n");

	bytesWritten = mailslotWrite(hFile, msg, sizeof(planet_type));
	
	if (bytesWritten!=-1)
		printf("data sent to server (bytes = %d)\n", bytesWritten);
	else
		printf("failed sending data to server\n");


	if (CloseHandle(hFile) != 0)
		printf("hFile handle was closed successfully!\n");
	else
		printf("Fail to close hFile handle, error %d\n", GetLastError());
		
}

DWORD mailslotWrite(HANDLE mailSlot, planet_type* msg, int msgSize) {

	/* Write a msg to a mailslot, return nr */
	/* of successful bytes written         */
	BOOL fResult;
	DWORD cbWritten;

	fResult = WriteFile(mailSlot, msg, msgSize, &cbWritten, (LPOVERLAPPED)NULL);

	if (!fResult)
	{
		printf("WriteFile() failed with error code %d.\n", GetLastError());
		return FALSE;
	}
	else
	{
		printf("WriteFile() is OK!\n");
		printf("Slot written to successfully.\n");
	}

	return cbWritten;
}

int	mailslotRead (HANDLE mailbox, void *msg, int msgSize) {

	/* Read a msg from a mailslot, return nr */
	/* of successful bytes read              */
	
	DWORD cbMessage, cMessage, cbRead;
	BOOL fResult;
	LPTSTR lpszBuffer;
	TCHAR achID[80];
	DWORD cAllMessages;
	HANDLE hEvent;
	OVERLAPPED ov;

	cbMessage = cMessage = cbRead = 0;

	hEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("ExampleSlot"));
	if (NULL == hEvent)
		return FALSE;
	ov.Offset = 0;
	ov.OffsetHigh = 0;
	ov.hEvent = hEvent;

	fResult = GetMailslotInfo(mailbox, // mailslot handle
		(LPDWORD)NULL,               // no maximum message size
		&cbMessage,                          // size of next message
		&cMessage,                            // number of messages
		(LPDWORD)NULL);              // no read time-out

	if (!fResult)
	{
		printf("GetMailslotInfo() failed with error code %d.\n", GetLastError());
		return FALSE;
	}
	else
		printf("GetMailslotInfo() is fine!\n");

	if (cbMessage == MAILSLOT_NO_MESSAGE)
	{
		printf("Waiting for a message...\n");
		return TRUE;
	}

	cAllMessages = cMessage;

	while (cMessage != 0)  // retrieve all messages
	{
		// Create a message-number string
		StringCchPrintf((LPTSTR)achID, 80, TEXT("\nMessage #%d of %d\n"), cAllMessages - cMessage + 1, cAllMessages);
		// Allocate memory for the message
		lpszBuffer = (LPTSTR)GlobalAlloc(GPTR, lstrlen((LPTSTR)achID)*sizeof(TCHAR) + cbMessage);

		if (lpszBuffer == NULL)
			return FALSE;
		lpszBuffer[0] = '\0';

		

		fResult = ReadFile(mailbox, msg, msgSize, &cbRead, &ov);
		
		if (!fResult)
		{
			printf("ReadFile() failed with error code %d.\n", GetLastError());
			GlobalFree((HGLOBAL)lpszBuffer);
			return FALSE;
		}
		else
			printf("ReadFile() is OK!\n");

		// Concatenate the message and the message-number string
		StringCbCat(msg, lstrlen((LPTSTR)achID)*sizeof(TCHAR) + msgSize, (LPTSTR)achID);
		// Display the message
		_tprintf(TEXT("Contents of the mailslot: %s\n"), msg);

		GlobalFree((HGLOBAL)lpszBuffer);

		fResult = GetMailslotInfo(mailbox,  // mailslot handle
			(LPDWORD)NULL,               // no maximum message size
			&cbMessage,                          // size of next message
			&cMessage,                            // number of messages
			(LPDWORD)NULL);              // no read time-out

		if (!fResult)
		{
			printf("GetMailslotInfo() failed with error code %d\n", GetLastError());
			return FALSE;
		}
		else
			printf("GetMailslotInfo() is OK!\n");
	}

	if (CloseHandle(hEvent) != 0)
		printf("hEvent handle was closed successfully!\n");
	else
		printf("Fail to close hEvent handle, error %d\n", GetLastError());

	return cbRead;
}

int mailslotClose(HANDLE mailSlot){
	
	/* close a mailslot, returning whatever the service call returns */
}


/******************** Wrappers for window management, used for lab 2 and 3 ***********************/
/******************** DONT CHANGE!!! JUST FYI ******************************************************/


HWND windowCreate (HINSTANCE hPI, HINSTANCE hI, int ncs, char *title, WNDPROC callbackFunc, int bgcolor) {

  HWND hWnd;
  WNDCLASS wc; 

  /* initialize and create the presentation window        */
  /* NOTE: The only important thing to you is that we     */
  /*       associate the function 'MainWndProc' with this */
  /*       window class. This function will be called by  */
  /*       windows when something happens to the window.  */
  if( !hPI) {
	 wc.lpszClassName = "GenericAppClass";
	 wc.lpfnWndProc = callbackFunc;          /* (this function is called when the window receives an event) */
	 wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	 wc.hInstance = hI;
	 wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	 wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	 wc.hbrBackground = (HBRUSH) bgcolor;
	 wc.lpszMenuName = "GenericAppMenu";

	 wc.cbClsExtra = 0;
	 wc.cbWndExtra = 0;

	 RegisterClass( &wc );
  }

  /* NOTE: This creates a window instance. Don't bother about the    */
  /*       parameters to this function. It is sufficient to know     */
  /*       that this function creates a window in which we can draw. */
  hWnd = CreateWindow( "GenericAppClass",
				 title,
				 WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,
				 0,
				 0,
				 CW_USEDEFAULT,
				 CW_USEDEFAULT,
				 NULL,
				 NULL,
				 hI,
				 NULL
				 );

  /* NOTE: This makes our window visible. */
  ShowWindow( hWnd, ncs );
  /* (window creation complete) */

  return hWnd;
}

void windowRefreshTimer (HWND hWnd, int updateFreq) {

  if(SetTimer(hWnd, TIMERID, updateFreq, NULL) == 0) {
	 /* NOTE: Example of how to use MessageBoxes, see the online help for details. */
	 MessageBox(NULL, "Failed setting timer", "Error!!", MB_OK);
	 exit (1);
  }
}


/******************** Wrappers for window management, used for lab  3 ***********************/
/*****  Lab 3: Check in MSDN GetOpenFileName and GetSaveFileName  *********/
/**************  what the parameters mean, and what you must call this function with *********/


HANDLE OpenFileDialog(char* string, DWORD accessMode, DWORD howToCreate)
{

	OPENFILENAME opf;
	char szFileName[_MAX_PATH]="";

	opf.Flags				= OFN_SHOWHELP | OFN_OVERWRITEPROMPT; 
	opf.lpstrDefExt			= "dat";
	opf.lpstrCustomFilter	= NULL;
	opf.lStructSize			= sizeof(OPENFILENAME);
	opf.hwndOwner			= NULL;
	opf.lpstrFilter			= NULL;
	opf.lpstrFile			= szFileName;
	opf.nMaxFile			= _MAX_PATH;
	opf.nMaxFileTitle		= _MAX_FNAME;
	opf.lpstrInitialDir		= NULL;
	opf.lpstrTitle			= string;
	opf.lpstrFileTitle		= NULL ; 
	
	if(accessMode == GENERIC_READ)
		GetOpenFileName(&opf);
	else
		GetSaveFileName(&opf);

	return CreateFile(szFileName, 
		accessMode, 
		0, 
		NULL, 
		howToCreate, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);


}

