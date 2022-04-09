#include <stdbool.h>
#ifndef _JOBSBUILD_H
#define _JOBSBUILD_H

// Les etats possible d'un processus
enum Etat{Actif,Suspendu};
typedef enum Etat Etat;

// liste des commandes
struct jobsBuild {
    int Identifiant;
    pid_t pid;
    Etat etat;
    char commande[200];
};
typedef struct jobsBuild jobsBuild;

//Retourne la position du processus dans le tableau
int positionProcessus(jobsBuild **listeProcessus, pid_t pidProcessus);

//Retourne le pid du processus dont l'identifiant est id
pid_t pidCommande(jobsBuild **listeProcessus, int id);

//Retourne un boolean qui indique si un processus est présent
bool Est_present(jobsBuild **listeProcessus, pid_t pid);

//Ajouter une commande à un tableau
void AjouterCommande(jobsBuild **listeProcessus, jobsBuild *processus);

//Supprimer une commande d'un tableau
void SupprimerCommande(jobsBuild **listeProcessus, pid_t pid);

//Modifier l'état d'une commande dans un tableau
void modifierEtatCmd(jobsBuild **listeProcessus, pid_t pid, Etat Etatdeux);

//Afficher le tableau
void afficherListe(jobsBuild **listeProcessus);

//Retourne l'état d'une commande
Etat etatcmd(jobsBuild **listeProcessus,pid_t pid);

#endif

