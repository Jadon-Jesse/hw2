#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXWORD 1024

pthread_t thread[4];
pthread_mutex_t lock;

typedef struct dict {
  char *word;
  int count;
  struct dict *next;
} dict_t;

typedef struct words
{
	dict_t *wordDict;
    char wordbuffer[MAXWORD];
	
}word_s;


char *
make_word( char *word ) {
  return strcpy( malloc( strlen( word )+1 ), word );
}

void *
make_dict(void *wordIn) {
  char * word = (char *) wordIn;

  //printf("%s\n", "Making dictionary");
  pthread_mutex_lock (&lock);
  dict_t *nd = (dict_t *) malloc( sizeof(dict_t) );
  nd->word = make_word( word );
  nd->count = 1;
  nd->next = NULL;
  pthread_mutex_unlock (&lock);
  return (void *)nd;
}

void *
insert_word( void * listing){

word_s *inGet = malloc(sizeof (*inGet));
inGet = (word_s *) listing;
dict_t *d =(dict_t *) inGet->wordDict;
char * word = (char *)inGet->wordbuffer;

  dict_t *nd;
  dict_t *pd = NULL;		// prior to insertion point 
  dict_t *di = d;		// following insertion point

  while(di && ( strcmp(word, di->word ) >= 0) ) { 
    if( strcmp( word, di->word ) == 0 ) { 
      di->count++;		// increment count 
      return (void *)d;			// return head 
    }
    pd = di;			// advance ptr pair
    di = di->next;
  }

  void * stat;
  pthread_create(&thread[3], NULL, make_dict, (void *) word);
  pthread_join(thread[3], &stat);

  dict_t *newD = malloc(sizeof(*newD));
  newD = (dict_t *) stat;
  
  newD->next = di;		// entry bigger than word or tail 
  if (pd) {
    pd->next = newD;
    return (void *) d;			// insert beond head 
  }
 //free(inGet);
  return (void * )newD;
}


void * print_dict( void *dict) {

    dict_t  * d = NULL;
	d = ( dict_t *) dict;
  while (d) {
    printf("[%d] %s\n", d->count, d->word);
    d = d->next;
  }
}

int
get_word( char *buf, int n, FILE *infile) {
  int inword = 0;
  int c;  
  while( (c = fgetc(infile)) != EOF ) {
    if (inword && !isalpha(c)) {
      buf[inword] = '\0';	// terminate the word string
      return 1;
    } 
    if (isalpha(c)) {
      buf[inword++] = c;
    }
  }
  return 0;			// no more words
}





void *
words( void * in ) {

  FILE *infile = NULL;
  infile = (FILE * ) in;
  dict_t *wd = NULL;
  char wordbuf[MAXWORD] ;//=malloc(sizeof(char));

  void * statis;

  word_s *send = malloc(sizeof (*send));
  

int d=0;
  
  while( get_word( wordbuf, MAXWORD, infile ) ) {

  	send->wordDict = wd;

 	strcpy(send->wordbuffer, wordbuf); 
 	//printf("%d%s\n", d++, send->wordbuffer);
  	pthread_create(&thread[2], NULL, insert_word, (void *) send);
  	pthread_join(thread[2], &statis);
  	wd = statis;
   // wd = insert_word(wd, wordbuf); // add to dict
  }

 
  free (send);
  return (void *) wd;
}

int
main( int argc, char *argv[] ) {
  dict_t *d = NULL;
  FILE *infile = stdin;
  if (argc >= 2) {
    infile = fopen (argv[1],"r");
  }
  if( !infile ) {
    printf("Unable to open %s\n",argv[1]);
    exit( EXIT_FAILURE );
  }

 void * status;

 int rc = pthread_create(&thread[0], NULL, words, (void *) infile);
 pthread_join(thread[0], &status);

 int rp = pthread_create(&thread[1], NULL, print_dict, (void *) status);

 
 pthread_join(thread[1], NULL);// must join threads with function!

  fclose( infile );
}
