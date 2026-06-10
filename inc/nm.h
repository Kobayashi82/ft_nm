
#pragma once

#pragma region "Includes"

	#include <stddef.h>

#pragma endregion

#pragma region "Enumerators"

	enum e_error {
		NOTHING					= 0,		// No error
		E_NO_MEMORY				= 990,		// No more memory available
		E_END					= 997,		// End the program
		// exit_error
		EE_FREE_NONE			= 290,		// Dont free any value
		EE_FREE_VAL1			= 291,		// Free 'value1'
		EE_FREE_VAL2			= 292,		// Free 'value2'
		EE_FREE_BOTH			= 294,		// Free 'value1' and 'value2'
		EE_RETURN				= 298,		// The shell must continue
		EE_EXIT					= 299		// The shell must exit
	};

#pragma endregion

#pragma region "Structures"

	typedef struct s_options {
		int		debug_syms;    				// -a
		int		print_filename;				// -A
		int		dynamic;       				// -D
		int		extern_only;   				// -g
		int		numeric_sort;  				// -n
		int		no_sort;       				// -p
		int		reverse_sort;  				// -r
		int		undefined_only;				// -u
		int		defined_only;  				// -U
		int		no_weak;       				// -W
	} t_options;

	typedef struct s_elf {
		char	*file;						// 
		int		fd;							// 
		size_t 	size;						// 
		void	*map;						// 
		int		bits;						// x32 / x64
	} t_elf;

	typedef struct s_nm {
		t_elf		*elf;					// 
		int			elf_total;				// 
		int			error;					// Error type
		t_options	options;				// Options
	} t_nm;

#pragma endregion

#pragma region "Variables"

	extern t_nm g_nm;

#pragma endregion

#pragma region "Methods"

	int	options(int argc, char **argv);

	int	load_file(t_elf *elf);

#pragma endregion
