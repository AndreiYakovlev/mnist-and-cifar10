// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "timer.h"

timer::timer() :countsPerSecond_(0), tickCount_(0), lastCount_(0), init_(false), pause_(true), maxDeltaTime_(0) {
	largeInteger_.QuadPart = 0;
}

bool timer::Init(double dtMax) {
	if (QueryPerformanceFrequency(&largeInteger_) == 0) return false;

	countsPerSecond_ = largeInteger_.QuadPart;

	if (QueryPerformanceCounter(&largeInteger_) == 0) return false;

	maxDeltaTime_ = dtMax;
	init_ = true;
	return true;
}

void timer::Start() {
	lastCount_ = largeInteger_.QuadPart;
	pause_ = false;
}

void timer::Pause() {
	pause_ = true;
}

void timer::Reset() {
	lastCount_ = tickCount_ = 0;
}

double timer::GetDeltaTime() {
	if (init_ && !pause_) {
		QueryPerformanceCounter(&largeInteger_);
		tickCount_ = (largeInteger_.QuadPart) - lastCount_;
		lastCount_ = largeInteger_.QuadPart;

		if (tickCount_ < 0) tickCount_ = 0;
		return min(double(tickCount_) / countsPerSecond_, maxDeltaTime_);
	}
	return 0;
}