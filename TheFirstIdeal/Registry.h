#pragma once

#define SUBKEY TEXT( "SOFTWARE\\fghj1" )
#define SUBKEY_OPTION TEXT( "SOFTWARE\\fghj1\\TheFirstIdeal\\0.1\\Option" )
#define VALUENAME_SUBSCREENBUFFER TEXT( "SubScreenBuffer" )

#define DEFAULT_EXPORTFILENAME TEXT( "EnvironmentValue.reg" )
#define PRE_PHRASE_FORMAT TEXT( "BACKUP_%.4d%.2d%.2d_%.2d%.2d%.2d%.4d_%s" )


class CEnvironmentValue
{
public:
	CEnvironmentValue();
	~CEnvironmentValue();

	// Registry key 생성 및 열기
	bool OpenRegKey( LPCTSTR in_lpszSubKey );

	// Registry key 쓰기
	bool WriteRegKey( LPCTSTR in_lpszValName,
					  DWORD in_dwValType,
					  const BYTE* in_pbyDataBuffer,
					  DWORD in_dwSizeOfDataBuffer );

	// Registry key 읽기
	// 주의!! in_lpszValName 인자만 할당하여 호출할 경우
	//		  내부 변수, m_lpvValueBuffer과 m_dwSizeOfValueBuffer에 읽은 값이 할당되므로
	//		  이에 접근하기 위해서 GetInsideValueBuffer()와 GetInsideSizeOfValueBuffer()을
	//		  호출해야 한다.
	bool ReadRegKey( LPCTSTR in_lpszValName,
					 LPDWORD inout_pdwSizeOfProvidedBuffer = NULL,
					 LPBYTE inout_pbyProvidedBuffer = NULL );

	// 읽으려는 Registry key type 조사
	DWORD GetRegValType( LPCTSTR in_lpszValName );

	// 읽기에 필요한 크기 산출
	DWORD GetNeedfulSizeOfDataBuffer( LPCTSTR in_lpszValName );

	// 내보내기
	// in_lpszProvidedFullPath : "C:\Download\*.reg"
	bool ExportRegKeyToFile( LPCTSTR in_lpszSubKey,
							 LPCTSTR in_lpszProvidedFullPath );

	// 가져오기
	// in_lpszProvidedFullPath : "C:\Download\*.reg"
	bool ImportRegKeyFromFile( LPCTSTR in_lpszProvidedFullPath );

	// Registry key 지우기
	// 지우기 전에 자동으로 '내보내기' 처리를 한다.
	bool DeleteRegKey( LPCTSTR in_lpszSubKey );

	inline const LPVOID GetInsideValueBuffer( VOID ) const { return m_lpvValueBuffer; }
	inline DWORD GetInsideSizeOfValueBuffer( VOID ) const { return m_dwSizeOfValueBuffer; }

private:
	HKEY m_hkKey;
	LPVOID m_lpvValueBuffer;
	DWORD m_dwSizeOfValueBuffer;

	VOID CloseRegKey( VOID );
};

// TEST
LONG WINAPI CustomizedExcFilter( PEXCEPTION_POINTERS ptExceptionInfo );
// TEST
