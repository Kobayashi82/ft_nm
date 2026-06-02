<div align="center">

![System & Kernel](https://img.shields.io/badge/System-brown?style=for-the-badge)
![Filesystem](https://img.shields.io/badge/Filesystem-Directory-blue?style=for-the-badge)
![C Language](https://img.shields.io/badge/Language-C-red?style=for-the-badge)

*A reimplementation of the classic nm command*

</div>

<div align="center">
  <img src="/images/ft_nm.jpg">
</div>

# ft_nm

[README en Español](README_es.md)

`ft_nm` is a from-scratch implementation of the Unix `nm` command. This project focuses on directory traversal, file metadata, sorting, and output formatting while matching the system `nm` behavior as closely as possible.

### What is nm?

`nm` is the standard utility to:

- List files and directories.
- Inspect permissions, sizes, owners, and timestamps.
- Sort and filter entries.
- Traverse directories recursively.

### Technical flow

1. Parse flags and input paths.
2. Read directory entries with `opendir` and `readdir`.
3. Collect metadata with `stat` or `lstat`.
4. Sort entries by name or time.
5. Format output (long format, hidden files, reverse order).
6. If `-R` is set, recurse into subdirectories.

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

| Option | Description                    |
|--------|--------------------------------|
| `-a`   | Display debugger-only symbols  |
| `-g`   | Display only external symbols  |
| `-u`   | Display only undefined symbols |
| `-r`   | Reverse the sense of the sort  |
| `-p`   | Do not sort the symbols        |

### Examples

```bash
./ft_nm
./ft_nm -la
./ft_nm -lrt src
./ft_nm -R .
```

## 📄 License

This project is licensed under the WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**📂 Developed as part of the 42 School curriculum 📂**

*"See your system as it really is"*

</div>
