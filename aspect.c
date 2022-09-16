#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#define BUFSIZE 256
extern int errno;
int halt = 0;

void render(pa_simple *conn) {
    uint8_t buffer[BUFSIZE];
        pa_simple_read(conn, buffer, sizeof(buffer), NULL);
        for (unsigned int i = 0; i < sizeof(buffer); i++) {
            printf("%d", buffer[i]);
    }
}

void cleanup(pa_simple *conn) {
    int error;
    if (conn) {
        pa_simple_free(conn);
    }
    else {
        error = errno;
        fprintf(stderr, "Exiting with error %d: %s", error, strerror(error));
        exit(error);
    }
}

void *await() {
    getchar();
    halt = 1;
}

int main(int argc, char* argv[]) {
    pa_simple *conn;
    pa_sample_spec connspec;
    connspec.format = PA_SAMPLE_S16LE;
    connspec.channels = 2;
    connspec.rate = 44100;
    int error;
    pthread_t wait;

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

    /* Spawn a thread to await a keypress to clean up and exit */
    pthread_create(&wait, NULL, await, NULL);

    /* Render the spectrograph */
    while (halt == 0) {
        render(conn);
    }

    pthread_join(wait, NULL);
    cleanup(conn);

    return 0;
}
