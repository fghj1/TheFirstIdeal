#include "stdafx.h"
#include "Console.h"
#include "Log.h"


extern CLog g_Log;
CConsoleManager g_ConsoleManager;

//////////////////////////////////////////////////////////////////////////
// CCommonConsole
CCommonConsole::CCommonConsole()
{
}

CCommonConsole::~CCommonConsole()
{
}

// Screen buffer 크기 설정(Column:80, Row:20480)
VOID CCommonConsole::SetScreenBufferSize( HANDLE /*in_*/hConsoleOutput )
{
	COORD dwSize;
	BOOL lResult = FALSE;

	dwSize.X = 80;  // Column
	dwSize.Y = 20480;  // Row

	lResult = SetConsoleScreenBufferSize( hConsoleOutput, dwSize );
	if( lResult == FALSE )
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
}

// 활성화 하려는 Screen buffer로 전환
BOOL CCommonConsole::SwitchingOfActiveScreenBuffer( HANDLE /*in_*/hConsoleOutput )
{
	BOOL lResult = FALSE;

	lResult = SetConsoleActiveScreenBuffer( hConsoleOutput );
	if( lResult == FALSE )
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );

	return lResult;
}

// 활성화 하려는 Screen buffer의 Title-bar 설정
VOID CCommonConsole::SetTitleOfActiveScreenBuffer( LPCTSTR /*in_*/lpszConsoleClassTitle,
												   tOPERATIONTIME& /*in_*/stOperationTime )
{
	assert( lpszConsoleClassTitle != NULL );

	TCHAR szConsoleTitle[CONSOLE_TITLE_MAX] = {0,};
	BOOL lResult = FALSE;

	// ex. Main Screen - XXXX:XX elapsed(XXXX.XX.XX XX:XX).
	_stprintf_s( szConsoleTitle, CONSOLE_TITLE_MAX,
				 TEXT( "%s - %4d:%2d elapsed(%4d.%2d.%2d %2d:%2d)." ),
				 lpszConsoleClassTitle,
				 stOperationTime.wElapsedHour, stOperationTime.wElapsedMinute,
				 stOperationTime.StartTime.GetYear(), stOperationTime.StartTime.GetMonth(), stOperationTime.StartTime.GetDay(),
				 stOperationTime.StartTime.GetHour(), stOperationTime.StartTime.GetMinute() );

	lResult = SetConsoleTitle( szConsoleTitle );
	if( lResult == FALSE )
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
}

// Screen buffer에 쓰기
bool CCommonConsole::WriteScreenBuffer( HANDLE /*in_*/hConsoleOutput,
										LPCTSTR /*in_*/lpszBuffer )
{
	DWORD dwNumberOfCharsWritten = 0;
	BOOL lResult = FALSE;

	dwNumberOfCharsWritten = static_cast<DWORD>( _tcsclen( lpszBuffer ) );
	lResult = WriteConsole( hConsoleOutput,
							lpszBuffer,
							dwNumberOfCharsWritten, &dwNumberOfCharsWritten,
							NULL );
	if( lResult == FALSE )
	{
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
		WriteConsole( hConsoleOutput,
					  TEXT( "...\n" ),
					  static_cast<DWORD>( _tcsclen( TEXT( "...\n" ) ) ),
					  &dwNumberOfCharsWritten, NULL );
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// CConsoleForInputAndError
CConsoleForInputAndError::CConsoleForInputAndError()
{
	InitStdErrorHandle();
}

CConsoleForInputAndError::~CConsoleForInputAndError()
{
}

// 입출력 관련 Main screen 초기화
VOID CConsoleForInputAndError::InitStdErrorHandle( VOID )
{
	// 표준 Error handle 추출 및 Main screen buffer 크기 설정
	m_hStdError = GetStdHandle( STD_ERROR_HANDLE );
	if( m_hStdError == INVALID_HANDLE_VALUE )
	{
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
		m_hStdError = NULL;
	}
	else
		SetScreenBufferSize( m_hStdError );
}

//////////////////////////////////////////////////////////////////////////
// CConsoleForOutput
CConsoleForOutput::CConsoleForOutput()
{
	InitOutputHandle();
}

CConsoleForOutput::~CConsoleForOutput()
{
	ReleaseOutputHandle();
}

// 출력 전용 Sub screen 초기화
VOID CConsoleForOutput::InitOutputHandle( VOID )
{
	// 출력 전용 Handle 추출 및 Sub screen buffer 크기 설정
	m_hOutput = CreateConsoleScreenBuffer( GENERIC_WRITE,
										   0,
										   NULL,
										   CONSOLE_TEXTMODE_BUFFER,
										   NULL );
	if( m_hOutput == INVALID_HANDLE_VALUE )
	{
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
		m_hOutput = NULL;
	}
	else
		SetScreenBufferSize( m_hOutput );
}

// 출력 전용 Sub screen 자원 반환
VOID CConsoleForOutput::ReleaseOutputHandle( VOID )
{
	BOOL lResult = FALSE;

	lResult = CloseHandle( m_hOutput );
	if( lResult == FALSE )
	{
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | Memory leak can occur.\n" ),
						TEXT( __FUNCTION__ ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// CConsoleManager
CConsoleManager::CConsoleManager():m_phCurrentConsole( NULL )
{
	InitOperationTime();
	SetCurrentConsole();
}

CConsoleManager::~CConsoleManager()
{
}

// 활성화 하려는 Screen으로 전환
bool CConsoleManager::SwitchingOfActiveScreenBuffer( BYTE /*in_*/byOrderOfScreenBuffer/* = CONSOLE_CLASS_MAIN*/ )
{
	BOOL lResult = FALSE;
	TCHAR szTitlePart[CONSOLE_TITLE_TITLEPART] = {0,};

	// 현재 Screen 변수 재설정
	SetCurrentConsole( byOrderOfScreenBuffer );

	if( IsCurrentHandleValid() )
	{
		// Screen 전환
		lResult = CCommonConsole::SwitchingOfActiveScreenBuffer( *m_phCurrentConsole );
		if( lResult == FALSE )
			return false;

		// Screen 전환에 따른 제목 변경
		switch( byOrderOfScreenBuffer )
		{
		case CONSOLE_CLASS_MAIN:
			_tcscpy_s( szTitlePart, CONSOLE_TITLE_TITLEPART, CONSOLE_CLASS_MAIN_TITLE );
			break;

		case CONSOLE_CLASS_1STSUB:
			_tcscpy_s( szTitlePart, CONSOLE_TITLE_TITLEPART, CONSOLE_CLASS_1STSUB_TITLE );
			break;

		default:
			_tcscpy_s( szTitlePart, CONSOLE_TITLE_TITLEPART, TEXT( "Unknown title" ) );
			break;
		}

		UpdateElapsedTime();
		// TODO: CConsoleManager에서 입력과 경과 시간 등의 처리를 위해 Thread가 별도로 운영되면
		//		 여기에 UpdateElapsedTime() 호출은 지워도 된다. Thread에서 주기적으로 UpdateElapsedTime()을
		//		 호출하여 경과 시간을 갱신할 것이므로 아래 SetTitleOfActiveScreenBuffer()에서는
		//		 이렇게 갱신된 시간 값을 가져다 쓰기만 하면 되기 때문이다.
		CCommonConsole::SetTitleOfActiveScreenBuffer( szTitlePart, m_stOperationTime );
	}
	else
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | Could not switch to the other screen buffer.\n" ),
						TEXT( __FUNCTION__ ) );
		return false;
	}

	return true;
}

// 쓰고 싶은 Screen buffer에 쓰기
bool CConsoleManager::WriteScreenBuffer( BYTE /*in_*/byOrderOfScreenBuffer, LPTSTR /*in_*/lpszBuffer, ... )
{
	assert( lpszBuffer != NULL );

	va_list pszVariableFactorList = NULL;
	bool bResult = false;
	TCHAR szTempBuffer[4096] = {0,};

	// 현재 Screen 변수 재설정
	SetCurrentConsole( byOrderOfScreenBuffer );

	// 출력 내용 추출
	va_start( pszVariableFactorList, lpszBuffer );
	_vstprintf_s( szTempBuffer, 4096,
				  lpszBuffer,
				  pszVariableFactorList );
	va_end( pszVariableFactorList );

	// Screen buffer에 출력
	if( IsCurrentHandleValid() )
		bResult = CCommonConsole::WriteScreenBuffer( *m_phCurrentConsole, szTempBuffer );
	else
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | Unable to write to the screen buffer.\n" ),
						TEXT( __FUNCTION__ ) );
		bResult = false;
	}

	return bResult;
}

// 운영 시간 초기화
VOID CConsoleManager::InitOperationTime( VOID )
{
	m_stOperationTime.StartTime = CTime::GetCurrentTime();

	m_stOperationTime.wElapsedHour = 0;
	m_stOperationTime.wElapsedMinute = 0;
}

// 경과 시간 갱신
VOID CConsoleManager::UpdateElapsedTime( VOID )
{
	CTime CurrentTime = CTime::GetCurrentTime();
	LONGLONG dElapsedHour = 0;
	CTimeSpan ElapsedTime;

	// m_stOperationTime.StartTime와 CurrentTime 비교
	if( m_stOperationTime.StartTime > CurrentTime )
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | Operation time value is invalid.\n" ),
						TEXT( __FUNCTION__ ) );

		m_stOperationTime.wElapsedHour = 0;
		m_stOperationTime.wElapsedMinute = 0;
		return;
	}

	ElapsedTime = CurrentTime - m_stOperationTime.StartTime;

	// 경과 시 갱신
	dElapsedHour = ElapsedTime.GetTotalHours();
	if( 9999 < dElapsedHour )  // 365일 * 24시간 = 8760시간
		m_stOperationTime.wElapsedHour = 9999;
	else
		m_stOperationTime.wElapsedHour = static_cast<WORD>( dElapsedHour );

	// 경과 분 갱신
	m_stOperationTime.wElapsedMinute = static_cast<WORD>( ElapsedTime.GetMinutes() );
}

// 현재 사용중인 Screen 변수 재설정
VOID CConsoleManager::SetCurrentConsole( BYTE /*in_*/byOrderOfScreenBuffer/* = CONSOLE_CLASS_MAIN*/ )
{
	switch( byOrderOfScreenBuffer )
	{
	case CONSOLE_CLASS_MAIN:  // Main screen
		m_phCurrentConsole = &m_hStdError;
		break;

	case CONSOLE_CLASS_1STSUB:  // 1st Sub screen
		m_phCurrentConsole = &m_hOutput;
		break;

	default:
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | Unknown value.\n" ),
						TEXT( __FUNCTION__ ) );
		break;
	}
}

// 현재 Screen 변수의 유효성 검사
bool CConsoleManager::IsCurrentHandleValid( VOID ) const
{
	if( ( *m_phCurrentConsole ) == NULL )
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | The handle of screen buffer is invalid.\n" ),
						TEXT( __FUNCTION__ ) );

		return false;
	}

	return true;
}
