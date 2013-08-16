//
//
// The official interface between Json grammar and undelying data
// base engine.   No other header files needed for c based embedded database
//
// Introduction:
// Nested or serialized formats are really expression trees, this model treats
// data as nested expression machs.
//
// The atom is the Element, a node on a mach obeying nested format grammar.
// Every Element is the first in a sequence of one or more Elements, and the 
// sequence obey's nested format grammar.  A Mach is simly a cursor set defined 
// over a sequence of Elements.  The Element contains a Key, which is the 
// unit of informaton, and the Elements points to the next Element in a set 
// with the Total field.  The Element comes with a link field containing an operator
// ID, exactly similar to an expresson tree within a compiler.
//
// **************************
// The nested store record format. 
//
// ****   Key: The actual data bytes ****
//  Passing the actual key bytes, the form is |int size|char bytes[size]|
// Implementations will implement a get_key/put_key private function 
// and accept or return the proper form
typedef int (*Handler)();
#define MAXNAME 40
typedef struct {int len; char * bytes;} Key;
typedef struct {int len; char * bytes; char buff[MAXNAME];} NameKey;

//typedef union {Key key;FlatKey flat;} NameKey;
//****  Element: The format of a mach node *****
// The actual mach node, as passed between layers
// actual internal form obviously up to the data base engine
typedef struct {
  int link;  // contains at least one byte from a Json parse
  int pointer;  // size of enclosed object, in element count
  Key key;   
} Element;
void elementPrint(Element *t);
// Link space. 128 available values can fill the link integer in a node
//
//  Sixteen are reserved for local system
#define SystemNone 15

// ****  A RowSequence: Defines the actual valid row pointers
//  The db can count out elements using the row sequence in a mach
typedef struct {
  int row;  //  current relative row
  int total; // total elements n the mach
  int offset; // offset to obtain absolute indexed row
} RowSequence;

// Machs contexts are nested cursors.  The row sequence is
// the nested cursor struct.
typedef struct Mach Mach;
typedef struct Cursor Cursor;
typedef struct Cursor  {
  RowSequence rdx;
  Cursor * parent;
  Mach * mach;  // Points back to the controlling object
} Cursor;
typedef Cursor *PCursor;
typedef PCursor *Glist;
// This is the structure that guides the general
// left_join() uses this
typedef struct JoinSet JoinSet;

typedef struct  JoinSet {
  Glist left;
  Glist right;
  Glist result;
  Element N;
}  JoinSet;
// 
// Databases must look like machs.
//
// It needs to implement the mach methods it consider important
//
// In the default grammar, uses fetch,step and read on table it is not writing to.
//
enum MachMethods{ 
  Select=0,
  Reset,
  Exec,
  Fetch,
  Step,
  StepFetch,
  FetchRow,
  Index,
  Update,
  Append,
  Eval,
  Script,
  None};
// Possible return status
#define EV_Error 0x0100
#define EV_Done 0x0200
#define EV_Data 0x0400
#define MachError 0x0100
#define MachDone 0x0200
#define MachData 0x0400

// The actual object between  Json execution unit and
// the D base engine
//
  // Preinstalled sub classes of Mach
  enum {Mach_Base,Mach_Mem,Mach_Flat,Mach_Parse,
    Mach_Json,Mach_Console,Mach_Net,Mach_File,Mach_None
  } MachTypes;

  // **************  Mach  *******************
  typedef struct Mach { 
    int subclass;
    Cursor *cursors;  // Single Linked  cursor list open on the object
// This is the main method selector and all DBs must have
    int (*exec)(Mach *obj,int method,Element *argument);
// object variables
    NameKey key;  // Everything has a name somewhere
    Mach *schema;  // Json schema available in a mem mach 
    Mach *parent;  // stacked
    int attributes;  // Square? Fixed size set elements? Has a parsed index? Empty? No data? Data ready? 
    void * variables[8]; // Available for local db
  } Mach;
  typedef Mach * (*MachInit)(Mach *);
  Mach * machInit(Mach *);
  int machAddName(char *,Mach * (*h)(Mach *)); 
// Each implementation exports the mach init function.  
// The execution unit will make a Mach and install defaults.
// Mach Init needs to modifiy and put in its one exec_operator and binds
// Mach *Init(Mach *obj,Key name);

extern JoinSet join;  // This is static, but often copies are pushed and popped
extern  Mach matchMach; // A permanent '*'y dbase


 
 
