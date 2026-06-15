
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
			t_sym *curr_sym = tmp->sym;
			while(curr_sym) {
				t_sym *tmp_sym = curr_sym;
				curr_sym = curr_sym->next;
				free(tmp_sym->name);
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
			if	(errno == ENOENT)	ft_dprintf(STDERR_FILENO, NAME": '%s': No such file\n", file->path);
			else 					ft_dprintf(STDERR_FILENO, NAME": %s: %s\n", file->path, strerror(errno));
			return (1);
		}

		// Obtener información del archivo
		struct stat st;
		if (fstat(fd, &st) == -1) {
			ft_dprintf(STDERR_FILENO, NAME": %s: %s\n", file->path, strerror(errno));
			close(fd);
			return (1);
		}

		// Verificar si es un directorio
		if (S_ISDIR(st.st_mode)) {
			ft_dprintf(STDERR_FILENO, NAME": Warning: '%s' is a directory\n", file->path);
			close(fd);
			return (1);
		}

		// Verificar si no es un archivo regular
		if (!S_ISREG(st.st_mode)) {
			ft_dprintf(STDERR_FILENO, NAME": Warning: '%s' is not an ordinary file\n", file->path);
			close(fd);
			return (1);
		}

		// Mapear el archivo en memoria
		file->size = st.st_size;
		file->base = mmap(NULL, file->size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (file->base == MAP_FAILED) {
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
				return (NO_MEMORY);
			}

			ft_memset(node, 0, sizeof(t_elf));

			node->file        = file;
			node->name        = ft_strdup(file->path);
			node->base        = file->base;
			node->size        = file->size;

			file->elf = node;
		}

		return (0);
	}

#pragma endregion
