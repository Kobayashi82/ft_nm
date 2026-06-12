
#pragma once

#pragma region "Includes"

	#include <stddef.h>

#pragma endregion

#pragma region "Methods"

	#pragma region "IS"

		int		ft_isdigit(int c);
		int		ft_isnum_s(const char *str);

	#pragma endregion

	#pragma region "STRLEN"

		size_t	ft_strlen(const char *str);

	#pragma endregion

	#pragma region "STRCMP"

		int		ft_strcmp(const char *s1, const char *s2);
		int		ft_strncmp(const char *s1, const char *s2, size_t n);

	#pragma endregion
	
	#pragma region "STRCHR"

		char	*ft_strchr(const char *s, int c);

	#pragma endregion

	#pragma region "STRDUP"

		char	*ft_strdup(const char *s1);
		char	*ft_strndup(const char *s1, size_t n);

	#pragma endregion

	#pragma region "MEMCPY"

		void	*ft_memcpy(void *dst, const void *src, size_t n);

	#pragma endregion

	#pragma region "MEMSET"

		void	*ft_memset(void *b, int c, size_t len);

	#pragma endregion
	
	#pragma region "ATOI"

		int		ft_atoi(const char *str);

	#pragma endregion

	#pragma region "DPRINTF"

		int		ft_dprintf(int fd, char const *format, ...);

	#pragma endregion

#pragma endregion
