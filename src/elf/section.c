
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

	#include <unistd.h>
	#include <errno.h>
	#include <elf.h>

#pragma endregion

#pragma region "Validate"

	int validate(t_elf *elf, const char *path) {
		if (!elf || !elf->base || !path) return (1);

		const unsigned char *m = (const unsigned char *)elf->base;

		// Truncado
		if (elf->size < 5) return (TRUNCATED);

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

int section() {
	// Encontrar las secciones relevantes — buscar .symtab y su .strtab asociada (o .dynsym/.dynstr si -D está activo)
	return (1);
}

int header() {
	// Localizar la Section Header Table — usando e_shoff, e_shnum y e_shstrndx del header
	return (1);
}
