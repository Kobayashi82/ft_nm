
#pragma region "Includes"

	#include <stddef.h>

#pragma endregion

#pragma region "Methods"

	int		ft_isdigit(int c);
	int		ft_isnum_s(const char *str);
	int		ft_islower(char c);
	size_t	ft_strlen(const char *str);
	int		ft_strcmp(const char *s1, const char *s2);
	int		ft_strncmp(const char *s1, const char *s2, size_t n);
	char	*ft_strchr(const char *s, int c);
	size_t	ft_strlcpy(char *dst, const char *src, size_t dstsize);
	char	*ft_strdup(const char *s1);
	char	*ft_strndup(const char *s1, size_t n);
	void	*ft_memcpy(void *dst, const void *src, size_t n);
	void	*ft_memset(void *b, int c, size_t len);
	int		ft_atoi(const char *str);
	int		ft_dprintf(int fd, char const *format, ...);

#pragma endregion
