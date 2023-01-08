objects = aspect-async.o options.o

aspect  : $(objects) 
	cc -o aspect $(objects) -lpulse -lm -lfftw3 -lncursesw -lmenuw

.PHONY  : clean

clean   :
	rm aspect $(objects)
