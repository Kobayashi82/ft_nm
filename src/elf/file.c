
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

	#include <stdlib.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <string.h>
	#include <errno.h>
	#include <sys/stat.h>
	#include <sys/mman.h>

#pragma endregion

#pragma region "UnMap"

	// Libera la memoria mapeada del archivo
	int unmap_file(t_file *file) {
		if (!file || !file->base || !file->size) return (1);

		// Liberar nodos 'ELF'
		t_elf *current = file->elf;
		while (current) {
			t_elf *tmp = current;
			current = current->next;
			t_sym *curr_sym = tmp->symbol;
			while(curr_sym) {
				t_sym *tmp_sym = curr_sym;
				curr_sym = curr_sym->next;
				free(tmp_sym);
			}
			free(tmp->name);
			free(tmp);
		}

		// Liberar memoria mapeada
		munmap(file->base, file->size);

		return (0);
	}

#pragma endregion

#pragma region "Map"

	// Mapea en memoria el archivo
	int map_file(t_file *file) {
		if (!file || !file->path) return (1);

		// Abrir archivo
		int fd = open(file->path, O_RDONLY);
		if (fd == -1) {
			if	(errno == ENOENT)	ft_dprintf(STDERR_FILENO, "ft_nm: '%s': No such file\n", file->path);
			else 					ft_dprintf(STDERR_FILENO, "ft_nm: %s: %s\n", file->path, strerror(errno));
			return (1);
		}

		// Obtener tamaño y verificar si no es un directorio
		struct stat st;
		if (fstat(fd, &st) == -1 || S_ISDIR(st.st_mode)) {
			ft_dprintf(STDERR_FILENO, "ft_nm: Warning: '%s' is a directory\n", file->path);
			close(fd);
			return (1);
		}
		file->size = st.st_size;

		// Mapear el archivo en memoria
		file->base = mmap(NULL, file->size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (file->base == MAP_FAILED) {
			ft_dprintf(STDERR_FILENO, "ft_nm: %s: %s\n", file->path, strerror(errno));
			close(fd);
			return (1);
		}
		close(fd);
	
		// Crear nodos 'ELF'
		if (is_archive(file->base, file->size)) {
			file->type = ARCHIVE;
			if (parse_archive(file)) return (1);
		} else {
			file->type = ELF; 
			t_elf *node = malloc(sizeof(t_elf));
			if (!node) {
				ft_dprintf(STDERR_FILENO, NAME": %s: memory allocation failed\n", file->path);
				return (1);
			}
			ft_memset(node, 0, sizeof(t_elf));

			node->name = ft_strdup(file->path);
			node->base = file->base;
			node->size = file->size;
			node->bits = 0;
			node->error = 0;
			node->symbol = NULL;
			node->next = NULL;

			file->elf = node;
		}

		return (0);
	}

#pragma endregion
