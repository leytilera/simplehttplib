#include <string.h>
#include <stdlib.h>
#include "httplib.h"
#include "httplib_internal.h"
#include <stdio.h>

http_header * httplib_header_new(char * key, char * value) {
    http_header * header = malloc(sizeof(http_header));
    header->next = 0;
    header->key = malloc(strlen(key)+1);
    header->value = malloc(strlen(value)+1);
    strcpy(header->key, key);
    strcpy(header->value, value);
    return header;
}

http_request * httplib_request_new() {
    http_request * req = malloc(sizeof(http_request));
    req->method = GET;
    req->headers = 0;
    req->path = 0;
    req->query = 0;
    req->content_length = 0;
    req->body = 0;
    return req;
}

http_response * httplib_response_new() {
    http_response * res = malloc(sizeof(http_request));
    res->status_message = 0;
    res->headers = 0;
    res->body = 0;
    res->content_length = 0;
    res->status_code = 100;
    return res;
}

int httplib_response_add_header(http_response * self, char * key, char * value) {
    if (!httplib_is_valid_header(key, value)) return -1;
    if (self->headers == 0) {
        self->headers = httplib_header_new(key, value);
    } else {
        http_header * run = self->headers;
        while (run) {
            if (run->next == 0) {
                run->next = httplib_header_new(key, value);
                break;
            }
            run = run->next;
        }
    }
    return 0;
}

char * httplib_request_get_header(http_request * self, char * key) {
    http_header * run = self->headers;
    while (run) {
        if (!strcasecmp(key, run->key)) {
            char * value = malloc(strlen(run->value)+1);
            strcpy(value, run->value);
            return value;
        }
        run = run->next;
    }
    return NULL;
}

int httplib_is_valid_header(const char * key, const char * value) {
    for (int i = 0; key[i] != '\0'; i++) {
        char c = key[i];
        if ((c < 65 && c != '-') || (c > 90 && c < 97 && c != '_') || c > 122) {
            return 0;
        }
    }
    for (int i = 0; value[i] != '\0'; i++) {
        char c = value[i];
        if (c < 32 || c > 126) {
            return 0;
        }
    }
    return 1;
}

void httplib_request_free(http_request * self) {
    if (self->headers != 0) {
        httplib_headers_free(self->headers);
    }
    if (self->query != 0) free(self->query);
    if (self->body != 0) free(self->body);
    if (self->path != 0) free(self->path);
    free(self);
}

void httplib_response_free(http_response * self) {
    if (self->headers != 0) {
        httplib_headers_free(self->headers);
    }
    if (self->status_message != 0) free(self->status_message);
    if (self->body != 0) free(self->body);
    free(self);
}

void httplib_headers_free(http_header * self) {
    http_header * run = self;
    while (run) {
        http_header * to_free = run;
        run = run->next;
        if (to_free->key != 0) free(to_free->key);
        if (to_free->value != 0) free(to_free->value);
        free(to_free);
    }
}

char * httplib_request_get_query_value(http_request * self, char * key) {
    if (self->query == 0) return NULL;
    char * tmp = malloc(strlen(self->query)+1);
    int pos = 0;
    int value_start = -1;
    for (int i = 0; self->query[i] != '\0'; i++) {
        if (self->query[i] == '=') {
            tmp[pos] = '\0';
            if (!strcmp(key, tmp)) {
                value_start = i+1;
            }
            pos = 0;
        } else if (self->query[i] == '&') {
            if (value_start > 0) {
                break;
            } else {
                pos = 0;
            }
        } else {
            tmp[pos] = self->query[i];
            pos++;
        }
    }
    tmp[pos] = '\0';

    if (value_start > 0) {
        char * value = malloc(strlen(tmp)+1);
        strcpy(value, tmp);
        free(tmp);
        return value;
    }
    free(tmp);
    return NULL;
}

void httplib_response_set_content_length(http_response * self, size_t length) {
    self->content_length = length;
    if (length > 0)
        self->body = malloc(length);
}

void httplib_response_set_status(http_response * self, int code, char * message) {
    self->status_code = code;
    self->status_message = malloc(strlen(message)+1);
    strcpy(self->status_message, message);
}

void httplib_tmp_increase(struct tmp * tmp) {
    tmp->size += 1024;
    tmp->buf = realloc(tmp->buf, tmp->size);
    bzero(tmp->buf + tmp->pos, 1024);
}

void httplib_tmp_reset(struct tmp * tmp) {
    tmp->pos = 0;
    bzero(tmp->buf, tmp->size);
}