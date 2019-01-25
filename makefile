#!/bin/bash


test2: test2.c
	mpicc -o test2 test2.c

clean:
	rm *~ test2
