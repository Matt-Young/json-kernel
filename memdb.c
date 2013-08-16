string.h>

#include 
#include 
#include "./include/mach.h"
#include "./include/g_types.h"
#include "./include/cursors.h"

#define DBP ((Element *) obj->variables[0])
char MemoryStrings[STRSIZE ];
char * MemoryStrptr;
// This is a null data base
#undef ATTRIBUTES
#define ATTRIBUTES (EV_Position | EV_Matched | EV_Matcheable)
int match_operator(Mach *obj,int opid,Element * data) {return '*';}
const Element matchElement={'*',1,{1,"*"}};
const RowSequence matchRdx={0,1,0}; 
Mach matchMach;
Cursor matchCursor;
JoinSet matchJoin= {&matchMach.cursors,&matchMach.cursors,&matchMach.cursors};
int matchExec(Mach * obj,int method, Element * arg) {
  matchMach.cursors=&matchCursor;
  if(arg) {
    if(method== Append) 
      elementPrint(arg);
    else 
      *arg = matchElement;
  }
  return 0;
}

int initmatchMach(){
  matchMach.attributes = ATTRIBUTES;
  new_mach_cursor(&matchMach);
  matchCursor.rdx=matchRdx;
  matchCursor.mach=&matchMach;
  matchMach.key.bytes = matchMach.key.buff;
  matchMach.cursors=&matchCursor;
  matchMach.exec = matchExec;
  return 0;
}

#define ORDX obj->cursors->rdx
#define memcpy( to,from,n){while(n--) to[n-1]=from[n=1];}

int memUpdate(Mach *obj,Element * e){ 
  DBP[ORDX.offset].pointer= ORDX.row;
  return EV_Done;
}
#define  keyCopy( to, from) \
  for(to.len=0;to.len < from.len;to.len++) to.bytes[to.len]=from.bytes[to.len];

int memAppend(Mach *obj,Element * from) {
  Element * to = &DBP[ORDX.row+ORDX.offset];
  if(!(obj->attributes &  EV_Result)) {
    printf("Syntax Error");
    return EV_Done;
  }
  to->key.bytes=MemoryStrptr;
  keyCopy(to->key,from->key);
  MemoryStrptr+=to->key.len;
  ORDX.row++;
  return EV_Done;
}
int memFetch(Mach *mach,Element * e) {
  if(mach->cursors->rdx.row >= (int) mach->variables[1])
    return EV_Done;
  *e = ((Element *) mach->variables[0])[mach->cursors->rdx.row+mach->cursors->rdx.offset];
  return EV_Data;
}
int memReset(Mach *obj,Element * e) {
  ORDX.row=0;
  ORDX.total=DBP[ORDX.row+ORDX.offset].pointer;
  return EV_Done;
}
int memStep(Mach *obj,Element * e) {
  ORDX.row++;
  return EV_Done;
}

int memStepFetch(Mach *obj,Element * e) {
  ORDX.row++;
  *e = DBP[ORDX.row+ORDX.offset];
  return EV_Done;
}
int memFetchRow(Mach *obj,Element *e) {
  int row;
  row = atoi(e->key.bytes+1);
  *e = DBP[row+ORDX.offset];
  return EV_Data;
}

int memDefault(Mach *obj,Element * e) {
  return EV_Done;
}
typedef int (*Handler)(Mach *,Element *);
Handler mem_methods[None-Select];
Handler flat_methods[None-Select];
#undef INSERT
#define INSERT(M,H) mem_methods[M]=H;flat_methods[M]=H;
#undef FLAT
#define FLAT(M,H) flat_methods[M]=H;
int mem_exec(Mach *mach,int method,Element * data) {
 return(method < None ? mem_methods[method-Select](mach,data) : memDefault(mach,data) );
}
int flat_exec(Mach *mach,int method,Element * data) {
   return(method < None ? flat_methods[method-Select](mach,data) : memDefault(mach,data) );
}
Mach *  MemInit(Mach * obj) {
  int size=0;
  Key *k = (Key *) (&obj->key);
    if(obj->attributes & EV_Flat){
    obj->exec =flat_exec;
    return 0;
    }
   obj->exec =mem_exec;
  if(!obj->attributes) 
    obj->attributes = LG->attributes;
  new_mach_cursor(obj);
  if(!obj->variables[0]) {  // caller supplied no memory
    size= (k->bytes && k->len && k->bytes[0]=='$')  ?
      atoi(obj->key.bytes+1) : ELSIZE;
      obj->variables[0] = (void *) malloc(size * sizeof(Element));
      obj->variables[1]= (void *) size;
  }
  if(!obj->variables[1])
    obj->variables[1]= (void *) size;
  obj->cursors->rdx.offset=0;
  return obj;
} 
void  MemDel(Mach * obj) {
  free(obj->variables[0]);
}

int memClose(Mach * obj,Element *t) {
  return EV_Done;
}
// this method steps through a flat file of the form:
// link1,pointer1,key1.len,key1.bytes,bytes[key1.len],link2,pointer2,key2....
int flatStep(Mach * mach,Element *e) {
  Element *cur;
  cur = (Element *) (int *) mach->variables[0];
  mach->variables[1] = cur + (cur->key.len/sizeof(int));
  mach->cursors->rdx.row++;
  return EV_Done;
}
int flatStepFetch(Mach * mach,Element *e) {
  flatStep(mach,e);
  return memFetch(mach,e);
}
int flatAppend(Mach * mach,Element *e) {
  Element *cur;int * I;
  I = (int *) mach->variables[0];
  cur = (Element *) I;
  cur->key.bytes = (char *)  &I[4];
  memcpy(cur->key.bytes,e->key.bytes,cur->key.len);
  mach->variables[1] = &I[(cur->key.len+3)/sizeof(int)];
  mach->cursors->rdx.row++;
  return EV_Done;
}

int initMemMach(){
  MemoryStrptr=MemoryStrings;
  initmatchMach();
  INSERT(Select,memReset);
  INSERT(Reset,memReset);
  INSERT(Append,memAppend);
  FLAT(Append,flatAppend);
  INSERT(Update,memUpdate);
  INSERT(Fetch,memFetch);
  INSERT(Step,memStep);
  FLAT(Step,flatStep);
  INSERT(FetchRow,memFetchRow);
  INSERT(StepFetch,memStepFetch);
  FLAT(StepFetch,flatStepFetch);
  machStart(&matchMach);
  return 0;
}

