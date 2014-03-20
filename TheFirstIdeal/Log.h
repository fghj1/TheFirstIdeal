#pragma once

// Console 화면에 색 속성
#define CONSOLE_DEFAULT ( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE )

#define CONSOLE_FG_RED ( FOREGROUND_RED | FOREGROUND_INTENSITY )
#define CONSOLE_FG_GREEN ( FOREGROUND_GREEN | FOREGROUND_INTENSITY )
#define CONSOLE_FG_BLUE ( FOREGROUND_BLUE | FOREGROUND_INTENSITY )
#define CONSOLE_FG_YELLOW ( CONSOLE_FG_RED | CONSOLE_FG_GREEN )
#define CONSOLE_FG_PINK ( CONSOLE_FG_RED | CONSOLE_FG_BLUE )
#define CONSOLE_FG_SKYBLUE ( CONSOLE_FG_GREEN | CONSOLE_FG_BLUE )
#define CONSOLE_FG_WHITE ( CONSOLE_FG_RED | CONSOLE_FG_GREEN | CONSOLE_FG_BLUE )

#define CONSOLE_BG_RED ( BACKGROUND_RED | BACKGROUND_INTENSITY )
#define CONSOLE_BG_GREEN ( BACKGROUND_GREEN | BACKGROUND_INTENSITY )
#define CONSOLE_BG_BLUE ( BACKGROUND_BLUE | BACKGROUND_INTENSITY )
#define CONSOLE_BG_YELLOW ( CONSOLE_BG_RED | CONSOLE_BG_GREEN )
#define CONSOLE_BG_PINK ( CONSOLE_BG_RED | CONSOLE_BG_BLUE )
#define CONSOLE_BG_SKYBLUE ( CONSOLE_BG_GREEN | CONSOLE_BG_BLUE )
#define CONSOLE_BG_WHITE ( CONSOLE_BG_RED | CONSOLE_BG_GREEN | CONSOLE_BG_BLUE )

// Log 종류
#define LOG_CLASS_ERROR TEXT( "ERROR" )  // Main process가 중지되는 문제가 발생하였을 때
#define LOG_CLASS_WARNING TEXT( "WARNING" )  // Main process가 중지되지 않지만 발생된 문제를 알려야 할 때
#define LOG_CLASS_INFORMATION TEXT( "INFORMATION" )  // 문제가 없는 Message를 알려야 할 때


// 주의!! WriteLog()와 PrintErrorMsg() 사용 기준
//		  전체 Code를 호출하는 쪽(호출자)과 호출 당하는 쪽(피호출자)으로 구분할 경우,
//		  호출자와 피호출자 관계는 상대적이며, 상하 관계에 의해 Depth가 있다. 최초에 호출된
//		  피호출자를 최초 피호출자로 표기하여 설명한다.
//
//		  호출자는 피호출자에게 의뢰한 처리가 성공했는지 실패했는지에 대해 알 수 있어야 한다.
//		  따라서, 특별한 경우가 아니면 최소한 최초 피호출자의 반환값은 bool형으로 유지하며,
//		  호출자는 최초 피호출자의 반환값을 통해 실패인 경우 PrintErrorMsg()을 호출하여
//		  개발자가 확실히 인지할 수 있도록 명시적으로 출력한다. 최초 피호출자 아래에 속한
//		  피호출자는 WriteLog()을 호출하여 최초 피호출자가 실패할 수 밖에 없었던 이유에
//		  해당하는 Log를 암시적으로 남긴다.
//
//		  위 규칙에서 예외되는 경우
//		  PrintErrorMsg()는 errno, GetLastError()처럼 FormatMessage()를 호출할 때
//		  피호출자의 중요도가 낮을 때
class CLog
{
public:
	CLog();
	~CLog();

	// Log file(UTF-8)에 기록 및 Visual Studio 출력창에 출력
	// 중요도가 낮아 즉각적인 대응이 필요하지 않은 Msg를 위한 출력
	INT WriteLog( bool in_bByCSV, LPTSTR in_lpszLogClass, LPTSTR in_lpszLogStringFormat, ... );

	// GetLastError() 또는 WSAGetLastError()의 Error code에 해당하는
	// Error Msg를 Console 및 Log file(UTF-8)에 출력
	// 중요도가 높아 즉각적인 대응이 필요한 Msg를 위한 출력
	// ex. PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
	// ex. PrintErrorMsg( TEXT( "Explanation" ), false );
	VOID PrintErrorMsg( LPTSTR in_lpszExplanation, bool in_bUsedwErrorNo, DWORD in_dwErrorNo = 0 );

#ifdef _DEBUG
	// Buffer 변수를 Dump한 ANSI file 생성
	// ex. DumpedBuffer.txt
	//
	// 00 00 00 00 00 00 00 00
	// 00 00 00 00 00 00 00 00
	// 00 00 00 00 00 00 00 00
	// 00 00 00 00 00 00 00 00
	// 00 00 00 00 00 00 00 00
	// 00 00 00 00 00 00 00 00
	// 00 00 00 00 00 00 00 00
	// 00 00 00 00 00 00 00 00
	// 00 00 00 00 00 00 00 00
	//
	// 00 한쌍은 1bytes 한 줄은 8bytes를 의미
	VOID DumpBuffer( PVOID in_pvBuffer, size_t in_unSize );
#endif

private:
	TCHAR m_szLogFilePath[520];
	TCHAR m_szLogBuffer[4096];
	TCHAR m_szLogFileName[520];
	SYSTEMTIME m_tSystemTime;
	FILE* m_pfLogFile;

	// Log file 생성 경로 구축
	VOID CreateLogPath( VOID );

	// Log file 생성
	INT UpdateLogFile( bool in_bByCSV, LPTSTR in_lpszLogClass );
};
