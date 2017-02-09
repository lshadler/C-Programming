#ifndef CHECK_INPUT
#define CHECK_INPUT

///
/// @param:  fp      - file pointer source to select
/// @param:  buf     - buffer to hold data
/// @param:  len     - maximum number of bytes to read
/// @param:  timeout - timeout in seconds
/// @return: -1 error, 0 timeout (no input read), 1 (input was read)
/// @pre: timeout != 0
/// @pre: buffer != NULL and size of buffer >= len + 1
///
int check_input( FILE * fp, char * buf, int len, int timeout ) ;

#endif // CHECK_INPUT
