/* Copyright 2022 Kyle Metscher
 * howdy@kylemetscher.com | https://github.com/kmetscher 
 * PulseAudio Aspect, part of the Aspect project, is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License as published by the Free Software Foundation, either version
 * 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. 
 * If not, see <https://www.gnu.org/licenses/>. */

#include <pulse/def.h>
#include <pulse/sample.h>
#include <stdint.h>
#include <stdio.h>
#include <ncurses.h>
#include <menu.h>
#include <pulse/pulseaudio.h>
#include <complex.h>
#include <fftw3.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <argp.h>
#ifndef OPTIONS
#define OPTIONS
#include "options.c"
#endif

#define FORMAT PA_SAMPLE_S16LE
#define SAMPLERATE 48000
// FIXME: set format and sampling based on chosen sink / pa params
#define CHANNELS 1
#define BUFSIZE 32
#define OUTSIZE 15
#define PEAK 131070.0

typedef struct sink {
    char description[256];
    char monitor[256];
} sink;

static pa_mainloop *main_loop = NULL;
static pa_context *context = NULL;
static pa_stream *stream = NULL;
static pa_mainloop_api *loop_api = NULL;
static pa_sample_spec sample_spec = {.format = FORMAT, .rate = SAMPLERATE, .channels = CHANNELS};
static pa_channel_map channel_map;
static int buf_index = 0;
static double in[BUFSIZE];
static double complex out[OUTSIZE];
static int sink_count = 0;
static int sink_index = 0;
static int TERMCOLS = 0;
static int TERMROWS = 0;

sink *sinks;
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
    int rows;
    int cols;
    getmaxyx(stdscr, rows, cols);
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
            fftw_execute(plan);
            int picture[OUTSIZE];
            for (unsigned int i = 1; i < OUTSIZE; i++) {
                // Starting from 1 because the DC at index 0 is just average amplitude
                double magnitude = sqrt(
                    (creal(out[i]) * creal(out[i])) + 
                    (cimag(out[i]) * cimag(out[i])));
                int bar_height = floor(magnitude * (LINES));
                picture[i - 1] = bar_height;
            }
            clear();
            for (int i = rows; i >= 0; i--) {
                for (int j = 0/*(OUTSIZE)*/; j < cols - 1; j++) {
                    if (picture[j / OUTSIZE] >= (LINES * 2) / 3) {
                        attron(COLOR_PAIR(3));
                    }
                    else if (picture[j / OUTSIZE] >= LINES / 3) {
                        attron(COLOR_PAIR(2));
                    }
                    else {
                        attron(COLOR_PAIR(1));
                    }
                    if (picture[j / OUTSIZE] >= i) {
                        printw("█");
                    }
                    else {
                        printw(" ");
                    }
                }
                printw("\n");
            }
            buf_index = 0;
            refresh();
        }
        double normal_sample = (*stream_data / PEAK);
        in[buf_index] = normal_sample;
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
    if (eol) { // there's a bunch of shit happening here i promise i'll explain it someday
        cbreak();
        noecho();
        keypad(stdscr, true);
        char chosen_sink[256];
        bool exit_menu = false;
        int ch;
        WINDOW *startup_window;
        WINDOW *menu_window;
        ITEM **sink_items;
        MENU *sink_menu;
        ITEM *curr_sink;
        
        sink_items = (ITEM**) calloc(sink_count + 1, sizeof(ITEM*));
        for (unsigned int i = 0; i < sink_count; i++) {
            sink_items[i] = new_item(sinks[i].description, sinks[i].monitor);
        }
        sink_items[sink_count] = (ITEM*) NULL;
        sink_menu = new_menu((ITEM**) sink_items);
        menu_opts_off(sink_menu, O_SHOWDESC);
        mvprintw(sink_count + 1, 0, "                           | |  \n   __ _ ___ _ __   ___  ___| |_ \n  / _` / __| '_ \\ / _ \\/ __| __|\n | (_| \\__ \\ |_) |  __/ (__| |_ \n  \\__,_|___/ .__/ \\___|\\___|\\__|\n           | |                  \n           |_|\n");
        // LOL
        printw("\nPulseAudio Aspect\nCopyright 2022 Kyle Metscher\nhowdy@kylemetscher.com\nAspect is free software under the terms of the GNU GPL v3 or later.\n");
        printw("Found %d sinks.\nChoose a sink with up/down and select with space/enter.\n", sink_count);
        post_menu(sink_menu);
        refresh();
        nodelay(stdscr, true);
        ch = getch();
        while (!exit_menu) {
            ch = getch();
            switch(ch) {
                case KEY_DOWN:
                    menu_driver(sink_menu, REQ_DOWN_ITEM);
                    refresh();
                    break;
                case KEY_UP:
                    menu_driver(sink_menu, REQ_UP_ITEM);
                    refresh();
                    break;
                case ' ':
                case KEY_ENTER:
                case '\n':
                    exit_menu = true;
                    refresh();
                    break;
            }
        }
        strcpy(chosen_sink, item_description(current_item(sink_menu)));
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
        pa_buffer_attr buffer_attrs = {.maxlength = (uint32_t) -1, .fragsize = pa_usec_to_bytes(11000, &sample_spec), .minreq = (uint32_t) -1, .prebuf = (uint32_t) -1, .tlength = (uint32_t) -1};
        pa_stream_set_read_callback(stream, stream_read_cb, NULL);
        pa_stream_connect_record(stream, chosen_sink/*"alsa_output.pci-0000_00_1f.3-platform-skl_hda_dsp_generic.HiFi__hw_sofhdadsp__sink.monitor"*/, &buffer_attrs, PA_STREAM_ADJUST_LATENCY);
        refresh();
        return;
    }
    else {
        sink_count++;
        sinks = realloc(sinks, sink_count * sizeof(sink));
        strcpy(sinks[sink_index].description, info->description);
        strcpy(sinks[sink_index].monitor, info->monitor_source_name);
        refresh();
        sink_index++;
        return;
    }
}

static void context_state_cb(pa_context *context, void *main_loop) {
    pa_context_state_t STATE = pa_context_get_state(context);
    switch (STATE) {
        case PA_CONTEXT_READY:
            pa_context_get_sink_info_list(context, &sink_info_cb, sinks);
            break;
        case PA_CONTEXT_FAILED: 
            fprintf(stderr, "Context failure");
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    args arguments;
    argp_parse(&parser, argc, argv, 0, 0, &arguments);

    setlocale(LC_CTYPE, "");
    // Initialize DFT plan
    plan = fftw_plan_dft_r2c_1d(BUFSIZE, in, out, FFTW_MEASURE);

    // Get the main loop
    main_loop = pa_mainloop_new();
    
    // Get the API
    pa_mainloop_api *loop_api = pa_mainloop_get_api(main_loop);
    
    // Create context
    pa_context *context = pa_context_new(loop_api, "aspect");

    // Set callback and connect context to server
    pa_context_set_state_callback(context, &context_state_cb, NULL);
    pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL);

    // Start curses
    initscr();
    nodelay(stdscr, true);
    if (arguments.colors == 1) {
        start_color();
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
    }
    // Iterate forever
    pa_mainloop_run(main_loop, NULL);

    endwin();

    plan = NULL;

    pa_context_disconnect(context);
    pa_context_unref(context);

    pa_mainloop_free(main_loop);
    return 0;
}
