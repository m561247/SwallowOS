#ifndef _DEFS_H
#define _DEFS_H

#define offset_of(type, member) \
    (uint64_t)&(((type *)0)->member)

#endif