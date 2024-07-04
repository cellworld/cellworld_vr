#pragma once

class GenericClock
{
public:
	struct FStopWatch
	{
	public:
		// Function to start the timer
		void Start()
		{
			StartTime = FPlatformTime::Seconds();
		}

		// Function to stop the timer and calculate elapsed time
		void Stop()
		{
			EndTime = FPlatformTime::Seconds();
		}

		// Function to get the elapsed time in seconds
		float GetElapsedTime() const
		{
			return static_cast<float>(EndTime - StartTime);
		}

	private:
		// Variables to store the start and end times
		double StartTime = 0.0f;
		double EndTime = 0.0f;
	};
};
