#include <pulse/pulseaudio.h>
#include <stdio.h>
#define FORMAT PA_SAMPLE_FLOAT32LE
#define SAMPLERATE 44100
#define CHANNELS 1
#define BUFSIZE 1024

static void stream_state_cb(pa_stream *stream, void *user_data) {
    pa_stream_state_t state = pa_stream_get_state(stream);
    if (state == PA_STREAM_FAILED) {
        fprintf(stderr, "Stream failed\n");
        exit(1);
    }
}

static void stream_read_cb(pa_stream *stream, size_t bytes, void *user_data) {
    uint16_t *stream_data = NULL;
    if (pa_stream_peek(stream, (const void**)&user_data, &bytes) != 0) {
        fprintf(stderr, "Peek failed\n");
        return;
    }
    if (user_data == NULL && bytes == 0) {
        return;
    }
    else if (user_data == NULL && bytes > 0) {
        pa_stream_drop(stream);
        return;
    }
    printf("%d", (int) user_data);
}

static void context_state_cb(pa_context *context, void *user_data) {
    pa_context_state_t state = pa_context_get_state(context);
    if (state == PA_CONTEXT_FAILED) {
        fprintf(stderr, "Context failed\n");
        exit(2);
    }
}

int main(int argc, char argv[]) {
    pa_mainloop *main_loop = pa_mainloop_new();
    pa_mainloop_api *api = pa_mainloop_get_api(main_loop);
    pa_context *context = pa_context_new(api, "aspect");
    pa_context_set_state_callback(context, &context_state_cb, NULL);
}