#!/usr/bin/env bats

load test_helper

@test "bonjour" {
	run ./extra echo bonjour le monde
	checki 0 <<FIN
/bin/echo
bonjour le monde
FIN
}

@test "script" {
	run ./extra ./script.sh
	checki 0 <<FIN
/bin/bash
hello world
FIN
}

@test "nice" {
	run ./extra nice echo bonjour le monde
	checki 0 <<FIN
/usr/bin/nice
/bin/echo
bonjour le monde
FIN
}

@test "env, nice, etc." {
	run ./extra env M=monde nice -n 10 bash -c 'echo bonjour le $M; exec echo au revoir le $M'
	checki 0 <<FIN
/usr/bin/env
/usr/bin/nice
/bin/bash
bonjour le monde
/bin/echo
au revoir le monde
FIN
}

@test "time" {
	run ./extra time -p echo bonjour le monde
	checki 0 <<FIN
/usr/bin/time
/bin/echo
bonjour le monde
real 0.00
user 0.00
sys 0.00
FIN
}


@test "env, time, etc." {
	run ./extra env B=ruojnob time -p dash -c 'nice echo $B | rev'
	output=$(echo "$output" | LC_ALL=C sort)
        checki 0 <<FIN
/bin/dash
/bin/echo
/usr/bin/env
/usr/bin/nice
/usr/bin/rev
/usr/bin/time
bonjour
real 0.00
sys 0.00
user 0.00
FIN
}

@test "man" {
	run bash -c './extra man -Tps man > man.ps'
	output=$(echo "$output" | LC_ALL=C sort)
        checki 0 <<FIN
/usr/bin/groff
/usr/bin/grops
/usr/bin/man
/usr/bin/preconv
/usr/bin/tbl
/usr/bin/troff
FIN
}

@test "trap" {
	run ./extra ./trap.sh
	checki 0 <<FIN
/bin/bash
traped!
FIN
}

@test "exit" {
	run ./extra bash -c 'exit 10'
	checki 10 <<FIN
/bin/bash
FIN
}

@test "kill" {
	run ./extra bash -c 'kill $$'
	checki 143 <<FIN
/bin/bash
FIN
}

@test "fail" {
	run ./extra failfailfail
	test "$status" == 127
	test "$output" != ""
}
