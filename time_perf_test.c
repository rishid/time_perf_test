

#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <stdint.h>

static int N = 10000000;

#define TEST_START()                            \
    unsigned long long t_start = gettime();

#define TEST_END(tname)                                                 \
    unsigned long long t_end = gettime();                               \
    printf(tname " => %llu ns per call\n", (t_end - t_start) / N);

#define TEST_END2(tname, subtname)                                      \
    unsigned long long t_end = gettime();                               \
    printf(tname " => %llu ns per call\n", subtname, (t_end - t_start) / N);

unsigned long long gettime(void)
{
    const int SEC_TO_NS = 1000000000;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * SEC_TO_NS + ts.tv_nsec;
}

void test_time(void)
{
    time_t t;

    TEST_START();
    for (unsigned long i = 0; i < N; ++i)
    {
        t = time(NULL);
    }
    TEST_END("time");
}

void test_ftime(void)
{
    struct timeb t;

    TEST_START();
    for (unsigned long i = 0; i < N; ++i)
    {
        ftime(&t);
    }
    TEST_END("ftime");
}

void test_gettimeofday(void)
{
    struct timeval tv;

    TEST_START();
    for (unsigned long i = 0; i < N; ++i)
    {
        gettimeofday(&tv, NULL);
    }
    TEST_END("gettimeofday");
}

void test_clock_gettime(void)
{
    struct timespec ts;
    const struct {
        char name[32];
        clockid_t clk_id;
    } clocks[] = {
        { "CLOCK_REALTIME", CLOCK_REALTIME },
        { "CLOCK_REALTIME_COARSE", CLOCK_REALTIME_COARSE },
        { "CLOCK_MONOTONIC", CLOCK_MONOTONIC },
        { "CLOCK_MONOTONIC_COARSE", CLOCK_MONOTONIC_COARSE },
        { "CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW },
        { "CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID },
        { "CLOCK_THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID },
    };

    for (unsigned long a = 0; a < 7; ++a)
    {
        clockid_t clk_id = clocks[a].clk_id;

        // check clock works for host first
        int rc = clock_gettime(clk_id, &ts);
        if (rc < 0)
        {
            printf("error using %s with clock_gettime, skipping clock\n", clocks[a].name);
            continue;
        }

        TEST_START();
        for (unsigned long i = 0; i < N; ++i)
        {
            clock_gettime(clk_id, &ts);
        }
        TEST_END2("clock_gettime(%s)", clocks[a].name);
    }
}

#include <x86intrin.h> // __rdtsc()
#include <unistd.h> // usleep
#define USEC_PER_SEC	1000000L
#define NSEC_PER_SEC	1000000000L
#define NSEC_PER_MSEC	1000000L
#define NSEC_PER_USEC	1000L

void ts_sub(struct timespec *a, struct timespec *b, struct timespec *result)
{
    result->tv_sec = a->tv_sec - b->tv_sec;
        result->tv_nsec = a->tv_nsec - b->tv_nsec;
        if (result->tv_nsec < 0) {
            --result->tv_sec;
            result->tv_nsec += NSEC_PER_SEC;
        }
}


inline uint64_t ts_to_usec(struct timespec *tsp)
{
    return (uint64_t)(tsp->tv_sec * USEC_PER_SEC) + ((uint64_t)(tsp->tv_nsec) / NSEC_PER_USEC);
}


uint64_t get_tsc_rate_per_second()
{
    uint64_t tsc_per_second, delta_usec;
    struct timespec ts_before, ts_after, ts_delta;
    uint64_t tsc_before, tsc_after, tsc_delta;

    clock_gettime(CLOCK_MONOTONIC, &ts_before);
    tsc_before = __rdtsc();
    usleep(100000);//0.1 sec
    clock_gettime(CLOCK_MONOTONIC, &ts_after);
    tsc_after = __rdtsc();

    // Calc delta's
    tsc_delta = tsc_after - tsc_before;

    ts_sub(&ts_after, &ts_before, &ts_delta);
    delta_usec = ts_to_usec(&ts_delta);

    // Calc rate
    tsc_per_second = tsc_delta * 1000000L / delta_usec;

    return tsc_per_second;
}

void test_rdtsc(void)
{
    uint64_t start, clock_count;

    start = __rdtsc();
    TEST_START();
    for (unsigned long i = 0; i < N; ++i)
    {
        clock_count = __rdtsc();
    }
    TEST_END("rdtsc");

    printf("rdtsc => %.03f ns per call (using rdtsc)\n",
           (double)((clock_count - start) / N) * (1000*1000*1000.0 / get_tsc_rate_per_second()) );
}

int main(void)
{
    test_time();
    test_ftime();
    test_gettimeofday();
    test_clock_gettime();
    test_rdtsc();

    return 0;
}
