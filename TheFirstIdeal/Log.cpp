#include "stdafx.h"
#include <errno.h>
#include "Log.h"


CLog g_Log;  // Log file 생성

CLog::CLog():m_pfLogFile( NULL )
{
	CreateLogPath();
}

CLog::~CLog()
{
}

// Log file(UTF-8)에 기록 및 Visual Studio 출력창에 출력
// 중요도가 낮아 즉각적인 대응이 필요하지 않은 Msg를 위한 출력
INT CLog::WriteLog( bool /*in_*/bByCSV, LPTSTR /*in_*/lpszLogClass, LPTSTR /*in_*/lpszLogStringFormat, ... )
{
	assert( lpszLogStringFormat != NULL );

	va_list pszVariableFactorList = NULL;
	TCHAR szTempBuffer[4096] = {0,};
	errno_t nErrorNo = 0;

	memset( m_szLogBuffer, 0, ( sizeof m_szLogBuffer ) );

	// Log 내용 추출
	va_start( pszVariableFactorList, lpszLogStringFormat );
	_vstprintf_s( szTempBuffer, 4096,
				  lpszLogStringFormat,
				  pszVariableFactorList );
	va_end( pszVariableFactorList );

	// Log 내용 재구성
	GetLocalTime( &m_tSystemTime );
	if( bByCSV == false )  // 일반 Text format
	{
		_stprintf_s( m_szLogBuffer,
					 _countof( m_szLogBuffer ),
					 TEXT( "%.2d:%.2d:%.2d[%6d]\t%s" ),
					 m_tSystemTime.wHour, m_tSystemTime.wMinute, m_tSystemTime.wSecond,
					 GetCurrentThreadId(),
					 szTempBuffer );
	}
	else  // CSV format
		_stprintf_s( m_szLogBuffer, _countof( m_szLogBuffer ), TEXT( "%s\n" ), szTempBuffer );

	// Log 작성
	nErrorNo = UpdateLogFile( bByCSV, lpszLogClass );

#ifdef _DEBUG
	OutputDebugString( m_szLogBuffer );
#endif

	return nErrorNo;
}

// GetLastError() 또는 WSAGetLastError()의 Error code에 해당하는
// Error Msg를 Console 및 Log file(UTF-8)에 출력
// 중요도가 높아 즉각적인 대응이 필요한 Msg를 위한 출력
// ex. PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
// ex. PrintErrorMsg( TEXT( "Explanation" ), false );
// TODO: lpszExplanation에 "[ERROR] ...", "[WARNING] ..." 등을 입력하면 앞에 []안에 담긴 단어에 따라 콘솔 창에 출력되는 글자 색을 바꿔준다.
// TODO: lpszExplanation의 출력되는 길이를 제한하면 그 제한 값에 맞춰 자동 줄바꿈이 되어 콘솔 창에 출력될 수 있도록 한다.
//		 창이 작은 콘솔 창에서 창 크기에 맞는 문장을 출력하여 글자 가독률을 높인다.
//		 LOG_CLASS_ERROR, LOG_CLASS_WARNING, LOG_CLASS_INFORMATION 을 활용 및 개선한다.
// TODO: printf() 처럼 출력할 문자열 포멧과 여기에 맞는 인자를 받을 수 있도록 한다.
VOID CLog::PrintErrorMsg( LPTSTR /*in_*/lpszExplanation, bool /*in_*/bUsedwErrorNo, DWORD /*in_*/dwErrorNo/* = 0*/ )
{
	assert( lpszExplanation != NULL );

	HANDLE hStdError = NULL;
	DWORD dwFlags = 0,
		  dwLanguageID = 0,
		  dwWrittenSizeInTCHAR = 0,
		  dwNumberOfCharsWritten = 0;
	TCHAR szErrorMsg[1024] = {0,};
	TCHAR szOutputMsg[2048] = {0,};

	hStdError = GetStdHandle( STD_ERROR_HANDLE );

	// Error code에 대응하는 Error Msg 추출
	if( bUsedwErrorNo )
	{
		dwFlags = FORMAT_MESSAGE_IGNORE_INSERTS |
				  FORMAT_MESSAGE_FROM_SYSTEM;
		dwLanguageID = MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT );
		dwWrittenSizeInTCHAR = FormatMessage( dwFlags,
											  NULL,
											  dwErrorNo,
											  dwLanguageID,
											  szErrorMsg, _countof( szErrorMsg ),
											  NULL );
		if( dwWrittenSizeInTCHAR == 0 )
		{
			dwErrorNo = GetLastError();
			_stprintf_s( szOutputMsg, 2048,
						 TEXT( "%s() failed with error no.%d" ),
						 TEXT( __FUNCTION__ ), dwErrorNo );
		}
		else
		{
			_stprintf_s( szOutputMsg, 2048,
						 TEXT( "%s. Error no.%d : %s" ),
						 lpszExplanation, dwErrorNo, szErrorMsg );
		}
	}
	else
	{
		_stprintf_s( szOutputMsg, 2048,
					 TEXT( "%s. Please check the log file.\r\n" ),
					 lpszExplanation );
	}

	// Error Msg 출력 준비
	SetConsoleTextAttribute( hStdError, CONSOLE_FG_RED );
	dwNumberOfCharsWritten = static_cast<DWORD>( _tcscnlen( szOutputMsg, _countof( szOutputMsg ) ) );

	// Error Msg 출력
	WriteConsole( hStdError,
				  szOutputMsg,
				  dwNumberOfCharsWritten, &dwNumberOfCharsWritten,
				  NULL );

	SetConsoleTextAttribute( hStdError, CONSOLE_DEFAULT );

	// Log file에 기록
	WriteLog( false, LOG_CLASS_ERROR, szOutputMsg );
}

#ifdef _DEBUG
// Buffer 변수를 Dump한 ANSI file 생성
VOID CLog::DumpBuffer( PVOID /*in_*/pvBuffer, size_t /*in_*/unSize )
{
	assert( ( pvBuffer != NULL ) && ( unSize != 0 ) );

	BYTE byHigh4Bits = 0x00, byLow4Bits = 0x00;
	CHAR* pszDumpedBuffer = NULL;
	UINT uiIndex = 0;
	CHAR sz1ByteDump[4] = {0,};
	FILE* pfFile = NULL;

	pszDumpedBuffer = ( CHAR* )( malloc( ( unSize + 1 ) * 2 ) );
	if( pszDumpedBuffer == NULL )
	{
		PrintErrorMsg( TEXT( __FUNCTION__ ), true, static_cast<DWORD>( errno ) );
		return;
	}
	memset( pszDumpedBuffer, 0, ( unSize + 1 ) * 2 );

	// To create
	for( uiIndex = 0; uiIndex < unSize; uiIndex++ )
	{
		// 주의!! Shift 연산은 부호 없는 정수형을 대상으로 한다.
		byHigh4Bits = ( *( ( static_cast<BYTE*>( pvBuffer ) ) + uiIndex ) ) >> 4;
		if( byHigh4Bits < 0x0a )
			*( pszDumpedBuffer + ( uiIndex * 2 ) ) = byHigh4Bits + 0x30;
		else
			*( pszDumpedBuffer + ( uiIndex * 2 ) ) = byHigh4Bits + 0x57;

		byLow4Bits = ( *( ( static_cast<BYTE*>( pvBuffer ) ) + uiIndex ) ) & 0x0f;
		if( byLow4Bits < 0x0a )
			*( pszDumpedBuffer + ( ( uiIndex * 2 ) + 1 ) ) = byLow4Bits + 0x30;
		else
			*( pszDumpedBuffer + ( ( uiIndex * 2 ) + 1 ) ) = byLow4Bits + 0x57;
	}
	memset( ( pszDumpedBuffer + ( unSize * 2 ) ), 0, 2 );

	// To write
	pfFile = _fsopen( "DumpedBuffer.txt", "w", _SH_DENYWR );

	for( uiIndex = 0; uiIndex < unSize; uiIndex++ )
	{
		memset( sz1ByteDump, 0, 4 );

		memcpy_s( sz1ByteDump, 4, ( pszDumpedBuffer + ( uiIndex * 2 ) ), 2 );
		if( ( ( uiIndex + 1 ) % 8 ) == 0 )
			sprintf_s( sz1ByteDump, 4, "%s\n", sz1ByteDump );
		else
			sprintf_s( sz1ByteDump, 4, "%s ", sz1ByteDump );

		fprintf_s( pfFile, "%s", sz1ByteDump );
		printf_s( "%s", sz1ByteDump );
	}
	fprintf_s( pfFile, "\n" );
	printf_s( "\n" );

	fclose( pfFile );
	free( pszDumpedBuffer );
}
#endif

// Log file 생성 경로 구축
VOID CLog::CreateLogPath( VOID )
{
	TCHAR szCurrentProcessPath[MAX_PATH] = {0,};
	size_t unLengthOfString = 0;
	INT nErrorNo = 0;

	// 현재 경로 추출
	GetCurrentDirectory( MAX_PATH, szCurrentProcessPath );

	// Log 생성 경로 구성
	_stprintf_s( m_szLogFilePath, _countof( m_szLogFilePath ),
				 TEXT( "%s\\Logs" ), szCurrentProcessPath );
	unLengthOfString = _tcsclen( m_szLogFilePath );  // 경로의 최대 길이 검사
	if( ( MAX_PATH < unLengthOfString ) || ( ( _tmkdir( m_szLogFilePath ) ) == -1 ) )
	{
		nErrorNo = errno;
		if( nErrorNo != EEXIST )
			m_szLogFilePath[0] = TEXT( '' );
	}
}

INT CLog::UpdateLogFile( bool /*in_*/bByCSV, LPTSTR /*in_*/lpszLogClass )
{
	assert( lpszLogClass != NULL );

	size_t unLengthOfString = 0;
	INT nErrorNo = 0;

	// File 열기 준비
	if( m_szLogFilePath[0] != TEXT( '' ) )
	{
		_stprintf_s( m_szLogFileName, _countof( m_szLogFileName ),
					 TEXT( "%s\\%s_%.4d_%.2d_%.2d.%s" ),
					 m_szLogFilePath,
					 lpszLogClass,
					 m_tSystemTime.wYear, m_tSystemTime.wMonth , m_tSystemTime.wDay,
					 ( bByCSV? TEXT( "csv" ):TEXT( "log" ) ) );
	}
	else
	{
		_stprintf_s( m_szLogFileName, _countof( m_szLogFileName ),
					 TEXT( "%s_%.4d_%.2d_%.2d.log" ),
					 lpszLogClass,
					 m_tSystemTime.wYear, m_tSystemTime.wMonth , m_tSystemTime.wDay,
					 ( bByCSV? TEXT( "csv" ):TEXT( "log" ) ) );
	}
	unLengthOfString = _tcsclen( m_szLogFileName );  // 경로의 최대 길이 검사
	if( MAX_PATH < unLengthOfString )
		m_szLogFileName[0] = TEXT( '' );

	// File 열기
	m_pfLogFile = _tfsopen( m_szLogFileName, TEXT( "a" ), _SH_DENYWR );
	if( m_pfLogFile == NULL )
	{
		nErrorNo = errno;

		TCHAR szErrorMsg[256] = {0,};

		_stprintf_s( szErrorMsg, 256,
					 TEXT( "_tfsopen() failed with error no.%d\n" ),
					 nErrorNo );
		_tprintf_s( szErrorMsg );
	}
	else  // File 쓰기(UTF-8)
	{
		INT nWrittenSize = 0;
		CHAR szUTF8LogBuffer[4096] = {0,};

		nWrittenSize = WideCharToMultiByte( CP_UTF8,
											NULL,
											m_szLogBuffer, -1,
											szUTF8LogBuffer, ( sizeof szUTF8LogBuffer ),
											NULL, NULL );
		if( nWrittenSize == 0 )
		{
			DWORD dwErrorNo = GetLastError();
			TCHAR szErrorMsg[256] = {0,};

			_stprintf_s( szErrorMsg, 256,
						 TEXT( "WideCharToMultiByte() failed with error no.%d\n" ),
						 dwErrorNo );
			_tprintf_s( szErrorMsg );
		}

		fprintf_s( m_pfLogFile, szUTF8LogBuffer );  // UNICODE -> UTF8 변환으로 fprintf_s() 사용
	}

	// File 닫기
	if( m_pfLogFile )
	{
		nErrorNo = fclose( m_pfLogFile );
		if( nErrorNo )
		{
			TCHAR szErrorMsg[256] = {0,};

			_stprintf_s( szErrorMsg, 256,
						 TEXT( "fclose() failed with error no.%d\n" ),
						 nErrorNo );
			_tprintf_s( szErrorMsg );
		}

		m_pfLogFile = NULL;
	}

	return nErrorNo;
}
