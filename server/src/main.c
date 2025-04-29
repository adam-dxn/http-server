#include <stdio.h> /* printf() */
#include <stdlib.h> /* malloc()/free() */
#include <string.h> /* strncpy() */
#include <server.h> /* manager_server_run() */

void user_interface_help()
{
	printf("Server input: ./server -n <number of workers> -m <maximum request size> ... options\n"
	       "   -n : Number of workers waiting on requests (maximum of 16)\n"
	       "   -m : Maximum request size in bytes (maximum of 10MB)\n"
	       "   -c : Certificate file path used for certificate validation\n"
	       "   -k : Certificate key file used for certificate validation\n"
	       "   -d : Debug level (0 = none, 1 = info, 2 = verbose)\n");
}
/*
 * Sample run of the executable: ./http_server -n 4 -m 10000000 -c server.crt -k server.key -d 2 -h
 * To test: curl -v --cert server.crt --key server.key --cacert server.crt  --http1.0 -k -X GET https://127.0.0.1:8080/file.html
 */
int main(int argc, char **argv) {

	struct manager *m = malloc(sizeof(struct manager));
	if (!m)
		return 0;
	m->cfg = malloc(sizeof(struct server_config));
	if (!m->cfg)
		return 0;

	while(++argv, --argc != 0) {
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
			case 'h':
				user_interface_help();
				break;
			case 'n':
				{
					int num_workers = atoi(argv[1]);
					if (num_workers > 0 && num_workers <= MAX_WORKERS)
						m->cfg->num_workers = num_workers;
					else
						printf("Invalid number of workers.\n");
					break;
				}
			case 'm':
				{
					int max_req = atoi(argv[1]);
					if (max_req > 0 && max_req <= 10000000) /* 10MB max */
						m->cfg->max_buffer_size = max_req;
					else
						printf("Invalid max request size.\n");
					break;
				}
			case 'c':
				strncpy(m->cfg->cert, argv[1], sizeof(m->cfg->cert));
				break;
			case 'k':
				strncpy(m->cfg->key, argv[1], sizeof(m->cfg->key));
				break;
			case 'd':
				{
					int debug = atoi(argv[1]);
					if (debug >= 0 && debug < 3)
						m->cfg->debug = debug;
				}
			default:
				break;
			}
		}
	}
	if (!m->cfg->num_workers || m->cfg->cert[0] == '\0' || m->cfg->key[0] == '\0') {
		printf("Invalid config. At least one worker, a certificate file, and a key file are required. Use the '-h' option for help\n");
		goto init_fail;
	}

	manager_server_run(m);
init_fail:
	free(m->cfg);
	free(m);
	return 0;
}