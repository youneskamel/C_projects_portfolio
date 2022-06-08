#!/usr/bin/env bats

load test_helper

@test "gratuit" {
	true
}

@test "touch" {
	run ./extra touch test.txt
	checki 0 <<FIN
/bin/touch
FIN
}

@test "mkdir" {
	run ./extra mkdir test_dir
	checki 0 <<FIN
/bin/mkdir
FIN
}

@test "touch option -v" {
	run ./extra -v touch test2.txt
	test "$status" == 0
}







