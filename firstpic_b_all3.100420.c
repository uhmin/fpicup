#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

/* firstpic_b

   Search and picup sequence using binary search,
   so this program require *.bindex file for
    binary index.
   This binary index can be made by fpicmakebindex1.c
   Use like that:
  fpicmakebindex1 < foo.mfa.index | sort > foo.mfa.bindex
   
   Sequence name must exactly match with dataname.
   
   If (from > to) output reverse complement sequence
    to "char *outseq".

   (int step) require the number which is written in
     *.index file.
 */

#define MAXLETTER_FIRSTPIC_B 256

typedef struct{
  char *name;
  char *sequence;
  int status;
  long truncate;
  int namelength;
  int seqlength;
  int namebuffer;
  int seqbuffer;
}RESULT;


/***** important function from here *******/
RESULT firstpic_bi2(FILE *filepointer[], char **seqname, long *from, long *to, int step);
RESULT firstpic_bi(FILE *mainfile, FILE *indexfile, FILE *bindexfile,
                 char **seqname, long *from, long *to, int step);
int initialize_for_fpicup_bi(char *file_name, FILE **gene_file_p, int *gene_step);
/***** important function finish *******/



int compare(char *bindex, char *seqname);
off_t bi_tree_search(FILE *bindexfile, FILE *indexfile, char *seqname);
char *allout(FILE *fin, FILE *findex);
char *extract_index(FILE *fin, FILE *findex, long *from, long *to, int step, RESULT result);
void terminate_msg(FILE *fout, long *from, long *to, RESULT result);
char complement(char a);
char *fgetl(FILE *fin, int *fileend);
off_t get_index_name(FILE *bindexfile, FILE *indexfile, char **data);
int compareTImeStamp(char *oldFile, char *newFile, char *comment);

int initialize_for_fpicup_bi(char *file_name, FILE **gene_file_p, int *gene_step){
  char *index_name, *bindex_name;
  char *point, *line=NULL;
  size_t name_length;


  name_length=strlen(file_name);
  /*index_name=(char *)calloc(name_length+100,sizeof(char));*/
  index_name=(char *)malloc(name_length+10);
  bindex_name=(char *)malloc(name_length+10);
  index_name[0]='\0';
  bindex_name[0]='\0';

  /* pointer to main file */
  if ((gene_file_p[0]=fopen(file_name,"r"))==NULL){
    fprintf(stderr,"Cannot open main file: [%s] (%s compiled on %s %s)\n"
      ,file_name, __FILE__, __DATE__, __TIME__);
    exit(0);
  }

  /* pointer to index file */
  sprintf(index_name,"%s.index",file_name);
  if ((gene_file_p[1]=fopen(index_name,"r"))==NULL){
    fprintf(stderr,"Cannot open index file: [%s] (%s compiled on %s %s)\n"
	    ,index_name, __FILE__, __DATE__, __TIME__);
    exit(0);
  }
  compareTImeStamp(file_name, index_name, "index file");

  /* pointer to bindex file */
  sprintf(bindex_name,"%s.bindex",file_name);
  if ((gene_file_p[2]=fopen(bindex_name,"r"))==NULL){
    fprintf(stderr,"Cannot open bindex file: [%s] (%s compiled on %s %s)\n"
	    ,bindex_name, __FILE__, __DATE__, __TIME__);
    exit(0);
  }
  compareTImeStamp(index_name, bindex_name, "bindex file");

  /* read step */
  /*  bindex_name=(char *)realloc(bindex_name,MAXLETTER_FIRSTPIC_B*sizeof(char));*/
  while(1){
    if(line!=NULL){ free(NULL); line=NULL;}
    if (uhmin_fgets(&line, 0, gene_file_p[1])==NULL) {
      fprintf(stderr,"index file is not adequate\n");
      exit(0);
    }
    point=strstr(line,"step");
    if (point!=NULL) break;
  }
  *gene_step=atoi(point+5);

  if(line!=NULL){
    free(NULL);
    line=NULL;
  }
  if(index_name!=NULL){
    free(index_name);
    index_name=NULL;
  }
  if(bindex_name!=NULL){
    free(bindex_name);
    bindex_name=NULL;
  }
  return 0;
}

RESULT firstpic_bi2(FILE *filepointer[], char **seqname, long *from, long *to, int step){
  return firstpic_bi(filepointer[0],filepointer[1], filepointer[2], seqname, from, to, step);
}

RESULT firstpic_bi(FILE *mainfile, FILE *indexfile, FILE *bindexfile,
                 char **seqname, long *from, long *to, int step){
  static char *preseqname=NULL;
  static size_t namelength=MAXLETTER_FIRSTPIC_B;
  static off_t position;
  char tmp[MAXLETTER_FIRSTPIC_B];
  long seqlength;
  RESULT result;

  /* initialize */
  if(preseqname==NULL){
    /*preseqname=(char *)calloc(namelength,sizeof(char));*/
    preseqname=(char *)malloc(namelength);
    preseqname[0]='\0';
  }
  result.namebuffer=strlen(*seqname)+10;
  result.name=(char *)malloc(result.namebuffer); 
  strcpy(result.name, *seqname); /* search key is *seqname */

  /* search for index position by binary index */
  if(strcmp(preseqname,result.name)!=0){
    preseqname[0]='\0';
    seqlength=strlen(result.name);

    if(seqlength+4>namelength){
      namelength=seqlength+MAXLETTER_FIRSTPIC_B;
      preseqname=(char *)realloc(preseqname,namelength*sizeof(char *));
    }
    strcpy(preseqname,result.name);
    /*fprintf(stderr,"search: [%s]\n",preseqname);*/
    position=bi_tree_search(bindexfile, indexfile, result.name);
  } 
  if (position==-1) {
    result.sequence=NULL;
    return result;
  }

  /* search sequence position from normal index */
  fseeko(indexfile,position,SEEK_SET);
  /* get full sequence name */
  if(result.name!=NULL){free(result.name); result.name=NULL; }
  /*uhmin_fgets(&tmpseqname,MAXLETTER_FIRSTPIC_B-1,indexfile);*/
  uhmin_fgets(&result.name, MAXLETTER_FIRSTPIC_B-1, indexfile);

  /* get sequence */
  result.sequence=extract_index(mainfile, indexfile, from, to, step, result);

  if (*from!=0 && *to!=0){
    /* add region information to seqname */
    seqlength=strlen(result.name);
    chomp(result.name);
    sprintf(tmp," from=%ld, to=%ld\n",*from,*to);
    result.name=(char *)realloc(result.name,(seqlength+strlen(tmp)+3)*sizeof(char));
    strcat(result.name,tmp);
  }

  if(*seqname!=NULL){free(*seqname); *seqname=NULL;}
  *seqname=result.name;

  return result;
}

off_t bi_tree_search(FILE *bindexfile, FILE *indexfile, char *seqname){
  char *tmp=NULL;
  off_t maxposition, minposition=-1, target;
  off_t fposition;
  int sw;
  size_t size;
  /* int no_found_count=0; */

  /* set position */
  fseeko(bindexfile, (off_t)0, SEEK_SET);

  /* measure data size */
  /*  tmp=(char *)malloc(MAXLETTER_FIRSTPIC_B);*/
  uhmin_fgets(&tmp,MAXLETTER_FIRSTPIC_B-1,bindexfile);

  size=strlen(tmp);
  /* measure file length */
  fseeko(bindexfile, (off_t)0, SEEK_END);
  /*  printf("Max File Position: %lld\n", ftello(bindexfile)); */ /* debug */
  maxposition=ftello(bindexfile)/size;

  /* seek sequence */
  target=(minposition+maxposition)/2;
  fseeko(bindexfile,target*(off_t)size,SEEK_SET);

  while(1){
    if(tmp!=NULL){free(tmp); tmp=NULL;}
    fposition=get_index_name(bindexfile,indexfile,&tmp);
    sw=compare(tmp,seqname);
    /* fprintf(stderr,"comp: [%s],  %d \n",tmp, sw); */ /* debug */
    if (sw==0) break;
    else if (sw<0) maxposition=target;
    else minposition=target;
    if (maxposition-minposition==1) break;
    
    target=(maxposition+minposition)/2  ;
    /*printf("current %ld, jump %ld\n",ftello(bindexfile),target*size-ftello(bindexfile));*/
    if (fseeko(bindexfile,target*(off_t)size,SEEK_SET)!=0) break;
  }

  if(tmp!=NULL){free(tmp); tmp=NULL;} 
  if (sw==0) {
    /*fprintf(stderr,"successfully find sequence \n%s\n",tmp);*/
  }else {
    fprintf(stderr,"could not find the sequence %s.\n",seqname);
    return -1;
  }
  return fposition;
}

int compare(char *bindex, char *seqname){
  int cmp;
  /*  fprintf(stderr,"comp:\n[%s]\n[%s]\n",seqname,bindex);*/
  cmp=strcmp(seqname,bindex);

  /*  if(cmp==0) fprintf(stderr,"FOUND\n\n");*/
  return cmp;
}

char *allout(FILE *fin, FILE *findex){
  char *outseq;
  char *m=NULL,inchar;
  long n=0;
  size_t length_firstpic=MAXLETTER_FIRSTPIC_B;

  /* initialize */
  /*outseq=(char *)calloc(length_firstpic,sizeof(char));*/
  outseq=(char *)malloc(length_firstpic);
  outseq[0]='\0';
  
  /* set position */
  if(m!=NULL){ free(m); m=NULL; }
  uhmin_fgets(&m,MAXLETTER_FIRSTPIC_B-1,findex);
  fseeko(fin,atoll(m),SEEK_SET);
  if(m!=NULL){ free(m); m=NULL; }

  inchar=fgetc(fin);
  while(inchar!=EOF && inchar!='>'){
    if (n+4>=length_firstpic) {
      length_firstpic=n+MAXLETTER_FIRSTPIC_B;
      outseq=(char *)realloc(outseq,length_firstpic*sizeof(char));
    }
    if (inchar>=' ' && inchar<='~') { 
       /* not (a-z || A-Z) because it have to recognize 
	  lessters such as '*' */
      outseq[n++]=inchar;
      outseq[n]='\0';
    }
    
    inchar=fgetc(fin);
    if (inchar=='>') break;
  }
  
  return outseq;
}


char *extract_index(FILE *fin, FILE *findex, long *from, long *to, int step, RESULT result){
  char x, *m=NULL, c;
  long t, n, jump, detail, swap, i, j;
  int reverse=0;

  m=(char *)malloc(MAXLETTER_FIRSTPIC_B);
  result.truncate=0;

  /* read all sequence */
  if(*from==0 && *to==0) {
    /*
      return allout(fin,findex);
    */
    result.sequence=allout(fin,findex);
    return result.sequence;
  }

  /* reverse? */
  if (*from>*to) {
    swap=*from;
    *from=*to;
    *to=swap;
    reverse=1;
  }

  /* initialize */
  /*length_firstpic=*to-*from+10;*/
  result.seqbuffer=*to-*from+10;
  /*outseq=(char *)calloc(length_firstpic,sizeof(char *));*/
  result.sequence=(char *)malloc(result.seqbuffer);
  result.sequence[0]='\0';

  /* search jumping position */
  /* fprintf(stderr, "search start. %ld-%ld\n", *from, *to); */
  jump=*from/step;
  for (t=-1;t<jump;t++){
    if(m!=NULL){ free(m); m=NULL;}
    uhmin_fgets(&m,MAXLETTER_FIRSTPIC_B-1,findex);
    if(m[0]=='>' || feof(findex)){
      /* fprintf(stderr, "detected termination! %ld on line %d\n", (t+1)*step, __LINE__); */
      result.truncate=(t+1)*step;
      result.sequence[0]='\0';
      break;
    }
  }
  fseeko(fin,atoll(m)-1,SEEK_SET);

  /* detail position */
  if(result.truncate==0){
    detail=*from-step*jump;
  }else{
    detail=0;
  }
  
  for(t=0;t<detail;){
    x=fgetc(fin);
    if(x=='>'){
      result.truncate=step*jump+t;
      /* fprintf(stderr, "detected termination! at %ld on line %d\n"
	 , result.truncate, __LINE__); */
      result.sequence[0]='\0';
      break;
    }
    if (x>=' ' && x<='~') { 
       /* not (a-z || A-Z) because it have to recognize 
	  lessters such as '*' */
      t++;
    }
  }

  for(n=0; n<=*to-*from && result.truncate==0;){
    x=fgetc(fin);
    if (x=='>' || x==EOF) {
      result.truncate=*from+n-1;
      /*terminate=n-1;*/
      break;
    }
    if (x>=' ' && x<='~') {
       /* not (a-z || A-Z) because it have to recognize 
	  lessters such as '*' */
      if (n+4>result.seqbuffer) {
        result.seqbuffer=n+MAXLETTER_FIRSTPIC_B;
        result.sequence=(char *)realloc(result.sequence, result.seqbuffer*sizeof(char));
      }
      result.sequence[n++]=x;
      result.sequence[n]='\0';
    }    
  }

  /* if reverse make complement */
  if (reverse==1) {
    /* make complement seq */
    for(i=0; result.sequence[i]!='\0'; i++){
      c=complement(result.sequence[i]);
      result.sequence[i]=c;
    }
    i--;
    /* reverse sequence */
    for(j=0;i>j*2;j++){
      swap=result.sequence[j];
      result.sequence[j]=result.sequence[i-j];
      result.sequence[i-j]=swap;
    }
    swap=*from;
    *from=*to;
    *to=swap;
  }
  /* fprintf(stderr, "terminate=%ld\n", result.truncate); */
  terminate_msg(stderr,from,to, result);
  if(m!=NULL){ free(m); m=NULL;}
  return result.sequence;
}

void terminate_msg(FILE *fout, long *from, long *to, RESULT result){
  long prefrom=*from, preto=*to;
  
  if(result.truncate!=0){
    fprintf(fout,"TERMINATED %s has only %ld bp. ", result.name, result.truncate);
    
    if(*from < *to){
      *to = result.truncate;
      if(*from > result.truncate){
	*from = result.truncate;
      }
    }else{
      *from=result.truncate;
      if(*to > result.truncate){
	*to = result.truncate;
      }
    }
    fprintf(fout,"(request %ld-%ld, output %ld-%ld)\n",prefrom,preto,*from,*to);
  }
  return;
}

char complement(char a){
  a=toupper(a);
  
  if (a=='A') return ('T');
  if (a=='T') return ('A');
  if (a=='G') return ('C');
  if (a=='C') return ('G');

  if (a=='B') return ('V');
  if (a=='V') return ('B');
  if (a=='D') return ('H');
  if (a=='H') return ('D');
  if (a=='K') return ('M');
  if (a=='M') return ('K');
  if (a=='S') return ('S');
  if (a=='W') return ('W');
  
  return a;
}

off_t get_index_name(FILE *bindexfile, FILE *indexfile, char **data){
  char *line=NULL, *tmp, *position_for_free=NULL;
  char *newdata=NULL;
  off_t fp;
  long from, to, i, j;

  if(position_for_free!=NULL){free(position_for_free); position_for_free=NULL;}
  uhmin_fgets(&line,MAXLETTER_FIRSTPIC_B-1,bindexfile);
  position_for_free=line;
  /* printf("%lld line: '%s'\n",ftello(bindexfile), line); */ /* debug */
  tmp=strtok(line," ");  from=atol(tmp);
  tmp=strtok(NULL," ");   to =atol(tmp);
  tmp=strtok(NULL," ");   fp =atoll(tmp);

  /*  printf("from: %d to: %d fp: %d\n",from, to, fp);*/
  fseeko(indexfile,fp,SEEK_SET);
  uhmin_fgets(&tmp,0,indexfile);
  newdata=(char *)malloc((to-from+10)*sizeof(char));
  /*  fprintf(stderr,"test::: %s", tmp);*/
  /*  tmp=fgetl(indexfile,&fileend);*/
  /*  for(i=from,j=0; i<to && i<MAXLETTER_FIRSTPIC_B;i++,j++){*/
  for(i=from,j=0; i<to;i++,j++){
    newdata[j]=tmp[i];
  }
  newdata[j]='\0';
  *data=newdata;
  if(tmp!=NULL){free(tmp); tmp=NULL;}
  if(position_for_free!=NULL){free(position_for_free); position_for_free=NULL;}
  /*  if(line!=NULL){free(line);}*/
  return fp;
}


char *fgetl(FILE *fin, int *fileend){
  size_t length=MAXLETTER_FIRSTPIC_B, position;
  char c,*inseq;

  /* initialize */
  inseq=(char *)malloc(length*sizeof(char));
  *fileend=1;

  /* read one line */
  c=fgetc(fin);
  for(position=0;c!='\n' && c!='\0' && c!=EOF;position++){
    inseq[position]=c;

    if(position+4>=length){
      length=position+MAXLETTER_FIRSTPIC_B;
      inseq=(char *)realloc(inseq,length*sizeof(char));
    }
    c=fgetc(fin);
  }

  if(c=='\n') inseq[position++]=c;
  inseq[position++]='\0';
  if(c==EOF) *fileend=0;

  return inseq;
}

int compareTImeStamp(char *oldFile, char *newFile, char *comment){
  struct stat sb1, sb2;

  if (stat(oldFile, &sb1) == -1) {
    fprintf(stderr, "Could not get file information on %s. (%s in line %d)\n"
	    , oldFile, __FILE__, __LINE__);
    exit(EXIT_SUCCESS);
  }
  if (stat(newFile, &sb2) == -1) {
    fprintf(stderr, "Could not get file information on %s. (%s in line %d)\n"
	    , newFile, __FILE__, __LINE__);
    exit(EXIT_SUCCESS);
  }
  /*
    fprintf(stderr, "fasta: %ld,  %s: %ld\n"
	  ,sb1.st_mtime, comment, sb2.st_mtime);
  */
  if(sb1.st_mtime>sb2.st_mtime){
    fprintf(stderr, "Err message from %s\n", __FILE__);
    fprintf(stderr, "%s file (%s) is older than the fasta file (%s).\n"
	    ,comment, newFile, oldFile);
    fprintf(stderr, "%s file must be newer than the fasta file.\n", comment);
    exit(0);
  }
  return 1;
}
