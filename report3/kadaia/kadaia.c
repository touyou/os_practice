#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "btree.h"

tnode *btree_create() {
    return NULL;
}

tnode *btree_insert(int v, tnode *t) {
    if (t == NULL) {
        t = btree_create();
        t = malloc(sizeof(tnode));
        t->v = v;
    }
    else if (v <= t->v) t->left = btree_insert(v, t->left);
    else if (v > t->v) t->right = btree_insert(v, t->right);
    return t;
}

void btree_destroy(tnode *t) {
    if (t == NULL) return;
    btree_destroy(t->left);
    btree_destroy(t->right);
    free(t);
}

void btree_dump(tnode *t) {
    if (t == NULL) return;
    btree_dump(t->left);
    printf("%d\n", t->v);
    btree_dump(t->right);
}

tnode *tree;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *func_thread(void *p) {
    int i;
    pthread_mutex_lock(&mutex);
    for (i=0; i<10; i++) {
        tree = btree_insert(i, tree);
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main()  {
    tree = btree_create();
    int i;
    int b = 42;

    pthread_t pthread, pthread1, pthread2, pthread3;
    pthread_create(&pthread, NULL, &func_thread, &b);
    pthread_create(&pthread1, NULL, &func_thread, &b);
    pthread_create(&pthread2, NULL, &func_thread, &b);
    pthread_create(&pthread3, NULL, &func_thread, &b);

    // ここでロックをかけるとプログラムが固まってしまう
    // pthread_mutex_lock(&mutex);
    // for (i = 0; i < 10; i++) {
    //     tree = btree_insert(i, tree);
    // }
    // pthread_mutex_lock(&mutex);

    pthread_join(pthread, NULL);
    pthread_join(pthread1, NULL);
    pthread_join(pthread2, NULL);
    pthread_join(pthread3, NULL);

    pthread_mutex_destroy(&mutex);

    btree_dump(tree);
    btree_destroy(tree);
    return 0;
}
