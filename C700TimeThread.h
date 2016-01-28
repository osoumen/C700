//
//  C700TimeThread.h
//  C700
//
//  Created by osoumen on 2016/01/03.
//
//

#ifndef C700_C700TimeThread_h
#define C700_C700TimeThread_h

#ifdef _MSC_VER

#include <Windows.h>
// QueryPerformanceCounter による時間計測(Windows)
typedef long long MSTime;
typedef LARGE_INTEGER OSTime;
inline MSTime calcusTime(const OSTime &end, const OSTime &st) {
	LARGE_INTEGER nFreq;
	QueryPerformanceFrequency(&nFreq);
	return (MSTime)((end.QuadPart - st.QuadPart) * 1000000 / nFreq.QuadPart);
}
inline void getNowOSTime(OSTime &time) {
	QueryPerformanceCounter(&time);
}
inline void operator += (OSTime &time, MSTime addus) {
    LARGE_INTEGER nFreq;
	QueryPerformanceFrequency(&nFreq);
	time.QuadPart += (addus * nFreq.QuadPart) / 1000000;
}
inline void WaitMicroSeconds(MSTime usec) {
	::Sleep(usec / 1000);	// 現状1ms未満の箇所は無い
}

// Windows標準のスレッド処理
typedef HANDLE ThreadObject;
typedef LPTHREAD_START_ROUTINE ThreadFunc;
inline void ThreadCreate(HANDLE &obj, ThreadFunc start_routine, LPVOID arg) {
    DWORD dwID;
	obj = CreateThread(NULL, 0, start_routine, arg, 0, &dwID);
	SetThreadPriority(obj, THREAD_PRIORITY_HIGHEST);
}
inline void ThreadJoin(HANDLE &obj) {
    WaitForSingleObject(obj, INFINITE);
}
typedef CRITICAL_SECTION MutexObject;
inline void MutexInit(MutexObject &obj) {
    InitializeCriticalSection(&obj);
}
inline void MutexDestroy(MutexObject &obj) {
    DeleteCriticalSection(&obj);
}
inline void MutexLock(MutexObject &obj) {
    EnterCriticalSection(&obj);
}
inline void MutexUnlock(MutexObject &obj) {
    LeaveCriticalSection(&obj);
}

#else
// timeval による時間計測(unix系)
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

typedef long long MSTime;
typedef timeval OSTime;
inline MSTime calcusTime(const OSTime &end, const OSTime &st) {
	return ((end.tv_sec - st.tv_sec) * 1e6 + (end.tv_usec - st.tv_usec));
}
inline void getNowOSTime(OSTime &time) {
	gettimeofday(&time, NULL);
}
inline void operator += (OSTime &time, MSTime addus) {
	time.tv_usec += addus;
	if (time.tv_usec >= 1000000) {
		time.tv_usec -= 1000000;
		time.tv_sec++;
	}
}
inline void WaitMicroSeconds(MSTime usec) {
	usleep(usec);
}

// pthread によるスレッド、同期処理
typedef pthread_t ThreadObject;
typedef void *(*ThreadFunc)(void*);
inline void ThreadCreate(ThreadObject &obj, ThreadFunc start_routine, void *arg) {
    pthread_create(&obj, NULL, start_routine, arg);
}
inline void ThreadJoin(ThreadObject &obj) {
    pthread_join(obj, NULL);
}
typedef pthread_mutex_t MutexObject;
inline void MutexInit(MutexObject &obj) {
    pthread_mutex_init(&obj, 0);
}
inline void MutexDestroy(MutexObject &obj) {
    pthread_mutex_destroy(&obj);
}
inline void MutexLock(MutexObject &obj) {
    pthread_mutex_lock(&obj);
}
inline void MutexUnlock(MutexObject &obj) {
    pthread_mutex_unlock(&obj);
}

#endif

#endif
