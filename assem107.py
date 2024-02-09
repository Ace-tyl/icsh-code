import argparse
import re, sys


def tokenizer(content: str) -> str:
    content = re.sub(r'\/\*.+?\*\/', "", content)  # Remove comments
    content = re.sub(r'\/\/.+?(?=\n)', "", content)
    return content.lower().split('\n')


def parse_integer(val: str, line_id: int) -> str:
    base = [2, 8, 10, 16, -1]["*$#%".find(val[0])]
    if base == -1:
        sys.stderr.write(f"At line {line_id}: Invalid integer.\n")
        exit(-1)
    value = int(val[1:], base)
    if value < -1024 or value >= 2048:
        sys.stderr.write(f"At line {line_id}: Too large or too small integer.\n")
        exit(-2)
    if value < 0: value += 2048
    return bin(value + 2048)[3:]


def parse_float(val: str, line_id: int) -> str:
    base = [2, 8, 10, 16, -1]["*$#%".find(val[0])]
    if base == -1:
        sys.stderr.write(f"At line {line_id}: Invalid float.\n")
        exit(-2)
    value = 0.
    posneg, multi = 1, 1.
    val = val[1:]
    if not val[0].isdigit():
        if val[0] == '+': posneg = 1
        else: posneg = -1
        val = val[1:]
    if len(val) == 1: val += '.'
    if val[1] != '.':
        sys.stderr.write(f"At line {line_id}: Invalid float.\n")
        exit(-2)
    val = val[0] + val[2:]
    for ch in val:
        digit = "0123456789abcdef".find(ch)
        if digit == -1 or digit >= base:
            sys.stderr.write(f"At line {line_id}: Invalid float.\n")
            exit(-2)
        value += multi * digit
        multi /= base
    result = posneg * round(value * 2**31)
    if result < -2**31 or result >= 2**31:
        sys.stderr.write(f"At line {line_id}: Invalid float.\n")
        exit(-2)
    if result < 0: result += 2**32
    return bin(result + 2**32)[3:]


commands = ["rev", "and", "add", "sub", "mul", "div", "addd", "or", "not", "send", "jmp", "jlz", "jnz", "out", "halt", "xor"]


def parse_one_argument_command(splitted: list, line_id: int) -> str:
    if len(splitted) != 2:
        sys.stderr.write(f"At line {line_id}: Invalid arguments after {splitted[0].upper()}.\n")
        exit(2)
    return f"{bin(commands.index(splitted[0]) + 16)[3:]}0{parse_integer(splitted[1], line_id)}"


def parse_command(line: str, line_id: int) -> str:
    splitted = line.split()
    if splitted[0] not in commands:
        sys.stderr.write(f"At line {line_id}: Invalid command.\n")
        exit(6)
    command_id = commands.index(splitted[0])
    if command_id not in [13, 14]:
        return parse_one_argument_command(splitted, line_id)
    else:
        if len(splitted) != 1:
            sys.stderr.write(f"At line {line_id}: Invalid arguments after {splitted[0].upper()}.\n")
            exit(3)
        if command_id == 13:
            return "1101000000000000"
        else:
            return "1110000000000000"


def assembler(file: str, output: str) -> None:
    if output == "":
        output = (file[0: len(file) - 4] if file.endswith(".asm") else file) + ".bin"
    content = tokenizer(open(file).read())
    memory = []
    line_id = 0
    mode = -1  # 0 is .code and 1 is .data
    for line in content:
        line = line.strip()
        line_id += 1
        if line == "":
            continue
        if line == ".code":
            mode = 0
            continue
        elif line == ".data":
            mode = 1
            continue
        if mode == -1:
            sys.stderr.write(f"At line {line_id}: Invalid code at the beginning.\n")
            exit(1)
        elif mode == 0:
            code = parse_command(line, line_id)
            memory.append(code)
        else:
            if len(memory) & 1:
                memory.append("0000000000000000")
            number = parse_float(line, line_id)
            memory.append(number[0: 16])
            memory.append(number[16:])
    if len(memory) & 1:
        memory.append("0000000000000000")
    output_file = open(output, "w")
    beginning = False
    for data_1, data_2 in zip(memory[0::2], memory[1::2]):
        if not beginning:
            beginning = True
        else:
            output_file.write("00000\n")
        data = data_1 + data_2
        for i in range(8):
            output_file.write(f"1{data[i * 4: i * 4 + 4]}\n")
    output_file.write("00100\n")


def run(args=None) -> None:
    Parser = argparse.ArgumentParser(prog="assem107", description="107 Computer Assembler by SodiumCl10")
    Parser.add_argument("-o", "--output", help="The output file name", nargs="?", default="")
    Parser.add_argument("file", help="The .asm file which will be assembled.", nargs="?", default="")
    Args = Parser.parse_args(args)
    if Args.file == "":
        Parser.print_help()
        exit()
    assembler(Args.file, Args.output)


if __name__ == "__main__":
    run()
