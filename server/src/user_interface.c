#include <server.h>
#include <server_stats.h> /* stats_table_show() */
#include <log.h>

extern struct manager *m;

static void
ui_server_commands_show()
{
	printf("Enter a command:\n"
	       "'stats'	       : Display server stats.\n"
	       "'exit'         : Shutdown the server.\n"
	       "'logs'         : Display server traffic logs.\n"
	       "'config'       : Display/edit server config.\n"
	       "'restart'      : Restart workers.\n");
}

static void
ui_config_show()
{
	printf("Config:\n");
	printf("  set debug %d\n", m->cfg->debug);
	printf("  set cert %s\n", m->cfg->cert);
	printf("  set key %s\n", m->cfg->key);
	printf("  set mreq %zu\n", m->cfg->max_buffer_size);
	printf("  set workers %u\n", m->cfg->num_workers);
}

static void
ui_config_show_commands()
{
	printf("Configuration commands:\n"
	       "'set debug <level>'         : Set debug level (0 - 2).\n"
	       "'set mreq <size_mb>'        : Set the maximum request size (in bytes).\n"
	       "'set cert <path>'           : Set the certificate file path.\n"
	       "'set key <path>'            : Set the key file path.\n"
	       "'set workers <num>'         : Set the number of worker processes.\n");
}

static void
ui_config_show_menu()
{
	printf("Enter a command:\n"
	       "'cmds'          : Display config commands.\n"
	       "'show'          : Display current server config.\n"
	       "'done'          : Exit config mode.\n");
}

static void
ui_debug_level_set(char *input)
{
	int level = atoi(input + strlen("set debug "));

	if (level < 0 || level > 2) {
		printf("Invalid debug level. Must be between 0 and 2.\n");
		return;
	}
	m->cfg->debug = level;
	printf("Debug level set to %d.\n", level);
}

static void
ui_max_request_size_set(char *input)
{
	int size = atoi(input + strlen("set mreq "));

	if (size <= 0 || size > 10000000) { /* 10MB max */
		printf("Invalid size. Must be between 1 and 10,000,000 bytes.\n");
		return;
	}
	m->cfg->max_buffer_size = (size_t)size;
	printf("Max request size set to %zu bytes.\n", m->cfg->max_buffer_size);
}

static void
ui_cert_path_set(char *input)
{
	char *path = input + strlen("set cert ");

	path[strcspn(path, "\n")] = '\0';
	strncpy(m->cfg->cert, path, sizeof(m->cfg->cert) - 1);
	m->cfg->cert[sizeof(m->cfg->cert) - 1] = '\0';
	printf("Cert path set to '%s'.\n", m->cfg->cert);
}

static void
ui_key_path_set(char *input)
{
	char *path = input + strlen("set key ");

	path[strcspn(path, "\n")] = '\0';
	strncpy(m->cfg->key, path, sizeof(m->cfg->key) - 1);
	m->cfg->key[sizeof(m->cfg->key) - 1] = '\0';
	printf("Key path set to '%s'.\n", m->cfg->key);
}

static void
ui_worker_count_set(char *input)
{
	int num = atoi(input + strlen("set workers "));

	if (num <= 0 || num > MAX_WORKERS) {
		printf("Invalid number of workers. Must be between 1 and %d.\n", MAX_WORKERS);
		return;
	}
	m->cfg->num_workers = (unsigned int)num;
	printf("Number of workers set to %u.\n", m->cfg->num_workers);
}

static void
ui_input_buf_flush()
{
	int c;
	while ((c = getchar()) != '\n' && c != EOF)
		;;
}

static void
ui_config_edit()
{
	char in[32];

	while (1) {
		if (in[0] != '\n')
			ui_config_show_menu();
		printf("server/config> ");
		if (!fgets(in, sizeof(in), stdin))
			break;
		if (!strchr(in, '\n')) {
			printf("Invalid command. Too many characters (32 max).\n");
			ui_input_buf_flush();
			continue;
		} else if (!strncmp(in, "show\n", strlen("show\n")))
			ui_config_show();
		else if (!strncmp(in, "cmds\n", strlen("cmds\n")))
			ui_config_show_commands();
		else if (!strncmp(in, "set debug ", strlen("set debug ")))
			ui_debug_level_set(in);
		else if (!strncmp(in, "set mreq ", strlen("set mreq ")))
			ui_max_request_size_set(in);
		else if (!strncmp(in, "set cert ", strlen("set cert ")))
			ui_cert_path_set(in);
		else if (!strncmp(in, "set key ", strlen("set key ")))
			ui_key_path_set(in);
		else if (!strncmp(in, "set workers ", strlen("set workers ")))
			ui_worker_count_set(in);
		else if (!strncmp(in, "done\n", strlen("done\n"))) {
			printf("Exiting config edit mode.\n");
			break;
		} else if (in[0] != '\n')
			printf("Unknown command. Try again.\n");
	}
	return;
}

void
ui_run()
{
	char in[16];

	while (1) {
		if (in[0] != '\n')
			ui_server_commands_show();
		printf("server> ");
		if (!fgets(in, sizeof(in), stdin))
			break;
		if (!strchr(in, '\n')) {
			printf("Invalid command. Too many characters (16 max).\n");
			ui_input_buf_flush();
		} else if (!strncmp(in, "stats\n", strlen("stats\n"))) {
			stats_table_show();
		} else if (!strncmp(in, "exit\n", strlen("exit\n"))) {
			return;
		} else if (!strncmp(in, "restart\n", strlen("restart\n"))) {
			manager_shutdown_workers();
			manager_fork_workers();
		} else if (!strncmp(in, "config\n", strlen("config\n"))) {
retry_config:
			ui_config_edit();
			if (!manager_shutdown_workers()) {
				exit(EXIT_FAILURE);
			}
			if (!manager_fork_workers()) {
				printf("Invalid config, unable to fork workers. Try again.\n");
				goto retry_config;
			}
		} else if (!strncmp(in, "logs\n", strlen("logs\n"))) {
			logs_show();
		} else {
			if (in[0] != '\n')
				printf("Invalid command.\n");
		}
	}
}