
functions= prof_main.o \
	functionsFiles.o \
	functionsData.o \
	functionsSort.o \
	functionsAllToAllv.o \
	functionsCreateType.o \
	temp.o \
	timestamp.o \
	verify.o \
	functionsSeqTree.o \
	functionsGlobalSeqTree.o \
	functionsGlobalSort.o \
	functionsSeqSearch.o \
	functionsTrees.o

prof_main: $(functions)
	mpicc -o prof_main $(functions) -lm


%.o: %.c headerFuncs.h
	mpicc -c -o $@ $< -I.

clean:
	rm *~ prof_main *.o
