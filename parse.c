#include 
#include 
#include 
#include 
#include "./include/mach.h"
#include "./include/g_types.h"
#include "./include/cursors.h"
#include "./include/names.h"
#define DISCARD 0xff
const char  *uglies = "\"._,{}$!:@()";
int isin(char c,const char * str);// {while(*str && (*str) != c) str++; return *str;}
#define isugly(a) isin(a,uglies)

// Front end key word from text and json operators
#define trim(tmp) while((tmp != (*key)) && isspace(*(tmp-1))) tmp--;
#define white(v)while(isspace(*v)) v++;
int keyop(char * *Json,char **key,int *link) {
  char * ptr = *Json;
  char * end;
  int i=0;
  int events=0;
  *key = "_"; // Initial condition
  *link = '_';
  // Key
  white(ptr);
  if(*ptr == 0)
    return -1;
  end = ptr;
  if(isdigit(*ptr) ) {
    int one=0;
    while(isdigit(*ptr) || *ptr == '.') {
      if(*ptr == '.') one++;
      if(one > 1) return -1;
      ptr++; 
    }
    end = ptr;
  }
  else if(*ptr == '"') {// Quote char?
    ptr++;  *key=ptr;
    while(*ptr  != '"')  ptr++;
    end = ptr;  ptr++;    
  } else {
    *key=ptr;
    while(!isugly(*ptr) && !isspace(*ptr) && (*ptr != 0) ) ptr++;
    end = ptr;
  }
  i =   (int) end;
  i -= (int) (*key) ; // char count

  if(i==0)
    *key = "_";  // valid null key

  white(ptr);
  if((*ptr == 0) || !isugly(*ptr) ){ // End of string
    *link = '_';
    return -1;
  }
  // next operator
  *link = *ptr;
  *end = 0;
  ptr++;
  *Json = ptr;
  return i;
}

int json_rules(PCursor *inner);
// builds a submach on inner from user text
Element current,next;
#define LIST &obj->parent->cursors;
int parseAppend(Mach *obj,Element *e) {
  int nchars;
  char *Json;
  Json=e->key.bytes;
  nchars=0;
  // points to first child
  printf("\nparse start %s\n",Json);
  while(nchars >= 0) {
    nchars = keyop(&Json,&next.key.bytes,&next.link);
    next.pointer=1;
    json_rules(&obj->cursors);
  }  
  // finish up
  while(obj->cursors) 
    if(count_cursor(obj->cursors))
      close_update_cursor(&obj->cursors);
    else delete_cursor(&obj->cursors);

    printf("\nparse done\n");
    return(EV_Ok);
}
//  The Json parser chews through the text left to right as
// sequences of name/ops with the free pointer to be assigned
//

int json_rules(PCursor *inner) {
  // Some combinations can be ignored
  printf("%c %d %s\n",next.link,next.pointer,next.key.bytes);
  // Using $ to mean local relative pointer
  // forms like $ $1 $4 are special names
  if(next.key.bytes[0] == '$') {
    next.pointer = atoi(next.key.bytes+1);
    next.link = '$';
  }

  if(current.link == '{' ) { // value{ illegal
       new_cursor(inner);  
 }
  else
  // One append through this series
  if( current.link  == ':') {
    new_cursor(inner);
    append_cursor(inner,Â¤t);}
  else if(current.link == ',') {
    append_cursor(inner,Â¤t);
    close_update_cursor(inner);
    new_cursor(inner);
   } 
  else if(current.link == '.' ){
    if(next.link != '{')  
      append_cursor(inner,Â¤t);
    else {
      new_cursor(inner);
      (*inner)->rdx.offset--; // this is a prepend
    }
  }
  else if (current.link == '$')
    append_cursor(inner,Â¤t);
  else if (current.link == '('){
    new_cursor(inner);
    append_cursor(inner,Â¤t);
    
  }
  else if ((current.link == '}') || (current.link == ')')) {
    append_cursor(inner,Â¤t);
    close_update_cursor(inner);
  }
  current = next; 
  return 0;
}

// Json and parse are each result graphs
#undef ATTRIBUTES
#define ATTRIBUTES (EV_Result)
// fill a flat memory buffer with json
#define RDX(a) a->cursors->rdx
int jsonAppend(Mach * obj, Element *e) {
  Key *out = (Key *) obj->variables;
   if(e->pointer > 1) {
     new_cursor(join.result);
     LC->rdx.total=e->pointer;
    out->bytes[out->len]='{';out->len++;
  }
   (*join.result)->rdx.row++;
  sprintf(out->bytes+out->len,"%4s%4s%4s",e->link,e->pointer,e->key.len);
  out->len+= 12;
  strncpy(out->bytes+out->len,e->key.bytes,e->key.len); 
  if(LC->rdx.row == LC->rdx.total) {
    delete_cursor(join.result);
    out->len +=e->key.len;
    out->bytes[out->len]='}';out->len++;
    }
  return EV_Done;
}

int execParse(Mach *obj,int method,Element *t){
  if(method== Append)
    return parseAppend(obj,t);
  else
    return EV_Done;
}
Mach * ParseInit(Mach *mach){
    mach->attributes =  ATTRIBUTES;
    new_mach_cursor(mach);
    mach->exec=execParse;
  return mach;}

int execJson(Mach *obj,int method,Element *t){
  if(method== Append)
    return jsonAppend(obj,t);
  else
    return EV_Done;
}
static Key JsonKey;
Mach * JsonInit(Mach *mach){
 mach->attributes =  ATTRIBUTES;
   new_mach_cursor(mach);
   mach->cursors->rdx.total=1;
  mach->variables[0] = (void *) &JsonKey;
  JsonKey.bytes = (char *) malloc(8000);
  mach->variables[0] = (void *) (*join.result)->mach;
  JsonKey.len=0;
  mach->exec=execJson;

  return mach;
}
Symbol json_names[] = { 
   {{4,"json"},G_TYPE_GRAPH,JsonInit},
    {{5,"parse"},G_TYPE_GRAPH,ParseInit},
   {0}
};
int initJsonMach(){
add_symbols(json_names);
return EV_Done;
}

