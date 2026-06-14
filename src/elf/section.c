
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

	#include <elf.h>

#pragma endregion

#pragma region "Validate"

	int validate(t_elf *elf) {
		if (!elf || !elf->base) return (1);

		const unsigned char *m = (const unsigned char *)elf->base;

		// Truncado
		if (elf->size < 5) return (UNRECOGNIZED);

		// Magic Number
		if (m[0] != 0x7f || m[1] != 'E' || m[2] != 'L' || m[3] != 'F') return (UNRECOGNIZED);

		// Class
		if		(m[EI_CLASS] == ELFCLASS32)	elf->bits = 32;
		else if	(m[EI_CLASS] == ELFCLASS64)	elf->bits = 64;
		else return (UNRECOGNIZED);

		// Little Endian
		if (m[EI_DATA] != ELFDATA2LSB) return (UNRECOGNIZED);

		// Truncado
		if (elf->bits == 32 && elf->size < sizeof(Elf32_Ehdr)) return (TRUNCATED);
		if (elf->bits == 64 && elf->size < sizeof(Elf64_Ehdr)) return (TRUNCATED);

		// Arquitectura
		if (elf->bits == 32 && ((Elf32_Ehdr *)elf->base)->e_machine != EM_386)		return (UNRECOGNIZED);
		if (elf->bits == 64 && ((Elf64_Ehdr *)elf->base)->e_machine != EM_X86_64)	return (UNRECOGNIZED);

		// Type
		if (elf->bits == 32) elf->type = ((Elf32_Ehdr *)elf->base)->e_type;
		if (elf->bits == 64) elf->type = ((Elf64_Ehdr *)elf->base)->e_type;

		return (0);
	}

#pragma endregion

#pragma region "Set"

	static int set_section(t_elf *elf, t_section *section, void *shdr) {
		if (!section || !shdr) return (UNRECOGNIZED);

		if (elf->bits == 32) {
			section->sh_offset	= ((Elf32_Shdr *)shdr)->sh_offset;
			section->sh_size	= ((Elf32_Shdr *)shdr)->sh_size;
			if (section->sh_offset + section->sh_size > elf->size) return (TRUNCATED);
			section->exists = 1;
			return (0);
		}

		if (elf->bits == 64) {
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

	int load_sections(t_elf *elf) {
		if (!elf) return (UNRECOGNIZED);

		if (elf->bits == 32) {
			Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elf->base;

			// Validar que SHT está dentro del archivo mapeado
			if (ehdr->e_shoff + ehdr->e_shnum * sizeof(Elf32_Shdr) > elf->size) return (TRUNCATED);
			// Dirección del SHT
			Elf32_Shdr *shdr = (Elf32_Shdr *)(elf->base + ehdr->e_shoff);
			elf->shdr_offset = ehdr->e_shoff;
			elf->shdr_num = ehdr->e_shnum;

			// Validar que e_shstrndx está dentro del archivo mapeado
			if (ehdr->e_shstrndx >= ehdr->e_shnum) return (TRUNCATED);
			// Dirección de header de .shstrtab
			Elf32_Shdr *shstrtab = &shdr[ehdr->e_shstrndx];

			// Validar que .shstrtab está dentro del archivo mapeado
			if (shstrtab->sh_offset + shstrtab->sh_size > elf->size) return (TRUNCATED);

			// Recorrer headers y añadir referencias
			for (int i = 0; i < ehdr->e_shnum; ++i) {
				if (shdr[i].sh_type == SHT_SYMTAB) {
					if (!elf->symtab.exists || shdr[i].sh_size > 0) {
						set_section(elf, &elf->symtab, &shdr[i]);
						if (shdr[i].sh_link < ehdr->e_shnum) {
							set_section(elf, &elf->strtab, &shdr[shdr[i].sh_link]);
						}
					}
				}
				if (shdr[i].sh_type == SHT_DYNSYM) {
					if (!elf->dynsym.exists || shdr[i].sh_size > 0) {
						set_section(elf, &elf->dynsym, &shdr[i]);
						if (shdr[i].sh_link < ehdr->e_shnum) {
							set_section(elf, &elf->dynstr, &shdr[shdr[i].sh_link]);
						}
					}
				}
			}

			return (0);
		}

		if (elf->bits == 64) {
			Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf->base;

			// Validar que SHT está dentro del archivo mapeado
			if (ehdr->e_shoff + ehdr->e_shnum * sizeof(Elf64_Shdr) > elf->size) return (TRUNCATED);
			// Dirección del SHT
			Elf64_Shdr *shdr = (Elf64_Shdr *)(elf->base + ehdr->e_shoff);
			elf->shdr_offset = ehdr->e_shoff;
			elf->shdr_num = ehdr->e_shnum;

			// Validar que e_shstrndx está dentro del archivo mapeado
			if (ehdr->e_shstrndx >= ehdr->e_shnum) return (TRUNCATED);
			// Dirección de header de .shstrtab
			Elf64_Shdr *shstrtab = &shdr[ehdr->e_shstrndx];

			// Validar que .shstrtab está dentro del archivo mapeado
			if (shstrtab->sh_offset + shstrtab->sh_size > elf->size) return (TRUNCATED);

			// Recorrer headers y añadir referencias
			for (int i = 0; i < ehdr->e_shnum; ++i) {
				if (shdr[i].sh_type == SHT_SYMTAB) {
					if (!elf->symtab.exists || shdr[i].sh_size > 0) {
						set_section(elf, &elf->symtab, &shdr[i]);
						if (shdr[i].sh_link < ehdr->e_shnum) {
							set_section(elf, &elf->strtab, &shdr[shdr[i].sh_link]);
						}
					}
				}
				if (shdr[i].sh_type == SHT_DYNSYM) {
					if (!elf->dynsym.exists || shdr[i].sh_size > 0) {
						set_section(elf, &elf->dynsym, &shdr[i]);
						if (shdr[i].sh_link < ehdr->e_shnum) {
							set_section(elf, &elf->dynstr, &shdr[shdr[i].sh_link]);
						}
					}
				}
			}

			return (0);
		}

		return (UNRECOGNIZED);
	}

#pragma endregion
