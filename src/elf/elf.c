
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

	#include <unistd.h>
	#include <fcntl.h>
	#include <string.h>
	#include <errno.h>
	#include <sys/stat.h>
	#include <sys/mman.h>
	#include <elf.h>

#pragma endregion

#pragma region "Validate"

	static int validate(t_elf *elf) {
		unsigned char *m = (unsigned char *)elf->map;

		// Magic Number
		if (m[0] != 0x7f || m[1] != 'E' || m[2] != 'L' || m[3] != 'F') {
			ft_dprintf(STDERR_FILENO, "ft_nm: %s: file format not recognized\n", elf->file);
			return (1);
		}

		// Class
		if		(m[EI_CLASS] == ELFCLASS32)	elf->bits = 32;
		else if	(m[EI_CLASS] == ELFCLASS64)	elf->bits = 64;
		else {
			ft_dprintf(STDERR_FILENO, "ft_nm: %s: file format not recognized\n", elf->file);
			return (1);
		}

		// Truncado
		if (elf->bits == 32 && elf->size < sizeof(Elf32_Ehdr)) {
			ft_dprintf(STDERR_FILENO, "ft_nm: %s: file truncated\n", elf->file);
			return (1);
		}
		if (elf->bits == 64 && elf->size < sizeof(Elf64_Ehdr)) {
			ft_dprintf(STDERR_FILENO, "ft_nm: %s: file truncated\n", elf->file);
			return (1);
		}

		// Little Endian
		if (m[EI_DATA] != ELFDATA2LSB) {
			ft_dprintf(STDERR_FILENO, "ft_nm: %s: file format not recognized\n", elf->file);
			return (1);
		}

		// Arquitectura
		if (elf->bits == 32 && ((Elf32_Ehdr *)elf->map)->e_machine != EM_386) {
			ft_dprintf(STDERR_FILENO, "ft_nm: %s: file format not recognized\n", elf->file);
			return (1);
		}
		if (elf->bits == 64 && ((Elf64_Ehdr *)elf->map)->e_machine != EM_X86_64) {
			ft_dprintf(STDERR_FILENO, "ft_nm: %s: file format not recognized\n", elf->file);
			return (1);
		}

		return (0);
	}

#pragma endregion

#pragma region "Map"

	static int map_file(t_elf *elf) {
		// Mapear el archivo en memoria
		elf->map = mmap(NULL, elf->size, PROT_READ, MAP_PRIVATE, elf->fd, 0);
		if (elf->map == MAP_FAILED) {
			ft_dprintf(STDERR_FILENO, "ft_nm: %s: %s\n", elf->file, strerror(errno));
			close(elf->fd);
			return (1);
		}
		close(elf->fd);

		return (0);
	}

#pragma endregion

#pragma region "Open"

	static int open_file(t_elf *elf) {
		struct stat st;

		// Abrir archivo
		elf->fd = open(elf->file, O_RDONLY);
		if (elf->fd == -1) {
			if	(errno == ENOENT)	ft_dprintf(STDERR_FILENO, "ft_nm: '%s': No such file\n", elf->file);
			else 					ft_dprintf(STDERR_FILENO, "ft_nm: %s: %s\n", elf->file, strerror(errno));
			return (1);
		}

		// Obtener tamaño y verificar si no es un directorio
		if (fstat(elf->fd, &st) == -1 || S_ISDIR(st.st_mode)) {
			ft_dprintf(STDERR_FILENO, "ft_nm: Warning: '%s' is a directory\n", elf->file);
			close(elf->fd);
			return (1);
		}
		elf->size = st.st_size;

		return (0);
	}

#pragma endregion

#pragma region "Load"

	int load_file(t_elf *elf) {
		if (open_file(elf))	return (1);
		if (map_file(elf))	return (1);
		if (validate(elf))	return (1);

		munmap(elf->map, elf->size);

		return (0);
	}

#pragma endregion
