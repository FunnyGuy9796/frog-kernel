#!/usr/bin/env python3
import subprocess
import sys

elf = sys.argv[1]
nm  = "/opt/cross/bin/i686-elf-nm"

result = subprocess.run([nm, "--numeric-sort", elf], capture_output=True, text=True)

syms = []

for line in result.stdout.splitlines():
    parts = line.split()

    if len(parts) != 3:
        continue

    addr, kind, name = parts

    if kind not in ("T", "t"):
        continue

    syms.append((int(addr, 16), name))

print("#include <stdint.h>")
print("")
print("typedef struct { uint32_t addr; const char *name; } ksym_entry_t;")
print("")
print("ksym_entry_t ksym_entries[] = {")
for addr, name in syms:
    print(f'    {{ 0x{addr:08x}u, "{name}" }},')
print("    { 0, 0 }")
print("};")
print("")
print(f"const uint32_t ksym_entry_count = {len(syms)}u;")