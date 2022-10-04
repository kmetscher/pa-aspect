#include <stdio.h>
#include <ncurses.h>
#include <pulse/pulseaudio.h>
#include <complex.h>
#include <fftw3.h>
#define FORMAT PA_SAMPLE_FLOAT32LE
#define SAMPLERATE 44100
#define CHANNELS 1
#define BUFSIZE 1024

// Callbacks
void context_state_cb(pa_context *context, void *main_loop) {
    // FIXME: actually handle state
    pa_context_state_t STATE = pa_context_get_state(context);
    switch (STATE) {
        case PA_CONTEXT_READY: break;
        case PA_CONTEXT_FAILED: break;
    }
}

static void stream_state_cb(pa_stream *stream, void *data) {
    const pa_stream_state_t STATE = pa_stream_get_state(stream);
    switch (STATE) {
        // FIXME: actually handle state
        case PA_STREAM_FAILED: break;
        default: break;
    }
}

static void stream_read_cb(pa_stream *stream, size_t bytes, void *user_data) {
    if (pa_stream_peek(stream, (const void**)&user_data, &bytes) != 0) {
        fprintf(stderr, "Peek failed\n");
        return;
    }
    if (user_data = NULL && bytes > 0) {
        if (pa_stream_drop(stream) != 0) {
            fprintf(stderr, "Dropping hole failed\n");
            return;
        }
    }
    printf("%d\n", (uint8_t) user_data);
    if (pa_stream_drop(stream) != 0) {
        fprintf(stderr, "Dropping after peek failed\n");
    }
}

int main(int argc, char argv[]) {
    // Get the main loop
    pa_mainloop *main_loop = pa_mainloop_new();
    // Get the API
    pa_mainloop_api *loop_api = pa_mainloop_get_api(main_loop);
    // Create context
    pa_context *context = pa_context_new(main_loop, "aspect");
    // Create stream
    pa_sample_spec sample_spec = {.format = FORMAT, .rate = SAMPLERATE, .channels = CHANNELS};
    pa_channel_map channel_map;
    pa_channel_map_init_mono(&channel_map);
    pa_buffer_attr atrributes;
    pa_stream *stream = pa_stream_new(context, "sinksamples", &sample_spec, &channel_map);

    // Set callback and connect context to server
    pa_context_set_state_callback(context, &context_state_cb, NULL);
    pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL);

    // Set callback and connect stream to context
    void *stream_data = NULL;
    pa_stream_set_state_callback(stream, stream_state_cb, main_loop);
    pa_stream_set_read_callback(stream, stream_read_cb, stream_data);
    // FIXME: list sources and query user for their choice
    pa_stream_connect_record(stream, "alsa_output.pci-0000_00_1f.3-platform-skl_hda_dsp_generic.HiFi__hw_sofhdadsp__sink.monitor", NULL, 0);

    for (unsigned int i = 0; i < 10; i++) {
        pa_mainloop_iterate(main_loop, 0, NULL);
    }

    pa_stream_disconnect(stream);
    pa_context_disconnect(context);
    pa_context_unref(context);

    pa_mainloop_quit(main_loop, 0);
    pa_mainloop_free(main_loop);
    return 0;
}
