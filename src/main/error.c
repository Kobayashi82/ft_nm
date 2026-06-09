
#pragma region "Includes"

	#include "main/nm.h"
	#include "utils/utils.h"

#pragma endregion

#pragma region "Messages"


#pragma endregion

#pragma region "Free & Exit"

	static void free_and_exit(int exit_code) {
		if (g_nm.options.argc) {
			for (int i = 0; i < g_nm.options.argc; ++i)
				free(g_nm.options.argv[i]);
			free(g_nm.options.argv);
		}

		exit(exit_code % 256);
	}

#pragma endregion

#pragma region "Exit Error"

	// Prints the error message and exits if required
	//
	// error_type   = Determines the error type (see enum values)
	// exit_code    = If 0, uses 'shell.exit_code'; otherwise set to 'exit_code'
	// value1       = Builtin name or error source
	// value2       = Argument or additional error information
	// free_mode    = Specifies whether 'value1' and/or 'value2' must be freed  (EE_FREE_NONE, EE_FREE_VAL1, EE_FREE_VAL2, EE_FREE_BOTH)
	// exit_mode    = Specifies whether to return or exit the shell             (EE_RETURN, EE_EXIT)
	//
	// Example:
	//
	// value1 ↘            ↙ value2
	//   42sh: cd: Makefile: Not a directory

	int	exit_error(int error_type, int exit_code, char *value1, char *value2, int free_mode, int exit_mode) {
		if (!value1) {
			value1 = "";
			if (free_mode == EE_FREE_VAL1) free_mode = EE_FREE_NONE;
			if (free_mode == EE_FREE_BOTH) free_mode = EE_FREE_VAL2;
		}
		if (!value2) {
			value2 = "";
			if (free_mode == EE_FREE_VAL2) free_mode = EE_FREE_NONE;
			if (free_mode == EE_FREE_BOTH) free_mode = EE_FREE_VAL1;
		}

		if (error_type >= E_NO_MEMORY && error_type < E_END) {
			ft_dprintf(STDERR_FILENO, NAME);
			if (value1 && *value1) ft_dprintf(STDERR_FILENO, ": ");
		}

		// shell_msg(error_type, value1, value2);

		if (free_mode == EE_FREE_VAL1 || free_mode == EE_FREE_BOTH)	free(value1);
		if (free_mode == EE_FREE_VAL2 || free_mode == EE_FREE_BOTH)	free(value2);

		if (exit_mode == EE_EXIT)	free_and_exit(exit_code);

		return (exit_code);
	}

#pragma endregion
