#ifndef _ELFLOADER_H
#define _ELFLOADER_H

#include "fat.h"

int load_elf(fat12_t *fs, const char *name);

#endif