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
  pthread_mutex_init (&mutex3, NULL);
}

void initbuff2(){

  buffer2=(char**)malloc(2*nombreDeSources*sizeof(char*));

  for(int i=0;i<2*nombreDeSources;i++){
    buffer2[i]=NULL;
  }

  printf("Buffer2 initialisé!\n");

  pthread_mutex_init (&mutex2, NULL);
  sem_init (&empty2,0,nombreDeSources*2);
  sem_init (&full2,0,0);
  pthread_mutex_init (&mutex4, NULL);
}

void initlistchainee(){
  head=(struct node *)malloc(sizeof(struct node));
  head->value=malloc(17*sizeof(char));
  current = head;
  pthread_mutex_init (&mutex5, NULL);

  printf("Liste Chainee initialisé!\n");
}

//------------------------------------------------------------------------------

/*fonction getHash
*---------------------
*La fonction va prendre les fichiers l'un après l'autre et va stocker
*les hash (des fichiers) dans le buffer1.
*/

void getHash(){

  ssize_t lire=32;
  int lu=0;
  int u=0;

  //printf("%d\n",nombreDeFichiers);
  //printf("%d\n",fichierlu);

  while(fichierlu!=nombreDeFichiers){


    uint8_t *buff=(uint8_t*)malloc(32);

    const char *file=tabfichiers[fichierlu];

    int ouvert = open(file,O_RDONLY);

    printf("%s\n",file);
    if(ouvert<0){
      printf("Ne peut pas ouvrir le fichier\n");
    }

    while(lire==32){
      printf("Rentré dan le getHash!\n");
      //printf("Lecture de Fichier!\n");
      lseek(ouvert,lu,SEEK_SET);
      lire=read(ouvert,(void*)buff,(size_t)32);
      sem_wait(&empty1); // attente d'un slot libre
      pthread_mutex_lock(&mutex1);
      for(int i = 0; i<2*nombreDeSources;i++){
        //printf("Stockage d'un hash!\n");
        if(buffer1[i]==NULL){
          buffer1[i]=(uint8_t*)malloc(32);
          memcpy(buffer1[i],(uint8_t*)buff,(size_t)32);
          break;
        }
      }
      pthread_mutex_unlock(&mutex1);
      sem_post(&full1);

      lu=lu+32;
      u++;
      free(buff);
      pthread_mutex_lock(&mutex3);
      nombreDeHash++;
      pthread_mutex_unlock(&mutex3);
      printf("Sorti du get Hash!\n");
    }
    printf("Kill Hash!\n");
    close(ouvert);
    fichierlu++;
  }
}

//------------------------------------------------------------------------------

/*fonction de reverse
*---------------------
*La fonction va prendre les hashs l'un après l'autre dans buffer1 et les
*inverser, puis les remettre dans le buffer2.
*/

void inverseur(){
  //printf("Ouvert l'inverseur!\n");

  char *motdepasse=malloc(17*sizeof(char));

  uint8_t *hash=(uint8_t*)malloc(32);

  //int l=0;

  while(!(nombreDeReverse==nombreDeHash && joinG==1)){

    sem_wait(&full1); // attente d'un slot rempli
    pthread_mutex_lock(&mutex1);

    for(int i=0;i<32;i++){
      //printf("%x2",buffer1[0][i]);
    }
    printf("Rentré dans l'inverseur!\n");

    for(int i = 0; i<2*nombreDeSources;i++){
      if(buffer1[i]!=NULL){
        memcpy(hash,(uint8_t*)buffer1[i],(size_t)32);
        //printf("Hash obtenu!\n");
        free(buffer1[i]);
        buffer1[i]=(uint8_t*)malloc(32);
        buffer1[i]=NULL;
        //printf("Buffer1 vidé!\n");
        break;
      }
    }
    pthread_mutex_unlock(&mutex1);
    sem_post(&empty1); // il y a un slot vide en plus
    //printf("Sorti premier buffer!\n");

    // for(int i=0;i<32;i++){
    //   printf("%x2",hash[i]);
    // }

    reversehash((uint8_t*)hash,(char*)motdepasse,(size_t)16);

    //printf("Hash inversé!\n");

    sem_wait(&empty2); // attente d'un slot libre
    pthread_mutex_lock(&mutex2);
    for(int i = 0; i<2*nombreDeSources;i++){
      if(buffer2[i]==NULL){
        buffer2[i]=(char*)malloc(16);
        strcpy(buffer2[i],motdepasse);
        break;
      }
      //printf("Motdepasse stocké!\n");
      //printf("%s %s\n",buffer2[l],"YAAAAASSS");
    }
    pthread_mutex_unlock(&mutex2);
    sem_post(&full2); // il y a un slot rempli en plus
    free(motdepasse);
    motdepasse=malloc(17*sizeof(char));
    //l++;
    pthread_mutex_lock(&mutex4);
    nombreDeReverse++;
    pthread_mutex_unlock(&mutex4);
    printf("Sorti de L'Inverseur\n");
  }
  printf("Kill Inverseur\n");
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

  //printf("Trieur ouvert\n");

  int r;
  char *candidat=malloc(17*sizeof(char));

  while(!(nombreTrie==nombreDeReverse && joinI==1)){
    printf("Entré dans Trieur\n");

    sem_wait(&full2); // attente d'un slot rempli
    pthread_mutex_lock(&mutex2);
    for(int i = 0; i<2*nombreDeSources;i++){
      //printf("Entré dans boucle for de Trieur!\n");
      if(buffer2[i]!=NULL){
        strcpy(candidat,buffer2[i]);
        //printf("Strcpy done!\n");
        free(buffer2[i]);
        buffer2[i]=malloc(17*sizeof(char));
        buffer2[i]=NULL;
        break;
      }
    }
    pthread_mutex_unlock(&mutex2);
    sem_post(&empty2); // il y a un slot vide en plus
    printf("%s\n",candidat );
    //printf("Mdp copié!\n");
    r=compteur(candidat);
    //printf("Nb calculé!\n");
    //printf("%d\n",r);
    if(r>nombre){
      nombre=r;
      printf("Il est plus grand!\n");
      //printf("%d\n",nombre);
      strcpy(head->value,candidat);
      //printf("Ca marche?\n");
      head->next=NULL;
      //printf("Et la?\n");
    }
    // else if(r==nombre){
    //   struct node *next=(struct node *)malloc(sizeof(struct node));
    //   strcpy(next->value,candidat);
    //   current->next=next;
    //   current=next;
    // }
    pthread_mutex_lock(&mutex5);
    nombreTrie++;
    pthread_mutex_unlock(&mutex5);
    printf("Sorti du trieur\n");
  }
  printf("Kill Trieur\n");
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
    printf("%d\n",nombreDeThread);
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
  //printf("Tableau de Fichier initialisé!\n");
  for(index = position; index < argc ;index++){
    tabfichiers[x]=argv[index];
    x++;
  }
  //printf("Fichiers pointés!\n");

  //printf("%d\n",nombreDeFichiers);
  //printf("%d\n",nombreDeThread);
  //printf("%s\n",tabfichiers[0]);

  int error=0;

  //création des threads
  pthread_t threadsG[1];
  pthread_t threadsI[nombreDeThread];
  pthread_t threadsT[1];

  initbuff1();

  //threads de getHash
  for(int i = 0; i<1; i++){
    error = pthread_create(&threadsG[i], NULL,(void*)&getHash ,NULL);
    if(error != 0){
      printf("Création du thread numéro %d \n", i);
      return -1;
    }
  }

  initbuff2();

  //threads de reverse
  for(int i = 0; i<2; i++){
    error = pthread_create(&threadsI[i], NULL,(void*)&inverseur ,NULL);
    if(error != 0){
      printf("Création du thread numéro %d \n", i);
      return -1;
    }
  }

  initlistchainee();

  //threads de triage
  for(int i = 0; i<1; i++){
    error = pthread_create(&threadsT[i], NULL,(void*)&trieur ,NULL);
    if(error != 0){
      printf("Création du thread numéro %d \n", i);
      return -1;
    }
  }

  //attente threads  getHash
  for(int i = 0; i<1; i++){
    pthread_join(threadsG[i],NULL);
    joinG++;
  }

  //attente threads reverse
  for(int i = 0; i<2; i++){
    pthread_join(threadsI[i],NULL);
    joinI++;
  }


  //attente threads de triage
  for(int i = 0; i<1; i++){
    pthread_join(threadsT[i],NULL);
  }

  return EXIT_SUCCESS;

}
