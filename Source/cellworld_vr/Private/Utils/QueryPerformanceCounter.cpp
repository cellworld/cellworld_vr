/*

Author: Felix A. Maldonado
Date  : January 28, 2023

Description:

	Unreal Engine 4.27 cannot handle LARGE_INTEGER (64-bit int) so we have a
	function that returns low and high 32-bit integers (uint32). This function uses LONGLONG
	QueryPerformanceCounter for a high-precision timer.
*/

#include "Utils/QueryPerformanceCounter.h"
#include "Windows/WindowsSystemIncludes.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h" 
#include "Windows/MinWindows.h" // smelnick98 https://forums.unrealengine.com/t/compile-error-when-including-windows-h/462619


/* to do -> update to remove need for highpart */

void splitLongLong(LONGLONG input, int32& high, int32& low) {
	// thank you ChatGPT (actually nvm, doesnt return what it says it should).
	high = (input >> 32) & 0xFFFFFFFF;
	low = input & 0xFFFFFFFF;
}

bool UQueryPerformanceCounter::GetCounter2(uint64& Counter)
{
	LARGE_INTEGER li;

	if (!QueryPerformanceCounter(&li)) {
		return false;
	}
	Counter = li.QuadPart;
	return true;
}

bool UQueryPerformanceCounter::GetQueryFrequency(uint64& frequency) {
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li)) {
		return false; 
	}

	frequency = li.QuadPart; // todo: confirm proper value 
	return true;
}

bool UQueryPerformanceCounter::GetQueryElapsedCounter(int32 t0, int32& t1)
{
	/* This function takes in input int32 QueryInit (same as QueryCounterStart) and finds the elapsed time by implementing a
	QueryPerformanceCounterFunction.
	*/

	uint64 tnow; 
	
	bool available = GetCounter2(tnow);
	if (!available) {
		t1 = -1; 
		return false;
	}
	//ElapsedTime = (UKismetMathLibrary::Subtract_IntInt(LowPart, QueryInit));
	t1 = (int32)tnow - t0;
	return true;
}

bool UQueryPerformanceCounter::GetQueryElapsedTime(uint64 t0, double& t1)
{
	/* This function takes in input int32 QueryInit (same as QueryCounterStart) and finds the elapsed time by implementing a
	QueryPerformanceCounterFunction.
	*/

	uint64 tnow;
	uint64 frequency; 

	if (!GetQueryFrequency(frequency)) {
		t1 = 0;
		return false;
	}

	if (!GetCounter2(tnow)) {
		t1 = 0;
		return false;
	}
	//ElapsedTime = (UKismetMathLibrary::Subtract_IntInt(LowPart, QueryInit));
	t1 = (double(tnow - t0)) / (double)frequency;
	return true;
}

