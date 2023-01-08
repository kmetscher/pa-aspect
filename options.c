#ifndef OPTIONS_H
#define OPTIONS_H
#include <unistd.h>
#include <stdlib.h>
#include <argp.h>
#endif

typedef struct argp_option argp_option;
typedef struct argp argp;

// Regulatory compliance
const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "howdy@kylemetscher.com";

// Option arguments
static argp_option options[] = {
    {
        .name = "colors", 
        .key = 'c', 
        .arg = 0, 
        .flags = OPTION_ARG_OPTIONAL, 
        .doc = "Turns on terminal colors for amplitude representation", 
        .group = 0
    },

    {
        .name = "lines", 
        .key = 'l', 
        .arg = "number", 
        .flags = OPTION_ARG_OPTIONAL, 
        .doc = "Specify the height of the spectrograph (ie, the number of lines taken up; terminal emulator max by default)", 
        .group = 0
    },

    {
        .name = "rows",
        .key = 'r',
        .arg = "number",
        .flags = OPTION_ARG_OPTIONAL,
        .doc = "Specify the width of the spectrograph (ie, the number of rows taken up; terminal emulator max by default)",
        .group = 0
    },

    {
        .name = "target-latency",
        .key = 't',
        .arg = "microseconds",
        .flags = OPTION_ARG_OPTIONAL,
        .doc = "Specify a target latency in microseconds; does not guarantee it will be reached. Default is 5000",
        .group = 0
    },

    { 0 } // Null termination
};

// State values
typedef struct args {
    char *args[4];
    int colors, lines, rows, target_latency;
} args;

// Argument parser 
static error_t parse_arg(int key, char *arg, struct argp_state *state) {
    args *arguments = state->input;
    switch (key) {
        case 'c':
            arguments->colors = arg ? 1 : 0;
            break;
        case 'l':
            arguments->lines = arg ? atoi(arg) : 0;
            break;
        case 'r':
            arguments->rows = arg ? atoi(arg) : 0;
            break;
        case 't':
            arguments->target_latency = arg ? atoi(arg) : 0;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 4) {
                argp_usage(state);
            }
            arguments->args[state->arg_num] = arg;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

// Parser
argp parser = {
    .options = options,
    .parser = parse_arg
};
