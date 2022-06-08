# TP1 - Extra ça trace exec

L'objectif du TP1 est de développer l'utilitaire `extra` capable de surveriller l'exécution d'un programme et ses utilisations de l'appel système `execve`.
Le TP sert aussi de contexte pour vous faire utiliser un appel système bas niveau (et très puissant) `ptrace`.

## Description de l'outil `extra`

```
extra commande [arguments...]
```

`extra` (*exec trace*) affiche les chemins complets des programmes exécutés par une commande et ses sous-processus.

À chaque fois qu'un `exec` a lieu, le chemin complet du nouvel exécutable est affiché sur la **sortie standard d'erreur**.

`extra` peut donc servir à déterminer facilement les programmes supplémentaires utilisés lors de l'exécution d'un programme ou d'un script.

Par exemple, `extra` peut lister les programmes utilisés pour la génération d'une manpage au format postscript.

```
$ extra man -Tps man > man.ps
/usr/bin/man
/usr/bin/preconv
/usr/bin/tbl
/usr/bin/groff
/usr/bin/troff
/usr/bin/grops
```

### Détail des fonctionalités

Comme `extra` affiche les chemins des programmes, le premier chemin affiché par `extra` est nécessairement celui du programme de la commande.

```
$ extra echo bonjour le monde
/bin/echo
bonjour le monde
```

Attention, ce n'est pas l'argument de l'appel système `exec` qui est affiché, mais le chemin du programme ultimement exécuté.
Ainsi, par exemple, en exécutant un script c'est le chemin de l'interpréteur qui sera affiché.

```
$ extra ./script.sh
/bin/bash
hello world
```

Si la commande exécutée fait des `exec`, ceux-ci sont également affichés au fur et à mesure.

```
$ extra nice echo bonjour le monde
/usr/bin/nice
/bin/echo
bonjour le monde
```

L'exemple suivant est un peu plus compliqué. Notez que le premier `echo` est une commande interne du shell (donc aucun `exec` n'a lieu) alors que le second `echo` correspond au programme autonome `/bin/echo`.


```
$ extra env M=monde nice -n 10 bash -c 'echo bonjour le $M; exec echo au revoir le $M'
/usr/bin/env
/usr/bin/nice
/bin/bash
bonjour le monde
/bin/echo
au revoir le monde
```

Les sous-processus (fork) sont également automatiquement suivis.

```
$ extra time -p echo bonjour le monde
/usr/bin/time
/bin/echo
bonjour le monde
real 0.00
user 0.00
sys 0.00
```

L'exemple suivant est un peu plus complexe:

```
$ extra env B=ruojnob time -p dash -c 'nice echo $B | rev'
/usr/bin/env
/usr/bin/time
/bin/dash
/usr/bin/nice
/usr/bin/rev
/bin/echo
bonjour
real 0.00
user 0.00
sys 0.00
```

### Code de retour

Le code de retour de `extra` est celui de la commande executée.
Si la commande ne peut pas être exécutée, le code de retour est 127.
Si la commande est terminée par un signal, le code de retour est 128 + le numéro du signal.

## Directives d'implémentation

### Appel système `ptrace`

L'appel système principal que vous devez utiliser est `ptrace`.
C'est un appel système bas niveau offert par le noyau Linux (et la plupart des systèmes Unix) qui sert à implémenter les débogueurs (comme `gdb`) ou d'autres outils comme `strace`.
Il y a beaucoup de magie bas niveau et le détail de la magie sera un peu plus survolé dans INF600C.
Dans le cadre du TP on reste à une utilisation très superficielle de `ptrace`.

`ptrace` est un appel système très puissant, car il permet à un processus de prendre le contrôle d'un autre processus, y compris sa mémoire et le code machine qu'il exécute.

La logique principale de `ptrace` c'est que le processus surveillé (*ptracé*) s'exécute normalement jusqu'à ce qu'il se passe un événement particulier (réception d'un signal, appel système, etc.).
À ce moment-là, le processus surveillé passe dans un état bloqué particulier (*ptrace stopped*) qui apparait sous un `t` avec la commande `ps`.

Le processus qui surveille (le traceur) est notifié que le tracé est arrêté avec l'appel système `wait`.
Plutôt que de créer un appel système dédié, les concepteurs ont réutilisé cet appel système, ce qui peut être déroutant.

### Directives d'utilisation de ptrace

Dans le cadre du TP, seul le sous-ensemble suivant des requêtes `ptrace` devra être utilisé

* `PTRACE_TRACEME` pour tracer la commande.
* `PTRACE_SETOPTIONS` avec les flags `PTRACE_O_EXITKILL`, `PTRACE_O_TRACEEXEC`, `PTRACE_O_TRACEFORK` et `PTRACE_O_TRACECLONE`.
* `PTRACE_CONT` pour faire continuer l'exécution du processus surveillé stoppé.

Pour déterminer le chemin des exécutables, consultez `/proc/PID/exe` avec l'appel système `readlink`.
Avec `PTRACE_O_TRACEEXEC`, le processus surveillé est stoppé après que l'exécutable a été chargé (et donc `/proc/PID/exe` est celui du nouvel exécutable).

Voici quelques conseils pour réussir à développer ce programme.

* Lisez la page de manuel de `ptrace`. Elle est dense et tout ne vous sera pas utile, mais il ne faut pas ignoner les parties qui le seront.
* Évitez les documentations tierces (forum, blog, projets gitbug, etc.) qui contiennent beaucoup d'imprécisions voire de bêtises.
* Commencez par `PTRACE_O_EXITKILL`, `PTRACE_O_TRACEEXEC` et ajoutez `PTRACE_O_TRACEFORK` et `PTRACE_O_TRACECLONE` dans un second temps.
* Attention, `PTRACE_TRACEME` ne stoppe pas l'appelant, vous pouvez donc faire `raise(SIGSTOP);` juste après (RTFM pour les détails)
* Attention, remettez le signal éventuel dans `PTRACE_CONT` (RTFM pour les détails).
* [Lisez](https://twitter.com/jcsrb/status/1392459191353286656) la [page](https://xkcd.com/293/) de [manuel](https://www.commitstrip.com/en/2015/06/29/as-the-last-resort/) de [`ptrace`](https://manpages.debian.org/buster/manpages-dev/ptrace.2.en.html).

### Autres directives

Vous devez développer le programme en C.
Le fichier source doit s'appeler `extra.c` et être à la racine du dépôt.
Vu la taille du projet, tout doit rentrer dans ce seul fichier source.

Comme le TP n'est pas si gros (de l'ordre de grandeur d'une ou deux centaines de lignes), il est attendu un effort important sur le soin du code et la gestion des cas d'erreurs.
Un bonus sera attribué si vous implémentez une option `-v` pertinente.

## Acceptation et remise du TP

### Acceptation

Une interface web expérimentale vous permet d'accepter le TP:

* [Interface web *travo*](https://travo.uqam.ca/?project=2685)

Autrement, vous pouvez accepter le TP manuellement en faisant les trois actions directement:

* Cloner (fork) ce dépôt sur le gitlab départemental.
* Le rendre privé : dans `Settings` → `General` → `Visibility` → `Project visibility` → `Private`.
* Ajouter l'utilisateur `@privat` comme mainteneur (oui, j'ai besoin de ce niveau de droits) : dans `Settings` → `Members` → `Invite member` → `@privat` (n'invitez pas @privat2, 3 ou 4 : ce sont mes comptes de tests).
* ⚠️ Mal effectuer ces étapes vous expose à des pénalités importantes.


### Remise

La remise s'effectue simplement en poussant votre code sur la branche `master` de votre dépôt gitlab privé.
Seule la dernière version disponible avant le **dimanche ~~13~~ 20 juin** sera considérée pour la correction.


### Intégration continue

Pour pouvez compiler avec `make` (le `Makefile` est fourni).

Vous pouvez vous familiariser avec le contenu du dépôt, en étudiant chacun des fichiers (`README.md`, `Makefile`, `check.bats`, `.gitlab-ci.yml`, etc.).

⚠️ À priori, il n'y a pas de raison de modifier un autre fichier du dépôt.
Si vous en avez besoin, ou si vous trouvez des bogues ou problèmes dans les autres fichiers, merci de me contacter.

Le système d'intégration continue vérifie votre TP à chaque `push`.
Vous pouvez vérifier localement avec `make check` (l'utilitaire `bats` entre autres est nécessaire).

Les tests fournis ne couvrent que les cas d'utilisation de base, en particulier ceux présentés ici.
Il est **fortement suggéré** d'ajouter vos propres tests dans [local.bats](local.bats) et de les pousser pour que l’intégration continue les prenne en compte.
Ils sont dans un job distincts pour avoir une meilleure vue de l'état du projet.

❤ Des points bonus pourront être attribués si `local.bats` contient des tests pertinents et généralisables.

❤ En cas de problème pour exécuter les tests sur votre machine, merci de 1. lire la documentation présente ici et 2. poser vos questions sur [/opt/tp](https://mattermost.info.uqam.ca/inf3173-e21/channels/tp).
Attention toutefois à ne pas fuiter de l’information relative à votre solution (conception, morceaux de code, etc.)

### Barème et critères de correction

Le barème utilisé est le suivant

* 50%: pour le jeu de test public fourni dans le sujet (voir section intégration).
* 50%: pour un jeu de test privé exécuté lors de la correction. Ces tests pourront être plus gros, difficiles et/ou impliquer des cas limites d'utilisation (afin de vérifier l'exactitude et la robustesse de votre code).
* Des pénalités pour des bogues spécifiques et des défauts dans le code source du programme, ce qui inclut, mais sans s'y limiter l'exactitude, la robustesse, la lisibilité, la simplicité, la conception, les commentaires, etc.
* Note: consultez la section suivante pour des exemples de pénalités et éventuellement des conseils pour les éviter.

## Mentions supplémentaires importantes

⚠️ **Intégrité académique**
Rendre public votre dépôt personnel ou votre code ici ou ailleurs ; ou faire des MR contenant votre code vers ce dépôt principal (ou vers tout autre dépôt accessible) sera considéré comme du **plagiat**.

⚠️ Attention, vérifier **≠** valider.
Ce n'est pas parce que les tests passent chez vous ou ailleurs ou que vous avez une pastille verte sur gitlab que votre TP est valide et vaut 100%.
Par contre, si des tests échouent quelque part, c'est généralement un bon indicateur de problèmes dans votre code.

⚠️ Si votre programme **ne compile pas** ou **ne passe aucun test public**, une note de **0 sera automatiquement attribuée**, et cela indépendamment de la qualité de code source ou de la quantité de travail mise estimée.
Il est ultimement de votre responsabilité de tester et valider votre programme.

Pour les tests, autant publics que privés, les résultats qui font foi sont, ceux exécutés sur java.labunix.uqam.ca et/ou sur l'instance gitlab.info.uqam.ca (au choix du correcteur). Si un test réussi presque ou de temps en temps, il est considéré comme échoué (sauf rares exceptions).


Quelques exemples de bogues fréquents dans les copies TP de INF3173 qui causent une perte de points, en plus d'être responsable de tests échoués:

* Utilisation de variables ou de mémoire non initialisés (comportement indéterminé).
* Mauvaise vérification des cas d'erreur des fonctions et appels système (souvent comportement indéterminé si le programme continue comme si de rien n'était)
* Utilisation de valeurs numériques arbitraires (*magic number*) qui cause des comportements erronés si ces valeurs sont dépassées (souvent dans les tailles de tableau).
* Mauvaise validation des entrées, de leurs formats, et de leurs valeurs (arguments, entrée standard, contenu des fichiers, etc.).
* Code inutilement compliqué, donc fragile dans des cas plus ou moins limites.


Quelques exemples de pénalités additionnelles:

* Vous faites une MR sur le dépôt public avec votre code privé : à partir de -10%
* Vous demandez à être membre du dépôt public : -5%
* Si vous critiquez à tort l'infrastructure de test : -10%
* Vous modifiez un fichier autre que le fichier source ou `local.bats` (ou en créez un) sans avoir l’autorisation : à partir de -10%
* Votre dépôt n'est pas un fork de celui-ci : -100%
* Votre dépôt n'est pas privé : -100%
* L'utilisateur `@privat` n'est pas mainteneur : -100%
* Votre dépôt n'est pas hébergé sur le gitlab départemental : -100%
* Vous faites une remise par courriel : -100%
* Vous utilisez « mais chez-moi ça marche » (ou une variante) comme argument : -100%
* Si je trouve des morceaux de votre code sur le net (même si vous en êtes l'auteur) : -100%
