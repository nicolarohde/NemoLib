# Makefile for nauty
HEADERS=gtools.h naurng.h nauty.h naututil.h nausparse.h nautinv.h schreier.h
FILES=nautil.c naututil.c sorttemplates.c naugraph.c nausparse.c nautinv.c schreier.c gtools.c naurng.c nauty.c

# compiler settings
CXX=gcc

# delete will generate 3 warnings that can be ignored, thus -w
FLAGS=-pipe -Ofast

ifeq ($(_USE_FAST_MATH), 1)
	FLAGS := $(FLAGS) -ffast-math -ffinite-math-only
endif

all:
	$(CXX) $(FLAGS) -c $(HEADERS) $(FILES)

clean:
	rm -f *.o *.h.gch
