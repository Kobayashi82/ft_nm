
#pragma region "Includes"

	#include "utils/utils.h"

#pragma endregion

#pragma region "Char"

	static void	print_c(int c, int *total, int fd) {
		int	result;

		result = write(fd, &c, 1);
		*total += result;
		if (result == -1)
			*total = -1;
	}

#pragma endregion

#pragma region "String"

	static void	print_s(char *s, int *total, int fd) {
		if (!s)
			s = "(null)";
		while (*s && *total >= 0)
			print_c(*s++, total, fd);
	}

#pragma endregion

#pragma region "Number"

	static void	print_n(unsigned long n, unsigned int base, int *total, int fd) {
		char	*strbase;

		if (base == 11)
		{
			base = 10;
			print_c('-', total, fd);
		}
		if (base == 17)
		{
			base = 16;
			print_s("0x", total, fd);
		}
		strbase = "0123456789abcdef";
		if (fd < 0)
			strbase = "0123456789ABCDEF";
		if (n >= base)
			print_n(n / base, base, total, fd);
		if (*total == -1)
			return ;
		if (fd < 0)
			fd = -fd;
		print_c(strbase[n % base], total, fd);
	}

#pragma endregion

#pragma region "Chooser"

	static void	chooser(int c, va_list args, int *total, int fd) {
		long long	n;

		if (c == '%')
			print_c('%', total, fd);
		if (c == 'c')
			print_c(va_arg(args, int), total, fd);
		if (c == 's')
			print_s(va_arg(args, char *), total, fd);
		if (c == 'd' || c == 'i')
		{
			n = va_arg(args, int);
			if (n < 0)
				print_n(n * -1, 11, total, fd);
			else
				print_n(n, 10, total, fd);
		}
		if (c == 'u')
			print_n(va_arg(args, unsigned int), 10, total, fd);
		if (c == 'p')
			print_n(va_arg(args, unsigned long), 17, total, fd);
		if (c == 'x')
			print_n(va_arg(args, unsigned int), 16, total, fd);
		if (c == 'X')
			print_n(va_arg(args, unsigned int), 16, total, -fd);
	}

#pragma endregion

#pragma region "ft_dprintf"

	int	ft_dprintf(int fd, char const *format, ...) {
		va_list	args;
		int		total;

		total = 0;
		va_start(args, format);
		while (format && *format && total >= 0)
		{
			if (*format == '%')
				chooser(*(++format), args, &total, fd);
			else
				print_c(*format, &total, fd);
			format++;
		}
		va_end(args);
		return (total);
	}

#pragma endregion
