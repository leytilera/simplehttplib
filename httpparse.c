#include <string.h>
#include <stdlib.h>
#include "httplib.h"
#include "httplib_internal.h"

int parse_buffer(char * buf, size_t bytes, http_request * req, struct tmp * tmp, enum req_pos * pos, int * was_cr) {
    for (int i = 0; i < bytes; i++) {
        if (buf[i] == '\r') {
            *was_cr = 1;
            continue;
        }

        if (*pos == METHOD && buf[i] == ' ') {
            int res = parse_method(tmp->buf, &req->method);
            if (res < 0) {
                return 4;
            } else {
                *pos = PATH;
                reset_tmp(tmp);
            }
        } else if (*pos == QUERY && buf[i] == ' ') {
            req->query = malloc(strlen(tmp->buf) + 1);
            strcpy(req->query, tmp->buf);
            *pos = VERSION;
            reset_tmp(tmp);
            //TODO check if valid
        } else if (*pos == PATH && buf[i] == ' ') {
            *pos = VERSION;
            req->path = malloc(strlen(tmp->buf) + 1);
            strcpy(req->path, tmp->buf);
            reset_tmp(tmp);
        } else if (*pos == VERSION && *was_cr && buf[i] == '\n') {
            *pos = HEADERS;
            if (strcmp(tmp->buf, "HTTP/1.1") != 0) {
                return 3;
            }
            reset_tmp(tmp);
        } else if (*pos == PATH && buf[i] == '?') {
            *pos = QUERY;
            req->path = malloc(strlen(tmp->buf) + 1);
            strcpy(req->path, tmp->buf);
            reset_tmp(tmp);
        } else if (*pos == HEADERS && *was_cr && buf[i] == '\n') {
            int res = parse_header(req, tmp->buf);
            if (res < 0) {
                return 4;
            } else if (res > 0) {
                *pos = BODY;
                if (req->content_length > 0) {
                    req->body = malloc(req->content_length);
                }
            }
            reset_tmp(tmp);
        } else if (*pos != BODY) {
            if (tmp->pos == tmp->size - 1) {
                increase_tmp(tmp);
            }
            tmp->buf[tmp->pos] = buf[i];
            tmp->pos++;
        } else if (req->content_length > tmp->pos) {
            req->body[tmp->pos] = buf[i];
            tmp->pos++;
        } else {
            //END OF BODY
            return 1;
        }

        *was_cr = 0;
    }

    if (*pos == BODY && req->content_length <= tmp->pos) {
        return 1;
    }

    return 0;
}

int parse_method(char * method_str, http_method * method_out) {
    if (!strcmp(method_str, "GET")) {
        *method_out = GET;
    } else if (!strcmp(method_str, "POST")) {
        *method_out = POST;
    } else if (!strcmp(method_str, "PUT")) {
        *method_out = PUT;
    } else if (!strcmp(method_str, "DELETE")) {
        *method_out = DELETE;
    } else if (!strcmp(method_str, "PATCH")) {
        *method_out = PATCH;
    } else if (!strcmp(method_str, "HEAD")) {
        *method_out = HEAD;
    } else if (!strcmp(method_str, "CONNECT")) {
        *method_out = CONNECT;
    } else if (!strcmp(method_str, "OPTIONS")) {
        *method_out = OPTIONS;
    } else if (!strcmp(method_str, "TRACE")) {
        *method_out = TRACE;
    } else {
        return -1;
    }
    return 0;
}

int parse_header(http_request * self, char * str) {
    size_t len = strlen(str);
    if (len == 0) {
        return 1;
    }

    char * key = malloc(len);
    bzero(key, len);
    char * value = malloc(len);
    bzero(value, len);

    int key_end = 0;
    int value_start = 0;
    char c;
    int i = 0;
    int pos = 0;
    while ((c = str[i]) != '\0') {
        if (!key_end) {
            if (c == ':') {
                key_end = 1;
                pos = 0;
            } else {
                key[pos] = c;
                pos++;
            }
        } else if (!value_start && c != ' ') {
            value_start = 1;
        }
        if (value_start) {
            value[pos] = c;
            pos++;
        }
        i++;
    }
    for (int j = pos - 1; j >= 0 ; j--) {
        if (value[j] == ' ') {
            value[j] = '\0';
        } else {
            break;
        }
    }

    if (strcasecmp("Content-Length", key) == 0) {
        self->content_length = strtoul(value, 0, 10);
        return 0;
    } else if (is_valid_header(key, value)) {
        if (self->headers == 0) {
            self->headers = new_header(key, value);
        } else {
            http_header * run = self->headers;
            while (run != 0) {
                if (run->next == 0) {
                    run->next = new_header(key, value);
                    break;
                }
                run = run->next;
            }
        }
        free(key);
        free(value);
        return 0;
    }
    free(key);
    free(value);
    return -1;
}