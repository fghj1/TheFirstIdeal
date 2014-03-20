#pragma once

// Screen buffer title 구조
// ex.
// 1st Sub_ - XXXX:XX elapsed(XXXX.XX.XX XX:XX). __[         Reserve Area        ]_
// 12345678901234567890123456789012345678901234567890123456789012345678901234567890
// 0        1         2         3         4         5         6         7         8
// |------| CONSOLE_TITLE_TITLEPART
//         |--------------------------------------| CONSOLE_TITLE_TIMEPART
//                       CONSOLE_TITLE_RESERVEPART |------------------------------|
// ※ '_'는 '\n' 삽입 고려한 여유 공간
#define CONSOLE_TITLE_MAX 80
#define CONSOLE_TITLE_TITLEPART 8
#define CONSOLE_TITLE_TIMEPART 40
#define CONSOLE_TITLE_RESERVEPART 32

// Screen buffer 종류
#define CONSOLE_CLASS_MAIN 0
#define CONSOLE_CLASS_MAIN_TITLE TEXT( "Main" )
#define CONSOLE_CLASS_1STSUB 1
#define CONSOLE_CLASS_1STSUB_TITLE TEXT( "1st Sub" )


typedef struct
{
	CTime StartTime;

	WORD wElapsedHour;  // 365일 * 24시간 = 8760시간
	WORD wElapsedMinute;
} tOPERATIONTIME;


//////////////////////////////////////////////////////////////////////////
// CCommonConsole
// CConsoleForInputAndError와 CConsoleForOutput에서 공통으로
// 사용하는 것은 여기에 선언 및 정의한다.
class CCommonConsole
{
public:
	CCommonConsole();
	~CCommonConsole();

protected:
	// Screen buffer 크기 설정(Column:80, Row:20480)
	VOID SetScreenBufferSize( HANDLE in_hConsoleOutput );

	// 활성화 하려는 Screen buffer로 전환
	BOOL SwitchingOfActiveScreenBuffer( HANDLE in_hConsoleOutput );

	// 활성화 하려는 Screen buffer의 Title-bar 설정
	VOID SetTitleOfActiveScreenBuffer( LPCTSTR in_lpszConsoleClassTitle,
									   tOPERATIONTIME& in_stOperationTime );

	// Screen buffer에 쓰기
	bool WriteScreenBuffer( HANDLE in_hConsoleOutput,
							LPCTSTR in_lpszBuffer );
};

//////////////////////////////////////////////////////////////////////////
// CConsoleForInputAndError
// CConsoleForInputAndError에서만 사용하는 것은 여기에 선언 및 정의한다.
class CConsoleForInputAndError:public CCommonConsole
{
public:
	CConsoleForInputAndError();
	~CConsoleForInputAndError();

protected:
	HANDLE m_hStdError;

private:
	// 입출력 관련 Main screen 초기화
	VOID InitStdErrorHandle( VOID );
};

//////////////////////////////////////////////////////////////////////////
// CConsoleForOutput
// CConsoleForOutput에서만 사용하는 것은 여기에 선언 및 정의한다.
class CConsoleForOutput:public CCommonConsole
{
public:
	CConsoleForOutput();
	~CConsoleForOutput();

protected:
	HANDLE m_hOutput;

private:
	// 출력 전용 Sub screen 초기화
	VOID InitOutputHandle( VOID );

	// 출력 전용 Sub screen 자원 반환
	VOID ReleaseOutputHandle( VOID );
};

//////////////////////////////////////////////////////////////////////////
// CConsoleManager
// 외부에서 들어오는 Console에 대한 모든 요청은
// 이 Class를 통해서 처리된다.
// TODO: Screen buffer를 2가지로 구분한 것은 하나의 Screen buffer를 통해서 출력되는 내용들이
//		 너무 많고 구분하기가 어렵기 때문이다. 각 Screen buffer의 용도에 맞게 출력 내용을
//		 구분하여 이를 기반으로 빠른 상황 파악이 가능하게 하려는 것이다. 이를 위해서는
//		 Screen buffer에 의한 구분만으로는 해결되지 않는 것 같다. 하나의 상황에 대한 관련성
//		 을 가진 출력 내용을 구분할 수 있는 다른 방법을 찾아야 한다. 예를 들어서,
//		 하나의 상황에 대한 고유 ID 번호를 부여하고 이 ID 번호와 일치하는 출력 내용은 모두
//		 같은 색으로 출력한다거나 같은 표식을 해두는 것이다.
class CConsoleManager:public CConsoleForInputAndError, public CConsoleForOutput
{
public:
	CConsoleManager();
	~CConsoleManager();

	// 활성화 하려는 Screen으로 전환
	bool SwitchingOfActiveScreenBuffer( BYTE in_byOrderOfScreenBuffer = CONSOLE_CLASS_MAIN );

	// 쓰고 싶은 Screen buffer에 쓰기
	bool WriteScreenBuffer( BYTE in_byOrderOfScreenBuffer, LPTSTR in_lpszBuffer, ... );

private:
	HANDLE* m_phCurrentConsole;
	tOPERATIONTIME m_stOperationTime;

	// 운영 시간 초기화
	VOID InitOperationTime( VOID );

	// 경과 시간 갱신
	VOID UpdateElapsedTime( VOID );  // TODO: CConsoleManager에서 Thread를 돌려 입력 및 경과 시간에 대한 갱신을 주기적으로 할 수 있도록 한다.

	// 현재 사용중인 Screen 변수 재설정
	VOID SetCurrentConsole( BYTE in_byOrderOfScreenBuffer = CONSOLE_CLASS_MAIN );

	// 현재 Screen 변수의 유효성 검사
	bool IsCurrentHandleValid( VOID ) const;
};
