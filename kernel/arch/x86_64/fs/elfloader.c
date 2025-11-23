#include <stdbool.h>
#include "fat.h"
#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <kernel/string.h>
#include "elf.h"
#include "elfloader.h"

static void read_elf(const char *elf_content, unsigned int len) {
    if (len == 0) {
        printk("Invalid elf content\n");
        return;
    }

    printk("Read ELF\n");

    /* check elf magic code */
    Elf64_Ehdr *elf_hdr = (Elf64_Ehdr* )elf_content;
    printk("    ELF Header: %s\n", &(elf_hdr->e_ident[0]));

    /* elf type */
    char *type = "NONE";
    switch (elf_hdr->e_type) {
        case ET_EXEC:
            type = "EXEC";
            break;
        case ET_DYN:
            type = "EYN";
            break;
        case ET_REL:
            type = "REL";
            break;
        case ET_NONE:
            type = "NONE";
            break;
        default:
            type = "OTHER";
            break;
    }
    printk("    ELF type: %s\n", type);

    /* elf machine */
    char *machine = "NONE";
    switch (elf_hdr->e_machine) {
        case EM_X86_64:
            machine = "x86_64";
            break;
        case EM_NONE:
            machine = "NONE";
            break;
        default:
            machine = "OTHER";
            break;
    }
    printk("    ELF machine: %s\n", machine);

    /* elf version */
    printk("    ELF version: %d\n", (elf_hdr->e_version & 0b11));

    /* elf program header */
    printk("    ELF header:\n");
    Elf64_Phdr *p_hdr = elf_content + elf_hdr->e_phoff;
    for (unsigned int i=0; i<elf_hdr->e_phnum; ++i) {
        /* program header type */
        char *p_type = "NULL";
        switch (p_hdr[i].p_type) {
            case PT_LOAD:
                p_type = "LOAD   ";
                break;
            case PT_DYNAMIC:
                p_type = "DYNAMIC";
                break;
            case PT_INTERP:
                p_type = "INTERP ";
                break;
            case PT_NOTE:
                p_type = "NOTE   ";
                break;
            case PT_PHDR:
                p_type = "PHDR   ";
                break;
            case PT_NUM:
                p_type = "NUM    ";
                break;
            case PT_NULL:
                p_type = "NULL   ";
                break;
            default:
                p_type = "OTHER  ";
                break;
        }
        printk("     Type: %s", p_type);

        /* program header flags */
        char p_flags[4] = {'_', '_', '_', '\0'};
        if (p_hdr[i].p_flags & PF_X)
            p_flags[0] = 'X';
        if (p_hdr[i].p_flags & PF_W)
            p_flags[1] = 'W';
        if (p_hdr[i].p_flags & PF_R)
            p_flags[2] = 'R';
        printk("  Flags: %s", &(p_flags[0]));

        /* program header alignment */
        printk("  Align: %u", p_hdr[i].p_align);

        /* program header offset */
        printk("  Offset: %x\n", p_hdr[i].p_offset);

        /* program header vaddr */
        printk("      Vaddr: %x", p_hdr[i].p_vaddr);

        /* program header paddr */
        printk("  Paddr: %x", p_hdr[i].p_paddr);

        /* program header file size */
        printk("  Filesz: %x", p_hdr[i].p_filesz);

        /* program header memory size */
        printk("  Memsz: %x", p_hdr[i].p_memsz);

        printk("\n");
    }

    return;
}

int load_elf(fat12_t *fs, const char *name) {
    uint8_t sec[BYTES_PER_SECTOR] = {0};
    int64_t size = fat12_get_file_size(fs, name);
    if (size <= 0) {
        printk("[Error] Failed to get file size of %s\n", name);
        return -1;
    }
    printk("Get elf size: %u\n", size);

    uint8_t *file_content = (uint8_t *)kmalloc(size + 1);
    if (!file_content) {
        printk("[Error] Failed to alloc file buffer of %s\n", name);
        return -1;
    }
    memset(file_content, '\0', size);

    unsigned int outlen;
    if (!fat12_read_file(fs, name, file_content, size, &outlen)) {
        printk("[Error] Failed to read file %s\n", name);
        kfree(file_content);
        return -1;
    }

    read_elf(file_content, outlen);
    kfree(file_content);

    return 0;
}

