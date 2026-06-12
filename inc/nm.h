
#pragma once

#pragma region "Includes"

	#include <stddef.h>
	#include <stdint.h>
	
	#define NAME "ft_nm"

#pragma endregion

#pragma region "Enumerators"

	enum s_error	{ TRUNCATED = 10, UNRECOGNIZED = 11 };
	enum e_filetype	{ ELF, ARCHIVE };

#pragma endregion

#pragma region "Structures"

	typedef struct s_options {
		int				debug_syms;    		// -a - descripción
		int				print_filename;		// -A - descripción
		int				dynamic;       		// -D - descripción
		int				extern_only;   		// -g - descripción
		int				numeric_sort;  		// -n - descripción
		int				no_sort;       		// -p - descripción
		int				reverse_sort;  		// -r - descripción
		int				undefined_only;		// -u - descripción
		int				defined_only;  		// -U - descripción
		int				no_weak;       		// -W - descripción
	} t_options;

	typedef struct s_sym {
		char			type;				// Symbol type
		char			*name;				// Symbol name
		uint64_t		value;				// Symbol pointer
		struct s_sym	*next;				// Next symbol
	} t_sym;

	typedef struct s_elf {
		char			*name;				// Member name
		int				bits;				// Architecture (32/64 bits)
		uint16_t		type;				// Type of member
		void			*base;				// Pointer to the member in memory
		size_t 			size;				// Length of the member
		int				error;				// Error for the member
		t_sym			*symbol;			// 
		struct s_elf	*next;				// Next member
	} t_elf;

	typedef struct s_file {
		char			*path;				// Path of the file
		int				type;				// Binary / .o / .so / .a
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

	// Section
	int		validate(t_elf *elf, const char *file);

	// File
	int		is_archive(void *base, size_t size);
	int		parse_archive(t_file *file);
	int		map_file(t_file *file);
	int		unmap_file(t_file *file);

	// Output
	t_sym	*reverse(t_sym *head);
	t_sym	*sort(t_sym *head, int by_value);
	int		print_file(t_file *file);

#pragma endregion
