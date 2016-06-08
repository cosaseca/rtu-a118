all:
#	$(CC) -c *.c $(INC) $(LIBS)
	$(CC) -shared -o libcJSON.so cJSON.c -lm -fPIC
	cp -rf libcJSON.so ../lib
clean:
	rm -rf *.o *.so
