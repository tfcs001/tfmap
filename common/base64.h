#ifndef _BASE64_H_
#define _BASE64_H_

int base64_encode(const char *indata, int inlen, char *outdata, int *outlen);

int base64_decode(const char *indata, int inlen, char *outdata, int *outlen);

#endif

