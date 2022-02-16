CURR_DIR=$(notdir $(basename $(shell pwd)))
PRJ=$(CURR_DIR)
SRC=$(filter-out $(wildcard ref*.c), $(wildcard *.c))
OBJ=$(patsubst %.c,%.o,$(SRC))

CC=gcc
INCLUDES=-I../../include
ifndef DEBUG
CFLAGS=-O3 -ffast-math -mavx2 -mfma -std=gnu99
LIB=
else
CFLAGS=-O0 -g3 -mavx2 -mfma -std=gnu99
LIB=
endif

all: $(PRJ)

$(PRJ): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $@ -lm

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LIB)	

clean:
	-rm -f $(OBJ) $(PRJ)


gcc -O3 -ffast-math -mavx2 -mfma -std=gnu99 -I../../include -c fail.c -o fail.o 	
gcc -O3 -ffast-math -mavx2 -mfma -std=gnu99 -I../../include -c img.c -o img.o 	
gcc -O3 -ffast-math -mavx2 -mfma -std=gnu99 -I../../include -c input.c -o input.o
gcc -O3 -ffast-math -mavx2 -mfma -std=gnu99 -I../../include -c main.c -o main.o 	
gcc -O3 -ffast-math -mavx2 -mfma -std=gnu99 -I../../include -c output.c -o output.o 	
gcc -O3 -ffast-math -mavx2 -mfma -std=gnu99 -I../../include -S compute.c  -o compute.s	
