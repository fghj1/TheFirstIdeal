#pragma once

#ifdef _DEBUG


class CHighResPerfCtr
{
public:
	CHighResPerfCtr();
	~CHighResPerfCtr();

	// ���� ������ ���� �ʱ�ȭ
	// ������ ���, ���� ���� �Ұ�
	bool InitPerfMeasurement( VOID );

	// ���� ���� ����
	VOID StartPerfMeasurement( VOID );

	// ���� ���� ���� �� ���� ��� ���
	VOID EndPerfMeasurement( VOID );

private:
	LARGE_INTEGER m_liFrequency;
	LARGE_INTEGER m_liPerfCtrAsStart;
	LARGE_INTEGER m_liPerfCtrAsEnd;

	VOID Initialization( VOID );
	VOID ResultOfPerfMeasurement( VOID );
};


#endif
