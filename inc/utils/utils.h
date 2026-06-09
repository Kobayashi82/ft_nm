
#pragma once

#pragma region "Includes"

	#include <fcntl.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#include <stdarg.h>
	#include <limits.h>
	#include <errno.h>
	#include <stdio.h>

#pragma endregion

#pragma region "Enumerators"

	enum e_join {
		J_FREE_NONE			= 290,		// Dont free any value
		J_FREE_VAL_1		= 291,		// Free 'value1'
		J_FREE_VAL_2		= 292,		// Free 'value2'
		J_FREE_VAL_3		= 293,		// Free 'value3'
		J_FREE_VAL_1_2		= 294,		// Free 'value1' and 'value2'
		J_FREE_VAL_1_3		= 295,		// Free 'value1' and 'value3'
		J_FREE_VAL_2_3		= 296,		// Free 'value2' and 'value3'
		J_FREE_VAL_1_2_3	= 297,		// Free 'value1' and 'value2' and 'value3'
	};

#pragma endregion

#pragma region "Methods"

	#pragma region "IS"

		int		ft_isspace_s(const char *str);
		int		ft_isalnum_s(const char *str);
		int		ft_isnum_s(const char *str);
		int		ft_isdigit_s(const char *str);
		char	*ft_toupper_s(char *str);
		char	*ft_tolower_s(char *str);

	#pragma endregion

	#pragma region "STRLCPY"

		int		ft_strlcat(char *dst, const char *src, int dstsize);

	#pragma endregion

	#pragma region "STRLCPY"

		int		ft_strlcpy(char *dst, const char *src, int dstsize);

	#pragma endregion

	#pragma region "STRJOIN"

		char	*ft_strjoin(const char *value1, const char *value2, int free_mode);
		char	*ft_strjoin_sep(const char *value1, const char *value2, const char *value3, int free_mode);

	#pragma endregion

	#pragma region "STRDUP"

		char	*ft_strdup(const char *s1);
		char	*ft_strndup(const char *s1, size_t n);

	#pragma endregion

	#pragma region "STRLEN"

		size_t	ft_strlen(const char *str);

	#pragma endregion

	#pragma region "MEMSET"

		void *ft_memset(void *b, int c, size_t len);

	#pragma endregion

	#pragma region "ITOA"

		char	*ft_itoa(int n);

	#pragma endregion

	#pragma region "DPRINTF"

		int		ft_dprintf(int fd, char const *format, ...);

	#pragma endregion

#pragma endregion
