// TheFirstIdeal.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Log.h"
#include "Console.h"
#include "Registry.h"

#ifdef _DEBUG
#include "Performance.h"
#endif


extern CLog g_Log;

HANDLE g_hSenderThreadDestroyEvent = NULL;
HANDLE g_hReceiverThreadDestroyEvent = NULL;
HANDLE g_hIOCP = NULL;
struct Packet
{
	WORD wSize;
	WORD wProtocol;
	UINT64 uni64Buffer;
	SOCKET uni64NSocket;
};
Packet* g_pReceiveBuffer = new Packet;

VOID Sender( __in SOCKET uni64NSocket );
unsigned __stdcall SenderThreadCallback( LPVOID lpParameter );
VOID Receiver( __in SOCKET uni64NSocket );
unsigned __stdcall ReceiverThreadCallback( LPVOID lpParameter );

int _tmain( int argc, _TCHAR* argv[] )
{
	// TODO: 출력 버퍼 교체를 위한 단축키 설정할 것!
	// TODO: 두개의 출력 버퍼 사용 여부를 결정할 수 있도록 한다. 하나만 사용하길 원하는 사람도 있을 것이다.
	// TODO: 입력란 출력할 것! 입력이 출력에 영향을 받지 않도록. 엔터키를 입력에도 사용하고 출력 버퍼 교체에도 사용?!
	//		 메인은 에러와 입력을 담당하며, 서브는 출력만 담당하는데 메인에서 입력하여 엔터를 누르면 입력한 것에 대한
	//		 처리 로그가 서브에 출력된다. 동시에 메인에서 서브로 출력 버퍼가 전환된다. 즉, 엔터 한번 누른 것으로
	//		 명령 입력과 처리 로그 서브로 전환이 이뤄지는 것이다. 다시 엔터를 누르면 메인으로 전환되면서 입력란으로 포
	//		 커스가 맞춰진다. 즉, 온라인 게임에서 채팅을 위해 엔터를 누르면 채팅창이 뜨는 것 같은 효과다.
	// TODO: CConsoleManager에서 Thread를 돌려 입력 및 경과 시간에 대한 갱신을 주기적으로 할 수 있도록 한다.
	// TODO: 스크린 버퍼에 어떤 메시지가 추가 되는 등, 사용자에게 알려야 하는 상황이 발생하면 이를 스크린 버퍼
	//		 자동 전환으로써 알리려하지 말고 타이틀 바에 사용자가 알아야 하는 상황이 몇 건 발생했는지 기록해서
	//		 보여준다던가 다른 방법으로 알려 사용자가 직접 스크린 버퍼를 전환할 수 있는 기회를 제공한다. 즉,
	//		 사용자로부터 스크린 버퍼 전환 권한을 뺏는 것이 아니라 권한을 지켜주는 것이다.
	// TODO: -printf_s() 라는 함수 중에 두번째 인수에 버퍼 크기가 정해지는 함수는 실행 중에 이 크기를 넘는 값이
	//		 인자로 제공되어 버퍼에 담기려는 경우도 발생할 수 있다. 이런 경우에는 프로그램이 강제로 중지되어 버린다.
	//		 이런 상황이 발생했을 때, 프로그램을 강제로 중지 시키지 않고 예외 처리 할 수 있는 방법은 무엇이 있을까?
	//		 try catch를 어떻게 효과적으로 사용할 수 있을까?
	//		 여기에서 말하고 있는 try catch에 대해서 고민하다가 회사 일에 집중하는 바람에 이 부분을 몇 주동안 방치해둔 상태다.
	//		 그러다 회사 일로 try catch에 대해서 다시 찾아봤는데 Registry.cpp에서 사용한 __try, __catch이 제대로 동작하지 않는
	//		 것 같은 문제에 대한 해결 실마리를 찾은 것 같다. 컴파일러 옵션에 /EHsc와 /EHa에 대해서 알아보자!!
	// TODO: "Pattern Oriented Software Archtecture Volume 2"에 "4.1 Scoped locking" 부분을 참고하여 Scoped locking
	//		 에 대해서 이해하고 적용하자!
//	_CrtSetBreakAlloc( ? );
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	{
		CConsoleManager ConsoleManager;

		g_Log.PrintErrorMsg( TEXT( "TEST:" ), false, 0x2746 );

		// TEST - about AE_Server
		DWORD dwErrNo = 0;
		WSADATA stWSAData;

		// WinSock2 lib 초기화
		dwErrNo = WSAStartup( MAKEWORD( 2, 2 ), &stWSAData );
		if( dwErrNo != 0 )
		{
			g_Log.PrintErrorMsg( TEXT( "Failed to initiate use of the Winsock DLL." ), true, dwErrNo );
			return 0;
		}

		//INT nCounter = 0;
		//for( nCounter = 0; nCounter < 100; ++nCounter )
		//{
			SOCKET uni64NSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED );
			if( uni64NSocket == INVALID_SOCKET )
			{
				g_Log.PrintErrorMsg( TEXT( "Failed to initiate use of the Winsock DLL." ), true, WSAGetLastError() );
				WSACleanup();
				return 0;
			}

			g_hIOCP = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
			if( g_hIOCP == NULL )
				return FALSE;

			Sender( uni64NSocket );
			Sleep( 100 );
			Receiver( uni64NSocket );

			DWORD Result = WaitForSingleObject( g_hSenderThreadDestroyEvent, INFINITE );
			if( WAIT_OBJECT_0 == Result )
				g_Log.PrintErrorMsg( TEXT( "Close the sender's thread." ), true, GetLastError() );
			if( g_hSenderThreadDestroyEvent )
			{
				CloseHandle( g_hSenderThreadDestroyEvent );
				g_hSenderThreadDestroyEvent = NULL;
			}

			Result = WaitForSingleObject( g_hReceiverThreadDestroyEvent, INFINITE );
			if( WAIT_OBJECT_0 == Result )
				g_Log.PrintErrorMsg( TEXT( "Close the sender's thread." ), true, GetLastError() );
			if( g_hReceiverThreadDestroyEvent )
			{
				CloseHandle( g_hReceiverThreadDestroyEvent );
				g_hReceiverThreadDestroyEvent = NULL;
			}
		//}

		WSACleanup();
		// TEST - about AE_Server

		// Registry
		// Registry 값 생성
		// 등록
		CEnvironmentValue EnValOption;

		EnValOption.OpenRegKey( SUBKEY_OPTION );

		// 쓰기
		BYTE byEnableSubScreenBuffer = 1;

		EnValOption.WriteRegKey( VALUENAME_SUBSCREENBUFFER,
								 REG_BINARY,
								 &byEnableSubScreenBuffer,
								 static_cast<DWORD>( sizeof byEnableSubScreenBuffer ) );

		// Registry 값 추출
		// 열기
		EnValOption.OpenRegKey( SUBKEY_OPTION );

		// 추출
//		EnValOption.ReadRegKey( VALUENAME_SUBSCREENBUFFER );

		BYTE byResult = 0;
		DWORD dwSize = ( sizeof byResult );
		EnValOption.ReadRegKey( VALUENAME_SUBSCREENBUFFER, &dwSize, &byResult );

		TCHAR szRegFileFullPath[MAX_PATH] = {0,};
		EnValOption.ExportRegKeyToFile( SUBKEY, szRegFileFullPath );
		EnValOption.ImportRegKeyFromFile( szRegFileFullPath );

		EnValOption.DeleteRegKey( SUBKEY );

		printf_s( "\n\nPress any key to exit the program..." );
		_getch();
	}
	_CrtDumpMemoryLeaks();

	return 0;
}

VOID Sender( __in SOCKET uni64NSocket )
{
	g_hSenderThreadDestroyEvent = CreateEvent( NULL, TRUE, FALSE, NULL );  // Manual-reset, Non-signal
	if( NULL == g_hSenderThreadDestroyEvent )
		g_Log.PrintErrorMsg( TEXT( "Failed to create the sender's thread destroy event." ), true, GetLastError() );

	HANDLE hWorkerThread = NULL;

	hWorkerThread = ( HANDLE )_beginthreadex( NULL, 0, SenderThreadCallback, ( LPVOID )&uni64NSocket, 0, NULL );
	if( NULL == hWorkerThread )
		g_Log.PrintErrorMsg( TEXT( "Failed to create the sender's thread." ), true, errno );
}

unsigned __stdcall SenderThreadCallback( LPVOID lpParameter )
{
	SOCKET uni64NSocket = *( ( SOCKET* )lpParameter );

	_tprintf_s( TEXT( "[%7d] Create the sender's thread.\n" ), GetCurrentThreadId() );

	INT nCounter = 0;
	SOCKADDR_IN stRemoteAddrInfo;

	stRemoteAddrInfo.sin_family = AF_INET;
	stRemoteAddrInfo.sin_port = htons( 3300 );
	stRemoteAddrInfo.sin_addr.S_un.S_addr = inet_addr( "127.0.0.1" );

	if( WSAConnect( uni64NSocket, ( LPSOCKADDR )&stRemoteAddrInfo, sizeof( SOCKADDR_IN ), NULL, NULL, NULL, NULL ) == SOCKET_ERROR )
	{
		g_Log.PrintErrorMsg( TEXT( "connection." ), true, WSAGetLastError() );
		SetEvent( g_hSenderThreadDestroyEvent );
		return 0;
	}

	g_hIOCP = CreateIoCompletionPort( ( HANDLE )uni64NSocket, g_hIOCP, NULL, 0 );
	if( g_hIOCP == NULL )
	{
		g_Log.PrintErrorMsg( TEXT( "Failed to associate the socket with IOCP." ), true, GetLastError() );
		SetEvent( g_hSenderThreadDestroyEvent );
		return 0;
	}

	INT iResult = 0;
	WSABUF stBuffers;
	DWORD dwNumberOfBytesSent = 0, dwFlags = 0, dwNumberOfBytesRecvd = 0;
	WSAOVERLAPPED stSendOverlapped;

	stSendOverlapped.hEvent = WSACreateEvent();

	ZeroMemory( g_pReceiveBuffer, sizeof( Packet ) );
	stBuffers.buf = ( CHAR* )g_pReceiveBuffer;
	stBuffers.len = sizeof( Packet );

	iResult = WSARecv( uni64NSocket, &stBuffers, 1, &dwNumberOfBytesRecvd, &dwFlags, &stSendOverlapped, NULL );
	if( iResult == SOCKET_ERROR )
	{
		iResult = WSAGetLastError();
		if( iResult != WSA_IO_PENDING )
		{
			g_Log.PrintErrorMsg( TEXT( "(1)Failed to receive packet." ), true, iResult );
			SetEvent( g_hSenderThreadDestroyEvent );
			return 0;
		}
	}

	DWORD dwCurtTime = timeGetTime();
	DWORD dwPreTime = dwCurtTime;
	INT nPreCounter = 0;
	INT nTotalSendSize = 0, nFailTotalSendSize = 0;
	INT nRealTotalSendSize = 0, nPreRealTotalSendSize = 0;
	Packet* pstPacket = new Packet;

	// TODO: 여기까지 처리하여 보내기 준비 후, 수신도 받을 준비가 되면 아래 처리를 재개한다.

	for( nCounter = 1; nCounter < 1000001; ++nCounter )
	{
		ZeroMemory( ( void* )pstPacket, sizeof( Packet ) );

		pstPacket->wSize = sizeof( Packet );
		pstPacket->wProtocol = 1111;
		pstPacket->uni64Buffer = nCounter;
		pstPacket->uni64NSocket = uni64NSocket;

		stBuffers.buf = ( CHAR* )pstPacket;
		stBuffers.len = sizeof( Packet );

		nTotalSendSize += stBuffers.len;

		iResult = WSASend( uni64NSocket, &stBuffers, 1, &dwNumberOfBytesSent, dwFlags, &stSendOverlapped, NULL );
		if( iResult == SOCKET_ERROR )
		{
			iResult = WSAGetLastError();
			if( iResult != WSA_IO_PENDING )
			{
				g_Log.PrintErrorMsg( TEXT( "send." ), true, iResult );
				nFailTotalSendSize += dwNumberOfBytesSent;
				continue;
			}
		}

		nRealTotalSendSize += dwNumberOfBytesSent;

		dwCurtTime = timeGetTime();
		if( ( dwCurtTime - dwPreTime ) > 1000 )
		{
			g_Log.WriteLog( false,
							LOG_CLASS_INFORMATION,
							TEXT( "송신 갯수 : %d | 송신 누적 크기 : %d | 초당 송신 크기 : %d | 총 누적 크기 : %d | 실패 누적 크기 : %d\n" ),
							( nCounter - nPreCounter ), nRealTotalSendSize, ( nRealTotalSendSize - nPreRealTotalSendSize ), nTotalSendSize, nFailTotalSendSize );
			nPreCounter = nCounter;
			nPreRealTotalSendSize = nRealTotalSendSize;
			dwPreTime = dwCurtTime;
		}
	}

	g_Log.WriteLog( false,
					LOG_CLASS_INFORMATION,
					TEXT( "송신 갯수 : %d | 송신 누적 크기 : %d | 초당 송신 크기 : %d | 총 누적 크기 : %d | 실패 누적 크기 : %d\n" ),
					( nCounter - nPreCounter ), nRealTotalSendSize, ( nRealTotalSendSize - nPreRealTotalSendSize ), nTotalSendSize, nFailTotalSendSize );

	if( closesocket( uni64NSocket ) == SOCKET_ERROR )
	{
		g_Log.PrintErrorMsg( TEXT( "close." ), true, WSAGetLastError() );
		SetEvent( g_hSenderThreadDestroyEvent );
		return 0;
	}

	delete pstPacket;
	pstPacket = NULL;

	SetEvent( g_hSenderThreadDestroyEvent );

	return 0;
}

VOID Receiver( __in SOCKET uni64NSocket )
{
	g_hReceiverThreadDestroyEvent = CreateEvent( NULL, TRUE, FALSE, NULL );  // Manual-reset, Non-signal
	if( NULL == g_hReceiverThreadDestroyEvent )
		g_Log.PrintErrorMsg( TEXT( "Failed to create the receiver's thread destroy event." ), true, GetLastError() );

	HANDLE hWorkerThread = NULL;

	hWorkerThread = ( HANDLE )_beginthreadex( NULL, 0, ReceiverThreadCallback, ( LPVOID )&uni64NSocket, 0, NULL );
	if( NULL == hWorkerThread )
		g_Log.PrintErrorMsg( TEXT( "Failed to create the receiver's thread." ), true, errno );
}

unsigned __stdcall ReceiverThreadCallback( LPVOID lpParameter )
{
	SOCKET uni64NSocket = *( ( SOCKET* )lpParameter );

	_tprintf_s( TEXT( "[%7d] Create the receiver's thread.\n" ), GetCurrentThreadId() );

	DWORD dwNumberOfBytes = 0, dwMilliseconds = INFINITE, dwErrNo = 0;
	LPVOID pCompletionKey = NULL;
	LPOVERLAPPED pstOverlapped = NULL;
	BOOL bResult = FALSE;

	INT iResult = 0;
	WSABUF stBuffers;
	DWORD dwFlags = 0, dwNumberOfBytesRecvd = 0;
	WSAOVERLAPPED stReceiveOverlapped;

	while( TRUE )
	{
		bResult = GetQueuedCompletionStatus( g_hIOCP, &dwNumberOfBytes, ( PULONG_PTR )&pCompletionKey, &pstOverlapped, dwMilliseconds );
		if( bResult == FALSE )
		{
			dwErrNo = GetLastError();
			if( dwErrNo == WAIT_TIMEOUT )
				g_Log.PrintErrorMsg( TEXT( "A completion packet does not appear within the specified time." ), true, dwErrNo );
			else if( dwErrNo == ERROR_ABANDONED_WAIT_0 )
			{
				g_Log.PrintErrorMsg( TEXT( "The I/O completion port handle is closed." ), true, dwErrNo );
				SetEvent( g_hReceiverThreadDestroyEvent );
				return 0;
			}
			else
			{
				if( pstOverlapped == NULL )
					g_Log.PrintErrorMsg( TEXT( "Failed to extract completion packet from the I/O completion queue." ), true, dwErrNo );
				else
				{
					if( dwNumberOfBytes == 0 )
					{
						g_Log.PrintErrorMsg( TEXT( "이곳과 관련된 코드 추가할 것!-1" ), true, dwErrNo );
						continue;
					}
					else
						g_Log.PrintErrorMsg( TEXT( "이곳과 관련된 코드 추가할 것!-2" ), true, dwErrNo );
				}

				if( pCompletionKey )
					g_Log.PrintErrorMsg( TEXT( "이곳과 관련된 코드 추가할 것!-3" ), true, dwErrNo );
			}  // else if( dwErrNo == ERROR_ABANDONED_WAIT_0 )
		}  // if( bResult == FALSE )
		else
		{
			if( pstOverlapped == NULL )
			{
				g_Log.PrintErrorMsg( TEXT( "이곳과 관련된 코드 추가할 것!-4" ), true, dwErrNo );
				SetEvent( g_hReceiverThreadDestroyEvent );
				return 0;
			}

			// TODO: 수신 처리
			g_Log.WriteLog( false, LOG_CLASS_INFORMATION, TEXT( "수신 소켓 : %d | 수신 값 : %d\n" ), g_pReceiveBuffer->uni64NSocket, g_pReceiveBuffer->uni64Buffer );
			ZeroMemory( g_pReceiveBuffer, sizeof( Packet ) );

			stReceiveOverlapped.hEvent = WSACreateEvent();

			stBuffers.buf = ( CHAR* )g_pReceiveBuffer;
			stBuffers.len = sizeof( Packet );

			iResult = WSARecv( uni64NSocket, &stBuffers, 1, &dwNumberOfBytesRecvd, &dwFlags, &stReceiveOverlapped, NULL );
			if( iResult == SOCKET_ERROR )
			{
				iResult = WSAGetLastError();
				if( iResult != WSA_IO_PENDING )
				{
					g_Log.PrintErrorMsg( TEXT( "(2)Failed to receive packet." ), true, iResult );
					SetEvent( g_hSenderThreadDestroyEvent );
					return 0;
				}
			}

		}  // if( bResult == FALSE ) else
	}  // while( TRUE )

	SetEvent( g_hReceiverThreadDestroyEvent );
	return 0;
}
