# Aide pour les tests bats

# Comme il y a de la concurence et des risques de blocages le setup et teardown sont un peu
# Compliqués car il faut préparer des mecanises de timout er nettoyer du mieux possible les processus qui s'echappent.
# De plus, les versions < 1.0.0 de bats échouent et bloquent sur des tests.

check() {
	res=0
	if [ "$status" != "$1" ]; then
		echo "Test échoué: le code de retour est $status ; $1 était attentu"
		echo "La sortie est:"
		echo "$output"
		res=1
	fi

	if [ "$output" != "$2" ]; then
		echo "Test échoué: la sortie inattendue est"
		echo "$output"
		echo ""
		echo "Le diff est"
		diff -u <(echo "$2") <(echo "$output")
		res=2
	fi

	return "$res"
}

checki() {
	out=`cat`
	check "$1" "$out"
}

setup() {
	true
}

teardown() {
	kill %% 2> /dev/null
	pchild=`pgrep -s 0 sleep`
	kill $pchild 2> /dev/null
	true
}

# Lancer les tests avec `BATS_VALGRIND` active la detection des fuites
if [[ -n "$BATS_VALGRIND" ]]; then
	eval orig_"$(declare -f run)"
	run() {
		orig_run valgrind -q --leak-check=full "$@"
	}
fi
