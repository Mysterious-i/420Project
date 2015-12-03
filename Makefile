#
# Makefile for MPI versions of Mandelbrot program
#
# "make -f Makefile.mandelbrot" or "make  -f Makefile.mandelbrot all"
#    to make all executables
# "make -f Makefile.mandelbrot clean" to remove executables
#

#CSTD=-std=c99
#CXXSTD=-std=c++98
# uncomment if using more-recent gcc
CSTD=-std=c11
CXXSTD=-std=c++11

OPT = -O3
MPICC = mpicc
CC = gcc
#CFLAGS = $(OPT) -Wall -pedantic -fopenmp $(CSTD)
CFLAGS = $(OPT) -Wall -pedantic $(CSTD)

LDFLAGS = -lm -I/usr/X11R6/include -L/usr/X11R6/lib -lX11

ALL = \
	mandelbrot_seq \
    block_mpi \
    cyclic_mpi \
    dynamic_mpi \
	points_omp \
    rows_omp

HFILES = mandelbrot-gui.h recorddata.h

.PHONY: all
all: $(ALL)

mandelbrot_seq:	mandelbrot_seq.c $(HFILES)
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

block_mpi: mandelbrot-block_mpi.c $(HFILES)
	$(MPICC) -o $@ $(OPT) -Wall -pedantic $(CSTD) $< $(LDFLAGS)

cyclic_mpi: mandelbrot-cyclic_mpi.c $(HFILES)
	$(MPICC) -o $@ $(OPT) -Wall -pedantic $(CSTD) $< $(LDFLAGS)

dynamic_mpi: mandelbrot-dynamic_mpi.c $(HFILES)
	$(MPICC) -o $@ $(OPT) -Wall -pedantic $(CSTD) $< $(LDFLAGS)

points_omp: mandelbrot-by-points_omp.c $(HFILES)
	$(CC) -o $@ $(OPT) -Wall -pedantic -fopenmp $(CSTD) $< $(LDFLAGS)

rows_omp: mandelbrot-by-rows_omp.c $(HFILES)
	$(CC) -o $@ $(OPT) -Wall -pedantic -fopenmp $(CSTD) $< $(LDFLAGS)

.PHONY: clean
clean:
	-rm $(ALL)

