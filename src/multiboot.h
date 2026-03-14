#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>
#include "ksym/elf.h"

#define MB2_MAGIC 0x36d76289

#define MB2_TAG_TYPE_END 0
#define MB2_TAG_TYPE_MODULE 3
#define MB2_TAG_TYPE_MMAP 6
#define MB2_TAG_TYPE_FRAMEBUFFER 8
#define MB2_TAG_TYPE_ELF_SECTIONS 9

typedef struct {
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) mb2_tag_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} __attribute__((packed)) mb2_tag_mmap_t;

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed)) mb2_mmap_entry_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
} __attribute__((packed)) mb2_tag_framebuffer_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t num;
    uint32_t entsize;
    uint32_t shndx;
    Elf32_Shdr sections[];
} __attribute__((packed)) mb2_tag_elf_sections_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char cmdline[0];
} __attribute__((packed)) mb2_tag_module_t;

#endif