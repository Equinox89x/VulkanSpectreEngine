#include "Timer.h"

void Timer::Init()
{
	m_BaseTime = Clock::now();
	m_SecondsPerCount = 1.0f / static_cast<float>(Clock::period::den);
}

void Timer::Update()
{
	if (m_IsStopped)
	{
		m_FPS = 0;
		m_DeltaTime = 0.0f;
		m_TotalTime = std::chrono::duration<float>(m_StopTime - m_BaseTime).count();
		return;
	}

	m_CurrentTime = Clock::now();
	m_DeltaTime = std::chrono::duration<float>(m_CurrentTime - m_PreviousTime).count();
	m_PreviousTime = m_CurrentTime;

	if (m_DeltaTime < 0.0f)
		m_DeltaTime = 0.0f;

	if (m_ForceElapsedUpperBound && m_DeltaTime > m_ElapsedUpperBound)
	{
		m_DeltaTime = m_ElapsedUpperBound;
	}

	m_TotalTime = std::chrono::duration<float>(m_CurrentTime - m_BaseTime).count();

	// FPS logic
	m_FPSTimer += m_DeltaTime;
	++m_FPSCount;
	if (m_FPSTimer >= 1.0f)
	{
		m_dFPS = m_FPSCount / m_FPSTimer;
		m_FPS = m_FPSCount;
		m_FPSCount = 0;
		m_FPSTimer = 0.0f;
	}
}

void Timer::Start()
{
	auto startTime = Clock::now();

	if (m_IsStopped)
	{
		m_PausedDuration += std::chrono::duration_cast<Duration>(startTime - m_StopTime);
		m_PreviousTime = startTime;
		m_StopTime = TimePoint();
		m_IsStopped = false;
	}
}

void Timer::Stop()
{
	if (!m_IsStopped)
	{
		m_CurrentTime = Clock::now();
		m_StopTime = m_CurrentTime;
		m_IsStopped = true;
	}
}