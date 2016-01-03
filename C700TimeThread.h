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

#include <thread>
#include <chrono>
// std::chrono による時間計測(非unix系)
typedef long long MSTime;
typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::system_clock::duration> OSTime;
inline MSTime calcusTime(const OSTime &end, const OSTime &st) {
	return std::chrono::duration_cast<std::chrono::microseconds>(end - st).count();
}
inline void getNowOSTime(OSTime &time) {
	time = std::chrono::system_clock::now();
}
inline void operator += (OSTime &time, MSTime addus) {
	time += std::chrono::microseconds(addus);
}
inline void WaitMicroSeconds(MSTime usec) {
	std::this_thread::sleep_for(std::chrono::microseconds(usec));
}

// TODO: Windows標準のスレッド処理

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
inline void ThreadCreate(pthread_t &obj, void *(*start_routine)(void*), void *arg) {
    pthread_create(&obj, NULL, start_routine, arg);
}
inline void ThreadJoin(pthread_t &obj) {
    pthread_join(obj, NULL);
}
typedef pthread_mutex_t MutexObject;
inline void MutexInit(pthread_mutex_t &obj) {
    pthread_mutex_init(&obj, 0);
}
inline void MutexDestroy(pthread_mutex_t &obj) {
    pthread_mutex_destroy(&obj);
}
inline void MutexLock(pthread_mutex_t &obj) {
    pthread_mutex_lock(&obj);
}
inline void MutexUnlock(pthread_mutex_t &obj) {
    pthread_mutex_unlock(&obj);
}

#endif

#endif
