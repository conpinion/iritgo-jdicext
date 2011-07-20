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

#ifndef		__PIPE_CLASS__
#define		__PIPE_CLASS__


#define		PIPE_MAX_READ_BUFFER_SIZE		0xFFF

// Pipe errors

enum
{
	PIPE_ERROR_SUCCESS						= 0x40000000 ,		// Success
	PIPE_ERROR_NAME_ALREADY_EXISTS						 ,		// The specified pipe name already exists
	PIPE_ERROR_NAME_REGISTRATION_FAILED				     ,		// The pipe name registration failed 
	PIPE_ERROR_NAME_DOES_NOT_EXISTS						 ,		// The specified pipe name doesn´t exists
	PIPE_ERROR_NAME_UNREGISTRATIONS_FAILED				 , 		// The pipe name unregistration failed 
	PIPE_ERROR_CREATION_FAILED							 ,		// Pipe creation failed
	PIPE_ERROR_CALLBACK									 ,		// Error related to callback
	PIPE_ERROR_READ_FAILED								 ,		// Error while reading from pipe
	PIPE_ERROR_WRITE_FAILED								 ,		// Error while Wrinting to pipe
	PIPE_CREATE_ALREADY_CALLED									// Create was already called
};

// Pipe access modes

enum
{			
	PIPE_READ								= 0x01	,			// Grant client read access
	PIPE_WRITE													// Grant client write access
};

typedef		UINT(*PIPE_CALLBACK)	( LPVOID pParam , LPVOID Buffer , DWORD dwLength );

class Pipe 
{
	private :
		
		HANDLE				callbackThreadHandle;

		char *				m_GUID;
		char *				m_SubKey;
		int					m_LastError;

		char *				m_PipeName;			
		DWORD				m_dwDesiredAccess;
		BOOL				m_bServerSide;
		PIPE_CALLBACK		m_CallBackFunc;
		LPVOID				m_pCallBackParam;

		BOOL				m_bCreateCalled;
		BOOL				m_bCloseCalled;
		BOOL				m_bPipeRegistered;

		HANDLE				m_hReadPipe;
		HANDLE				m_hWritePipe;
		HANDLE				m_hClientReadPipe;
		HANDLE				m_hClientWritePipe;
		HANDLE				m_hMutex;

		BOOL				*m_pbVaildPtr;
		BOOL				m_bVaildThread;
		BOOL				m_bRunThread;

		// Private operations 

		BOOL				RegisterPipe		( void );
		BOOL				UnRegisterPipe		( void );
		BOOL				UpdateRegistration	( void );
		BOOL				RetrievePipe		( void );
		
		BOOL				CallBack			( void );

		void				SetLastError		( int nErrorCode );

		static ULONG WINAPI		CallBackThread		( LPVOID pParam );

	public :

							Pipe				( void );		
							~Pipe				( void );
	
		// Public operations

		BOOL				Create				( char * PipeName , DWORD dwDesiredAccess = PIPE_READ | PIPE_WRITE , BOOL bServerSide = TRUE , PIPE_CALLBACK CallBackFunc = NULL , LPVOID pCallBackParam = NULL );
		BOOL				ReadPipe			( LPVOID Buffer , DWORD nNumberOfBytesToRead , DWORD* lpNumberOfBytesRead );
		BOOL				WritePipe			( LPVOID Buffer , DWORD nNumberOfBytesToWrite );
		void				Close				( void );

		int					GetLastError		( void );

		BOOL				IsCreated			( void );
};

#endif
