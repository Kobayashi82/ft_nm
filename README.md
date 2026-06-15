<div align="center">

![System & Kernel](https://img.shields.io/badge/System-brown?style=for-the-badge)
![Binary](https://img.shields.io/badge/Binary-Analysis-blue?style=for-the-badge)
![ELF Format](https://img.shields.io/badge/ELF-archive-orange?style=for-the-badge)
![C Language](https://img.shields.io/badge/Language-C-red?style=for-the-badge)

*A reimplementation of the nm command*

</div>

<div align="center">
  <img src="/images/ft_nm.jpg">
</div>

# ft_nm

[README en Español](README_es.md)

`ft_nm` is an implementation of the Unix `nm` command. This project focuses on `ELF` binary parsing, symbol table extraction, type classification, and output formatting.

### Features:

- Inspect symbol tables inside regular ELF files and static archive libraries (`.a`).
- Identify the type, address, and name of each symbol.
- Filter symbols by visibility, linkage, or definition status.
- Sort symbols alphabetically or numerically.

### Technical flow

1. Parse options and input paths.
2. Memory-map the file.
3. Detect file type: `object`, `binary` or `archive`.
4. Validate the ELF header and detect 32-bit or 64-bit class.
5. Locate the section header table and resolve `.symtab`/`.strtab` (or `.dynsym`/`.dynstr` with `-D`).
6. Extract symbols and classify each one into its type character.
7. Filter symbols based on active options.
8. Sort the symbol list using merge sort.
9. Print results.

## 🔧 Installation

```bash
git clone https://github.com/Kobayashi82/ft_nm.git
cd ft_nm
make
```

## 🖥️ Usage

```bash
./ft_nm [options] [files]
```

### Options

| Option | Long form           | Description                                    |
|--------|---------------------|------------------------------------------------|
| `-a`   | `--debug-syms`      | Display debugger-only symbols                  |
| `-A`   | `--print-file-name` | Print the filename before every symbol         |
| `-D`   | `--dynamic`         | Display dynamic symbols instead of normal ones |
| `-g`   | `--extern-only`     | Display only global symbols                    |
| `-n`   | `--numeric-sort`    | Sort symbols numerically by address            |
| `-p`   | `--no-sort`         | Do not sort the symbols                        |
| `-r`   | `--reverse-sort`    | Reverse the sense of the sort                  |
| `-u`   | `--undefined-only`  | Display only undefined symbols                 |
| `-U`   | `--defined-only`    | Display only defined symbols                   |
| `-W`   | `--no-weak`         | Ignore weak symbols                            |
| `-h`   | `--help`            | Display usage information                      |
| `-V`   | `--version`         | Display version information                    |
|

### Examples

```bash
./ft_nm                                 # defaults to a.out
./ft_nm -g /usr/lib/libc.a              # display global symbols
./ft_nm -n --undefined-only ./binary    # display undefined symbols with not sort
./ft_nm -A -r libfoo.a libbar.a         # print filenames and reverse sort
```

## 🔍 Implementation

### ELF Parsing

The parser supports both `ELF32` and `ELF64` formats targeting x86 (`EM_386`) and x86-64 (`EM_X86_64`) architectures, `little-endian` only.  
After validating the magic number and class field, the section header table is located and iterated to find the relevant sections.

All access is bounds checked against the mapped file size to detect truncated or malformed binaries.

### Symbol Type Classification

Each symbol is assigned a single character that encodes its type and visibility, following the GNU `nm` convention:

| Character | Meaning                  | Upper   | Lower     |
|-----------|--------------------------|---------|-----------|
| `A` / `a` | Absolute symbol          | Global  | Local     |
| `B` / `b` | Unitialized data section | Global  | Local     |
| `C` / `c` | Common symbol            | Global  | Local     |
| `D` / `d` | Initialized data section | Global  | Local     |
| `R` / `r` | Read-only data section   | Global  | Local     |
| `T` / `t` | Text section             | Global  | Local     |
| `U`       | Undefined symbol         | N/A     | N/A       |
| `W` / `w` | Weak symbol              | Defined | Undefined |
| `V` / `v` | Weak object              | Defined | Undefined |
| `u`       | Unique symbol            | N/A     | N/A       |
| `i`       | Indirect function        | N/A     | N/A       |
| `N`       | Debug symbol             | N/A     | N/A       |
| `S` / `n` | Other section symbol     | Global  | Local     |
|

Classification follows the symbol's binding (`STB_GLOBAL`, `STB_WEAK`, `STB_LOCAL`), type (`STT_OBJECT`, `STT_FUNC`, `STT_SECTION`, etc.), and which section it belongs to (`sh_type` and `sh_flags`).

### Section Symbol Name Resolution

`STT_SECTION` symbols have an empty `st_name` by convention. For these, `ft_nm` uses the symbol's `st_shndx` to index into the section header array and reads the section name from `.shstrtab` instead of `.strtab`, producing names like `.text` or `.data`.

### Archive Support

`ft_nm` handles static archive files (`.a`) following the `ar` format. It scans the archive for the long-name table (`//`) first, then iterates over each member, extracting its name and validating the ELF magic number before processing it as an independent binary. Members that are not valid ELF files are reported as malformed.

### Address Formatting

Addresses are printed in hexadecimal, zero-padded to match the binary's pointer width:

- ``32-bit``: 8 hex digits
- ``64-bit``: 16 hex digits

Undefined symbols print spaces instead of an address to maintain column alignment.

### Sorting

Symbols are sorted using `merge sort`.

- **By name:** (default): alphabetical comparison
- **By address:** (`-n`): numeric order, with undefined symbols placed first
- **Reverse:** (`-r`): reverses the result of whichever criterion is active
- **No sort:** (`-p`): skips sorting entirely and preserves symbol table order

## 📄 License

This project is licensed under the WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**⛓️ Developed as part of the 42 School curriculum ⛓️**

*"Undefined symbols are just symbols waiting to be found"*

</div>
