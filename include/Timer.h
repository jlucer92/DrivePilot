#ifndef _TIMER_H_
#define _TIMER_H_

#include <time.h>
#include <errno.h>
#include <signal.h>
#include <iostream>

#define ONE_SECOND 1000000L
#define HALF_SECOND 500000L

using namespace std;

// A wrapper around the posix timer system. It implements
// a downwards ticking one-shot timer that expires after the 
// specified time.
class Timer
{
	public:
	int timerCreate();
	int timerDelete();
	int timerSet(const time_t sec, const long nsec);
	int timerSet(const long usecs);
	int timerUnset(); // disable the timer
	int timerGet(time_t &sec, long &nsec);
	bool timerExpired();

	private:
	timer_t timerid;
};

// A wrapper around the posix timer system. It implements
// a downwards ticking periodic timer that expires after the 
// specified time and calls a callback function.
class PeriodicTimer
{
	public:
	int timerCreate(long threadId = -1, int signalNum = SIGALRM);
	int timerDelete();
	int timerSet(const time_t sec, const long nsec);
	int timerSet(const long usecs);
	int timerUnset(); // disable the timer
	int timerGet(time_t &sec, long &nsec);
	int timerWait(long sec = -1, long nsec = -1);
	int timerIntervalGet(time_t &sec, long &nsec);
	bool timerEnabled();

	private:
	timer_t timerid;
	int signum;
	sigset_t set;
};

// An upwards counting stopwatch-like timer that can be used
// for timing system performance.
class StopWatch
{
	public:
	int start();
	int stop();
	long report();

	private:
	timespec start_t;
	timespec stop_t;
};

// The most accurate real-time sleeper:
extern int rt_sleep(const time_t sec, const long nsec);
extern int rt_sleep(const long usec);

// Utility functions:
extern void usec_2_sec_nsec(long usec, time_t& sec, long& nsec);

#endif