#ifndef TIMER_H
#define TIMER_H

// Starts the global timer for both CPU and wall clock measurements.
void timer_start(void);

// Prints the elapsed CPU time since start and since the previous call.
// message: A label to include in the printed output.
void print_time(const char* message);

// Prints both CPU and wall clock time since start and since the previous call.
// message: A label to include in the printed output.
void print_full_time(const char* message);

// Legacy wrapper for timer_start.
void TimerStart(void);

// Prints elapsed CPU time and returns the time since the previous call.
// message: A label to include in the printed output.
// Returns: CPU time in 1/100ths of a second since the previous call.
long PrintTime(const char* message);

// Prints elapsed CPU time and returns both the stage and total time.
// message: A label to include in the printed output.
// pTotalTime: Pointer to store the total CPU time in 1/100ths of a second.
// Returns: CPU time in 1/100ths of a second since the previous call.
long int PrintTimeT(const char* message, long int* pTotalTime);

// Gets the total elapsed CPU time since the timer was started.
// Returns: CPU time in 1/100ths of a second.
long TimerGet(void);

// Gets the total elapsed wall clock time since the timer was started.
// Returns: Wall clock time in 1/100ths of a second.
long WallTimerGet(void);

// Formats the current total CPU time into a HH:MM:SS.CC string.
// buffer: A character buffer to store the formatted string.
void sprint_time(char* buffer);

// Gets the precise CPU time for the calling thread.
// Returns: Thread CPU time in nanoseconds.
double get_time_pthread(void);

#endif  // TIMER_H
