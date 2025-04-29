#include <assert.h>
#include <server.h>

extern struct manager *m;

void
_console_print(enum server_debug_level level, const char *file, int line,
	      const char *format, ...)
{
	if (level > m->cfg->debug)
		return;
	printf("[dbg] %s:%d: ", file, line);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}