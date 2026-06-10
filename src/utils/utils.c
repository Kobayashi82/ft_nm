
#pragma region "Includes"

	#include "utils.h"

	#include <stdlib.h>

#pragma endregion

#pragma region "IS"

	int ft_isdigit(int c) {
		return (c >= '0' && c <= '9');
	}

	int	ft_isnum_s(const char *str) {
		if (!str) return (0);

		while (*str) {
			if (!ft_isdigit(*str++)) return (0);
		}

		return (1);
	}

#pragma endregion

#pragma region "STRLEN"

	size_t ft_strlen(const char *str) {
		size_t i = 0;

		while (str && str[i]) ++i;
		return (i);
	}

#pragma endregion

#pragma region "STRNCMP"

	int ft_strcmp(const char *s1, const char *s2) {
		if (!s1 || !s2) return (-1);

		while (*s1 && (*s1 == *s2)) {
			s1++;
			s2++;
		}

		return (*(unsigned char *)s1 - *(unsigned char *)s2);
	}

	int ft_strncmp(const char *s1, const char *s2, size_t n) {
		unsigned char	*str1 = (unsigned char *)s1;
		unsigned char	*str2 = (unsigned char *)s2;
		size_t			i = 0;

		while (*str1 && (*str1 == *str2) && i < n && n > 0) {
			++str1;
			++str2;
			++i;
		}

		if ((!*str1 && !*str2) || i == n) return (0);
		return (*str1 - *str2);
	}

#pragma endregion

#pragma region "STRCHR"

	char *ft_strchr(const char *s, int c) {
		while (*s) {
			if (*s == (char)c) break ;
			s++;
		}

		if (*s == (char)c) return ((char *)s);
		return (NULL);
	}

#pragma endregion

#pragma region "STRDUP"

	char *ft_strdup(const char *s1) {
		if (!s1) return (NULL);

		size_t len = ft_strlen(s1);
		char *copy = malloc(len + 1);
		if (!copy) return (NULL);

		ft_memcpy(copy, s1, len);
		copy[len] = '\0';
		return (copy);
	}

#pragma endregion

#pragma region "MEMCPY"

	void *ft_memcpy(void *dst, const void *src, size_t n) {
		unsigned char		*d = (unsigned char *)dst;
		const unsigned char	*s = (const unsigned char *)src;

		if (!d && !s && n)	return (dst);
		if (!n) 			return (dst);
		while (n-- > 0)		*d++ = *s++;

		return (dst);
	}

#pragma endregion

#pragma region "MEMSET"

	void *ft_memset(void *b, int c, size_t len) {
		unsigned char *p = b;

		while (len--)
			*p++ = (unsigned char)c;
		return (b);
	}

#pragma endregion
