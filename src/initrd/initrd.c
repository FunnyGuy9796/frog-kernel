#include "initrd.h"

initrd_t initrd;

void initrd_init(void *mb2_info) {
    mb2_tag_t *tag = (mb2_tag_t *)((uint8_t *)mb2_info + 8);
    mb2_tag_module_t *module_tag = 0;

    while (tag->type != MB2_TAG_TYPE_END) {
        if (tag->type == MB2_TAG_TYPE_MODULE) {
            module_tag = (mb2_tag_module_t *)tag;

            break;
        }

        tag = (mb2_tag_t *)(((uint32_t)tag + tag->size + 7) & ~7);
    }

    if (!module_tag)
        panic(NULL, "initrd_init() -> no module tag found in multiboot2 info");
    
    initrd.phys_addr = module_tag->mod_start;
    initrd.size = module_tag->mod_end - module_tag->mod_start;
}

void initrd_remap() {
    uint32_t initrd_pages = (initrd.size + PAGE_SIZE - 1) / PAGE_SIZE;

    initrd.addr = (uint32_t)(uintptr_t)kalloc_pages(initrd_pages);

    memcpy((void *)initrd.addr, (void *)(uintptr_t)initrd.phys_addr, initrd.size);
}

initrd_file_t initrd_lookup(const char *filepath) {
    initrd_file_t result = { NULL, NULL, 0, 0 };
    uint8_t *ptr = (uint8_t *)(uintptr_t)initrd.addr;
    uint8_t *end = ptr + initrd.size;

    while (ptr + 512 <= end) {
        ustar_header_t *hdr = (ustar_header_t *)ptr;

        if (hdr->name[0] == '\0')
            break;
        
        if (memcmp(hdr->magic, "ustar", 5) != 0)
            break;
        
        char fullpath[256];

        if (hdr->prefix[0] != '\0') {
            size_t plen = strnlen(hdr->prefix, 155);
            size_t nlen = strnlen(hdr->name, 100);

            memcpy(fullpath, hdr->prefix, plen);

            fullpath[plen] = '/';

            memcpy(fullpath + plen + 1, hdr->name, nlen);

            fullpath[plen + 1 + nlen] = '\0';

            size_t total = plen + 1 + nlen;

            if (total > 1 && fullpath[total - 1] == '/')
                fullpath[total - 1] = '\0';
        } else {
            strncpy(fullpath, hdr->name, 255);

            fullpath[255] = '\0';

            size_t nlen = strnlen(fullpath, 255);

            if (nlen > 1 && fullpath[nlen - 1] == '/')
                fullpath[nlen - 1] = '\0';
        }

        uint32_t filesize = oct2int(hdr->size, 12);
        uint8_t *data = ptr + 512;

        if (strcmp(fullpath, filepath) == 0) {
            result.name = hdr->name;
            result.data = (char *)data;
            result.size = (size_t)filesize;
            result.typeflag = hdr->typeflag;

            return result;
        }

        ptr = data + ((filesize + 511) & ~511);
    }

    return result;
}