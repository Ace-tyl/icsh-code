import argparse
import re, sys
from numpy import uint32, uint64
import warnings

warnings.filterwarnings('ignore', '.*')


def write_to_memory(memory, data: list, begin: int) -> None:
    tmpdata = ""
    address = begin
    for d in data:
        if d == "": continue
        if len(d) != 5:
            sys.stderr.write("Invalid paper tape binary file!")
            exit(-66)
        if d[0] == '0':
            if d == "00100":
                return
            continue
        tmpdata += d[1:]
        if len(tmpdata) == 32:
            if address < 0 or address >= 1024:
                sys.stderr.write("Invalid address!")
                exit(-65)
            memory[address] = int(tmpdata, 2)
            address += 1
            tmpdata = ""


def simulator(file: str, begin: int, step: bool, detail: bool) -> None:
    memory = [uint32(0) for _ in range(1024)]
    data = open(file).read().split()
    write_to_memory(memory, data, begin)
    A, mem = uint32(0), uint32(0)
    address = begin * 2
    access16 = lambda addr: (memory[addr >> 1] >> 16) if not (addr & 1) else (memory[addr >> 1] & uint32(65535))
    access32 = lambda addr: memory[addr >> 1] if not (addr & 1) else uint32((memory[addr >> 1] << 16) | (memory[(addr >> 1) + 1] >> 16))
    while True:
        instruction = access16(address)
        addr = instruction & 2047
        mem = access32(addr)
        address += 1
        instrtype = instruction >> 12
        if instrtype == 0:
            # REV
            A = mem
        elif instrtype == 1:
            # AND
            A &= mem
        elif instrtype == 2:
            # ADD
            if (A >> 31) == (mem >> 31) and (A >> 31) != ((A + mem) >> 31):
                sys.stderr.write("ADD Overflow!")
                exit(1)
            A += mem
        elif instrtype == 3:
            # SUB
            if (A >> 31) != (mem >> 31) and (A >> 31) != ((A - mem) >> 31):
                sys.stderr.write("SUB Overflow!")
                exit(2)
            A -= mem
        elif instrtype == 4:
            # MUL
            neg = 1
            if (A >> 31): neg, A = -neg, uint32(-A)
            if (mem >> 31): neg, mem = -neg, uint32(-mem)
            A = uint32((uint64(A) * uint64(mem)) // 2**31 * neg)
        elif instrtype == 5:
            # DIV
            neg = 1
            if (A >> 31): neg, A = -neg, uint32(-A)
            if (mem >> 31): neg, mem = -neg, uint32(-mem)
            if A > mem or (neg == 1 and A == mem):
                sys.stderr.write("DIV Overflow!")
                exit(4)
            A = uint32((uint64(A) * 2**31) // uint64(mem) * neg)
        elif instrtype == 6:
            # ADDD
            A += mem
        elif instrtype == 7:
            # OR
            A |= mem
        elif instrtype == 8:
            # NOT
            A = ~mem
        elif instrtype == 9:
            # SEND
            if addr & 1:
                memory[addr >> 1] = (memory[addr >> 1] & -65536) + (A >> 16)
                memory[addr >> 1 | 1] = (memory[addr >> 1 | 1] & 65535) + (A << 16)
            else:
                memory[addr >> 1] = A
        elif instrtype == 10:
            # JMP
            address = addr
        elif instrtype == 11:
            # JLZ
            if (A >> 31): address = addr
        elif instrtype == 12:
            # JNZ
            if A: address = addr
        elif instrtype == 13:
            # OUT
            print(f"{int(A) / 2**31 if not (A >> 31) else -int(-A) / 2**31:.7f}")
        elif instrtype == 14:
            # HALT
            return
        else:
            # XOR
            A ^= mem
        if detail:
            print(f"A = {int(A) / 2**31 if not (A >> 31) else -int(-A) / 2**31:.7f}")
            print(f"Accessing address: {addr}, value: {int(mem) / 2**31 if not (mem >> 31) else -int(-mem) / 2**31:.7f}")
        if step:
            input("Press Enter to continue ...")


def run(args=None) -> None:
    Parser = argparse.ArgumentParser(prog="simu107", description="107 Computer Simulator by SodiumCl10")
    Parser.add_argument("-b", "--begin", help="The beginning memory address of code.", type=int, nargs="?", default=500)
    Parser.add_argument("-s", "--step", help="Step mode.", action="store_true", default=False)
    Parser.add_argument("-d", "--detail", help="Detailed mode.", action="store_true", default=False)
    Parser.add_argument("file", help="The .bin file which will be simulated.", nargs="?", default="")
    Args = Parser.parse_args(args)
    if Args.file == "":
        Parser.print_help()
        exit()
    simulator(Args.file, Args.begin, Args.step, Args.detail)


if __name__ == "__main__":
    run()
