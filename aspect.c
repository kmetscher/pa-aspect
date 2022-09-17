#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <errno.h>
#include <ncurses.h>
#include <pthread.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#define BUFSIZE 1024
#define PI 3.14159265359
extern int errno;

double *fourierTransform(uint16_t *buffer, int iter) {
    double pi = PI;
    typedef double complex cmplx;
}

void render(pa_simple *conn) {
    /* Initialize a power-2 array to hold discrete frequency ranges 
     * in preparation for a Fast Fourier Transform (FFT) on the 
     * PCM data received from the opened capture stream. The read 
     * operation writes to the declared buffer chunkwise so the 
     * sample can be operated on in close to real time. The array that
     * will contain the complex values as a result of the transform
     * is initialized with half the number of sample points in our 
     * buffer, since we can safely discard either hand side of the 
     * resulting FFT array without sacrificing useful information. */
    int spectra[8];
    uint16_t buffer[BUFSIZE];
        pa_simple_read(conn, buffer, sizeof(buffer), NULL);
        for (unsigned int i = 0; i < sizeof(buffer); i++) {
            printw("%d", buffer[i]);
            refresh();
            clear();
            refresh();
    }
}

void cleanup(pa_simple *conn) {
    int error;
    if (conn) {
        pa_simple_free(conn);
        endwin();
    }
    else {
        error = errno;
        fprintf(stderr, "Exiting with error %d: %s", error, strerror(error));
        endwin();
        exit(error);
    }
}

int main(int argc, char* argv[]) {
    pa_simple *conn;
    pa_sample_spec connspec;
    connspec.format = PA_SAMPLE_S16LE;
    connspec.channels = 1;
    connspec.rate = 44100;
    char ch;
    int halt = 0;
    int error;

    /* Create a new recording connection to the server. 
     * NULL: Default server
     * "Aspect": The application name
     * PA_STREAM_RECORD: Defines recording stream
     * NULL: Default device
     * "Visualizer": Describe the stream
     * &connspec: Reference to our defined sampling format
     * NULL: Default channel map
     * NULL: Default buffer
     * NULL: Suppress errors FIXME
     * */
    conn = pa_simple_new(NULL, "Aspect", PA_STREAM_RECORD, NULL,
        "Visualizer", &connspec, NULL, NULL, NULL);

    /* Error reporting */
    if (!conn) {
        error = errno;
        fprintf(stderr, "Exiting with error %d: %s", error, strerror(error));
        return error;
    }

    /* Start curses mode */
    initscr();
    noecho();

    /* Render the spectrograph and await non-blocking input to interrupt */
    timeout(0);
    while (halt == 0) {
        ch = getch();
        if (ch != ERR) {
            halt = 1;
        }
        render(conn);
    }

    /* Clean up our stream and end curses mode */
    cleanup(conn);

    return 0;
}
