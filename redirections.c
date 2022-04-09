#include "redirections.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
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
