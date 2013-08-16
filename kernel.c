#include "./include/config.h"
#include 
#include
#include
#include "./include/mach.h"
#include "./include/g_types.h"
#include "./include/names.h"
#include "./include/cursors.h"
#include "./include/items.h"

#ifndef DBG_ENGINE
#endif
// Will be shared Protected
OP operators[128];

JoinSet join;
// Using  cursor list grammar, a join becomes 
//  two program counters marching sequentially over two 
// different graphs. in a descent
// the handle argument will manipulate the
// cursor stack when skipping elements in a set.
// handle is expected to recursively call join
int left_join() {
  do {
    while(RC->rdx.row < RC->rdx.total) {
      LG->exec(LG,Fetch,&join.N);
      //Open a new expression tree
      if(join.N.pointer >  1) {
        new_cursor(join.left);  // a push
        LC->rdx.total=join.N.pointer;
      }
      operators[join.N.link & EV_Opcode].handler(); }
  }  while(LC->rdx.row < LC->rdx.total);
  if( LC->rdx.total > 1)
      delete_cursor(join.left);  
  return EV_Done;
}
int joinSwap() {
  PCursor *c;
  c = join.left; join.left= join.right; join.right = c; // parenthesis mean right join
  return 0;
}
int keycmp(Key k1,Key k2) {
  while(k1.len && k2.len && (k1.bytes[k1.len] == k2.bytes[k2.len]))
  { k1.len--;k2.len--;}
  if(k1.len || k2.len)
    return 0;
  return 1;
}
//
//  This is the one 'Arithmetic' method of the 'ALU' 
// Key out = match_pass_collect(Key left Key right
#define PROPERTY(a) operators[a.link & EV_Opcode].properties
int DotHandler(){
  Element left,right;
  RG->exec(RG,Fetch,&right);
  LG->exec(LG,Fetch,&left);
  // Operator precadence.
  if((right.pointer > 1) || (PROPERTY(right) & EV_Left))
    return joinSwap();
  if( (PROPERTY(right) & EV_Matched)) {
    (*join.left)->rdx.row++;
    // if(!(RSLG->attributes & EV_Not))
    RSLG->exec(RSLG,Append,&left);
    if(!(PROPERTY(right) & EV_NoInc)  || keycmp(left.key,right.key)) {
      (*join.right)->rdx.row++;
    }
  } else if( !(PROPERTY(left) & EV_Not) || left.link == '?') {
    (*join.right)->rdx.row++;
     // if(!(RSLG->attributes & EV_Not))
    RSLG->exec(RSLG,Append,&right);
    if(!(PROPERTY(left) & EV_NoInc)  ||  keycmp(left.key,right.key)) {
      (*join.left)->rdx.row++;
    }
  } else if( PROPERTY(left) & PROPERTY(right) & EV_Matcheable ) {
      (*join.right)->rdx.row++;
      (*join.left)->rdx.row++;
      if( // if(!(RSLG->attributes & EV_Not)) &&
        keycmp(left.key,right.key))
        RSLG->exec(RSLG,Append,&right);
      else {
        // skipping to the next left element
         delete_cursor(join.left); // this sequence fails
         LG->exec(LG,Fetch,&right);
         (*join.left)->rdx.row = right.pointer; 
         if((*join.left)->rdx.row < (*join.left)->rdx.total)
            (*join.right)->rdx.row=0;  // and restart the right
         else if((*join.left)->rdx.row > (*join.left)->rdx.total)
           printf("Sytax error\n");
      }
    } else  if(left.key.bytes[0] == '$') {// by relative row number of rightside arguments
      RG->exec(RG,FetchRow,&right); 
      // if(!(RSLG->attributes & EV_Not))
      RSLG->exec(RSLG,Append,&right);
      (*join.left)->rdx.row++;
      (*join.right)->rdx.row++;
    }  else return joinSwap(); // Dunno
    return EV_Done;
  }
  void MakeName(NameKey *namekey,Key *key) {
    namekey->len=  key->len > MAXNAME ? MAXNAME-1  : key->len; 
    strncpy(namekey->buff,key->bytes,namekey->len);
    namekey->bytes=  namekey->buff;namekey->buff[namekey->len]=0;
  }
  // sql:name@parse:,console:
  int ColonHandler() {
    Element n; Symbol *s;
    LG->exec(LG,StepFetch,&n);  // fetch the key of the next element
    //if(!key_compare())
    s=find_key(n.key); //look it up
    if(s && s->type == G_TYPE_HANDLER)
      ((Handler) s->value)();
    else if(s && s->type == G_TYPE_GRAPH) {  
      Mach g;
      LG->exec(LG,StepFetch,&n);  // fetch anyname
      MakeName(&g.key,&n.key);
      ((MachInit) s->value)(&g);
      return EV_Done;
    }
    else if(s && s->type == G_TYPE_OPID) 
      operators[(int) s->value].handler();
    LC->rdx.row=LC->rdx.total; //Clear
    return EV_Done;
  }
  int UnderHandler(){
    if(RG->attributes & EV_Matcheable){
      (*join.left)->rdx.row = (*join.left)->rdx.total;
      return 0;
    }
    else return joinSwap();
  }
  //
  int CommaHandler(){
    return DotHandler();
  }
   int NotHandler(){
     LG->attributes |= EV_Not;
    if(join.N.pointer == 1){
       DotHandler();
       LG->attributes &= ~EV_Not;
  } else 
    LC->rdx.row++;
  return 0;
   }

  int AsteriskHandler(){
    if(RG->attributes & EV_Matched){
      DotHandler();
    }  else{
      joinSwap();
      RG->attributes |= EV_Matched;
    }
    return 0;
  }
  int ParenthHandler(){
    joinSwap();
    return ColonHandler();
  }
  // Three At operators, one is visible
  int PreambleHandler(){
    LG->attributes |= EV_Preamble;
    return DotHandler();
  }
  // restart the join and place the result on the left
  int AtHandler(){
    //  set up right cursor
    Element N;
    Mach mach;
    Mach mem={Mach_Mem};
    Glist g=join.result;
    LG->attributes &= ~EV_Result;  // Clear any preamble condition
    mach = *(*join.left)->mach;
    mach.attributes &= ~EV_Left;
    mach.attributes |= EV_Right;
    mach.cursors= new_mach_cursor(&mach);
    LG->exec(LG,StepFetch,&N);
    // making the split
    mach.cursors->rdx.offset = (*join.left)->rdx.offset+N.pointer+1;
    mach.cursors->rdx.total = (*join.left)->rdx.total-N.pointer-1;
    (*join.left)->rdx.row++;
    machStart(&mach);
    mem.attributes |= EV_Result;
    machInit(&mem);
    machStart(&mem);
    left_join();
    machEnd(&mem);
    mem.attributes &= ~EV_Result;
    mem.attributes |= EV_Left;
    machStart(&mem);
    return 0;
  }
  enum { SystemNull,SystemCall,SystemConfig,SystemJson,SystemFile,SystemNet,SystemParse};
  int console_handler();
  //#define KEY(a) {strlen("a"),"a"}
#define KEY(a) {sizeof(#a)-1,0,#a}
#define OPCODE(a,h)   {a,KEY(a),h,0, EV_Installed}
#define UGLYCODE(a,h,p) {a,{1,0,a,0},h,0,p | EV_Ugly | EV_Matcheable}
  int opcode_initialize(OP *map);
  // install key at installed operand position pointer
  int  config_handler() {
    int opid;Element var;
    OP op=OPCODE(SystemNull,UnderHandler);
    Key name = {sizeof("Installed"),"installed"};
    MakeName(&op.key,&name); 
    LG->exec(LG,StepFetch,&var);
    opid = atoi(var.key.bytes);  // key value is opid
    op.opid=opid;
    if(SystemNone > opid ) 
      return(EV_Error);
    LG->exec(LG,StepFetch,&var);
    LG->exec(LG,Eval,&var);
    op.stmt=var.key.bytes;
    opcode_initialize(&op);
    LC->rdx.row=LC->rdx.total;
    return 0;
  }
  OP pre_installed[] = {
    OPCODE(SystemNull,UnderHandler),
    OPCODE(SystemConfig,config_handler),
    UGLYCODE('*',AsteriskHandler,EV_Matched|EV_NoInc),
    UGLYCODE('(',ParenthHandler,EV_Right),
    UGLYCODE(':',ColonHandler,EV_Left),
    UGLYCODE('_',UnderHandler,0),
    UGLYCODE(',',CommaHandler,0),
    UGLYCODE('.',DotHandler,0),
    UGLYCODE('!',NotHandler,0),
    UGLYCODE('@',AtHandler,EV_Left),
    UGLYCODE('?',UnderHandler,EV_Matched),
    OPCODE(128,0)
  };
  int opcode_initialize(OP *map) {
    Symbol s;
    if( map->opid > 127) 
      return EV_Error;
    if(operators[map->opid].properties) //not empty? 
      return EV_Error;
    map->key.bytes=map->key.buff;
    map->key.len = 
      strlen(map->key.bytes); //Key integrety
    if(map->key.len >20)
      return EV_Error;
    operators[map->opid]= *map;

    //if(!(operators[map->opid].properties & EV_Ugly)) {
    s.key=  *((Key *) &operators[map->opid].key);
    s.type = G_TYPE_OPID;
    s.value = (void *) map->handler;
    new_symbol(&s);
    // }
    return EV_Done;
  }
int handlerName(char * name,void * handler) {
Key k;
k.len=strlen(name);k.bytes = name;
add_key( k,G_TYPE_HANDLER, handler);
return 0;
}
  int init_handlers() {
    int i;
    for(i=0;pre_installed[i].opid!=128;i++)
      opcode_initialize(&pre_installed[i]);
    machInitInstalled();
    return EV_Done;
  }

