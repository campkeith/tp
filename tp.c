#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct {
    uint64_t size;
    uint64_t * elems;
} Buffer;

typedef struct {
    pthread_t thread;
    Buffer buf;
    uint64_t start;
    uint64_t end;
    uint64_t sum_out;
} Work;


static uint64_t ceil_div(uint64_t a, uint64_t b) {
    return (a + b - 1) / b;
}

static uint64_t get_time(void) {
    struct timespec time;
    int err = clock_gettime(CLOCK_MONOTONIC_RAW, &time);
    if (err) {
        perror("clock_gettime");
        exit(-1);
    }
    uint64_t sec_in_nsecs = 1000 * 1000 * 1000;
    return sec_in_nsecs * (uint64_t)time.tv_sec + (uint64_t)time.tv_nsec;
}

static Buffer buffer_create(uint64_t size) {
    return (Buffer){.size = size,
                    .elems = malloc(size * sizeof(uint64_t))};
}

static void * worker(void * arg) {
    Work * work = arg;
    uint64_t sum = 0;
    for (unsigned index = work->start; index < work->end; index++) {
        sum += work->buf.elems[index];
    }
    work->sum_out = sum;
    return NULL;
}

static void go(unsigned num_threads, uint64_t size_bytes) {
    uint64_t size = ceil_div(size_bytes, sizeof(uint64_t));
    Buffer buf = buffer_create(size);
    Work tasks[num_threads];
    uint64_t start_time = get_time();
    for (unsigned index = 0; index < num_threads; index++) {
        tasks[index] = (Work){.buf = buf,
                              .start = (index * size) / num_threads,
                              .end = ((index + 1) * size) / num_threads};
        int err = pthread_create(&tasks[index].thread, NULL,
                                 worker, &tasks[index]);
        if (err) {
            perror("pthread_create");
            exit(-1);
        }
    }
    for (unsigned index = 0; index < num_threads; index++) {
        int err = pthread_join(tasks[index].thread, NULL);
        if (err) {
            perror("pthread_join");
            exit(-1);
        }
    }
    uint64_t end_time = get_time();
    uint64_t elapsed_time = end_time - start_time;
    printf("%.3f GB/s\n", (float)size_bytes / (float)elapsed_time);
}

static void usage_and_exit(const char * program) {
    fprintf(stderr, "%s <num_threads> <size_bytes>\n", program);
    exit(1);
}

int main(int num_args, char * args[]) {
    if (num_args == 3) {
        unsigned num_threads = strtol(args[1], NULL, 10);
        uint64_t size_bytes = strtol(args[2], NULL, 10);
        if (!num_threads || !size_bytes) {
            usage_and_exit(args[0]);
        }
        go(num_threads, size_bytes);
    } else {
        usage_and_exit(args[0]);
    }
}