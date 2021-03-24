#
CFLAGS =  -O -g

#要生成的程序
TARGET = player

#LIBS
LIBS := `pkg-config --cflags --libs gstreamer-1.0 gstreamer-pbutils-1.0`


all:player_mp4.o  main.o
	gcc  $(CFLAGS)  -o $(TARGET)  player_mp4.o main.o  $(LIBS)

#mp4.o
player_mp4.o:player_mp4.c player_mp4.h
	gcc  -c -g player_mp4.c $(LIBS)

#main.0
main.o:main.c player_mp4.h
	gcc -c -g  main.c $(LIBS)

.PHONY: clean  
clean:  
	rm -rf *.o  $(TARGET)


#objects = foo.o bar.o    
#all: $(objects)    

#$(objects): %.o: %.c  
#	$(CC) -c $(CFLAGS) $< -o $@