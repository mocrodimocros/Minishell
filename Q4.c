#include  <stdio.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <sys/wait.h>
#include  <string.h>
#include  "readcmd.h"
#include  "readcmd.c"


int main() {
    int retour, status;
    while(1) {
	    //Création d'une variable rep qui permettra de stocker le répertoire courant
	    char rep[256];
            getcwd(rep, sizeof(rep));
    	    printf("\nchoukrani@sec:%s ",rep);
	    //Construction de la commande
	    struct cmdline *c;
	    c = readcmd();
	    //Traitement dans le cas où la commande est "cd" ou "exit"
    	    if (strcmp(c->seq[0][0], "cd") == 0 || strcmp(c->seq[0][0], "exit") ==0) {
		//Traitement dans le cas où la commande est "cd"
	    	if (strcmp(c->seq[0][0], "cd") == 0) {
        		chdir(c->seq[0][1]);
    		}
		//Traitement dans le cas où la commande est "exit"
    		else if (strcmp(c->seq[0][0], "exit") ==0) {
                	exit(EXIT_SUCCESS);
    		}
    	    }
	    //Traitement réservé aux autres commandes
    	    else{
	    	retour = fork();
	    	if (retour < 0) {
			//L'ouverture d'un processus fils echoue
			printf("Erreur fork\n");
			exit(1);
	    	}
	    	else if (retour == 0) {
			//L'ouverture d'un processus fils succède
			//On exécute la commande stockée dans c
			execvp(c->seq[0][0],c->seq[0]);
			//On se retrouve ici en cas d'erreur
			printf("erreur d'exécution\n");
			exit(2);
	    	}
	    	else {
			//Le processus père attend un signal de fin de son fils
			wait(&status);
			if WIFEXITED(status) {
				printf(" ");
			}
			else if WIFSIGNALED(status) {
				printf("processus signalé\n");
			}
	   	 }
    	    }
    }
return EXIT_SUCCESS;
}

