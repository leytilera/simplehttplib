#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "httplib.h"
#include "httplib_internal.h"

int httplib_start_server(int port, http_request_handler handler) {

    int rfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rfd < 0) {
        return SOCKET_ERROR;
    }
    int option = 1;
    setsockopt(rfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    struct sockaddr_in server;
    struct sockaddr client;
    socklen_t client_len = sizeof(struct sockaddr);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if (bind(rfd, (const struct sockaddr *) &server, sizeof(server)) < 0) {
        return BIND_ERROR;
    }
    if (listen(rfd, 3) < 0) {
        return LISTEN_ERROR;
    }

    int cfd;

    while (1) {
        cfd = accept(rfd, &client, &client_len);
        if (cfd < 0) {
            printf("Connection Error, waiting for next connection\n");
            continue;
        }
        int pid = fork();
        if (pid == 0) {

            http_request * req = httplib_request_new();

            int result = httplib_receive(cfd, req);

            http_response * res = httplib_response_new();

            if (result == 1) handler(req, res); //TODO HTTP unsupported version response
            else if (result == 4) httplib_response_set_status(res, 400, "Bad Request");
            else httplib_response_set_status(res, 500, "Internal Server Error");

            httplib_request_free(req);

            httplib_respond(cfd, res);

            close(cfd);
            exit(0);
        }
        close(cfd);
    }
}

int httplib_receive(int cfd, http_request * req_out) {
    char buf[1024];
    size_t bytes_received;

    enum req_pos pos = METHOD;
    int was_cr = 0;
    int result = 0;

    struct tmp tmp;
    tmp.buf = malloc(1024);
    bzero(tmp.buf, 1024);
    tmp.size = 1024;
    tmp.pos = 0;

    do {
        bytes_received = read(cfd, buf, 1024);
        if ((result = httplib_parse_buffer(buf, bytes_received, req_out, &tmp, &pos, &was_cr))) break;
    } while (bytes_received > 0);
    free(tmp.buf);
    return result;
}

void httplib_respond(int cfd, http_response * res) {
    size_t res_size = httplib_response_string_size(res);
    char * res_bytes = malloc(res_size);
    int ser_err = httplib_response_serialize(res, res_bytes);
    if (ser_err != 0) {
        http_response * err_res = httplib_response_new();
        httplib_response_set_status(res, 500, "Internal Server Error");
        res_bytes = realloc(res_bytes, httplib_response_string_size(err_res));
        httplib_response_serialize(err_res, res_bytes);
        free(err_res);
    }
    httplib_response_free(res);
    write(cfd, res_bytes, res_size);
    free(res_bytes);
}

