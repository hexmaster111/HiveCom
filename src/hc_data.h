// HEADER FILE
#ifndef HC_DATA_H
#define HC_DATA_H

#include <stdint.h>
#include <string.h>

// This layout matches clays layout.
typedef struct hc_slice
{
    bool isstatic;
    int32_t len;
    char *base;
} hc_slice;

hc_slice hc_slice_from(const char *cstr); // does not copy
int hc_slice_cmp(hc_slice a, hc_slice b); // 0 == same string
#define HC_SLICE_FROM_CLIT(CLIT) (hc_slice){.base = (CLIT), .len = sizeof((CLIT)) - 1}

typedef struct hc_MsgAuther
{
    hc_slice name;
} hc_MsgAuther;

typedef struct hc_MsgContent
{
    hc_slice text;
} hc_MsgContent;

typedef struct hc_MsgTree
{
    struct hc_MsgTree *right, *down;
    struct hc_MsgAuther auther;
    struct hc_MsgContent content;
} hc_MsgTree;

#endif // HC_DATA_H

// C FILE
#ifdef HC_DATA_IMPL
#undef HC_DATA_IMPL

hc_slice hc_slice_from(const char *cstr)
{
    hc_slice ret = {};
    ret.base = (char *)cstr;
    ret.len = strlen(cstr);
    return ret;
}

int hc_slice_cmp(hc_slice a, hc_slice b)
{
    if (a.len != b.len)
        return a.len - b.len;

    int diff = 0;

    for (int32_t i = 0; i < a.len; i++)
    {
        if (a.base[i] != b.base[i])
            diff += 1;
    }

    return diff;
}

#endif // HC_DATA_IMPL