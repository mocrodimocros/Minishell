#include  <stdio.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <sys/wait.h>
#include  <string.h>
#include  "readcmd.h"
#include  "readcmd.c"
#include "signalsHandlers.h"
#include "signalsHandlers.c"
#include "cmdTreatment.h"
#include "cmdTreatment.c"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

jobsBuild ** listeProcessus;                     
pid_t processusActif = 0;
int n;

int main() {
    //Allouer la mémoire nécessaire pour le tableau des processus
    listeProcessus = malloc(200);
    while(1) {
	ctrl_z = 0;
    	ctrl_c = 0;
    	processusActif = 0;
        //Associer les handlers à leur signal
	signal(SIGCHLD,handler_sigchld);
        signal(SIGTSTP,handler_SIGTSTP);
        signal(SIGINT,handler_SIGINT);
	//Création d'une variable rep qui permettra de stocker le répertoire courant
	char rep[256];
        getcwd(rep, sizeof(rep));
    	printf("\nchoukrani@sec:%s ",rep);
	//Construction de la commande
	struct cmdline *c;
	c = readcmd();
    //Traitement de la commande
	traiter_commande(c);
	}
}

