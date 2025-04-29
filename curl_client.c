#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <errno.h>

#define SERVER_URL "https://127.0.0.1:8080"

static char cert[64];
static char key[64];

#define HTTP_CODE_SUCCESS 200

#define BIG_STRING "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"

struct response_data {
	char *data;
	size_t size;
};

enum client_http_version {
	/* curl doesn't support version 0_9... must manually test it */
	http_1_0,
	http_1_1,
	http_2_0,
};

static size_t
write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	char *ptr;
	const size_t realsize = size * nmemb;
	struct response_data *mem = userp;

	ptr = realloc(mem->data, mem->size + realsize + 1);
	if(!ptr)
		return 0;

	mem->data = ptr;
	memcpy(&(mem->data[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->data[mem->size] = 0;
	return realsize;
}

char *file_contents(const char *path, size_t *len)
{
	FILE *f;
	char *data = NULL;
	size_t file_len;

	*len = 0;
	f = fopen(path, "rw");
	if (!f) {
		printf("fopen() fail, err: '%s'\n", strerror(errno));
		goto open_error;
	}
	/* Calculate the length of the file and allocate that much memory. */
	fseek(f, 0, SEEK_END);
	file_len = ftell(f);
	fseek(f, 0, SEEK_SET);
	data = malloc(file_len + 1);
	if (!data) {
		printf("malloc() fail: out of memory\n");
		goto error;
	}
	if (fread(data, 1, file_len, f) != file_len) {
		printf("fread() fail, err: '%s'", strerror(errno));
		goto error;
	}

	data[file_len] = '\0';
	*len = file_len;
error:
	fclose(f);
open_error:
	return data;
}

/*
 * Validates the http status return code and that the data recieved is correct.
 * Only http_1_0 and http_0_9 are allowed ver values.
 */
static void
test_positive_get_request(const char *endpoint, enum client_http_version ver)
{
	CURL *curl;
	CURLcode res;
	struct response_data chunk = {0};
	char url[256];
	long http_code = 0;
	size_t file_len = 0;
	const char *path = endpoint + 1;

	char *expected_data = file_contents(path, &file_len);
	if (!expected_data) {
		printf("Failed to read expected file: %s\n", path);
		return;
	}
	if (snprintf(url, sizeof(url), "%s%s", SERVER_URL, endpoint) < 0) {
		printf("url truncated. returning\n");
		assert(0);
		goto done;
	}
	curl = curl_easy_init();
	if(!curl) {
		printf("curl init fail\n");
		assert(0);
		goto done;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSLCERT, cert);
	curl_easy_setopt(curl, CURLOPT_SSLKEY, key);
	switch(ver) {
	case http_1_0:
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
		break;
	case http_1_1:
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		break;
	case http_2_0:
		printf("Invalid use of this function. Returning.");
		assert(0);
		return;
	}
	res = curl_easy_perform(curl);

	if(res != CURLE_OK) {
		fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
		goto done;
	}
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
done:
	if (http_code != HTTP_CODE_SUCCESS) {
		printf("Test failed... expected: %lu, actual: %lu, url: %s\n",
		       HTTP_CODE_SUCCESS, http_code, url);
	} else if (chunk.size != file_len) {
        	printf("Test failed... size mismatch. Expected: %zu, Got: %zu, url: %s\n", 
        	       file_len, chunk.size, url);
	} else if (memcmp(chunk.data, expected_data, file_len) != 0) {
		printf("Test failed... content mismatch!\n");
		for (size_t i = 0; i < file_len; i++) {
			if (chunk.data[i] != expected_data[i]) {
				printf("First difference at byte %zu, content = '%s'\n", i, chunk.data);
				break;
			}
		}
	} else {
		printf("Test passed... method: GET, http_code: %d, url: '%s'\n", http_code, url);
	}
	free(chunk.data);
	curl_easy_cleanup(curl);
	return;
}

static void
test_request(const char *endpoint, enum client_http_version ver,
		 long expected_http_code, const char *post_data)
{
	CURL *curl;
	CURLcode res;
	struct response_data chunk = {0};
	char url[256];
	long http_code = 0;

	if (snprintf(url, sizeof(url), "%s%s", SERVER_URL, endpoint) < 0) {
		printf("url truncated. returning\n");
		assert(0);
		goto done;
	}
	curl = curl_easy_init();
	if(!curl) {
		printf("curl init fail\n");
		assert(0);
		goto done;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSLCERT, cert);
	curl_easy_setopt(curl, CURLOPT_SSLKEY, key);
	switch(ver) {
	case http_1_0:
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
		break;
	case http_1_1:
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		break;
	case http_2_0:
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
		break;
	}
	if (post_data != NULL) {
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
	}

	res = curl_easy_perform(curl);

	if(res != CURLE_OK) {
		fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
		goto done;
	}
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
done:
	free(chunk.data);
	curl_easy_cleanup(curl);
	if (http_code != expected_http_code) {
		printf("Test failed... expected: %lu, actual: %lu, url: %s\n",
		       expected_http_code, http_code, url);
	} else {
		printf("Test passed... method: %s, http_code: %d, url: '%s'\n",
		       post_data != NULL ? "POST" : "GET", http_code, url);
	}
	return;
}

static void
test_get_request(const char *endpoint, enum client_http_version ver,
		 long expected_http_response)
{
	return test_request(endpoint, ver, expected_http_response, NULL);
}

static void
test_post_request(const char *endpoint, enum client_http_version ver,
		  long expected_http_response, const char *post_data)
{
	return test_request(endpoint, ver, expected_http_response, post_data);
}

void test_custom_request(const char *request, long expected_http_code) {
	CURL *curl = curl_easy_init();
	long http_code = 0;
	
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request);
	curl_easy_setopt(curl, CURLOPT_URL, SERVER_URL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSLCERT, cert);
	curl_easy_setopt(curl, CURLOPT_SSLKEY, key);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
	
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	if (http_code != expected_http_code)
		printf("Test failed... expected: %lu, actual: %lu, request = %s\n",
		expected_http_code, http_code, request);
	else
		printf("Test passed... http_code: %lu, request = '%s'\n",
		       http_code, request);
	
	curl_easy_cleanup(curl);
}

static void
test_malicious_requests()
{
	printf("Starting malicious requests...\n");
	test_get_request("/test/etc/passwd%00.txt", http_1_0, 999);
	test_get_request("/%00/test/etc/passwd.txt", http_1_0, 999);
	test_get_request("/test/etc/passwd.txt%00", http_1_0, 999);
	test_post_request("/test/etc/passwd%00.txt", http_1_0, 999, "Request body");
	test_post_request("/test/etc/passwd.txt%00", http_1_0, 999, "Request body");

	test_get_request("/test/UNION/mantra.txt", http_1_0, 999);
	test_get_request("/test/SELECT/mantra.txt", http_1_0, 999);
	test_get_request("/test/DROP/mantra.txt", http_1_0, 999);
	test_get_request("/test/'/mantra.txt", http_1_0, 999);
	test_get_request("/test/;/mantra.txt", http_1_0, 999);
	test_get_request("/test/--/mantra.txt", http_1_0, 999);
	test_get_request("/test/*/mantra.txt", http_1_0, 999);
	test_get_request("/test*//mantra.txt", http_1_0, 999);

	test_post_request("/test/UNION/mantra.txt", http_1_0, 999,"Request body");
	test_post_request("/test/SELECT/mantra.txt", http_1_0, 999,"Request body");
	test_post_request("/test/DROP/mantra.txt", http_1_0, 999,"Request body");
	test_post_request("/test/'/mantra.txt", http_1_0, 999,"Request body");
	test_post_request("/test/;/mantra.txt", http_1_0, 999,"Request body");
	test_post_request("/test/--/mantra.txt", http_1_0, 999,"Request body");
	test_post_request("/test/*/mantra.txt", http_1_0, 999,"Request body");
	test_post_request("/test*//mantra.txt", http_1_0, 999,"Request body");

	/*
	 * curl limitation: '..' will be removed from URLs if you try using
	 * the CURLOPT_URL option. Instead pass a request in the form of a
	 * string and use CURLOPT_CUSTOMREQUEST.
	 */
	test_custom_request("GET /test/../etc/passwd.txt HTTP/1.0\r\n\r\n", 999);
	test_custom_request("GET ../test/etc/passwd.txt HTTP/1.0\r\n\r\n", 999);
	test_custom_request("GET /test/etc/passwd.txt.. HTTP/1.0\r\n\r\n", 999);
}

static void
test_positive_requests()
{
	printf("Starting positive requests...\n");
	test_positive_get_request("/test/mantra.txt", http_1_0);
	test_positive_get_request("/test/harry_potter.txt", http_1_0);
	/* POST then verify with a GET */
	test_post_request("/test/post_test.txt", http_1_0, 200, "Hey it's Adam.\n");
	test_positive_get_request("/test/post_test.txt", http_1_0);
	test_post_request("/test/post_test2.txt", http_1_0, 200, "BeepBoop.\n");
	test_positive_get_request("/test/post_test2.txt", http_1_0);
	test_post_request("/test/post_test3.txt", http_1_0, 200, "");
	test_positive_get_request("/test/post_test3.txt", http_1_0);
}

static void
test_negative_requests()
{
	printf("Starting negative requests...\n");
	/* Resources unavailible */
	test_get_request("/test/banana.txt", http_1_0, 404);
	test_get_request("/beep/banana.txt", http_1_0, 404);
	test_get_request("/banana.txt", http_1_0, 404);
	/* Version unsupported */
	test_get_request("/test/mantra.txt", http_1_1, 505);
	test_get_request("/test/mantra.txt", http_2_0, 505);
	/* Method unsupported */
	test_custom_request("BANANA /test/mantra.txt HTTP/1.0\r\n\r\n", 405);
	test_custom_request("DELETE /test/mantra.txt HTTP/1.0\r\n\r\n", 405);
	test_custom_request("ORANGE /test/mantra.txt HTTP/1.0\r\n\r\n", 405);
	test_custom_request("NNN /test/mantra.txt HTTP/1.0\r\n\r\n", 405);
	/* Oversized Request */
	test_post_request("/test/b.txt", http_1_0, 400, BIG_STRING);
}

int main(int argc, char **argv) {
	curl_global_init(CURL_GLOBAL_ALL);
	
	memset(cert, 0, sizeof(cert));
	memset(key, 0, sizeof(key));

	while(++argv, --argc != 0) {
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
				case 'c':
				strncpy(cert, argv[1], sizeof(cert));
				break;
			case 'k':
				strncpy(key, argv[1], sizeof(key));
				break;
			}
		}
	}

	printf("Starting tests...\n");

	/*
	 * Test POST and GET requests. Verify that the return code is 200,
	 * indicating success. 
	 * 
	 * Every POST will be followed by a GET for the same resource.
	 * 
	 * Verification of GET requests do a byte-by-byte comparison on file
	 * contents and the server response body.
	 */
	test_positive_requests();
	/*
	 * Test POST and GET requests. Verify that the return code is correct
	 * depending on the request sent...
	 * 
	 * - 400 Bad Request (Oversized Request)
	 * - 404 Not Found
	 * - 405 Method Unsupported
	 * - 505 Version Unsupported
	 * - *** Not test-able ***: 500 Internal Error. This error occurs if
	 *   there's a server malloc error, or error when the server encounters
	 *   an error reading/writing to the endpoint.
	 */
	test_negative_requests();
	/*
	 * Test the following malicious requests:
	 *
	 * 1) URL-encoded null byte injection
	 * 2) SQL injection
	 * 3) Path traversal attack
	 * 
	 * Verify that the return code is 999 indicating a malicious request
	 * was detected by the server.
	 */
	test_malicious_requests();
	curl_global_cleanup();
	return 0;
}