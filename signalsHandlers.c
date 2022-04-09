#include  "signalsHandlers.h"

jobsBuild ** listeProcessus;                     
pid_t processusActif;
int ctrl_z,ctrl_c;

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
                // Traitement de la suspension.
                if (ctrl_z == 0 ){
                    modifierEtatCmd(listeProcessus,processusActif,Suspendu);
                    ctrl_z = 1;
                }
            } else if (WIFCONTINUED(etat_fils)) {
                // Traitement au cas où le processus reprend
                modifierEtatCmd(listeProcessus,processusActif,Actif);
            } else if (WIFEXITED(etat_fils)) {
                // Traitement au cas où le processus s'arrête
                SupprimerCommande(listeProcessus,pid_fils);
                ctrl_c = 1;
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
