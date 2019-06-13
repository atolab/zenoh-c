#include "zenoh/rname.h"
#include <string.h>

#define CEND(str) (str[0] == 0 || str[0] == '/')
#define CWILD(str) (str[0] == '*')
#define CNEXT(str) str+1
#define CEQUAL(str1, str2) str1[0] == str2[0]

#define END(str) (str[0] == 0)
#define WILD(str) (str[0] == '*' && str[1] == '*' && (str[2] == '/' || str[2] == 0))
#define NEXT(str) next_chunk(str)

#define DEFINE_INTERSECT(name, end, wild, next, elem_intersect) \
int name(char *c1, char *c2){ \
    if(end(c1)  && end(c2))  return 1; \
    if(wild(c1) && end(c2))  return name(next(c1), c2); \
    if(end(c1)  && wild(c2)) return name(c1, next(c2)); \
    if(wild(c1) || wild(c2)) { \
        if(name(next(c1), c2)) return 1; \
        else return name(c1, next(c2)); \
    } \
    if(end(c1)  || end(c2))  return 0; \
    if(elem_intersect(c1, c2)) return name(next(c1), next(c2)); \
    return 0; \
}

DEFINE_INTERSECT(sub_chunk_intersect, CEND, CWILD, CNEXT, CEQUAL)

int chunk_intersect(char *c1, char *c2){
    if((CEND(c1) && !CEND(c2)) || (!CEND(c1) && CEND(c2))) return 0;
    return sub_chunk_intersect(c1, c2);
}

char *next(char *str){
    char *res = strchr(str, '/');
    if(res != NULL) return res + 1;
    return strchr(str, 0);
}

DEFINE_INTERSECT(intersect, END, WILD, next, chunk_intersect)