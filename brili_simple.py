#!/usr/bin/env python3
"""
brili_simple.py — Interpretador Bril mínimo para o MVP Cool->Bril.
Suporta: const, id, add, sub, mul, div, le, lt, eq, not,
         call, print, br, jmp, ret.
"""
import json, sys

def run(prog):
    funcs = {f["name"]: f for f in prog["functions"]}

    def call_func(name, args):
        func = funcs[name]
        env = {}

        # Mapear argumentos
        if "args" in func:
            for param, val in zip(func["args"], args):
                env[param["name"]] = val

        # @main não tem parâmetros mas o corpo referencia 'self'
        if "self" not in env:
            env["self"] = 0

        # Construir mapa de labels → índice de instrução
        instrs = func["instrs"]
        label_map = {}
        for i, instr in enumerate(instrs):
            if "label" in instr:
                label_map[instr["label"]] = i

        pc = 0
        while pc < len(instrs):
            instr = instrs[pc]

            # Labels são apenas marcadores, pular
            if "label" in instr:
                pc += 1
                continue

            op = instr.get("op")

            if op == "const":
                env[instr["dest"]] = instr["value"]
                pc += 1

            elif op == "id":
                env[instr["dest"]] = env[instr["args"][0]]
                pc += 1

            elif op in ("add","sub","mul","div","le","lt","eq"):
                a = env[instr["args"][0]]
                b = env[instr["args"][1]]
                if   op == "add": env[instr["dest"]] = a + b
                elif op == "sub": env[instr["dest"]] = a - b
                elif op == "mul": env[instr["dest"]] = a * b
                elif op == "div": env[instr["dest"]] = a // b
                elif op == "le":  env[instr["dest"]] = a <= b
                elif op == "lt":  env[instr["dest"]] = a < b
                elif op == "eq":  env[instr["dest"]] = a == b
                pc += 1

            elif op == "not":
                env[instr["dest"]] = not env[instr["args"][0]]
                pc += 1

            elif op == "print":
                for arg in instr["args"]:
                    val = env[arg]
                    # Brilirs imprime bool como true/false, int como número
                    if isinstance(val, bool):
                        print("true" if val else "false")
                    else:
                        print(val)
                pc += 1

            elif op == "jmp":
                pc = label_map[instr["labels"][0]] + 1

            elif op == "br":
                cond = env[instr["args"][0]]
                label = instr["labels"][0] if cond else instr["labels"][1]
                pc = label_map[label] + 1

            elif op == "call":
                callee = instr["funcs"][0]
                call_args = [env[a] for a in instr.get("args", [])]
                result = call_func(callee, call_args)
                if "dest" in instr:
                    env[instr["dest"]] = result
                pc += 1

            elif op == "ret":
                if instr.get("args"):
                    return env[instr["args"][0]]
                return None

            else:
                print(f"[warn] op desconhecido: {op}", file=sys.stderr)
                pc += 1

        return None

    call_func("main", [])

if __name__ == "__main__":
    prog = json.load(sys.stdin)
    run(prog)
