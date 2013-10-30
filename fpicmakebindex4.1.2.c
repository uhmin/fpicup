/*
   fpicupmakebindex.c
   create index data for binary search.

   USAGE:
  fpicupmakebindex.exe < hoo.mfa.index > hoo.mfa.bindex
  */
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLETTER 512
#define NAMELENGTH 50

typedef struct {
  char *name;
  fpos_t fposition;
  int from;
  int to;
} NAMELIST;

NAMELIST *read_all(FILE *fin);
NAMELIST *edit_list(NAMELIST *list, size_t *list_size_p);
NAMELIST seqnameDelimitBySpace(NAMELIST list);
NAMELIST nameWithSpace(NAMELIST *input);
NAMELIST splitNames(long *from, long *to, char *name, fpos_t position);
void sort_and_output(NAMELIST *list, size_t list_size, FILE *fout);
char *fgetl(FILE *fin, int *fileend);
int CompFunc(const NAMELIST *key, const NAMELIST *element);
int interface(int argc, char *argv[], FILE **indexfile, FILE **bindexfile);
void help(void);

int main(int argc, char *argv[]){
  FILE *fin,*fout;
  /*  char seqchar, seqname[MAXLETTER];*/
  /*  char outname[MAXLETTER];*/
  /*  fpos_t position;*/
  /*  int i,sw;*/
  size_t list_size;
  NAMELIST *list, *list2;

  interface(argc, argv, &fin, &fout);

  fprintf(stderr,"Reading index\n");
  list=read_all(fin);
  fclose(fin);

  fprintf(stderr,"Editing index\n");
  list2=edit_list(list, &list_size);

  fprintf(stderr,"Sorting index\n");
  sort_and_output(list2, list_size, fout);
  fclose(fout);
  fprintf(stderr,"fpicupmakebindex finished successfully.\n");
  return 0;
}

NAMELIST *read_all(FILE *fin){
  char *seqname=NULL, *seqname2;
  fpos_t position;
  int fileend=1;
  long list_size=MAXLETTER, i=0, length;
  NAMELIST *list;

  list=(NAMELIST *)malloc(list_size*sizeof(NAMELIST));

  fseeko(fin,0,SEEK_SET);
  fgetpos(fin, &position);
  while((seqname=fgetl(fin,&fileend))==NULL || fileend!=0){
    if (seqname[0]=='>'){
      if(i+2>list_size){
  list_size+=MAXLETTER;
	list=(NAMELIST *)realloc(list,list_size*sizeof(NAMELIST));
      }

      /* delete return and white space */
      for(length=strlen(seqname); length >= 0 && isspace(seqname[length]); length--);
      seqname[length+1]='\0';
      length--;

      /*length=strlen(seqname);*/
      /*fprintf(stderr, "seqname: %s//\n", seqname);*/
      seqname2=(char*)malloc(sizeof(char)*(length+2));
      strcpy(seqname2, seqname+1);
      list[i].name=seqname2;
      list[i].fposition=position;
      list[i].from=1;
      list[i].to=length;
      i++;
      list[i].name=NULL;
      /*if(i>1000) break;*/
    }
    if(seqname != NULL) { 
      free(seqname);
      seqname=NULL;
    }
    fgetpos(fin, &position);
  }
  return list;
}

NAMELIST *edit_list(NAMELIST *list, size_t *list_size_p){
  long list_size=MAXLETTER, i, j;
  NAMELIST *list2, tmp;
  char *name, *periodName;
  long from, to, lengthLimit;
  fpos_t fposition;
  char *periodPosition;

  list2=(NAMELIST *)malloc(list_size*sizeof(NAMELIST));

  /* delimit by space */
  j=0;
  for(i=0;list[i].name!=NULL;i++){
    tmp=seqnameDelimitBySpace(list[i]);
    /*fprintf(stderr, "after delimit by space: [%s]\n", tmp.name);*/
    if(tmp.name[0]!='\0'){
      if(j+10 >= list_size){
	list_size+=MAXLETTER;
	list2=(NAMELIST *)realloc(list2,list_size*sizeof(NAMELIST));
      }
      list2[j]=tmp;
      j++;
      if(isspace(tmp.name[0])){
	list2[j]=nameWithSpace(&tmp);
	j++;
      }

      name=(char *)malloc((strlen(tmp.name)+1)*sizeof(char));
      strcpy(name, tmp.name);
      lengthLimit=strlen(name)-1;
      /*printf("      input: %s, length: %ld\n",name, lengthLimit); */
      for(from=tmp.from-1, to=tmp.from-1, fposition=tmp.fposition; 
	  name!=NULL && from < lengthLimit;
	  j++){
	if(j>=list_size-2){
	  list_size+=MAXLETTER;
	  list2=(NAMELIST *)realloc(list2,list_size*sizeof(NAMELIST));
	}
	/* printf("Input: %s %ld-%ld::%lld\n", name, from, to, fposition); *//*debug */
	tmp=splitNames(&from, &to, name, fposition);
	/* printf("result; %s %ld-%ld::%lld / %ld\n", tmp.name, from, to, fposition, lengthLimit); */ /* debug */
	if(tmp.name!=NULL && tmp.name[0]!='\0'){
	  list2[j]=tmp;
	  /* name+=list2[j].to+1;*/
	}else{
	  break;
	}

	if(strstr(tmp.name, ".")!=NULL){
	  j++;
	  periodName=(char *)malloc((strlen(tmp.name)+1)*sizeof(char));
	  strcpy(periodName, tmp.name);
	  periodPosition=strstr(periodName, ".");
	  *periodPosition='\0';
	  list2[j].name=periodName;
	  list2[j].from     =list2[j-1].from;
	  list2[j].to       =list2[j-1].to-(strlen(tmp.name)-strlen(periodName));
	  list2[j].fposition=list2[j-1].fposition;
	  /* printf("found period: %s %d-%d::%lld\n", list2[j].name, list2[j].from,
	     list2[j].to, list2[j].fposition);*/
	}
      }
    }
  }

  /* add line to line2 */
  for(i=0;list[i].name!=NULL;j++, i++){
    if(j>=list_size){
      list_size+=MAXLETTER;
      list2=(NAMELIST *)realloc(list2,list_size*sizeof(NAMELIST));
    }
    list2[j]=list[i];
  }

  *list_size_p=j;
  return list2;
}

NAMELIST nameWithSpace(NAMELIST *input){
  NAMELIST result;
  char *p;
  for(p=input->name+1; isspace(*p); p++);
  /*fprintf(stderr, "name with space: [%s]\n", p);*/
  result.name     = p;
  result.fposition = input->fposition;
  result.from     = ( p - input->name ) + input->from;
  result.to       = input->to;
  return result;
}

NAMELIST seqnameDelimitBySpace(NAMELIST list){
  int length;
  int k;
  NAMELIST result;
  int sw;
  length=strlen(list.name);
  result.name=(char *)malloc((length+1)*sizeof(char));
  result.name[0]='\0';
  /*
  result.from=0;
  result.to=0;
  */

  for(k=0, sw=0;k<length;k++){
    if(list.name[k]==' ' || list.name[k]=='\t'){
      if(sw==1){
	break;
      }
    }else{
      sw=1;
    }
  }
  if(0<k && k<length){
    result.name=(char *)malloc((length+1)*sizeof(char));
    strcpy(result.name, list.name); result.name[k]='\0';
    result.name=(char *)realloc(result.name,(k+1)*sizeof(char));
    result.fposition = list.fposition;
    result.from      = list.from;
    result.to        = k+1;
    /* fprintf(stdout,    "'%s'\n", list.name  );  *//* debug */  
    /* fprintf(stdout, "-> '%s'\n", result.name);  *//* debug */  
  }
  return result;
}

/* list2[j]=splitNames(&from, &to, name);*/
NAMELIST splitNames(long *from, long *to, char *name, fpos_t position){
  char *newname;
  NAMELIST result;
  long i;

  result.name=NULL;
  /*
  result.from=0;
  result.to=0;
  */
  i=*from;
  newname=name+i;
  /*printf("Input name:::: %s -> %s %ld-%ld\n",name,  newname, *from, *to); *//* debug */
  while(1){
    for(; name[i]!='\0'; i++){
      /*printf("%c, ", name[i]);*/
      if(name[i]=='|' || (name[i]==' ' && name[i+1]!=' ')){
	/*printf("found %c\n", name[i]); *//* debug */
	name[i]='\0';
	break;
      }
    }
    *to=i-1;
    /*printf("samle: %s, from: %ld-%ld\n", newname, *from, *to); *//* debug */
    if(strcmp(newname, ">gi")!=0
       && strcmp(newname, "gi")!=0 
       && strcmp(newname, "ri")!=0
       && strcmp(newname, "ref")!=0
       && strcmp(newname, "emb")!=0
       && strcmp(newname, "gb")!=0
       && strcmp(newname, "dbj")!=0
       && strcmp(newname, "pir")!=0
       && strcmp(newname, "sp")!=0
       && newname[0]!='\0'
       ) break;
    i++;
    newname=name+i;
    /* printf("#samle: %s, from: %ld\n", newname, i);*/
    *from=i;
    if(newname[0]=='\0') {fprintf(stderr, "found null name exit\n"); exit(0);}
  }
  
  /*printf("@name; %s, %ld-%ld\n", newname, *from, *to); *//* debug */
  if(name!=NULL){
    result.name=newname;
    result.from=*from+1;
    result.to=*to+2;
    result.fposition=position;
    /*printf("name: %s. %ld-%ld position: %lld\n", 
      result.name, result.from, result.to, result.fposition); *//* debug */
  }
  *from=*to+2;
  return result;
}

void sort_and_output(NAMELIST *list, size_t list_size, FILE *fout){
  long i;
  unsigned long maxFrom=0, maxTo=0;
  unsigned long long maxPosition=0;
  unsigned int orderFrom, orderTo, orderPosition;
  char tmp[MAXLETTER];
  char format[MAXLETTER];

  qsort(list,(size_t)list_size,sizeof(NAMELIST),
	(int (*)(const void *, const void *))CompFunc);
  
  for(i=0;i<list_size;i++){
    /* printf("%s\n", list[i].name);  *//*debug */
    if(maxFrom     < list[i].from           ) maxFrom     = list[i].from;
    if(maxTo       < list[i].to             ) maxTo       = list[i].to;
    if(maxPosition < list[i].fposition.__pos) maxPosition = list[i].fposition.__pos;
  }

  sprintf(tmp, "%ld",  maxFrom    ); orderFrom=strlen(tmp);
  sprintf(tmp, "%ld",  maxTo      ); orderTo=strlen(tmp);
  sprintf(tmp, "%lld", maxPosition); orderPosition=strlen(tmp);

  /* printf("max     from: %ld %d\n" ,maxFrom,     orderFrom    ); */ /* debug */
  /* printf("max       to: %ld %d\n" ,maxTo,       orderTo      ); */ /* debug */
  /* printf("max position: %lld %d\n",maxPosition, orderPosition); */ /* debug */
  sprintf(format, "%%%dd %%%dd %%%dlld\n", orderFrom, orderTo, orderPosition);
  /* printf("format; %s", format); */ /* debug */

  for(i=0;i<list_size;i++){
    /*    fprintf(stderr,"%s\n",list[i].name);*/
    /* fprintf(fout,"%3d %4d %15lld\n",list[i].from, list[i].to, list[i].fposition); */
    fprintf(fout,format,list[i].from, list[i].to, list[i].fposition);
  }
}


int interface(int argc, char *argv[], FILE **indexfile, FILE **bindexfile){
  int count,index=-1,bindex=-1;
  /*int n,p;*/
  /*char tmp[MAXLETTER];*/

  /* default */
  *indexfile=stdin;
  *bindexfile=stdout;

  /* assign */
  if ((argc % 2)==0) help();
  
  for(count=1;count<argc-1;count+=2)
    switch(argv[count][1]){
    case 'i':
      index=count+1;
      break;
    case 'b':
      bindex=count+1;
      break;
    default:
      help();
      break;
    }

  /* indexfile */
  if (index!=-1) {
    if ((*indexfile=fopen(argv[index],"r"))==NULL){
      fprintf(stderr,"Cannot open infile.[%s]\n",argv[index]);
      exit(0);
    }
    
  }
  
  /* outfile */
  if (bindex!=-1){
    if ((*bindexfile=fopen(argv[bindex],"w"))==NULL){
      fprintf(stderr,"Cannot open outfile.[%s]\n",argv[bindex]);
      exit(0);
    }
  }
  
  return 0;
}

void help(void){
  fprintf(stderr,"%s written by Uhmin\n",__FILE__);
  fprintf(stderr,"  compiled on %s %s\n\n",__DATE__, __TIME__);
  fprintf(stderr,"function: Make binary search index file for binary search program \"fpicup2\" .\n");
  fprintf(stderr,"          from index file made by \"fpicmakeindex1_2.c\".\n");
  fprintf(stderr,"\nusage:\n");
  fprintf(stderr,"      fpicmakebindex1.exe < foo.mfa.index > foo.mfa.bindex \n\n");
  fprintf(stderr,"      Suffix of the bindex file must be *.bindex\n");
  fprintf(stderr,"\n");
  fprintf(stderr,"#### TO MAKE INDEX FILE ####\n");
  fprintf(stderr,"fpicmakeindex.exe (infile)\n");
  fprintf(stderr,"index-file-name will be \"infile.index\"\n");
  fprintf(stderr,"\n");
  exit(0);
}

char *fgetl(FILE *fin, int *fileend){
  size_t length=MAXLETTER, position;
  char c,*inseq;

  /* initialize */
  inseq=(char *)malloc(length*sizeof(char));
  *fileend=1;

  /* read one line */
  c=fgetc(fin);
  for(position=0;c!='\n' && c!='\0' && c!=EOF;position++){
    inseq[position]=c;

    if(position+3>=length){
      length=position+MAXLETTER;
      inseq=(char *)realloc(inseq,length*sizeof(char));
    }
    c=fgetc(fin);
  }

  if(c=='\n') inseq[position++]=c;
  inseq[position++]='\0';
  if(c==EOF) *fileend=0;

  return inseq;
}

int CompFunc(const NAMELIST *key, const NAMELIST *element){
  return strcmp(key->name,element->name);
}
