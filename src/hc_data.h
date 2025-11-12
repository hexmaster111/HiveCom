// HEADER FILE
#ifndef HC_DATA_H
#define HC_DATA_H

typedef struct hc_slice
{
    char *base;
    int len;
} hc_slice;

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

#endif //HC_DATA_H


// C FILE
#ifdef HC_DATA_IMPL
#undef HC_DATA_IMPL



#endif //HC_DATA_IMPL