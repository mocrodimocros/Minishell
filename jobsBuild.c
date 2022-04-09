#include <stdio.h>    
#include <unistd.h>   
#include <stdlib.h>   
#include <string.h>
#include "jobsBuild.h"

//Retourne la taille du tableau (le nombre de processus)
int tailleListe(jobsBuild **listeProcessus){
    int i = 0;
    while(!(listeProcessus[i] == NULL)){
        i++;
    }
    return i;
}

//Retourne la position du processus dans le tableau
int positionprocessus(jobsBuild **listeProcessus, pid_t pidProcessus){
    int i = 0;
    int position = -1;
    while(!(listeProcessus[i] == NULL)){
        if (listeProcessus[i]->pid == pidProcessus) {
            position = i;
            break;
        }
        i++;
    }
    return position;
}

//Retourne le pid du processus dont l'identifiant est id
pid_t pidCommande(jobsBuild **listeProcessus, int id){
    int i = 0;
    int pid = -1;
    while(!(listeProcessus[i] == NULL)){
        if (listeProcessus[i]->Identifiant == id) {
            pid = listeProcessus[i]->pid;
            break;
        }
        i++;
    }
    return pid;
}

//Retourne un boolean qui indique si un processus est présent
bool Est_present(jobsBuild **listeProcessus, pid_t pid){
    return !(positionprocessus(listeProcessus,pid) == -1);
}

//Ajouter une commande à un tableau
void AjouterCommande(jobsBuild **listeProcessus, jobsBuild *processus) {
    int n;
    if (! Est_present(listeProcessus,processus->pid)) {
        n = tailleListe(listeProcessus);
        listeProcessus[n] = xmalloc(512);
        listeProcessus[n]->Identifiant = processus->Identifiant;
        listeProcessus[n]->pid = processus->pid;
        listeProcessus[n]->etat = Actif;
        strcpy(listeProcessus[n]->commande,processus->commande);
    }
}

//Supprimer une commande d'un tableau
void SupprimerCommande(jobsBuild **listeProcessus, pid_t pid){
    int n, position;
    if (Est_present(listeProcessus,pid)) {
        n = tailleListe(listeProcessus);
        position = positionprocessus(listeProcessus,pid);
        for (int i = position+1; i < n; i++){
            *(listeProcessus[i-1]) = *(listeProcessus[i]);
        }
        free(listeProcessus[n-1]);
        listeProcessus[n-1] = NULL;
        listeProcessus = calloc(n-1,sizeof(listeProcessus));
    }
}

//Modifier l'état d'une commande dans un tableau
void modifierEtatCmd(jobsBuild **listeProcessus, pid_t pid, Etat Etatdeux){
    int position;
    if (Est_present(listeProcessus,pid)) {
        position = positionprocessus(listeProcessus, pid);
        listeProcessus[position]->etat = Etatdeux;
    }
}

//Afficher le tableau
void afficherListe(jobsBuild **listeProcessus){
    int n;
    n = tailleListe(listeProcessus);
    if (n>0){
        printf("Id\t Pid\t Etat\t\tCommande\n");
        for (int i = 0; i<n; i++) {
            int id = listeProcessus[i]->Identifiant;
            int pid = listeProcessus[i]->pid;
            char *status;
            switch (listeProcessus[i]->etat){
                case Actif : status = "Actif"; break;           
                case Suspendu : status = "Suspendu"; break;
            }
            printf("[%d]\t %d\t %s\t",id,pid, status);
            printf("%s\n",listeProcessus[i]->commande);
        }
    } 
    else {
        printf("La liste est vide !\n");
    }
}

//Retourne l'état d'une commande
Etat etatcmd(jobsBuild **listeProcessus,pid_t pid) {
    Etat etat;
    if (Est_present(listeProcessus,pid)) {
        int position = positionprocessus(listeProcessus, pid);
        etat = listeProcessus[position]->etat;
    }
    return etat;
}
