#ifndef _MY_STRING_H_
#define _MY_STRING_H_

#include <stdlib.h>
#include <string.h>

typedef struct string string;

string *string_new();
string *string_init(string *s);
void string_free(string *s);

char *string_cstr(string *s);

string *string_ncat(string *s1, const char *s2, size_t n);
string *string_cat(string *s1, const char *s2);

size_t string_length(string *s);
int string_empty(string *s);

#endif
