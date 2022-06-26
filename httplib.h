#ifndef SIMPLEHTTPLIB_HTTPLIB_H
#define SIMPLEHTTPLIB_HTTPLIB_H

#include <stddef.h>

/**
 * Represents a HTTP request method.
 */
typedef enum http_method_enum {
    GET, POST, PUT, DELETE, PATCH, HEAD, CONNECT, OPTIONS, TRACE
} http_method;

/**
 * Represents a HTTP header. Contains the header field key, the value and a pointer
 * to the next header to form a linked list.
 * All pointers of this struct must point to allocated memory or 0.
 */
typedef struct http_header_struct {
    char * key;
    char * value;
    struct http_header_struct * next;
} http_header;

/**
 * Represents a HTTP request. Contains the request method, the HTTP path, a linked list
 * of headers, the query string (optional, without leading '?'), the request body (optional)
 * and the content length of the body (0 if there is no body).
 * All pointers of this struct must point to allocated memory or 0.
 */
typedef struct http_request_struct {
    http_method method;
    char * path;
    http_header * headers;
    char * query;
    size_t content_length;
    char * body;
} http_request;

/**
 * Represents a HTTP response. Contains the HTTP status code, the status message,
 * a linked list of headers, the response body (optional) and the content length
 * of the body (0 if there is no body).
 * All pointers of this struct must point to allocated memory or 0.
 */
typedef struct http_response_struct {
    int status_code;
    char * status_message;
    http_header * headers;
    size_t content_length;
    char * body;
} http_response;

/**
 * Function pointer type for the HTTP request handler function.
 */
typedef void (*http_request_handler) (http_request * req, http_response * res_out);

/**
 * Used to start the HTTP server.
 * @param port the port on which the server should listen
 * @param handler a function pointer to the request handler function
 * @return a negative integer, if the server exited with an error
 */
int httplib_start_server(int port, http_request_handler handler);

/**
 * Helper function to add a header to a HTTP response.
 * @param self a pointer to the response struct
 * @param key the header field key
 * @param value the header value
 * @return 0 if the header was set, a negative value if the header was not valid
 */
int httplib_response_add_header(http_response * self, char * key, char * value);

/**
 * Helper function to set the content length and to allocate the memory
 * for the body of a response struct
 * @param self a pointer to the response struct
 * @param length the content length
 */
void httplib_response_set_content_length(http_response * self, size_t length);

/**
 * Helper function to set the response status of a response struct
 * @param self a pointer to the response struct
 * @param code the status code
 * @param message the status message
 */
void httplib_response_set_status(http_response * self, int code, char * message);

/**
 * Helper function to get a header value from a HTTP.
 * Returned value is allocated memory and needs to be freed, if not null.
 * @param self a pointer to the request struct
 * @param key the header field key
 * @return the value of the header or null, if the header does not exist in the request
 */
char * httplib_request_get_header(http_request * self, char * key);

/**
 * Helper function to get a query value from a HTTP request.
 * Returned value is allocated memory and needs to be freed, if not null.
 * @param self a pointer to the request struct
 * @param key the query key
 * @return the value of the query or null, if the query key does not exist in the request
 */
char * httplib_request_get_query_value(http_request * self, char * key);

/**
 * Helper function to check if a header is valid.
 * @param key the header field key
 * @param value the header value
 * @return true if the header is valid, otherwise false
 */
int httplib_is_valid_header(const char * key, const char * value);

/**
 * Helper function to decode a percent encoded character.
 * @param encoded a pointer to the first of the two characters in the string
 * after the percent symbol, which make up the encoded character
 * @return the decoded character
 */
char httplib_decode_hex(const char * encoded);

#define SOCKET_ERROR -1
#define BIND_ERROR -2
#define LISTEN_ERROR -3

#endif //SIMPLEHTTPLIB_HTTPLIB_H