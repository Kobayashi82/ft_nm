
#pragma region "Includes"

#include "nm.h"
#include "utils.h"

#include <unistd.h>
#include <stdlib.h>
#include <ar.h>

#pragma endregion

#pragma region "Is Archive"

	// Comprueba si es un 'Archive'
	int is_archive(void *base, size_t size) {
		if (size < SARMAG) return (0);

		return (!ft_strncmp((char *)base, ARMAG, SARMAG));
	}

#pragma endregion

#pragma region "Is Special"

	// Determina si el miembro es una entrada especial del archivo, no un objeto ELF
	// '/'  -> índice de símbolos
	// '//' -> tabla de nombres largos
	static int is_special(const char *name) {
		if (!name || name[0] == '\0') return (1);
		if (!ft_strcmp(name, "/") || !ft_strcmp(name, "//")) return (1);

		return (0);
	}

#pragma endregion

#pragma region "Get Name"

	// Extrae el nombre del miembro desde ar_name
	// Si ar_name empieza por '/' seguido de números, es un nombre largo
	// En otro caso, elimina espacios finales y el terminador '/'
	static char *get_name(const char *ar_name, const char *long_names, size_t long_names_size) {
		char	buf[17];
		int		i = 15;

		// Nombre largo: /NNN donde NNN es un offset en la tabla '//'
		if (ar_name[0] == '/' && ar_name[1] >= '0' && ar_name[1] <= '9') {
			size_t	offset = (size_t)ft_atoi(ar_name + 1);

			if (long_names && offset < long_names_size) {
				const char	*start = long_names + offset;
				const char	*end = start;

				// Los nombres en la tabla terminan con '/' seguido de '\n'
				while (end < long_names + long_names_size && *end != '/')
					end++;
				return (ft_strndup(start, (size_t)(end - start)));
			}
		}

		// Nombre corto: rellenado con espacios y opcionalmente terminado en '/'
		ft_memcpy(buf, ar_name, 16);
		buf[16] = '\0';

		while (i >= 0 && (buf[i] == ' ' || buf[i] == '/'))
			buf[i--] = '\0';

		return (ft_strdup(buf));
	}

#pragma endregion

#pragma region "Add Node"

	// Crea nodo 'ELF'
	static t_elf *add_node(t_file *file, void *base, size_t size) {
		t_elf *node = malloc(sizeof(t_elf));
		if (!node) return (NULL);

		ft_memset(node, 0, sizeof(t_elf));

		node->file        = file;
		node->base        = base;
		node->size        = size;

		return (node);
	}

#pragma endregion

#pragma region "Parse"

	// Parsea todos los miembros del 'Archive'
	int parse_archive(t_file *file) {
		void			*cursor = file->base + SARMAG;
		void			*end = file->base + file->size;
		struct ar_hdr	*hdr;
		size_t			member_size;
		t_elf			*node;
		t_elf			*tail = NULL;
		char			*name;
		const char		*long_names = NULL;
		size_t			long_names_size = 0;

		// Buscar la tabla de nombres largos '//'
		void *scan = cursor;
		while (scan < end) {
			if (scan + sizeof(struct ar_hdr) > end) break;

			struct ar_hdr *shdr = (struct ar_hdr *)scan;
			if (ft_strncmp(shdr->ar_fmag, ARFMAG, 2)) break;

			size_t ssize = (size_t)ft_atoi(shdr->ar_size);

			if (!ft_strncmp(shdr->ar_name, "//", 2)) {
				long_names = (const char *)scan + sizeof(struct ar_hdr);
				long_names_size = ssize;
				break;
			}
			scan += sizeof(struct ar_hdr) + ssize + (ssize % 2);
		}

		// Procesar todos los miembros del archivo
		while (cursor < end) {
			// Comprobar que queda espacio suficiente para una cabecera completa
			if (cursor + sizeof(struct ar_hdr) > end) break;

			hdr = (struct ar_hdr *)cursor;

			// Validar el terminador de la cabecera
			if (ft_strncmp(hdr->ar_fmag, ARFMAG, 2)) break;

			member_size = (size_t)ft_atoi(hdr->ar_size);
			cursor += sizeof(struct ar_hdr);

			// Comprobar que queda espacio suficiente para los datos del miembro
			if (cursor + member_size > end) break;

			name = get_name(hdr->ar_name, long_names, long_names_size);

			// Saltar miembros especiales (índice de símbolos '/' y tabla de nombres '//')
			if (!is_special(name)) {
				node = add_node(file, cursor, member_size);
				if (!node) {
					free(name);
					ft_dprintf(STDERR_FILENO, NAME": %s: memory allocation failed\n", file->path);
					return (NO_MEMORY);
				}
				node->name = name;

				if (member_size < 4 || ft_strncmp((char *)cursor, "\x7f\x45\x4c\x46", 4)) {
					free(name);
					free(node);
					ft_dprintf(STDERR_FILENO, NAME": %s: malformed archive\n", file->path);
					return (1);
				}

				if (!file->elf)	file->elf = node;
				else			tail->next = node;
				tail = node;
			} else {
				free(name);
			}

			// Avanzar pasando los datos del miembro + relleno (alineación a 2 bytes)
			cursor += member_size + (member_size % 2);
		}

		return (0);
	}

#pragma endregion
