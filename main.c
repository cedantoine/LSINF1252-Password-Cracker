#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "constantes.h"

typedef struct node {
struct node *next;
char *value;
} node_t;

//------------------------------------------------------------------------------

int type; //0=voyelles 1=consonnes
int sortie; //0=rien 1=fichier de sortie donné
int nombreDeSources=1;
int nombreDeFichiers=1;

const char** tabfichiers;

uint8_t **buffer1;
pthread_mutex_t mutex1; //mutex et semaphore du getHash
sem_t empty1;
sem_t full1;

char **buffer2;
pthread_mutex_t mutex2; //mutex et semaphore de l'Inverseur
sem_t empty2;
sem_t full2;

int nombreDeHash=0;
int fichierlu=0;

//------------------------------------------------------------------------------

void initbuff1(){

  buffer1=(uint8_t**)malloc(nombreDeSources*2*sizeof(uint8_t*));

  for(int i=0;i<2*nombreDeSources;i++){
    buffer1[i]=NULL;
  };

  printf("Buffer1 initialisé!\n");

  pthread_mutex_init (&mutex1, NULL);
  sem_init (&empty1,0,nombreDeSources*2);
  sem_init (&full1,0,0);
}

void initbuff2(){

  buffer2=(char**)malloc(2*nombreDeSources*7);

  for(int i=0;i<2*nombreDeSources;i++){
    buffer2[i]=NULL;
  }

  printf("Buffer2 initialisé!\n");

  pthread_mutex_init (&mutex2, NULL);
  sem_init (&empty2,0,nombreDeSources*2);
  sem_init (&full2,0,0);
}

//------------------------------------------------------------------------------

void getHash(){

  ssize_t lire=32;
  int lu=0;

  printf("%d\n",nombreDeFichiers);
  printf("%d\n",fichierlu);

  while(fichierlu!=nombreDeFichiers){
    printf("Rentré dan le getHash!\n");

    uint8_t *buff=(uint8_t*)malloc(32);

    const char *file=tabfichiers[fichierlu];

    int ouvert = open(file,O_RDONLY);

    printf("%s\n",file);
    if(ouvert<0){
      printf("Ne peut pas ouvrir le fichier\n");
    }

    while(lire==32){
      printf("Lecture de Fichier!\n");
      lseek(ouvert,lu,SEEK_SET);
      lire=read(ouvert,(void*)buff,(size_t)32);
      sem_wait(&empty1); // attente d'un slot libre
      pthread_mutex_lock(&mutex1);
      for(int i = 0; i<2*nombreDeSources;i++){
        printf("Stockage d'un hash!\n");
        if(buffer1[i]==NULL){
          buffer1[i]=(uint8_t*)malloc(32);
          memcpy(buffer1[i],(uint8_t*)buff,(size_t)32);
          break;
        }
        for(int i=0;i<32;i++){
          printf("%x2",buffer1[0][i]);
        }
      }
      pthread_mutex_unlock(&mutex1);
      sem_post(&full1);
      lu=lu+32;
      printf("Relecture dans le fichier!\n");
    }
    printf("Nettoyage dans Hash!\n");
    close(ouvert);
    fichierlu++;
    free(buff);
  }
}

//------------------------------------------------------------------------------

int main(int argc, const char *argv[]){

  int position=1;
  int nombreDeThread=1;

  type=0;
  sortie=0;

  if(argc==0){
    printf("Veuillez donner un fichier en input!\n");
  }

  if(argc > 2 && strcmp(argv[position],"-t")==0){
    int temp=atoi(argv[position+1]);
    if(temp>=1){
      nombreDeThread=temp;
    }
    position=position+2;
  }

  if(argc > 2 && strcmp(argv[position],"-c")==0){
    type=1;
    position++;
  }

  if(argc > 2 && strcmp(argv[position],"-o")==0){
    sortie=1;
    position++;
  }

  nombreDeFichiers = argc-position;
  int index;
  int x=0;

  tabfichiers=malloc(nombreDeFichiers*sizeof(char*));
  printf("Tableau de Fichier initialisé!\n");
  for(index = position; index < argc ;index++){
    tabfichiers[x]=argv[index];
    x++;
  }
  printf("Fichiers pointés!\n");

  printf("%d\n",nombreDeFichiers);
  printf("%d\n",nombreDeThread);
  printf("%s\n",tabfichiers[0]);

  int error=0;
  //création des threads
  pthread_t threadsG[nombreDeSources];
  // pthread_t threadsI[nombreDeThread];
  // pthread_t threadsT[1];
  initbuff1();

  printf("merde4\n");

  //threads de reverse
  for(int i = 0; i<1; i++){
    printf("caca2\n");
    printf("%p\n",&nombreDeFichiers);
    error = pthread_create(&threadsG[i], NULL,(void*)&getHash ,NULL);
    if(error != 0){
      printf("Création du thread numéro %d \n", i);
      return -1;
    }
  }

  //attente threads reverse
  for(int i = 0; i<1; i++){
    pthread_join(ThreadsG[i],NULL);
  }

  initbuff2();

}
