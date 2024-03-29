/*
 * MPI Mandelbrot program using static task assignment, cyclic
 * distribution
 *
 * This program computes and displays all or part of the Mandelbrot
 * set.  By default, it examines all points in the complex plane
 * that have both real and imaginary parts between -2 and 2.
 * Command-line parameters allow zooming in on a specific part of
 * this range.
 *
 * Usage:
 *   mandelbrot maxiter [x0 y0 size]
 * where
 *   maxiter denotes the maximum number of iterations at each point
 *   x0, y0, and size specify the range to examine (a square
 *     centered at x0 + iy0 of size 2*size by 2*size -- by default,
 *     a square of size 4 by 4 centered at the origin)
 *
 * Input:  none, except the optional command-line arguments
 * Output: a graphical display as described in Wilkinson & Allen,
 *   displayed using the X Window system, plus text output to
 *   standard output showing the above parameters, plus execution
 *   time in seconds.
 */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <float.h>

#include "timer.h"
#include "cmdline.h"

/* Default values for things. */
#define N           2           /* size of problem space (x, y from -N to N) */
#define NPIXELS     800         /* size of display window in pixels */

/* Constants for message-passing */
#define DATA_TAG    1           /* "data" message (worker to master) */

/* Structure definition for complex numbers */
typedef struct {
    double real, imag;
} complex;

/* Functions for GUI */
#include "mandelbrot-gui.h"     /* has setup(), interact() */
#include "recorddata.h"

/* ---- Function declarations ---- */

int master_pgm(int nworkers, int width, int height,
               double real_min, double real_max,
               double imag_min, double imag_max,
               int maxiter);
int worker_pgm(int myID, int nworkers, int width, int height,
               double real_min, double real_max,
               double imag_min, double imag_max,
               int maxiter);

/* ---- Main program ---- */

int main (int argc, char *argv[]) {
    
    int nprocs;
    int myid;
    int returnval;
    int maxiter;
    double real_min = -N;
    double real_max = N;
    double imag_min = -N;
    double imag_max = N;
    int width = NPIXELS;         /* dimensions of display window */
    int height = NPIXELS;
    
    /* Initialize for MPI */
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        fprintf(stderr, "MPI initialization error\n");
        exit(EXIT_FAILURE);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    if (nprocs < 2) {
        fprintf(stderr, "Number of processes must be at least 2\n");
        MPI_Finalize(); exit(EXIT_FAILURE);
    }
    
    /* Process command-line arguments */
    char *usage_msg = "usage:  %s maxiter [x0 y0 size]\n";
    maxiter = get_integer_arg_extended(argc, argv, 1, 1,
                                       "maxiter", usage_msg,
                                       (myid == 0), (void (*)(void))MPI_Finalize);
    if (argc > 2) {
        double x0 = get_floating_arg_extended(argc, argv, 2, -DBL_MAX,
                                              "x0", usage_msg,
                                              (myid == 0), (void (*)(void))MPI_Finalize);
        double y0 = get_floating_arg_extended(argc, argv, 3, -DBL_MAX,
                                              "y0", usage_msg,
                                              (myid == 0), (void (*)(void))MPI_Finalize);
        double size = get_floating_arg_extended(argc, argv, 4, 0,
                                                "size", usage_msg,
                                                (myid == 0), (void (*)(void))MPI_Finalize);
        real_min = x0 - size;
        real_max = x0 + size;
        imag_min = y0 - size;
        imag_max = y0 + size;
    }
    
    /* Call master or worker code as appropriate */
    if (myid == 0) {
        returnval = master_pgm(nprocs-1, width, height,
                               real_min, real_max, imag_min, imag_max, maxiter);
    }
    else {
        returnval = worker_pgm(myid, nprocs-1, width, height,
                               real_min, real_max, imag_min, imag_max, maxiter);
    }
    
    /* Finish up */
    MPI_Finalize();
    return returnval;
}

/*
 * ---- Program for master process ----
 *
 * Contains most code from original main program, slightly modified for MPI.
 * Returns EXIT_SUCCESS or EXIT_FAILURE as appropriate.
 */
int master_pgm(int nworkers, int width, int height,
               double real_min, double real_max,
               double imag_min, double imag_max,
               int maxiter) {
    
    Display *display;
    Window win;
    GC gc;
    long min_color = 0, max_color = 0;
    int this_row;
    double start_time, end_time;
    long *data_msg = malloc((width+1) * sizeof(*data_msg));
    MPI_Status status;
    int setup_return;
    
    /* Initialize for graphical display */
    setup_return =
    setup(width, height, &display, &win, &gc, &min_color, &max_color);
    if (setup_return != EXIT_SUCCESS) {
        fprintf(stderr, "Unable to initialize display, continuing\n");
    }
    /* (if not successful, continue but don't display results) */
    
    /* Start timing  */
    /* could use MPI_Wtime but get_time is what sequential code uses */
    start_time = get_time();
    
    /* START OF CHANGED SECTION */
    
    /* Broadcast values workers will need and can't compute */
    MPI_Bcast(&min_color, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max_color, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    
    /* Receive results from workers and draw points */
    
    for (int row = 0; row < height; ++row) {
        
        MPI_Recv(data_msg, width+1, MPI_LONG, MPI_ANY_SOURCE,
                 DATA_TAG, MPI_COMM_WORLD, &status);
        
        this_row = data_msg[0];
        
        for (int col = 0; col < width; ++col) {
            if (setup_return == EXIT_SUCCESS) {
                XSetForeground (display, gc, data_msg[col+1]);
                XDrawPoint (display, win, gc, col, this_row);
            }
        }
    }
    
    /* END OF CHANGED SECTION  */
    
    /* Be sure everything is written out. */
    if (setup_return == EXIT_SUCCESS) {
        XFlush (display);
    }
    
    /* End timing  */
    end_time = get_time();
    
    /* Produce text output  */
    fprintf(stdout, "\n");
    fprintf(stdout, "MPI program with static task assignment (cyclic)\n");
    fprintf(stdout, "number of worker processes = %d\n", nworkers);
    fprintf(stdout, "center = (%g, %g), size = %g\n",
            (real_max + real_min)/2, (imag_max + imag_min)/2,
            (real_max - real_min)/2);
    fprintf(stdout, "maximum iterations = %d\n", maxiter);
    double total_t =end_time - start_time;
    fprintf(stdout, "execution time in seconds = %g\n", total_t);
    fprintf(stdout, "\n");
    char *filen = "mpi-c.txt";
    record(filen,nworkers+1,maxiter,total_t);
    /* Wait for user response, then exit program  */
    /*if (setup_return == EXIT_SUCCESS) {
        interact(display, &win, width, height,
                 real_min, real_max, imag_min, imag_max);
    }*/
    free(data_msg);
        sleep(2);
    return EXIT_SUCCESS;
}

/*
 * ---- Program for worker process ----
 *
 * Returns EXIT_SUCCESS or EXIT_FAILURE as appropriate.
 */
int worker_pgm(int myID, int nworkers, int width, int height,
               double real_min, double real_max,
               double imag_min, double imag_max,
               int maxiter) {
    
    long min_color, max_color;
    double scale_real, scale_imag, scale_color;
    long *data_msg = malloc((width+1) * sizeof(*data_msg));
    
    /* START OF CHANGED SECTION  */
    
    /* Receive broadcast values */
    MPI_Bcast(&min_color, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max_color, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    
    /* Compute factors to scale computational region to window */
    scale_real = (double) (real_max - real_min) / (double) width;
    scale_imag = (double) (imag_max - imag_min) / (double) height;
    
    /* Compute factor for color scaling */
    scale_color = (double) (max_color - min_color) / (double) (maxiter - 1);
    
    /* Calculate points and send back to master a row at a time
     * (cyclic distribution of work among workers).
     */
    
    for (int row = (myID-1); row < height; row += nworkers) {
        
        data_msg[0] = row;
        
        for (int col = 0; col < width; ++col) {
            
            complex z, c;
            
            z.real = z.imag = 0;
            
            /* Scale display coordinates to actual region */
            c.real = real_min + ((double) col * scale_real);
            c.imag = imag_min + ((double) (height-1-row) * scale_imag);
            /* height-1-row so y axis displays
             * with larger values at top
             */
            
            /* Calculate z0, z1, .... until divergence or maximum iterations */
            int k = 0;
            double lengthsq, temp;
            do  {
                temp = z.real*z.real - z.imag*z.imag + c.real;
                z.imag = 2*z.real*z.imag + c.imag;
                z.real = temp;
                lengthsq = z.real*z.real + z.imag*z.imag;
                ++k;
            } while (lengthsq < (N*N) && k < maxiter);
            
            /* Scale color and store */
            long color = (long) ((k-1) * scale_color) + min_color;
            data_msg[col+1] = color;
        }
        
        MPI_Send(data_msg, width+1, MPI_LONG, 0, DATA_TAG,
                 MPI_COMM_WORLD);
    }
    
    /* END OF CHANGED SECTION  */
    
    free(data_msg);
    return EXIT_SUCCESS;
}
