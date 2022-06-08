#!/usr/bin/env bats

load test_helper

@test "gratuit" {
	true
}

@test "test.txt" {
	run ./teepee touch test.txt % echo "this is test.txt" >> test.txt % cat; rm test.txt
	check 0 "this is test.txt"
	expectout 0
}

@test "test.txt -s" {
	run ./teepee -s mazeltov touch test.txt mazeltov echo "this is test.txt" >> test.txt mazeltov cat; rm test.txt
	check 0 "this is test.txt"
	expectout 0
}

@test "test.txt -o" {
	run ./teepee -o out touch test.txt % echo "this is test.txt" >> test.txt % cat; rm test.txt
	check 0 "this is test.txt"
	expectout 0
	diff -u - out.2 <<<"this is test.txt"
}

@test "out sep hello inverse" {
	run ./teepee -s monde -o "$OUT/xx" echo bonjour le monde wc -w;
	check 0 "2"
	expectout 1
	diff -u - "$OUT/xx.1" <<< "bonjour le"
}

@test "bonjour le monde % wc -r" {
	run ./teepee -r test.txt echo bonjour le monde % wc -c
	diff -u - test.txt <<< "17"
}

@test "bonjour le monde % wc -r -o -s" {
	run ./teepee -r test.txt -o xx -s anticonstitutionellement echo bonjour le monde anticonstitutionellement wc -c
	diff -u - test.txt <<< "17"
	diff -u - xx.1 <<< "bonjour le monde"
}

@test "bonjour le monde % wc -r -s -o" {
	run ./teepee -r test.txt -s anticonstitutionellement -o xx echo bonjour le monde anticonstitutionellement wc -c
	diff -u - test.txt <<< "17"
	diff -u - xx.1 <<< "bonjour le monde"
}

@test "bonjour le monde % wc -s -r -o" {
	run ./teepee -s anticonstitutionellement -r test.txt -o xx echo bonjour le monde anticonstitutionellement wc -c
	diff -u - test.txt <<< "17"
	diff -u - xx.1 <<< "bonjour le monde"
}

@test "bonjour le monde % wc -s -o -r" {
	run ./teepee -s anticonstitutionellement -o xx -r test.txt echo bonjour le monde anticonstitutionellement wc -c
	diff -u - test.txt <<< "17"
	diff -u - xx.1 <<< "bonjour le monde"
}

@test "bonjour le monde % wc -o -s -r" {
	run ./teepee -o xx -s anticonstitutionellement -r test.txt echo bonjour le monde anticonstitutionellement wc -c
	diff -u - test.txt <<< "17"
	diff -u - xx.1 <<< "bonjour le monde"
}

@test "bonjour le monde % wc -o -r -s" {
	run ./teepee -o xx -r test.txt -s anticonstitutionellement echo bonjour le monde anticonstitutionellement wc -c
	diff -u - test.txt <<< "17"
	diff -u - xx.1 <<< "bonjour le monde"
}

