#include <server.h>
#include <libgen.h>

#define debug_print(level, format, ...) \
    _console_print(level, basename(__FILE__), __LINE__, format, ##__VA_ARGS__)

void _console_print(enum server_debug_level level, const char *file, int line,
	     	    const char *format, ...);
