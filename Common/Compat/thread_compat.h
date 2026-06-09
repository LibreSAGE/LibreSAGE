#pragma once

#ifdef __APPLE__
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abs_timeout) {
    struct timespec remaining, current_time, ts;
    clock_gettime(CLOCK_REALTIME, &current_time);
    
    // Calculate remaining time
    remaining.tv_sec = abs_timeout->tv_sec - current_time.tv_sec;
    remaining.tv_nsec = abs_timeout->tv_nsec - current_time.tv_nsec;
    if (remaining.tv_nsec < 0) {
        remaining.tv_sec--;
        remaining.tv_nsec += 1000000000;
    }

    while (pthread_mutex_trylock(mutex) == EBUSY) {
        clock_gettime(CLOCK_REALTIME, &current_time);
        if (current_time.tv_sec > abs_timeout->tv_sec || 
           (current_time.tv_sec == abs_timeout->tv_sec && current_time.tv_nsec >= abs_timeout->tv_nsec)) {
            return ETIMEDOUT;
        }
        
        // Sleep for a short interval before checking again
        usleep(10000); // 10 milliseconds
    }
    return 0;
}
#endif