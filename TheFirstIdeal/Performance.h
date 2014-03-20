#pragma once

#ifdef _DEBUG


class CHighResPerfCtr
{
public:
	CHighResPerfCtr();
	~CHighResPerfCtr();

	// 성능 측정을 위한 초기화
	// 실패할 경우, 성능 측정 불가
	bool InitPerfMeasurement( VOID );

	// 성능 측정 시작
	VOID StartPerfMeasurement( VOID );

	// 성능 측정 종료 및 측정 결과 출력
	VOID EndPerfMeasurement( VOID );

private:
	LARGE_INTEGER m_liFrequency;
	LARGE_INTEGER m_liPerfCtrAsStart;
	LARGE_INTEGER m_liPerfCtrAsEnd;

	VOID Initialization( VOID );
	VOID ResultOfPerfMeasurement( VOID );
};


#endif
