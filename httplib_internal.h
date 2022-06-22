#ifndef SIMPLEHTTPLIB_HTTPLIB_INTERNAL_H
#define SIMPLEHTTPLIB_HTTPLIB_INTERNAL_H

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

void free_request(http_request * self);
void free_response(http_response * self);

http_header * new_header(char * key, char * value);
http_request * new_request();
http_response * new_response();

size_t header_string_size(http_header * header);
int header_to_string(http_header * header, char * str_out);

size_t response_size(http_response * res);
int serialize_response(http_response * res, char * str_out);

int parse_buffer(char * buf, size_t bytes, http_request * req, struct tmp * tmp, enum req_pos * pos, int * was_cr);
int parse_method(char * method_str, http_method * method_out);
int parse_header(http_request * self, char * str);

void increase_tmp(struct tmp * tmp);
void reset_tmp(struct tmp * tmp);

int receive(int cfd, http_request * req_out);
void respond(int cfd, http_response * res);

#endif //SIMPLEHTTPLIB_HTTPLIB_INTERNAL_H
