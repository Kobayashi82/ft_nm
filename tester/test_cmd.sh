cp ft_nm tester/ft_nm
cd tester

run_nm() {
	local label="TEST (${*:-no args})"

	./ft_nm "$@" > ft_out 2> ft_err; local ft_code=$?
	nm "$@" > nm_out 2> nm_err; local nm_code=$?

	# Normaliza sufijos de versión GLIBC
	sed -i 's/@GLIBC_[0-9.]*//g' ft_out nm_out

	if diff -q ft_out nm_out >/dev/null && diff -q <(sed 's/^ft_nm:/nm:/' ft_err) nm_err >/dev/null && [ "$ft_code" -eq "$nm_code" ]; then
		echo "$label OK"
	else
		echo "$label FAIL"
		diff ft_out nm_out
		diff <(sed 's/^ft_nm:/nm:/' ft_err) nm_err
		[ "$ft_code" -ne "$nm_code" ] && echo "exit code: $ft_code <> $nm_code"
	fi

	rm -f ft_out ft_err nm_out nm_err
}

run_nm "$@"
rm ft_nm
