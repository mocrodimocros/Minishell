#include "cmdTreatment.h"
#include "redirections.c"
#include "jobsBuild.c"

void traiter_commande(struct cmdline *c) {
    if (!(c->seq[0] == NULL)){//ce premier if sert à continuer à proposer 
                                //à l'utilisateur de taper une commande même si il tape entrée.
    if (c->seq[1] == NULL) {
        //Le cas où la commande est cd
		if (strcmp(c->seq[0][0], "cd") == 0) {
        		chdir(c->seq[0][1]);
    		}
        //Le cas où la commande est exit, le miniShell se ferme
		else if (strcmp(c->seq[0][0], "exit") ==0) {
        		exit(EXIT_SUCCESS);
    		}
        //Le cas où la commande est jobs, la liste des processus s'affiche
		else if (strcmp(c->seq[0][0], "jobs") == 0){
               		 afficherListe(listeProcessus);               
    		}
        //Le cas où la commande est stop, un signal SIGSTP est envoyé au processus dont l'id est mis en argument
		else if (strcmp(c->seq[0][0], "stop") == 0){
                	if (c->seq[0][1] == NULL){
                    		printf("Saisissez un id  !\n");
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
            //Le cas où la commande est bg, le processus interrompu est relancé en fond, l'utilisateur peut donc toujours entrer des  commandes.
       		else if (strcmp(c->seq[0][0], "bg") == 0){
			if (!(c->seq[0][1] == NULL)){
        			pid_t pid_proc = pidCommande(listeProcessus,atoi(c->seq[0][1]));
				if (Est_present(listeProcessus,pid_proc)) {
					kill(pid_proc,18);
            				printf("Repris en arrière-plan");
				} else {
            				printf("Processus non trouvé ( Id = %d )\n",atoi(c->seq[0][1]));	
        			}
    			} else {
        			printf("Erreur : L'identifiant du processus est non saisi\n");
    			}
       		}
            //Le cas où la commande est fg, le processus interrompu est relancé en avant plan, l'utilisateur peut toujours entrer des commandes, mais elles ne sont exécuté que à la fin du processus courant.
      		else if (strcmp(c->seq[0][0], "fg") == 0){
       			if (!(c->seq[0][1] == NULL)){
        			pid_t pid_proc = pidCommande(listeProcessus,atoi(c->seq[0][1]));
        			if (Est_present(listeProcessus,pid_proc)) { 
                            printf("Repris en Avant-plan");
            				kill(pid_proc,18);
                            while((ctrl_z == 0) && (ctrl_c == 0)){                   
                                sleep(0);
                            }
                            processusActif = 0;
        			} else {
            				printf("Processus non trouvé ( Id = %d )\n",atoi(c->seq[0][1]));
            			}
    			} else {
        			printf("Erreur : L'identifiant du processus est non saisi\n");
    			}
       		}
       		else {
			pid_t retour = fork();
			if (retour < 0) {
				printf("Erreur fork\n");
				exit(1);
			}
			else if (retour == 0) {
				if (!(c->backgrounded == NULL)){
            				//Masquer les signaux SIGINT et SIGTSTP.
            				//pour les commandes en arrière plan.
            				sigset_t signaux;
            				sigemptyset(&signaux);
            				sigaddset(&signaux,SIGINT);
            				sigaddset(&signaux,SIGTSTP);
            				sigprocmask(SIG_BLOCK,&signaux,NULL);
        			}
            			if (c->in != NULL) {
                        	//redirection de l'entrée standard
                        	rediriger_entree(c->in);
                    		}
            			if (c->out != NULL) {    
                  			//redirection de la sortie standard
                  			rediriger_sortie(c->out);
            			}
                //Exécuter la commande
				execvp(c->seq[0][0],c->seq[0]);
				printf("erreur d'exécution\n");
				exit(2);
            
			}
			else {
                    		jobsBuild *process = malloc(512);
                            //Ajout du processus enregistré dans la liste des processus
                    		process->pid = retour;
                    		process->Identifiant = ++n;
                    		process->etat = Actif;
                    		strcat(process->commande,c->seq[0][0]);
                    		AjouterCommande(listeProcessus,process);
                            // Attendre le processus fils si la commande 
            			    // n'est pas en tâche de fond.
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
        else {
		    pid_t pidFils;
    		int p[2];
    		int entree = 0;
    		int index = 0;

    		while (!(c->seq[index] == NULL)) {
        		pipe(p);
        		pidFils = fork();

        		if (pidFils == -1) {
            			printf("Erreur fork\n");
            			exit(1);
            		
       			}
        		if (pidFils == 0) {  
            			if (!(c->backgrounded == NULL)){
               			 //Toujours masquer les signaux SIGINT et SIGTSTP.
               			 //pour les commandes en arrière plan ...
                		sigset_t signaux;
                		sigemptyset(&signaux);
                		sigaddset(&signaux,SIGINT);
                		sigaddset(&signaux,SIGTSTP);
               			sigprocmask(SIG_BLOCK,&signaux,NULL);
            			}
            			// Rediriger l'entree vers celle mémorisée dans la variable entree
            			dup2(entree,0);

            			// Rediriger la sortie si elle existe une commande suivante
            			if (!(c->seq[index + 1] == NULL)){
                		dup2(p[1],1);
                		close(p[0]);
                		close(p[1]);
            			}

            			// Rediriger l'entrée 
            			if (!(c->in == NULL) && (index == 0)){
                			rediriger_entree(c->in);
            			}
            			// Rediriger la sortie 
            			if (!(c->out == NULL) && (c->seq[index + 1] == 0)){
                			rediriger_sortie(c->in);
            			}

            
            			//Exécuter la commande
				        if (execvp(c->seq[index][0],c->seq[index]) < 0){
                        printf("Erreur : Commande invalide\n");
                        exit(EXIT_FAILURE);
                        }

        		} else {  /* père */
            			    jobsBuild *process = malloc(512);
				            process->pid = pidFils;
                    		process->Identifiant = ++n;
                    		process->etat = Actif;
                    		strcat(process->commande,c->seq[0][0]);
                    		AjouterCommande(listeProcessus,process);
            	
            
            			// Attendre le processus fils si la commande 
            			// n'est pas en tâche de fond. 
            			if (c->backgrounded == NULL) {  
               				int status;
                			processusActif = pidFils;
                			waitpid(pidFils,&status,WUNTRACED);
                			if (!WIFSTOPPED(status)){
                    				SupprimerCommande(listeProcessus,pidFils);
                			}
                			processusActif = 0;    
            			}
            			close(p[1]);
            			//faire le pont
            			entree = p[0];
				        index ++;
			}

    		}
	}
    }
}
