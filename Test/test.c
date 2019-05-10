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
#include "../constantes.h"
#include "../Template/reverse.h"
#include "../Template/sha256.h"
#include "CUnit/Basic.h"
#include <CUnit/CUnit.h>


int type;

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


void compteurvoy(void) { //test servant a verifier qu'il compte bien le nombre de voyelle
  char *voy="coucou";
  type=0;
  int reponse=4;
	CU_ASSERT_EQUAL(compteur(voy),reponse);
}

void compteurcons(void) { //test servant a verifier qu'il compte bien le nombre de consonne
  char *voy="coucou";
  type=1;
  int reponse=2;
	CU_ASSERT_EQUAL(compteur(voy),reponse);
}



int main(int argc, const char *argv[]) {
	CU_pSuite pSuite = NULL;

	if(CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

//création de la suite
	pSuite = CU_add_suite("Suite",NULL,NULL);
	if(NULL == pSuite){
		CU_cleanup_registry();
		return CU_get_error();
	}

//ajout des tests
	if(NULL == CU_add_test(pSuite, "CompteurVoy", compteurvoy) ||
	   NULL == CU_add_test(pSuite, "CompteurCons", compteurcons))
	   {
		CU_cleanup_registry();
		return CU_get_error();
	}

//run les tests
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}
