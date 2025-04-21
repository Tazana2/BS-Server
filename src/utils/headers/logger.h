#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

void init_logger(const char *filename);
void close_logger();
void log_info(const char *format, ...);
void log_error(const char *format, ...);

#endif
