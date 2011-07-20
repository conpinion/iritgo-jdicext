/*** Disclaimer ******************************************************************/
/*                                                                               */
/*  Win32 Named Pipe Wrapper                          (C) Clemens Fischer 2001   */
/*                                                                               */
/*  THE CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,                   */
/*  EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO WARRANTIES          */
/*  OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.                      */
/*  IN NO EVENT WILL THE AUTHOR OR AUTHORS BE LIABLE TO YOU FOR ANY DAMAGES,     */
/*  INCLUDING INCIDENTAL OR CONSEQUENTIAL DAMAGES, ARISING OUT OF THE USE        */
/*  OF THE CODE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.             */
/*  YOU ACKNOWLEDGE THAT YOU HAVE READ THIS LICENSE, UNDERSTAND IT AND AGREE     */
/*  TO BE BOUND BY ITS TERMS AS THE COMPLETE AND EXCLUSIVE STATEMENT OF          */
/*  THE AGREEMENT BETWEEN US, SUPERSEDING ANY PROPOSAL OR PRIOR AGREEMENT,       */
/*  ORAL OR WRITTEN, AND ANY OTHER COMMUNICATIONS BETWEEN US RELATING            */
/*  TO THE SUBJECT MATTER OF THIS LICENSE.                                       */
/*                                                                               */
/*********************************************************************************/

/*** History *********************************************************************/
/*	                                                                             */
/*    02-08-2001     Initial version.                                            */
/*                                                                               */
/*********************************************************************************/

#include <windows.h>
#include <stdio.h>
#include "pipe.h"

/*** Constructor *****************************************************************/
/*	                                                                             */
/*  Return Value:   none                                                         */
/*	                                                                             */
/*  Parameters:     none                                                         */
/*	                                                                             */
/*********************************************************************************/

Pipe::Pipe ( void )
{
	// Init

	m_LastError				= PIPE_ERROR_SUCCESS;
	m_GUID					= "Iritgo Technologies JDICExt";
	m_SubKey				= "";

	m_bCreateCalled			= FALSE;
	m_bCloseCalled			= FALSE;
	m_bPipeRegistered		= FALSE;

	m_hReadPipe				= NULL;
	m_hWritePipe			= NULL;
	m_hClientReadPipe		= NULL;
	m_hClientWritePipe		= NULL;
	m_hMutex				= NULL;

	m_bVaildThread			= FALSE;
	m_bRunThread			= TRUE;
}

/*** Destructor ******************************************************************/
/*	                                                                             */
/*  Return Value:   none.                                                        */
/*	                                                                             */
/*  Parameters:     none.                                                        */
/*	                                                                             */
/*********************************************************************************/

Pipe::~Pipe ( void )
{
	Close();
}

/*** Create **********************************************************************/
/*	                                                                             */
/*  Return Value:   true if successful.                                          */
/*	                                                                             */
/*  Parameters:     PipeName           CString containing the pipe name.         */
/*                  dwDesiredAccess    Specifies the type of access.             */
/*                  bServerSide        Specifies server/client side.             */
/*                  CallBackFunc       Pointer to callback functions.            */
/*                  pCallBackParam     Pointer to a single parameter value       */
/*                                     passed to the callback function.          */
/*	                                                                             */
/*  Description:    Creates and registers the neccessary pipe(s).                */
/*	                                                                             */
/*********************************************************************************/

BOOL Pipe::Create ( char * PipeName , DWORD dwDesiredAccess , BOOL bServerSide , PIPE_CALLBACK CallBackFunc , LPVOID pCallBackParam )
{
	BOOL					bCreationSucceeded;
	SECURITY_ATTRIBUTES		SecAttrib;

	if ( m_bCreateCalled )
	{
//		SetLastError ( PIPE_CREATE_ALREADY_CALLED );

//		return FALSE;
		return TRUE;
	}

	m_PipeName				= PipeName;
	m_dwDesiredAccess		= dwDesiredAccess;
	m_bServerSide			= bServerSide;
	m_CallBackFunc			= CallBackFunc;
	m_pCallBackParam		= pCallBackParam;

	// Check if server side

	if ( m_bServerSide )
	{
		if ( RegisterPipe() )
		{
			// Pipe registration succeeded

			bCreationSucceeded				= FALSE;

			SecAttrib.nLength				= sizeof ( SECURITY_ATTRIBUTES );
			SecAttrib.lpSecurityDescriptor	= NULL;
			SecAttrib.bInheritHandle		= TRUE;

			if ( dwDesiredAccess & PIPE_READ )
			{
				bCreationSucceeded = ( CreatePipe ( &m_hClientReadPipe , &m_hWritePipe , &SecAttrib ,0 ) )?TRUE:FALSE;
			}

			if ( dwDesiredAccess & PIPE_WRITE )
			{
				bCreationSucceeded = ( CreatePipe ( &m_hReadPipe , &m_hClientWritePipe , &SecAttrib ,0 ) )?TRUE:FALSE;
			}

			if ( bCreationSucceeded && UpdateRegistration () && CallBack() )
			{
				m_bCreateCalled			= TRUE;
				return TRUE;
			}
			else
			{
				SetLastError	( PIPE_ERROR_CREATION_FAILED );
				UnRegisterPipe	();
			}
		}
	}
	else
	{
		if ( RetrievePipe () && CallBack () )
		{
			m_bCreateCalled			= TRUE;
			return TRUE;
		}
	}

	// Pipe creation failed

	return FALSE;
}

/*** Close ***********************************************************************/
/*	                                                                             */
/*  Return Value:   none.                                                        */
/*	                                                                             */
/*  Parameters:     none.                                                        */
/*	                                                                             */
/*  Description:    Closes the pipe(s).                                          */
/*	                                                                             */
/*********************************************************************************/

void Pipe::Close ( void )
{
	if ( m_bCloseCalled ) return;

	m_bCloseCalled	= TRUE;
	m_bRunThread	= FALSE;

	UnRegisterPipe	();

	CloseHandle		( m_hClientWritePipe );
	CloseHandle		( m_hClientReadPipe );
	CloseHandle		( m_hWritePipe );
	CloseHandle		( m_hReadPipe );
	CloseHandle		( m_hMutex );

	if ( m_bVaildThread ) *m_pbVaildPtr = FALSE;
}

/*** ReadPipe ********************************************************************/
/*	                                                                             */
/*  Return Value:   true if successful.                                          */
/*	                                                                             */
/*  Parameters:     Buffer                  Pointer to the buffer that receives  */
/*                                          the data.                            */
/*                  nNumberOfBytesToRead    Specifies the number of bytes to be  */
/*                                          read.                                */
/*                  lpNumberOfBytesRead     Pointer to the variable that         */
/*                                          receives the number of bytes read.   */
/*	                                                                             */
/*  Description:    Performs read operation on the pipe.                         */
/*	                                                                             */
/*********************************************************************************/

BOOL Pipe::ReadPipe ( LPVOID Buffer , DWORD nNumberOfBytesToRead , DWORD* lpNumberOfBytesRead )
{
	if ( ReadFile ( m_hReadPipe , Buffer , nNumberOfBytesToRead , lpNumberOfBytesRead , NULL ) )
	{
		// Read pipe succeeded

		return TRUE;
	}

	SetLastError ( PIPE_ERROR_READ_FAILED );

	// Read pipe failed

	return FALSE;
}

/*** WritePipe *******************************************************************/
/*	                                                                             */
/*  Return Value:   true if successful.                                          */
/*	                                                                             */
/*  Parameters:     Buffer                  Pointer to the buffer containing the */
/*                                          data to be written.                  */
/*                  nNumberOfBytesToWrite   Specifies the number of bytes to     */
/*                                          write.                               */
/*	                                                                             */
/*  Description:    Performs write operation on the pipe.                        */
/*	                                                                             */
/*********************************************************************************/

BOOL Pipe::WritePipe ( LPVOID Buffer , DWORD nNumberOfBytesToWrite )
{
	DWORD	nNumberOfBytesWritten;

	if ( WriteFile ( m_hWritePipe , Buffer , nNumberOfBytesToWrite , &nNumberOfBytesWritten , NULL ) )
	{
		if ( nNumberOfBytesWritten == nNumberOfBytesToWrite )
		{
			// Read write succeeded

			return TRUE;
		}
	}

	SetLastError ( PIPE_ERROR_WRITE_FAILED );

	// Write pipe failed

	return FALSE;
}

/*** GetLastError ****************************************************************/
/*	                                                                             */
/*  Return Value:   The return value is the last-error code value.               */
/*	                                                                             */
/*  Parameters:     none.                                                        */
/*	                                                                             */
/*  Description:    Returns the latest Pipe error code. Inside Pipe call       */
/*                  call ::GetLastError() to retrieve the calling thread's last  */
/*                  error code value.                                            */
/*	                                                                             */
/*********************************************************************************/

int Pipe::GetLastError ( void )
{
	// Return latest error value

	return m_LastError;
}

/*** RegisterPipe ****************************************************************/
/*	                                                                             */
/*  Return Value:   true if successful.                                          */
/*	                                                                             */
/*  Parameters:     none.                                                        */
/*	                                                                             */
/*********************************************************************************/

BOOL Pipe::RegisterPipe ( void )
{
	HKEY		hRegKey;
	DWORD		dwDisposition;
	DWORD		dwValuePId;
	DWORD		dwValueDef;
	DWORD		dwDataLen;
	HANDLE		hProcess;
	HANDLE		hMutex;
	char		buf[1024];

	// Create registry key

	sprintf (buf, "SOFTWARE\\%s\\%s", m_GUID, m_PipeName);
	m_SubKey = (char *) malloc (strlen (buf) + 1);
	strcpy (m_SubKey, buf);

	if ( RegCreateKeyEx ( HKEY_CURRENT_USER , m_SubKey , 0 , "" , 0 , KEY_WRITE , NULL , &hRegKey , &dwDisposition ) == ERROR_SUCCESS )
	{
		if ( dwDisposition == REG_OPENED_EXISTING_KEY )
		{
			dwDataLen = sizeof ( DWORD );

			if ( RegQueryValueEx ( hRegKey , "PID" , 0 , NULL , (BYTE*)&dwValuePId , &dwDataLen ) == ERROR_SUCCESS )
			{
				// Check process id &  mutex

				if ( ( hProcess = OpenProcess ( PROCESS_QUERY_INFORMATION , FALSE , dwValuePId ) ) != NULL )
				{
					CloseHandle		( hProcess );

					sprintf (buf, "%s/%s", m_GUID, m_PipeName);
					if ( ( hMutex = OpenMutex ( MUTEX_ALL_ACCESS , FALSE , buf ) ) != NULL )
					{
						CloseHandle		( hMutex );

						SetLastError	( PIPE_ERROR_NAME_ALREADY_EXISTS );
						RegCloseKey		( hRegKey );

						// Pipe registration failed

						return FALSE;
					}
				}
			}
		}

		// Create mutex

		sprintf (buf, "%s/%s", m_GUID, m_PipeName);
		if ( ( m_hMutex = CreateMutex ( NULL , TRUE , buf ) ) != NULL )
		{
			dwValuePId = GetCurrentProcessId();
			dwValueDef = (DWORD)INVALID_HANDLE_VALUE;

			// Save data

			if ( (	RegSetValueEx ( hRegKey , "PID" , 0 , REG_DWORD , (BYTE*)&dwValuePId , sizeof ( DWORD ) ) &
					RegSetValueEx ( hRegKey , "HRP" , 0 , REG_DWORD , (BYTE*)&dwValueDef , sizeof ( DWORD ) ) &
					RegSetValueEx ( hRegKey , "HWP" , 0 , REG_DWORD , (BYTE*)&dwValueDef , sizeof ( DWORD ) ) ) == ERROR_SUCCESS )
			{
				RegCloseKey ( hRegKey );
				m_bPipeRegistered = TRUE;

				// Pipe registration succeeded

				return TRUE;
			}
		}

		RegCloseKey ( hRegKey );
	}

	SetLastError ( PIPE_ERROR_NAME_REGISTRATION_FAILED );

	// Pipe registration failed

	return FALSE;
}

/*** UnRegisterPipe **************************************************************/
/*	                                                                             */
/*  Return Value:   true if successful.                                          */
/*	                                                                             */
/*  Parameters:     none.                                                        */
/*	                                                                             */
/*********************************************************************************/

BOOL Pipe::UnRegisterPipe ( void )
{
	// Delete registry key

	if ( m_bPipeRegistered && ( RegDeleteKey ( HKEY_CURRENT_USER , m_SubKey ) == ERROR_SUCCESS ) )
	{
		// Pipe unregistration succeeded

		m_bPipeRegistered = FALSE;
		return TRUE;
	}

	SetLastError ( PIPE_ERROR_NAME_UNREGISTRATIONS_FAILED );

	// Pipe unregistration failed

	return FALSE;
}

/*** UpdateRegistration **********************************************************/
/*	                                                                             */
/*  Return Value:   true if successful.                                          */
/*	                                                                             */
/*  Parameters:     none.                                                        */
/*	                                                                             */
/*********************************************************************************/

BOOL Pipe::UpdateRegistration ( void )
{
	HKEY	hRegKey;
	DWORD	dwDisposition;

	// Open registry key

	if ( RegCreateKeyEx ( HKEY_CURRENT_USER , m_SubKey , 0 , "" , 0 , KEY_ALL_ACCESS , NULL , &hRegKey , &dwDisposition ) == ERROR_SUCCESS )
	{
		if ( (	RegSetValueEx ( hRegKey , "HRP" , 0 , REG_DWORD , (BYTE*)&m_hClientReadPipe , sizeof ( DWORD ) ) &
				RegSetValueEx ( hRegKey , "HWP" , 0 , REG_DWORD , (BYTE*)&m_hClientWritePipe , sizeof ( DWORD ) ) ) == ERROR_SUCCESS )
		{
			// Registry update succeeded

			RegCloseKey ( hRegKey );

			return TRUE;
		}

		RegCloseKey ( hRegKey );
	}

	// Registry update failed

	return FALSE;
}

/*** RetrievePipe ****************************************************************/
/*	                                                                             */
/*  Return Value:   true if successful.                                          */
/*	                                                                             */
/*  Parameters:     none.                                                        */
/*	                                                                             */
/*********************************************************************************/

BOOL Pipe::RetrievePipe ( void )
{
	HKEY	hRegKey;
	DWORD	dwValuePId;
	DWORD	dwReadPipe;
	DWORD	dwWritePipe;
	HANDLE	hProcess;
	DWORD	dwDataLen;
	HANDLE	hMutex;
	char	buf[1024];

	// Open registry key

	sprintf (buf, "SOFTWARE\\%s\\%s", m_GUID, m_PipeName);
	m_SubKey = (char *) malloc (strlen (buf) + 1);
	strcpy (m_SubKey, buf);

	if ( RegOpenKeyEx ( HKEY_CURRENT_USER , m_SubKey , 0 , KEY_READ, &hRegKey ) == ERROR_SUCCESS )
	{
		dwDataLen = sizeof ( DWORD );

		if ( RegQueryValueEx ( hRegKey , "PID" , 0 , NULL , (BYTE*)&dwValuePId , &dwDataLen ) == ERROR_SUCCESS )
		{
			// Check process id & mutex

			if ( ( hProcess = OpenProcess ( PROCESS_DUP_HANDLE , FALSE , dwValuePId ) ) != NULL )
			{
				sprintf (buf, "%s/%s", m_GUID, m_PipeName);
				if ( ( hMutex = OpenMutex ( MUTEX_ALL_ACCESS , FALSE , buf ) ) != NULL )
				{
					if ( ( RegQueryValueEx ( hRegKey , "HRP" , 0 , NULL , (BYTE*)&dwReadPipe , &dwDataLen ) &
						   RegQueryValueEx ( hRegKey , "HWP" , 0 , NULL , (BYTE*)&dwWritePipe , &dwDataLen ) ) == ERROR_SUCCESS )
					{
						if ( dwReadPipe != 0 )
						{
							if ( !DuplicateHandle ( hProcess , (HANDLE)dwReadPipe , GetCurrentProcess() , &m_hReadPipe , 0 , FALSE , DUPLICATE_SAME_ACCESS ) )
							{
								m_hReadPipe = INVALID_HANDLE_VALUE;
							}
						}

						if ( dwWritePipe != 0 )
						{
							if ( !DuplicateHandle ( hProcess , (HANDLE)dwWritePipe , GetCurrentProcess() , &m_hWritePipe , 0 , FALSE , DUPLICATE_SAME_ACCESS ) )
							{
								m_hWritePipe = INVALID_HANDLE_VALUE;
							}
						}

						if ( ( m_hReadPipe != INVALID_HANDLE_VALUE ) && ( m_hWritePipe != INVALID_HANDLE_VALUE ) )
						{
							CloseHandle	( hProcess );
							CloseHandle	( hMutex );
							RegCloseKey	( hRegKey );

							// Retrieve pipe succeeded

							return TRUE;
						}
					}

					SetLastError	( PIPE_ERROR_CREATION_FAILED );
					CloseHandle		( hProcess );
					CloseHandle		( hMutex );
					RegCloseKey		( hRegKey );

					// Retrieve pipe failed

					return FALSE;
				}
			}

			CloseHandle	( hProcess );
		}

		RegCloseKey ( hRegKey );
	}

	SetLastError ( PIPE_ERROR_NAME_DOES_NOT_EXISTS );

	// Retrieve pipe failed

	return FALSE;
}

/*** CallBackThread **************************************************************/
/*	                                                                             */
/*  Return Value:   thread exit code.                                            */
/*	                                                                             */
/*  Parameters:     pParam             Pointer to a single parameter value       */
/*                                     passed to the thread function.            */
/*	                                                                             */
/*********************************************************************************/

ULONG WINAPI Pipe::CallBackThread ( LPVOID pParam )
{
	Pipe	*pThis			= (Pipe*)pParam;
	BOOL	bValidPtr		= TRUE;
	BYTE	Buffer[PIPE_MAX_READ_BUFFER_SIZE];
	DWORD	dwBytesRead;

	// Init

	pThis->m_bVaildThread	= TRUE;
	pThis->m_pbVaildPtr		= &bValidPtr;

	while ( pThis->m_bRunThread && pThis->ReadPipe ( &Buffer[0] , PIPE_MAX_READ_BUFFER_SIZE , &dwBytesRead ) )
	{
		if ( bValidPtr ) pThis->m_CallBackFunc ( pThis->m_pCallBackParam , &Buffer[0] , dwBytesRead );
	}

	// Leave thread

	if ( bValidPtr ) pThis->m_bVaildThread = FALSE;

	return 0;
}

/*** CallBack ********************************************************************/
/*	                                                                             */
/*  Return Value:   true if successful.                                          */
/*	                                                                             */
/*  Parameters:     none.                                                        */
/*	                                                                             */
/*********************************************************************************/

BOOL Pipe::CallBack ( void )
{
	if ( !m_CallBackFunc ) return TRUE;

	callbackThreadHandle = (HANDLE) CreateThread (NULL, 0, CallBackThread, (LPVOID) this, 0, 0);

	if (callbackThreadHandle != NULL)
	{
		// CallBack succeeded

		return TRUE;
	}

	SetLastError ( PIPE_ERROR_CALLBACK );

	// CallBack failed

	return FALSE;
}

/*** SetLastError ****************************************************************/
/*	                                                                             */
/*  Return Value:   none.                                                        */
/*	                                                                             */
/*  Parameters:     nErrorCode      Specifies the last-error code.               */
/*	                                                                             */
/*********************************************************************************/

void Pipe::SetLastError ( int nErrorCode )
{
	// Set last erroer value

	m_LastError = nErrorCode;
}



BOOL Pipe::IsCreated ( void )
{
	return m_bCreateCalled;
}
