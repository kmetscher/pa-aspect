#include <unistd.h>
#include <argp.h>

typedef struct argp_option argp_option;

// POSIX arguments
const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "howdy@kylemetscher.com";

// Option arguments
argp_option colors = {
    .name = "colors", 
    .key = 'c', 
    .arg = 0, 
    .flags = OPTION_ARG_OPTIONAL, 
    .doc = "Turns on terminal colors for amplitude representation", 
    .group = 0
};

argp_option lines = {
    .name = "lines", 
    .key = 'l', 
    .arg = "number", 
    .flags = OPTION_ARG_OPTIONAL, 
    .doc = "Specify the height of the spectrograph (ie, the number of lines taken up; terminal emulator max by default)", 
    .group = 0
};

argp_option rows = {
    .name = "rows",
    .key = 'r',
    .arg = "number",
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "Specify the width of the spectrograph (ie, the number of rows taken up; terminal emulator max by default)",
    .group = 0
};

argp_option target_latency = {
    .name = "target-latency",
    .key = 't',
    .arg = "microseconds",
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "Specify a target latency in microseconds; does not guarantee it will be reached. Default is 5000",
    .group = 0
};

