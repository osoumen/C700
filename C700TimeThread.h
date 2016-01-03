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

#else

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

#endif


#endif
