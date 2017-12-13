#include "core.h"


static LARGE_INTEGER get_clock_frequency()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency;
}

static bool try_set_sleep_granularity()
{
	UINT sleep_granularity_ms = 1;
	return timeBeginPeriod(sleep_granularity_ms) == TIMERR_NOERROR;
}

static LARGE_INTEGER g_timer_clock_frequency = get_clock_frequency(); // todo(jbr) move all globals to one place
static bool g_timer_sleep_granularity_was_set = try_set_sleep_granularity();


void timer_restart(Timer* timer)
{
	QueryPerformanceCounter(&timer->start);
}

Timer timer_create()
{
	Timer timer = {};
	timer_restart(&timer);
	return timer;
}

float32 timer_get_s(Timer* timer)
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	return (float32)(now.QuadPart - timer->start.QuadPart) / (float32)g_timer_clock_frequency.QuadPart;
}

void timer_wait_until(Timer* timer, float32 wait_time_s)
{
	float32 time_taken_s = timer_get_s(timer);

	while (time_taken_s < wait_time_s)
	{
		if (g_timer_sleep_granularity_was_set)
		{
			DWORD time_to_wait_ms = (DWORD)((wait_time_s - time_taken_s) * 1000);
			if (time_to_wait_ms > 0)
			{// todo(jbr) test this for possible oversleep
				Sleep(time_to_wait_ms);
			}
		}

		time_taken_s = timer_get_s(timer);
	}
}

void memory_allocator_create(Memory_Allocator* allocator, uint8* memory, uint32 size)
{
	allocator->memory = memory;
	allocator->next = memory;
	allocator->bytes_remaining = size;
}

uint8* memory_allocator_alloc(Memory_Allocator* allocator, uint32 size)
{
	assert(allocator->bytes_remaining >= size);
	uint8* mem = allocator->next;
	allocator->next += size;
	return mem;
}