#!/bin/bash

gcc -o two_squares.exe two_squares.c \
	-Ilibs/SDL3/include \
	-Llibs/SDL3/lib -lSDL3 \
	-Wall -Wl,-rpath,libs/SDL3/lib
