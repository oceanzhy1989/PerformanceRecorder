#ifndef __PERFORMANCERECORDER_H__
#define __PERFORMANCERECORDER_H__

#pragma once

#define USE_RDTSC

#define PERF_BEGIN(x)	do{ PerfRecorderWrapper __local_perf_wrapper(x);
#define PERF_END(x)		} while(0);

#include <map>
#include <string>
using namespace std;

//Accurate Time Counter
#ifdef USE_RDTSC
	inline unsigned __int64 GetCycleCount()
	{
		__asm
		{
			_emit 0x0F;
			_emit 0x31;
		}
	}
#else
	#ifdef WIN32
		#include <windows.h>
		inline unsigned __int64 GetCycleCount()
		{
			LARGE_INTEGER cur;
			::QueryPerformanceCounter(&cur);

			return (unsigned __int64)cur.QuadPart;
		}
	#else
	#endif
#endif

class PerformanceRecorder
{
public:

	static PerformanceRecorder	&GetInstance();

	static bool					RegisterPerfName(int iPerfId, const char *sz_PerfName);
	static int					GetPerfIdByName(const char *sz_PerfName);

	static __int64				GetSegGallTimes(int iPerfId);
	static __int64				GetSegGallTimes(const char *sz_PerfName);

	static __int64				GetEmptyTickCount();
	static double				GetPerformance(int iPerfId);
	static double				GetPerformance(const char *sz_PerfName);

	static void					PrintAllPerformance();
	static void					PrintAllSegmentCallTimes();

public:

	PerformanceRecorder();
	virtual ~PerformanceRecorder();
	
	inline void AddTickCount(int iPerfId, __int64 iTickCount)
	{
		m_iCPUTickCounts[iPerfId]+=iTickCount;
		m_iSegCallTimes[iPerfId]++;
	}	

protected:

	typedef map<string,int> PERFNAMEMAP;
	PERFNAMEMAP m_PerfNameMap;

	__int64 m_iCPUTickCounts[257];
	__int64 m_iSegCallTimes[257];

	//Preserved
	typedef map<string,__int64> UNREGISTEREDPERFMAP;
	UNREGISTEREDPERFMAP m_mCPUTickCntMap;
};


class PerfRecorderWrapper
{
public:

	inline PerfRecorderWrapper(int iPerfId=255) : m_iPerfId(iPerfId) , m_perfRcdrInst(PerformanceRecorder::GetInstance())
	{
		//Discard empty ticks
		m_iEmptyTickNum=GetCycleCount();
		m_perfRcdrInst.AddTickCount(m_iPerfId, m_iEmptyTickNum-GetCycleCount());

		m_iStartTickNum=GetCycleCount();
	}

	inline PerfRecorderWrapper(const char *sz_PerfName) : m_iPerfId(PerformanceRecorder::GetPerfIdByName(sz_PerfName)), 
														  m_perfRcdrInst(PerformanceRecorder::GetInstance())
	{
		if(m_iPerfId<0)
			m_iPerfId=255;

		m_iEmptyTickNum=GetCycleCount();
		m_perfRcdrInst.AddTickCount(m_iPerfId, m_iEmptyTickNum-GetCycleCount());
		
		
		m_iStartTickNum=GetCycleCount();
	}

	inline ~PerfRecorderWrapper()
	{
		m_perfRcdrInst.AddTickCount(m_iPerfId, GetCycleCount()-m_iStartTickNum);
	}

protected:

	int m_iPerfId;
	unsigned __int64 m_iStartTickNum;
	unsigned __int64 m_iEmptyTickNum;

	PerformanceRecorder &m_perfRcdrInst;
};

#endif