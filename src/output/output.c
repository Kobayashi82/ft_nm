
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

	#include <unistd.h>

#pragma endregion

#pragma region "Filter"

	// Filtra la informacion que se debe imprimir
	static int filter(t_sym *sym) {
		// -g: only external (uppercase = global)
		if (g_nm.options.extern_only) {
			// Si es un símbolo de debug ('a' o 'N'), el flag -g SIEMPRE lo oculta
			if (sym->type == 'a' || sym->type == 'N') return (0);
			if (ft_islower(sym->type) && sym->type != 'w' && sym->type != 'i') return (0);
		}

		// -u: only undefined
		if (g_nm.options.undefined_only && sym->type != 'U' && sym->type != 'w') return (0);

		// -U: only defined
		if (g_nm.options.defined_only && (sym->type == 'U' || sym->type == 'w')) return (0);

		// -W: ignore weak
		if (g_nm.options.no_weak) {
			// Si no está -g, se ocultan todos los weak (indefinidos y definidos)
			if (!g_nm.options.extern_only) {
				if (sym->type == 'w' || sym->type == 'W' || sym->type == 'v' || sym->type == 'V') return (0);
			} 
			// Si está -g, este tiene prioridad sobre las versiones globales de weak
			// Solo se ocultan los INDEFINIDOS ('w' y 'v')
			else if (sym->type == 'w' || sym->type == 'v') return (0);
		}

		// -a: debug symbols (N type). Without -a, filter them out
		if (!g_nm.options.debug_syms) {
    		if (sym->type == 'f' || sym->type == 'N' || !*sym->name || sym->is_section) return (0);
		}

		return (1);
	}

#pragma endregion

#pragma region "Print Address"

	// Imprime la direccion de memoria
	static void print_address(uint64_t value, int bits) {
		int		width = (bits == 32) ? 8 : 16;
		char	buffer[17];

		buffer[width] = '\0';

		for (int i = width - 1; i >= 0; --i){
			buffer[i] = "0123456789abcdef"[value & 0xf];
			value >>= 4;
		}

		write(STDOUT_FILENO, buffer, width);
	}

#pragma endregion

#pragma region "Print File"

	// Imprime los simbolos
	int print_file(t_file *file) {
		if (!file) return (1);

		if (file->type == ARCHIVE && g_nm.file_total > 1) {
			ft_dprintf(STDOUT_FILENO, "\n%s:\n", file->path);
		}

		t_elf *member = file->elf;
		while (member) {
			if (member->sym == NULL && !member->error) member->error = NO_SYMBOLS;

			// Cabecera del miembro, fuera del bucle de simbolos
			if (!member->error || member->error == NO_SYMBOLS) {
				if ((file->type == ARCHIVE || g_nm.file_total > 1) && !g_nm.options.print_filename)
					ft_dprintf(STDOUT_FILENO, "\n%s:\n", member->name);
			}

			if (!member->error) {
				if (!g_nm.options.no_sort) member->sym = sort(member->sym, g_nm.options.numeric_sort, g_nm.options.reverse_sort);;

				t_sym *symbol = member->sym;
				while (symbol) {
					if (filter(symbol)) {
						if (g_nm.options.print_filename) {
							if (file->type == ARCHIVE)	ft_dprintf(STDOUT_FILENO, "%s:%s:", file->path, member->name);
							else						ft_dprintf(STDOUT_FILENO, "%s:", file->path);
						}

						// else								ft_dprintf(STDOUT_FILENO, "\n%s:\n", member->name);
						
						if (is_undefined(symbol)) {
							if (member->bits == 32)			ft_dprintf(STDOUT_FILENO, "        ");
							if (member->bits == 64)			ft_dprintf(STDOUT_FILENO, "                ");
						} else								print_address(symbol->value, member->bits);
						
						ft_dprintf(STDOUT_FILENO, " %c ", (symbol->type == 'f') ? 'a' : symbol->type);
						ft_dprintf(STDOUT_FILENO, "%s\n", symbol->name);
					}

					symbol = symbol->next;
				}
			} else {
				switch (member->error) {
					// case TRUNCATED:		{ ft_dprintf(STDERR_FILENO, NAME": %s: file truncated\n",				member->name);	break; }
					case TRUNCATED:		{ ft_dprintf(STDERR_FILENO, NAME": %s: file format not recognized\n",	member->name);	break; }
					case UNRECOGNIZED:	{ ft_dprintf(STDERR_FILENO, NAME": %s: file format not recognized\n",	member->name);	break; }
					case NO_SYMBOLS:	{ ft_dprintf(STDERR_FILENO, NAME": %s: no symbols\n",					member->name);	break; }
					default:			{ ft_dprintf(STDERR_FILENO, NAME": %s: undefined error\n",				member->name);	break; }
				}
			}

			member = member->next;
		}

		return (0);
	}

#pragma endregion
