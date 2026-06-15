
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
		return (s->type == 'U' || s->type == 'u' || s->type == 'w' || s->type == 'v');
	}

#pragma endregion

#pragma region "Add Symbol"

	static int add_symbol(t_elf *elf, char *name, char type, uint64_t value, int is_section) {
		if (!name) return (0);

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

	#pragma region "32 Bits"

		static char get_type_32(t_elf *elf, void *src_sym) {
			Elf32_Sym *sym = (Elf32_Sym *)src_sym;

			unsigned char	bind = ELF64_ST_BIND(sym->st_info);
			unsigned char	type = ELF64_ST_TYPE(sym->st_info);
			int				is_global  = (bind == STB_GLOBAL || bind == STB_GNU_UNIQUE);
			int				is_weak    = (bind == STB_WEAK);
			int				is_unique  = (bind == STB_GNU_UNIQUE);

			if (sym->st_shndx == SHN_UNDEF) {
				if (is_weak && type == STT_OBJECT)								return ('v');
				if (is_weak)													return ('w');
				return ('U');
			}

			if (is_unique)														return ('u');

			if (type == STT_FILE)												return ('f');
			if (type == STT_GNU_IFUNC)											return ('i');
			if (sym->st_shndx == SHN_ABS)										return (is_global  ? 'A' : 'a');
			if (sym->st_shndx == SHN_COMMON)									return (is_global  ? 'C' : 'c');
			if (is_weak) {
				if (type == STT_OBJECT) return ('V');
				return ('W');
			}

			// Obtener la seccion de los symbolos
			Elf32_Shdr *shdr = (Elf32_Shdr *)(elf->base + elf->shdr_offset);
			Elf32_Shdr *sec  = &shdr[sym->st_shndx];

			if (sec->sh_type   == SHT_NOBITS)									return (is_global ? 'B' : 'b');
			if (sec->sh_flags   & SHF_EXECINSTR)								return (is_global ? 'T' : 't');
			if (!(sec->sh_flags & SHF_WRITE) && (sec->sh_flags & SHF_ALLOC))	return (is_global ? 'R' : 'r');
			if (sec->sh_flags   & SHF_WRITE)									return (is_global ? 'D' : 'd');

			// Comprobar si el nombre de la sección es de debug
			char *sec_name = (char *)(elf->shstrtab.base) + sec->sh_name;
			if (!ft_strncmp(sec_name, ".debug", 6))								return ('N');
			if (!ft_strncmp(sec_name, ".zdebug", 7))							return ('N');
			if (!ft_strncmp(sec_name, ".gnu.debuglto_.debug_", 21))				return ('N');
			if (!ft_strncmp(sec_name, ".gnu.linkonce.wi.", 17))					return ('N');
			if (!ft_strncmp(sec_name, ".line", 5))								return ('N');
			if (!ft_strncmp(sec_name, ".stab", 5))								return ('N');
			if (!ft_strcmp(sec_name, ".gdb_index"))								return ('N');

			return (is_global ? 'S' : 'n');
		}

	#pragma endregion

	#pragma region "64 Bits"

		static char get_type_64(t_elf *elf, void *src_sym) {
			Elf64_Sym *sym = (Elf64_Sym *)src_sym;

			unsigned char	bind = ELF64_ST_BIND(sym->st_info);
			unsigned char	type = ELF64_ST_TYPE(sym->st_info);
			int				is_global  = (bind == STB_GLOBAL || bind == STB_GNU_UNIQUE);
			int				is_weak    = (bind == STB_WEAK);
			int				is_unique  = (bind == STB_GNU_UNIQUE);

			if (sym->st_shndx == SHN_UNDEF) {
				if (is_weak && type == STT_OBJECT)								return ('v');
				if (is_weak)													return ('w');
				return ('U');
			}

			if (is_unique)														return ('u');

			if (type == STT_FILE)												return ('f');
			if (type == STT_GNU_IFUNC)											return ('i');
			if (sym->st_shndx == SHN_ABS)										return (is_global  ? 'A' : 'a');
			if (sym->st_shndx == SHN_COMMON)									return (is_global  ? 'C' : 'c');
			if (is_weak) {
				if (type == STT_OBJECT) return ('V');
				return ('W');
			}

			// Obtener la seccion de los symbolos
			Elf64_Shdr *shdr = (Elf64_Shdr *)(elf->base + elf->shdr_offset);
			Elf64_Shdr *sec  = &shdr[sym->st_shndx];

			if (sec->sh_type   == SHT_NOBITS)									return (is_global ? 'B' : 'b');
			if (sec->sh_flags   & SHF_EXECINSTR)								return (is_global ? 'T' : 't');
			if (!(sec->sh_flags & SHF_WRITE) && (sec->sh_flags & SHF_ALLOC))	return (is_global ? 'R' : 'r');
			if (sec->sh_flags   & SHF_WRITE)									return (is_global ? 'D' : 'd');

			// Comprobar si el nombre de la sección es de debug
			char *sec_name = (char *)(elf->shstrtab.base) + sec->sh_name;
			if (!ft_strncmp(sec_name, ".debug", 6))								return ('N');
			if (!ft_strncmp(sec_name, ".zdebug", 7))							return ('N');
			if (!ft_strncmp(sec_name, ".gnu.debuglto_.debug_", 21))				return ('N');
			if (!ft_strncmp(sec_name, ".gnu.linkonce.wi.", 17))					return ('N');
			if (!ft_strncmp(sec_name, ".line", 5))								return ('N');
			if (!ft_strncmp(sec_name, ".stab", 5))								return ('N');
			if (!ft_strcmp(sec_name, ".gdb_index"))								return ('N');

			return (is_global ? 'S' : 'n');
		}

	#pragma endregion

		static char get_type(t_elf *elf, void *src_sym) {
			if (elf->bits == 32) return (get_type_32(elf, src_sym));
			if (elf->bits == 64) return (get_type_64(elf, src_sym));

			return ('?');
		}

#pragma endregion

#pragma region "Load"

	#pragma region "32 Bits"

		static int load_symbols_32(t_elf *elf, t_section *sym_tab, t_section *sym_str) {
			size_t sym_total = sym_tab->sh_size / sizeof(Elf32_Sym);
			if (sym_total < 2) return (0);

			Elf32_Sym *sym = (Elf32_Sym *)sym_tab->base;
			char *strings  = (char *)sym_str->base;
				
			for (size_t i = 1; i < sym_total; ++i) {

				// Salta el símbolo si el Offset del nombre está fuera de los límites de la tabla de cadenas
				if (sym[i].st_name >= sym_str->sh_size) continue;

				int is_section = 0;
				// Obtiene el nombre del símbolo usando st_name como offset dentro de la tabla de cadenas
				char *name = get_name(strings + sym[i].st_name, sym_str->sh_size - sym[i].st_name);
				if (!name) return (NO_MEMORY);

				// Los símbolos de tipo STT_SECTION tienen por convención un st_name vacío, así que se muestran utilizando el nombre de la sección a la que apuntan
				if (!*name && ELF32_ST_TYPE(sym[i].st_info) == STT_SECTION) {
					free(name); 

					// st_shndx contiene el índice de la sección a la que hace referencia este símbolo
					uint16_t shndx = sym[i].st_shndx;
					if (shndx < elf->shdr_num) {
						// Obtiene el array de cabeceras de secciones
						Elf32_Shdr *shdr = (Elf32_Shdr *)(elf->base + elf->shdr_offset);

						// Valida que shstrtab esté cargada y que sh_name esté dentro de sus límites
						if (!elf->shstrtab.exists || shdr[shndx].sh_name >= elf->shstrtab.sh_size) continue;

						// Obtiene el nombre de la sección usando sh_name como offset dentro de shstrtab
						char *sec_name = (char *)(elf->shstrtab.base) + shdr[shndx].sh_name;
						name = ft_strdup(sec_name);
						if (!name) return (NO_MEMORY);
						is_section = 1;
					} else continue;
				}

				char 		type  = get_type(elf, &sym[i]);
				uint64_t	value = (type == 'U' || type == 'w' || type == 'v') ? 0 : sym[i].st_value;

				if (type == '?') {
					free(name);
					continue;
				}

				if (add_symbol(elf, name, type, value, is_section)) {
					free(name);
					return (NO_MEMORY);
				}
			}

			return (0);
		}

	#pragma endregion

	#pragma region "64 Bits"

		static int load_symbols_64(t_elf *elf, t_section *sym_tab, t_section *sym_str) {
			size_t sym_total = sym_tab->sh_size / sizeof(Elf64_Sym);
			if (sym_total < 2) return (0);

			Elf64_Sym *sym = (Elf64_Sym *)sym_tab->base;
			char *strings  = (char *)sym_str->base;

			for (size_t i = 1; i < sym_total; ++i) {
				
				// Salta el símbolo si el Offset del nombre está fuera de los límites de la tabla de cadenas
				if (sym[i].st_name >= sym_str->sh_size) continue;

				int is_section = 0;
				// Obtiene el nombre del símbolo usando st_name como offset dentro de la tabla de cadenas
				char *name = get_name(strings + sym[i].st_name, sym_str->sh_size - sym[i].st_name);
				if (!name) return (NO_MEMORY);

				// Los símbolos de tipo STT_SECTION tienen por convención un st_name vacío, así que se muestran utilizando el nombre de la sección a la que apuntan
				if (!*name && ELF64_ST_TYPE(sym[i].st_info) == STT_SECTION) {
					free(name); 

					// st_shndx contiene el índice de la sección a la que hace referencia este símbolo
					uint16_t shndx = sym[i].st_shndx;
					if (shndx < elf->shdr_num) {
						// Obtiene el array de cabeceras de secciones
						Elf64_Shdr *shdr = (Elf64_Shdr *)(elf->base + elf->shdr_offset);

						// Valida que shstrtab esté cargada y que sh_name esté dentro de sus límites
						if (!elf->shstrtab.exists || shdr[shndx].sh_name >= elf->shstrtab.sh_size) continue;

						// Obtiene el nombre de la sección usando sh_name como offset dentro de shstrtab
						char *sec_name = (char *)(elf->shstrtab.base) + shdr[shndx].sh_name;
						name = ft_strdup(sec_name);
						if (!name) return (NO_MEMORY);
						is_section = 1;
					} else continue;
				}

				char 		type  = get_type(elf, &sym[i]);
				uint64_t	value = (type == 'U' || type == 'w' || type == 'v') ? 0 : sym[i].st_value;

				if (type == '?') {
					free(name);
					continue;
				}

				if (add_symbol(elf, name, type, value, is_section)) {
					free(name);
					return (NO_MEMORY);
				}
			}

			return (0);
		}

	#pragma endregion

	int load_symbols(t_elf *elf) {
		t_section	*sym_tab  = (g_nm.options.dynamic) ? &elf->dynsym : &elf->symtab;
		t_section	*sym_str  = (g_nm.options.dynamic) ? &elf->dynstr : &elf->strtab;

		if (!sym_tab->exists || !sym_str->exists) return (0);

		int ret = 0;

		if (elf->bits == 32) ret = load_symbols_32(elf, sym_tab, sym_str);
		if (elf->bits == 64) ret = load_symbols_64(elf, sym_tab, sym_str);

		return (ret);
	}

#pragma endregion
