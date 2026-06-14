#!/bin/bash

cp ft_nm tester/ft_nm
cd tester
ft_nm=./ft_nm
# ft_nm="valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes -q ./ft_nm"
TEST_NUM=0
PASS=0
FAIL=0

# ─────────────────────────────────────────────
# Helpers
# ─────────────────────────────────────────────

run_nm() {
	TEST_NUM=$((TEST_NUM + 1))
	local label="TEST $TEST_NUM"
	[ "$#" -gt 0 ] && label="$label ($*)" || label="$label (no args)"

	$ft_nm "$@" > ft_nm_out 2> ft_nm_err
	ft_nm_code=$?
	nm --without-symbol-versions "$@" > nm_out 2> nm_err
	nm_code=$?

	sed -i 's/@GLIBC_[0-9.]*//g' ft_nm_out
    sed -i 's/@GLIBC_[0-9.]*//g' nm_out

	local failed=0
	diff -q ft_nm_out nm_out > /dev/null 2>&1 || failed=1
	diff -q <(sed 's/ft_nm/nm/g' ft_nm_err) nm_err > /dev/null 2>&1 || failed=1
	[ "$ft_nm_code" -ne "$nm_code" ] && failed=1

	if [ "$failed" -eq 0 ]; then
		echo "$label OK"
		PASS=$((PASS + 1))
	else
		echo "$label FAIL"
		diff ft_nm_out nm_out
		diff <(sed 's/ft_nm/nm/g' ft_nm_err) nm_err
		[ "$ft_nm_code" -ne "$nm_code" ] && echo "exit code: $ft_nm_code <> $nm_code"
		FAIL=$((FAIL + 1))
	fi
}

# Creates a minimal valid ELF64 object with one global symbol
make_elf_object() {
	local out="$1"
	local sym="$2"
	# Compile a tiny C file on the fly
	echo "int ${sym}(void) { return (42); }" | cc -x c - -c -o "$out" 2>/dev/null
}

# Creates a minimal static library (.a) from one or more .o files
make_static_lib() {
	local out="$1"
	shift
	ar rcs "$out" "$@" 2>/dev/null
}

# Creates a totally random binary file
make_random_binary() {
	local out="$1"
	dd if=/dev/urandom bs=256 count=1 > "$out" 2>/dev/null
}

# Creates a valid .a with a corrupt internal .o
make_ar_with_corrupt_member() {
	local out="$1"
	# Start with valid archive magic, then a fake member header with garbage data
	printf '!<arch>\n' > "$out"
	# ar member header: name(16) date(12) uid(6) gid(6) mode(8) size(10) end(2)
	printf 'corrupt.o/      0           0     0     644     12        `\n' >> "$out"
	printf '\x7fELF\xff\xff\xff\xff\xff\xff\xff\xff' >> "$out"
}

cleanup() {
	rm -f test_foo.o test_bar.o test_baz.o test_valid.a test_single.a test_big.a \
		test_corrupt_member.a test_random.bin test_text.txt \
		test_empty.txt test_zero.o test_elf32.o test_shared.so test_weak.o \
		test_debug.o test_undef.o test_static_syms.o test_many_syms.o \
		ft_nm_out ft_nm_err nm_out nm_err \
		/tmp/_weak_src.c /tmp/_debug_src.c /tmp/_undef_src.c /tmp/_static_src.c /tmp/_many_syms.c

	TEST_NUM=$(( PASS + FAIL ))

	echo -e "\r    "
	echo "=== Results: $PASS passed, $FAIL failed / $TEST_NUM total ==="

    exit 0
}

# ─────────────────────────────────────────────
# Setup: build test
# ─────────────────────────────────────────────

trap cleanup SIGINT

# Simple ELF objects
make_elf_object test_foo.o foo_symbol
make_elf_object test_bar.o bar_symbol
make_elf_object test_baz.o baz_symbol

# Static libraries
make_static_lib test_valid.a test_foo.o test_bar.o
make_static_lib test_single.a test_baz.o

# Corrupt/edge-case files
make_ar_with_corrupt_member test_corrupt_member.a
make_random_binary test_random.bin
echo "this is plain text" > test_text.txt
echo "" > test_empty.txt
touch test_zero.o   # Zero-byte file

# 32-bit ELF (if toolchain supports it)
echo "int x32 = 1;" | cc -x c - -c -m32 -o test_elf32.o 2>/dev/null || touch test_elf32.o

# Shared object (dynamic)
echo "int shared_fn(void) { return (1); }" | cc -x c - -shared -fPIC -o test_shared.so 2>/dev/null

# ELF with weak symbols
cat > /tmp/_weak_src.c << 'EOF'
__attribute__((weak)) int weak_func(void) { return (0); }
int strong_func(void) { return (1); }
EOF
cc -x c /tmp/_weak_src.c -c -o test_weak.o 2>/dev/null

# ELF with debug symbols
cat > /tmp/_debug_src.c << 'EOF'
static int internal_var = 42;
int public_var = 1;
static void internal_fn(void) {}
void public_fn(void) { internal_fn(); }
EOF
cc -x c /tmp/_debug_src.c -c -g -o test_debug.o 2>/dev/null

# ELF with only undefined symbols (references)
cat > /tmp/_undef_src.c << 'EOF'
extern int external_var;
extern void external_fn(void);
void caller(void) { external_fn(); }
EOF
cc -x c /tmp/_undef_src.c -c -o test_undef.o 2>/dev/null

# ELF with only static/local symbols (no externals)
cat > /tmp/_static_src.c << 'EOF'
static int only_local = 7;
static void only_static_fn(void) { (void)only_local; }
void trigger(void) { only_static_fn(); }
EOF
cc -x c /tmp/_static_src.c -c -o test_static_syms.o 2>/dev/null

# ELF with many symbols
python3 -c "
import sys
lines = []
for i in range(50):
    lines.append(f'int symbol_{i:03d} = {i};')
print('\n'.join(lines))
" > /tmp/_many_syms.c 2>/dev/null
cc -x c /tmp/_many_syms.c -c -o test_many_syms.o 2>/dev/null

# .a with multiple members including one with many symbols
make_static_lib test_big.a test_foo.o test_bar.o test_baz.o test_debug.o test_weak.o 2>/dev/null

echo "=== Running tests ==="
echo ""

# ─────────────────────────────────────────────
# SECTION 1 — Default behavior (no flags)
# ─────────────────────────────────────────────

echo "--- [1] Default (no flags) ---"
run_nm
run_nm ./ft_nm
run_nm /bin/ls
run_nm /bin/date
run_nm /usr/bin/gcc 2>/dev/null || run_nm /usr/bin/cc
run_nm test_foo.o
run_nm test_bar.o
run_nm test_debug.o
run_nm test_weak.o
run_nm test_undef.o
run_nm test_static_syms.o
run_nm test_many_syms.o
run_nm test_valid.a
run_nm test_single.a
run_nm test_big.a
run_nm /usr/lib/x86_64-linux-gnu/libc.a
run_nm /usr/lib/x86_64-linux-gnu/libm.a

# ─────────────────────────────────────────────
# SECTION 2 — Multiple files (no flags)
# ─────────────────────────────────────────────

echo ""
echo "--- [2] Multiple files, no flags ---"
run_nm test_foo.o test_bar.o
run_nm test_foo.o test_bar.o test_baz.o
run_nm test_foo.o test_valid.a
run_nm test_debug.o test_weak.o test_undef.o
run_nm ./ft_nm /bin/ls /bin/date
run_nm ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a
run_nm /bin/ls /usr/lib/x86_64-linux-gnu/libm.a
run_nm /usr/lib/x86_64-linux-gnu/libc.a /usr/lib/x86_64-linux-gnu/libm.a
run_nm test_valid.a test_single.a
run_nm test_big.a /usr/lib/x86_64-linux-gnu/libc.a

# ─────────────────────────────────────────────
# SECTION 3 — Invalid / edge-case files
# ─────────────────────────────────────────────

echo ""
echo "--- [3] Invalid / edge-case files ---"
run_nm noexiste.o
run_nm /nonexistent/path/file.o
run_nm test_text.txt
run_nm test_empty.txt
run_nm test_zero.o
run_nm test_random.bin
run_nm test_corrupt_member.a
run_nm Makefile
run_nm src
run_nm /tmp
run_nm /dev/null

# ─────────────────────────────────────────────
# SECTION 4 — Mixed valid + invalid
# ─────────────────────────────────────────────

echo ""
echo "--- [4] Mixed valid + invalid ---"
run_nm ./ft_nm noexiste.o
run_nm noexiste.o ./ft_nm
run_nm test_foo.o noexiste.o test_bar.o
run_nm test_valid.a noexiste.o
run_nm ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a Makefile
run_nm /bin/ls /usr/lib/x86_64-linux-gnu/libm.a noexiste.o src
run_nm test_text.txt test_foo.o
run_nm test_zero.o test_valid.a /bin/ls

# ─────────────────────────────────────────────
# SECTION 5 — Flag: -g / --extern-only
# ─────────────────────────────────────────────

echo ""
echo "--- [5] -g / --extern-only ---"
run_nm -g ./ft_nm
run_nm -g test_foo.o
run_nm -g test_debug.o
run_nm -g test_weak.o
run_nm -g test_undef.o
run_nm -g test_static_syms.o
run_nm -g test_valid.a
run_nm -g /usr/lib/x86_64-linux-gnu/libc.a
run_nm --extern-only ./ft_nm
run_nm --extern-only test_debug.o
run_nm -g ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a
run_nm -g test_foo.o test_bar.o

# ─────────────────────────────────────────────
# SECTION 6 — Flag: -A / --print-file-name
# ─────────────────────────────────────────────

echo ""
echo "--- [6] -A / --print-file-name ---"
run_nm -A ./ft_nm
run_nm -A test_foo.o
run_nm -A test_debug.o
run_nm -A test_valid.a
run_nm -A /usr/lib/x86_64-linux-gnu/libc.a
run_nm --print-file-name test_foo.o
run_nm -A ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a /bin/ls
run_nm -A test_foo.o noexiste.o test_bar.o
run_nm -A test_valid.a test_single.a

# ─────────────────────────────────────────────
# SECTION 7 — Flag: -n / --numeric-sort
# ─────────────────────────────────────────────

echo ""
echo "--- [7] -n / --numeric-sort ---"
run_nm -n ./ft_nm
run_nm -n test_foo.o
run_nm -n test_many_syms.o
run_nm -n test_debug.o
run_nm -n test_undef.o
run_nm -n test_valid.a
run_nm -n /usr/lib/x86_64-linux-gnu/libc.a
run_nm --numeric-sort test_debug.o
run_nm -n ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a Makefile

# ─────────────────────────────────────────────
# SECTION 8 — Flag: -r / --reverse-sort
# ─────────────────────────────────────────────

echo ""
echo "--- [8] -r / --reverse-sort ---"
run_nm -r ./ft_nm
run_nm -r test_foo.o
run_nm -r test_many_syms.o
run_nm -r test_debug.o
run_nm -r test_valid.a
run_nm -r /usr/lib/x86_64-linux-gnu/libc.a
run_nm --reverse-sort test_debug.o
run_nm -r test_foo.o test_bar.o

# ─────────────────────────────────────────────
# SECTION 9 — Flag: -u / --undefined-only
# ─────────────────────────────────────────────

echo ""
echo "--- [9] -u / --undefined-only ---"
run_nm -u ./ft_nm
run_nm -u test_foo.o
run_nm -u test_undef.o
run_nm -u test_debug.o
run_nm -u test_valid.a
run_nm -u /usr/lib/x86_64-linux-gnu/libc.a
run_nm --undefined-only test_undef.o
run_nm -u /usr/lib/x86_64-linux-gnu/libc.a /bin/ls
run_nm -u test_foo.o noexiste.o

# ─────────────────────────────────────────────
# SECTION 10 — Flag: -U / --defined-only
# ─────────────────────────────────────────────

echo ""
echo "--- [10] -U / --defined-only ---"
run_nm -U ./ft_nm
run_nm -U test_foo.o
run_nm -U test_undef.o
run_nm -U test_debug.o
run_nm -U test_valid.a
run_nm -U /usr/lib/x86_64-linux-gnu/libc.a
run_nm --defined-only test_debug.o
run_nm -U test_foo.o test_undef.o

# ─────────────────────────────────────────────
# SECTION 11 — Flag: -p / --no-sort
# ─────────────────────────────────────────────

echo ""
echo "--- [11] -p / --no-sort ---"
run_nm -p ./ft_nm
run_nm -p test_foo.o
run_nm -p test_many_syms.o
run_nm -p test_debug.o
run_nm -p test_valid.a
run_nm -p /usr/lib/x86_64-linux-gnu/libc.a
run_nm --no-sort test_debug.o
run_nm -p test_foo.o test_bar.o

# ─────────────────────────────────────────────
# SECTION 12 — Flag: -a / --debug-syms
# ─────────────────────────────────────────────

echo ""
echo "--- [12] -a / --debug-syms ---"
run_nm -a ./ft_nm
run_nm -a test_foo.o
run_nm -a test_debug.o
run_nm -a test_valid.a
run_nm -a /usr/lib/x86_64-linux-gnu/libc.a
run_nm --debug-syms test_debug.o
run_nm -a test_foo.o test_bar.o

# ─────────────────────────────────────────────
# SECTION 13 — Flag: -D / --dynamic
# ─────────────────────────────────────────────

echo ""
echo "--- [13] -D / --dynamic ---"
run_nm -D ./ft_nm
run_nm -D /bin/ls
run_nm -D /usr/lib/x86_64-linux-gnu/libc.so.6 2>/dev/null || true
run_nm -D test_shared.so 2>/dev/null
run_nm --dynamic /bin/ls
run_nm -D test_foo.o          # Static object — should show nothing or error
run_nm -D test_valid.a        # Archive — undefined behavior on most nm

# ─────────────────────────────────────────────
# SECTION 14 — Flag: -W / --no-weak
# ─────────────────────────────────────────────

echo ""
echo "--- [14] -W / --no-weak ---"
run_nm -W test_weak.o
run_nm -W ./ft_nm
run_nm -W test_foo.o
run_nm -W test_debug.o
run_nm -W test_valid.a
run_nm --no-weak test_weak.o
run_nm -W test_weak.o test_foo.o

# ─────────────────────────────────────────────
# SECTION 15 — Flag combinations (2 flags)
# ─────────────────────────────────────────────

echo ""
echo "--- [15] Flag combinations (2 flags) ---"
run_nm -g -u ./ft_nm
run_nm -g -n test_debug.o
run_nm -g -r test_debug.o
run_nm -g -p test_debug.o
run_nm -g -a test_debug.o
run_nm -g -A test_valid.a
run_nm -g -U test_debug.o
run_nm -g -W test_weak.o
run_nm -n -r test_many_syms.o
run_nm -n -A test_debug.o
run_nm -r -A test_debug.o
run_nm -u -A ./ft_nm
run_nm -U -A test_debug.o
run_nm -u -n test_undef.o
run_nm -u -r test_undef.o
run_nm -p -A test_debug.o
run_nm -a -g test_debug.o
run_nm -a -n test_debug.o
run_nm -a -u test_debug.o
run_nm -D -A /bin/ls
run_nm -W -g test_weak.o
run_nm -W -u test_weak.o
run_nm -W -n test_weak.o

# ─────────────────────────────────────────────
# SECTION 16 — Flag combinations (3 flags)
# ─────────────────────────────────────────────

echo ""
echo "--- [16] Flag combinations (3 flags) ---"
run_nm -r -n ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a
run_nm -g -n -r test_debug.o
run_nm -g -n -A test_valid.a
run_nm -g -r -A test_debug.o
run_nm -u -n -A ./ft_nm
run_nm -u -r -A test_undef.o
run_nm -g -u -A ./ft_nm
run_nm -a -g -n test_debug.o
run_nm -a -g -u test_debug.o
run_nm -W -g -n test_weak.o
run_nm -W -g -A test_weak.o
run_nm -p -g -A test_debug.o
run_nm -U -g -A test_debug.o
run_nm -n -r -A test_many_syms.o

# ─────────────────────────────────────────────
# SECTION 17 — Flag combinations (4+ flags)
# ─────────────────────────────────────────────

echo ""
echo "--- [17] Flag combinations (4+ flags) ---"
run_nm -g -n -r -A test_debug.o
run_nm -a -g -n -A test_debug.o
run_nm -W -g -n -A test_weak.o
run_nm -g -n -r -A ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a
run_nm -u -g -n -r test_debug.o
run_nm -a -g -u -A test_debug.o
run_nm -W -g -n -r -A test_weak.o

# ─────────────────────────────────────────────
# SECTION 18 — Long flag equivalents
# ─────────────────────────────────────────────

echo ""
echo "--- [18] Long flags ---"
run_nm --extern-only --numeric-sort test_debug.o
run_nm --extern-only --reverse-sort test_debug.o
run_nm --extern-only --print-file-name test_debug.o
run_nm --undefined-only --print-file-name test_undef.o
run_nm --defined-only --print-file-name test_debug.o
run_nm --no-sort --print-file-name test_debug.o
run_nm --debug-syms --extern-only test_debug.o
run_nm --no-weak --extern-only test_weak.o
run_nm --numeric-sort --reverse-sort test_many_syms.o
run_nm --extern-only --numeric-sort --print-file-name test_valid.a

# ─────────────────────────────────────────────
# SECTION 19 — Corrupt / edge-case + flags
# ─────────────────────────────────────────────

echo ""
echo "--- [19] Corrupt files + flags ---"
run_nm -u test_corrupt_member.a
run_nm -g test_random.bin
run_nm -A test_text.txt
run_nm -g test_zero.o
run_nm -n noexiste.o
run_nm -A noexiste.o
run_nm -g test_empty.txt

# ─────────────────────────────────────────────
# SECTION 20 — .a with flags + multiple files
# ─────────────────────────────────────────────

echo ""
echo "--- [20] .a + multiple files + flags ---"
run_nm -g ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a
run_nm -A ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a /bin/ls
run_nm -n ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a Makefile
run_nm -u /usr/lib/x86_64-linux-gnu/libc.a /bin/ls
run_nm -r -n ./ft_nm /usr/lib/x86_64-linux-gnu/libc.a
run_nm -g -A test_valid.a test_single.a
run_nm -n -A test_valid.a test_big.a
run_nm -u -A test_valid.a /usr/lib/x86_64-linux-gnu/libc.a
run_nm -W -A test_big.a test_weak.o
run_nm -g -n -A test_valid.a /usr/lib/x86_64-linux-gnu/libc.a

# ─────────────────────────────────────────────
# SECTION 21 — 32-bit ELF (if available)
# ─────────────────────────────────────────────

echo ""
echo "--- [21] 32-bit ELF ---"
run_nm test_elf32.o
run_nm -g test_elf32.o
run_nm -n test_elf32.o
run_nm -A test_elf32.o
run_nm -u test_elf32.o

# ─────────────────────────────────────────────
# SECTION 22 — Shared objects (.so)
# ─────────────────────────────────────────────

echo ""
echo "--- [22] Shared objects ---"
run_nm test_shared.so
run_nm -D test_shared.so
run_nm -g test_shared.so
run_nm -u test_shared.so
run_nm -A test_shared.so
run_nm -D /lib/x86_64-linux-gnu/libc.so.6 2>/dev/null || run_nm -D /usr/lib/x86_64-linux-gnu/libc.so.6 2>/dev/null

# ─────────────────────────────────────────────
# SECTION 23 — Repeated flags (should not crash)
# ─────────────────────────────────────────────

echo ""
echo "--- [23] Repeated flags ---"
run_nm -g -g test_foo.o
run_nm -n -n test_foo.o
run_nm -r -r test_foo.o
run_nm -A -A test_foo.o
run_nm -u -u test_foo.o
run_nm -g -g -n -n test_debug.o

# ─────────────────────────────────────────────
# SECTION 24 — Conflicting flags
# ─────────────────────────────────────────────

echo ""
echo "--- [24] Conflicting / override flags ---"
run_nm -u -U test_debug.o      # undefined-only vs defined-only
run_nm -n -p test_many_syms.o  # numeric-sort vs no-sort
run_nm -r -p test_many_syms.o  # reverse-sort vs no-sort

# ─────────────────────────────────────────────
# SECTION 25 — Stress: many object files
# ─────────────────────────────────────────────

echo ""
echo "--- [25] Many files at once ---"
run_nm test_foo.o test_bar.o test_baz.o test_debug.o test_weak.o test_undef.o test_static_syms.o test_many_syms.o
run_nm -g test_foo.o test_bar.o test_baz.o test_debug.o test_weak.o test_undef.o test_many_syms.o
run_nm -A test_foo.o test_bar.o test_baz.o test_debug.o test_weak.o test_undef.o test_many_syms.o
run_nm -n test_foo.o test_bar.o test_baz.o test_debug.o test_weak.o test_many_syms.o
run_nm -n -r test_foo.o test_bar.o test_baz.o test_debug.o test_weak.o test_many_syms.o

# ─────────────────────────────────────────────
# Cleanup
# ─────────────────────────────────────────────

rm -f ft_nm test_foo.o test_bar.o test_baz.o test_valid.a test_single.a test_big.a \
	test_corrupt_member.a test_random.bin test_text.txt \
	test_empty.txt test_zero.o test_elf32.o test_shared.so test_weak.o \
	test_debug.o test_undef.o test_static_syms.o test_many_syms.o \
	ft_nm_out ft_nm_err nm_out nm_err \
	/tmp/_weak_src.c /tmp/_debug_src.c /tmp/_undef_src.c /tmp/_static_src.c /tmp/_many_syms.c

echo ""
echo "=== Results: $PASS passed, $FAIL failed / $TEST_NUM total ==="
