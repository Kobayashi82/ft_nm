#!/bin/bash

cp ft_nm tester/ft_nm
cd tester
ft_nm=./ft_nm
# ft_nm="valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes -q ./ft_nm"
TEST_NUM=0

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
		rm -f ft_nm_out ft_nm_err nm_out nm_err out_diff
    else
        echo "$label FAIL"
        diff ft_nm_out nm_out > out_diff
        diff <(sed 's/ft_nm/nm/g' ft_nm_err) nm_err
        [ "$ft_nm_code" -ne "$nm_code" ] && echo "exit code: $ft_nm_code <> $nm_code"
    fi
}

run_nm "$@"
