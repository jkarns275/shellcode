from sys import stdin

lines = []
import re

sym_pattern = re.compile(r"^[0-9a-fA-F]+ <(?P<name>[\w@\-\.]+)> \(File Offset: (?P<offset>0x[0-9a-fA-F]+)\):")

lines = []

for line in stdin:
    lines.append(line)

m = {}
plt_slots = {}
for i in range(0, len(lines) // 2):
    symbol = lines[i*2]
    plt_location = lines[i*2+1]

    mat = sym_pattern.match(symbol)
    if not mat:
        continue

    if "jmp" not in plt_location:
        offset = mat.group("offset")
        offset = int(offset, 16)
    else:
        split = plt_location.split("#")
        if len(split) < 2:
            continue
    
        plt_slot_offset = split[1].split("<")[0].strip()
        offset = int(plt_slot_offset, 16)

    name = mat.group("name")

    m[name] = offset
    # print(f"{name} -> 0x{offset:x}")

offsets = []
values  = []

for name, offset in m.items():
    if name + "@plt" in m:
        slot = m[name+"@plt"]
        # print(f"*0x{slot:x} = 0x{offset:x} // {name}")
        offsets.append(f"0x{slot:x}")
        values.append(f"0x{offset:x}")
    ns = name.split("@")
    if name.endswith("@plt") and ns[0] not in m:
        # print(f"*0x{offset:x} = {ns[0]} // {name}")
        offsets.append(f"0x{offset:x}")
        values.append(ns[0])

print(f"#define HOOK_OFFSET {m['base_hook']}")

print("#define PLT_SLOTS { BASE + " + ", BASE + ".join(offsets) + " }")
s = []
for val in values:
    if "0x" in val:
        s.append(f"add + BASE + {val}")
    else:
        s.append(f"(void *) {val}")

print("#define SLOT_VALUES { " + ", ".join(s) + " }")
