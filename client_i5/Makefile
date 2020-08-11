src=$(wildcard ./*.c)
OBJ=$(patsubst %.c,%.o,$(src))
TARGET=sock
CC=arm-linux-gnueabihf-gcc
FLAGS= -lpthread

$(TARGET):$(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(FLAGS)

%.o:%.c
	$(CC) -c $< -o $@  

.PHONY: clean
clean:
	-rm *.o $(TARGET)
