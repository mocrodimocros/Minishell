#include  <stdio.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <sys/wait.h>
#include  <string.h>
#include  "readcmd.h"
#include  "readcmd.c"
#include "jobsBuild.h"
#include "jobsBuild.c"

jobsBuild ** listeProcessus;                     
pid_t processusActif = 0;
int ctrl_z,ctrl_c;                        

void handler_SIGTSTP(){
    if (processusActif != 0) {
        kill(processusActif,SIGSTOP);
        printf("\n");
        printf("[%d]+ Arrêté\n", processusActif);
        modifierEtatCmd(listeProcessus,processusActif,Suspendu);
    }
    fflush(stdin);
}

void handler_SIGINT() {
    if (processusActif != 0) {
        kill(processusActif,SIGKILL);
        printf("\n");
        printf("[%d]+ Fermé\n", processusActif);
        SupprimerCommande(listeProcessus, processusActif);
    }
    fflush(stdin);
}

void handler_sigchld(int signal){
	int wstatus;
        pid_t pid_child;
        do {
        	pid_child = (int) waitpid(-1, &wstatus, WNOHANG | WUNTRACED | WCONTINUED);
                if ((pid_child == -1) && (errno != ECHILD)) {
                	perror("waitpid");
                        exit(-1);
                } else if (pid_child > 0) {
                        if (WIFSTOPPED(wstatus)) {    
                              if (ctrl_z ==0 ){
				    modifierEtatCmd(listeProcessus,pid_child,Suspendu);
                              }
                        } else if (WIFCONTINUED(wstatus)) {  
                               /*Reprise de processus.*/   
				modifierEtatCmd(listeProcessus,pid_child,Actif);
                        } else if (WIFEXITED(wstatus) || WIFSIGNALED(wstatus)) {
                                /*le processus à été arreté.*/ 
                                SupprimerCommande(listeProcessus,pid_child);
                                processusActif= 0;
                        }
                 }
        } while (pid_child > 0);
}


int main() {
    
    int n = 0;
    listeProcessus = malloc(200);

    while(1) {
	int retour;
    	processusActif = 0;
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
    	if (strcmp(c->seq[0][0], "cd") == 0 || strcmp(c->seq[0][0], "exit") ==0 || strcmp(c->seq[0][0], "stop") == 0 || strcmp(c->seq[0][0], "bg") == 0 || strcmp(c->seq[0][0], "fg") == 0 || strcmp(c->seq[0][0], "jobs") == 0) {
		if (strcmp(c->seq[0][0], "cd") == 0) {
        		chdir(c->seq[0][1]);
    		}
    		else if (strcmp(c->seq[0][0], "exit") ==0) {
        		exit(EXIT_SUCCESS);
    		}
    		else if (strcmp(c->seq[0][0], "jobs") == 0){
               		 afficherListe(listeProcessus);               
    		}
    		else if (strcmp(c->seq[0][0], "stop") == 0){
                	if (c->seq[0][1] == NULL){
                    		printf("Saisissez un pid  !\n");
                	} 
			else {
                    		pid_t pid = pidCommande(listeProcessus,atoi(c->seq[0][1]));                    
                    		if ((Est_present(listeProcessus,pid))) {
                        		if (etatcmd(listeProcessus,pid) == Actif) {
                            			printf("[%d]+ Tué\n", pid);
                            			kill(pid,SIGSTOP);
                            			modifierEtatCmd(listeProcessus,pid,Suspendu);
                        		} 
					else {
                            			printf("Le processus n'est pas actif !\n");
                        		}
                    		}
				else {
                        		printf("Processus introuvable !\n");
                    		}		
                	}
                	processusActif = 0;                
            	} 
		else if (strcmp(c->seq[0][0], "bg") == 0){                
                	if (c->seq[0][1] == NULL){
                    		printf("Mauvais identifiant de processus !\n");
                	} 
			else {
                    		pid_t pid = pidCommande(listeProcessus,atoi(c->seq[0][1]));
                    		if(Est_present(listeProcessus,pid)){
                        		printf("Reprise du processus en arrière-plan \n");
                        		kill(pid,SIGCONT);
                    		} 
				else {
                        		printf("Processus introuvable !\n");
                    		}
                	}                
            	}
	 	else if (strcmp(c->seq[0][0], "fg") == 0){
                	if (c->seq[0][1] == NULL){
                    		printf("Mauvais identifiant de processus !\n");
                	} 
			else {
                    		pid_t pid = pidCommande(listeProcessus,atoi(c->seq[0][1]));
                   	 	if(Est_present(listeProcessus,pid)){
                        		kill(pid,SIGCONT);
                        		printf("Reprise du processus en avant-plan \n");                        
                        		processusActif = pid;
					modifierEtatCmd(listeProcessus,pid,Actif);
                        		int status;
                        		waitpid(pid,&status,WUNTRACED);
                        		SupprimerCommande(listeProcessus, pid);
                        		continue;
                    		}
			 	else {
                        		printf("Processus introuvable !\n");
                        		continue;
                    		}
                    		processusActif = pid;
                	}
                	processusActif = 0;
            	}
    	}
    	else{
		retour = fork();
		if (retour < 0) {
			printf("Erreur fork\n");
			exit(1);
		}
		else if (retour == 0) {
			execvp(c->seq[0][0],c->seq[0]);
			printf("erreur d'exécution\n");
			exit(2);
		}
		else {
                    	jobsBuild *process = malloc(512);
                    	process->pid = retour;
                    	process->Identifiant = ++n;
                    	process->etat = Actif;
                    	strcat(process->commande,c->seq[0][0]);
                    	AjouterCommande(listeProcessus,process);
                        if (c->backgrounded == NULL){
                        	processusActif = retour;
                        	int status;
                        	waitpid(retour,&status,WUNTRACED);
                        	if (!WIFSTOPPED(status)) {                           
                               		SupprimerCommande(listeProcessus, retour);                            
                        	}
                        	processusActif = 0;
                    	}
                }
    	}
    }
    return EXIT_SUCCESS;
}

