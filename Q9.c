#include  <stdio.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <sys/wait.h>
#include  <string.h>
#include  "readcmd.h"
#include  "readcmd.c"
#include "jobsBuild.h"
#include "jobsBuild.c"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

jobsBuild ** listeProcessus;                     
pid_t processusActif = 0;


void rediriger_sortie(char* fichier) {
    int desc_fich_out, dup_desc;
    desc_fich_out = open (fichier, O_WRONLY| O_CREAT | O_TRUNC, 0640);

    if (desc_fich_out < 0) {
        perror(fichier);
        exit(1);
    }
    dup_desc = dup2(desc_fich_out, 1);

    if (dup_desc == -1) {
        printf("Erreur dup2 \n");
        exit(1);
    }
}

void rediriger_entree(char* fichier) {
    int desc_fich_in, dup_desc;
    desc_fich_in = open (fichier, O_RDONLY);
    if (desc_fich_in < 0) {
        perror(fichier);
        exit(1);
    }
    dup_desc = dup2(desc_fich_in, 0);
    if (dup_desc == -1) {
        printf("Erreur dup2 \n");
        exit(1);
    }
}                    

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


int main() {
    
    int n = 0;
    listeProcessus = malloc(200);

    while(1) {
    int pipe1[2];
	int retour;
    	processusActif = 0; 
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
                        		modifierEtatCmd(listeProcessus,pid,Actif);
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
    	else if (c->seq[1] == NULL ) {
		retour = fork();
		if (retour < 0) {
			printf("Erreur fork\n");
			exit(1);
		}
		else if (retour == 0) {
            if (c->in != NULL) {
                        /*redirection de l'entrée standard*/
                        rediriger_entree(c->in);
                    }
            if (c->out != NULL) {
    
                  /*redirection de la sortie standard*/
                  rediriger_sortie(c->out);
            }
            
            
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
        else if (c->seq[1] != NULL && c->seq[2] == NULL){
        // Q9. Tubes simples
                    
                        // creation du pipe
                        if (pipe(pipe1) == -1){
                            perror("Error pipe\n");     /* échec du pipe */
                            exit(1);
                        }
                        int retour1 = fork();
                        /* Bonne pratique : tester systématiquement le retour des appels système */
                        if (retour1 < 0) {   /* échec du fork */
                            printf("Erreur fork\n") ;
                            exit(1);
                        }
                        /* fils */
                        else if (retour1 == 0) {
                            close(pipe1[0]);
                            if (dup2(pipe1[1],STDOUT_FILENO) == -1) {   /* échec du dup2 */
                                printf("Erreur dup2\n") ;
                                exit(1) ;
                            }
                            if (execvp(c->seq[0][0], c->seq[0]) < 0) {
                                printf("Error\n");
                                exit(1);
                            }
                            /* pere */
                        } else {
                            close(pipe1[1]);
                            if (dup2(pipe1[0],STDIN_FILENO) == -1) {   /* échec du dup2 */
                                printf("Erreur dup2\n") ;
                                exit(1) ;
                            }
                            
                            if (execvp(c->seq[1][0], c->seq[1]) < 0) {
                                printf("Error\n");
                                exit(1);
                            }
                        }
            
        }
    }
    return EXIT_SUCCESS;
}

