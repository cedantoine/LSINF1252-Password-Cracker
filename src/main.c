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
#include "./Template/reverse.h"
#include "./Template/sha256.h"

typedef struct node {
  struct node *next;
  char *value;
} node_t;

//------------------------------------------------------------------------------

int type; //0=voyelles 1=consonnes
int sortie; //0=rien 1=fichier de sortie donné
int nombreDeSources=1;
int nombreDeFichiers=1;
int nombreDeThread=1;

const char** tabfichiers;
const char* fichiersortie;

uint8_t **buffer1;
pthread_mutex_t mutex1; //mutex et semaphore du getHash
sem_t empty1;
sem_t full1;
pthread_mutex_t mutex3;

char **buffer2;
pthread_mutex_t mutex2; //mutex et semaphore de l'Inverseur
sem_t empty2;
sem_t full2;
pthread_mutex_t mutex4;

struct node *head;
struct node *current;
pthread_mutex_t mutex5;

int nombreDeHash=0;
int fichierlu=0;

int nombreDeReverse=0;

int nombre=0; //nombre voyelles ou consonnes max de la liste
int nombreTrie=0;

int joinG=0;
int joinI=0;

pthread_mutex_t mutex6;
pthread_mutex_t mutex7;

//------------------------------------------------------------------------------

void initbuff1(){

  buffer1=(uint8_t**)malloc(nombreDeSources*2*sizeof(uint8_t*));
  int p;

  for(p=0;p<2*nombreDeSources;p++){
    buffer1[p]=NULL;
  };


  pthread_mutex_init (&mutex1, NULL);
  sem_init (&empty1,0,nombreDeSources*2);
  sem_init (&full1,0,0);
  pthread_mutex_init (&mutex3, NULL);
}

void initbuff2(){

  buffer2=(char**)malloc(2*nombreDeSources*sizeof(char*));

  int z;
  for(z=0;z<2*nombreDeSources;z++){
    buffer2[z]=NULL;
  }


  pthread_mutex_init (&mutex2, NULL);
  sem_init (&empty2,0,nombreDeSources*2);
  sem_init (&full2,0,0);
  pthread_mutex_init (&mutex4, NULL);
  pthread_mutex_init (&mutex6, NULL);
}

void initlistchainee(){
  head=(struct node *)malloc(sizeof(struct node));
  head->value=malloc(17*sizeof(char));
  head->next=NULL;
  current = head;
  current->value=head->value;
  pthread_mutex_init (&mutex5, NULL);
  pthread_mutex_init (&mutex7, NULL);

}

//------------------------------------------------------------------------------

/*fonction getHash
*---------------------
*La fonction va prendre les fichiers l'un après l'autre et va stocker
*les hash (des fichiers) dans le buffer1.
*/

void getHash(){

  int lire=32;
  int lu=0;
  uint8_t *buff;

  while(fichierlu!=nombreDeFichiers){

    lu=0;


    const char *file=tabfichiers[fichierlu];

    int ouvert = open(file,O_RDONLY);

    if(ouvert<0){
      printf("Ne peut pas ouvrir le fichier\n");
    }

    while(ouvert>0){
      //printf("Rentré dan le getHash!\n");
      buff=(uint8_t*)malloc(32);
      lseek(ouvert,lu,SEEK_SET);
      lire=read(ouvert,(void*)buff,(size_t)32);
      if(lire!=32){
        break;
      }

      sem_wait(&empty1); // attente d'un slot libre
      pthread_mutex_lock(&mutex1);
      int o;
      for(o = 0; o<2*nombreDeSources;o++){
        if(buffer1[o]==NULL){
          free(buffer1[o]);
          buffer1[o]=(uint8_t*)malloc(32);
          memcpy(buffer1[o],(uint8_t*)buff,(size_t)32);
          break;
        }
      }
      pthread_mutex_unlock(&mutex1);
      sem_post(&full1);
      lu=lu+32;

      free(buff);

      pthread_mutex_lock(&mutex3);
      nombreDeHash++;
      pthread_mutex_unlock(&mutex3);


    }
    close(ouvert);
    free(buff);
    fichierlu++;

  }
  pthread_mutex_lock(&mutex7);
  joinG++;
  pthread_mutex_unlock(&mutex7);
  //printf("Kill Hash!\n");
  pthread_exit(NULL);
}

//------------------------------------------------------------------------------

/*fonction de reverse
*---------------------
*La fonction va prendre les hashs l'un après l'autre dans buffer1 et les
*inverser, puis les remettre dans le buffer2.
*/

void inverseur(){

  char *motdepasse=malloc(17*sizeof(char));

  uint8_t *hash=(uint8_t*)malloc(32);

  pthread_mutex_lock(&mutex3);
  pthread_mutex_lock(&mutex4);

  while(!(nombreDeReverse==nombreDeHash && joinG==1)){

    pthread_mutex_unlock(&mutex3);
    pthread_mutex_unlock(&mutex4);

    sem_wait(&full1); // attente d'un slot rempli

    pthread_mutex_lock(&mutex4);
    nombreDeReverse++;
    pthread_mutex_unlock(&mutex4);

    pthread_mutex_lock(&mutex1);

    int u;
    for(u = 0; u<2*nombreDeSources;u++){
      if(buffer1[u]!=NULL){
        memcpy(hash,(uint8_t*)buffer1[u],(size_t)32);
        free(buffer1[u]);
        buffer1[u]=NULL;
        break;
      }
    }

    pthread_mutex_unlock(&mutex1);
    sem_post(&empty1); // il y a un slot vide en plus

    reversehash((uint8_t*)hash,(char*)motdepasse,(size_t)16);

    sem_wait(&empty2); // attente d'un slot libre
    pthread_mutex_lock(&mutex2);

    int f;
    for(f = 0; f<2*nombreDeSources;f++){
      if(buffer2[f]==NULL){
        free(buffer2[f]);
        buffer2[f]=(char*)malloc(16);
        strcpy(buffer2[f],motdepasse);
        break;
      }

    }
    pthread_mutex_unlock(&mutex2);
    sem_post(&full2); // il y a un slot rempli en plus
    free(motdepasse);
    motdepasse=malloc(17*sizeof(char));


    pthread_mutex_lock(&mutex3);
    pthread_mutex_lock(&mutex4);

  }

  pthread_mutex_unlock(&mutex3);
  pthread_mutex_unlock(&mutex4);


  free(motdepasse);
  free(hash);

  pthread_mutex_lock(&mutex6);
  joinI++;
  pthread_mutex_unlock(&mutex6);

  pthread_exit(NULL);
}

//------------------------------------------------------------------------------

/*fonction de comptage
*---------------------
*La fonction va compter le nombre de voyelles ou de consonnes d'un password.
*@params: prend comme argument un password
*/

int compteur(char *password){
  int j=0;
  int nombretemp=0; //nombre de voyelles ou consonnes du mot
  while(password[j]!='\0'){
    if(type==0){
      if(password[j]=='i' || password[j]=='o' || password[j]=='u'
      || password[j]=='e' || password[j]=='a'|| password[j]=='y'){
        nombretemp++;
      }
    }
    else if(type==1){
      if(password[j]!='i' && password[j]!='o' && password[j]!='u'
      && password[j]!='e' && password[j]!='a'&& password[j]!='y'){
        nombretemp++;
      }
    }
    j++;
  }
  return(nombretemp);
}

/*fonction de triage
*---------------------
*La fonction va prendre les mots de passe les uns après les autres et les trier
*selon leur type (plus d'occurence de consonne ou voyelle) puis les remettre
*dans un nouveau tableau.
*/

void trieur(){

  int r;
  char *candidat=malloc(17*sizeof(char));

  pthread_mutex_lock(&mutex4);
  pthread_mutex_lock(&mutex5);

  while(!(nombreTrie==nombreDeReverse && joinI==nombreDeThread)){

    pthread_mutex_unlock(&mutex4);
    pthread_mutex_unlock(&mutex5);

    pthread_mutex_lock(&mutex5);
    nombreTrie++;
    pthread_mutex_unlock(&mutex5);

    sem_wait(&full2); // attente d'un slot rempli

    pthread_mutex_lock(&mutex2);
    int y;
    for(y = 0; y<2*nombreDeSources;y++){
      if(buffer2[y]!=NULL){
        strcpy(candidat,buffer2[y]);
        free(buffer2[y]);
        buffer2[y]=NULL;
        break;
      }
    }
    printf("%s\n",candidat );
    pthread_mutex_unlock(&mutex2);
    sem_post(&empty2); // il y a un slot vide en plus
    r=compteur(candidat);

    if(nombre==0){
      strcpy(head->value,candidat);
      nombre=r;
    }

    else if(r>nombre){
      nombre=r;
      current=head;
      while(head->value!=NULL){
        free(head->value);
        if(head->next!=NULL){
          head=head->next;
        }
        else{
          head->value=NULL;
        }
        if(current->value!=NULL){
          free(current);
        }
        current=head;
      }
      free(head);

      head=(struct node *)malloc(sizeof(struct node));

      head->value=malloc(17*sizeof(char));
      strcpy(head->value,candidat);
      head->next=NULL;
    }
    else if(r==nombre){
      struct node *next=(struct node *)malloc(sizeof(struct node));
      next->value=malloc(17*sizeof(char));
      strcpy(next->value,candidat);
      next->next=NULL;
      current->next=next;
      current=next;
    }


    pthread_mutex_lock(&mutex4);
    pthread_mutex_lock(&mutex5);

  }

  pthread_mutex_unlock(&mutex4);
  pthread_mutex_unlock(&mutex5);

  free(candidat);
  pthread_exit(NULL);

}

//------------------------------------------------------------------------------
int main(int argc, const char *argv[]){

  int position=1;

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
    fichiersortie=malloc(sizeof(char*));
    fichiersortie=argv[position+1];
    position=position+2;
  }

  nombreDeFichiers = argc-position;
  int index;
  int x=0;

  tabfichiers=malloc(nombreDeFichiers*sizeof(char*));
  for(index = position; index < argc ;index++){
    tabfichiers[x]=argv[index];
    x++;
  }

  int error=0;

  //création des threads
  pthread_t threadsG[1];
  pthread_t threadsI[nombreDeThread];
  pthread_t threadsT[1];


  initbuff1();

  //threads de getHash
  int k;
  for(k = 0; k<1; k++){
    error = pthread_create(&threadsG[k], NULL,(void*)&getHash ,NULL);
    if(error != 0){
      printf("Création du thread numéro %d \n", k);
      return -1;
    }
  }

  initbuff2();

  //threads de reverse
  int h;
  for(h = 0; h<nombreDeThread; h++){
    error = pthread_create(&threadsI[h], NULL,(void*)&inverseur ,NULL);
    if(error != 0){
      printf("Création du thread numéro %d \n", h);
      return -1;
    }
  }

  initlistchainee();

  //threads de triage
  int l;
  for(l = 0; l<1; l++){
    error = pthread_create(&threadsT[l], NULL,(void*)&trieur ,NULL);
    if(error != 0){
      printf("Création du thread numéro %d \n", l);
      return -1;
    }
  }

  //attente threads  getHash
  int b;
  for(b = 0; b<1; b++){
    pthread_join(threadsG[b],NULL);
  }

  //attente threads reverse
  int a;
  for(a = 0; a<nombreDeThread; a++){
    pthread_join(threadsI[a],NULL);
  }

  //attente threads de triage
  int c;
  for(c = 0; c<1; c++){
    pthread_join(threadsT[c],NULL);
  }

  pthread_mutex_destroy(&mutex1);
  pthread_mutex_destroy(&mutex2);
  pthread_mutex_destroy(&mutex3);
  pthread_mutex_destroy(&mutex4);
  pthread_mutex_destroy(&mutex5);
  pthread_mutex_destroy(&mutex6);
  pthread_mutex_destroy(&mutex7);

  sem_destroy(&empty1);
  sem_destroy(&empty2);
  sem_destroy(&full1);
  sem_destroy(&full2);

  current=head;
  if(sortie==1){
    int ouvert = open(fichiersortie,O_RDWR|O_APPEND|O_CREAT);
    if(ouvert<0){
      printf("Ne peut pas ouvrir le fichier\n");
    }
    while(head!=NULL){
      int ecriture=write(ouvert,(void *)head->value, (size_t)sizeof(char)*17);
      if(ecriture<0){
        printf("Problème d'écriture dans le fichier\n");
      }
      write(ouvert,"\n",1);
      free(head->value);
      head=head->next;
      free(current);
      current=head;
    }
    close(ouvert);
  }
  else{
    printf("Les candidats sont:\n");
    while(head!=NULL){
      if(head->value!=NULL){
        printf("%s\n", head->value);
        free(head->value);
      }
      if(head->next!=NULL){
        head=head->next;
      }
      else{
        head=NULL;
      }
      if(current!=NULL){
        free(current);
      }
      current=head;
    }
  }

  free(tabfichiers);
  free(buffer1);
  free(buffer2);
  return EXIT_SUCCESS;

}
