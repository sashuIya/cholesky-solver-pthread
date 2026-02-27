/* ������ � �������� */
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <unistd.h>
#include "timer.h"

/* CPU time for each thread */
#define MEASURE_THREAD_TIME

/* � ���������������� ��������� � ��� ���������� ����� � �������������
   ��������� ����� ���� ��������� ��������������� ����� ������.
   ���� ����������, �� �������� ����� � ���. */
#define MEASURE_FULL_TIME

/* ������ ���� �� �������� ����������� ������� ������ ���� ���������. */
#define USE_getrusage
//#define USE_times
//#define USE_clock

/* ������ ���� �� �������� ����������� ���������������� ������� ������ ���� ���������. */
#define USE_gettimeofday

#ifdef USE_getrusage
#include <sys/time.h>
#include <sys/resource.h>

#ifdef __hpux
#include <sys/syscall.h>
#define getrusage(a,b) syscall(SYS_getrusage,a,b)
#endif

/* ������� ����� � ����� ����� �������.
   ����� ������� ������ user time, system time �� ������������. */
static long int get_time (void)
{
  struct rusage buf;

  getrusage (RUSAGE_SELF, &buf);
  return   buf.ru_utime.tv_sec * 100            // ����������� ����� � ��������
                                                // � ����� ���� �������
         + buf.ru_utime.tv_usec / 10000;        // ����������� ����� � �������������
                                                // � ����� ���� �������
}
#endif /* USE_getrusage */

#ifdef USE_times
#include <time.h>
#include <sys/times.h>

/* ������� ����� � ����� ����� �������.
   ����� ������� ������ user time, system time �� ������������. */
static long int get_time (void)
{
  struct tms buf;

  times (&buf);

  return buf.tms_utime / (CLK_TCK / 100);       // ����������� ����� � CLK_TCK
                                                // � ����� ���� �������
}
#endif /* USE_times */

#ifdef USE_clock
#include <time.h>

/* ������� ����� � ����� ����� �������.
   ����� ������� user time + system time. */
static long int get_time (void)
{
  long int t;

  t = (long int) clock ();

  return t / (CLOCKS_PER_SEC / 100);            // ����������� ����� � CLOCKS_PER_SEC
                                                // � ����� ���� �������
}
#endif /* USE_clock */

#ifdef USE_gettimeofday
#include <sys/time.h>

/* ���������� ��������������� (!) �����
   � ����� ����� ������� */
static long int get_full_time (void)
{
  struct timeval buf;

  gettimeofday (&buf, 0);
	   /* ����������� ����� � ��������
	      � ����� ���� ������� */
  return   buf.tv_sec * 100
	   /* ����������� ����� � �������������
	      � ����� ���� ������� */
	 + buf.tv_usec / 10000;
}
#endif /* USE_gettimeofday */

typedef struct _timer_
{
  int  tic;
  int  sec;
  int  min;
  int  hour;
} TIMER;

/* ����������� ����� � ����� ����� ������� � ������ ��.��.��:�� */
static void ConvertTime (long clocks, TIMER *t)
{
  t->hour = clocks/360000L;
  clocks %= 360000L;
  t->min  = clocks/6000;
  clocks %= 6000;
  t->sec  = clocks/100;
  t->tic  = clocks%100;
}

static int TimerStarted;        // 1 ���� �������
static long int StartTime;      // ����� ������
static long int PrevTime;       // ����� ���������� ������ get_time

#ifdef MEASURE_FULL_TIME
static long int StartFullTime;	// ��������������� ����� ������
static long int PrevFullTime;	  // ��������������� ����� ���������� ������ get_time
#endif /* MEASURE_FULL_TIME */

/* ��������� ������ */
void timer_start (void)
{
  TimerStarted = 1;
  StartTime = PrevTime = get_time ();
#ifdef MEASURE_FULL_TIME
  StartFullTime = PrevFullTime = get_full_time ();
#endif /* MEASURE_FULL_TIME */
}

/* ���������� ����� �� ������ � �� ���������� ������ � ���������� MESSAGE */
void print_time (const char *message)
{
  long t;
  TIMER summ, stage;

  t = get_time ();

  if (TimerStarted)
    {
      ConvertTime (t - StartTime, &summ);
      ConvertTime (t - PrevTime, &stage);
      printf("Time: total = %2.2d:%2.2d:%2.2d.%2.2d, %s = %2.2d:%2.2d:%2.2d.%2.2d\n",
              summ.hour, summ.min, summ.sec, summ.tic, message,
              stage.hour, stage.min, stage.sec, stage.tic);
      PrevTime = t;
    }
  else
    {
      TimerStarted = 1;
      StartTime = PrevTime = t;
    }
}

/* ���������� ����� �� ������ � �� ���������� ������ � ���������� MESSAGE */
void print_full_time (const char *message)
{
  long t;
  TIMER summ, stage;

#ifdef MEASURE_FULL_TIME
  TIMER summ_full, stage_full;
  long t_full = get_full_time ();
#endif /* MEASURE_FULL_TIME */

  t = get_time ();

  if (TimerStarted)
    {
      ConvertTime (t - StartTime, &summ);
      ConvertTime (t - PrevTime, &stage);
#ifdef MEASURE_FULL_TIME
      ConvertTime (t_full - StartFullTime, &summ_full);
      ConvertTime (t_full - PrevFullTime, &stage_full);
#endif /* MEASURE_FULL_TIME */
#ifdef MEASURE_FULL_TIME
      printf("Time: total=%2.2d:%2.2d:%2.2d.%2.2d (%2.2d:%2.2d:%2.2d.%2.2d), %s=%2.2d:%2.2d:%2.2d.%2.2d (%2.2d:%2.2d:%2.2d.%2.2d)\n",
	      summ.hour, summ.min, summ.sec, summ.tic,
	      summ_full.hour, summ_full.min, summ_full.sec, summ_full.tic,
	      message,
	      stage.hour, stage.min, stage.sec, stage.tic,
	      stage_full.hour, stage_full.min, stage_full.sec, stage_full.tic);
      PrevFullTime = t_full;
#else
      printf("Time: total=%2.2d:%2.2d:%2.2d.%2.2d, %s=%2.2d:%2.2d:%2.2d.%2.2d\n",
	      summ.hour, summ.min, summ.sec, summ.tic, message,
	      stage.hour, stage.min, stage.sec, stage.tic);
#endif /* MEASURE_FULL_TIME */
      PrevTime = t;
    }
  else
    {
      TimerStarted = 1;
      StartTime = PrevTime = t;
#ifdef MEASURE_FULL_TIME
      StartFullTime = PrevFullTime = t_full;
#endif /* MEASURE_FULL_TIME */
    }
}

/* ��������� ������ */
void TimerStart (void)
{
  timer_start ();
}

/* ���������� ����� �� ������ � �� ���������� ������ � ���������� MESSAGE.
   ���������� ����� �� ����������� ������. */
long PrintTime (const char *message)
{
  long t;
  TIMER summ, stage;
  long res;

  t = get_time ();

  if (TimerStarted)
    {
      ConvertTime (t - StartTime, &summ);
      ConvertTime (res = t - PrevTime, &stage);
      printf("Time: total = %2.2d:%2.2d:%2.2d.%2.2d, %s = %2.2d:%2.2d:%2.2d.%2.2d\n",
              summ.hour, summ.min, summ.sec, summ.tic, message,
              stage.hour, stage.min, stage.sec, stage.tic);
      PrevTime = t;
    }
  else
    {
      TimerStarted = 1;
      StartTime = PrevTime = t;
      res = 0;
    }
  return res;
}

/* ������� � ������ ����� ������� ����� ������ */
void sprint_time (char *buffer)
{
  long t;
  TIMER summ;

  t = get_time ();

  if (TimerStarted)
    {
      ConvertTime (t - StartTime, &summ);
      sprintf (buffer, "%2.2d:%2.2d:%2.2d.%2.2d",
               summ.hour, summ.min, summ.sec, summ.tic);
    }
  else
    {
      TimerStarted = 1;
      StartTime = PrevTime = t;
    }
}

/* ���������� ����� �� ������ � �� ���������� ������ � ���������� MESSAGE.
   ���������� ����� �� ����������� ������ � ����� �����. */
long int PrintTimeT (const char *message, long int * pTotalTime)
{
  long t;
  TIMER summ, stage;
  long res;

  t = get_time ();

  if (TimerStarted)
    {
      ConvertTime (t - StartTime, &summ);
      ConvertTime (res = t - PrevTime, &stage);
      printf("Time: total = %2.2d:%2.2d:%2.2d.%2.2d, %s = %2.2d:%2.2d:%2.2d.%2.2d\n",
	      summ.hour, summ.min, summ.sec, summ.tic, message,
	      stage.hour, stage.min, stage.sec, stage.tic);
      PrevTime = t;
      *pTotalTime = t - StartTime;
    }
  else
    {
      TimerStarted = 1;
      StartTime = PrevTime = t;
      res = 0;
    }
  return res;
}

/* ������� ����� � ����� ����� ������� (�� ������ ��������). */
long TimerGet (void)
{
  return get_time ();
}

#ifdef MEASURE_THREAD_TIME

#include <sys/types.h>
#include <pthread.h>
#include <time.h>

/* CPU time for pthread in nanosec */
double get_time_pthread(void) {
  struct timespec timer;
  
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &timer);
  
  return (double)(timer.tv_sec)*1e9 + (double)(timer.tv_nsec);
}

#endif /* MEASURE_THREAD_TIME */

