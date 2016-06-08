#include <stdio.h>
#include <stdint.h>
#include <linux/time.h>

int timer_sleep(struct timeval *t1) {
	struct timeval tc;
	while(1) {
		gettimeofday(&tc, NULL);
//		printf("in while %u %u\n", tc.tv_sec, tc.tv_usec);
		if(tc.tv_sec*1000 + tc.tv_usec/1000 > t1->tv_sec * 1000 + t1->tv_usec/1000) {
			break;
		}
		usleep(100);
	}
	return 0;
}

int main() {
	struct timeval syn_t0;
	uint16_t syn_along_info[3] = {900, 3600, 200};
	gettimeofday(&syn_t0, NULL);
	uint16_t tm_first = (syn_along_info[1]/2 + syn_along_info[0])%(syn_along_info[1]);
	syn_t0.tv_sec += tm_first * 10/1000;
	syn_t0.tv_usec += tm_first * 10 % 1000 * 1000;
	uint16_t n = syn_along_info[2];
	uint16_t step_tm = syn_along_info[1] * 10/n;
	if(step_tm < 80) {
		n = syn_along_info[1] * 10/80;
		step_tm = 80;
	}
	printf("time %u %u %u %u\n", syn_t0.tv_sec, syn_t0.tv_usec, time(NULL), n);
	timer_sleep(&syn_t0);
	int i;
	for(i = 0;i < n;++i) {
		//function
		printf("time %u %016x %u %u\n", syn_t0.tv_sec, syn_t0.tv_usec, time(NULL), n);
		syn_t0.tv_sec += step_tm/1000;
		syn_t0.tv_usec += step_tm%1000*1000;
		timer_sleep(&syn_t0);
	}
	return 0;
}
