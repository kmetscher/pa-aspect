#include <stdio.h>
#include <ncurses.h>
#include <menu.h>
#include <pulse/pulseaudio.h>
#include <complex.h>
#include <fftw3.h>
#include <signal.h>
#include <math.h>
#include <string.h>

#define FORMAT PA_SAMPLE_S16LE
#define SAMPLERATE 48000
// FIXME: set format and sampling based on chosen sink / pa params
#define CHANNELS 1
#define BUFSIZE 20
#define OUTSIZE 9
#define PEAK 32767.0

typedef struct sink {
    char monitor[256];
    char desc[256];
};

static pa_mainloop *main_loop = NULL;
static pa_context *context = NULL;
static pa_stream *stream = NULL;
static pa_mainloop_api *loop_api = NULL;
static pa_sample_spec sample_spec = {.format = FORMAT, .rate = SAMPLERATE, .channels = CHANNELS};
static pa_channel_map channel_map;
static int buf_index = 0;
static double in[BUFSIZE];
static double complex out[OUTSIZE];
static struct sink **sinks;
static int sink_count = 0;
static int sink_index = 0;
static bool sink_cb_signal = false;

fftw_plan plan;

static void stream_state_cb(pa_stream *stream, void *data) {
    const pa_stream_state_t STATE = pa_stream_get_state(stream);
    switch (STATE) {
        // FIXME: actually handle state
        case PA_STREAM_FAILED: 
            fprintf(stderr, "Stream failure\n");
            break;
        default: 
            break;
    }
}

static void stream_read_cb(pa_stream *stream, size_t bytes, void *user_data) {
    if (getch() != ERR) {
        pa_mainloop_quit(main_loop, 0);
        return;
    }
    while (pa_stream_readable_size(stream) > 0) {
        int16_t *stream_data = NULL;
        if (pa_stream_peek(stream, (const void**)&stream_data, &bytes) != 0) {
            fprintf(stderr, "Peek failed\n");
            refresh();
            return;
        }
        if (stream_data == NULL && bytes > 0) {
            if (pa_stream_drop(stream) != 0) {
                fprintf(stderr, "Dropping hole failed\n");
                // I wish someone would drop my hole
                refresh();
                return;
            }
        }
        if (stream_data == NULL && bytes == 0) {
            fprintf(stderr, "Buffer seemingly empty\n");
            refresh();
            return;
        }
        if (buf_index == BUFSIZE) {
            clear();
            fftw_execute(plan);
            for (unsigned int i = 1; i < OUTSIZE; i++) {
                // Starting from 1 because the DC at index 0 is just average amplitude
                double magnitude = sqrt(
                    (creal(out[i]) * creal(out[i])) + 
                    (cimag(out[i]) * cimag(out[i])));
                int bar_height = floor(magnitude * 20);
                for (unsigned int j = 0; j < bar_height; j++) {
                    printw("#");
                }
                printw("#\n");
                refresh();
            }
            buf_index = 0;
        }
        double normal_sample = (*stream_data / PEAK);
        in[buf_index] = normal_sample;
        refresh();
        buf_index++;
        if (pa_stream_drop(stream) != 0) {
            fprintf(stderr, "Dropping after peek failed\n");
        }
    }
}

static void sink_info_cb(pa_context *context, const pa_sink_info *info, int eol, void *user_data) {
    if (eol < 0) {
        fprintf(stderr, "Sink info callback failure\n");
        return;
    }
    if (eol) {
        printw("Found nothing\n");
        return;
    }
    sink_count++;
    realloc(sinks, sizeof(struct sink) * sink_count);
    strcpy(sinks[sink_index]->monitor, info->monitor_source_name);
    strcpy(sinks[sink_index]->desc, info->description);
    sink_index++;
}

static void context_state_cb(pa_context *context, void *main_loop) {
    pa_context_state_t STATE = pa_context_get_state(context);
    switch (STATE) {
        case PA_CONTEXT_READY:
        // there's a bunch of shit happening here i promise i'll explain it someday
            nodelay(stdscr, false);
            cbreak();
            noecho();
            keypad(stdscr, true);
            char chosen_sink[256];
            bool exit_menu = false;

            pa_context_get_sink_info_list(context, &sink_info_cb, sinks);

            ITEM **sink_items;
            ITEM *curr_sink;
            MENU *sink_menu;
            int ch;

            sink_items = (ITEM**) calloc(sink_count + 1, sizeof(ITEM*));
            for (unsigned int i = 0; i < sink_count; i++) {
                sink_items[i] = new_item(sinks[i]->desc, sinks[i]->monitor);
            }
            sink_items[sink_count] = (ITEM*) NULL;
            sink_menu = new_menu((ITEM**) sink_items);
            mvprintw(LINES - 2, 0, "Found %d running sinks.\n", sink_count);
            post_menu(sink_menu);
            refresh();

            while (((ch = getch()) != KEY_EXIT) || !exit_menu) {
                switch(ch) {
                    case KEY_DOWN:
                        menu_driver(sink_menu, REQ_DOWN_ITEM);
                        break;
                    case KEY_UP:
                        menu_driver(sink_menu, REQ_UP_ITEM);
                        break;
                    case KEY_ENTER:
                    case ' ':
                    case '\n':
                        strcpy(chosen_sink, item_description(current_item(sink_menu)));
                        exit_menu = true;
                        break;
                }
            }
            unpost_menu(sink_menu);
            for (unsigned int i = 0; i < sink_count; i++) {
                free_item(sink_items[i]);
            }
            free_menu(sink_menu);
            free(sinks);
            clear();
            refresh();

            pa_channel_map_init_mono(&channel_map);
            if (!(stream = pa_stream_new(context, "aspect", &sample_spec, &channel_map))) {
                fprintf(stderr, "Stream creation failure");
                return;
            }
            pa_stream_set_state_callback(stream, stream_state_cb, NULL);
            pa_stream_set_read_callback(stream, stream_read_cb, NULL);
            pa_stream_connect_record(stream, chosen_sink, NULL, 0);
            break;
        case PA_CONTEXT_FAILED: 
            fprintf(stderr, "Context failure");
            break;
        default:
            break;
    }
}

int main(int argc, char argv[]) {
    // Initialize DFT plan
    plan = fftw_plan_dft_r2c_1d(BUFSIZE, in, out, FFTW_MEASURE);

    // Get the main loop
    main_loop = pa_mainloop_new();
    printf("Made loop\n");
    
    // Get the API
    pa_mainloop_api *loop_api = pa_mainloop_get_api(main_loop);
    printf("Made API\n");
    
    // Create context
    pa_context *context = pa_context_new(loop_api, "aspect");
    printf("Made context\n");

    // Set callback and connect context to server
    pa_context_set_state_callback(context, &context_state_cb, NULL);
    printf("Set state callback\n");
    pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL);
    printf("Connected context\n");

    // Start curses
    initscr();
    nodelay(stdscr, true);
    
    // Iterate forever
    pa_mainloop_run(main_loop, NULL);

    endwin();
    printf("After loop, tearing down\n");

    plan = NULL;
    printf("Destroyed plan\n");

    pa_context_disconnect(context);
    printf("Context disconnected\n");
    pa_context_unref(context);
    printf("Context unref'd\n");

    pa_mainloop_free(main_loop);
    printf("Main loop freed\n");
    return 0;
}
