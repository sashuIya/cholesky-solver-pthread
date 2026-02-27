#ifndef TIMER_H
#define TIMER_H

/* Запустить таймер */
void timer_start(void);
/* Напечатать время от старта и от последнего вызова с заголовком MESSAGE */
void print_time(const char* message);
/* Напечатать время от старта и от последнего вызова с заголовком MESSAGE */
void print_full_time(const char* message);

/* Запустить таймер (то же, что timer_start) */
void TimerStart(void);
/* Напечатать время от старта и от последнего вызова с заголовком MESSAGE.
   Возвращает время от предыдущего вызова. */
long PrintTime(const char* message);
/* Напечатать время от старта и от последнего вызова с заголовком MESSAGE.
   Возвращает время от предыдущего вызова и общее время. */
long int PrintTimeT(const char* message, long int* pTotalTime);
/* Вернуть время в сотых долях секунды (от начала процесса). */
long TimerGet(void);

/* Вывести в строку время текущее время работы */
void sprint_time(char* buffer);

/* CPU for pthread in nanosec */
double get_time_pthread(void);

#endif /* TIMER_H */
