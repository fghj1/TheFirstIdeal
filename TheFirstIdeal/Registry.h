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

	// Registry key ���� �� ����
	bool OpenRegKey( LPCTSTR in_lpszSubKey );

	// Registry key ����
	bool WriteRegKey( LPCTSTR in_lpszValName,
					  DWORD in_dwValType,
					  const BYTE* in_pbyDataBuffer,
					  DWORD in_dwSizeOfDataBuffer );

	// Registry key �б�
	// ����!! in_lpszValName ���ڸ� �Ҵ��Ͽ� ȣ���� ���
	//		  ���� ����, m_lpvValueBuffer�� m_dwSizeOfValueBuffer�� ���� ���� �Ҵ�ǹǷ�
	//		  �̿� �����ϱ� ���ؼ� GetInsideValueBuffer()�� GetInsideSizeOfValueBuffer()��
	//		  ȣ���ؾ� �Ѵ�.
	bool ReadRegKey( LPCTSTR in_lpszValName,
					 LPDWORD inout_pdwSizeOfProvidedBuffer = NULL,
					 LPBYTE inout_pbyProvidedBuffer = NULL );

	// �������� Registry key type ����
	DWORD GetRegValType( LPCTSTR in_lpszValName );

	// �б⿡ �ʿ��� ũ�� ����
	DWORD GetNeedfulSizeOfDataBuffer( LPCTSTR in_lpszValName );

	// ��������
	// in_lpszProvidedFullPath : "C:\Download\*.reg"
	bool ExportRegKeyToFile( LPCTSTR in_lpszSubKey,
							 LPCTSTR in_lpszProvidedFullPath );

	// ��������
	// in_lpszProvidedFullPath : "C:\Download\*.reg"
	bool ImportRegKeyFromFile( LPCTSTR in_lpszProvidedFullPath );

	// Registry key �����
	// ����� ���� �ڵ����� '��������' ó���� �Ѵ�.
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
