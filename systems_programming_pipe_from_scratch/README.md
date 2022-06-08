# TP2 - teepee pour loguer les tubes

L'objectif du TP2 est de développer l'utilitaire `teepee` capable d'exécuter des conduites (*pipelines*) de commandes (comme le shell) mais aussi de sauvegarder les données qui passent sur chacun des tubes.
Ce TP vous permet d'approfondir la communication interprocessus avec des tubes (et l'appel système `pipe`).


## Description de l'outil `teepee`

### Usage

```
teepee [-o filename] [-s separator] <cmd> [<arg>]... [% <cmd> [<arg>]...]...
```

`teepee` exécute en parallèle une séquence de commandes (chaque commande est séparée par un caractère pourcent, `%`, par défaut) qui sont reliées entre elles par un tube de communication, comme dans une conduite (*pipeline*) shell.
La sortie standard d'une commande est donc redirigée vers l'entrée standard de la commande suivante.
L'entrée standard de la première commande correspond à l'entrée standard de `teepee` et la sortie standard de la dernière commande correspond à la sortie standard de `teepee` (habituellement le terminal dans les deux cas).

La sortie standard pour les messages d'erreur n'est pas modifiée.

Exemple :

```
$ teepee echo "bonjour le monde" % wc -w
3
```

Qui est +/- équivalent avec

```
$ echo "bonjour le monde" | wc -w
3
```

### Option `-o`

L'option `-o filename` force la sauvegarde des sorties redirigées.
Pour chaque sortie redirigée, un fichier dont le nom est de la forme `filename.x` est créé et contient le contenu de la sortie redirigée.
`filename` est l'argument passé à l'option `-o`, `.` est le caractère littéral point et `x` est le numéro de la sortie redirigée (1 pour la sortie de la première commande, 2 pour la sortie de la seconde commande, etc.).

Note: la dernière commande n'est pas redirigée donc s'il y a 4 commandes seulement 3 fichiers seront créés.

Exemple :

```
$ teepee -o out echo "bonjour le monde" % wc -w
3
$ cat out.1
bonjour le monde
```

### Option `-s`

L'option `-s separator` force `teepee` à utiliser `separator` comme séparateur à la place du défaut qui est le symbole de pourcentage.
Toute chaîne de caractère est utilisable.
Un argument de `teepee` est considéré comme un séparateur si l'argument correspond exactement et en entier à `separator`.

Exemple :

```
$ teepee -s . echo bonjour le monde . wc -w
3
```

### Valeur de retour


En cas de succès, l'outil retourne le code de retour de la dernière commande.
Si la dernière commande n'existe pas, le code de retour est 127.
Si la dernière commande se termine à cause d'un signal, le code de retour est 128+numéro du signal.

Dans tous les cas, le programme `teepee` ne se termine que lorsque toutes les commandes se sont terminées.
À part pour la dernière commande, les valeur de retour des autres commandes ne sont pas prise en compte.


En cas d'échec lors de mise en place du pipeline ou de tout problème interne de `teepee`, un message d'erreur est affiché sur la sortie standard d'erreur et 1 est retourné.


## Réalisation

Le programme devra être réalisé en C en utilisant principalement les appels système UNIX vus en classe.
Le code source final (correctement commenté et nettoyé) devra tenir dans un seul fichier C.

Comme le TP n'est pas si gros (de l'ordre de grandeur d'une ou deux centaines de lignes), il est attendu un effort important sur le soin du code et la gestion des cas d'erreurs.


Il est suggéré de développer l'outil en suivant les étapes décrites ci-dessous :

* 1. Exécution d'une commande simple avec ses arguments (utilisez entre autres les appels système `fork` et `exec`).

```
$ teepee echo bonjour le monde
bonjour le monde
```

* 2. Exécution de deux commandes séparées par un seul tube (utilisez en plus entre autres les appels système `pipe`, `dup2` et `close`).

```
$ teepee echo bonjour le monde % wc -c
17
```

* 3. Implémentation de l'option `-s`.

```
$ teepee -s world echo hello world wc -c
6
```

* 4. Exécution d'un nombre quelconque de commandes séparées par des pourcents.
Attention : fermez correctement les bouts de tubes qui doivent l'être.

```
$ teepee cat dir/fable.txt % grep -w le % wc -l
2
```

* 5. Implémentation de l'option `-o`.

```
$ teepee -o xyz cat dir/fable.txt % grep -w le % wc -l
2
$ cat xyz.2
Vous êtes le Phénix des hôtes de ces bois. "
A ces mots le Corbeau ne se sent pas de joie ;
```

Attention : pour chaque tube, quelqu'un doit se charger d'écrire dans le fichier. Ce n'est certainement pas la commande précédant le tube, ni la commande suivante. C'est donc votre programme qui doit s'en charger.

Pour cette partie, vous devez décider quelles stratégies utiliser.
*Expliquez* en commentaire au début de votre fichier C la solution que vous avez choisie (vous pouvez illustrer votre proposition avec un petit graphisme ASCII).


### Quelques remarques

* Votre proposition pour `-o` n'a pas besoin d'être inutilement compliqué (en termes de code C ou de comportement). Plusieurs solutions sont possibles.

* Il est important que les commandes s'exécutent en parallèle et puissent lire ou produire des données aussi rapidement que possible. Un processus ne devrait pas se bloquer ou être ralenti à cause de `teepee` sauf, naturellement, si elle doit attendre une donnée de la commande précédente (tube vide) ou si elle doit attendre que la commande suivante consomme des données (tube plein).

* Ceci est également vrai si l'option `-o` est présente : les écritures dans les fichiers intermédiaires devraient influer le moins possible sur l'exécution des commandes.
Un bonus sera attribué si vous implémentez une option `-r` pertinente.

## Acceptation et remise du TP

### Acceptation

Une interface web expérimentale vous permet d'accepter le TP:

* [Interface web *travo*](https://travo.uqam.ca/?project=2916)

Autrement, vous pouvez accepter le TP manuellement en faisant les trois actions directement:

* Cloner (fork) ce dépôt sur le gitlab départemental.
* Le rendre privé : dans `Settings` → `General` → `Visibility` → `Project visibility` → `Private`.
* Ajouter l'utilisateur `@privat` comme mainteneur (oui, j'ai besoin de ce niveau de droits) : dans `Settings` → `Members` → `Invite member` → `@privat` (n'invitez pas @privat2, 3 ou 4 : ce sont mes comptes de tests).
* ⚠️ Mal effectuer ces étapes vous expose à des pénalités importantes.


### Remise

La remise s'effectue simplement en poussant votre code sur la branche `master` de votre dépôt gitlab privé.
Seule la dernière version disponible avant le **dimanche 8 août** sera considérée pour la correction.


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

* au moins 50%: pour le jeu de test public fourni dans le sujet (voir section intégration).
* jusqu'à 50%: pour un jeu de test privé exécuté lors de la correction. Ces tests pourront être plus gros, difficiles et/ou impliquer des cas limites d'utilisation (afin de vérifier l'exactitude et la robustesse de votre code).
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
