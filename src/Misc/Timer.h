#pragma once

#ifndef singleton
#include "Singleton.h"
#define singleton
#endif

#include <chrono>
#include <cstdint>

class Timer final : public Singleton<Timer>
{
public:
	void Init();
	void Update();
	void Start();
	void Stop();

	float	 GetDeltaTime() const { return m_DeltaTime; }
	uint32_t GetFPS() const { return m_FPS; }
	float	 GetTotalTime() const { return m_TotalTime; }

private:
	friend class Singleton<Timer>;
	Timer() = default;

	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;
	using Duration = Clock::duration;


	TimePoint m_BaseTime;
	Duration  m_PausedDuration = Duration::zero();

	TimePoint m_StopTime;
	TimePoint m_PreviousTime;
	TimePoint m_CurrentTime;

	uint32_t m_FPS = 0;
	float	 m_dFPS = 0.0f;
	uint32_t m_FPSCount = 0;

	float m_TotalTime = 0.0f;
	float m_DeltaTime = 0.0f;
	float m_SecondsPerCount = 0.0f;
	float m_ElapsedUpperBound = 0.03f;
	float m_FPSTimer = 0.0f;

	bool m_IsStopped = true;
	bool m_ForceElapsedUpperBound = false;
};