#pragma once

#include <Windows.h>

#if COMPILING_DLL
#define LIB_API __declspec(dllexport)
#else
#define LIB_API __declspec(dllimport)
#endif

class LIB_API timer
{
	__int64 countsPerSecond_;
	__int64 lastCount_;
	__int64 tickCount_;
	LARGE_INTEGER largeInteger_;
	bool init_;
	bool pause_;
	double maxDeltaTime_;

public:
	timer();
	bool Init(double dtMax);
	void Start();
	void Pause();
	void Reset();
	//if timer not started then return 0
	double GetDeltaTime();
};
