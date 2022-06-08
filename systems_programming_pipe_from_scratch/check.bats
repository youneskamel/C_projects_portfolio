#!/usr/bin/env bats

load test_helper

@test "bonjour" {
	run ./teepee echo bonjour le monde
	check 0 "bonjour le monde"
	expectout 0
}


@test "bonjour % wc" {
	run ./teepee echo bonjour le monde % wc -c
	check 0 "17"
	expectout 0
}

@test "bonjour stdout" {
	run ./teepee bash -c 'echo bonjour le monde >&2' % true
	check 0 "bonjour le monde"
	expectout 0
}

@test "out bonjour % wc" {
	run ./teepee -o "$OUT/xx" echo bonjour le monde % wc -c
	check 0 "17"
	expectout 1
	diff -u - "$OUT/xx.1" <<<"bonjour le monde"
}

@test "fable4" {
	run ./teepee cat dir/fable.txt % grep -v le % grep un % wc -l
	check 0 "3"
	expectout 0
}

@test "out fable4" {
	run ./teepee -o "$OUT/yy" cat dir/fable.txt % grep -v le % grep un % wc -l
	check 0 "3"
	expectout 3
	diff -u - "$OUT/yy.1" <<FIN
Maître Corbeau, sur un arbre perché,
Tenait en son bec un fromage.
Maître Renard, par l'odeur alléché,
Lui tint à peu près ce langage :
"Hé ! bonjour, Monsieur du Corbeau.
Que vous êtes joli ! que vous me semblez beau !
Sans mentir, si votre ramage
Se rapporte à votre plumage,
Vous êtes le Phénix des hôtes de ces bois. "
A ces mots le Corbeau ne se sent pas de joie ;
Et pour montrer sa belle voix,
Il ouvre un large bec, laisse tomber sa proie
Le Renard s'en saisit, et dit : "Mon bon Monsieur,
Apprenez que tout flatteur
Vit aux dépens de celui qui l'écoute :
Cette leçon vaut bien un fromage, sans doute. "
Le Corbeau, honteux et confus,
FIN

	diff -u - "$OUT/yy.2" <<FIN
Maître Corbeau, sur un arbre perché,
Tenait en son bec un fromage.
Maître Renard, par l'odeur alléché,
Lui tint à peu près ce langage :
"Hé ! bonjour, Monsieur du Corbeau.
Sans mentir, si votre ramage
Se rapporte à votre plumage,
Il ouvre un large bec, laisse tomber sa proie
Le Renard s'en saisit, et dit : "Mon bon Monsieur,
Apprenez que tout flatteur
Vit aux dépens de celui qui l'écoute :
Le Corbeau, honteux et confus,
FIN

	diff -u - "$OUT/yy.3" <<FIN
Maître Corbeau, sur un arbre perché,
Tenait en son bec un fromage.
Il ouvre un large bec, laisse tomber sa proie
FIN
}

@test "sep hello" {
	run ./teepee -s . echo bonjour le monde . wc -w
	check 0 "3"
	expectout 0
}

@test "out sep hello" {
	run ./teepee -o "$OUT/xx" -s monde echo bonjour le monde wc -w
	check 0 "2"
	expectout 1
	diff -u - "$OUT/xx.1" <<< "bonjour le"
}

@test "sep teepee" {
	run ./teepee -s . \
		./teepee cat dir/fable.txt % grep -v le . \
		wc -l
	check 0 "12"
	expectout 0
}

@test "out sep teepee" {
	run ./teepee -o "$OUT/xx" -s . \
		./teepee -o "$OUT/yy" cat dir/fable.txt % grep le . \
		./teepee -o "$OUT/zz" -s .. grep de .. wc -l
	check 0 "2"
	expectout 3

	diff -u - "$OUT/xx.1" <<FIN
Que vous êtes joli ! que vous me semblez beau !
Vous êtes le Phénix des hôtes de ces bois. "
A ces mots le Corbeau ne se sent pas de joie ;
Et pour montrer sa belle voix,
Cette leçon vaut bien un fromage, sans doute. "
FIN

	diff -u dir/fable.txt "$OUT/yy.1"

	diff -u - "$OUT/zz.1" <<FIN
Vous êtes le Phénix des hôtes de ces bois. "
A ces mots le Corbeau ne se sent pas de joie ;
FIN
}

@test "retour" {
	run ./teepee sh -c 'echo hello; exit 1' % sh -c 'cat; exit 2' % sh -c 'cat; exit 3'
	check 3 "hello"
	expectout 0
}

@test "fable big" {
	run ./teepee cat dir/fable.txt % cat % cat % cat % cat % cat % sort % cat % cat % cat % cat % cat % sort -r % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % wc -l
	check 0 "17"
	expectout 0
}

@test "out fable big" {
	run ./teepee -o "$OUT/xx" cat dir/fable.txt % cat % cat % cat % cat % cat % sort % cat % cat % cat % cat % cat % sort -r % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % cat % wc -l
	check 0 "17"
	expectout 61
	diff -u dir/fable.txt "$OUT/xx.1" 
}

@test "para2" {
	run ./teepee \
		bash -c 'sleep 0.1; echo deux >&2; sleep 0.2; echo quatre >&2' % \
		bash -c 'echo un >&1; sleep 0.2; echo trois >&2; sleep 0.2; echo cinq >&2'
	checki 0 <<FIN
un
deux
trois
quatre
cinq
FIN
	expectout 0
}

@test "out para2" {
	run ./teepee -o "$OUT/xx" \
		bash -c 'sleep 0.1; echo deux >&2; sleep 0.2; echo quatre >&2; echo hello' % \
		bash -c 'echo un >&1; sleep 0.2; echo trois >&2; sleep 0.2; echo cinq >&2'
	checki 0 <<FIN
un
deux
trois
quatre
cinq
FIN
	expectout 1
	diff -u - "$OUT/xx.1" <<< "hello"
}

@test "para4" {
	run ./teepee \
		bash -c 'echo hello; sleep 0.3; echo trois >&2; sleep 0.1; echo quatre >&2' % \
		bash -c 'sleep 0.2; echo deux >&2;  sleep 0.3; echo cinq >&2; cat' % \
		bash -c 'sleep 0.0; echo zero >&2;  sleep 0.7; echo sept >&2; cat' % \
		bash -c 'sleep 0.1; echo un >&1;    sleep 0.5; echo six >&2; cat'
	checki 0 <<FIN
zero
un
deux
trois
quatre
cinq
six
sept
hello
FIN
	expectout 0
}

@test "out para4" {
	run ./teepee -o "$OUT/xx" \
		bash -c 'echo hello; sleep 0.3; echo trois >&2; sleep 0.1; echo quatre >&2' % \
		bash -c 'sleep 0.2; echo deux >&2;  sleep 0.3; echo cinq >&2; cat' % \
		bash -c 'sleep 0.0; echo zero >&2;  sleep 0.7; echo sept >&2; cat' % \
		bash -c 'sleep 0.1; echo un >&1;    sleep 0.5; echo six >&2; cat'
	checki 0 <<FIN
zero
un
deux
trois
quatre
cinq
six
sept
hello
FIN
	expectout 3
	diff -u - "$OUT/xx.1" <<< "hello"
	diff -u - "$OUT/xx.2" <<< "hello"
	diff -u - "$OUT/xx.3" <<< "hello"
}

@test "broken pipe" {
	run ./teepee cat /dev/urandom % strings % head -n 5 % wc -l
	check 0 "5"
	expectout 0
}

@test "long broken pipe" {
	run ./teepee cat /dev/urandom % strings % head -n 1000000 % wc -l
	check 0 "1000000"
	expectout 0
}

@test "full pipe" {
	run ./teepee -o "$OUT/xx" seq 1000000 % bash -c 'sleep 0.2; head -n 10000; sleep 0.1' % wc -l
	check 0 "10000"
	expectout 2
	grep -q 20000 "$OUT/xx.1"
}
