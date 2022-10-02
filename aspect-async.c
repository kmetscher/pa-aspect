#include <stdio.h>
#include <ncurses.h>
#include <pulse/pulseaudio.h>
#include <complex.h>
#include <fftw3.h>
#define FORMAT PA_SAMPLE_FLOAT32LE
#define SAMPLERATE 44100
#define CHANNELS 1
#define BUFSIZE 1024

static void *buffer = NULL;
static size_t buf_len = 0;
static size_t buf_index = 0;
static pa_io_event *stdio_event = NULL;

// Callbacks
void context_state_cb(pa_context *context, void *main_loop);
void stream_state_cb(pa_stream *stream, void *data);
static void stream_read_cb(pa_stream *stream, size_t len, void *user_data) {
    const void *stream_data;
}

int main(int argc, char argv[]) {
    // Get and lock the main loop
    pa_threaded_mainloop *main_loop = pa_threaded_mainloop_new();
    pa_threaded_mainloop_lock(main_loop);
    pa_threaded_mainloop_start(main_loop);
    // Get the API
    pa_mainloop_api *loop_api = pa_threaded_mainloop_get_api(main_loop);
    // Create context
    pa_context *context = pa_context_new_with_proplist(main_loop, "aspect", NULL);
    // Create stream
    pa_sample_spec sample_spec = {.format = FORMAT, .rate = SAMPLERATE, .channels = CHANNELS};
    pa_channel_map channel_map;
    pa_channel_map_init_mono(&channel_map);
    pa_buffer_attr atrributes;
    pa_stream *stream = pa_stream_new(context, "sinksamples", &sample_spec, &channel_map);
    void *data;

    // Set callback and connect context to server
    pa_context_set_state_callback(context, &context_state_cb, data);
    pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL);
    // Spin until context ready
    while (1) {
        pa_context_state_t context_state = pa_context_get_state(context);
        if (context_state == PA_CONTEXT_READY) {
            break;
        }
        pa_threaded_mainloop_wait(main_loop);
    }

    // Set callback and connect stream to context
    void *stream_data = NULL;
    pa_stream_set_state_callback(stream, stream_state_cb, main_loop);
    pa_stream_set_read_callback(stream, stream_read_cb, stream_data);
    // FIXME: list sources and query user for their choice
    pa_stream_connect_record(stream, "alsa_output.pci-0000_00_1f.3-platform-skl_hda_dsp_generic.HiFi__hw_sofhdadsp__sink.monitor", NULL, 0);
    // Spin until stream ready
    while (1) {
        pa_stream_state_t stream_state = pa_context_get_state(stream);
        if (stream_state == PA_STREAM_READY) {
            break;
        }
        pa_threaded_mainloop_wait(main_loop);
    }

    pa_stream_disconnect(stream);
    pa_context_disconnect(context);
    pa_context_unref(context);


    pa_threaded_mainloop_stop(main_loop);
    pa_threaded_mainloop_free(main_loop);
    return 0;
}
