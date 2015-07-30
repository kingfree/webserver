#include "string.h"

#define BUFFSIZE 128

struct string {
    char *data;
    size_t len;
    size_t size;
};

string *string_new()
{
    string *s = malloc(sizeof(string));
    return string_init(s);
}

string *string_init(string *s)
{
    if (!s) return NULL;
    s->len = 0;
    s->size = 0;
    s->data = NULL;
    return s;
}

char *string_cstr(string *s)
{
    if (s) return s->data;
    return NULL;
}

size_t string_length(string *s)
{
    if (!s) return 0;
    return s->len;
}

int string_empty(string *s)
{
    return string_length(s) == 0;
}

string *string_ncat(string *s1, const char *s2, size_t n)
{
    if (s1->len + n + 1 >= s1->size) {
        s1->size += n + 1 > BUFFSIZE ? n + 1 : BUFFSIZE;
        if (s1->data == NULL) {
            s1->data = malloc(s1->size);
        } else {
            s1->data = realloc(s1->data, s1->size);
        }
        if (s1->data == NULL) return NULL;
    }
    // char *res = strncat(s1->data + s1->len, s2, n);
    char *p = s1->data + s1->len;
    for (size_t i = 0; i < n; i++) *p++ = s2[i];
    // printf("%ld %ld %s %ld\n", s1->len, s1->size, s1->data, n);
    s1->len += n;
    s1->data[s1->len] = '\0';  // strncat() 会自动加 '\0'
    return s1;
    // return res == s1->data ? s1 : NULL;
}

string *string_cat(string *s1, const char *s2)
{
    return string_ncat(s1, s2, strlen(s2));
}

void string_free(string *s)
{
    if (s->data) free(s->data);
    s->data = NULL;
    s->len = s->size = 0;
}
