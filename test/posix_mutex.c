/*
 * posix_mutex.c
 *
 *  Created on: Oct 2, 2014
 *      Author: ygz
 */

#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int a = 0;

/**
int pthread_mutexattr_init (pthread_mutexattr_t * attr);
int pthread_mutexattr_destroy (pthread_mutexattr_t * attr);
int pthread_mutexattr_getpshared (const pthread_mutexattr_t * attr, int * pshared);
int pthread_mutexattr_setpshared (pthread_mutexattr_t * attr, int pshared);
int pthread_mutexattr_settype (pthread_mutexattr_t * attr, int kind);
int pthread_mutexattr_gettype (pthread_mutexattr_t * attr, int * kind);
int pthread_mutex_init (pthread_mutex_t * mutex, const pthread_mutexattr_t * attr);
 */



void *thread_a(void *arg) {
	while(1) {
		pthread_mutex_lock(&mutex);
		a = 1;
//		pthread_mutex_unlock(&mutex);
		if(1) {
			printf("a %d\n", a);
		}
		usleep(10000);
	}
	return 0;
}

void *thread_b(void *arg) {
	while(1) {
		pthread_mutex_unlock(&mutex);
//		pthread_mutex_lock(&mutex);
		a = 2;
//		pthread_mutex_unlock(&mutex);
		if(1) {
			printf("b %d\n", a);
		}
		usleep(10000);
	}
	return 0;
}

void *thread_c(void *arg) {
	while(1) {
		pthread_mutex_lock(&mutex);
		a = 3;
//		pthread_mutex_unlock(&mutex);
		if(1) {
			printf("c %d\n", a);
		}
		usleep(10000);
	}
	return 0;
}

int main() {
	pthread_t id;
	pthread_create(&id, NULL, thread_a, NULL);
	pthread_create(&id, NULL, thread_b, NULL);
	pthread_create(&id, NULL, thread_c, NULL);
	while(1) {
		sleep(100);
	}
	return 0;
}
