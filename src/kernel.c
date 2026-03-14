#include "multiboot.h"
#include "misc/util.h"
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "devices/rtc/rtc.h"
#include "devices/pit/pit.h"
#include "devices/keyboard/keyboard.h"
#include "devices/mouse/mouse.h"
#include "devices/null/null.h"
#include "devices/zero/zero.h"
#include "devices/fb/fb.h"
#include "devices/tty/tty.h"
#include "memory/mmap.h"
#include "memory/pmm.h"
#include "memory/kheap.h"
#include "initrd/initrd.h"
#include "initrd/initrdfs.h"
#include "vfs/vfs.h"
#include "devfs/devfs.h"
#include "ksym/ksym.h"
#include "misc/printf.h"
#include "misc/panic.h"
#include "framebuffer/framebuffer.h"
#include "framebuffer/console.h"
#include "thread/kthread.h"
#include "userspace/uprocess.h"
#include "userspace/usyscall.h"

static void remove_identity_map() {
    uint32_t *page_directory = (uint32_t *)PAGE_DIR_VIRT;

    page_directory[0] = 0;

    uint32_t cr3;

    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    __asm__ volatile ("mov %0, %%cr3" :: "r"(cr3));
}

static void idle_entry() {
    while (1)
        __asm__ volatile ("hlt");
}

static void main_entry() {
    serial_printf("boot time: %d ms\n", pit_get_ms());

    while (1) {
        
    }
}

void kmain(uint32_t mb2_magic, uint32_t *mb2_info) {
    if (mb2_magic != MB2_MAGIC)
        panic(NULL, "main() -> invalid multiboot magic\n  magic: 0x%x\n", mb2_magic);

    serial_printf("\nmagic: 0x%x\nmb2_info addr: 0x%08x\n", mb2_magic, mb2_info);

    gdt_init();
    serial_printf("gdt initialized\n");

    idt_init();
    serial_printf("idt initialized\n");

    rtc_init();
    rtc_read_time(&curr_time);
    serial_printf("rtc initialized\n  boot time: %02d/%02d/%02d - %02d:%02d:%02d\n",
        boot_time.month, boot_time.day, boot_time.year, boot_time.hours, boot_time.minutes, boot_time.seconds);

    pit_init();
    serial_printf("pit initialized to %d Hz\n", PIT_HZ);

    mmap_init(mb2_info);
    serial_printf("mmap initialized\n  usable regions: %d\n", usable_region_count);

    uint32_t usable_memory = 0;

    for (size_t i = 0; i < usable_region_count; i++)
        usable_memory += usable_regions[i].length;
    
    serial_printf("  available memory: %.2f MiB\n", (double)usable_memory / 1048576);
    serial_printf("  kernel start: 0x%08x\n  kernel end: 0x%08x\n  kernel size: %d bytes\n", kstart, kend, ksize);

    initrd_init(mb2_info);
    serial_printf("initrd initialized\n  phys addr: 0x%08x\n  size: %u bytes\n", initrd.phys_addr, initrd.size);

    fb_parse_tag(mb2_info);

    pmm_init(fb_tag);
    serial_printf("pmm initialized\n  bitmap addr: 0x%08x\n  bitmap size: %u bytes\n", bitmap_addr, bitmap_size);

    paging_init();
    serial_printf("paging initialized\n");

    ksym_init();
    serial_printf("ksym initialized\n");

    kheap_init();
    serial_printf("kheap initialized\n  addr: 0x%08x\n  size: %u bytes\n  head addr: 0x%08x\n  end: 0x%08x\n", kheap_start, kheap_size, head, kheap_end);

    fb_init();
    serial_printf("fb initialized\n  pitch: %d\n  width: %d\n  height: %d\n  bpp: %d\n  addr: 0x%08x\n", fb.pitch, fb.width, fb.height, fb.bpp, fb.addr);

    initrd_remap();
    serial_printf("initrd remapped\n  addr: 0x%08x\n", initrd.addr);

    remove_identity_map();
    serial_printf("identity mapping removed\n");

    if (vfs_mount("/", initrd_fs_get_driver(), NULL) != 0)
        panic(NULL, "main() -> failed to mount '/'");

    serial_printf("mounted /\n");

    if (vfs_mount("/dev", devfs_get_driver(), NULL) != 0)
        panic(NULL, "main() -> failed to mount '/dev'");
    
    serial_printf("mounted /dev\n");

    null_init();
    serial_printf("null initialized\n");

    zero_init();
    serial_printf("zero initialized\n");

    mouse_init();
    serial_printf("mouse initialized\n");

    keyboard_init();
    serial_printf("keyboard initialized\n");

    userspace_init();
    serial_printf("userspace syscalls initialized\n");

    fb_register(fb.phys_addr, fb.addr, fb.width, fb.height, fb.pitch, fb.bpp);
    serial_printf("fb0 initialized\n");

    console_init();

    tty_register(console_cols, console_rows, console_bg, console_fg);
    serial_printf("tty0 initialized\n");

    idle_thread = kthread_create(idle_entry, KSTACK_SIZE);
    idle_thread->state = THREAD_READY;

    thread_t *main_thread = kthread_create(main_entry, KSTACK_SIZE);

    main_thread->state = THREAD_READY;

    struct stat st;
    int init_fd = vfs_open("/bin/init/init", 0);
    
    vfs_stat(init_fd, &st);
    
    uint8_t *buf = kmalloc(st.st_size);

    if (!buf)
        panic(NULL, "main() -> failed to allocate buffer for /bin/init/init");

    vfs_read(init_fd, buf, st.st_size);
    vfs_close(init_fd);

    int argc = 1;
    char **argv = kmalloc(argc * sizeof(char *));

    argv[0] = kmalloc(20 * sizeof(char));

    strcpy(argv[0], "/bin/init/init");

    uint32_t heap_start;
    uint32_t initial_esp;
    uint32_t *pd_phys = uprocess_create_pagedir();
    uint32_t entry = uprocess_load(pd_phys, buf, st.st_size, &heap_start, &initial_esp, argc, argv);
    process_t *init = uprocess_create(entry, pd_phys, heap_start, initial_esp);

    kfree(argv[0]);
    kfree(argv);
    kfree(buf);

    scheduler_add(idle_thread);
    scheduler_add(main_thread);
    scheduler_add(init->thread);

    curr_thread = main_thread;
    curr_thread->state = THREAD_RUNNING;

    tss_set_kernel_stack((uint32_t)curr_thread->stack_base + curr_thread->stack_size);

    __asm__ volatile (
        "mov %0, %%esp\n"
        "pop %%gs\n"
        "pop %%fs\n"
        "pop %%es\n"
        "pop %%ds\n"
        "popa\n"
        "add $8, %%esp\n"
        "iret\n"
        :
        : "r"(curr_thread->esp)
        : "memory"
    );
}