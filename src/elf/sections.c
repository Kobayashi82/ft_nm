
int section() {
	// Encontrar las secciones relevantes — buscar .symtab y su .strtab asociada (o .dynsym/.dynstr si -D está activo)
	return (1);
}

int header() {
	// Localizar la Section Header Table — usando e_shoff, e_shnum y e_shstrndx del header
	return (1);
}
