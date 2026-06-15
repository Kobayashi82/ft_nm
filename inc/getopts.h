
#pragma once

#pragma region "Includes"

	#include <stddef.h>

#pragma endregion

#pragma region "Enumerators"

	enum e_getopt {
		NO_ARGUMENT,						// La opción no lleva ningún argumento
		REQUIRED_ARG,						// El argumento es obligatorio (ej. -o valor o -ovalor)
		OPTIONAL_ARG,						// El argumento es opcional, pero debe ir pegado a la letra (ej. -ovalor)
		OPTIONAL_ARG_SEP,					// El argumento es opcional y puede ir separado por espacio si lo siguiente no es otra opción
		IGNORE_OFF,							// Desactiva el filtrado (comportamiento normal del parseador)
		IGNORE_NUMBER,						// Ignora los números con signo para que no se confundan con opciones (ej. -10 +10)
		IGNORE_STRING,						// Ignora cadenas de texto completas si el parseador detecta que no son opciones válidas

		E_OPT_MAX				= 300,		// Se superó el número máximo de opciones
		E_OPT_INVALID			= 301,		// Opción inválida
		E_OPT_NO_ARGUMENT		= 302,		// Falta un argumento obligatorio para la opción
		E_OPT_ARGUMENT			= 303,		// Argumento inesperado para la opción
		E_NO_MEMORY				= 990,		// No hay memoria disponible
	};

#pragma endregion

#pragma region "Structures"

	typedef struct s_long_option {
		const char			*name;				// Nombre de la opción larga
		int					has_arg;			// NO_ARG, REQUIRED_ARG, OPTIONAL_ARG
		char				opt;				// Opción corta (0 si no tiene)
	} t_long_option;

	typedef struct s_opt_value {
		char				*long_name;			// Nombre de la opción larga (NULL si es opción corta)
		int					is_long;			// 1 si es opción larga, 0 si es opción corta
		char				opt;				// Carácter de la opción
		char				*value;				// Valor de la opción (NULL si no tiene)
		int					is_plus;			// 1 si la opción usa el prefijo '+', 0 si usa '-'
		struct s_opt_value	*next;
	} t_opt_value;

	typedef struct s_parse_result {
		t_opt_value			*options;			// Lista de opciones parseadas
		t_opt_value			*plus_options;		// Lista de opciones '+' parseadas
		int					double_dash;		// Indica si se proporcionó '--' (fin de las opciones)
		char				**argv_original;	// El argv original
		int					argc_original;		// El argc original
		int					argc;				// Número de argumentos que no son opciones
		char				**argv;				// Argumentos que no son opciones
		const char			*name;				// Nombre del comando que llama a getopt
		const char			*usage;				// Sintaxis de uso del comando que realiza la llamada
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
