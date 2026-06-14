
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

	#include <unistd.h>
	#include <stdlib.h>

#pragma endregion

#pragma region "Variables"

	t_nm g_nm;

#pragma endregion

#pragma region "Process"

	// Procesa cada archivo y sus miembros (si es un 'Archive')
	static int process(t_file *file) {
		if (!file || !file->path) return (1);

		int error = map_file(file);

		if (!error) {
			t_elf *current = file->elf;
			while (current) {
				if (!current->error) current->error = load_sections(current);
				if (!current->error) current->error = load_symbols(current);

				if (current->error == NO_MEMORY) {
					unmap_file(file);
					return (NO_MEMORY);
				}

				if (current->error && file->type == ELF) error = 1;

				current = current->next;
			}

			print_file(file);
		}

		unmap_file(file);

		return (error);
	}

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		int ret = options(argc, argv);

		if (!ret) {
			for (int i = 0; i < g_nm.file_total; ++i) {
				int error = process(&g_nm.file[i]);
				ret += error;

				if (error == NO_MEMORY) {
					ft_dprintf(STDERR_FILENO, NAME": %s: memory allocation failed\n", g_nm.file[i]);
					return (1);
				}

				free(g_nm.file[i].path);
			}
			free(g_nm.file);
		}

		return ((ret == -1) ? 0 : ret);
	}

#pragma endregion
