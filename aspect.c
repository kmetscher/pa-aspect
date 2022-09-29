#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <errno.h>
#include <ncurses.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <fftw3.h>
#define BUFSIZE 1024
#define PI 3.14159265359
#define SAMPLERATE 44100
typedef double complex cmplx;
extern int errno;

void fourierTransform(uint16_t *buffer, fftw_complex *samples, fftw_plan plan) {
    for (unsigned int i = 0; i < BUFSIZE; i++) {
        /* PCM data is in the real plane. Most Fourier transforms 
        * operate on complex inputs to produce complex
        * outputs. While there are implementations of the 
        * algorithm with real inputs, the computational cost of 
        * turning our real PCM samples into complex numbers with
        * an imaginary value of 0 feels worth it for the algorithm
        * to make sense at a glance, and to avoid strange math that
        * invariably occurs when you dare to look in the face of i
        * and sneeze. We can also normalize our samples here for the 
        * 16-bit size 65,536. PulseAudio's simple API is also blocking 
        * when returning samples, so we have time, right? */
        double normal = (buffer[i] / 65536.0);
        samples[i] = CMPLX(normal, 0);
    }
    /* Time for the Fastest Fourier Transform in the West */
    fftw_execute(plan);
}

void render(pa_simple *conn, fftw_complex *samples, fftw_plan plan) {
    uint16_t buffer[BUFSIZE];
    pa_simple_read(conn, buffer, sizeof(buffer), NULL);
    fourierTransform(buffer, samples, plan);
    for (unsigned int i = 0; i < BUFSIZE; i += 2) {
        printw("%d, %d", creal(samples[i]), cimag(samples[i]));
    }
    refresh();
}

void cleanup(pa_simple *conn, fftw_complex *samples, fftw_plan plan) {
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
    fftw_free(samples);
    fftw_destroy_plan(plan);
}

int main(int argc, char* argv[]) {
    pa_simple *conn;
    pa_sample_spec connspec;
    connspec.format = PA_SAMPLE_S16LE;
    connspec.channels = 1;
    connspec.rate = SAMPLERATE;
    fftw_complex *samples;
    fftw_plan plan;
    char ch;
    int halt = 0;
    int error;

    samples = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * BUFSIZE);
    plan = fftw_plan_dft_1d(BUFSIZE, samples, samples, FFTW_FORWARD, FFTW_MEASURE);

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
        render(conn, samples, plan);
    }

    /* Clean up our stream and end curses mode */
    cleanup(conn, samples, plan);

    return 0;
}
