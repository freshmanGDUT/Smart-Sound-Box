CC = mipsel-openwrt-linux-g++
CPP = Player
CPP_O = 
OBJS = $(CPP).o
# Enter your python head doc path
PYTHON_INCLUDE = /home/oujiayu/EmbededProject/python-install/include/python2.7
# Enter your python lib path
PYTHON_LIB_STATIC = /home/oujiayu/EmbededProject/python-install/lib/python2.7/config
# Enter your libz path
LIBZ_A = /home/oujiayu/EmbededProject/python-install/lib/

Player : $(OBJS)
	$(CC) $(OBJS) -o Player -I $(PYTHON_INCLUDE) -L $(PYTHON_LIB_STATIC)  -lpython2.7 -lpthread -lm -lutil -lnsl -ldl -L $(LIBZ_A) -lz

$(CPP).o : $(CPP).cpp CppMPCPlay.h CppMPCPlayAPI.h 
	$(CC) -c $(CPP).cpp -o $(CPP).o -I $(PYTHON_INCLUDE) -L $(PYTHON_LIB_STATIC)  -lpython2.7 -lpthread -lm -lutil -lnsl -ldl -L $(LIBZ_A) -lz

clean :
	rm -rf *.o Player
