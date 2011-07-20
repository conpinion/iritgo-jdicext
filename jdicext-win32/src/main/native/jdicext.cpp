#include <windows.h>
#include <jni.h>
#include "de_iritgo_jdicext_JDICExt.h"
#include "pipe.h"


#ifdef __cplusplus
extern "C" {
#endif

	
#define PIPE_MSG_KEYDOWN 0x01
#define PIPE_MSG_KEYUP 0x02


HHOOK keyboardHook = NULL;
HANDLE hookThread = NULL;
JavaVM * javaVM = NULL;
HINSTANCE dllInstance = NULL;
bool hookThreadRunning = true;
Pipe pipe;
jobject jdicExtInstance = NULL;
jmethodID globalKeyPressedMethod = NULL;
jmethodID globalKeyReleasedMethod = NULL;


BOOL WINAPI DllMain (HMODULE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
			dllInstance = (HINSTANCE) instance;
			break;

		case DLL_PROCESS_DETACH:
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
    }

    return TRUE;
}


bool isModifierKey (DWORD keyCode)
{
	return 	
		keyCode == VK_SHIFT ||
		keyCode == VK_LSHIFT ||
		keyCode == VK_RSHIFT ||
		keyCode == VK_CONTROL ||
		keyCode == VK_LCONTROL ||
		keyCode == VK_RCONTROL ||
		keyCode == VK_LMENU ||
		keyCode == VK_RMENU;
}


UINT calculateModifiers ()
{
	UINT modifiers = 0;
	if (GetAsyncKeyState (VK_SHIFT))
	{
		modifiers |= 1;
	}
	if (GetAsyncKeyState (VK_CONTROL))
	{
		modifiers |= 2;
	}
	if (GetAsyncKeyState (VK_MENU))
	{
		modifiers |= 4;
	}
	return modifiers;
}


UINT calculateKeyChar (UINT keyCode)
{
	return MapVirtualKey (keyCode, 2);
}


LRESULT CALLBACK keyboardHookProc (int nCode, WPARAM wParam, LPARAM lParam)  
{	
	DWORD pipeMsg[16];
	
	if(nCode == HC_ACTION)
	{
//		if (wParam == WM_KEYDOWN)
//		{
//			LPKBDLLHOOKSTRUCT event = (LPKBDLLHOOKSTRUCT) lParam;
//
//			if (! isModifierKey (event->vkCode))
//			{				
//				Pipe clientPipe;
//
//				if (! clientPipe.Create ("JDICEXTHOOKPIPE", 0, FALSE, NULL, NULL))
//				{
//					OutputDebugString ("Unable to create hook client pipe\n");
//				}
//
//				sprintf (pipeMsg, "%02x%08x%08x", PIPE_MSG_KEYDOWN, event->time, event->vkCode);
//
//				clientPipe.WritePipe (pipeMsg, strlen (pipeMsg) + 1);
//
//				clientPipe.Close ();
//			}
//		}
		if (wParam == WM_KEYUP)
		{
			LPKBDLLHOOKSTRUCT event = (LPKBDLLHOOKSTRUCT) lParam;

			if (! isModifierKey (event->vkCode))
			{
				Pipe clientPipe;

				if (! clientPipe.Create ("JDICEXTHOOKPIPE", 0, FALSE, NULL, NULL))
				{
					OutputDebugString ("Unable to create hook client pipe\n");
				}

				pipeMsg[0] = PIPE_MSG_KEYUP;
				pipeMsg[1] = event->time;
				pipeMsg[2] = event->vkCode;

				clientPipe.WritePipe ((char *) pipeMsg, 3 * sizeof (DWORD));

				clientPipe.Close ();
			}
		}
	}

	return CallNextHookEx (keyboardHook, nCode, wParam, lParam);
}


ULONG WINAPI hookThreadProc (LPVOID data)
{
	char buf[80];
	
	if (! keyboardHook) 
	{
		keyboardHook = SetWindowsHookEx (WH_KEYBOARD_LL, (HOOKPROC) keyboardHookProc, dllInstance, 0);
		while (hookThreadRunning)
		{
			MSG msg;
			if (GetMessage (& msg, NULL, 0, 0))
			{
				TranslateMessage (& msg);
				DispatchMessage(& msg);
			}
		}
	}

	return 0;
}


PIPE_CALLBACK hookPipeHandler (LPVOID param, LPVOID buffer, DWORD length)
{
	DWORD * msg = (DWORD *) buffer;
	
	switch (msg[0])
	{
		case PIPE_MSG_KEYDOWN:
			{
				DWORD time = msg[1];
				DWORD keyCode = msg[2];
				JNIEnv * env = NULL;
				javaVM->AttachCurrentThread ((void **) & env, (void *) NULL);
				env->CallVoidMethod (jdicExtInstance, globalKeyPressedMethod, time, 
					calculateModifiers (), keyCode, calculateKeyChar (keyCode));
				javaVM->DetachCurrentThread  ();
			}
			break;

		case PIPE_MSG_KEYUP:
			{
				DWORD time = msg[1];
				DWORD keyCode = msg[2];
				JNIEnv * env = NULL;
				javaVM->AttachCurrentThread ((void **) & env, (void *) NULL);
				env->CallVoidMethod (jdicExtInstance, globalKeyReleasedMethod, time, 
					calculateModifiers (), keyCode, calculateKeyChar (keyCode));
				javaVM->DetachCurrentThread  ();
			}
			break;		
	}

	return 0;
}


#ifdef __cplusplus
}
#endif


/*
 * Class:     de_iritgo_jdicext_JDICExt
 * Method:    nativeInstallSystemHooks
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_iritgo_jdicext_JDICExt_nativeInstallSystemHooks
  (JNIEnv * env, jobject self)
{
	env->GetJavaVM (& javaVM);
	jdicExtInstance = env->NewGlobalRef (self);
	jclass klass = env->GetObjectClass (self);
	globalKeyPressedMethod = env->GetMethodID (klass, "globalKeyPressed", "(IIII)V");
	globalKeyReleasedMethod = env->GetMethodID (klass, "globalKeyReleased", "(IIII)V");

	if (hookThread == NULL)
	{
		hookThread = (HANDLE) CreateThread (NULL, 0, hookThreadProc, NULL, 0, 0);

		if (! pipe.Create ("JDICEXTHOOKPIPE", PIPE_READ|PIPE_WRITE, TRUE, (PIPE_CALLBACK) hookPipeHandler, & pipe))
		{
			OutputDebugString ("Unable to create hook pipe\n");
		}
	}
}

/*
 * Class:     de_iritgo_jdicext_JDICExt
 * Method:    nativeRemoveSystemHooks
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_iritgo_jdicext_JDICExt_nativeRemoveSystemHooks
  (JNIEnv * env, jobject self)
{
	{
		if (hookThread != NULL) 
		{
			hookThreadRunning = false;
			TerminateThread (hookThread, 0);
			CloseHandle (hookThread);

			pipe.Close ();

			UnhookWindowsHookEx (keyboardHook);

			env->DeleteGlobalRef (jdicExtInstance);
		}
	}
}

/*
 * Class:     de_iritgo_jdicext_JDICExt
 * Method:    copySelectedTextInActiveWindowToClipboard
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_iritgo_jdicext_JDICExt_copySelectedTextInActiveWindowToClipboard
  (JNIEnv * env, jobject self)
{
	HWND window = GetForegroundWindow ();

	OpenClipboard (window);
	EmptyClipboard ();
	CloseClipboard ();

	BOOL res = AttachThreadInput (GetCurrentThreadId (), GetWindowThreadProcessId (window, NULL), TRUE);
	if (res)
	{
		HWND focusWindow = GetFocus();
		if (focusWindow != NULL)
		{
			/*
			INPUT input[4];
			memset (input, 0, sizeof (input));
			input[0].type = INPUT_KEYBOARD;
			input[0].ki.wVk = VK_CONTROL;
			input[0].ki.dwFlags = 0;
			input[0].ki.time = GetTickCount ();
			input[1].type = INPUT_KEYBOARD;
			input[1].ki.wVk = 'C';
			input[1].ki.dwFlags = 0;
			input[1].ki.time = GetTickCount ();
			input[2].type = INPUT_KEYBOARD;
			input[2].ki.wVk = 'C';
			input[2].ki.dwFlags = KEYEVENTF_KEYUP;
			input[2].ki.time = GetTickCount ();
			input[3].type = INPUT_KEYBOARD;
			input[3].ki.wVk = VK_CONTROL;
			input[3].ki.dwFlags = KEYEVENTF_KEYUP;
			input[3].ki.time = GetTickCount ();
			SendInput (4, input, sizeof (INPUT));
			*/
			keybd_event (VK_CONTROL, 0, 0, 0);
			Sleep (250);
			keybd_event ('C', 0, 0, 0);
			Sleep (250);
			keybd_event ('C', 0, KEYEVENTF_KEYUP, 0);
			Sleep (250);
			keybd_event (VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
			Sleep (250);
		}

		AttachThreadInput (GetCurrentThreadId (), GetWindowThreadProcessId (window, NULL), FALSE);
	}
}
