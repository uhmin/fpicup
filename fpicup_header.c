/* fpicup2.c */
#define TMAXLETTER 256

int chomp(char *line){
  int i, result=0;
  for(i=0; line[i]!='\0'; i++){
    if(line[i]=='\n' || line[i]=='\r'){
      line[i]='\0';
      result=1;
      break;
    }
  }
  return result;
}

char *uhmin_fgets(char **outchar, int n, FILE *istream){
  char *result, *line, c;
  size_t length=TMAXLETTER;
  size_t i;

  if((line=(char *)malloc((size_t)TMAXLETTER))==NULL){
    fprintf(stderr, "Memory err on %s on line %d\n", __FILE__, __LINE__);
    exit(0);
  }

  c=fgetc(istream);
  for(i=0; c!=EOF && c!='\n' && c!='\0'; i++){
    if(i+5>length){
      length+=TMAXLETTER;
      if((line=(char *)realloc(line, length*sizeof(char)))==NULL){
  fprintf(stderr, "Memory Err\n");
	exit(0);
      }
    }
    line[i]=c;
    c=fgetc(istream);
  }

  if(c=='\n'){ line[i++]=c; }
  line[i++]='\0';
  if(c==EOF){
    result=NULL;
  }else{
    result=line;
  }
  *outchar=line;
  return result;
}
