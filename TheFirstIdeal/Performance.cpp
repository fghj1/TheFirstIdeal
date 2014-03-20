#include "stdafx.h"
#include "Performance.h"
#include "Log.h"

#ifdef _DEBUG


extern CLog g_Log;

CHighResPerfCtr::CHighResPerfCtr()
{
	Initialization();
}

CHighResPerfCtr::~CHighResPerfCtr()
{
}

// ���� ������ ���� �ʱ�ȭ
// ������ ���, ���� ���� �Ұ�
bool CHighResPerfCtr::InitPerfMeasurement( VOID )
{
	BOOL bDoesSupport = FALSE;

	// ���� ���� Ȯ�� �� ���ļ� ����
	bDoesSupport = QueryPerformanceFrequency( &m_liFrequency );
	if( bDoesSupport == FALSE )
	{
		g_Log.PrintErrorMsg( TEXT( "QueryPerformanceFrequency" ), true, GetLastError() );

		memset( &m_liFrequency, 0, sizeof( LARGE_INTEGER ) );
		return false;
	}

	return true;
}

// ���� ���� ����
VOID CHighResPerfCtr::StartPerfMeasurement( VOID )
{
	// ���� ���� Ȯ��
	if( m_liFrequency.QuadPart == 0 )
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | This computer don't support QueryPerformance function sets.\n" ),
						TEXT( __FUNCTION__ ) );
		return;
	}

	BOOL lResult = FALSE;

	// Performance-counter �� ����
	lResult = QueryPerformanceCounter( &m_liPerfCtrAsStart );
	if( lResult == FALSE )
		g_Log.PrintErrorMsg( TEXT( "QueryPerformanceCounter" ), true, GetLastError() );
}

// ���� ���� ���� �� ���� ��� ���
VOID CHighResPerfCtr::EndPerfMeasurement( VOID )
{
	// ���� ���� Ȯ��
	if( m_liFrequency.QuadPart == 0 )
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | This computer don't support QueryPerformance function sets.\n" ),
						TEXT( __FUNCTION__ ) );
		return;
	}

	BOOL lResult = FALSE;

	// Performance-counter �� ����
	lResult = QueryPerformanceCounter( &m_liPerfCtrAsEnd );
	if( lResult == FALSE )
	{
		g_Log.PrintErrorMsg( TEXT( "QueryPerformanceCounter" ), true, GetLastError() );
		return;
	}

	// ��� ���
	ResultOfPerfMeasurement();
}

VOID CHighResPerfCtr::Initialization( VOID )
{
	m_liFrequency.QuadPart = 0;
	m_liPerfCtrAsStart.QuadPart = 0;
	m_liPerfCtrAsEnd.QuadPart = 0;
}

VOID CHighResPerfCtr::ResultOfPerfMeasurement( VOID )
{
	double dDiffOfStartEnd = 0, dProcessedTime = 0;
	TCHAR szOutputMsg[256] = {0,};

	dDiffOfStartEnd = static_cast<double>( m_liPerfCtrAsEnd.QuadPart - m_liPerfCtrAsStart.QuadPart );
	dProcessedTime = ( dDiffOfStartEnd / m_liFrequency.QuadPart ) * 1000000;  // micro sec.

	_stprintf_s( szOutputMsg, 256,
				 TEXT( "Elapsed time:%8.16f micro sec. = ( ( %I64d - %I64d ) / %I64d * 1000000 )\n" ),
				 dProcessedTime,
				 m_liPerfCtrAsEnd.QuadPart,
				 m_liPerfCtrAsStart.QuadPart,
				 m_liFrequency.QuadPart );
	OutputDebugString( szOutputMsg );
}


#endif
