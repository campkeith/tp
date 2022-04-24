#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
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

static uint64_t buffer_init(Buffer buf) {
    uint64_t state = 1;
    uint64_t sum = 0;
    for (unsigned index = 0; index < buf.size; index++) {
        state ^= state << 13;
        state ^= state << 7;
        state ^= state << 17;
        buf.elems[index] = state;
        sum += state;
    }
    return sum;
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

static uint64_t run_workers(Buffer buf, unsigned num_threads) {
    Work tasks[num_threads];
    for (unsigned index = 0; index < num_threads; index++) {
        tasks[index] = (Work){.buf = buf,
                              .start = (index * buf.size) / num_threads,
                              .end = ((index + 1) * buf.size) / num_threads};
        int err = pthread_create(&tasks[index].thread, NULL,
                                 worker, &tasks[index]);
        if (err) {
            perror("pthread_create");
            exit(-1);
        }
    }
    uint64_t sum = 0;
    for (unsigned index = 0; index < num_threads; index++) {
        int err = pthread_join(tasks[index].thread, NULL);
        if (err) {
            perror("pthread_join");
            exit(-1);
        }
        sum += tasks[index].sum_out;
    }
    return sum;
}

static void go(unsigned num_threads, uint64_t size_bytes) {
    uint64_t size = ceil_div(size_bytes, sizeof(uint64_t));
    Buffer buf = buffer_create(size);
    uint64_t expected_sum = buffer_init(buf);
    uint64_t start_time = get_time();
    uint64_t sum = run_workers(buf, num_threads);
    assert(sum == expected_sum);
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
