
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

	#include <stdlib.h>

#pragma endregion

#pragma region "Variables"

	t_nm g_nm;

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		int ret = options(argc, argv);

		if (!ret) {
			for (int i = 0; i < g_nm.elf_total; ++i) {
				int error = 0;
				error = load_file(&g_nm.elf[i]);
				// if (!error) algo();
				// if (!error) algo();
				if (error) ret++;
				free(g_nm.elf[i].file);
			}
			free(g_nm.elf);
		}

		return (ret);
	}

#pragma endregion
