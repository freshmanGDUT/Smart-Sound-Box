all :
	gcc *.c -I/home/qunzi/QUNZI/DLNA/DLNA_INSTALL/include -I/usr/local/include/libxml2/ -c
	gcc *.o -static -o main -I/home/qunzi/QUNZI/DLNA/DLNA_INSTALL/include -I/usr/local/include/libxml2/ -lxml2 -lz -lm -llzma -L/home/qunzi/QUNZI/DLNA/DLNA_INSTALL/lib -lixml -lupnp -lthreadutil -lpthread
clean :
	rm -f *.o main *~
