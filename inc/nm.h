
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
		int				debug_syms;    		// -a - Display all symbols, including debugging symbols
		int				print_filename;		// -A - Print the name of the input file with each symbol
		int				dynamic;       		// -D - Display dynamic symbols only
		int				extern_only;   		// -g - Display only external (global) symbols
		int				numeric_sort;  		// -n - Sort numerically by address/value
		int				no_sort;       		// -p - Do not sort symbols (preserve order)
		int				reverse_sort;  		// -r - Reverse the sort order
		int				undefined_only;		// -u - Display only undefined symbols
		int				defined_only;  		// -U - Display only defined symbols
		int				no_weak;       		// -W - Do not display weak symbols
	} t_options;

	typedef struct s_sym {
		char			type;				// Symbol type
		char			*name;				// Symbol name
		uint64_t		value;				// Symbol pointer
		int				is_section;			// Is a section name
		struct s_sym	*next;				// Next symbol
	} t_sym;

	typedef struct s_section {
		uint64_t		sh_offset;			// 
		uint64_t		sh_size;			// 
		uint8_t			exists ;			// 
	} t_section;

	typedef struct s_elf {
		struct s_file	*file;				// Pointer to the parent file
		char			*name;				// Member name
		int				bits;				// Architecture (32/64 bits)
		uint16_t		type;				// Type of member
		void			*base;				// Pointer to the member in memory
		size_t 			size;				// Length of the member
		t_ferror		error;				// Error for the member
		uint64_t		shdr_offset;		// Offset of SHT
    	uint16_t		shdr_num;			// Number of sections
		t_section		symtab;				// 
		t_section		strtab;				// 
		t_section		dynsym;				// 
		t_section		dynstr;				// 
		t_sym			*sym;				// List of symbols
		t_sym			*last_sym;			// Last symbol
		struct s_elf	*next;				// Next member
	} t_elf;

	typedef struct s_file {
		char			*path;				// Path of the file
		t_ftype			type;				// Archive or elf
		void			*base;				// Pointer to the file in memory
		size_t 			size;				// Length of the file
		t_elf			*elf;				// ELF content
	} t_file;

	typedef struct s_nm {
		t_file			*file;				// Array of files
		int				file_total;			// Total of files
		t_options		options;			// Options
	} t_nm;

#pragma endregion

#pragma region "Variables"

	extern t_nm g_nm;

#pragma endregion

#pragma region "Methods"

	// Main
	int		options(int argc, char **argv);

	// Symbols
	int		load_symbols(t_elf *elf);

	// Section
	int		validate(t_elf *elf);
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
