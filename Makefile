

CC=gcc
CFLAGS=-std=gnu99 -O3

time_perf_test: time_perf_test.c
	$(CC) $(CFLAGS) -o $@ $< -lrt

clean:
	rm -f time_perf_test
