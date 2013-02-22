/*****  fpicmakeindex.exe  *****
  create an index file for firstpic program
  */
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define MAXLETTER 1024
#define step 10000

typedef struct{
  char *inName;
  char *tmpname;
  char *name;
  struct stat sb0;
  struct stat sb1;
  FILE *fp;
}OUTFILE;

void help(void);
int make_index(char *inName, int overWrite);
int NameAndPosition(FILE **fin, FILE **fout);
OUTFILE create_outfile(char *inName, int overWrite);
int rename_outfile(OUTFILE outfile);

int main(int argc, char *argv[]){
  int modified;
  int overwrite=0;

  /* version information */
  fprintf(stderr,"fpicmakeindex ver.1 \n");
  if (argc!=2) help();

  do{
    modified=make_index(argv[1], overwrite);
    overwrite++;
  }while(modified!=0);

  printf("Index created successfully after %d trial\n", overwrite);
  return 0;
}

int make_index(char *inName, int overWrite){
  FILE *fin;
  char c;
  long seqs;
  int modified;
  OUTFILE outfile;

  /* infile open */
  if ((fin=fopen(inName,"r"))==NULL){
    fprintf(stderr,"Cannot open infile.[%s]\n", inName);
    help();
    exit(0);
  }

  outfile=create_outfile(inName, overWrite);
  printf("Creating index files ...  ");
  fflush(stdout);
  
  /* create header */
  fprintf(outfile.fp,"index file ver.1 made by fpicmakeindex\n");
  fprintf(outfile.fp,"step=%d\n",step);
  fprintf(outfile.fp,"data start\n\n");
  
  /* search seqname */
  while(feof(fin)==0){
    c=fgetc(fin);
    if (c=='>') break;
  }
  if(feof(fin)==0){
    seqs=1;
    while(NameAndPosition(&fin,&outfile.fp)) seqs++;
    printf("Found %ld sequences\n",seqs);
  }else{
    fprintf(stderr, "Could not detect fasta sequence.\n");
    fprintf(stderr, "Is this file fasta format?.\n");
  }
  fclose (fin);
  fclose (outfile.fp);

  modified=rename_outfile(outfile);
  return modified;
}

OUTFILE create_outfile(char *inName, int overWrite){
  OUTFILE outfile;
  int namelength;
  char c;
  time_t timer;
  FILE *pointer;

  if (stat(inName, &outfile.sb0) == -1) {
    fprintf(stderr, "Could not get file information on %s in line %d.\n"
      , inName, __LINE__);
    exit(1);
  }

  namelength=strlen(inName);
  outfile.inName=inName;
  outfile.name=(char*)malloc(namelength+10);
  sprintf(outfile.name, "%s.index", inName);

  if((pointer=fopen(outfile.name, "r"))!=NULL && overWrite==0){
    fprintf(stderr,"Index file already exists, replace? (Y/N)  ");
    c=getchar();
    if (c!='y' && c!='Y') exit(0);
    fprintf(stderr,"Okey, this program will replace index file.\n");
    fclose(pointer);
  }

  time(&timer);
  outfile.tmpname=(char*)malloc(namelength+50);
  sprintf(outfile.tmpname, "%s.%d", outfile.name, (int)timer);
  if ((outfile.fp=fopen(outfile.tmpname,"w"))==NULL){
    fprintf(stderr,"Cannot open outfile.[%s]\n",outfile.tmpname);
    help();
    exit(0);
  }

  return outfile;
}

int rename_outfile(OUTFILE outfile){
  int result=0;
  if (stat(outfile.inName, &outfile.sb1) == -1) {
    fprintf(stderr, "Could not get file information on %s in line %d.\n"
	    , outfile.inName, __LINE__);
    exit(EXIT_SUCCESS);
  }
  if(outfile.sb0.st_mtime != outfile.sb1.st_mtime){
    fprintf(stderr, "The original file is modified during indexing!\n");
    fprintf(stderr, "Will delete the index file and re-index the data\n");
    remove(outfile.tmpname);
    result=1;
  }else{
    if(rename(outfile.tmpname, outfile.name)!=0){
      fprintf(stderr, "Could not rename tmpfile!\n");
      result=1;      
    }
  }
  return result;
}

int NameAndPosition(FILE **fin, FILE **fout){
  unsigned long long int position;
  long count;
  char c='>';
  
  /* get seqname */
  while(c!='\n'){
    fputc(c,*fout);
    c=fgetc(*fin);
  }
  
  /* store position */
  position=ftello(*fin);
  fprintf(*fout,"\n%lld\n",position-1);
  
  /* position marking */
  count=0;
  while(1){
    c=fgetc(*fin);
    /*if (('a'<=c && c<='z') || ('A'<=c && c<='Z')) {*/
    if (c >= ' ' && c <= '~') {
      /* not (a-z) || (A-Z) because it have to recognize
	 letters such as '*' or '-' */
      count++;
      if (count % step==0) fprintf(*fout,"%lld\n",(long long int)ftello(*fin));
    }
    if (c=='>') return (1);
    if (c==EOF) return (0);
  }
}


void help(void){
  fprintf(stderr,"fpicmakeindex.exe written by Uhmin\n");
  fprintf(stderr,"%s compiled on %s %s\n",__FILE__, __DATE__, __TIME__);
  fprintf(stderr,"Create index file for firstpic.exe\n");
  fprintf(stderr,"+++ usage +++\n");
  fprintf(stderr,"fpicmakeindex.exe (infile)\n");
  fprintf(stderr,"index-file-name will be \"infile.index\"\n");
  fprintf(stderr,"\n");
  exit (0);
}
