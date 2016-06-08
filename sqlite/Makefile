all:
#	$(CC) -c *.c $(INC) $(LIBS)
	$(CC) -DSQLITE_ENABLE_MEMORY_MANAGEMENT -shared -o libsqlite.so sqlite3.c -ldl -fPIC
	cp -rf libsqlite.so ../lib
clean:
	rm -rf *.o *.so
