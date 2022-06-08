/* ********************* *
 * TP1 INF3173 E2021
 * Code permanent: KAMY15029708
 * Nom: Kamel
 * Prénom: Younes
 * ********************* */

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

//Sert a imprimer le temps de l'ordinateur pour l'option -v, -v doit etre passe dans argv[1]
int imprimer_timestamp() {
    time_t temps;
    struct tm* heure_actuelle;
  
    temps = time(NULL);  
    heure_actuelle = localtime(&temps);
  
    printf("%02d:%02d:%02d\n", heure_actuelle->tm_hour, heure_actuelle->tm_min, heure_actuelle->tm_sec);  
    return 0;
}

//Cette fonction sert a afficher les fichier pointes par un lien symbolique
char * afficher_readlink(int pid, int option_v) {
   char chemin[50];
   sprintf(chemin, "/proc/%d/exe", pid);
   char tampon[1024];
   ssize_t taille = readlink(chemin, tampon, sizeof(tampon));
   tampon[taille] = '\0';
   // si l'option -v est activee, on imprime le timestamp en plus du chemin demande
   if (option_v) {
     imprimer_timestamp();
   }
   fprintf(stderr,"%s\n", tampon);
}

//Cette fonction traite le processus fils du fork() initial, on y lance les commandes passees en argument
int traiter_enfant(char ** argv) {
  ptrace(PTRACE_TRACEME, 0, NULL, NULL);
  raise(SIGSTOP);

  int exec;  
  if (!strcmp(argv[1], "-v")) {
    exec = execvp(argv[2], argv + 2); 
  } else {
    exec = execvp(argv[1], argv + 1);  
  } 
  if (exec == -1) {
    perror("la commande ne peut pas etre executee");
    return 127;
  } else {
    return 0;
  }   
}

//Cette fonction sert a traiter le processus parent, qui utilise ptrace sur ses enfants
int traiter_parent(int enfant, int option_v) {  
  int status = 0;      
  wait(&status); 
  ptrace(PTRACE_SETOPTIONS, enfant, NULL, PTRACE_O_TRACEEXEC | PTRACE_O_EXITKILL | PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK);     
  ptrace(PTRACE_CONT, enfant, 0, NULL);

  while (1) {     
      int pid_wait = wait(&status);      

      if (status>>8 == (SIGTRAP | (PTRACE_EVENT_EXEC<<8))) {
          afficher_readlink(pid_wait, option_v);
      }  

      if(WIFSTOPPED(status)) {       
        ptrace(PTRACE_CONT, pid_wait, 0, WSTOPSIG(status));
      }     
      
      if (WIFSIGNALED(status)) {
        if (pid_wait == enfant) {
          return 128 + WTERMSIG(status);        
        }   
      }

      if(WIFEXITED(status)) { 
        if (pid_wait == enfant) {
          return WEXITSTATUS(status);
        }       
      } 
  } 
}

//Fonction principale
int main(int argc, char *argv[]) {
  pid_t enfant;
  int option_v = 0;
  if (!strcmp(argv[1], "-v")) {
    option_v = 1;
  }
  enfant = fork();

  if (enfant < 0) {
		perror("fork");    	
	} 
  if(enfant == 0) {
    int return_value = traiter_enfant(argv);
    if(return_value == 127) {
      return return_value;
    }    
  } else {  
      int return_value = traiter_parent(enfant, option_v); 
      return return_value;      
  } 
}




