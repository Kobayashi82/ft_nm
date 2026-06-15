<div align="center">

![System & Kernel](https://img.shields.io/badge/System-brown?style=for-the-badge)
![Binary](https://img.shields.io/badge/Binary-Analysis-blue?style=for-the-badge)
![ELF Format](https://img.shields.io/badge/ELF-archive-orange?style=for-the-badge)
![C Language](https://img.shields.io/badge/Language-C-red?style=for-the-badge)

*Una reimplementación del comando nm*

</div>

<div align="center">
  <img src="/images/ft_nm.jpg">
</div>

# ft_nm

[README in English](README.md)

`ft_nm` es una implementación del comando Unix `nm`. Este proyecto se centra en el análisis de binarios `ELF`, la extracción de tablas de símbolos, la clasificación de tipos y el formato de salida.

### Características:

- Inspecciona tablas de símbolos en archivos ELF y bibliotecas estáticas (`.a`).
- Identifica el tipo, la dirección y el nombre de cada símbolo.
- Filtra símbolos por visibilidad, enlazado o estado de definición.
- Ordena símbolos alfabética o numéricamente.

### Flujo técnico

1. Parsear opciones y rutas de entrada.
2. Mapear el archivo en memoria.
3. Detectar el tipo de archivo: `object`, `binary` o `archive`.
4. Validar la cabecera ELF y detectar la clase 32 o 64 bits.
5. Localizar la tabla de cabeceras de sección y resolver `.symtab`/`.strtab` (o `.dynsym`/`.dynstr` con `-D`).
6. Extraer símbolos y clasificar cada uno en su carácter de tipo.
7. Filtrar símbolos según las opciones activas.
8. Ordenar la lista de símbolos con merge sort.
9. Imprimir los resultados.

## 🔧 Instalación

```bash
git clone https://github.com/Kobayashi82/ft_nm.git
cd ft_nm
make
```

## 🖥️ Uso

```bash
./ft_nm [opciones] [archivos]
```

### Opciones

| Opción | Forma larga         | Descripción                                          |
|--------|---------------------|------------------------------------------------------|
| `-a`   | `--debug-syms`      | Mostrar símbolos solo de depuración                  |
| `-A`   | `--print-file-name` | Imprimir el nombre del archivo antes de cada símbolo |
| `-D`   | `--dynamic`         | Mostrar símbolos dinámicos en lugar de los normales  |
| `-g`   | `--extern-only`     | Mostrar solo símbolos globales                       |
| `-n`   | `--numeric-sort`    | Ordenar símbolos numéricamente por dirección         |
| `-p`   | `--no-sort`         | No ordenar los símbolos                              |
| `-r`   | `--reverse-sort`    | Invertir el sentido del orden                        |
| `-u`   | `--undefined-only`  | Mostrar solo símbolos no definidos                   |
| `-U`   | `--defined-only`    | Mostrar solo símbolos definidos                      |
| `-W`   | `--no-weak`         | Ignorar símbolos débiles                             |
| `-h`   | `--help`            | Mostrar información de uso                           |
| `-V`   | `--version`         | Mostrar información de versión                       |
|

### Ejemplos

```bash
./ft_nm                                 # a.out por defecto
./ft_nm -g /usr/lib/libc.a              # mostrar símbolos globales
./ft_nm -n --undefined-only ./binary    # mostrar símbolos indefinidos sin ordenar
./ft_nm -A -r libfoo.a libbar.a         # imprimir nombres de archivo y orden inverso
```

## 🔍 Implementación

### Análisis ELF

El parser soporta los formatos `ELF32` y `ELF64` para arquitecturas x86 (`EM_386`) y x86-64 (`EM_X86_64`), únicamente en `little-endian`.  
Tras validar el número mágico y el campo de clase, se localiza la tabla de cabeceras de sección y se itera para encontrar las secciones relevantes.

Todos los accesos se comprueban contra el tamaño del archivo mapeado para detectar binarios truncados o malformados.

### Clasificación de tipos de símbolo

A cada símbolo se le asigna un único carácter que codifica su tipo y visibilidad, siguiendo la convención de GNU `nm`:

| Carácter  | Significado                      | Mayúscula | Minúscula   |
|-----------|----------------------------------|-----------|-------------|
| `A` / `a` | Símbolo absoluto                 | Global    | Local       |
| `B` / `b` | Sección de datos no inicializada | Global    | Local       |
| `C` / `c` | Símbolo común                    | Global    | Local       |
| `D` / `d` | Sección de datos inicializada    | Global    | Local       |
| `R` / `r` | Sección de solo lectura          | Global    | Local       |
| `T` / `t` | Sección de texto                 | Global    | Local       |
| `U`       | Símbolo no definido              | N/A       | N/A         |
| `W` / `w` | Símbolo débil                    | Definido  | No definido |
| `V` / `v` | Objeto débil                     | Definido  | No definido |
| `u`       | Símbolo único                    | N/A       | N/A         |
| `i`       | Función indirecta                | N/A       | N/A         |
| `N`       | Símbolo de depuración            | N/A       | N/A         |
| `S` / `n` | Otro símbolo de sección          | Global    | Local       |
|

La clasificación sigue el enlazado del símbolo (`STB_GLOBAL`, `STB_WEAK`, `STB_LOCAL`), su tipo (`STT_OBJECT`, `STT_FUNC`, `STT_SECTION`, etc.) y la sección a la que pertenece (`sh_type` y `sh_flags`).

### Resolución de nombres de símbolos de sección

Los símbolos `STT_SECTION` tienen un `st_name` vacío por convención. Para estos, `ft_nm` usa el campo `st_shndx` del símbolo para indexar en el array de cabeceras de sección y lee el nombre de la sección desde `.shstrtab` en lugar de `.strtab`, produciendo nombres como `.text` o `.data`.

### Soporte de archivos estáticos

`ft_nm` maneja archivos estáticos (`.a`) siguiendo el formato `ar`. Primero escanea el archivo en busca de la tabla de nombres largos (`//`), luego itera sobre cada miembro, extrae su nombre y valida el número mágico ELF antes de procesarlo como un binario independiente. Los miembros que no son archivos ELF válidos se reportan como malformados.

### Formato de direcciones

Las direcciones se imprimen en hexadecimal, con ceros a la izquierda según el ancho de puntero del binario:

- ``32-bit``: 8 dígitos hexadecimales
- ``64-bit``: 16 dígitos hexadecimales

Los símbolos no definidos imprimen espacios en lugar de una dirección para mantener la alineación de columnas.

### Ordenación

Los símbolos se ordenan usando `merge sort`.

- **Por nombre** (por defecto): comparación alfabética
- **Por dirección** (`-n`): orden numérico, con los símbolos no definidos primero
- **Inverso** (`-r`): invierte el resultado del criterio activo
- **Sin orden** (`-p`): omite la ordenación y preserva el orden de la tabla de símbolos

## Estructura ELF

```text
┌────────────────────────┬───────────────────────────────────────────────────┐
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

## 📄 Licencia

Este proyecto está bajo la licencia WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**⛓️ Desarrollado como parte del currículo de la 42 School ⛓️**

*"Undefined symbols are just symbols waiting to be found"*

</div>
