

#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <stdio.h>

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

int main(void)
{
    test_time();
    test_ftime();
    test_gettimeofday();
    test_clock_gettime();

    return 0;
}
