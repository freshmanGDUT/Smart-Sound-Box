device : device.o libupnp/sample_util.o
	g++ playerserver.o libupnp/sample_util.o -o playerserver -I/usr/local/include/upnp  -L/usr/local/lib -static -lupnp -lixml -lthreadutil -lpthread
device.o : PlayerServer.cpp
	g++ PlayerServer.cpp -c -o playerserver.o  -I/usr/local/include/upnp  -L/usr/local/lib -lupnp -lixml -lthreadutil
libupnp/sample_util.o : libupnp/sample_util.cpp
	g++ libupnp/sample_util.cpp -c -o libupnp/sample_util.o -I/usr/local/include/upnp
	

