#include "PerformanceRecorder.h"
#include <iostream>
using namespace std;

static __int64 s_iFreq;
static PerformanceRecorder s_PerfRecorder;

#ifdef WIN32
	#include <windows.h>

	static bool calFrequency()
	{
		LARGE_INTEGER _freq, _t_begin, _t_end;

		::QueryPerformanceFrequency(&_freq);
		::QueryPerformanceCounter(&_t_begin);

		LONGLONG cntr1=(LONGLONG)GetCycleCount();
		::Sleep(1000);
		LONGLONG cntr2=(LONGLONG)GetCycleCount();

		::QueryPerformanceCounter(&_t_end);

		//s_iFreq=__int64( (cntr2 - cntr1) * _freq.QuadPart / (_t_end.QuadPart - _t_begin.QuadPart) );
		s_iFreq=__int64( (cntr2 - cntr1) );

		return true;

	}

	static bool calFrequency1(int iSleepTime)
	{
		HANDLE hp = ::GetCurrentProcess(); //��ȡ��ǰ����
		HANDLE ht = ::GetCurrentThread(); //��ȡ��ǰ�߳�
		DWORD  pc = ::GetPriorityClass( hp ); //��ȡ��ǰ�������ȶ�
		DWORD  tp = ::GetThreadPriority( ht ); //��ȡ��ǰ�߳����ȶ�
		BOOL   flag1 = FALSE , flag2 = FALSE;
		DWORD  low1  = 0 , high1 = 0 , low2 = 0 , high2 = 0;
		flag1 = ::SetPriorityClass( hp , REALTIME_PRIORITY_CLASS );  //����ǰ�������ȶ��������
		flag2 = ::SetThreadPriority( ht , THREAD_PRIORITY_HIGHEST ); //����ǰ�߳����ȶ��������

		::Sleep( 10 );
		LARGE_INTEGER fq,st,ed;

		::QueryPerformanceFrequency(&fq); //��ȷ��ʱ
		::QueryPerformanceCounter(&st);  //�����ʼʱ��
		__asm        //��õ�ǰcpu��ʱ��������
		{
		  rdtsc
		  mov low1 , eax
		  mov high1 , edx
		}
		::Sleep( iSleepTime );     //���̹߳���Ƭ��
		::QueryPerformanceCounter(&ed);  //��ý���ʱ��
		__asm        //��õ�ǰcpu��ʱ��������
		{
		  rdtsc
		  mov low2  ,eax
		  mov high2 ,edx
		}
		if( flag1 )
		  ::SetPriorityClass( hp , pc );  //����ǰ�������ȶȻָ�ԭ��
		if( flag2 )
		  ::SetThreadPriority( ht , tp );  //����ǰ�߳����ȶȻָ�ԭ��
		::CloseHandle( hp );
		::CloseHandle( ht );
		//��cpu��ʱ��������ת����64λ����
		LONGLONG begin = (LONGLONG)high1<<32 | low1;
		LONGLONG end = (LONGLONG)high2<<32 | low2;

		s_iFreq = (end - begin)*fq.QuadPart/(ed.QuadPart-st.QuadPart);

		return true;
	}

	static bool calFrequency_winPerf()
	{
		LARGE_INTEGER freq;

		::QueryPerformanceFrequency(&freq);
		s_iFreq = (__int64)freq.QuadPart;

		return true;
	}

	#ifdef USE_RDTSC
		#define CALFREQUENCY() calFrequency1(1000)
	#else
		#define CALFREQUENCY() calFrequency_winPerf()
	#endif

#else
	//Mac or other OS
	#define CALFREQUENCY() false
#endif


PerformanceRecorder::PerformanceRecorder()
{
	memset(m_iCPUTickCounts,0,256*sizeof(__int64));
	memset(m_iSegCallTimes,0,256*sizeof(__int64));
	
	s_PerfRecorder.m_PerfNameMap["AllUnnamed"]=255;
	CALFREQUENCY();
}

PerformanceRecorder::~PerformanceRecorder()
{
}

PerformanceRecorder &PerformanceRecorder::GetInstance()
{
	return s_PerfRecorder;
}


bool PerformanceRecorder::RegisterPerfName(int iPerfId, const char *sz_PerfName)
{
	if(iPerfId<0 || iPerfId>254 || !sz_PerfName)
		return false;

	s_PerfRecorder.m_PerfNameMap[sz_PerfName]=iPerfId;

	return true;
}

int PerformanceRecorder::GetPerfIdByName(const char *sz_PerfName)
{
	if(!sz_PerfName)
		return -1;

	PERFNAMEMAP::iterator iter=s_PerfRecorder.m_PerfNameMap.find(sz_PerfName);

	if(iter==s_PerfRecorder.m_PerfNameMap.end())
		return -1;

	return s_PerfRecorder.m_PerfNameMap[sz_PerfName];
}

__int64	PerformanceRecorder::GetSegGallTimes(int iPerfId)
{
	return s_PerfRecorder.m_iSegCallTimes[iPerfId]>>1;
}

__int64	PerformanceRecorder::GetSegGallTimes(const char *sz_PerfName)
{
	return GetSegGallTimes( GetPerfIdByName(sz_PerfName) )>>1;
}

__int64 PerformanceRecorder::GetEmptyTickCount()
{
	s_PerfRecorder.m_iCPUTickCounts[257]=0;

	PERF_BEGIN(257)
	PERF_END(257)

	return s_PerfRecorder.m_iCPUTickCounts[257];
}

double PerformanceRecorder::GetPerformance(int iPerfId)
{
	//return double(s_PerfRecorder.m_iCPUTickCounts[iPerfId] - GetSegGallTimes(iPerfId)*GetEmptyTickCount() )/s_iFreq;
	return double(s_PerfRecorder.m_iCPUTickCounts[iPerfId])/s_iFreq;
}

double PerformanceRecorder::GetPerformance(const char *sz_PerfName)
{
	return GetPerformance( GetPerfIdByName(sz_PerfName) );
}

void PerformanceRecorder::PrintAllPerformance()
{
	PERFNAMEMAP::iterator iter = s_PerfRecorder.m_PerfNameMap.begin();

	cout<<"Time consuming of segments:	(second)"<<endl<<endl;
	for(;iter!=s_PerfRecorder.m_PerfNameMap.end();iter++)
	{
		double fPerf = GetPerformance( iter->second );
		__int64 iCallTimes = GetSegGallTimes( iter->second );

		if( iCallTimes != 0 )
		{
			cout<<"Segment '"<<iter->first<<"':		"<<fPerf<<endl;
		}
	}

	//double fPerf = GetPerformance( 255 );
	//if( fPerf > 0 )
	//	cout<<"Time consuming of all unnamed segments: "<<fPerf<<endl;

}

void PerformanceRecorder::PrintAllSegmentCallTimes()
{
	PERFNAMEMAP::iterator iter = s_PerfRecorder.m_PerfNameMap.begin();

	cout<<"Times of segments called:"<<endl<<endl;
	for(;iter!=s_PerfRecorder.m_PerfNameMap.end();iter++)
	{
		__int64 iCallTimes = GetSegGallTimes( iter->second );

		if( iCallTimes != 0 )
		{
			cout<<"Segment '"<<iter->first<<"':		"<<iCallTimes<<endl;
		}
	}

}