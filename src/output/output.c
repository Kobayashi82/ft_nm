
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

	#include <unistd.h>

#pragma endregion

#pragma region "Is Undefined"

	// Comprueba si el símbolo no está definido
	static int is_undefined(t_sym *s) {
		return (s->type == 'U' || s->type == 'u' || s->type == 'w' || s->type == 'v');
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

		if (file->type == ARCHIVE || (file->elf && g_nm.file_total > 1 && !file->elf->error)) {
			ft_dprintf(STDOUT_FILENO, "\n%s\n", file->path);
		}

		t_elf *member = file->elf;
		while (member) {
			if (!member->error) {
				if (!g_nm.options.no_sort)		sort(member->symbol, g_nm.options.numeric_sort);
				if (g_nm.options.reverse_sort)	reverse(member->symbol);

				t_sym *symbol = member->symbol;
				while (symbol) {
					if (g_nm.options.print_filename)	ft_dprintf(STDOUT_FILENO, "%s:%s:", file->path, member->name);
					else								ft_dprintf(STDOUT_FILENO, "\n%s:\n", member->name);
					
					if (is_undefined(member->symbol)) {
						if (member->bits == 32)			ft_dprintf(STDOUT_FILENO, "        ");
						if (member->bits == 64)			ft_dprintf(STDOUT_FILENO, "                ");
					} else								print_address(member->symbol->value, member->bits);
					
					ft_dprintf(STDOUT_FILENO, " %c ", member->symbol->type);
					ft_dprintf(STDOUT_FILENO, "%s\n", member->symbol->name);

					symbol = symbol->next;
				}
			} else {
				switch (member->error) {
					case TRUNCATED:		{ ft_dprintf(STDOUT_FILENO, NAME": %s: file truncated\n", member->name);				break; }
					case UNRECOGNIZED:	{ ft_dprintf(STDOUT_FILENO, NAME": %s: file format not recognized\n", member->name);	break; }
					default:			{ ft_dprintf(STDOUT_FILENO, NAME": %s: undefined error\n", member->name);				break; }
				}
			}

			member = member->next;
		}

		return (0);
	}

#pragma endregion
