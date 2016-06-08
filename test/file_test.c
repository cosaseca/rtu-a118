/*
 * file_test.c
 *
 *  Created on: Nov 21, 2014
 *      Author: ygz
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	FILE *file = fopen("test_a", "a");
	char buf[1024 * 1024];
	int rc = 0;
	if(NULL != file) {
		while(1) {
			fprintf(file, "%s\n", "hello world !");
			rc = fwrite(buf, sizeof(buf), 1, file);
			fflush(file);
			fsync(fileno(file));
			printf("rc %d\n", rc);
			sleep(2);
		}
		fclose(file);
	}
	return 0;
}
