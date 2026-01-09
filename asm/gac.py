#!/usr/bin/env python3

import sys
import re
from typing import Dict, List, Tuple, Any
from pathlib import Path
from dataclasses import dataclass


OPCODES = {
    # OTHER
    "NOP": 0x00,
    "HLT": 0x02,
    "SYNC": 0x03,
    "FILL": 0x04,
    "FILL.W": 0x05,
    "SEED": 0x06,
    "RAND": 0x07,
    "RAND.W": 0x08,
    # REG
    "MOV": 0x0A,
    "MOV.W": 0x0B,
    "LDI": 0x0C,
    "LDI.W": 0x0D,
    "SRB": 0x0E,
    # STACK
    "PUSHI": 0x10,
    "PUSHI.W": 0x11,
    "PUSH": 0x12,
    "PUSH.W": 0x13,
    "POP": 0x15,
    "POP.W": 0x16,
    "SWP": 0x18,
    "SWP.W": 0x19,
    "DUP": 0x1B,
    "DUP.W": 0x1C,
    "ADJSP": 0x1E,
    # RAM
    "LD": 0x20,
    "LD.W": 0x21,
    "ST": 0x22,
    "ST.W": 0x23,
    "LDR": 0x25,
    "LDR.W": 0x26,
    "STR": 0x27,
    "STR.W": 0x28,
    "LDRI": 0x2A,
    "LDRI.W": 0x2B,
    "STRI": 0x2C,
    "STRI.W": 0x2D,
    # ROMA
    "LDA": 0x30,
    "LDA.W": 0x31,
    "LDAR": 0x33,
    "LDAR.W": 0x34,
    "CPYRA": 0x36,
    "SPRRA": 0x37,
    # ALU
    "ADD": 0x40,
    "SUB": 0x41,
    "MUL": 0x42,
    "AND": 0x43,
    "OR": 0x44,
    "XOR": 0x45,
    "CMP": 0x46,
    "ADD.W": 0x48,
    "SUB.W": 0x49,
    "MUL.W": 0x4A,
    "AND.W": 0x4B,
    "OR.W": 0x4C,
    "XOR.W": 0x4D,
    "CMP.W": 0x4E,
    "ADDR": 0x50,
    "SUBR": 0x51,
    "MULR": 0x52,
    "ANDR": 0x53,
    "ORR": 0x54,
    "XORR": 0x55,
    "CMPR": 0x56,
    "ADDR.W": 0x58,
    "SUBR.W": 0x59,
    "MULR.W": 0x5A,
    "ANDR.W": 0x5B,
    "ORR.W": 0x5C,
    "XORR.W": 0x5D,
    "CMPR.W": 0x5E,
    "INC": 0x60,
    "INC.W": 0x61,
    "DEC": 0x62,
    "DEC.W": 0x63,
    "NOT": 0x65,
    "NOT.W": 0x66,
    "SHL": 0x67,
    "SHL.W": 0x68,
    "SHR": 0x69,
    "SHR.W": 0x6A,
    "ASR": 0x6B,
    "ASR.W": 0x6C,
    "SEXT8.W": 0x6E,
    # JUMPS
    "JMP": 0x70,
    "JZ": 0x71,
    "JNZ": 0x72,
    "JC": 0x73,
    "JNC": 0x74,
    "JLT": 0x75,
    "JGE": 0x76,
    "JMPR": 0x78,
    "JZR": 0x79,
    "JNZR": 0x7A,
    "JCR": 0x7B,
    "JNCR": 0x7C,
    "JLTR": 0x7D,
    "JGER": 0x7E,
    # CSTACK
    "PUSHC.W": 0x80,
    "POPC.W": 0x81,
    "ADJCSP": 0x82,
    # CALLS
    "CALL": 0x90,
    "CALLR": 0x91,
    "RET": 0x92,
}

INSTRUCTION_FORMATS = {
    "NOP": (0, []),
    "HLT": (0, []),
    "SYNC": (0, []),
    "FILL": (3, ["R", "R", "R"]),
    "FILL.W": (3, ["R", "R", "R"]),
    "SEED": (1, ["R"]),
    "RAND": (1, ["R"]),
    "RAND.W": (1, ["R"]),
    "MOV": (2, ["R", "R"]),
    "MOV.W": (2, ["R", "R"]),
    "LDI": (2, ["R", "I8"]),
    "LDI.W": (2, ["R", "I16"]),
    "SRB": (1, ["R"]),
    "PUSHI": (1, ["I8"]),
    "PUSHI.W": (1, ["I16"]),
    "PUSH": (1, ["R"]),
    "PUSH.W": (1, ["R"]),
    "POP": (1, ["R"]),
    "POP.W": (1, ["R"]),
    "SWP": (0, []),
    "SWP.W": (0, []),
    "DUP": (0, []),
    "DUP.W": (0, []),
    "ADJSP": (1, ["I8"]),
    "LD": (2, ["R", "A16"]),
    "LD.W": (2, ["R", "A16"]),
    "ST": (2, ["A16", "R"]),
    "ST.W": (2, ["A16", "R"]),
    "LDR": (2, ["R", "R"]),
    "LDR.W": (2, ["R", "R"]),
    "STR": (2, ["R", "R"]),
    "STR.W": (2, ["R", "R"]),
    "LDRI": (3, ["R", "R", "I8"]),
    "LDRI.W": (3, ["R", "R", "I8"]),
    "STRI": (3, ["R", "I8", "R"]),
    "STRI.W": (3, ["R", "I8", "R"]),
    "LDA": (2, ["R", "A16"]),
    "LDA.W": (2, ["R", "A16"]),
    "LDAR": (2, ["R", "R"]),
    "LDAR.W": (2, ["R", "R"]),
    "CPYRA": (3, ["R", "R", "I8"]),
    "SPRRA": (4, ["R", "R", "I8", "I8"]),
    "ADD": (2, ["R", "I8"]),
    "SUB": (2, ["R", "I8"]),
    "MUL": (2, ["R", "I8"]),
    "AND": (2, ["R", "I8"]),
    "OR": (2, ["R", "I8"]),
    "XOR": (2, ["R", "I8"]),
    "CMP": (2, ["R", "I8"]),
    "ADD.W": (2, ["R", "I16"]),
    "SUB.W": (2, ["R", "I16"]),
    "MUL.W": (2, ["R", "I16"]),
    "AND.W": (2, ["R", "I16"]),
    "OR.W": (2, ["R", "I16"]),
    "XOR.W": (2, ["R", "I16"]),
    "CMP.W": (2, ["R", "I16"]),
    "ADDR": (2, ["R", "R"]),
    "SUBR": (2, ["R", "R"]),
    "MULR": (2, ["R", "R"]),
    "ANDR": (2, ["R", "R"]),
    "ORR": (2, ["R", "R"]),
    "XORR": (2, ["R", "R"]),
    "CMPR": (2, ["R", "R"]),
    "ADDR.W": (2, ["R", "R"]),
    "SUBR.W": (2, ["R", "R"]),
    "MULR.W": (2, ["R", "R"]),
    "ANDR.W": (2, ["R", "R"]),
    "ORR.W": (2, ["R", "R"]),
    "XORR.W": (2, ["R", "R"]),
    "CMPR.W": (2, ["R", "R"]),
    "INC": (1, ["R"]),
    "INC.W": (1, ["R"]),
    "DEC": (1, ["R"]),
    "DEC.W": (1, ["R"]),
    "NOT": (1, ["R"]),
    "NOT.W": (1, ["R"]),
    "SHL": (2, ["R", "I8"]),
    "SHL.W": (2, ["R", "I8"]),
    "SHR": (2, ["R", "I8"]),
    "SHR.W": (2, ["R", "I8"]),
    "ASR": (2, ["R", "I8"]),
    "ASR.W": (2, ["R", "I8"]),
    "SEXT8.W": (1, ["R"]),
    "JMP": (1, ["A16"]),
    "JZ": (1, ["A16"]),
    "JNZ": (1, ["A16"]),
    "JC": (1, ["A16"]),
    "JNC": (1, ["A16"]),
    "JLT": (1, ["A16"]),
    "JGE": (1, ["A16"]),
    "JMPR": (1, ["R"]),
    "JZR": (1, ["R"]),
    "JNZR": (1, ["R"]),
    "JCR": (1, ["R"]),
    "JNCR": (1, ["R"]),
    "JLTR": (1, ["R"]),
    "JGER": (1, ["R"]),
    "PUSHC.W": (1, ["R"]),
    "POPC.W": (1, ["R"]),
    "ADJCSP": (1, ["I8"]),
    "CALL": (1, ["A16"]),
    "CALLR": (1, ["R"]),
    "RET": (0, []),
}


@dataclass
class Token:
    """Token from source code"""

    type: str
    value: str
    line: int
    col: int


@dataclass
class Instruction:
    """Instruction for code generation"""

    mnemonic: str
    operands: List[Any]
    line: int
    address: int = 0


@dataclass
class DataDirective:
    """Data directive"""

    type: str
    data: List[Any]
    line: int
    address: int = 0


class AssemblerError(Exception):
    """Assembly error"""

    def __init__(self, message: str, line: int = 0, filename: str = ""):
        self.message = message
        self.line = line
        self.filename = filename
        super().__init__(self.format_error())

    def format_error(self):
        if self.filename and self.line:
            return f"{self.filename}:{self.line}: error: {self.message}"
        elif self.line:
            return f"line {self.line}: error: {self.message}"
        return f"error: {self.message}"


class Assembler:
    def __init__(self):
        self.labels: Dict[str, int] = {}
        self.constants: Dict[str, int] = {}
        self.macros: Dict[str, Tuple[List[str], List[str]]] = {}
        self.current_section = "romb"
        self.sections = {"roma": bytearray(65536), "romb": bytearray(65536)}
        self.section_sizes = {"roma": 0, "romb": 0}
        self.section_addresses = {"roma": 0, "romb": 0}
        self.instructions: List[Instruction | DataDirective] = []
        self.current_file = ""
        self.macro_counter = 0

    def error(self, message: str, line: int = 0):
        """Raises an assembly error"""
        raise AssemblerError(message, line, self.current_file)

    def parse_number(self, s: str, line: int = 0) -> int:
        """Parse numeric literal"""
        s = s.strip()
        try:
            if s.startswith("0x") or s.startswith("0X"):
                return int(s, 16)
            elif s.startswith("0b") or s.startswith("0B"):
                return int(s, 2)
            elif s.startswith("'") and s.endswith("'"):
                # Character literal
                char = s[1:-1]
                if char.startswith("\\"):
                    escape_chars = {
                        "n": 10,
                        "r": 13,
                        "t": 9,
                        "\\": 92,
                        "'": 39,
                        '"': 34,
                        "0": 0,
                    }
                    return escape_chars.get(char[1], ord(char[1]))
                return ord(char)
            else:
                return int(s, 10)
        except ValueError:
            self.error(f"Invalid number format: {s}", line)
            return 0

    def parse_register(self, s: str, line: int = 0) -> int:
        """Parse register name"""
        s = s.strip().upper()
        if s in ["R0", "R1", "R2", "R3"]:
            return int(s[1])
        self.error(f"Invalid register: {s}", line)
        return 0

    def evaluate_expression(self, expr: str, line: int = 0) -> int:
        """Evaluate expression with constants and labels"""
        expr = expr.strip()

        # Replace constants
        for name, value in self.constants.items():
            expr = re.sub(r"\b" + re.escape(name) + r"\b", str(value), expr)

        # Replace labels (in second pass)
        for name, value in self.labels.items():
            expr = re.sub(r"\b" + re.escape(name) + r"\b", str(value), expr)

        # Safe evaluation
        try:
            result = self._eval_simple_expression(expr)
            return result
        except Exception as e:
            self.error(f"Cannot evaluate expression '{expr}': {str(e)}", line)
            return 0

    def _eval_simple_expression(self, expr: str) -> int:
        """Simple expression evaluation"""
        expr = expr.replace(" ", "")

        try:
            # Using eval for numeric expressions
            allowed_names = {"__builtins__": {}}
            result = eval(expr, allowed_names)
            return int(result)
        except:
            # If evaluation fails, return 0 (label will be resolved later)
            return 0

    def tokenize_line(self, line: str, line_num: int) -> List[Token]:
        """Tokenize a line of assembly code"""
        # Remove comments
        if ";" in line:
            line = line[: line.index(";")]

        line = line.strip()
        if not line:
            return []

        tokens = []
        i = 0
        col = 0

        while i < len(line):
            # Skip whitespace
            if line[i].isspace():
                i += 1
                col += 1
                continue

            # Label (including local labels starting with dot)
            if i == 0 or (i > 0 and line[i - 1].isspace()):
                # Check for local labels with dot and macro parameters
                label_match = re.match(
                    r"(\.[a-zA-Z_][a-zA-Z0-9_\\@]*|[a-zA-Z_][a-zA-Z0-9_]*)\s*:",
                    line[i:],
                )
                if label_match:
                    label = label_match.group(1)
                    tokens.append(Token("LABEL", label, line_num, col))
                    i += len(label_match.group(0))
                    col += len(label_match.group(0))
                    continue

            # Directive (only if not a label with dot)
            if line[i] == ".":
                # Check if this is not a label
                if not re.match(r"\.[a-zA-Z_][a-zA-Z0-9_\\@]*\s*:", line[i:]):
                    directive_match = re.match(r"\.([a-zA-Z_][a-zA-Z0-9_]*)", line[i:])
                    if directive_match:
                        directive = directive_match.group(1)
                        tokens.append(
                            Token("DIRECTIVE", directive.lower(), line_num, col)
                        )
                        i += len(directive_match.group(0))
                        col += len(directive_match.group(0))
                        continue

            # String literal
            if line[i] == '"':
                end = i + 1
                while end < len(line) and line[end] != '"':
                    if line[end] == "\\":
                        end += 2
                    else:
                        end += 1
                if end >= len(line):
                    self.error("Unterminated string", line_num)
                string_val = line[i + 1 : end]
                tokens.append(Token("STRING", string_val, line_num, col))
                i = end + 1
                col += end - i + 1
                continue

            # Character literal
            if line[i] == "'":
                end = i + 1
                if end < len(line) and line[end] == "\\":
                    end += 2
                else:
                    end += 1
                if end >= len(line) or line[end] != "'":
                    self.error("Unterminated character literal", line_num)
                char_val = line[i : end + 1]
                tokens.append(Token("CHAR", char_val, line_num, col))
                i = end + 1
                col += end - i + 1
                continue

            # Macro parameter (with backslash)
            if line[i] == "\\":
                param_match = re.match(r"\\([a-zA-Z_@][a-zA-Z0-9_@]*)", line[i:])
                if param_match:
                    param = param_match.group(0)
                    tokens.append(Token("IDENT", param, line_num, col))
                    i += len(param)
                    col += len(param)
                    continue

            # Identifier or mnemonic
            if line[i].isalpha() or line[i] == "_":
                match = re.match(r"([a-zA-Z_][a-zA-Z0-9_.]*)", line[i:])
                if match:
                    ident = match.group(1)
                    tokens.append(Token("IDENT", ident, line_num, col))
                    i += len(ident)
                    col += len(ident)
                    continue

            # Number
            if line[i].isdigit() or (
                line[i] == "0" and i + 1 < len(line) and line[i + 1] in "xXbB"
            ):
                num_match = re.match(r"(0[xX][0-9a-fA-F]+|0[bB][01]+|[0-9]+)", line[i:])
                if num_match:
                    num = num_match.group(1)
                    tokens.append(Token("NUMBER", num, line_num, col))
                    i += len(num)
                    col += len(num)
                    continue

            # Comma
            if line[i] == ",":
                tokens.append(Token("COMMA", ",", line_num, col))
                i += 1
                col += 1
                continue

            # Expression operators
            if line[i] in "+-*/%&|^~()[]":
                tokens.append(Token("OPERATOR", line[i], line_num, col))
                i += 1
                col += 1
                continue

            # Unknown character
            self.error(f"Unexpected character: '{line[i]}'", line_num)
            i += 1
            col += 1

        return tokens

    def parse_operands(self, tokens: List[Token], start_idx: int) -> List[str]:
        """Parse comma-separated operands"""
        operands = []
        current = []

        for i in range(start_idx, len(tokens)):
            if tokens[i].type == "COMMA":
                if current:
                    operands.append(" ".join(t.value for t in current))
                    current = []
            else:
                current.append(tokens[i])

        if current:
            operands.append(" ".join(t.value for t in current))

        return operands

    def first_pass(self, lines: List[str]):
        """First pass: collect labels, macros, and constants"""
        in_macro = False
        macro_name = ""
        macro_params = []
        macro_body = []

        i = 0
        while i < len(lines):
            line = lines[i]
            line_num = i + 1

            tokens = self.tokenize_line(line, line_num)
            if not tokens:
                i += 1
                continue

            # Process macros
            if tokens[0].type == "DIRECTIVE" and tokens[0].value == "macro":
                if len(tokens) < 2:
                    self.error("Macro name expected", line_num)
                macro_name = tokens[1].value.lower()  # Store macro name in lowercase
                macro_params = self.parse_operands(tokens, 2)
                macro_body = []
                in_macro = True
                i += 1
                continue

            if in_macro:
                if tokens[0].type == "DIRECTIVE" and tokens[0].value == "endmacro":
                    self.macros[macro_name] = (macro_params, macro_body)
                    in_macro = False
                else:
                    macro_body.append(line)
                i += 1
                continue

            # Process labels
            if tokens[0].type == "LABEL":
                label = tokens[0].value
                if label in self.labels:
                    self.error(f"Duplicate label: {label}", line_num)
                addr = self.section_addresses[self.current_section]
                self.labels[label] = addr
                tokens = tokens[1:]
                if not tokens:
                    i += 1
                    continue

            # Process directives
            if tokens and tokens[0].type == "DIRECTIVE":
                directive = tokens[0].value

                if directive == "section":
                    if len(tokens) < 2:
                        self.error("Section name expected", line_num)
                    # .section can work with or without comma for single argument
                    start_idx = 2 if len(tokens) > 2 and tokens[1].type == "COMMA" else 1
                    section = tokens[start_idx].value.lower()
                    if section not in ["roma", "romb"]:
                        self.error(f"Invalid section: {section}", line_num)
                    self.current_section = section

                elif directive == "org":
                    if len(tokens) < 2:
                        self.error("Address expected", line_num)
                    # .org can work with or without comma for single argument
                    start_idx = 2 if len(tokens) > 2 and tokens[1].type == "COMMA" else 1
                    addr = self.evaluate_expression(tokens[start_idx].value, line_num)
                    self.section_addresses[self.current_section] = addr

                elif directive == "equ":
                    if len(tokens) < 4:
                        self.error(
                            "Constant definition requires name, comma, and value", line_num
                        )
                    if tokens[2].type != "COMMA":
                        self.error("Expected comma after constant name", line_num)
                    name = tokens[1].value
                    value = self.evaluate_expression(
                        " ".join(t.value for t in tokens[3:]), line_num
                    )
                    self.constants[name] = value

                elif directive == "db":
                    operands = self.parse_operands(tokens, 1)
                    size = 0
                    for op in operands:
                        if op.startswith('"'):
                            size += len(op) - 2  # Without quotes
                        else:
                            size += 1
                    self.section_addresses[self.current_section] += size

                elif directive == "dw":
                    operands = self.parse_operands(tokens, 1)
                    self.section_addresses[self.current_section] += len(operands) * 2

                elif directive == "string":
                    if len(tokens) < 2:
                        self.error("String expected", line_num)
                    # .string can work with or without comma for single argument
                    start_idx = 2 if len(tokens) > 2 and tokens[1].type == "COMMA" else 1
                    string_val = tokens[start_idx].value
                    self.section_addresses[self.current_section] += len(string_val) + 1

                elif directive == "reserve":
                    if len(tokens) < 2:
                        self.error("Size expected", line_num)
                    # .reserve can work with or without comma for single argument
                    start_idx = 2 if len(tokens) > 2 and tokens[1].type == "COMMA" else 1
                    size = self.evaluate_expression(tokens[start_idx].value, line_num)
                    self.section_addresses[self.current_section] += size

                elif directive == "incbin":
                    operands = self.parse_operands(tokens, 1)
                    filename = operands[0].strip('"')
                    try:
                        filepath = Path(self.current_file).parent / filename
                        size = filepath.stat().st_size
                        if len(operands) >= 3:
                            size = min(
                                size, self.evaluate_expression(operands[2], line_num)
                            )
                        self.section_addresses[self.current_section] += size
                    except:
                        self.error(f"Cannot read file: {filename}", line_num)

                elif directive in ["incbmp", "incwav"]:
                    # Skip for now, size will be determined in second pass
                    pass

                elif directive == "inc":
                    # Handle file inclusion in first pass
                    # Extract filename from original line (after .inc directive)
                    # This avoids issues with tokenization of filenames containing dots/dashes
                    original_line = lines[i].strip()
                    if ";" in original_line:
                        original_line = original_line[:original_line.index(";")]
                    original_line = original_line.strip()
                    
                    # Find .inc and get everything after it
                    inc_match = re.search(r'\.inc\s+(.+)', original_line, re.IGNORECASE)
                    if not inc_match:
                        self.error("Filename expected", line_num)
                    
                    filename = inc_match.group(1).strip().strip('"')
                    try:
                        filepath = Path(self.current_file).parent / filename
                        with open(filepath, "r", encoding="utf-8") as f:
                            inc_lines = f.readlines()

                        # Save current file
                        saved_file = self.current_file
                        self.current_file = str(filepath)

                        # Recursive processing
                        self.first_pass(inc_lines)

                        # Restore file
                        self.current_file = saved_file
                    except Exception as e:
                        self.error(
                            f"Cannot include file {filename}: {str(e)}", line_num
                        )

                i += 1
                continue

            # Process instructions
            if tokens and tokens[0].type == "IDENT":
                mnemonic_lower = tokens[0].value.lower()
                mnemonic = tokens[0].value.upper()

                # Check for macro call
                if mnemonic_lower in self.macros:
                    # Expand macro and insert lines for processing
                    args = self.parse_operands(tokens, 1)
                    expanded_lines = self.expand_macro(mnemonic_lower, args, line_num)

                    # Insert expanded lines
                    lines = lines[:i] + expanded_lines + lines[i + 1 :]
                    continue

                if mnemonic in INSTRUCTION_FORMATS:
                    # Determine instruction size
                    num_ops, op_types = INSTRUCTION_FORMATS[mnemonic]
                    size = 1  # Opcode

                    for op_type in op_types:
                        if op_type == "R":
                            size += 1
                        elif op_type == "I8":
                            size += 1
                        elif op_type == "I16" or op_type == "A16":
                            size += 2

                    self.section_addresses[self.current_section] += size
                else:
                    self.error(f"Unknown instruction: {mnemonic}", line_num)

            i += 1

    def expand_macro(
        self, macro_name: str, args: List[str], line_num: int
    ) -> List[str]:
        """Expand a macro"""
        if macro_name not in self.macros:
            self.error(f"Unknown macro: {macro_name}", line_num)
            return []

        params, body = self.macros[macro_name]

        if len(args) != len(params):
            self.error(
                f"Macro {macro_name} expects {len(params)} arguments, got {len(args)}",
                line_num,
            )
            return []

        # Create parameter replacements
        replacements = {}
        for param, arg in zip(params, args):
            replacements[f"\\{param.strip()}"] = arg.strip()

        # Replace \@ with unique number
        unique_id = self.macro_counter
        self.macro_counter += 1

        # Expand macro body
        expanded = []
        for line in body:
            # Replace parameters
            for old, new in replacements.items():
                line = line.replace(old, new)
            # Replace \@
            line = line.replace("\\@", str(unique_id))
            expanded.append(line)

        return expanded

    def second_pass(self, lines: List[str]):
        """Second pass: code generation"""
        # Reset section addresses
        self.section_addresses = {"roma": 0, "romb": 0}
        self.current_section = "romb"

        in_macro = False

        i = 0
        while i < len(lines):
            line = lines[i]
            line_num = i + 1

            tokens = self.tokenize_line(line, line_num)
            if not tokens:
                i += 1
                continue

            # Skip macro definitions
            if tokens[0].type == "DIRECTIVE" and tokens[0].value == "macro":
                in_macro = True
                i += 1
                continue

            if in_macro:
                if tokens[0].type == "DIRECTIVE" and tokens[0].value == "endmacro":
                    in_macro = False
                i += 1
                continue

            # Skip labels
            if tokens[0].type == "LABEL":
                tokens = tokens[1:]
                if not tokens:
                    i += 1
                    continue

            # Process directives
            if tokens and tokens[0].type == "DIRECTIVE":
                directive = tokens[0].value

                if directive == "section":
                    # .section can work with or without comma for single argument
                    start_idx = 2 if len(tokens) > 2 and tokens[1].type == "COMMA" else 1
                    section = tokens[start_idx].value.lower()
                    self.current_section = section

                elif directive == "org":
                    # .org can work with or without comma for single argument
                    start_idx = 2 if len(tokens) > 2 and tokens[1].type == "COMMA" else 1
                    addr = self.evaluate_expression(tokens[start_idx].value, line_num)
                    self.section_addresses[self.current_section] = addr

                elif directive == "equ":
                    # Already processed in first pass
                    pass

                elif directive == "db":
                    operands = self.parse_operands(tokens, 1)
                    for op in operands:
                        if op.startswith('"') and op.endswith('"'):
                            # String
                            string_val = self._parse_string(op[1:-1])
                            for byte in string_val:
                                self._emit_byte(byte)
                        elif op.startswith("'"):
                            # Character
                            val = self.parse_number(op, line_num)
                            self._emit_byte(val)
                        else:
                            # Number
                            val = self.evaluate_expression(op, line_num)
                            self._emit_byte(val & 0xFF)

                elif directive == "dw":
                    operands = self.parse_operands(tokens, 1)
                    for op in operands:
                        val = self.evaluate_expression(op, line_num)
                        self._emit_word(val)

                elif directive == "string":
                    # .string can work with or without comma for single argument
                    start_idx = 2 if len(tokens) > 2 and tokens[1].type == "COMMA" else 1
                    string_val = self._parse_string(tokens[start_idx].value)
                    for byte in string_val:
                        self._emit_byte(byte)
                    self._emit_byte(0)  # Null terminator

                elif directive == "reserve":
                    # .reserve can work with or without comma for single argument
                    start_idx = 2 if len(tokens) > 2 and tokens[1].type == "COMMA" else 1
                    size = self.evaluate_expression(tokens[start_idx].value, line_num)
                    for _ in range(size):
                        self._emit_byte(0)

                elif directive == "incbin":
                    operands = self.parse_operands(tokens, 1)
                    filename = operands[0].strip('"')
                    offset = 0
                    size = None
                    if len(operands) >= 2:
                        offset = self.evaluate_expression(operands[1], line_num)
                    if len(operands) >= 3:
                        size = self.evaluate_expression(operands[2], line_num)

                    try:
                        filepath = Path(self.current_file).parent / filename
                        with open(filepath, "rb") as f:
                            f.seek(offset)
                            data = f.read(size) if size else f.read()
                            for byte in data:
                                self._emit_byte(byte)
                    except Exception as e:
                        self.error(f"Cannot read file {filename}: {str(e)}", line_num)

                elif directive == "incbmp":
                    # .incbmp can work with or without comma for single argument
                    start_idx = 2 if len(tokens) > 2 and tokens[1].type == "COMMA" else 1
                    filename = tokens[start_idx].value.strip('"')
                    try:
                        filepath = Path(self.current_file).parent / filename
                        data = self._read_bmp(filepath)
                        for byte in data:
                            self._emit_byte(byte)
                    except Exception as e:
                        self.error(
                            f"Cannot read BMP file {filename}: {str(e)}", line_num
                        )

                elif directive == "incwav":
                    # .incwav can work with or without comma for single argument
                    start_idx = 2 if len(tokens) > 2 and tokens[1].type == "COMMA" else 1
                    filename = tokens[start_idx].value.strip('"')
                    try:
                        filepath = Path(self.current_file).parent / filename
                        data = self._read_wav(filepath)
                        for byte in data:
                            self._emit_byte(byte)
                    except Exception as e:
                        self.error(
                            f"Cannot read WAV file {filename}: {str(e)}", line_num
                        )

                elif directive == "inc":
                    # Extract filename from original line (after .inc directive)
                    # This avoids issues with tokenization of filenames containing dots/dashes
                    original_line = lines[i].strip()
                    if ";" in original_line:
                        original_line = original_line[:original_line.index(";")]
                    original_line = original_line.strip()
                    
                    # Find .inc and get everything after it
                    inc_match = re.search(r'\.inc\s+(.+)', original_line, re.IGNORECASE)
                    if not inc_match:
                        self.error("Filename expected", line_num)
                    
                    filename = inc_match.group(1).strip().strip('"')
                    try:
                        filepath = Path(self.current_file).parent / filename
                        with open(filepath, "r", encoding="utf-8") as f:
                            inc_lines = f.readlines()

                        # Save current file
                        saved_file = self.current_file
                        self.current_file = str(filepath)

                        # Recursive processing
                        self.second_pass(inc_lines)

                        # Restore file
                        self.current_file = saved_file
                    except Exception as e:
                        self.error(
                            f"Cannot include file {filename}: {str(e)}", line_num
                        )

                i += 1
                continue

            # Process instructions
            if tokens and tokens[0].type == "IDENT":
                mnemonic_lower = tokens[0].value.lower()
                mnemonic = tokens[0].value.upper()

                # Check for macro call
                if mnemonic_lower in self.macros:
                    args = self.parse_operands(tokens, 1)
                    expanded_lines = self.expand_macro(mnemonic_lower, args, line_num)

                    # Insert expanded lines
                    lines = lines[:i] + expanded_lines + lines[i + 1 :]
                    continue

                if mnemonic in INSTRUCTION_FORMATS:
                    operands = self.parse_operands(tokens, 1)
                    self._assemble_instruction(mnemonic, operands, line_num)
                else:
                    self.error(f"Unknown instruction: {mnemonic}", line_num)

            i += 1

    def _assemble_instruction(self, mnemonic: str, operands: List[str], line_num: int):
        """Assemble a single instruction"""
        if mnemonic not in OPCODES:
            self.error(f"Unknown instruction: {mnemonic}", line_num)
            return

        opcode = OPCODES[mnemonic]
        num_ops, op_types = INSTRUCTION_FORMATS[mnemonic]

        if len(operands) != num_ops:
            self.error(
                f"{mnemonic} expects {num_ops} operands, got {len(operands)}", line_num
            )
            return

        # Emit opcode
        self._emit_byte(opcode)

        # Emit operands
        for operand, op_type in zip(operands, op_types):
            if op_type == "R":
                reg = self.parse_register(operand, line_num)
                self._emit_byte(reg)
            elif op_type == "I8":
                val = self.evaluate_expression(operand, line_num)
                self._emit_byte(val & 0xFF)
            elif op_type == "I16":
                val = self.evaluate_expression(operand, line_num)
                self._emit_word(val & 0xFFFF)
            elif op_type == "A16":
                val = self.evaluate_expression(operand, line_num)
                self._emit_word(val & 0xFFFF)

    def _emit_byte(self, value: int):
        """Write a byte to current section"""
        addr = self.section_addresses[self.current_section]
        self.sections[self.current_section][addr] = value & 0xFF
        self.section_addresses[self.current_section] += 1
        self.section_sizes[self.current_section] = max(
            self.section_sizes[self.current_section],
            self.section_addresses[self.current_section],
        )

    def _emit_word(self, value: int):
        """Write a 16-bit word (little-endian)"""
        self._emit_byte(value & 0xFF)
        self._emit_byte((value >> 8) & 0xFF)

    def _parse_string(self, s: str) -> bytes:
        """Parse string literal with escape sequences"""
        result = []
        i = 0
        while i < len(s):
            if s[i] == "\\" and i + 1 < len(s):
                escape_chars = {
                    "n": 10,
                    "r": 13,
                    "t": 9,
                    "\\": 92,
                    "'": 39,
                    '"': 34,
                    "0": 0,
                }
                next_char = s[i + 1]
                result.append(escape_chars.get(next_char, ord(next_char)))
                i += 2
            else:
                result.append(ord(s[i]))
                i += 1
        return bytes(result)

    def _read_bmp(self, filepath: Path) -> bytes:
        """Read 8-bit BMP file and return pixel data"""
        with open(filepath, "rb") as f:
            # Read BMP header
            header = f.read(54)
            if len(header) < 54:
                raise ValueError("Invalid BMP file")

            # Skip palette (256 colors * 4 bytes)
            f.read(1024)

            # Read all pixel data
            pixel_data = f.read()

            return pixel_data

    def _read_wav(self, filepath: Path) -> bytes:
        """Read 8-bit WAV file and return audio samples"""
        with open(filepath, "rb") as f:
            # Simple read: skip header (44 bytes) and read data
            header = f.read(44)
            if len(header) < 44:
                raise ValueError("Invalid WAV file")

            # Read all audio data
            audio_data = f.read()

            return audio_data

    def assemble_file(
        self,
        filename: str,
        output_roma: str = "output.roma",
        output_romb: str = "output.romb",
    ):
        """Assemble a file"""
        self.current_file = filename

        try:
            with open(filename, "r", encoding="utf-8") as f:
                lines = f.readlines()
        except Exception as e:
            print(f"Error reading file {filename}: {str(e)}")
            return False

        try:
            # First pass
            print(f"First pass: {filename}")
            self.first_pass(lines)

            # Second pass
            print(f"Second pass: {filename}")
            self.second_pass(lines)

            # Write output files
            print(f"Writing {output_roma}...")
            with open(output_roma, "wb") as f:
                f.write(self.sections["roma"][: self.section_sizes["roma"]])

            print(f"Writing {output_romb}...")
            with open(output_romb, "wb") as f:
                f.write(self.sections["romb"][: self.section_sizes["romb"]])

            print(f"Assembly successful!")
            print(f"  ROMA size: {self.section_sizes['roma']} bytes")
            print(f"  ROMB size: {self.section_sizes['romb']} bytes")
            print(f"  Labels: {len(self.labels)}")
            print(f"  Constants: {len(self.constants)}")
            print(f"  Macros: {len(self.macros)}")

            return True

        except AssemblerError as e:
            print(e.format_error())
            return False
        except Exception as e:
            print(f"Unexpected error: {str(e)}")
            import traceback

            traceback.print_exc()
            return False


def main():
    if len(sys.argv) < 2:
        print("Usage: gac.py <input.asm> [output_roma] [output_romb]")
        print("  Default output files: output.roma, output.romb")
        sys.exit(1)

    input_file = sys.argv[1]
    output_roma = sys.argv[2] if len(sys.argv) > 2 else "output.roma"
    output_romb = sys.argv[3] if len(sys.argv) > 3 else "output.romb"

    assembler = Assembler()
    success = assembler.assemble_file(input_file, output_roma, output_romb)

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
