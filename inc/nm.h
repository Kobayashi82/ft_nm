
#pragma once

#pragma region "Includes"

	#include <stddef.h>
	#include <stdint.h>
	
	#define NAME "ft_nm"

#pragma endregion

#pragma region "Enumerators"

	typedef enum e_ferror	{ TRUNCATED = 10, UNRECOGNIZED = 11, NO_SYMBOLS = 12, NO_MEMORY = -1 } t_ferror;
	typedef enum e_ftype	{ ELF, ARCHIVE } t_ftype;

#pragma endregion

#pragma region "Structures"

	typedef struct s_options {
		int				debug_syms;    		// -a - Muestra todos los símbolos, incluidos los símbolos de depuración
		int				print_filename;		// -A - Imprime el nombre del archivo de entrada junto a cada símbolo
		int				dynamic;       		// -D - Muestra únicamente los símbolos dinámicos
		int				extern_only;   		// -g - Muestra únicamente los símbolos externos (globales)
		int				numeric_sort;  		// -n - Ordena numéricamente por dirección/valor
		int				no_sort;       		// -p - No ordena los símbolos (preserva el orden original)
		int				reverse_sort;  		// -r - Invierte el orden de la ordenación
		int				undefined_only;		// -u - Muestra únicamente los símbolos no definidos
		int				defined_only;  		// -U - Muestra únicamente los símbolos definidos
		int				no_weak;       		// -W - No muestra los símbolos débiles (weak)
	} t_options;

	typedef struct s_sym {
		char			type;				// Tipo de símbolo
		char			*name;				// Nombre del símbolo
		uint64_t		value;				// Puntero/Valor del símbolo
		int				is_section;			// Indica si es el nombre de una sección
		struct s_sym	*next;				// Siguiente símbolo
	} t_sym;

	typedef struct s_section {
		uint64_t		sh_offset;			// Offset de la sección
		uint64_t		sh_size;			// Tamaño de la sección
		uintptr_t       base;				// Puntero a la sección en memoria
		uint8_t			exists ;			// Indica si la sección existe en el archivo
	} t_section;

	typedef struct s_elf {
		struct s_file	*file;				// Puntero al archivo
		char			*name;				// Nombre del miembro (archivo u objeto dentro del archive)
		int				bits;				// Arquitectura (32/64 bits)
		void			*base;				// Puntero al miembro en memoria
		size_t 			size;				// Longitud del miembro
		t_ferror		error;				// Error asociado al miembro
		uint64_t		shdr_offset;		// Offset de la Tabla de Cabeceras de Secciones (SHT)
    	uint16_t		shdr_num;			// Número de secciones
		t_section		symtab;				// Sección .symtab (tabla de símbolos)
		t_section		strtab;				// Sección .strtab (tabla de cadenas)
		t_section		dynsym;				// Sección .dynsym (tabla de símbolos dinámicos)
		t_section		dynstr;				// Sección .dynstr (tabla de cadenas dinámicas)
		t_section		shstrtab;			// Sección .shstrtab (tabla de cadenas de nombres de secciones)
		t_sym			*sym;				// Lista de símbolos
		t_sym			*last_sym;			// Último símbolo de la lista
		struct s_elf	*next;				// Siguiente miembro
	} t_elf;

	typedef struct s_file {
		char			*path;				// Ruta del archivo
		t_ftype			type;				// Tipo de archivo (Archive o ELF)
		void			*base;				// Puntero al archivo mapeado en memoria
		size_t 			size;				// Longitud total del archivo
		t_elf			*elf;				// Contenido ELF asociado
	} t_file;

	typedef struct s_nm {
		t_file			*file;				// Archivos a procesar
		int				file_total;			// Total de archivos
		t_options		options;			// Opciones de configuración
	} t_nm;

#pragma endregion

#pragma region "Variables"

	extern t_nm g_nm;

#pragma endregion

#pragma region "Methods"

	// Main
	int		options(int argc, char **argv);

	// Symbols
	int		is_undefined(t_sym *s);
	int		load_symbols(t_elf *elf);

	// Section
	int		load_sections(t_elf *elf);

	// File
	int		is_archive(void *base, size_t size);
	int		parse_archive(t_file *file);
	int		map_file(t_file *file);
	int		unmap_file(t_file *file);

	// Output
	t_sym	*sort(t_sym *head, int by_value, int reverse);
	int		print_file(t_file *file);

#pragma endregion
