#pragma once

// Screen buffer title ����
// ex.
// 1st Sub_ - XXXX:XX elapsed(XXXX.XX.XX XX:XX). __[         Reserve Area        ]_
// 12345678901234567890123456789012345678901234567890123456789012345678901234567890
// 0        1         2         3         4         5         6         7         8
// |------| CONSOLE_TITLE_TITLEPART
//         |--------------------------------------| CONSOLE_TITLE_TIMEPART
//                       CONSOLE_TITLE_RESERVEPART |------------------------------|
// �� '_'�� '\n' ���� ����� ���� ����
#define CONSOLE_TITLE_MAX 80
#define CONSOLE_TITLE_TITLEPART 8
#define CONSOLE_TITLE_TIMEPART 40
#define CONSOLE_TITLE_RESERVEPART 32

// Screen buffer ����
#define CONSOLE_CLASS_MAIN 0
#define CONSOLE_CLASS_MAIN_TITLE TEXT( "Main" )
#define CONSOLE_CLASS_1STSUB 1
#define CONSOLE_CLASS_1STSUB_TITLE TEXT( "1st Sub" )


typedef struct
{
	CTime StartTime;

	WORD wElapsedHour;  // 365�� * 24�ð� = 8760�ð�
	WORD wElapsedMinute;
} tOPERATIONTIME;


//////////////////////////////////////////////////////////////////////////
// CCommonConsole
// CConsoleForInputAndError�� CConsoleForOutput���� ��������
// ����ϴ� ���� ���⿡ ���� �� �����Ѵ�.
class CCommonConsole
{
public:
	CCommonConsole();
	~CCommonConsole();

protected:
	// Screen buffer ũ�� ����(Column:80, Row:20480)
	VOID SetScreenBufferSize( HANDLE in_hConsoleOutput );

	// Ȱ��ȭ �Ϸ��� Screen buffer�� ��ȯ
	BOOL SwitchingOfActiveScreenBuffer( HANDLE in_hConsoleOutput );

	// Ȱ��ȭ �Ϸ��� Screen buffer�� Title-bar ����
	VOID SetTitleOfActiveScreenBuffer( LPCTSTR in_lpszConsoleClassTitle,
									   tOPERATIONTIME& in_stOperationTime );

	// Screen buffer�� ����
	bool WriteScreenBuffer( HANDLE in_hConsoleOutput,
							LPCTSTR in_lpszBuffer );
};

//////////////////////////////////////////////////////////////////////////
// CConsoleForInputAndError
// CConsoleForInputAndError������ ����ϴ� ���� ���⿡ ���� �� �����Ѵ�.
class CConsoleForInputAndError:public CCommonConsole
{
public:
	CConsoleForInputAndError();
	~CConsoleForInputAndError();

protected:
	HANDLE m_hStdError;

private:
	// ����� ���� Main screen �ʱ�ȭ
	VOID InitStdErrorHandle( VOID );
};

//////////////////////////////////////////////////////////////////////////
// CConsoleForOutput
// CConsoleForOutput������ ����ϴ� ���� ���⿡ ���� �� �����Ѵ�.
class CConsoleForOutput:public CCommonConsole
{
public:
	CConsoleForOutput();
	~CConsoleForOutput();

protected:
	HANDLE m_hOutput;

private:
	// ��� ���� Sub screen �ʱ�ȭ
	VOID InitOutputHandle( VOID );

	// ��� ���� Sub screen �ڿ� ��ȯ
	VOID ReleaseOutputHandle( VOID );
};

//////////////////////////////////////////////////////////////////////////
// CConsoleManager
// �ܺο��� ������ Console�� ���� ��� ��û��
// �� Class�� ���ؼ� ó���ȴ�.
// TODO: Screen buffer�� 2������ ������ ���� �ϳ��� Screen buffer�� ���ؼ� ��µǴ� �������
//		 �ʹ� ���� �����ϱⰡ ��Ʊ� �����̴�. �� Screen buffer�� �뵵�� �°� ��� ������
//		 �����Ͽ� �̸� ������� ���� ��Ȳ �ľ��� �����ϰ� �Ϸ��� ���̴�. �̸� ���ؼ���
//		 Screen buffer�� ���� ���и����δ� �ذ���� �ʴ� �� ����. �ϳ��� ��Ȳ�� ���� ���ü�
//		 �� ���� ��� ������ ������ �� �ִ� �ٸ� ����� ã�ƾ� �Ѵ�. ���� ��,
//		 �ϳ��� ��Ȳ�� ���� ���� ID ��ȣ�� �ο��ϰ� �� ID ��ȣ�� ��ġ�ϴ� ��� ������ ���
//		 ���� ������ ����Ѵٰų� ���� ǥ���� �صδ� ���̴�.
class CConsoleManager:public CConsoleForInputAndError, public CConsoleForOutput
{
public:
	CConsoleManager();
	~CConsoleManager();

	// Ȱ��ȭ �Ϸ��� Screen���� ��ȯ
	bool SwitchingOfActiveScreenBuffer( BYTE in_byOrderOfScreenBuffer = CONSOLE_CLASS_MAIN );

	// ���� ���� Screen buffer�� ����
	bool WriteScreenBuffer( BYTE in_byOrderOfScreenBuffer, LPTSTR in_lpszBuffer, ... );

private:
	HANDLE* m_phCurrentConsole;
	tOPERATIONTIME m_stOperationTime;

	// � �ð� �ʱ�ȭ
	VOID InitOperationTime( VOID );

	// ��� �ð� ����
	VOID UpdateElapsedTime( VOID );  // TODO: CConsoleManager���� Thread�� ���� �Է� �� ��� �ð��� ���� ������ �ֱ������� �� �� �ֵ��� �Ѵ�.

	// ���� ������� Screen ���� �缳��
	VOID SetCurrentConsole( BYTE in_byOrderOfScreenBuffer = CONSOLE_CLASS_MAIN );

	// ���� Screen ������ ��ȿ�� �˻�
	bool IsCurrentHandleValid( VOID ) const;
};
