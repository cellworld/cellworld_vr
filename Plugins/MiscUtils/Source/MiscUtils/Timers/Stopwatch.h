#pragma once

class Stopwatch
{
public:
	struct FStopWatch
	{
	public:
		// Function to start the timer
		void Start()
		{
			StartTime = FPlatformTime::Seconds();
			bRunning = true;
		}

		// Function to stop the timer and calculate elapsed time
		void Stop()
		{
			EndTime = FPlatformTime::Seconds();
			bRunning = false;
		}

		// Function to get the elapsed time in seconds
		float GetElapsedTime() const
		{
			if (bRunning) { return FPlatformTime::Seconds() - StartTime; }
			
			return static_cast<float>(EndTime - StartTime);
		}

	private:
		// Variables to store the start and end times
		bool bRunning = false;
		double StartTime = 0.0f;
		double EndTime = 0.0f;
	};

};
