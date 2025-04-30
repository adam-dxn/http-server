# HTTP Server

Here's my [HTTP 1.0 compliant](https://datatracker.ietf.org/doc/html/rfc1945) server written completely from scratch in C. With a few more features.

This server binds to port `8080` and spawns workers that wait to process `GET`, `POST` and `PUT` requests.

## Certificate validation

Although not required for HTTP 1.0, plaintext data should not be sent through the internet. That's why I've added SSL encryption and certificate validations as server requirements.

[Here's](https://www.baeldung.com/openssl-self-signed-cert) how you can generate your own self-signed certificate and key file.

## How to build and run the server

Make sure that you have [git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git), [gcc](https://gcc.gnu.org/install/), [OpenSSL](https://docs.openiam.com/docs-4.2.1.3/appendix/2-openssl) and [curl](https://www.cyberciti.biz/faq/how-to-install-curl-command-on-a-ubuntu-linux/) installed. Clone this repository and enter the git directory:

```
# Navigate to the project directory
cd http_server

# Run 'make' or 'make all'
make

# To build the test suite, run 'make tests'
make tests
```

To run the server you must provide the executable `http_server` with valid arguments.

| Option | Description   | Values                          |                 
|--------|------------|--------------------------------------|
| `-d`   | Debug level                           | `0-2` (None, Info, Verbose)                  |
| `-m`   |     Maximum request size in bytes             | `1 - 10000000` (10MB)         |
| `-c`   |  Path to the certificate file                 | `string`                      |
| `-k`   |             Path to the key file             | `string`                      |
| `-n`   | Number of worker processes                    | `1 - 16`                      |    
| `-h`   | Display helptext regarding server arguments                    | `none`                      |   

Ex: `./http_server -n 4 -m 10000000 -c server.crt -k server.key -d 2`

## CLI

Users can view stats, traffic logs, restart workers and edit the config at any time. Upon boot users will enter the CLI:

```Enter a command:
Enter a command:
'stats'	       : Display server stats.
'exit'         : Shutdown the server.
'logs'         : Display server traffic logs.
'config'       : Display/edit server config.
'restart'      : Restart workers.
server> 
```

## Automated Test Suite

The test suite contains over 40 test, implemented via [libcurl](https://curl.se/libcurl/). Run the `curl_client` executable with `-c` and `-k` arguments:

- `./curl_client -c server.crt -k server.key`

Alternatively, to manually test the server you can send a curl request:

- `curl -k --http1.0 --cert server.crt --key server.key https://127.0.0.1:8080/test/mantra.txt`

**Note:** Run the server with a maximum request size of `2000` bytes for the oversized request tests to pass.

## Stats

The `stats` command has been added to view tallies of request metadata, malicious requests, memory leaks and server failures. 

**Note:** Every `manager.mem.xxx_malloc` must have the same `manager.mem.xxx_free` value. Otherwise there's been a memory leak.

## Logs
Users can view traffic logs via the `logs` command. Here's the log of an HTTP GET on `test/mantra.txt`:

```
1: date="Sat, 12 Apr 2025 16:50:39 GMT", uri="/test/mantra.txt", status="HTTP/1.0 200 OK", method="GET", version="HTTP/1.0", content-length="2254", subject="/C=CA/ST=BC/L=Vancouver/O=Adam/CN=Adam Dixon/emailAddress=adamdxn97@gmail.com", issuer="/C=CA/ST=BC/L=Vancouver/O=Adam/CN=Adam Dixon/emailAddress=adamdxn97@gmail.com", ip_addr="127.0.0.1", port="53606", worker_pid="11262"
```
