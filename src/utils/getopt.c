
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"
	#include "getopts.h"

	#include <unistd.h>
	#include <stdlib.h>

	#define MAX_OPTIONS	128

#pragma endregion

#pragma region "Internal"

	#pragma region "Common"

		#pragma region "Create Opt Value"

			static t_opt_value *create_opt_value(char opt, const char *value, int is_long, const char *long_name, int is_plus) {
				t_opt_value *node = malloc(sizeof(t_opt_value));
				if (!node) return (g_nm.error = E_NO_MEMORY, NULL);
				ft_memset(node, 0, sizeof(t_opt_value));

				node->opt = opt;
				node->is_plus = is_plus;
				node->value = ft_strdup(value);
				node->long_name = ft_strdup(long_name);
				if ((value && !node->value) || (long_name && !node->long_name)) {
					free(node->value);
					free(node->long_name);
					free(node);
					return (g_nm.error = E_NO_MEMORY, NULL);
				}
				node->is_long = is_long;
				node->next = NULL;

				return (node);
			}

		#pragma endregion

		#pragma region "Add Opt Value"

			static void add_opt_value(t_opt_value **list, t_opt_value *node) {
				if (!list || !node) return;

				if (!*list) {
					*list = node;
				} else {
					t_opt_value *curr = *list;
					while (curr->next) curr = curr->next;
					curr->next = node;
				}
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Long"

		#pragma region "Find Long Opt"

			static const t_long_option *find_long_opt(const t_long_option *long_opts, const char *name, size_t len) {
				if (!long_opts || !name) return (NULL);

				for (int i = 0; long_opts[i].name; ++i) {
					if (!ft_strncmp(long_opts[i].name, name, len) && long_opts[i].name[len] == '\0') return (&long_opts[i]);
				}

				return (NULL);
			}

		#pragma endregion

		#pragma region "Parse Long"

			static int parse_long_option(t_parse_result *result, char **argv, int *idx, int argc, const t_long_option *long_opts, int silent_mode, int opt_count) {
				if (opt_count >= MAX_OPTIONS) return (g_nm.error = E_OPT_MAX, 1);

				const char	*arg = argv[*idx] + 2;
				const char	*equals = ft_strchr(arg, '=');

				// Find long option
				size_t name_len = (equals) ? (size_t)(equals - arg) : ft_strlen(arg);
				const t_long_option *opt = find_long_opt(long_opts, arg, name_len);
				if (!opt) {
					if (!silent_mode) {
						ft_dprintf(STDERR_FILENO, NAME": --: invalid option\n");
						ft_dprintf(STDERR_FILENO, NAME": usage: %s\n", result->usage);
					}
					return (g_nm.error = E_OPT_INVALID, 1);
				}

				char *value = NULL;

				if (opt->has_arg == REQUIRED_ARG) {
					if (equals) {
						// --option=value
						value = ft_strdup(equals + 1);
						if (!value) return (g_nm.error = E_NO_MEMORY, 1);
					} else if (*idx + 1 < argc) {
						// --option value
						(*idx)++;
						value = ft_strdup(argv[*idx]);
						if (!value) return (g_nm.error = E_NO_MEMORY, 1);
					} else {
						// Missing Argument
						if (!silent_mode) {
							ft_dprintf(STDERR_FILENO, NAME": --%s: option requires an argument\n", opt->name);
							ft_dprintf(STDERR_FILENO, NAME": usage: %s\n", result->usage);
						}
						return (g_nm.error = E_OPT_NO_ARGUMENT, 1);
					}
				} else if (opt->has_arg == OPTIONAL_ARG) {
					if (equals) {
						// --option=value	(If there is no '=', the optional argument is not taken from the next argv)
						value = ft_strdup(equals + 1);
						if (!value) return (g_nm.error = E_NO_MEMORY, 1);
					}
				} else if (opt->has_arg == NO_ARGUMENT) {
					if (equals) {
						if (!silent_mode) {
							ft_dprintf(STDERR_FILENO, NAME": --%s: doesn't allow an argument\n", arg);
							ft_dprintf(STDERR_FILENO, NAME": usage: %s\n", result->usage);
						}
						return (g_nm.error = E_OPT_ARGUMENT, 1);
					}
				}

				t_opt_value *node = create_opt_value(opt->opt, value, 1, opt->name, 0);
				free(value);
				if (!node) return (1);

				if (node) add_opt_value(&result->options, node);

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Short"

		#pragma region "Extract UTF8 Char"

			// Returns the number of bytes in the character (1-4 for valid UTF-8)
			static int extract_utf8_char(const char *str, char *buffer, int max_len) {
				unsigned char	first = (unsigned char)str[0];
				int				len = 1;

				// UTF-8 character length from first byte
				if		((first & 0x80) == 0x00) len = 1;	// 0xxxxxxx - ASCII
				else if ((first & 0xE0) == 0xC0) len = 2;	// 110xxxxx - 2 bytes
				else if ((first & 0xF0) == 0xE0) len = 3;	// 1110xxxx - 3 bytes
				else if ((first & 0xF8) == 0xF0) len = 4;	// 11110xxx - 4 bytes

				if (len > max_len) len = max_len;

				for (int i = 0; i < len; i++) {
					if (!str[i]) break;
					buffer[i] = str[i];
				}
				buffer[len] = '\0';

				return (len);
			}

		#pragma endregion

		#pragma region "Get Short Type"

			static int get_short_type(const char *short_opts, char opt) {
				if (!short_opts) return (0);						// Not Required

				const char *pos = ft_strchr(short_opts, opt);
				if (!pos) return (-1);								// Invalid

				if (pos[1] == '.')					return (3);		// Optional (support arg separated)
				if (pos[1] == ':' && pos[2] == ':')	return (2);		// Optional
				if (pos[1] == ':')					return (1);		// Required

				return (0);											// Not Required
			}

		#pragma endregion

		#pragma region "Parse Short"

			static int parse_short_option(t_parse_result *result, char **argv, int *idx, int argc, const char *short_opts, int is_plus, int silent_mode, int ignore_mode, int opt_count) {
				const char	*arg = argv[*idx] + 1;

				for (int i = 0; arg[i]; ++i) {
					if (opt_count++ >= MAX_OPTIONS) return (g_nm.error = E_OPT_MAX, 1);										// Note: max_option error may trigger even when the option is an argument in ignore_mode = 2

					char opt = arg[i];
					int arg_type = get_short_type(short_opts, opt);

					if (ignore_mode == IGNORE_STRING) {
						int len = ft_strlen(arg);
						for (int i = 0; i < len; ++i) {
							if (get_short_type(short_opts, arg[i]) == -1) return (2);
						}
					}

					if (arg_type == -1) {
						char utf8_char[5];
						extract_utf8_char(&arg[i], utf8_char, 4);
						if (!silent_mode) {
							if (is_plus)	ft_dprintf(STDERR_FILENO, NAME": +%s: invalid option\n", utf8_char);
							else			ft_dprintf(STDERR_FILENO, NAME": -%s: invalid option\n", utf8_char);
							ft_dprintf(STDERR_FILENO, NAME": usage: %s\n", result->usage);
						}
						return (g_nm.error = E_OPT_INVALID, 1);
					}

					char *value = NULL;

					if (arg_type == REQUIRED_ARG) {
						if (arg[i + 1]) {
							// -avalue
							value = ft_strdup(arg + i + 1);
							if (!value) return (g_nm.error = E_NO_MEMORY, 1);
							i = ft_strlen(arg) - 1;
						} else if (*idx + 1 < argc) {
							// -a value
							(*idx)++;
							value = ft_strdup(argv[*idx]);
							if (!value) return (g_nm.error = E_NO_MEMORY, 1);
						} else {
							// Missing Argument
							if (!silent_mode) {
								char buff[2] = {opt, '\0'};
								ft_dprintf(STDERR_FILENO, NAME": -%s: option requires an argument\n", buff);
								ft_dprintf(STDERR_FILENO, NAME": usage: %s\n", result->usage);
							}
							return (g_nm.error = E_OPT_NO_ARGUMENT, 1);
						}
					} else if (arg_type == OPTIONAL_ARG) {
						if (arg[i + 1]) {
							// -avalue		(Don't take the next argv for optional arguments)
							value = ft_strdup(arg + i + 1);
							if (!value) return (g_nm.error = E_NO_MEMORY, 1);
							i = ft_strlen(arg) - 1;
						}
					} else if (arg_type == OPTIONAL_ARG_SEP) {
						if (arg[i + 1]) {
							// -avalue
							value = ft_strdup(arg + i + 1);
							if (!value) return (g_nm.error = E_NO_MEMORY, 1);
							i = ft_strlen(arg) - 1;
						} else if (*idx + 1 < argc) {
							char *next = argv[*idx + 1];
							if (ft_strcmp(next, "--") && ft_strcmp(next, "-") && ft_strcmp(next, "+") && next[0] != '-' && next[0] != '+') {
								// -a value
								(*idx)++;
								value = ft_strdup(argv[*idx]);
								if (!value) return (g_nm.error = E_NO_MEMORY, 1);
							}
						}
					}

					t_opt_value *node = create_opt_value(opt, value, 0, NULL, is_plus);
					free(value);
					if (!node) return (1);
					add_opt_value(&result->options, node);
				}

				return (0);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region "Methods"

	#pragma region "Long"

		#pragma region "Find Long Option"

			t_opt_value *find_long_option(t_parse_result *result, const char *name) {
				if (!result || !name) return (NULL);

				for (t_opt_value *curr = result->options; curr; curr = curr->next) {
					if (curr->is_long && curr->long_name && !ft_strcmp(curr->long_name, name)) return (curr);
				}

				return (NULL);
			}

		#pragma endregion

		#pragma region "Has Long Option"

			int has_long_option(t_parse_result *result, const char *name) {
				if (!result || !name) return (0);

				for (t_opt_value *curr = result->options; curr; curr = curr->next) {
					if (curr->is_long && curr->long_name && !ft_strcmp(curr->long_name, name)) return (1);
				}

				return (0);
			}

		#pragma endregion

		#pragma region "Get Long Option Value"

			const char *get_long_option_value(t_parse_result *result, const char *name) {
				t_opt_value *node = find_long_option(result, name);
				if (!node) return (NULL);

				return (node->value);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Short"

		#pragma region "Find Option"

			t_opt_value *find_option(t_parse_result *result, char opt, int is_plus) {
				if (!result) return (NULL);

				t_opt_value *found = NULL;
				for (t_opt_value *curr = result->options; curr; curr = curr->next) {
					if (curr->opt == opt && curr->is_plus == is_plus) found = curr;
				}

				return (found);
			}

		#pragma endregion

		#pragma region "Has Option"

			int has_option(t_parse_result *result, char opt, int is_plus) {
				return (find_option(result, opt, is_plus) != NULL);
			}

		#pragma endregion

		#pragma region "Get Option Value"

			const char *get_option_value(t_parse_result *result, char opt, int is_plus) {
				t_opt_value *node = find_option(result, opt, is_plus);
				if (!node) return (NULL);

				return (node->value);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Parse Options"

		t_parse_result *parse_options(int argc, char **argv, const char *short_opts, const char *short_opts_plus, const t_long_option *long_opts, const char *usage, int ignore_mode) {
			if (!argv || argc < 1) return (NULL);

			g_nm.error = 0;
			t_parse_result *result = malloc(sizeof(t_parse_result));
			if (!result) {
				ft_dprintf(STDERR_FILENO, NAME": memory allocation failed\n");
				return (g_nm.error = E_NO_MEMORY, NULL);
			}
			ft_memset(result, 0, sizeof(t_parse_result));

			char **tmp_args = malloc(argc * sizeof(char*));
			if (!tmp_args) {
				ft_dprintf(STDERR_FILENO, NAME": memory allocation failed\n");
				free_options(result);
				return (g_nm.error = E_NO_MEMORY, NULL);
			}
			ft_memset(tmp_args, 0, argc * sizeof(char*));

			int opt_count = 0;
			int arg_count = 0;
			int done_with_opts = 0;
			int silent_mode = 0;
			int silent_mode_plus = 0;

			if (short_opts && *short_opts == ':') {
				silent_mode = 1;
				short_opts++;
			}

			if (short_opts_plus && *short_opts_plus == ':') {
				silent_mode_plus = 1;
				short_opts_plus++;
			}

			result->argv_original = argv;
			result->argc_original = argc;
			result->usage = usage;
			for (int i = 1; i < argc; i++) {
				if (done_with_opts) {
					tmp_args[arg_count] = ft_strdup(argv[i]);
					if (!tmp_args[arg_count++]) { g_nm.error = E_NO_MEMORY; break; }
					continue;
				}

				const char *arg = argv[i];

				// End of options '--'
				if (!ft_strcmp(arg, "--")) {
					result->double_dash = 1;
					done_with_opts = 1;
					continue;
				}

				// Ignore -number or +number if ignore_numbers is active
				if (ignore_mode == IGNORE_NUMBER && (arg[0] == '-' || arg[0] == '+') && arg[1] != '\0' && ft_isnum_s(&arg[1])) {
					tmp_args[arg_count] = ft_strdup(arg);
					if (!tmp_args[arg_count++]) { g_nm.error = E_NO_MEMORY; break; }
					done_with_opts = 1;
					continue;
				}

				// Long options (--...)
				if (arg[0] == '-' && arg[1] == '-' && long_opts) {
					if (parse_long_option(result, argv, &i, argc, long_opts, silent_mode, opt_count++) == 1) break;
					continue;
				}

				// Options with '-'
				if (arg[0] == '-' && short_opts && arg[1]) {
					int ret = parse_short_option(result, argv, &i, argc, short_opts, 0, silent_mode, ignore_mode, opt_count);
					if (ret == 0) opt_count++;
					if (ret == 1) break;
					if (ret == 2) {
						tmp_args[arg_count] = ft_strdup(arg);
						if (!tmp_args[arg_count++]) { g_nm.error = E_NO_MEMORY; break; }
						done_with_opts = 1;
					}
					continue;
				}

				// Options with '+'
				if (arg[0] == '+' && short_opts_plus && arg[1]) {
					int ret = parse_short_option(result, argv, &i, argc, short_opts_plus, 1, silent_mode_plus, ignore_mode, opt_count);
					if (ret == 0) opt_count++;
					if (ret == 1) break;
					if (ret == 2) {
						tmp_args[arg_count] = ft_strdup(arg);
						if (!tmp_args[arg_count++]) { g_nm.error = E_NO_MEMORY; break; }
						done_with_opts = 1;
					}
					continue;
				}

				// Argument
				tmp_args[arg_count] = ft_strdup(arg);
				if (!tmp_args[arg_count++]) { g_nm.error = E_NO_MEMORY; break; }
				done_with_opts = 1;
			}

			result->argc = arg_count;
			if (!g_nm.error && arg_count > 0) {
				result->argv = malloc((arg_count + 1) * sizeof(char*));
				if (!result->argv) g_nm.error = E_NO_MEMORY;
				else {
					for (int i = 0; i < arg_count; i++)
						result->argv[i] = tmp_args[i];
					result->argv[arg_count] = NULL;
				}
			}

			if (g_nm.error) {
				if (g_nm.error == E_NO_MEMORY)	ft_dprintf(STDERR_FILENO, NAME": memory allocation failed\n");
				if (g_nm.error == E_OPT_MAX)	ft_dprintf(STDERR_FILENO, NAME": Too many options\n");
				if (tmp_args) {
					for (int i = 0; tmp_args[i]; ++i)
						free(tmp_args[i]);
					free(tmp_args);
				}
				free_options(result);
				return (NULL);
			}

			free(tmp_args);
			return (result);
		}

	#pragma endregion

	#pragma region "Free Options"

		void free_argv_original(t_parse_result *result) {
			if (!result) return;

			for (int i = 0; i < result->argc_original; ++i) {
				free(result->argv_original[i]);
			}

			free(result->argv_original);
			result->argv_original = NULL;
			result->argc_original = 0;
		}

		void free_options(t_parse_result *result) {
			if (!result) return;

			while (result->options) {
				t_opt_value *next = result->options->next;
				free(result->options->value);
				free(result->options->long_name);
				free(result->options);
				result->options = next;
			}

			if (result->argv) {
				for (int i = 0; i < result->argc; i++)
					free(result->argv[i]);
				free(result->argv);
			}

			free(result);
		}

	#pragma endregion

#pragma endregion
