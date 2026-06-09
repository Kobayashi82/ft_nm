
#pragma region "Includes"

	#include "main/nm.h"
	#include "utils/utils.h"

#pragma endregion

#pragma region "Variables"

	t_nm g_nm;

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		int ret = options(argc, argv);
		if (ret) return (ret == E_END) ? 0 : ret;

		return (exit_error(E_END, ret, NULL, NULL, EE_FREE_NONE, EE_EXIT));
	}

#pragma endregion
