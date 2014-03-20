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

// 성능 측정을 위한 초기화
// 실패할 경우, 성능 측정 불가
bool CHighResPerfCtr::InitPerfMeasurement( VOID )
{
	BOOL bDoesSupport = FALSE;

	// 지원 여부 확인 및 주파수 추출
	bDoesSupport = QueryPerformanceFrequency( &m_liFrequency );
	if( bDoesSupport == FALSE )
	{
		g_Log.PrintErrorMsg( TEXT( "QueryPerformanceFrequency" ), true, GetLastError() );

		memset( &m_liFrequency, 0, sizeof( LARGE_INTEGER ) );
		return false;
	}

	return true;
}

// 성능 측정 시작
VOID CHighResPerfCtr::StartPerfMeasurement( VOID )
{
	// 지원 여부 확인
	if( m_liFrequency.QuadPart == 0 )
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | This computer don't support QueryPerformance function sets.\n" ),
						TEXT( __FUNCTION__ ) );
		return;
	}

	BOOL lResult = FALSE;

	// Performance-counter 값 추출
	lResult = QueryPerformanceCounter( &m_liPerfCtrAsStart );
	if( lResult == FALSE )
		g_Log.PrintErrorMsg( TEXT( "QueryPerformanceCounter" ), true, GetLastError() );
}

// 성능 측정 종료 및 측정 결과 출력
VOID CHighResPerfCtr::EndPerfMeasurement( VOID )
{
	// 지원 여부 확인
	if( m_liFrequency.QuadPart == 0 )
	{
		g_Log.WriteLog( false,
						LOG_CLASS_WARNING,
						TEXT( "%s | This computer don't support QueryPerformance function sets.\n" ),
						TEXT( __FUNCTION__ ) );
		return;
	}

	BOOL lResult = FALSE;

	// Performance-counter 값 추출
	lResult = QueryPerformanceCounter( &m_liPerfCtrAsEnd );
	if( lResult == FALSE )
	{
		g_Log.PrintErrorMsg( TEXT( "QueryPerformanceCounter" ), true, GetLastError() );
		return;
	}

	// 결과 출력
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
