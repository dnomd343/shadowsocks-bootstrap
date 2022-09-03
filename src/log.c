#include "log.h"

int log_level = LOG_DEBUG; // default log level

static const char *log_string[] = {
    "[DEBUG]",
    "[INFO]",
    "[WARN]",
    "[ERROR]",
    "[FATAL]",
};

static const char *log_color[] = {
    "\x1b[39m", // debug
    "\x1b[32m", // info
    "\x1b[33m", // warn
    "\x1b[31m", // error
    "\x1b[95m", // fatal
};

void log_printf(int level, const char *fmt, ...) {
    if (level < log_level) { // skip low log level
        return;
    }
    time_t t = time(NULL);
    char time_str[20]; // YYYY-mm-dd HH:MM:SS (20 bytes)
    time_str[strftime(time_str, 20, "%Y-%m-%d %H:%M:%S", localtime(&t))] = '\0'; // generate time str

    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "\x1b[36m[Bootstrap]\x1b[0m \x1b[90m%s\x1b[0m", time_str); // show log prefix
    fprintf(stderr, " %s%s\x1b[0m ", log_color[level], log_string[level]); // show log level
    vfprintf(stderr, fmt, ap); // output log content
    fprintf(stderr, "\n"); // add LF after line
    fflush(stderr);
    va_end(ap);
}
