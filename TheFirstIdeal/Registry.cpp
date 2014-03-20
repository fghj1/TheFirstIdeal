#include "stdafx.h"
#include "Registry.h"
#include "Log.h"
#include "Console.h"


extern CLog g_Log;
extern CConsoleManager g_ConsoleManager;

CEnvironmentValue::CEnvironmentValue():m_lpvValueBuffer( NULL ),
									   m_dwSizeOfValueBuffer( 0 )
{
}

CEnvironmentValue::~CEnvironmentValue()
{
	CloseRegKey();
	if( m_lpvValueBuffer != NULL )
	{
		free( m_lpvValueBuffer );
		m_lpvValueBuffer = NULL;
	}
	m_dwSizeOfValueBuffer = 0;
}

// Registry key ���� �� ����
bool CEnvironmentValue::OpenRegKey( LPCTSTR /*in_*/lpszSubKey )
{
	assert( lpszSubKey != NULL );

	LONG lResult = ERROR_SUCCESS;

	lResult = RegCreateKeyEx( HKEY_CURRENT_USER,
							  lpszSubKey,
							  0,
							  NULL,
							  REG_OPTION_NON_VOLATILE,
							  KEY_ALL_ACCESS,
							  NULL,
							  &m_hkKey,
							  NULL );
	if( lResult != ERROR_SUCCESS )
	{
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, lResult );
		return false;
	}

	return true;
}

// Registry key ����
bool CEnvironmentValue::WriteRegKey( LPCTSTR /*in_*/lpszValName,
									 DWORD /*in_*/dwValType,
									 const BYTE* /*in_*/pbyDataBuffer,
									 DWORD /*in_*/dwSizeOfDataBuffer )
{
	assert( ( lpszValName != NULL ) && ( pbyDataBuffer != NULL ) );

	LONG lResult = ERROR_SUCCESS;

	lResult = RegSetValueEx( m_hkKey,
							 lpszValName,
							 0,
							 dwValType,
							 pbyDataBuffer,
							 dwSizeOfDataBuffer );
	if( lResult != ERROR_SUCCESS )
	{
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, lResult );
		return false;
	}

	return true;
}

// Registry key �б�
// ����!! in_lpszValName ���ڸ� �Ҵ��Ͽ� ȣ���� ���
//		  ���� ����, m_lpvValueBuffer�� m_dwSizeOfValueBuffer�� ���� ���� �Ҵ�ǹǷ�
//		  �̿� �����ϱ� ���ؼ� GetInsideValueBuffer()�� GetInsideSizeOfValueBuffer()��
//		  ȣ���ؾ� �Ѵ�.
bool CEnvironmentValue::ReadRegKey( LPCTSTR /*in_*/lpszValName,
									LPDWORD /*inout_*/pdwSizeOfProvidedBuffer/* = NULL*/,
									LPBYTE /*inout_*/pbyProvidedBuffer/* = NULL*/ )
{
	assert( lpszValName != NULL );

	LONG lResult = ERROR_SUCCESS;
	DWORD dwValType = 0;
	LPBYTE pbyBuffer = pbyProvidedBuffer;
	LPDWORD pdwSizeOfBuffer = pdwSizeOfProvidedBuffer;

	if( pbyBuffer == NULL )
	{
		// �ʱ�ȭ
		if( m_lpvValueBuffer != NULL )
		{
			free( m_lpvValueBuffer );
			m_lpvValueBuffer = NULL;
			m_dwSizeOfValueBuffer = 0;
		}

		// �ʿ��� ũ�� ���� �� �Ҵ�
		m_dwSizeOfValueBuffer = GetNeedfulSizeOfDataBuffer( lpszValName );
		if( m_dwSizeOfValueBuffer != 0 )
		{
			m_lpvValueBuffer = malloc( m_dwSizeOfValueBuffer );
			if( m_lpvValueBuffer == NULL )
			{
				g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, errno );
				return false;
			}
			memset( m_lpvValueBuffer, 0, m_dwSizeOfValueBuffer );
		}

		pbyBuffer = static_cast<BYTE*>( m_lpvValueBuffer );
		pdwSizeOfBuffer = &m_dwSizeOfValueBuffer;
	}

	lResult = RegQueryValueEx( m_hkKey,
							   lpszValName,
							   NULL,
							   NULL,
							   pbyBuffer,
							   pdwSizeOfBuffer );
	if( lResult != ERROR_SUCCESS )
	{
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, lResult );
		return false;
	}

	return true;
}

// �������� Registry key type ����
DWORD CEnvironmentValue::GetRegValType( LPCTSTR /*in_*/lpszValName )
{
	LONG lResult = ERROR_SUCCESS;
	DWORD dwValType = 0xffffffff;

	lResult = RegQueryValueEx( m_hkKey, lpszValName, NULL, &dwValType, NULL, NULL );
	if( lResult != ERROR_SUCCESS )
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, lResult );

	return dwValType;
}

// �б⿡ �ʿ��� ũ�� ����
DWORD CEnvironmentValue::GetNeedfulSizeOfDataBuffer( LPCTSTR /*in_*/lpszValName )
{
	LONG lResult = ERROR_SUCCESS;
	DWORD dwSizeOfDataBuffer = 0;

	lResult = RegQueryValueEx( m_hkKey, lpszValName, NULL, NULL, NULL, &dwSizeOfDataBuffer );
	if( lResult != ERROR_SUCCESS )
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, lResult );

	return dwSizeOfDataBuffer;
}

// ��������
// in_lpszProvidedFullPath : "C:\Download\*.reg"
bool CEnvironmentValue::ExportRegKeyToFile( LPCTSTR /*in_*/lpszSubKey,
											LPCTSTR /*in_*/lpszProvidedFullPath )
{
	assert( ( lpszSubKey != NULL ) && ( lpszProvidedFullPath != NULL ) );

	size_t unLengthOfString = 0;
	TCHAR szRegFileFullPath[MAX_PATH] = {0,};
	TCHAR szParameters[1024] = {0,};
	SHELLEXECUTEINFO tShellExecuteInfo;

	// �������� ��� ����
	if( ( lpszSubKey == NULL ) || ( lpszProvidedFullPath == NULL ) )
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | Parameter is invalid.\n" ),
						TEXT( __FUNCTION__ ) );
		return false;
	}
	else
	{
		unLengthOfString = _tcsclen( lpszProvidedFullPath );
		if( ( unLengthOfString == 0 ) || ( MAX_PATH < unLengthOfString ) )
		{
			g_Log.WriteLog( false,
							LOG_CLASS_WARNING,
							TEXT( "%s | The length of the file path is invalid. | %d\n" ),
							TEXT( __FUNCTION__ ),
							unLengthOfString );
			return false;
		}

		_tcscpy_s( szRegFileFullPath, MAX_PATH, lpszProvidedFullPath );
	}

	// Parameter ����
//	_stprintf_s( szParameters, 1024,
//				 TEXT( "/e \"%s\" \"%s\\%s\"" ), szRegFileFullPath, TEXT( "HKEY_CURRENT_USER" ), lpszSubKey );
	// TEST
	char szTest[256] = {0,};
	sprintf( szTest, "\n���� ��!!\n" );
	OutputDebugStringA( szTest );
	printf( "\n���� ��!!\n" );

//	LPTOP_LEVEL_EXCEPTION_FILTER pfnTopLevelExcFilter = NULL;
//	pfnTopLevelExcFilter = SetUnhandledExceptionFilter( CustomizedExcFilter );

	__try  // try
	{
//		_stprintf_s( szParameters, 126,
//					 TEXT( "/e \"%s\" \"%s\\%s\"" ), szRegFileFullPath, TEXT( "HKEY_CURRENT_USER" ), lpszSubKey );
		int* pnTest = NULL;
		*pnTest = 1;
	}
	__except( CustomizedExcFilter( GetExceptionInformation() ) )  // catch( ? )
	{
		sprintf( szTest, "\n���� �߻�!!\n" );
		OutputDebugStringA( szTest );
		printf( "\n���� �߻�!!\n" );
	}
	sprintf( szTest, "\n���� ��!!\n" );
	OutputDebugStringA( szTest );
	printf( "\n���� ��!!\n" );

//	SetUnhandledExceptionFilter( pfnTopLevelExcFilter );
	// TEST

	// �������� ����
	ZeroMemory( &tShellExecuteInfo, sizeof( SHELLEXECUTEINFO ) );
	tShellExecuteInfo.cbSize = sizeof( SHELLEXECUTEINFO );
	tShellExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	tShellExecuteInfo.lpVerb = TEXT( "open" );
	tShellExecuteInfo.lpFile = TEXT( "regedit.exe" );
	tShellExecuteInfo.lpParameters = szParameters;
	tShellExecuteInfo.nShow = SW_HIDE;

	if( ShellExecuteEx( &tShellExecuteInfo ) == FALSE )
	{
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
		return false;
	}

	// ����!! ShellExecuteEx() ��, WaitForSingleObject() ȣ�� ���� �ٷ� ��ȯ�� ���
	//		  '��������'�� ���� *.reg file�� �������� �ʴ´�.
	if( tShellExecuteInfo.hProcess != NULL )
	{
		if( WaitForSingleObject( tShellExecuteInfo.hProcess, 100 ) == WAIT_FAILED )
		{
			g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
			CloseHandle( tShellExecuteInfo.hProcess );
			return false;
		}

		CloseHandle( tShellExecuteInfo.hProcess );
	}

	return true;
}

// ��������
// in_lpszProvidedFullPath : "C:\Download\*.reg"
bool CEnvironmentValue::ImportRegKeyFromFile( LPCTSTR /*in_*/lpszProvidedFullPath )
{
	assert( lpszProvidedFullPath != NULL );

	size_t unLengthOfString = 0;
	TCHAR szRegFileFullPath[MAX_PATH] = {0,};
	TCHAR szParameters[1024] = {0,};
	BOOL lResult = FALSE;
	SHELLEXECUTEINFO tShellExecuteInfo;

	// �������� ��� �˻�
	if( lpszProvidedFullPath == NULL )
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | Parameter is invalid.\n" ),
						TEXT( __FUNCTION__ ) );
		return false;
	}
	else
	{
		unLengthOfString = _tcsclen( lpszProvidedFullPath );
		if( ( unLengthOfString == 0 ) || ( MAX_PATH < unLengthOfString ) )
		{
			g_Log.WriteLog( false,
							LOG_CLASS_WARNING,
							TEXT( "%s | The length of the file path is invalid. | %d\n" ),
							TEXT( __FUNCTION__ ),
							unLengthOfString );
			return false;
		}

		_tcscpy_s( szRegFileFullPath, MAX_PATH, lpszProvidedFullPath );
	}

	// Parameter ����
	_stprintf_s( szParameters, 1024, TEXT( "-s \"%s\"" ), szRegFileFullPath );

	// �������� ����
	ZeroMemory( &tShellExecuteInfo, sizeof( SHELLEXECUTEINFO ) );
	tShellExecuteInfo.cbSize = sizeof( SHELLEXECUTEINFO );
	tShellExecuteInfo.lpVerb = TEXT( "open" );
	tShellExecuteInfo.lpFile = TEXT( "regedit.exe" );
	tShellExecuteInfo.lpParameters = szParameters;
	tShellExecuteInfo.nShow = SW_HIDE;

	lResult = ShellExecuteEx( &tShellExecuteInfo );
	if( lResult == FALSE )
	{
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, GetLastError() );
		return false;
	}

	return true;
}

// Registry key �����
// ����� ���� �ڵ����� '��������' ó���� �Ѵ�.
bool CEnvironmentValue::DeleteRegKey( LPCTSTR /*in_*/lpszSubKey )
{
	assert( lpszSubKey != NULL );

	SYSTEMTIME tSystemTime;
	TCHAR szCurrentProcessPath[MAX_PATH] = {0,};
	TCHAR szBackupFileName[MAX_PATH] = {0,};
	TCHAR szRegFileFullPath[520] = {0,};
	size_t unLengthOfString = 0;
	DWORD dwResult = ERROR_SUCCESS;

	// �������� ��� ����
	GetCurrentDirectory( MAX_PATH, szCurrentProcessPath );

	GetLocalTime( &tSystemTime );
	_stprintf_s( szBackupFileName, MAX_PATH,
				 PRE_PHRASE_FORMAT,
				 tSystemTime.wYear, tSystemTime.wMonth, tSystemTime.wDay,
				 tSystemTime.wHour, tSystemTime.wMinute, tSystemTime.wSecond, tSystemTime.wMilliseconds,
				 DEFAULT_EXPORTFILENAME );

	_stprintf_s( szRegFileFullPath, 520, TEXT( "%s\\%s" ), szCurrentProcessPath, szBackupFileName );
	unLengthOfString = _tcsclen( szRegFileFullPath );  // ����� �ִ� ���� �˻�
	if( MAX_PATH < unLengthOfString )
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | The length of the file path is invalid. | %d\n" ),
						TEXT( __FUNCTION__ ),
						unLengthOfString );
		return false;
	}

	// szRegFileFullPath ���� ���� �˻�
	if( _taccess_s( szRegFileFullPath, 0 ) == 0 )
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | Exist the same file as file that want to backup. " )
						TEXT( "Because could not complete the backup, can not delete.\n" ),
						TEXT( __FUNCTION__ ) );
		return false;
	}

	// ��������
	if( ExportRegKeyToFile( lpszSubKey, szRegFileFullPath ) == false )
		return false;

	// �����
	dwResult = SHDeleteKey( HKEY_CURRENT_USER, lpszSubKey );
	if( dwResult != ERROR_SUCCESS )
	{
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, dwResult );
		return false;
	}

	return true;
}

VOID CEnvironmentValue::CloseRegKey( VOID )
{
	LONG lResult = ERROR_SUCCESS;

	lResult = RegCloseKey( m_hkKey );
	if( lResult != ERROR_SUCCESS )
		g_Log.PrintErrorMsg( TEXT( __FUNCTION__ ), true, lResult );
}

// TEST
LONG WINAPI CustomizedExcFilter( PEXCEPTION_POINTERS ptExceptionInfo )
{
	char szTest[256] = {0,};
	sprintf( szTest, "\nCustomizedExcFilter()!!\n" );
	OutputDebugStringA( szTest );
	printf( "\nCustomizedExcFilter()!!\n" );

	return EXCEPTION_CONTINUE_SEARCH;
}
// TEST
