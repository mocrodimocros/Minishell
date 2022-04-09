#ifndef __CMDTREATMENT_H
#define __CMDTREATMENT_H

#include  <stdio.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <sys/wait.h>
#include  <string.h>
#include  "readcmd.h"
#include "jobsBuild.h"
#include "redirections.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

jobsBuild ** listeProcessus;                     
pid_t processusActif ;
int n;
int ctrl_z,ctrl_c;

//Traiter la ligne de commande lu au clavier
void traiter_commande(struct cmdline *c);

#endif
