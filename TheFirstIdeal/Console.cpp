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

// Screen buffer ũ�� ����(Column:80, Row:20480)
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

// Ȱ��ȭ �Ϸ��� Screen buffer�� ��ȯ
BOOL CCommonConsole::SwitchingOfActiveScreenBuffer( HANDLE /*in_*/hConsoleOutput )
{
	BOOL lResult = FALSE;

	lResult = SetConsoleActiveScreenBuffer( hConsoleOutput );
	if( lResult == FALSE )
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );

	return lResult;
}

// Ȱ��ȭ �Ϸ��� Screen buffer�� Title-bar ����
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

// Screen buffer�� ����
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

// ����� ���� Main screen �ʱ�ȭ
VOID CConsoleForInputAndError::InitStdErrorHandle( VOID )
{
	// ǥ�� Error handle ���� �� Main screen buffer ũ�� ����
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

// ��� ���� Sub screen �ʱ�ȭ
VOID CConsoleForOutput::InitOutputHandle( VOID )
{
	// ��� ���� Handle ���� �� Sub screen buffer ũ�� ����
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

// ��� ���� Sub screen �ڿ� ��ȯ
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

// Ȱ��ȭ �Ϸ��� Screen���� ��ȯ
bool CConsoleManager::SwitchingOfActiveScreenBuffer( BYTE /*in_*/byOrderOfScreenBuffer/* = CONSOLE_CLASS_MAIN*/ )
{
	BOOL lResult = FALSE;
	TCHAR szTitlePart[CONSOLE_TITLE_TITLEPART] = {0,};

	// ���� Screen ���� �缳��
	SetCurrentConsole( byOrderOfScreenBuffer );

	if( IsCurrentHandleValid() )
	{
		// Screen ��ȯ
		lResult = CCommonConsole::SwitchingOfActiveScreenBuffer( *m_phCurrentConsole );
		if( lResult == FALSE )
			return false;

		// Screen ��ȯ�� ���� ���� ����
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
		// TODO: CConsoleManager���� �Է°� ��� �ð� ���� ó���� ���� Thread�� ������ ��Ǹ�
		//		 ���⿡ UpdateElapsedTime() ȣ���� ������ �ȴ�. Thread���� �ֱ������� UpdateElapsedTime()��
		//		 ȣ���Ͽ� ��� �ð��� ������ ���̹Ƿ� �Ʒ� SetTitleOfActiveScreenBuffer()������
		//		 �̷��� ���ŵ� �ð� ���� ������ ���⸸ �ϸ� �Ǳ� �����̴�.
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

// ���� ���� Screen buffer�� ����
bool CConsoleManager::WriteScreenBuffer( BYTE /*in_*/byOrderOfScreenBuffer, LPTSTR /*in_*/lpszBuffer, ... )
{
	assert( lpszBuffer != NULL );

	va_list pszVariableFactorList = NULL;
	bool bResult = false;
	TCHAR szTempBuffer[4096] = {0,};

	// ���� Screen ���� �缳��
	SetCurrentConsole( byOrderOfScreenBuffer );

	// ��� ���� ����
	va_start( pszVariableFactorList, lpszBuffer );
	_vstprintf_s( szTempBuffer, 4096,
				  lpszBuffer,
				  pszVariableFactorList );
	va_end( pszVariableFactorList );

	// Screen buffer�� ���
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

// � �ð� �ʱ�ȭ
VOID CConsoleManager::InitOperationTime( VOID )
{
	m_stOperationTime.StartTime = CTime::GetCurrentTime();

	m_stOperationTime.wElapsedHour = 0;
	m_stOperationTime.wElapsedMinute = 0;
}

// ��� �ð� ����
VOID CConsoleManager::UpdateElapsedTime( VOID )
{
	CTime CurrentTime = CTime::GetCurrentTime();
	LONGLONG dElapsedHour = 0;
	CTimeSpan ElapsedTime;

	// m_stOperationTime.StartTime�� CurrentTime ��
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

	// ��� �� ����
	dElapsedHour = ElapsedTime.GetTotalHours();
	if( 9999 < dElapsedHour )  // 365�� * 24�ð� = 8760�ð�
		m_stOperationTime.wElapsedHour = 9999;
	else
		m_stOperationTime.wElapsedHour = static_cast<WORD>( dElapsedHour );

	// ��� �� ����
	m_stOperationTime.wElapsedMinute = static_cast<WORD>( ElapsedTime.GetMinutes() );
}

// ���� ������� Screen ���� �缳��
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

// ���� Screen ������ ��ȿ�� �˻�
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
