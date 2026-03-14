#ifndef FDTABLE_H
#define FDTABLE_H

#define MAX_FDS 256

struct file;

typedef struct fd_table {
    struct file *entries[MAX_FDS];
} fd_table_t;

#endif