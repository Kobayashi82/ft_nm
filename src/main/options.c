
#pragma region "Includes"

	#include "main/nm.h"
	#include "utils/utils.h"
	#include "utils/getopt.h"

#pragma endregion

#pragma region "Help"

	static int help() {
		char *msg = 
			"Usage: nm [option(s)] [file(s)]\n"
			" List symbols in [file(s)] (a.out by default).\n\n"
			" The options are:\n"
			"  -a, --debug-syms       Display debugger-only symbols\n"
			"  -A, --print-file-name  Print name of the input file before every symbol\n"
			"  -D, --dynamic          Display dynamic symbols instead of normal symbols\n"
			"  -g, --extern-only      Display only external symbols\n"
			"  -n, --numeric-sort     Sort symbols numerically by address\n"
			"  -p, --no-sort          Do not sort the symbols\n"
			"  -r, --reverse-sort     Reverse the sense of the sort\n"
			"  -u, --undefined-only   Display only undefined symbols\n"
			"  -U, --defined-only     Display only defined symbols\n"
			"  -W, --no-weak          Ignore weak symbols\n"
			"  -h, --help             Display this information\n"
			"  -V, --version          Display this program's version number\n\n"

			"nm: supported targets: elf64-x86-64 elf32-i386\n";

		ft_dprintf(STDOUT_FILENO, "%s", msg);

		return (E_END);
	}

#pragma endregion

#pragma region "Version"

	static int version() {
		char *msg =
			"ft_nm 1.0.\n"
			"Copyright (C) 2026 Kobayashi Corp ⓒ.\n"
			"This is free software: you are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n\n"

			"Written by Kobayashi82 (vzurera-).\n";

		ft_dprintf(STDOUT_FILENO, "%s", msg);

		return (E_END);
	}

#pragma endregion

#pragma region "Options"

	int options(int argc, char **argv) {
		t_long_option long_opts[] = {
			{"help",	NO_ARGUMENT, 'h'},
			{"version",	NO_ARGUMENT, 'v'},
			{NULL, 0, 0}
		};

		t_parse_result *result = parse_options(argc, argv, "aADgnpruUWhv", NULL, long_opts, "ft_nm [option(s)] [file(s)]", IGNORE_OFF);
		if (!result) return (free_options(result), (g_nm.error == E_OPT_MAX || g_nm.error == E_OPT_INVALID) ? 2 : 1);

		if (find_long_option(result, "help")	|| has_option(result, 'h', 0))	return (free_options(result), help());
		if (find_long_option(result, "version")	|| has_option(result, 'v', 0))	return (free_options(result), version());

		int ret = 0;

		if (result->argc) {
			g_nm.options.argc = result->argc;
			g_nm.options.argv = result->argv;
			result->argc = 0;
			result->argv = NULL;
		}

		g_nm.options.debug_syms		= has_option(result, 'a', 0);
		g_nm.options.print_filename	= has_option(result, 'A', 0);
		g_nm.options.dynamic		= has_option(result, 'D', 0);
		g_nm.options.extern_only	= has_option(result, 'g', 0);
		g_nm.options.numeric_sort	= has_option(result, 'n', 0);
		g_nm.options.no_sort		= has_option(result, 'p', 0);
		g_nm.options.reverse_sort	= has_option(result, 'r', 0);
		g_nm.options.undefined_only	= has_option(result, 'u', 0);
		g_nm.options.defined_only	= has_option(result, 'U', 0);
		g_nm.options.no_weak		= has_option(result, 'W', 0);

		return (free_options(result), ret);
	}

#pragma endregion
