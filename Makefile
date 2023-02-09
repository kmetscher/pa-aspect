objects = aspect-async.o 

libs    = -lpulse -lm -lfftw3 -lncursesw -lmenuw

aspect  : $(objects) 
	gcc -o aspect $(objects) $(libs)

debug   : $(objects)
	gcc -o aspect $(objects) $(libs) -g -Wall

.PHONY  : clean

clean   :
	rm aspect $(objects)
