#include "Clock.h"

MTClock::MTClock()
{
	PrevTime = std::chrono::high_resolution_clock::now();
	CurrentTime = PrevTime;
}

void MTClock::Update()
{
	PrevTime = CurrentTime;
	CurrentTime = std::chrono::high_resolution_clock::now();
}

float MTClock::GetDeltaSeconds() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - PrevTime).count() / 1000.0f;
}

float MTClock::GetDeltaMilliSeconds() const
{
	return std::chrono::duration_cast<std::chrono::microseconds>(CurrentTime - PrevTime).count() / 1000.0f;
}
