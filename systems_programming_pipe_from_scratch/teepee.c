/*******************
 * teepee TP2 INF3172
 * Nom: Kamel
 * Prénom: Younes
 * Code permanent: KAMY15029708
 *********************/

/************************************************************************
 * Stratégie pour l'option -o:
 *
 * Ma strategie pour implementer l'option -o fut d'utiliser un second pipe. Lorsque la sortie d'une commande
 * est redirigee sur le bout ecriture du premier pipe, je lis ensuite le premier pipe pour en ecrire le contenu
 * dans un deuxieme pipe et dans le fichier de sorite simultanement.
 *
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

/*Definition d'une structure de type ensemble me permettant de stocker les indices des debuts de commandes dans argv et de gerer l'allocation dynamique.
Les fonctions suivantes me permettent de manipuler cette structure*/
typedef struct {
    int size;
    int capacity;
    int *data;
} struct_indices;

void set_struct(struct_indices *struct_indices, int index, int value) {
    if (index < struct_indices->size && index >= 0) {
        struct_indices->data[index] = value;
    }
}

void extend_struct(struct_indices *struct_indices) {
    if (struct_indices->size >= struct_indices->capacity) {
        struct_indices->capacity *= 2;
        struct_indices->data = realloc(struct_indices->data, sizeof(int) * struct_indices->capacity);
    }
}

void ajouterElement(struct_indices *struct_indices, int value) {
    extend_struct(struct_indices);
    struct_indices->data[struct_indices->size++] = value;
}

struct_indices creer_struct_indices(struct_indices *struct_indices, int capacity) {
    struct_indices->size = 0;
    struct_indices->capacity = capacity;
    struct_indices->data = malloc(sizeof(int) * struct_indices->capacity);
    for (int i=struct_indices->size; i < struct_indices->capacity; ++i) {
        set_struct(struct_indices, i, 0);
    }
}

void free_struct(struct_indices *struct_indices) {
    free(struct_indices->data);
}

/* Cette fonction sert a creer le nom du fichier a ouvrir dans lire ecrire en combinant
le nom sans extension fourni avec l'option -o et l'extension ".x"*/
char * retourner_nom_fichier (char * debut, int extention) {
    char * nom_fichier = malloc(strlen(debut));
    strcpy(nom_fichier,debut);
    strcat(nom_fichier, ".");
    char * str = malloc(2);
    snprintf(str, 4, "%d", extention);
    strcat(nom_fichier, str);
    return nom_fichier;
}

/* Cette fonction est utilisee pour l'option -o, elle sert a lire le contenu du premier pipe et a l'ecrire simultanement
a la fois dans le second pipe et dans le fichier d'output de l'option -o*/
int lire_ecrire(int premier_pipe_lecture, int deuxieme_pipe_ecriture, int i, int size, char ** argv, char * nom_du_fichier) {
    char * nom_fichier = retourner_nom_fichier(nom_du_fichier, i);
    
    int descripteur_fichier = open(nom_fichier, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
    if (descripteur_fichier == -1) {
        perror("erreur open");
        return 1;
    }
    char * buffer = malloc(sizeof(char)*1);
    ssize_t size_read = 1;
    while (size_read > 0) {
        size_read = read(premier_pipe_lecture, buffer, sizeof(buffer) -1);
        if (size_read == -1) {
            perror("erreur read");
            return 1;
        }
        buffer[size_read] = '\0';
        int message = write(descripteur_fichier, buffer, strlen(buffer));
        if (message == -1) {
            perror("erreur write");
            return 1;
        }
        if (errno != EAGAIN) {
            if ((write(deuxieme_pipe_ecriture, buffer, strlen(buffer))) == -1) {
                if (errno == EAGAIN) {
                continue;
                } else {
                  perror("erreur write");
                  return 1;
               }
            }
        }
    }
    free(buffer);
    if ((close(descripteur_fichier)) == -1) {
      perror("close descripteur fichier");
      return 1;
    }
    if ((close(deuxieme_pipe_ecriture)) == -1) {
      perror("close deuxieme pipe ecriture");
      return 1;
    }
    if ((close(premier_pipe_lecture)) == -1) {
      perror("close premier pipe");
      return 1;
    }
    return 0;
}

/*Dans cette fonction le traitement des pipes est effectue, pour chaque commande passe en argument un nouveau processus est cree et sa sortie est ecrite dans le pipe
afin qu'elle puisse etre lue par la fonction suivante*/
int traiter_pipes(int tab_indices_debut[], char **argv, int size, int option_o, int option_r, char * nom_fichier_option_o, char * nom_fichier_option_r) {
    pid_t pid;
    int p[2];
    int p2[2];

    for (int i = 0; i < size; i++) {
        pipe(p);
        if ((pid = fork()) == -1) {
            perror("fork");
            return 1;
        } else if (pid == 0) { // enfant
            if (pipe(p2) == -1) {
                perror("pipe");
                return 1;
            }
            int flags = fcntl(0, F_GETFL);
            fcntl(p2[1], F_SETFL, flags | O_NONBLOCK);
            if (option_o && i != 0) {
                pid_t pid_2 = fork();
                if (pid_2 == 0) { // petit enfant
                    if ((lire_ecrire(0, p2[1], i, size, argv, nom_fichier_option_o)) == 1) {
                        return 1;
                    }
                    
                    if ((close(p2[0])) == -1) {
                        perror("close p2[0]");
                        return 1;
                    }

                    if ((close(p[0])) == -1) {
                        perror("close p[0]");
                        return 1;
                    }
                    if ((close(p[1])) == -1) {
                        perror("close p[1]");
                        return 1;
                    }
                    exit(0); // fin petit enfant

                } else { // enfant
                    dup2(p2[0], 0); // transfert du deuxieme pipe pour exec
                    if ((close(p2[0])) == -1) {
                        perror("close p2[0]");
                        return 1;
                    }
                    if ((close(p2[1])) == -1) {
                        perror("close p2[1]");
                        return 1;
                    }
                }
            } else { // (!option_o && i == 0) (enfant)
                if ((close(p2[0])) == -1) {
                    perror("close p[0]");
                    return 1;
                }
                if ((close(p2[1])) == -1) {
                    perror("close p[1]");
                    return 1;
                }
            }

            // Execution des commandes (enfant)
            if (i + 1 < size) {
                if((dup2(p[1], 1)) == -1) {
                    perror("dup2");
                    return 1;
                }
            }
            if ((close(p[1])) == -1) {
                perror("close p[1]");
                return 1;
            }
            if((close(p[0])) == -1) {
                perror("close p[0]");
                return 1;
            }

            // Si l'option -r est activee la sortie de teepee.c est redirigee vers un fichier nomme nom_fichier_option_r
            if (option_r && i + 1 == size) {
                int fd;
                if((fd = open(nom_fichier_option_r, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU)) == -1) {
                  perror("open");
                  return 1;
                } 
                
                if((dup2(fd, 1)) == -1) {
                  perror("dup2");
                  return 1;
                }

                if ((close(fd)) == -1) {
                  perror("close");
                  return 1;
                }
            }
            execvp(argv[tab_indices_debut[i]], argv + tab_indices_debut[i]);
        } else { // Parent
            if ((close(p[1])) == -1) {
              perror("close p[1]");
            }            
            if (i + 1 < size) {
                if(dup2(p[0], 0)) {
                  perror("dup2");
                  return 1;
                }
                if (close(p[0])) {
                  perror("close p[0]");
                  return 1;
                }
                
            } else {
                if ((close(p[0])) == -1) {
                  perror("close p[0]");
                  return 1;
                }
                int wstatus;
                waitpid(pid, &wstatus, 0);
                if (WIFSIGNALED(wstatus)) {
                    return 128 + WTERMSIG(wstatus);       
                }

                if(WIFEXITED(wstatus)) { 
                    return WEXITSTATUS(wstatus);                        
                }    

                if(i == 0) { 
                    return 127;                        
                }               
            }
        }
    }
}

//Cette fonction sert a gerer les options fournies en arguments dans argv telles que -o -r -s
void gerer_options (char ** argv, char ** separateur, int * option_o, int * option_r, int * i, char ** nom_fichier_option_o, char ** nom_fichier_option_r, struct_indices * struct_indices_debuts) {
//premiere option est s
   if (!strcmp(argv[1], "-s")) {
       if (strcmp(argv[3], "-r") && strcmp(argv[3], "-o")) {
        *separateur = argv[2];
        *i = 3;
        ajouterElement(struct_indices_debuts, 3);
       }
       //deuxieme argument est o
       if (!strcmp(argv[3], "-o")) {
        if (strcmp(argv[5], "-r")) {
            *separateur = argv[2];
            *option_o = 1;
            *nom_fichier_option_o = argv[4];
            *i = 5;
            ajouterElement(struct_indices_debuts, 5);
        }
        if (!strcmp(argv[5], "-r")) {
            *option_r = 1;
            *nom_fichier_option_r = argv[6];
            *separateur = argv[2];
            *option_o = 1;
            *nom_fichier_option_o = argv[4];
            *i = 7;
            ajouterElement(struct_indices_debuts, 7);
        }
       }
       //deuxieme option est r
       if (!strcmp(argv[3], "-r")) {
        if (strcmp(argv[5], "-o")) {
          *option_r = 1;
          *nom_fichier_option_r = argv[4];
          *separateur = argv[2];
          *i = 5;
          ajouterElement(struct_indices_debuts, 5);    
        }
        if (!strcmp(argv[5], "-o")) {
           *option_r = 1;
           *nom_fichier_option_r = argv[4];
           *separateur = argv[2];
            *option_o = 1;
            *nom_fichier_option_o = argv[6];
            *i = 7;
            ajouterElement(struct_indices_debuts, 7);
        }
       }
    }
   //premiere option est o
    if (!strcmp(argv[1], "-o")) {
      if (strcmp(argv[3], "-s") && strcmp(argv[3], "-r")) {
        *separateur = "%";
        *option_o = 1;
        *nom_fichier_option_o = argv[2];
        *i = 3;
        ajouterElement(struct_indices_debuts, 3);
      }
      if (!strcmp(argv[3], "-s")) {
        if (strcmp(argv[5], "-r")) {
          *separateur = argv[4];
          *option_o = 1;
          *nom_fichier_option_o = argv[2];
          *i = 5;
          ajouterElement(struct_indices_debuts, 5);
        }
        if (!strcmp(argv[5], "-r")) {
          *option_r = 1;
          *nom_fichier_option_r = argv[6];
          *separateur = argv[4];
          *option_o = 1;
          *nom_fichier_option_o = argv[2];
          *i = 7;
          ajouterElement(struct_indices_debuts, 7);
        }
       }

       if (!strcmp(argv[3], "-r")) {
        if (strcmp(argv[5], "-s")) {
          *option_r = 1;
          *nom_fichier_option_r = argv[4];
          *option_o = 1;
          *separateur = "%";
          *nom_fichier_option_o = argv[2];
          *i = 5;
          ajouterElement(struct_indices_debuts, 5);   
        }
        if (!strcmp(argv[5], "-s")) {
          *option_r = 1;
          *nom_fichier_option_r = argv[4];
          *separateur = argv[6];
          *option_o = 1;
          *nom_fichier_option_o = argv[2];
          *i = 7;
          ajouterElement(struct_indices_debuts, 7);
        }
       } 
    }

    //premiere option est r
    if (!strcmp(argv[1], "-r")) {
       if (strcmp(argv[3], "-s") && strcmp(argv[3], "-o")) {
        *option_r = 1;
        *separateur = "%";
        *nom_fichier_option_r = argv[2];
        *i = 3;
        ajouterElement(struct_indices_debuts, 3);
       }
       if (!strcmp(argv[3], "-o")) {
        if (strcmp(argv[5], "-s")) {
            *option_r = 1;
            *nom_fichier_option_r = argv[2];
            *option_o = 1;
            *separateur = "%";
            *nom_fichier_option_o = argv[4];
            *i = 5;
            ajouterElement(struct_indices_debuts, 5);
        }
        if (!strcmp(argv[5], "-s")) {
          *option_r = 1;
          *nom_fichier_option_r = argv[2];
          *separateur = argv[6];
          *option_o = 1;
          *nom_fichier_option_o = argv[4];
          *i = 7;
          ajouterElement(struct_indices_debuts, 7);  
        }
       }

       if (!strcmp(argv[3], "-s")) {
        if (strcmp(argv[5], "-o")) {
          *option_r = 1;
          *nom_fichier_option_r = argv[2];
          *separateur = argv[4];
          *i = 5;
          ajouterElement(struct_indices_debuts, 5);
        }
        if (!strcmp(argv[5], "-o")) {
          *option_r = 1;
          *nom_fichier_option_r = argv[2];
          *separateur = argv[4];
          *option_o = 1;
          *nom_fichier_option_o = argv[6];
          *i = 7;
          ajouterElement(struct_indices_debuts, 7); 
        }
       }
    }

    if ((strcmp(argv[1], "-o") && strcmp(argv[1], "-s")) && strcmp(argv[1], "-r")) {
      *separateur = "%";
      *i = 1;
      ajouterElement(struct_indices_debuts, 1);
    }    
}

//Fonction main
int main(int argc, char *argv[]) {
    struct_indices struct_indices_debuts;
    creer_struct_indices(&struct_indices_debuts, 10);

    char * separateur;
    int option_o = 0;
    int option_r = 0;
    int i;
    char * nom_fichier_option_o;
    char * nom_fichier_option_r;
    gerer_options(argv, &separateur, &option_o, &option_r, &i, &nom_fichier_option_o, &nom_fichier_option_r, &struct_indices_debuts);

    int indice_debut = 0;
    for (i; i != argc; i++) {
        if (strcmp(argv[i], separateur) == 0) {
            argv[i] = NULL;
            indice_debut = i + 1;
            ajouterElement(&struct_indices_debuts, indice_debut);
        }
    }
    int valeur_retour = traiter_pipes(struct_indices_debuts.data, argv, struct_indices_debuts.size, option_o, option_r, nom_fichier_option_o, nom_fichier_option_r);
    free_struct(&struct_indices_debuts);
    return valeur_retour;
}
