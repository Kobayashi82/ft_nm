# Notas

Para mostrar los tres campos se necesitan dos secciones trabajando juntas:

.symtab			Contiene las entradas de símbolos. Cada entrada es una struct Elf64_Sym con campos numéricos. Aquí está el valor/puntero y los metadatos, pero no el nombre como string.
.strtab			Es una tabla de strings, básicamente un bloque de bytes donde los nombres están concatenados separados por \0. Cada entrada en .symtab tiene un campo st_name que es un offset dentro de .strtab. Así se obtiene el nombre.
Son inseparables. Sin .strtab solo tienes números.

De dónde sale cada campo

### PUNTERO:

Sale de st_value en Elf64_Sym. Pero ojo:

- Para símbolos definidos (st_shndx != SHN_UNDEF) es la dirección virtual del símbolo
- Para símbolos SHN_COMMON es la alineación requerida, no una dirección
- Para símbolos U (undefined) es 0, y nm no muestra nada, deja el espacio en blanco

### LETRA:

No viene de ningún campo directamente. Se calculas a partir de varios campos combinados:

|                   |                                                                                       |
|-------------------|---------------------------------------------------------------------------------------|
| st_info           | Binding (global/local/weak) y type (func/object/section...)                           |
| st_shndx          | A qué sección pertenece, o si es UNDEF/ABS/COMMON                                     |
| Nombre de sección | Determina si es T/t, D/d, B/b, R/r...  corresponde a (.text, .data, .bss, .rodata...) |
| Mayúscula         | Global                                                                                |
| Minúscula         | Local                                                                                 |
|

### NOMBRE:

- st_name				Es el offset. Haces strtab_base + st_name y tienes el string.

Para calcular la letra correctamente necesitas saber en qué sección está el símbolo y cómo se llama esa sección:

Los section headers (Elf64_Shdr) te dicen el nombre, tipo y flags de cada sección.
Los nombres de las secciones también viven en otra string table: .shstrtab (section header string table), cuyo índice está en e_shstrndx del ELF header

```text
ELF Header
  └─ e_shstrndx ──────────────────► .shstrtab (nombres de secciones)
  └─ e_shoff ──► Section Headers
                    └─ sh_name ──► .shstrtab
                    └─ (una entrada apunta a) .symtab
                    └─ (otra apunta a) .strtab

.symtab (array de Elf64_Sym)
  └─ st_name ──────────────────────► .strtab (nombres de símbolos)
  └─ st_value ─────────────────────► dirección (el puntero que muestras)
  └─ st_info ──────────────────────► binding + type (para calcular letra)
  └─ st_shndx ─────────────────────► índice en Section Headers (para calcular letra)
```

  Para mostrar una línea necesitas haber parseado:

|                 |                                                                                   |
|-----------------|-----------------------------------------------------------------------------------|
| ELF header      | Para encontrar la sección headers y el índice de .shstrtab                        |
| Section headers | Para localizar .symtab, .strtab, y tener los nombres/flags de todas las secciones |
| .strtab         | Para resolver nombres de símbolos                                                 |
| .shstrtab       | Para saber si una sección se llama .text, .bss, etc.                              |
| .symtab         | Iterar sobre los símbolos y con todo lo anterior calcular los tres campos         |
|


Los que se cubren con lo que hemos visto:

|                        |                                                                                                   |
|------------------------|---------------------------------------------------------------------------------------------------|
| -D / --dynamic         | En vez de leer .symtab + .strtab, lees .dynsym + .dynstr. Misma lógica, distinta sección          |
| -g / --extern-only     | Filtras por binding. Solo muestras símbolos donde ELF64_ST_BIND(st_info) == STB_GLOBAL o STB_WEAK |
| -u / --undefined-only  | Filtras por st_shndx == SHN_UNDEF                                                                 |
| -U / --defined-only    | Filtras por st_shndx != SHN_UNDEF                                                                 |
| -W / --no-weak         | Filtras por binding, descartas STB_WEAK                                                           |
| -n / --numeric-sort    | Ordenas por st_value en vez de por nombre                                                         |
| -p / --no-sort         | No ordenas, muestras en el orden del array                                                        |
| -r / --reverse-sort    | Inviertes el orden del sort que apliques                                                          |
| -A / --print-file-name | Prefijas cada línea con el nombre del archivo. Solo presentación                                  |
| -a / --debug-syms      | No filtrar STT_FILE / STT_SECTION                                                                 |
|

Los símbolos de debug no viven en .symtab de forma especial, sino que se filtran los símbolos cuyo type es STT_FILE y los que apuntan a secciones de debug.
Con -a se incluyen todos sin filtrar, incluidos:

Símbolos con type STT_FILE		El nombre del archivo fuente .c
Símbolos con type STT_SECTION	Entradas que representan secciones, no código real

```text
┌────────────────────────────────────────────────────────────────────────────┐
│                             ELF FILE STRUCTURE                             │
├────────────────────────┬───────────────────────────────────────────────────┤
│         Offset         │                     Contenido                     │
├────────────────────────┼───────────────────────────────────────────────────┤
│                        │ ELF HEADER                                        │
│ 0x00                   │   Magic:      0x7f 'E' 'L' 'F'                    │
│                        │   e_ident:    clase (32/64), endianness, OS ABI   │
│                        │   e_type:     ET_EXEC / ET_DYN / ET_REL           │
│                        │   e_machine:  EM_X86_64, EM_ARM...                │
│                        │   e_entry:    punto de entrada (virtual addr)     │
│                        │   e_phoff:    offset → Program Header Table       │
│                        │   e_shoff:    offset → Section Header Table       │
│                        │   e_shstrndx: índice de .shstrtab en la SHT       │
├────────────────────────┼───────────────────────────────────────────────────┤
│                        │ PROGRAM HEADER TABLE                              │
│ e_phoff                │   Describe segmentos en memoria                   │
│                        │   PT_LOAD, PT_DYNAMIC, PT_INTERP...               │
│                        │   p_offset, p_vaddr, p_filesz, p_memsz            │
│                        │   p_flags: R / W / X                              │
├────────────────────────┼───────────────────────────────────────────────────┤
│                        │ .text                                             │
│                        │   Código máquina ejecutable                       │
│                        │   SHT_PROGBITS | SHF_ALLOC | SHF_EXECINSTR        │
│                        ├───────────────────────────────────────────────────┤
│                        │ .rodata                                           │
│                        │   Datos de solo lectura (literales, constantes)   │
│                        │   SHT_PROGBITS | SHF_ALLOC                        │
│                        ├───────────────────────────────────────────────────┤
│                        │ .data                                             │
│                        │   Variables globales y estáticas inicializadas    │
│                        │   SHT_PROGBITS | SHF_ALLOC | SHF_WRITE            │
│                        ├───────────────────────────────────────────────────┤
│                        │ .bss                                              │
│                        │   Variables globales y estáticas sin inicializar  │
│                        │   SHT_NOBITS — 0 bytes en disco, se zeroan al     │
│                        │   cargar en memoria                               │
│                        ├───────────────────────────────────────────────────┤
│                        │ .symtab                                           │
│                        │   Tabla de símbolos (ausente si stripped)         │
│        SECTIONS        │   Elf64_Sym: st_name, st_value, st_size,          │
│                        │              st_info (type+bind), st_shndx        │
│                        ├───────────────────────────────────────────────────┤
│                        │ .strtab                                           │
│                        │   Strings de nombres de símbolos (null-term)      │
│                        │   Indexado por st_name de cada Elf64_Sym          │
│                        ├───────────────────────────────────────────────────┤
│                        │ .rela.text / .rel.text                            │
│                        │   Entradas de reubicación para el linker          │
│                        │   r_offset, r_info (sym + type), r_addend         │
│                        ├───────────────────────────────────────────────────┤
│                        │ .dynamic                                          │
│                        │   Solo en shared libs / PIE                       │
│                        │   DT_NEEDED, DT_RPATH, DT_SYMTAB...               │
│                        ├───────────────────────────────────────────────────┤
│                        │ .got / .plt                                       │
│                        │   Global Offset Table + Procedure Linkage Table   │
│                        │   Lazy binding de símbolos externos en runtime    │
│                        ├───────────────────────────────────────────────────┤
│                        │ .shstrtab                                         │
│                        │   Strings de nombres de secciones (null-term)     │
│                        │   e_shstrndx apunta a esta sección                │
├────────────────────────┼───────────────────────────────────────────────────┤
│                        │ SECTION HEADER TABLE                              │
│ e_shoff                │   Una Elf64_Shdr por cada sección                 │
│                        │   sh_name, sh_type, sh_flags, sh_addr,            │
│                        │   sh_offset, sh_size, sh_link, sh_info,           │
│                        │   sh_addralign, sh_entsize                        │
└────────────────────────┴───────────────────────────────────────────────────┘
```

## Tabla de Simbolos

| Bind (`ELF64_ST_BIND`) | Type (`ELF64_ST_TYPE`) | `st_shndx` Value | Letter | Description | Source of Symbol Name |
|------------------------|------------------------|------------------|--------|-------------|-----------------------|
| `STB_LOCAL` (0)        | `STT_NOTYPE` (0)       | `SHN_UNDEF` (0)  | `u`    | Local undefined symbol (rare, debugging; **no value displayed**) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_LOCAL`            | `STT_FUNC` (2)         | `SHN_UNDEF` (0)  | `u`    | Local undefined function (rare; **no value displayed**) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_LOCAL`            | `STT_NOTYPE`           | Section index (code: `.text`, `.plt`) | `t` | Local symbol in code section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_LOCAL`            | `STT_FUNC`             | Section index (code: `.text`, `.plt`) | `t` | Local function in code section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_LOCAL`            | `STT_NOTYPE`           | Section index (data-like: `.data`, `.init_array`, `.fini_array`, `.data.rel.ro`, `.got`, `.got.plt`, `.dynamic`, `.ctors`, `.dtors`, `.jcr`) | `d` | Local symbol in data-like section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_LOCAL`            | `STT_OBJECT` (1)       | Section index (data-like: `.data`, `.init_array`, `.fini_array`, `.data.rel.ro`, `.got`, `.got.plt`, `.dynamic`, `.ctors`, `.dtors`, `.jcr`) | `d` | Local data object in data-like section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_LOCAL`            | `STT_NOTYPE`           | Section index (`.bss`) | `b` | Local symbol in `.bss` (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_LOCAL`            | `STT_OBJECT`           | Section index (`.bss`) | `b` | Local data object in `.bss` (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_LOCAL`            | `STT_NOTYPE`           | Section index (read-only: `.rodata`, `.rodata1`, `.eh_frame`, `.eh_frame_hdr`, `.note.ABI-tag`) | `r` | Local symbol in read-only section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_LOCAL`            | `STT_OBJECT`           | Section index (read-only: `.rodata`, `.rodata1`, `.eh_frame`, `.eh_frame_hdr`, `.note.ABI-tag`) | `r` | Local data object in read-only section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_LOCAL`            | `STT_SECTION` (3)      | Section index    | `n`    | Section symbol (display `st_value`) | Section name from section header string table (`.shstrtab`), offset by `Elf64_Shdr.sh_name` for the section at `st_shndx` |
| `STB_LOCAL`            | `STT_FILE` (4)         | `SHN_ABS`        | `n`    | Source file name (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`), via `sh_link` |
| `STB_LOCAL`            | `STT_NOTYPE`/`STT_OBJECT` | `SHN_ABS`     | `a`    | Local absolute symbol (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL` (1)       | `STT_NOTYPE`           | `SHN_UNDEF`      | `U`    | Undefined global symbol (**no value displayed**) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_FUNC`             | `SHN_UNDEF`      | `U`    | Undefined global function (**no value displayed**) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_NOTYPE`           | `SHN_COMMON`     | `C`    | Common symbol (unallocated; display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_NOTYPE`           | Section index (code: `.text`, `.plt`) | `T` | Global symbol in code section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_FUNC`             | Section index (code: `.text`, `.plt`) | `T` | Global function in code section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_NOTYPE`           | Section index (data-like: `.data`, `.init_array`, `.fini_array`, `.data.rel.ro`, `.got`, `.got.plt`, `.dynamic`, `.ctors`, `.dtors`, `.jcr`) | `D` | Global symbol in data-like section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_OBJECT`           | Section index (data-like: `.data`, `.init_array`, `.fini_array`, `.data.rel.ro`, `.got`, `.got.plt`, `.dynamic`, `.ctors`, `.dtors`, `.jcr`) | `D` | Global data object in data-like section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_NOTYPE`           | Section index (`.bss`) | `B` | Global symbol in `.bss` (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_OBJECT`           | Section index (`.bss`) | `B` | Global data object in `.bss` (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_NOTYPE`           | Section index (read-only: `.rodata`, `.rodata1`, `.eh_frame`, `.eh_frame_hdr`, `.note.ABI-tag`) | `R` | Global symbol in read-only section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_OBJECT`           | Section index (read-only: `.rodata`, `.rodata1`, `.eh_frame`, `.eh_frame_hdr`, `.note.ABI-tag`) | `R` | Global data object in read-only section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_GLOBAL`           | `STT_NOTYPE`/`STT_OBJECT` | `SHN_ABS`     | `A`    | Global absolute symbol (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK` (2)         | `STT_NOTYPE`/`STT_OBJECT` | `SHN_UNDEF` | `w`    | Weak undefined symbol (**no value displayed**) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_FUNC`             | `SHN_UNDEF`      | `w`    | Weak undefined function (**no value displayed**) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_NOTYPE`           | `SHN_COMMON`     | `C`    | Weak common symbol (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_NOTYPE`           | Section index (code: `.text`, `.plt`) | `W` | Weak symbol in code section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_FUNC`             | Section index (code: `.text`, `.plt`) | `W` | Weak function in code section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_NOTYPE`           | Section index (data-like: `.data`, `.init_array`, `.fini_array`, `.data.rel.ro`, `.got`, `.got.plt`, `.dynamic`, `.ctors`, `.dtors`, `.jcr`) | `W` | Weak symbol in data-like section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_OBJECT`           | Section index (data-like: `.data`, `.init_array`, `.fini_array`, `.data.rel.ro`, `.got`, `.got.plt`, `.dynamic`, `.ctors`, `.dtors`, `.jcr`) | `W` | Weak data object in data-like section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_NOTYPE`           | Section index (`.bss`) | `W` | Weak symbol in `.bss` (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_OBJECT`           | Section index (`.bss`) | `W` | Weak data object in `.bss` (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_NOTYPE`           | Section index (read-only: `.rodata`, `.rodata1`, `.eh_frame`, `.eh_frame_hdr`, `.note.ABI-tag`) | `W` | Weak symbol in read-only section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_OBJECT`           | Section index (read-only: `.rodata`, `.rodata1`, `.eh_frame`, `.eh_frame_hdr`, `.note.ABI-tag`) | `W` | Weak data object in read-only section (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |
| `STB_WEAK`             | `STT_NOTYPE`/`STT_OBJECT` | `SHN_ABS`     | `W`    | Weak absolute symbol (display `st_value`) | `.strtab` section, offset by `Elf64_Sym.st_name` (for `SHT_SYMTAB`) or `.dynstr` (for `SHT_DYNSYM`), via `sh_link` |