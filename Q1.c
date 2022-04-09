#include  <stdio.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <sys/wait.h>
#include  <string.h>
#include  "readcmd.h"
#include  "readcmd.c"


int main() {
    int retour;
    while(1) {
	    //Création d'une variable rep qui permettra de stocker le répertoire courant
	    char rep[256];
            getcwd(rep, sizeof(rep));
	    printf("\nchoukrani@sec:%s ",rep);
	    //Construction de la commande
	    struct cmdline *c;
	    c = readcmd();	
	    retour = fork();
	    //Distinction des cas à partir du fork
	    if (retour < 0) {
		    //L'ouverture d'un processus fils echoue
		    printf("Erreur fork\n");
		    exit(1);
	    }
	    else if (retour == 0) {
		    //L'ouverture succède et on est sur le processus fils
		    printf("\n");
		    //On exécute la commande stockée dans c
		    execvp(c->seq[0][0],c->seq[0]);
		    //On se retrouve ici en cas d'erreur
		    printf("erreur d'exécution\n");
		    exit(2);
	    }
	    //Pas besoin de traiter le processus pere pour l'instant
    }
    return EXIT_SUCCESS;
}

