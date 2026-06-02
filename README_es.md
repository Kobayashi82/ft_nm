<div align="center">

![System & Kernel](https://img.shields.io/badge/System-brown?style=for-the-badge)
![Filesystem](https://img.shields.io/badge/Filesystem-Directory-blue?style=for-the-badge)
![C Language](https://img.shields.io/badge/Language-C-red?style=for-the-badge)

*Una reimplementacion del comando nm clasico*

</div>

<div align="center">
	<img src="/images/ft_nm.jpg">
</div>

# ft_nm

[README in English](README.md)

`ft_nm` es una implementacion desde cero del comando Unix `nm`. Este proyecto se centra en el recorrido de directorios, metadatos de archivos, ordenacion y formato de salida, igualando el comportamiento de `nm` lo mas posible.

### Que es nm?

`nm` es la utilidad estandar para:

- Listar archivos y directorios.
- Ver permisos, tamanos, propietarios y fechas.
- Ordenar y filtrar entradas.
- Recorrer directorios de forma recursiva.

### Flujo tecnico

1. Parsear flags y rutas de entrada.
2. Leer entradas con `opendir` y `readdir`.
3. Obtener metadatos con `stat` o `lstat`.
4. Ordenar por nombre o fecha.
5. Formatear salida (long format, ocultos, orden inverso).
6. Si `-R` esta activo, entrar en subdirectorios.

## Instalacion

```bash
git clone https://github.com/Kobayashi82/ft_nm.git
cd ft_nm
make
```

## 🖥️ Uso

```bash
./ft_nm [opciones] [archivos]
```

### Opciones

| Opción | Descripción                    |
|--------|--------------------------------|
| `-a`   | Display debugger-only symbols  |
| `-g`   | Display only external symbols  |
| `-u`   | Display only undefined symbols |
| `-r`   | Reverse the sense of the sort  |
| `-p`   | Do not sort the symbols        |

### Ejemplos

```bash
./ft_nm
./ft_nm -la
./ft_nm -lrt src
./ft_nm -R .
```

## Licencia

Este proyecto esta licenciado bajo la WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**📂 Desarrollado como parte del curriculum de 42 School 📂**

*"See your system as it really is"*

</div>
