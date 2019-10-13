#pragma implementation
#include <ptlib.h>
#include <pthread.h>
#include <ptlib/svcproc.h>
#include "condvar.hpp"
PCondVar :: PCondVar ( PMutex & mut ) : m ( mut ) { }

#define PAssertPTHREAD(func, args) \
	{ \
	unsigned threadOpRetry = 0; \
	while (PAssertThreadOp(func args, threadOpRetry, #func, __FILE__, __LINE__)); \
	}

static BOOL PAssertThreadOp(int retval,
                            unsigned & retry,
                            const char * funcname,
                            const char * file,
                            unsigned line)
{
  if (retval == 0) {
    PTRACE_IF(2, retry > 0, "PWLib\t" << funcname << " required " << retry << " retries!");
    return FALSE;
  }

  if (errno == EINTR || errno == EAGAIN) {
    if (++retry < 1000) {
#if defined(P_RTEMS)
      rtems_task_wake_after(10);
#else
      usleep(10000); // Basically just swap out thread to try and clear blockage
#endif
      return TRUE;   // Return value to try again
    }
    // Give up and assert
  }

  PAssertFunc(file, line, NULL, psprintf("Function %s failed", funcname));
  return FALSE;
}

void PCondVar :: Wait ( ) {
	m.ownerThreadId = (pthread_t)-1;
	PSYSTEMLOG ( Info, "wait start" );
	PAssertPTHREAD ( pthread_cond_wait, ( & condVar, & m.mutex ) );
	PSYSTEMLOG ( Info, "wait stop" );
}

BOOL PCondVar :: Wait ( const PTimeInterval & waitTime ) {
	PTime finishTime;
	finishTime += waitTime;
	struct timespec absTime;
	absTime.tv_sec  = finishTime.GetTimeInSeconds ( );
	absTime.tv_nsec = finishTime.GetMicrosecond ( ) * 1000;
	m.ownerThreadId = (pthread_t)-1;
	pthread_t currentThreadId = pthread_self ( );
//	PSYSTEMLOG ( Info, "timedwait start" );
	int err = pthread_cond_timedwait ( & condVar, & m.mutex, & absTime );
//	PSYSTEMLOG ( Info, "timedwait stop" );
	m.ownerThreadId = currentThreadId;
	if ( err == 0 || err == ETIMEDOUT )
		return err == 0;
	PAssertOS ( err == EINTR && errno == EINTR );
	return false;
}

void PCondVar :: Signal ( ) {
//	PSYSTEMLOG ( Info, "cond signal" );
	PAssertPTHREAD ( pthread_cond_signal, ( & condVar ) );
//	m.ownerThreadId = pthread_self ( );
//	PSYSTEMLOG ( Info, "unlockras signal" );
//	m.Signal ( );
}

BOOL PCondVar :: WillBlock ( ) const {
	return true;
}
