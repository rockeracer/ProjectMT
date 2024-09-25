#pragma once

#include <chrono>

class MTClock
{
public:
	CORE_API MTClock();
    
	CORE_API void Update();
	CORE_API float GetDeltaSeconds() const;
	CORE_API float GetDeltaMilliSeconds() const;
    
private:
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint;
    TimePoint PrevTime;
    TimePoint CurrentTime;
};
