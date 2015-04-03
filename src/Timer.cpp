#include "Timer.h"

int Timer::timerCreate()
{
	// Create monotonic clock. It does not rely on the
	// system-wide real-time "settable" clock.
	// The monotonic clock is unsettable and is
	// good for relative timing...
	// Don't send signal on timer notify:
	struct sigevent sevp;
	sevp.sigev_notify = SIGEV_NONE;

	if( timer_create(CLOCK_MONOTONIC, &sevp, &timerid) < 0)
	{
		return -1;
	}

	return 0;
}

int Timer::timerDelete()
{
	return timer_delete(timerid);
}

int Timer::timerSet(const time_t sec, const long nsec)
{
	struct itimerspec timer;

	// Disable interval timing, we want a "one-shot" timer:
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_nsec = 0;

	// Set timer to expire at desired relative time:
	timer.it_value.tv_sec = sec;
	timer.it_value.tv_nsec = nsec;

	if( timer_settime(timerid, 0, &timer, NULL) < 0 )
	{
		return -1;
	}

	return 0;
}

int Timer::timerSet(const long usec)
{
	struct itimerspec timer;

	// Disable interval timing, we want a "one-shot" timer:
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_nsec = 0;

	// Set timer to expire at desired relative time:
	timer.it_value.tv_sec = usec / 1000000;
	timer.it_value.tv_nsec = (usec % 1000000) * 1000;

	if( timer_settime(timerid, 0, &timer, NULL) < 0 )
	{
		return -1;
	}

	return 0;
}

int Timer::timerUnset()
{
	struct itimerspec timer;

	// Disable timer:
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_nsec = 0L;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_nsec = 0L;

	if( timer_settime(timerid, 0, &timer, NULL) < 0 )
	{
		return -1;
	}

	return 0;
}

int Timer::timerGet(time_t &sec, long &nsec)
{
	struct itimerspec timer;

	if( timer_gettime(timerid, &timer) < 0)
	{
		return -1;
	}

	sec = timer.it_value.tv_sec;
	nsec = timer.it_value.tv_nsec;

	return 0;
}

bool Timer::timerExpired()
{
	time_t sec;
	long nsec;

	if( timerGet( sec, nsec) < 0 )
	{
		return true; // on an error, assume timer has expired so we don't block forever
	}

	if( (sec > 0) || (nsec > 0) )
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

int PeriodicTimer::timerCreate(long threadId, int signalNum)
{
	// Create monotonic clock. It does not rely on the
	// system-wide real-time "settable" clock.
	// The monotonic clock is unsettable and is
	// good for relative timing...
	// Send a signal on timer expiration:
	struct sigevent sevp;
	if( threadId >= 0 )
	{
		// If thread id is given notify only 
		// that specific thread.
		sevp.sigev_notify = SIGEV_THREAD_ID;
		sevp._sigev_un._tid = threadId; 
		// same as sigev_notify_thread_id
		// see here: http://stackoverflow.com/questions/16826898/error-struct-sigevent-has-no-member-named-sigev-notify-thread-id
	}
	else
	{
		// Otherwise notify the entire process.
		sevp.sigev_notify = SIGEV_SIGNAL;
	}

	// Send an alarm signal:
	signum = signalNum;
	sevp.sigev_signo = signum;

	// Allow alarm signal to be caught:
	sigemptyset(&set);
	sigaddset(&set, signum);
	sigprocmask(SIG_BLOCK, &set, NULL);

	if( timer_create(CLOCK_MONOTONIC, &sevp, &timerid) < 0)
	{
		return -1;
	}

	return 0;
}

int PeriodicTimer::timerDelete()
{
	// Disallow alarm signal to be caught:
	sigemptyset(&set);
	sigaddset(&set, signum);
	sigprocmask(SIG_UNBLOCK, &set, NULL);

	return timer_delete(timerid);
}

int PeriodicTimer::timerSet(const time_t sec, const long nsec)
{
	struct itimerspec timer;

	// Enable interval timing, we want a "periodic" timer:
	timer.it_interval.tv_sec = sec;
	timer.it_interval.tv_nsec = nsec;

	// Set timer to expire at desired relative time:
	timer.it_value.tv_sec = sec;
	timer.it_value.tv_nsec = nsec;

	if( timer_settime(timerid, 0, &timer, NULL) < 0 )
	{
		return -1;
	}

	return 0;
}

int PeriodicTimer::timerSet(const long usec)
{
	struct itimerspec timer;

	// Enable interval timing, we want a "periodic" timer:
	timer.it_interval.tv_sec = usec / 1000000;
	timer.it_interval.tv_nsec = (usec % 1000000) * 1000;

	// Set timer to expire at desired relative time:
	timer.it_value.tv_sec = usec / 1000000;
	timer.it_value.tv_nsec = (usec % 1000000) * 1000;

	if( timer_settime(timerid, 0, &timer, NULL) < 0 )
	{
		return -1;
	}

	return 0;
}

int PeriodicTimer::timerUnset()
{
	struct itimerspec timer;

	// Disable timer:
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_nsec = 0L;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_nsec = 0L;

	if( timer_settime(timerid, 0, &timer, NULL) < 0 )
	{
		return -1;
	}

	return 0;
}


int PeriodicTimer::timerGet(time_t &sec, long &nsec)
{
	struct itimerspec timer;

	if( timer_gettime(timerid, &timer) < 0)
	{
		return -1;
	}

	sec = timer.it_value.tv_sec;
	nsec = timer.it_value.tv_nsec;

	return 0;
}

int PeriodicTimer::timerIntervalGet(time_t &sec, long &nsec)
{
	struct itimerspec timer;

	if( timer_gettime(timerid, &timer) < 0)
	{
		return -1;
	}

	sec = timer.it_interval.tv_sec;
	nsec = timer.it_interval.tv_nsec;

	return 0;
}

bool PeriodicTimer::timerEnabled()
{
	time_t sec;
	long nsec;

	if( timerIntervalGet( sec, nsec ) < 0 )
	{
		return false;
	}

	if( (sec > 0) || (nsec > 0) )
	{
		return true;
	}

	return false;
}

int PeriodicTimer::timerWait(long sec, long nsec)
{
	struct timespec timeout;
	int sig;

	if( sec >= 0 || nsec >= 0 )
	{
		timeout.tv_sec = sec;
		timeout.tv_nsec = nsec;

		if( (sig = sigtimedwait(&set, NULL, &timeout)) < 0 )
		{
			if( errno == EAGAIN )
			{
				return 0;
			}

			return -1;
		}

		return sig;
	}
	else
	{
		return sigwaitinfo(&set, NULL);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
int StopWatch::start()
{ 
	return clock_gettime(CLOCK_MONOTONIC, &start_t); 
}

// stop timing functions
int StopWatch::stop()
{ 
	return clock_gettime(CLOCK_MONOTONIC, &stop_t); 
}

// get data functions:
long StopWatch::report()
{ 
	return (stop_t.tv_sec*1000000000 + stop_t.tv_nsec) - (start_t.tv_sec*1000000000 + start_t.tv_nsec);
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
int rt_sleep(const time_t sec, const long nsec)
{
	struct timespec ts;
    ts.tv_sec = sec;
    ts.tv_nsec = nsec;

    int status;
    do {
        status = clock_nanosleep(CLOCK_MONOTONIC, 
        							0, &ts, &ts);
    } while(status == EINTR); 
    // Continue sleeping if interupted by signal handler
    return status;
}

int rt_sleep(const long usec)
{
	struct timespec ts;
	ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;

    int status;
    do {
        status = clock_nanosleep(CLOCK_MONOTONIC, 
        							0, &ts, &ts);
    } while(status == EINTR); 
    // Continue sleeping if interupted by signal handler
    return status;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void usec_2_sec_nsec(long usec, time_t& sec, long& nsec)
{
	sec = usec / 1000000;
	nsec = (usec % 1000000) * 1000;
}