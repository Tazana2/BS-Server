#include "headers/logger.h"
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

static FILE *log_file = NULL;

void init_logger(const char *filename) {
    log_file = fopen(filename, "w");
    if (!log_file) {
        perror("Could not open the log file.");
        exit(EXIT_FAILURE);
    }
}

void close_logger() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

void log_with_prefix(const char *level, const char *format, va_list args) {
    if (!log_file) return;

    // Timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log_file, "[%02d-%02d-%04d %02d:%02d:%02d] [%s] ",
        t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
        t->tm_hour, t->tm_min, t->tm_sec,
        level);

    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");
    fflush(log_file); // para que se escriba inmediatamente
}

void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_with_prefix("INFO", format, args);
    va_end(args);
}

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_with_prefix("ERROR", format, args);
    va_end(args);
}
