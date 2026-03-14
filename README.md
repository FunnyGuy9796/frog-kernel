# The Frog Kernel

The frog kernel is an i686 OS that aims to be simple and user-friendly. It doesn't yet have a purpose and may tend to jump around between many different ideas but it promises to stay true to the following:
- A monolithic architecture
- "Everything is a file"
- Any and all components are free to use and modify

As of now the project is very early in development and hardly does anything more than demonstrate simple features.  
  
## Roadmap

- [x] A higher-half kernel mapped to 0xc0000000
- [x] Both physical and virtual memory management with a kernel heap
- [x] Interrupts including PS/2 keyboard and mouse as well as the PIT
- [x] Kernel symbol resolution for stack traces and debugging
- [x] Kernel multithreading
- [x] An initrd with a basic ELF loader
- [x] A VFS with an easy way to attach new drivers
- [x] A framebuffer with text output
- [x] A mounted device filesystem to expose devices as files such as `/dev/fb0/` and `/dev/tty0/`
- [x] A small set of syscalls for userspace to spawn programs and a small custom libc to take advantage of those syscalls
- [x] Userspace processes with PIDs
- [ ] More syscalls
- [ ] Userspace memory management via a heap
- [ ] Functional userspace shell
- [ ] IPC via "files" in `/ipc/`
- [ ] DOOM
- [ ] A simple GUI  

## Goal

The goal of this project is to have fun while developing a system that others can use and enjoy. While it is very unlikely that this OS will ever go beyond being a fun hobby of mine I hope it is able to become something genuinely usable.  

## License

The entire project, including the kernel as well as anything within userspace, is licensed under [GPL v3](LICENSE)
