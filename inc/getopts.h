
#pragma once

#pragma region "Includes"

	#include <stddef.h>

#pragma endregion

#pragma region "Enumerators"

	enum e_getopt {
		NO_ARGUMENT,						// 
		REQUIRED_ARG,						// 
		OPTIONAL_ARG,						// 
		OPTIONAL_ARG_SEP,					// 
		IGNORE_OFF,							// 
		IGNORE_NUMBER,						// 
		IGNORE_STRING,						// 
		// error
		E_OPT_MAX				= 300,		// Maximum number of options exceeded
		E_OPT_INVALID			= 301,		// Invalid option
		E_OPT_NO_ARGUMENT		= 302,		// Missing required argument for option
		E_OPT_ARGUMENT			= 303,		// Unexpected argument for option
		E_SOPT_INVALID			= 304,		// Invalid shell option
		E_NO_MEMORY				= 990,		// No memory available
	};

#pragma endregion

#pragma region "Structures"

	typedef struct s_long_option {
		const char			*name;				// Long option name
		int					has_arg;			// NO_ARG, REQUIRED_ARG, OPTIONAL_ARG
		char				opt;				// Short option (0 if none)
	} t_long_option;

	typedef struct s_opt_value {
		char				*long_name;			// Long option name (NULL if short option)
		int					is_long;			// 1 if long option, 0 if short option
		char				opt;				// Option character
		char				*value;				// Option value (NULL if none)
		int					is_plus;			// 1 if option uses '+' prefix, 0 if uses '-'
		struct s_opt_value	*next;
	} t_opt_value;

	typedef struct s_parse_result {
		t_opt_value			*options;			// List of parsed options
		t_opt_value			*plus_options;		// List of parsed '+' options
		int					double_dash;		// Indicates if '--' was provided (end of options)
		char				**argv_original;	// Original argv
		int					argc_original;		// Original argc
		int					argc;				// Number of non-option arguments
		char				**argv;				// Non-option arguments
		const char			*name;				// Name of the command calling getopt
		const char			*usage;				// Usage syntax of the calling command
	} t_parse_result;

#pragma endregion

#pragma region "Methods"

	// Long
	t_opt_value     *find_long_option(t_parse_result *result, const char *name);
	int             has_long_option(t_parse_result *result, const char *name);
	const char      *get_long_option_value(t_parse_result *result, const char *name);

	// Short
	t_opt_value     *find_option(t_parse_result *result, char opt, int is_plus);
	int             has_option(t_parse_result *result, char opt, int is_plus);
	const char      *get_option_value(t_parse_result *result, char opt, int is_plus);

	// Parsing
	t_parse_result *parse_options(int argc, char **argv, const char *short_opts, const char *short_opts_plus, const t_long_option *long_opts, const char *usage, int ignore_mode);

	// Free
	void			free_argv_original(t_parse_result *result);
	void            free_options(t_parse_result *result);

#pragma endregion
