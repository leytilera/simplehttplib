#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "httplib.h"
#include "httplib_internal.h"

int httplib_header_serialize(http_header * header, char * str_out) {
    if (!httplib_is_valid_header(header->key, header->value))
        return -1;

    strcpy(str_out, header->key);
    strcat(str_out, ":");
    strcat(str_out, header->value);
    return 0;
}

size_t httplib_header_string_size(http_header * header) {
    size_t key_len = strlen(header->key);
    size_t value_len = strlen(header->value);
    return key_len + value_len + 2;
}

int httplib_response_serialize(http_response * res, char * str_out) {
    strcpy(str_out, "HTTP/1.1 ");
    if (res->status_code < 100 || res->status_code > 599) return -1;
    char status[4];
    sprintf(status, "%i", res->status_code);
    strcat(str_out, status);
    strcat(str_out, " ");
    strcat(str_out, res->status_message);
    strcat(str_out, "\r\n");
    http_header * run = res->headers;
    while (run) {
        char * tmp = malloc(httplib_header_string_size(run));
        int err = httplib_header_serialize(run, tmp);
        if (err != 0) return err;
        strcat(str_out, tmp);
        free(tmp);
        strcat(str_out, "\r\n");
        run = run->next;
    }
    if (res->content_length > 0) {
        strcat(str_out, "Content-Length:");
        char * tmp = malloc(2 + (res->content_length / 10));
        sprintf(tmp, "%lu", res->content_length);
        strcat(str_out, tmp);
        free(tmp);
        strcat(str_out, "\r\n");
    }
    strcat(str_out, "\r\n");
    size_t end_pos = strlen(str_out);
    if (res->body != 0) {
        for (size_t i = 0; i < res->content_length; i++) {
            str_out[end_pos + i] = res->body[i];
        }
    }
    return 0;
}

size_t httplib_response_string_size(http_response * res) {
    size_t res_line = 15 + strlen(res->status_message); //"HTTP/1.1" 000 (message)\r\n
    size_t headers = 2; // \r\n
    http_header * run = res->headers;
    while (run) {
        headers += httplib_header_string_size(run) + 1; // \r\n instead of \0
        run = run->next;
    }
    if (res->content_length > 0) {
        headers += 18;
        headers += res->content_length / 10;
    }
    size_t body = res->content_length;
    return res_line + headers + body;
}