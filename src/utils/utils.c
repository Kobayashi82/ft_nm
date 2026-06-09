
#pragma region "Includes"

	#include "utils/utils.h"

#pragma endregion

#pragma region "IS"

	int	ft_isspace_s(const char *str) { while (str && *str) { if (!isspace(*str++)) return (0); } return (1); }
	int	ft_isalnum_s(const char *str) { while (str && *str) { if (!isalnum(*str++)) return (0); } return (1); }

	int	ft_isnum_s(const char *str) {
		if (!str) return (0);

		while (*str) {
			if (!isdigit(*str++)) return (0);
		}

		return (1);
	}

	int	ft_isdigit_s(const char *str) {
		if (str && (*str == '+' || *str == '-')) str++;
		if (!str || *str == '\0') return (0);
		while (*str) {
			if (!isdigit(*str++)) return (0);
		}

		return (1);
	}

	char *ft_toupper_s(char *str) {
		int i = -1;

		while (str[++i]) if (str[i] >= 'a' && str[i] <= 'z') str[i] -= 32;
		return (str);
	}

	char *ft_tolower_s(char *str) {
		int	i = -1;

		while (str[++i]) { if (str[i] >= 'A' && str[i] <= 'Z') str[i] += 32; }
		return (str);
	}

#pragma endregion

#pragma region "STRCAT"

	#pragma region "StrLCat"

		int	ft_strlcat(char *dst, const char *src, int dstsize) {
			int src_len = 0, dest_len = 0, i;

			while (dst[dest_len] && dest_len < dstsize) dest_len++;
			while (src[src_len]) src_len++;
			if (dstsize == 0 || dest_len >= dstsize) return (dest_len + src_len);
			i = dest_len - 1;
			while (++i < dstsize - 1 && src[i - dest_len]) dst[i] = src[i - dest_len];
			dst[i] = '\0';
			return (dest_len + src_len);
		}

	#pragma endregion

	#pragma region "StrLCpy"

		int	ft_strlcpy(char *dst, const char *src, int dstsize) {
			int		i = 0, srclen = 0;

			if (!src) return (0);
			while (src[srclen]) ++srclen;
			if (dstsize > 0) {
				while (dstsize > 0 && src[i] && i < dstsize - 1) { dst[i] = src[i]; ++i; }
				dst[i] = '\0';
			}

			return (srclen);
		}

	#pragma endregion

#pragma endregion

#pragma region "STRJOIN"

	#pragma region "StrJoin"

		char *ft_strjoin(const char *value1, const char *value2, int free_mode) {
			if (!value1 && !value2) return (NULL);

			int len = ft_strlen(value1) + ft_strlen(value2) + 1;
			char *new_str = malloc(len);
			if (new_str) {
				if (value1) {
					ft_strlcpy(new_str, value1, len);
					if (value2) ft_strlcat(new_str, value2, len);
				} else if (value2) {
					strcpy(new_str, value2);
				}
			}

			if (free_mode == J_FREE_VAL_1 || free_mode == J_FREE_VAL_1_2) free((void *)value1);
			if (free_mode == J_FREE_VAL_2 || free_mode == J_FREE_VAL_1_2) free((void *)value2);

			return (new_str);
		}

	#pragma endregion

	#pragma region "StrJoinSep"

		char *ft_strjoin_sep(const char *value1, const char *value2, const char *value3, int free_mode) {
			int	len = ft_strlen(value1) + ft_strlen(value2) + ft_strlen(value3);

			if (!value1 && !value2 && !value3) return (NULL);

			char *new_str = malloc(len + 1);
			if (new_str) {
				if (value1) {
					ft_strlcpy(new_str, value1, len + 1);
					if (value2)  ft_strlcat(new_str, value2, len + 1);
					if (value3) ft_strlcat(new_str, value3, len + 1);
				} else if (value2) {
					ft_strlcpy(new_str, value2, len + 1);
					if (value3) ft_strlcat(new_str, value3, len + 1);
				} else if (value3) {
					ft_strlcpy(new_str, value3, len + 1);
				}
			}

			if (value1 && (free_mode == J_FREE_VAL_1 || free_mode == J_FREE_VAL_1_2 || free_mode == J_FREE_VAL_1_3 || free_mode == J_FREE_VAL_1_2_3)) free((void *)value1);
			if (value2 && (free_mode == J_FREE_VAL_2 || free_mode == J_FREE_VAL_1_2 || free_mode == J_FREE_VAL_2_3 || free_mode == J_FREE_VAL_1_2_3)) free((void *)value2);
			if (value3 && (free_mode == J_FREE_VAL_3 || free_mode == J_FREE_VAL_1_3 || free_mode == J_FREE_VAL_2_3 || free_mode == J_FREE_VAL_1_2_3)) free((void *)value3);

			return (new_str);
		}

	#pragma endregion

#pragma endregion

#pragma region "STRDUP"

	#pragma region "StrDup"

		char *ft_strdup(const char *s1) {
			if (!s1) return (NULL);

			size_t len = ft_strlen(s1);
			char *copy = malloc(len + 1);
			if (!copy) return (NULL);

			memcpy(copy, s1, len);
			copy[len] = '\0';
			return (copy);
		}

	#pragma endregion

	#pragma region "StrNDup"

		char *ft_strndup(const char *s1, size_t n) {
			if (!s1) return (NULL);

			size_t len = 0;
    		while (len < n && s1[len] != '\0') len++;

			char *copy = malloc(len + 1);
			if (!copy) return (NULL);

			memcpy(copy, s1, len);
			copy[len] = '\0';
			return (copy);
		}

	#pragma endregion

#pragma endregion

#pragma region "STRLEN"

	size_t ft_strlen(const char *str) {
		size_t i = 0;

		while (str && str[i]) ++i;
		return (i);
	}

#pragma endregion

#pragma region "ITOA"

	char	*ft_itoa(int n) {
		char *str;
		int len = 0, sign = 1, tmp = n;

		if (n == 0) return (ft_strdup("0"));
		if (n < 0) { sign = -1; len++; }
		while (tmp != 0) { tmp /= 10; len++; }
		str = malloc((len + 1));
		if (!str) return (NULL);
		str[len] = '\0';
		while (n != 0) {
			str[--len] = '0' + (sign * (n % 10));
			n /= 10;
		}
		if (sign == -1) str[0] = '-';
		return (str);
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
