#ifndef SIMPLEHTTPLIB_HTTPLIB_INTERNAL_H
#define SIMPLEHTTPLIB_HTTPLIB_INTERNAL_H

#include <stddef.h>
#include "httplib.h"

struct tmp {
    char * buf;
    size_t size;
    size_t pos;
};

enum req_pos {
    METHOD,
    PATH,
    QUERY,
    VERSION,
    HEADERS,
    BODY
};

void httplib_request_free(http_request * self);
void httplib_response_free(http_response * self);
void httplib_headers_free(http_header * self);

http_header * httplib_header_new(char * key, char * value);
http_request * httplib_request_new();
http_response * httplib_response_new();

size_t httplib_header_string_size(http_header * header);
int httplib_header_serialize(http_header * header, char * str_out);

size_t httplib_response_string_size(http_response * res);
int httplib_response_serialize(http_response * res, char * str_out);

int httplib_parse_buffer(char * buf, size_t bytes, http_request * req, struct tmp * tmp, enum req_pos * pos, int * was_cr);
int httplib_parse_method(char * method_str, http_method * method_out);
int httplib_parse_header(http_request * self, char * str);

void httplib_tmp_increase(struct tmp * tmp);
void httplib_tmp_reset(struct tmp * tmp);

int httplib_receive(int cfd, http_request * req_out);
void httplib_respond(int cfd, http_response * res);

#endif //SIMPLEHTTPLIB_HTTPLIB_INTERNAL_H
