#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Bounded Buffer */
int bb_buf[10];
int bb_sz = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gcond = PTHREAD_COND_INITIALIZER;
pthread_cond_t pcond = PTHREAD_COND_INITIALIZER;
int bb_get();
void bb_put(int i);

int bb_get() {
    int ret, i;
    pthread_mutex_lock(&mutex);
    while (bb_sz <= 0) {
        pthread_cond_wait(&gcond, &mutex);
    }
    ret = bb_buf[0];
    for (i = 0; i < 9; i++) {
        bb_buf[i] = bb_buf[i + 1];
    }
    bb_buf[9] = 0;
    bb_sz = bb_sz - 1;
    pthread_cond_signal(&pcond);
    pthread_mutex_unlock(&mutex);
    return ret;
}

void bb_put(int i) {
    pthread_mutex_lock(&mutex);
    while (bb_sz >= 10) {
        pthread_cond_wait(&pcond, &mutex);
    }
    bb_buf[bb_sz] = i;
    bb_sz = bb_sz + 1;
    pthread_cond_signal(&gcond);
    pthread_mutex_unlock(&mutex);
}

void *thread_put(void *p) {
    int *thnum = (int *) p;
    bb_put(*thnum);
    printf("put: %d\n", *thnum);
    pthread_exit(NULL);
}

void *thread_get(void *p) {
    int *thnum = (int *) p;
    printf("in %d thread, get %d\n", *thnum, bb_get());
    pthread_exit(NULL);
}

int main() {

    pthread_t th1, th2, th3, th4, th5, th6, th7, th8, th9, th10, th11, th12;
    int tn1 = 1;
    int tn2 = 2;
    int tn3 = 3;
    int tn4 = 4;
    int tn5 = 5;
    int tn6 = 6;
    int tn7 = 7;
    int tn8 = 8;
    int tn9 = 9;
    int tn10 = 10;
    int tn11 = 11;
    int tn12 = 12;

    pthread_create(&th1, NULL, thread_put, &tn1);
    pthread_create(&th2, NULL, thread_put, &tn2);
    pthread_create(&th3, NULL, thread_put, &tn3);
    pthread_create(&th9, NULL, thread_get, &tn9);
    pthread_create(&th10, NULL, thread_get, &tn10);
    pthread_create(&th4, NULL, thread_put, &tn4);
    pthread_create(&th5, NULL, thread_put, &tn5);
    pthread_create(&th6, NULL, thread_put, &tn6);
    pthread_create(&th7, NULL, thread_put, &tn7);
    pthread_create(&th8, NULL, thread_get, &tn8);
    pthread_create(&th11, NULL, thread_get, &tn11);
    pthread_create(&th12, NULL, thread_get, &tn12);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    pthread_join(th3, NULL);
    pthread_join(th4, NULL);
    pthread_join(th5, NULL);
    pthread_join(th6, NULL);
    pthread_join(th7, NULL);
    pthread_join(th8, NULL);
    pthread_join(th9, NULL);
    pthread_join(th10, NULL);
    pthread_join(th11, NULL);
    pthread_join(th12, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&gcond);
    pthread_cond_destroy(&pcond);

    int i;
    for (i = 0; i < 10; i++) printf("bb_buf[%d] = %d\n", i, bb_buf[i]);

    return 0;
}
