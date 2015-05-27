#include <stdio.h>
#include <iostream>
#include <windows.h>
#include "PerformanceRecorder.h"
using namespace std;

enum
{
	PERF_SEG1 = 1,
	PERF_SEG2,
	PERF_SEG3,
	PERF_FOO,
	PERF_EMPTY,
};

static void foo()
{
PERF_BEGIN(PERF_FOO)
	double ff=1.0;
	for(int i=0;i<10000;i++)
		i=i;
	//Sleep(1000);
PERF_END(PERF_FOO)
}

void main()
{
	PerformanceRecorder::RegisterPerfName(PERF_SEG1,"MySeg1");
	PerformanceRecorder::RegisterPerfName(PERF_SEG2,"MySeg2");
	PerformanceRecorder::RegisterPerfName(PERF_SEG3,"MySeg3");
	PerformanceRecorder::RegisterPerfName(PERF_FOO,"PerfOfFoo");
	PerformanceRecorder::RegisterPerfName(PERF_EMPTY,"Empty Segment");

	PERF_BEGIN(PERF_SEG1)
		
		for(int i=0;i<10000;i++)
			foo();

	PERF_END(PERF_SEG1)

	PERF_BEGIN(PERF_SEG1)
		for(int i=0;i<100000;i++)
			foo();
	PERF_END(PERF_SEG1)

	PERF_BEGIN(PERF_EMPTY)
	PERF_END(PERF_EMPTY)

	PERF_BEGIN("MySeg3")
		Sleep(1000);
	PERF_END("MySeg3")

	PERF_BEGIN("sdf")
		Sleep(2000);
	PERF_END("sdf")


	PerformanceRecorder::PrintAllPerformance();
	cout<<endl;
	PerformanceRecorder::PrintAllSegmentCallTimes();


	system("pause");
}