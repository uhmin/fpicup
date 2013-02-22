/* fpicup2.c */
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#define _INCLUDE_ "firstpic_b_all3.100420.c"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "fpicup_header.c"
#include _INCLUDE_

#define MAXLETTER 512

typedef struct{
  char *str;
  int buffer;
  int length;
}STRING;

typedef struct{
  FILE *mainfile;
  FILE *fout;
  FILE *indexfile;
  FILE *bindexfile;
  char *key;
  long from;
  long to;
  int width;
  int ranking_from;
  int ranking_width;
  char *list_file;
  STRING index;
  STRING bindex;
}OPTIONS;

void help(void);
void indexhelp(char *level);
OPTIONS interface(int argc, char *argv[]);
void display(FILE *fout, RESULT result, int width);
char **get_keylist(int from, int width, FILE *indexfile);
char **read_keylist(char *list_file, int *list_number);
int count_sequence(FILE *indexfile);
int free_outseq(RESULT result);
int sstrcat(STRING *string, char *a, char *b);

int main(int argc, char *argv[]){
  char *tmp=NULL, **keylist;
  long from, to;
  /*long originalFrom, originalTo;*/
  int step, i;
  RESULT outseq;
  OPTIONS options;
  outseq.sequence=NULL;

  options=interface(argc, argv);

  if(strcmp(options.key,">")==0 && options.ranking_from==0 && options.list_file==NULL) {
    fprintf(options.fout,"%d\n",count_sequence(options.indexfile));
    exit(0);
  }

  /* check index file */
  while(1){
    if(tmp!=NULL) {free(tmp); tmp=NULL;}
    if (uhmin_fgets(&tmp,MAXLETTER-1,options.indexfile)==NULL) {
      fprintf(stderr,"index err\n");
      exit(0);
    }
    if (strstr(tmp,"step=")!=NULL){
      step=atoi(tmp+5);
      break;
    }
  }

  /* make key list */
  if(options.list_file != NULL){
    keylist=read_keylist(options.list_file, &options.ranking_width);
  }else if(strcmp(options.key,">")==0 
     || options.ranking_from!=0){ /* get multisequence from head */
    keylist=get_keylist(options.ranking_from, options.ranking_width, options.indexfile);
    if(keylist==NULL){
      /*fprintf(stderr,"no data found\n");*/
      exit(0);
    } 
  }else{
    keylist=(char **)malloc((size_t)10);
    keylist[0]=options.key;
  }

  /*printf("step=%d\n",step);*/
  for(i=0; i<options.ranking_width; i++){
    from = options.from;
    to   = options.to;
    if (keylist[i]!=NULL && keylist[i][0]!='\0'){
      outseq=firstpic_bi(options.mainfile, options.indexfile, options.bindexfile
			 , &keylist[i], &from, &to, step);
      if(outseq.sequence!=NULL){
	display(options.fout, outseq, options.width);
      }
      free_outseq(outseq);
    }
  }

  fclose(options.mainfile);
  fclose(options.fout);
  fclose(options.indexfile);
  fclose(options.bindexfile);
  if(tmp!=NULL) {free(tmp); tmp=NULL;}
  return 0;
}

int free_outseq(RESULT result){
  if(result.name!=NULL){
    free(result.name);
    result.name=NULL;
  }
  if(result.sequence!=NULL){
    free(result.sequence);
    result.sequence=NULL;
  }
  result.status=0;
  result.truncate=0;
  result.namelength=0;
  result.seqlength=0;
  result.namebuffer=0;
  result.seqbuffer=0;
  return 0;
}

void display(FILE *fout, RESULT result, int width){
  long i=0;

  /* seqname */
  /*fprintf(fout,"%s",seqname);*/
  fprintf(fout, "%s", result.name);

  /* sequence */
  while(result.sequence[i]!='\0'){
    fprintf(fout,"%c",result.sequence[i]);
    if(width!=0 && (i+1) % width == 0 && i!=0)
      fprintf(fout,"\n");
    
    i++;
  }
  if(width == 0 || i % width!=0) fprintf(fout,"\n");
}


char **get_keylist(int from, int width, FILE *indexfile){
  char *tmpname=NULL, *name_found;
  char **namelist;
  size_t list_size=width+10;
  int i;

  namelist=(char **)malloc(list_size*sizeof(char**));
  /* to find first name */
  fseek(indexfile,(unsigned long long int)0,SEEK_SET);

  for(i=0;i<from;){
    if(tmpname!=NULL) {free(tmpname); tmpname=NULL;}
    if(uhmin_fgets(&tmpname,MAXLETTER-1,indexfile)==NULL){
      fprintf(stderr,"no data found\n");
      return NULL;
    }
    if(tmpname[0]=='>') i++;
  }

  for(i=0;i<width;i++){
    namelist[i]=NULL;
    do{
      if(i+3>list_size){
	list_size+=10;
	namelist=(char **)realloc(namelist, list_size*sizeof(char **));
      }
      if(tmpname[0]=='>'){
	strtok(tmpname," ");
	name_found=(char *)malloc((strlen(tmpname)+1)*sizeof(char));
	chomp(tmpname);
	/* fprintf(stderr, "name: '%s'\n", tmpname); */
	sprintf(name_found,"%s",tmpname+1);
	namelist[i]=name_found;
	if(tmpname!=NULL) {free(tmpname); tmpname=NULL;}
	uhmin_fgets(&tmpname,MAXLETTER-1,indexfile);
	break;
      }
      if(tmpname!=NULL) {free(tmpname); tmpname=NULL;}
    }while(uhmin_fgets(&tmpname,MAXLETTER-1,indexfile));
  }
  fseek(indexfile,(unsigned long long int)0,SEEK_SET);
  if(tmpname!=NULL) {free(tmpname); tmpname=NULL;}
  return namelist;
}


char **read_keylist(char *list_file, int *list_number){
  char *tmpname=NULL, *name_found;
  char **namelist;
  FILE *flist;
  int list_size=MAXLETTER, i;

  namelist=(char **)malloc(list_size*sizeof(char**));

  if((flist=fopen(list_file,"r"))==NULL){
    fprintf(stderr, "Could not open list file [%s]\n", list_file);
    exit(0);
  }

  if(tmpname!=NULL){free(tmpname); tmpname=NULL;}
  for(i=0;uhmin_fgets(&tmpname,MAXLETTER-1,flist);i++){
    if(i+1>=list_size){
      list_size+=MAXLETTER;
      namelist=(char **)realloc(namelist,list_size*sizeof(char**));
    }
    chomp(tmpname);

    name_found=(char *)malloc((strlen(tmpname)+1)*sizeof(char));
    sprintf(name_found,tmpname);
    namelist[i]=name_found;
    if(tmpname!=NULL){free(tmpname); tmpname=NULL;}
  }
  if(tmpname!=NULL){free(tmpname); tmpname=NULL;}

  fclose(flist);
  *list_number=i;
  if(tmpname!=NULL) {free(tmpname); tmpname=NULL;}
  return namelist;
}


int count_sequence(FILE *indexfile){
  int i=0;
  char *tmp=NULL;

  if(tmp!=NULL){free(tmp); tmp=NULL;}
  while(uhmin_fgets(&tmp,MAXLETTER-1,indexfile)){
    if(tmp[0]=='>') i++;
    if(tmp!=NULL){free(tmp); tmp=NULL;}
  }
  if(tmp!=NULL){free(tmp); tmp=NULL;}
  return i;
}

OPTIONS interface(int argc, char *argv[]){
  int count,infile=-1,outfile=-1,region=-1, ranking=-1, listN=-1, key=-1;
  int n,p;
  char tmp[MAXLETTER];
  OPTIONS result;

  /* default */
  result.mainfile=stdin;
  result.fout=stdout;
  result.width=60;
  result.from=0;
  result.to=0;
  result.ranking_width=1;
  result.ranking_from=0;
  result.list_file=NULL;
  result.key=NULL;

  /* assign */
  if ((argc % 2)==0) help();
  
  for(count=1;count<argc-1;count+=2)
    switch(argv[count][1]){
    case 'i':
      infile=count+1;
      break;
    case 'o':
      outfile=count+1;
      break;
    case 'r':
      region=count+1;
      break;
    case 'k':
      key=count+1;
      break;
    case 'w':
      result.width=atoi(argv[count+1]);
      if (result.width<0) help();
      break;
    case 's':
      ranking=count+1;
      break;
    case 'l':
      listN=count+1;
      break;
    default:
      help();
      break;
    }

  /* key */
  if(key!=-1){
    result.key=(char*)malloc((strlen(argv[key])+1)*sizeof(char));
    strcpy(result.key, argv[key]);
  }else{
    result.key=(char*)malloc(10);
    strcpy(result.key, ">");
  }

  /* infile */
  if (infile!=-1) {
    if ((result.mainfile=fopen(argv[infile],"r"))==NULL){
      fprintf(stderr,"Cannot open infile.[%s]\n",argv[infile]);
      exit(0);
    }
    /* index file */
    sstrcat(&result.index, argv[infile], ".index");
    compareTImeStamp(argv[infile], result.index.str, "index file");
    if ((result.indexfile=fopen(result.index.str,"r"))==NULL){
      fprintf(stderr,"Cannot open index file.[%s]\n",result.index.str);
      indexhelp("index");
      exit(0);
    }
    
    /* bindex file */
    sstrcat(&result.bindex, argv[infile], ".bindex");
    compareTImeStamp(result.index.str, result.bindex.str, "bindex file");
    if ((result.bindexfile=fopen(result.bindex.str,"r"))==NULL){
      fprintf(stderr,"Cannot open bindex file.[%s]\n", result.bindex.str);
      indexhelp("bindex");
      exit(0);
    }
    /*printf("bindex file: %s\n",result.bindex.str);*/
  }else{
    fprintf(stderr,"infile name is essential\n");
    help();
  }
  
  /* outfile */
  if (outfile!=-1){
    if ((result.fout=fopen(argv[outfile],"w"))==NULL){
      fprintf(stderr,"Cannot open outfile.[%s]\n",argv[outfile]);
      exit(0);
    }
  }
  
  /* region */
  if (region!=-1){
    n=0;
    for(p=0;argv[region][p]!='-';p++){
      tmp[n++]=argv[region][p];
      if (argv[region][p]=='\0') help();
    }
    tmp[n]='\0';
    result.from=atol(tmp);

    n=0;p++;
    for(;argv[region][p]!='\0';p++){
      tmp[n++]=argv[region][p];
    }
    tmp[n]='\0';
    result.to=atol(tmp);
  }

  /* ranking */
  if (ranking!=-1){
    n=0;
    for(p=0;argv[ranking][p]!='-';p++){
      tmp[n++]=argv[ranking][p];
      if (argv[ranking][p]=='\0') help();
    }
    tmp[n]='\0';
    result.ranking_from=atol(tmp);

    n=0;p++;
    for(;argv[ranking][p]!='\0';p++){
      tmp[n++]=argv[ranking][p];
    }
    tmp[n]='\0';
    result.ranking_width=atol(tmp)-result.ranking_from+1;
    if(result.ranking_from<0 || result.ranking_width<=0) help();
  }

  /* list */
  if(listN!=-1) result.list_file=argv[listN];

  return result;
}

int sstrcat(STRING *string, char *a, char *b){
  string->length=strlen(a)+strlen(b);
  string->buffer=string->length+10;
  string->str=(char*)malloc(string->buffer);
  if(string->str==NULL){
    fprintf(stderr, "Memory err occurred on line %d\n", __LINE__);
  }
  sprintf(string->str, "%s%s", a, b);
  return 0;
}

void help(void){
  fprintf(stderr, "\n  **** %s written by Uhmin compiled on %s %s ****\n"
	  ,__FILE__, __DATE__, __TIME__);
  fprintf(stderr, "     This program uses binary search algorithm ");
  fprintf(stderr, "and run firster than firstpic.c\n");
  fprintf(stderr, "     source file name: %s\n",__FILE__);
  fprintf(stderr, "     binary search:    %s\n\n", _INCLUDE_);
  fprintf(stderr, "function: Get a partial sequence from multi fasta sequence with index file.\n");
  fprintf(stderr,"\nusage:\n");
  fprintf(stderr,"      -i: Infile name. stdin if default.\n");
  fprintf(stderr,"      -o: Outfile name. stdout if default.\n");
  fprintf(stderr,"      -k: Keyword of sequence searching.\n");
  fprintf(stderr,"      -r: Region to pic up (from)-(to). All sequence ( 0-0 ) if default.\n");
  fprintf(stderr,"      -w: Letters per line. Display in one line if 0. Zero if default.\n");
  fprintf(stderr,"      -s: extract oligofasta from multifasta. to get from M-th to N-th sequence [-s M-N] (M <= N)\n");
  fprintf(stderr,"          to count number of sequence in amultifasta file use only -i option\n");
  fprintf(stderr,"      -l: File name of query List.\n");
  exit(0);
}

void indexhelp(char *level){
  fprintf(stderr,"Something is wrong with %s file. \n",level);
  fprintf(stderr,"%s file is not found or broken.\n",level);
  if (strcmp(level,"index")==0)
    fprintf(stderr,"Make sure that %s file is made by  \"fpicmakeindex.exe\"\n",level);
  else{
    fprintf(stderr,"Make sure that %s file is made by  \"fpicmakebindex1.exe\"\n",level);
    fprintf(stderr,"To create bindex file type like that:\n");
    fprintf(stderr,"fpicmakebindex1.exe < foo.mfa.index | sort > foo.mfa.bindex\n");
  }
  fprintf(stderr,"Name of %s file is automatically denoted by INFILE.%s\n",level,level);
  fprintf(stderr,"\n");  
  exit(0);
}

