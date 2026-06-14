
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

	#include <elf.h>

#pragma endregion

#pragma region "Validate"

	static int validate(t_elf *elf) {
		const char *ehdr = (const char *)elf->base;

		// Magic Number
		if (elf->size <= SELFMAG || ft_strncmp(ehdr, ELFMAG, SELFMAG)) return (UNRECOGNIZED);

		// Class
		if		(ehdr[EI_CLASS] == ELFCLASS32)	elf->bits = 32;
		else if	(ehdr[EI_CLASS] == ELFCLASS64)	elf->bits = 64;
		else return (UNRECOGNIZED);

		// Little Endian
		if (ehdr[EI_DATA] != ELFDATA2LSB) return (UNRECOGNIZED);

		// Truncado
		if (elf->bits == 32 && elf->size < sizeof(Elf32_Ehdr)) return (TRUNCATED);
		if (elf->bits == 64 && elf->size < sizeof(Elf64_Ehdr)) return (TRUNCATED);

		// Arquitectura
		if (elf->bits == 32 && ((Elf32_Ehdr *)elf->base)->e_machine != EM_386)		return (UNRECOGNIZED);
		if (elf->bits == 64 && ((Elf64_Ehdr *)elf->base)->e_machine != EM_X86_64)	return (UNRECOGNIZED);

		return (0);
	}

#pragma endregion

#pragma region "Set"

	static int set_section(t_elf *elf, t_section *section, void *shdr) {
		if (elf->bits == 32) {
			section->base       = (uintptr_t)((uint8_t *)elf->base + ((Elf32_Shdr *)shdr)->sh_offset);
			section->sh_offset	= ((Elf32_Shdr *)shdr)->sh_offset;
			section->sh_size	= ((Elf32_Shdr *)shdr)->sh_size;
			if (section->sh_offset + section->sh_size > elf->size) return (TRUNCATED);

			section->exists = 1;
			return (0);
		}

		if (elf->bits == 64) {
			section->base       = (uintptr_t)((uint8_t *)elf->base + ((Elf64_Shdr *)shdr)->sh_offset);
			section->sh_offset	= ((Elf64_Shdr *)shdr)->sh_offset;
			section->sh_size	= ((Elf64_Shdr *)shdr)->sh_size;
			if (section->sh_offset + section->sh_size > elf->size) return (TRUNCATED);

			section->exists = 1;
			return (0);
		}

		return (UNRECOGNIZED);
	}

#pragma endregion

#pragma region "Load"

	#pragma region "32 Bits"

		static int load_sections_32(t_elf *elf) {
			Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elf->base;

			// Validar que SHT está dentro del archivo mapeado
			if (ehdr->e_shoff + ehdr->e_shnum * sizeof(Elf32_Shdr) > elf->size) return (TRUNCATED);
			// Dirección del SHT
			Elf32_Shdr *shdr = (Elf32_Shdr *)(elf->base + ehdr->e_shoff);

			elf->shdr_offset = ehdr->e_shoff;
			elf->shdr_num = ehdr->e_shnum;

			// Validar que e_shstrndx está dentro del archivo mapeado
			if (ehdr->e_shstrndx == SHN_UNDEF || ehdr->e_shstrndx >= ehdr->e_shnum) return (TRUNCATED);
			// Dirección del header de .shstrtab
			Elf32_Shdr *shstrtab = &shdr[ehdr->e_shstrndx];
			// Validar que .shstrtab está dentro del archivo mapeado
			if (shstrtab->sh_offset + shstrtab->sh_size > elf->size) return (TRUNCATED);

			// Añadir referencia a .shstrtab
			if (shstrtab->sh_type == SHT_STRTAB)
				set_section(elf, &elf->shstrtab, shstrtab);

			// Recorrer headers y añadir referencias
			for (int i = 0; i < ehdr->e_shnum; ++i) {

				if (!g_nm.options.dynamic && shdr[i].sh_type == SHT_SYMTAB) {
					if (!elf->symtab.exists && shdr[i].sh_size > 0) {
						set_section(elf, &elf->symtab, &shdr[i]);
						if (shdr[i].sh_link > 0 && shdr[i].sh_link < ehdr->e_shnum) {
							if (shdr[shdr[i].sh_link].sh_type == SHT_STRTAB)
								set_section(elf, &elf->strtab, &shdr[shdr[i].sh_link]);
						}
					}
				}

				if (g_nm.options.dynamic && shdr[i].sh_type == SHT_DYNSYM) {
					if (!elf->dynsym.exists && shdr[i].sh_size > 0) {
						set_section(elf, &elf->dynsym, &shdr[i]);
						if (shdr[i].sh_link > 0 && shdr[i].sh_link < ehdr->e_shnum) {
							if (shdr[shdr[i].sh_link].sh_type == SHT_STRTAB)
								set_section(elf, &elf->dynstr, &shdr[shdr[i].sh_link]);
						}
					}
				}

			}

			return (0);
		}

	#pragma endregion

	#pragma region "64 Bits"

		static int load_sections_64(t_elf *elf) {
			Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf->base;

			// Validar que SHT está dentro del archivo mapeado
			if (ehdr->e_shoff + ehdr->e_shnum * sizeof(Elf64_Shdr) > elf->size) return (TRUNCATED);
			// Dirección del SHT
			Elf64_Shdr *shdr = (Elf64_Shdr *)(elf->base + ehdr->e_shoff);

			elf->shdr_offset = ehdr->e_shoff;
			elf->shdr_num = ehdr->e_shnum;

			// Validar que e_shstrndx está dentro del archivo mapeado
			if (ehdr->e_shstrndx == SHN_UNDEF || ehdr->e_shstrndx >= ehdr->e_shnum) return (TRUNCATED);
			// Dirección del header de .shstrtab
			Elf64_Shdr *shstrtab = &shdr[ehdr->e_shstrndx];
			// Validar que .shstrtab está dentro del archivo mapeado
			if (shstrtab->sh_offset + shstrtab->sh_size > elf->size) return (TRUNCATED);

			// Añadir referencia a .shstrtab
			if (shstrtab->sh_type == SHT_STRTAB)
				set_section(elf, &elf->shstrtab, shstrtab);

			// Recorrer headers y añadir referencias
			for (int i = 0; i < ehdr->e_shnum; ++i) {

				if (!g_nm.options.dynamic && shdr[i].sh_type == SHT_SYMTAB) {
					if (!elf->symtab.exists && shdr[i].sh_size > 0) {
						set_section(elf, &elf->symtab, &shdr[i]);
						if (shdr[i].sh_link > 0 && shdr[i].sh_link < ehdr->e_shnum) {
							if (shdr[shdr[i].sh_link].sh_type == SHT_STRTAB)
								set_section(elf, &elf->strtab, &shdr[shdr[i].sh_link]);
						}
					}
				}

				if (g_nm.options.dynamic && shdr[i].sh_type == SHT_DYNSYM) {
					if (!elf->dynsym.exists && shdr[i].sh_size > 0) {
						set_section(elf, &elf->dynsym, &shdr[i]);
						if (shdr[i].sh_link > 0 && shdr[i].sh_link < ehdr->e_shnum) {
							if (shdr[shdr[i].sh_link].sh_type == SHT_STRTAB)
								set_section(elf, &elf->dynstr, &shdr[shdr[i].sh_link]);
						}
					}
				}

			}

			return (0);
		}

	#pragma endregion

	int load_sections(t_elf *elf) {
		int ret = 0;

		if (!ret)						ret = validate(elf);
		if (!ret && elf->bits == 32)	ret = load_sections_32(elf);
		if (!ret && elf->bits == 64)	ret = load_sections_64(elf);
		
		return (ret);
	}

#pragma endregion
