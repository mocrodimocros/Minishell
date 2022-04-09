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
int ctrl_z,ctrl_c;

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

void handler_sigchld() {
    int etat_fils;
    pid_t pid_fils;
    do {
        pid_fils = (int) waitpid(-1, &etat_fils, WNOHANG | WUNTRACED | WCONTINUED);
        if ((pid_fils == -1) && (errno != ECHILD)) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        } else if (pid_fils > 0) {
            if (WIFSTOPPED(etat_fils)) {
                /* Traiter la suspension. */
                /* Changer l'état du processus dans la liste des 
                ** processus à l'état suspendu. */
                if (ctrl_z == 0 ){
                    modifierEtatCmd(listeProcessus,processusActif,Suspendu);
                }
            } else if (WIFCONTINUED(etat_fils)) {
                modifierEtatCmd(listeProcessus,processusActif,Actif);
            } else if (WIFEXITED(etat_fils)) {
                /* Supprimer le processus de la liste des processus 
                non encore temrinées. */
                SupprimerCommande(listeProcessus,pid_fils);
                processusActif= 0;
            }
        }
    } while (pid_fils > 0);
}

void handler_SIGTSTP() {
    if (processusActif != 0) {
        kill(processusActif,SIGSTOP);
        printf("\nProcessus suspendu");
        modifierEtatCmd(listeProcessus,processusActif,Suspendu);
        ctrl_z = 1;
    }
    fflush(stdin);
}

void handler_SIGINT() {
    if (processusActif != 0) {
        kill(processusActif,SIGKILL);
        printf("\nProcessus tué");
        SupprimerCommande(listeProcessus, processusActif);
        ctrl_c = 1;
    }
    fflush(stdin);
}



int main() {
    
    int n = 0;
    listeProcessus = malloc(200);

    while(1) {
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
		if (!(c->seq[0][1] == NULL)){
        		pid_t pid_proc = pidCommande(listeProcessus,atoi(c->seq[0][1]));
			if (Est_present(listeProcessus,pid_proc)) {
				kill(pid_proc,18);
            			printf("Reprise en arrière-plan");
			} else {
            			printf("Processus non trouvé ( Id = %d )\n",atoi(c->seq[0][1]));	
        		}
    		} else {
        			printf("Erreur : L'identifiant du processus est non saisi\n");
    		}
       }
       else if (strcmp(c->seq[0][0], "fg") == 0){
       		if (!(c->seq[0][1] == NULL)){
        		pid_t pid_proc = pidCommande(listeProcessus,atoi(c->seq[0][1]));
        		if (Est_present(listeProcessus,pid_proc)) { 
            			kill(pid_proc,18);
            			printf("Reprise en Avant-plan");
            			while((ctrl_z == 0) && (ctrl_c == 0)){                   
                			sleep(1);
            			}
        		} else {
            			printf("Processus non trouvé ( Id = %d )\n",atoi(c->seq[0][1]));
            			
        		}
    		} else {
        			printf("Erreur : L'identifiant du processus est non saisi\n");
    		}
       }
       else if (c->seq[0] != NULL && c->seq[1] == NULL){
		pid_t retour = fork();
		if (retour < 0) {
			printf("Erreur fork\n");
			exit(1);
		}
		else if (retour == 0) {
			if (!(c->backgrounded == NULL)){
            		/*Masquer les signaux SIGINT et SIGTSTP.
            		** pour les commandes en arrière plan. */
            		sigset_t signaux;
            		sigemptyset(&signaux);
            		sigaddset(&signaux,SIGINT);
            		sigaddset(&signaux,SIGTSTP);
            		sigprocmask(SIG_BLOCK,&signaux,NULL);
        		}
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
    }
}

