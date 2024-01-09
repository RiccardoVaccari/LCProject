%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.2"
%defines

%define api.token.constructor
%define api.location.file none
%define api.value.type variant
%define parse.assert

%code requires {
  # include <string>
  #include <exception>
  class driver;
  class RootAST;
  class ExprAST;
  class NumberExprAST;
  class VariableExprAST;
  class CallExprAST;
  class FunctionAST;
  class SeqAST;
  class PrototypeAST;
  class BlockAST;
  class BindingAST;
  class VarBindingAST;
  class ArrayBindingAST;
  class StmtAST;
  class AssignmentAST;
  class GlobalVarAST;
  class IfStmtAST;
  class ForStmtAST;
  class WhileStmtAST;
  class VarOperation;
  class ArrayExprAST;
}

// The parsing context.
%param { driver& drv }

%locations

%define parse.trace
%define parse.error verbose

%code {
# include "driver.hpp"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  SEMICOLON  ";"
  COMMA      ","
  MINUS      "-"
  PLUS       "+"
  STAR       "*"
  SLASH      "/"
  LPAREN     "("
  RPAREN     ")"
  QMARK	     "?"
  COLON      ":"
  LT         "<"
  GT         ">"
  EQ         "=="
  ASSIGN     "="
  LBRACE     "{"
  RBRACE     "}"
  LSQBR      "["
  RSQBR      "]"
  EXTERN     "extern"
  DEF        "def"
  VAR        "var"
  GLOBAL     "global"
  IF         "if"
  ELSE       "else"
  FOR        "for"
  WHILE      "while"
  AND        "and"
  OR         "or"
  NOT        "not"
;

%token <std::string> IDENTIFIER "id"
%token <double> NUMBER "number"
%type <ExprAST*> exp
%type <ExprAST*> initexp
%type <ExprAST*> idexp
%type <ExprAST*> expif
%type <ExprAST*> condexp
%type <ExprAST*> relexp
%type <std::vector<ExprAST*>> optexp
%type <std::vector<ExprAST*>> explist
%type <RootAST*> program
%type <RootAST*> top
%type <FunctionAST*> definition
%type <PrototypeAST*> external
%type <PrototypeAST*> proto
%type <std::vector<std::string>> idseq
%type <BlockAST*> block
%type <std::vector<BindingAST*>> vardefs
%type <BindingAST*> binding
%type <GlobalVarAST*> globalvar
%type <std::vector<StmtAST*>> stmts
%type <StmtAST*> stmt
%type <AssignmentAST*> assignment
%type <IfStmtAST*> ifstmt
%type <ForStmtAST*> forstmt
%type <WhileStmtAST*> whilestmt
%type <VarOperation*> init

%%
%start startsymb;

startsymb:
program                 { drv.root = $1; };

program:
  %empty                { $$ = new SeqAST(nullptr,nullptr); }
|  top ";" program      { $$ = new SeqAST($1,$3); };

top:
%empty                  { $$ = nullptr; }
| definition            { $$ = $1; }
| external              { $$ = $1; }
| globalvar             { $$ = $1; };

definition:
  "def" proto block     { $$ = new FunctionAST($2,$3); $2->noemit(); }; //!

external:
  "extern" proto        { $$ = $2; };

proto:
  "id" "(" idseq ")"    { $$ = new PrototypeAST($1,$3);  };

globalvar:
  "global" "id"                   { $$ = new GlobalVarAST($2); }
| "global" "id" "[" "number" "]"  { $$ = new GlobalVarAST($2, $4); };

idseq:
  %empty                { std::vector<std::string> args;
                         $$ = args; }
| "id" idseq            { $2.insert($2.begin(),$1); $$ = $2; };

%left ":";
%left "<" ">" "==";
%left "+" "-";
%left "*" "/";

stmts:
  stmt                  { std::vector<StmtAST*> statements;
                          statements.push_back($1);
                          $$ = statements; }
| stmt ";" stmts        { $3.insert($3.begin(), $1);
                          $$ = $3; };

stmt:
  assignment            { $$ = $1; }
| block                 { $$ = $1; }
| ifstmt                { $$ = $1; }
| forstmt               { $$ = $1; }
| whilestmt             { $$ = $1; }
| exp                   { $$ = $1; };

assignment:
 "id" "=" exp               { $$ = new AssignmentAST($1, $3); } 
| "+" "+" "id"              { $$ = new AssignmentAST($3,new BinaryExprAST('+',new VariableExprAST($3),new NumberExprAST(1.0)));}   
//| "id" "+" "+"            { $$ = new AssignmentAST($1,new BinaryExprAST('+',new VariableExprAST($1),new NumberExprAST(1.0)));}
| "-" "-" "id"              { $$ = new AssignmentAST($3,new BinaryExprAST('-',new VariableExprAST($3),new NumberExprAST(1.0)));}
//| "id" "-" "-"            { $$ = new AssignmentAST($1,new BinaryExprAST('-',new VariableExprAST($1),new NumberExprAST(1.0)));};          
| "id" "[" exp "]" "=" exp  { $$ = new AssignmentAST($1,$3,$6); }; //NEW


block:
  "{" stmts "}"             { $$ = new BlockAST($2); }
| "{" vardefs ";" stmts "}" { $$ = new BlockAST($2,$4); };

vardefs:
  binding               { std::vector<BindingAST*> definitions;
                          definitions.push_back($1);
                          $$ = definitions; }
| vardefs ";" binding   { $1.push_back($3); $$ = $1; };

binding:
  "var" "id" initexp                                  { $$ = new VarBindingAST($2,$3); }
| "var" "id" "[" "number" "]"                         { $$ = new ArrayBindingAST($2,$4); } //NEW
| "var" "id" "[" "number" "]" "=" "{" explist "}"     { $$ = new ArrayBindingAST($2,$4,$8); }; //NEW
                    
exp:
  exp "+" exp           { $$ = new BinaryExprAST('+',$1,$3); }
| exp "-" exp           { $$ = new BinaryExprAST('-',$1,$3); }
| exp "*" exp           { $$ = new BinaryExprAST('*',$1,$3); }
| exp "/" exp           { $$ = new BinaryExprAST('/',$1,$3); }
| idexp                 { $$ = $1; }
| "(" exp ")"           { $$ = $2; }
| "-" exp               { $$ = new BinaryExprAST('*',$2, new NumberExprAST(-1.0)); }
| "number"              { $$ = new NumberExprAST($1); }
| expif                 { $$ = $1; };

initexp: 
  %empty                { $$ = nullptr; }
| "=" exp               { $$ = $2; };

expif:
  condexp "?" exp ":" exp { $$ = new IfExprAST($1,$3,$5); };

condexp:
  relexp                  { $$ = $1; }
| relexp "and" condexp    { $$ = new BinaryExprAST('a',$1,$3); }
| relexp "or" condexp     { $$ = new BinaryExprAST('o',$1,$3); }
| "not" condexp           { $$ = new BinaryExprAST('n',$2); }
| "(" condexp ")"         { $$ = $2; };

relexp:
  exp "<" exp           { $$ = new BinaryExprAST('<',$1,$3); }
| exp ">" exp           { $$ = new BinaryExprAST('>',$1,$3); }
| exp "==" exp          { $$ = new BinaryExprAST('=',$1,$3); };

idexp:
  "id"                  { $$ = new VariableExprAST($1); }
| "id" "(" optexp ")"   { $$ = new CallExprAST($1,$3); }
| "id" "[" exp "]"      { $$ = new ArrayExprAST($1,$3); }; //NEW

optexp:
  %empty                { std::vector<ExprAST*> args;
			                    $$ = args; }
| explist               { $$ = $1; };

explist:
  exp                   { std::vector<ExprAST*> args;
                         args.push_back($1);
			                   $$ = args;
                        }
| exp "," explist       { $3.insert($3.begin(), $1); $$ = $3; };
 
%right "else" ")";

ifstmt:
  "if" "(" condexp ")" stmt                          { $$ = new IfStmtAST($3, $5); }
| "if" "(" condexp ")" stmt "else" stmt              { $$ = new IfStmtAST($3, $5, $7); };

init:
  binding              { $$ = new VarOperation($1); }
| assignment           { $$ = new VarOperation($1); };

forstmt:
  "for" "(" init ";" condexp ";" assignment ")" stmt { $$ = new ForStmtAST($3, $5, $7, $9);};

whilestmt:
  "while" "(" condexp ")" stmt                       { $$ = new WhileStmtAST($3, $5); };

%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
