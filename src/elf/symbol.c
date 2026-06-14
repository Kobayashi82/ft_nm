
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

	#include <unistd.h>
	#include <stdlib.h>
	#include <elf.h>

#pragma endregion

#pragma region "Is Undefined"

	// Comprueba si el símbolo no está definido
	int is_undefined(t_sym *s) {
		return (s->type == 'U' || s->type == 'u' || s->type == 'w');
	}

#pragma endregion

#pragma region "Get Name"

	static char *get_name(const char *str, size_t maxlen) {
		size_t i = 0;
		while (i < maxlen && str && str[i]) ++i;

		char *name = malloc(i + 1);
		if (!name) return (NULL);

		ft_strlcpy(name, str, i + 1);

		return (name);
	}

#pragma endregion

#pragma region "Get Type"

	static char get_type(t_elf *elf, void *src_sym) {
		if (!elf || !src_sym) return ('?');

		if (elf->bits == 32) {
			Elf32_Sym *sym = (Elf32_Sym *)src_sym;

			unsigned char	bind = ELF64_ST_BIND(sym->st_info);
			unsigned char	type = ELF64_ST_TYPE(sym->st_info);
			int				is_global  = (bind == STB_GLOBAL);
			int				is_weak    = (bind == STB_WEAK);

			// Special shndx values
			if (sym->st_shndx == SHN_UNDEF)										return (is_weak    ? 'w' : 'U');
			if (type == STT_FILE)												return ('a');
			if (type == STT_GNU_IFUNC)											return ('i');
			if (sym->st_shndx == SHN_ABS)										return (is_global  ? 'A' : 'a');
			if (sym->st_shndx == SHN_COMMON)									return (is_global  ? 'C' : 'c');
			if (is_weak) {
				if (type == STT_OBJECT) return ('V');
				return ('W');
			}

			// Obtener la seccion de los symbolos
			Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elf->base;
			Elf32_Shdr *shdr = (Elf32_Shdr *)(elf->base + elf->shdr_offset);
			Elf32_Shdr *sec  = &shdr[sym->st_shndx];

			if (sec->sh_type   == SHT_NOBITS)									return (is_global ? 'B' : 'b');
			if (sec->sh_flags   & SHF_EXECINSTR)								return (is_global ? 'T' : 't');
			if (!(sec->sh_flags & SHF_WRITE) && (sec->sh_flags & SHF_ALLOC))	return (is_global ? 'R' : 'r');
			if (sec->sh_flags   & SHF_WRITE)									return (is_global ? 'D' : 'd');

			char *shstrtab = (char *)(elf->base + shdr[ehdr->e_shstrndx].sh_offset);
			char *sec_name = shstrtab + sec->sh_name;

			// Si el nombre de la sección empieza por ".debug" o ".zdebug", es 'N'
			if (!ft_strncmp(sec_name, ".debug", 6) || !ft_strncmp(sec_name, ".zdebug", 7)) return ('N');

			return (is_global ? 'S' : 'n');
		}

		if (elf->bits == 64) {
			Elf64_Sym *sym = (Elf64_Sym *)src_sym;

			unsigned char	bind = ELF64_ST_BIND(sym->st_info);
			unsigned char	type = ELF64_ST_TYPE(sym->st_info);
			int				is_global  = (bind == STB_GLOBAL);
			int				is_weak    = (bind == STB_WEAK);
			
			// Special shndx values
			if (sym->st_shndx == SHN_UNDEF)										return (is_weak    ? 'w' : 'U');
			if (type == STT_FILE)												return ('a');
			if (type == STT_GNU_IFUNC)											return ('i');
			if (sym->st_shndx == SHN_ABS)										return (is_global  ? 'A' : 'a');
			if (sym->st_shndx == SHN_COMMON)									return (is_global  ? 'C' : 'c');
			if (is_weak) {
				if (type == STT_OBJECT) return ('V');
				return ('W');
			}

			// Obtener la seccion de los symbolos
			Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf->base;
			Elf64_Shdr *shdr = (Elf64_Shdr *)(elf->base + elf->shdr_offset);
			Elf64_Shdr *sec  = &shdr[sym->st_shndx];

			if (sec->sh_type   == SHT_NOBITS)									return (is_global ? 'B' : 'b');
			if (sec->sh_flags   & SHF_EXECINSTR)								return (is_global ? 'T' : 't');
			if (!(sec->sh_flags & SHF_WRITE) && (sec->sh_flags & SHF_ALLOC))	return (is_global ? 'R' : 'r');
			if (sec->sh_flags   & SHF_WRITE)									return (is_global ? 'D' : 'd');

			char *shstrtab = (char *)(elf->base + shdr[ehdr->e_shstrndx].sh_offset);
			char *sec_name = shstrtab + sec->sh_name;

			// Si el nombre de la sección empieza por ".debug" o ".zdebug", es 'N'
			if (!ft_strncmp(sec_name, ".debug", 6) || !ft_strncmp(sec_name, ".zdebug", 7)) return ('N');

			return (is_global ? 'S' : 'n');
		}

		return ('?');
	}

#pragma endregion

#pragma region "Add"

	static int add_symbol(t_elf *elf, char *name, char type, uint64_t value, int is_section) {
		if (!elf || !name) return (0);

		t_sym *new_sym = malloc(sizeof(t_sym));
		if (!new_sym) {
			ft_dprintf(STDERR_FILENO, NAME": %s: memory allocation failed\n", elf->file->path);
			return (NO_MEMORY);
		}

		new_sym->name  = name;
		new_sym->type  = type;
		new_sym->value = value;
		new_sym->is_section = is_section;
		new_sym->next  = NULL;

		if (!elf->last_sym)	elf->sym = new_sym;
		else				elf->last_sym->next = new_sym;

		elf->last_sym = new_sym;

		return (0);
	}

#pragma endregion

#pragma region "Load"

	int load_symbols(t_elf *elf) {
		if (!elf) return (UNRECOGNIZED);

		t_section *sym_tab = (g_nm.options.dynamic) ? &elf->dynsym : &elf->symtab;
		t_section *sym_str = (g_nm.options.dynamic) ? &elf->dynstr : &elf->strtab;

		if (!sym_tab->exists || !sym_str->exists) return (0);

		if (elf->bits == 32) {
			size_t sym_total = sym_tab->sh_size / sizeof(Elf32_Sym);
			if (sym_total < 2) return (0);

			Elf32_Sym *sym = (Elf32_Sym *)(elf->base + sym_tab->sh_offset);
			char *strings = (char *)(elf->base + sym_str->sh_offset);

			for (size_t i = 1; i < sym_total; ++i) {
				char 		*name = get_name(strings + sym[i].st_name, sym_str->sh_size - sym[i].st_name);
				char 		type  = get_type(elf, &sym[i]);
				uint64_t	value = (type == 'U' || type == 'w') ? 0 : sym[i].st_value;

				if (!name) return (NO_MEMORY);
				if (type == '?') {
					free(name);
					continue;
				}

				int is_section = 0;
				if (*name == '\0' && ELF32_ST_TYPE(sym[i].st_info) == STT_SECTION) {
					free(name); 

					uint16_t shndx = sym[i].st_shndx;
					if (shndx < elf->shdr_num) {
						Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elf->base;
						Elf32_Shdr *shdr = (Elf32_Shdr *)(elf->base + elf->shdr_offset);
						char *shstrtab = (char *)(elf->base + shdr[ehdr->e_shstrndx].sh_offset); 
						char *sec_name = shstrtab + shdr[shndx].sh_name;
						name = ft_strdup(sec_name);
						is_section = 1;
						if (!name) return (NO_MEMORY);
					}
				}

				if (add_symbol(elf, name, type, value, is_section)) {
					free(name);
					return (NO_MEMORY);
				}
			}

			return (0);
		}

		if (elf->bits == 64) {
			size_t sym_total = sym_tab->sh_size / sizeof(Elf64_Sym);
			if (sym_total < 2) return (0);

			Elf64_Sym *sym = (Elf64_Sym *)(elf->base + sym_tab->sh_offset);
			char *strings = (char *)(elf->base + sym_str->sh_offset);

			for (size_t i = 1; i < sym_total; ++i) {
				char 		*name = get_name(strings + sym[i].st_name, sym_str->sh_size - sym[i].st_name);
				char 		type  = get_type(elf, &sym[i]);
				uint64_t	value = (type == 'U' || type == 'w') ? 0 : sym[i].st_value;

				if (!name) return (NO_MEMORY);
				if (type == '?') {
					free(name);
					continue;
				}

				int is_section = 0;
				if (*name == '\0' && ELF64_ST_TYPE(sym[i].st_info) == STT_SECTION) {
					free(name); 

					uint16_t shndx = sym[i].st_shndx;
					if (shndx < elf->shdr_num) {
						Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf->base;
						Elf64_Shdr *shdr = (Elf64_Shdr *)(elf->base + elf->shdr_offset);
						char *shstrtab = (char *)(elf->base + shdr[ehdr->e_shstrndx].sh_offset); 
						char *sec_name = shstrtab + shdr[shndx].sh_name;
						name = ft_strdup(sec_name);
						is_section = 1;
						if (!name) return (NO_MEMORY);
					}
				}

				if (add_symbol(elf, name, type, value, is_section)) {
					free(name);
					return (NO_MEMORY);
				}
			}

			return (0);
		}

		return (UNRECOGNIZED);
	}

#pragma endregion
