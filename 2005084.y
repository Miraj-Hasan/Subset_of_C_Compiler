%{
#include "2005084_SymbolTable.h"
#include "functions.h"
#include<iostream>
#include<fstream>
#include<cstdlib>
#include <algorithm>
#include <cctype>
#include<cstring>
#include<string>
#include<vector>
using namespace std;


//extern FILE *log;
FILE *parse = fopen("2005084_parseTree.txt","w");
FILE *fp = fopen("2005084_assembly.asm","w");

FILE *fin;

extern FILE *yyin;
extern int lineNo;
extern int yylex();
extern int errors;

int yylex_destroy(void);


string parseFile = "2005084_parseTree.txt";

extern SymbolTable *st;

extern int tableCount;

vector <SymbolInfo*> functions;
vector <SymbolInfo*> globalVars;


SymbolInfo* root;

void yyerror(char *s){
	printf("%d   %s\n",lineNo,s);
}



%}

%union {
    SymbolInfo *symbol;
}

%token <symbol> LPAREN RPAREN SEMICOLON COMMA LCURL RCURL INT FLOAT VOID LTHIRD RTHIRD CONST_FLOAT CONST_INT 
%token <symbol> IF ELSE FOR WHILE MULOP ADDOP LOGICOP PRINTLN RELOP RETURN ASSIGNOP DECOP INCOP NOT ID

%type <symbol> start program unit func_declaration func_definition parameter_list compound_statement var_declaration 
%type <symbol> type_specifier declaration_list statements statement expression_statement variable expression logic_expression 
%type <symbol> rel_expression simple_expression unary_expression term factor argument_list arguments


%%

start : program {
    $$ = new SymbolInfo();
    $$->setName("start");
    $$->allChildren = "program";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;
    root = $$;

    initialize(root, fp, globalVars);

    //fprintf(log, "start : program \n");
    //fprintf(log, "Total Lines: %d\nTotal Errors: %d\n",lineNo,errors);
}
;

program : program unit { 
    $$ = new SymbolInfo();
    $$->setName("program");
    $$->allChildren = "program unit";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->startLine = $1->startLine;
    $$->endLine = $2->endLine;

    //fprintf(log, "program : program unit \n"); 
}
| unit { 
    $$ = new SymbolInfo();
    $$->setName("program");
    $$->allChildren = "unit";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "program : unit \n"); 
}
;

unit : var_declaration {
    $$ = new SymbolInfo();
    $$->setName("unit");
    $$->allChildren = "var_declaration";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;
    //fprintf(log, "unit : var_declaration  \n"); 
}
| func_declaration { 
    $$ = new SymbolInfo();
    $$->setName("unit");
    $$->allChildren = "func_declaration";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;
    //fprintf(log, "unit : func_declaration \n"); 
}
| func_definition { 
    $$ = new SymbolInfo();
    $$->setName("unit");
    $$->allChildren = "func_definition";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;
    //fprintf(log, "unit : func_definition  \n"); 
}
;

func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON { 
    $$ = new SymbolInfo();
    $$->setName("func_declaration");
    $$->allChildren = "type_specifier ID LPAREN parameter_list RPAREN SEMICOLON";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->childList.push_back($5);
    $$->childList.push_back($6);
    $$->startLine = $1->startLine;
    $$->endLine = $6->endLine;

    SymbolInfo* func = new SymbolInfo($2->getName(), "FUNCTION");
    func->returnType = $1->getType();
    func->argList = $4->argList;
    st->insert(func);
    functions.push_back(func);

    //fprintf(log, "func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON \n"); 
}
| type_specifier ID LPAREN RPAREN SEMICOLON { 
    $$ = new SymbolInfo();
    $$->setName("func_declaration");
    $$->allChildren = "type_specifier ID LPAREN RPAREN SEMICOLON";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->childList.push_back($5);
    $$->startLine = $1->startLine;
    $$->endLine = $5->endLine;

    SymbolInfo* func = new SymbolInfo($2->getName(), "FUNCTION");
    func->returnType = $1->getType();
    st->insert(func);
    functions.push_back(func);
   
    //fprintf(log, "func_declaration : type_specifier ID LPAREN RPAREN SEMICOLON \n"); 
}
;

func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement { 
    $$ = new SymbolInfo();
    $$->setName("func_definition");
    $$->allChildren = "type_specifier ID LPAREN parameter_list RPAREN compound_statement";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->childList.push_back($5);
    $$->childList.push_back($6);
    $$->startLine = $1->startLine;
    $$->endLine = $6->endLine;

    int flag = 1;
    SymbolInfo* func = new SymbolInfo($2->getName(), "FUNCTION");
    func->returnType = $1->getType();
    functions.push_back(func);
        
    for(SymbolInfo* sym : $4->argList) {
        func->argList.push_back(sym);
    }

    int declared = 0;
    for(SymbolInfo* fun : functions) {
        if(fun->getName() == func->getName())
            declared = 1;
    }
            
    //fclose(log);
    //st->printAll("2005084_log.txt");
    st->exitScope();
    //log = fopen("2005084_log.txt","a");
    if(!declared)
        st->insert(func);
    //fprintf(log, "func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement \n");
    
}
| type_specifier ID LPAREN RPAREN compound_statement { 
    $$ = new SymbolInfo();
    $$->setName("func_definition");
    $$->allChildren = "type_specifier ID LPAREN RPAREN compound_statement";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->childList.push_back($5);
    $$->startLine = $1->startLine;
    $$->endLine = $5->endLine;

    int flag = 1;
    SymbolInfo* func = new SymbolInfo($2->getName(), "FUNCTION");
    func->returnType = $1->getType();

    int declared = 0;
    for(SymbolInfo* fun : functions) {
        if(fun->getName() == func->getName())
            declared = 1;
    }
    if(!declared)
        functions.push_back(func);
    
    //fclose(log);
    //st->printAll("2005084_log.txt");
    st->exitScope();
    //log = fopen("2005084_log.txt","a");
    if(!declared)
        st->insert(func);
    //fprintf(log, "func_definition : type_specifier ID LPAREN RPAREN compound_statement\n");
}
;

parameter_list : parameter_list COMMA type_specifier ID {
    $$ = new SymbolInfo();
    $$->setName("parameter_list");
    $$->allChildren = "parameter_list COMMA type_specifier ID";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->startLine = $1->startLine;
    $$->endLine = $4->endLine;

    SymbolInfo* param = new SymbolInfo($4->getName(), $4->getType());
    param->dataType = $3->getType();
    $4->dataType = $3->getType();
    $$->argList = $1->argList;
    $$->argList.push_back(param);
    st->pendingArg.clear();
    st->pendingArg = $$->argList;
    
    //fprintf(log, "parameter_list  : parameter_list COMMA type_specifier ID\n"); 
}
| parameter_list COMMA type_specifier { 
    $$ = new SymbolInfo();
    $$->setName("parameter_list");
    $$->allChildren = "parameter_list COMMA type_specifier";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;

    SymbolInfo* param = new SymbolInfo("##", $3->getType());
    param->dataType = $3->getType();
    $$->argList = $1->argList;
    $$->argList.push_back(param);
    st->pendingArg.clear();
    st->pendingArg = $$->argList;
    
    //fprintf(log, "parameter_list  : parameter_list COMMA type_specifier\n"); 
}
| type_specifier ID { 
    $$ = new SymbolInfo();
    $$->setName("parameter_list");
    $$->allChildren = "type_specifier ID";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->startLine = $1->startLine;
    $$->endLine = $2->endLine;

    SymbolInfo* param = new SymbolInfo($2->getName(), $2->getType());
    $2->dataType = $1->getType();
    param->dataType = $1->getType();
    $$->argList.push_back(param);
    st->pendingArg.clear();
    st->pendingArg = $$->argList;
    
    //fprintf(log, "parameter_list  : type_specifier ID\n");
}
| type_specifier {
    $$ = new SymbolInfo();
    $$->setName("parameter_list");
    $$->allChildren = "type_specifier";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    SymbolInfo* param = new SymbolInfo("##", $1->getType());
    param->dataType = $1->getType();
    $$->argList.push_back(param);
    st->pendingArg.clear();
    st->pendingArg = $$->argList;

    //fprintf(log, "parameter_list  : type_specifier\n");
}
;

compound_statement : LCURL statements RCURL { 
    $$ = new SymbolInfo();
    $$->setName("compound_statement");
    $$->allChildren = "LCURL statements RCURL";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;

    //fprintf(log, "compound_statement : LCURL statements RCURL  \n"); 
    
}
| LCURL RCURL { 
    $$ = new SymbolInfo();
    $$->setName("compound_statement");
    $$->allChildren = "LCURL RCURL";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->startLine = $1->startLine;
    $$->endLine = $2->endLine;

    //fprintf(log, "compound_statement : LCURL RCURL  \n"); 
    
}
;

var_declaration : type_specifier declaration_list SEMICOLON {
    $$ = new SymbolInfo();
    $$->setName("var_declaration");
    $$->allChildren = "type_specifier declaration_list SEMICOLON";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;

    int flag = 1;
    for(SymbolInfo* s1 : $2->argList) {
        SymbolInfo* si = st->find(s1->getName());
        if(s1->arraySize != -1) {
            if(si == NULL) 
            {
                s1->setType("ARRAY");
                s1->arrayType = $1->getType();
                s1->dataType = $1->getType();
            }
        }  
        else {
            if(si == NULL) {
                s1->dataType = $1->getType();
            }
                
        }
            
        if(st->getCurrentID() == 1) {
            for(SymbolInfo* sym : globalVars) {
                if(s1->getName() == sym->getName()) {
                    flag = 0;
                }
            }
        }

        if(flag == 1) {
            SymbolInfo *temp = st->find(s1->getName());
            s1->scope = st->current->id;
            st->insert(s1);
            if(st->getCurrentID() == 1) {
                globalVars.push_back(s1);
            }   
        }
        flag = 1;
    }

    //fprintf(log, "var_declaration : type_specifier declaration_list SEMICOLON  \n"); 
}
;

type_specifier : INT {
    $$ = new SymbolInfo();
    $$->setName("type_specifier");
    $$->setType("INT");
    $$->allChildren = "INT";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "type_specifier	: INT \n"); 
}
| FLOAT {
    $$ = new SymbolInfo();
    $$->setName("type_specifier");
    $$->setType("FLOAT");
    $$->allChildren = "FLOAT";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "type_specifier	: FLOAT \n");
}
| VOID {
    $$ = new SymbolInfo();
    $$->setName("type_specifier");
    $$->setType("VOID");
    $$->allChildren = "VOID";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "type_specifier	: VOID\n");
}
;

declaration_list : declaration_list COMMA ID {
    $$ = new SymbolInfo();
    $$->setName("declaration_list");
    $$->allChildren = "declaration_list COMMA ID";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;

    SymbolInfo *sym = new SymbolInfo($3->getName(), $3->getType());
    $$->argList = $1->argList;
    $$->argList.push_back(sym);
    //fprintf(log, "declaration_list : declaration_list COMMA ID  \n");
}
| declaration_list COMMA ID LTHIRD CONST_INT RTHIRD {
    $$ = new SymbolInfo();
    $$->setName("declaration_list");
    $$->allChildren = "declaration_list COMMA ID LSQUARE CONST_INT RSQUARE";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->childList.push_back($5);
    $$->childList.push_back($6);
    $$->startLine = $1->startLine;
    $$->endLine = $6->endLine;

    SymbolInfo *sym = new SymbolInfo($3->getName(), "ARRAY");
    sym->arraySize = stoi($5->getName());
    $$->argList = $1->argList;
    $$->argList.push_back(sym);
    //fprintf(log, "declaration_list : declaration_list COMMA ID LSQUARE CONST_INT RSQUARE \n");
}
| ID {
    $$ = new SymbolInfo();
    $$->setName("declaration_list");
    $$->allChildren = "ID";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    SymbolInfo* temp = new SymbolInfo($1->getName(), $1->getType());
    $$->argList.push_back(temp);

    //fprintf(log, "declaration_list : ID \n");
}
| ID LTHIRD CONST_INT RTHIRD {
    $$ = new SymbolInfo();
    $$->setName("declaration_list");
    $$->setType("ARRAY");
    $$->allChildren = "ID LSQUARE CONST_INT RSQUARE";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->startLine = $1->startLine;
    $$->endLine = $4->endLine;

    SymbolInfo *sym = new SymbolInfo($1->getName(), "ARRAY");
    sym->arraySize = stoi($3->getName());
    $$->argList.push_back(sym);

    //fprintf(log, "declaration_list : ID LSQUARE CONST_INT RSQUARE \n");
}
;

statements : statement {
    $$ = new SymbolInfo();
    $$->setName("statements");
    $$->allChildren = "statement";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;
    
    //fprintf(log, "statements : statement  \n");
}
| statements statement {
    $$ = new SymbolInfo();
    $$->setName("statements");
    $$->allChildren = "statements statement";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->startLine = $1->startLine;
    $$->endLine = $2->endLine;

    //fprintf(log, "statements : statements statement  \n");
}
;

statement : var_declaration {
    $$ = new SymbolInfo();
    $$->setName("statement");
    $$->allChildren = "var_declaration";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "statement : var_declaration \n");
}
| expression_statement {
    $$ = new SymbolInfo();
    $$->setName("statement");
    $$->allChildren = "expression_statement";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "statement : expression_statement  \n");
}
| compound_statement {
    $$ = new SymbolInfo();
    $$->setName("statement");
    $$->allChildren = "compound_statement";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "statement : compound_statement \n");
    st->exitScope();
}
| FOR LPAREN expression_statement expression_statement expression RPAREN statement {
    $$ = new SymbolInfo();
    $$->setName("statement");
    $$->allChildren = "FOR LPAREN expression_statement expression_statement expression RPAREN statement";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->childList.push_back($5);
    $$->childList.push_back($6);
    $$->childList.push_back($7);
    $$->startLine = $1->startLine;
    $$->endLine = $7->endLine;

    //fprintf(log, "statement : FOR LPAREN expression_statement expression_statement expression RPAREN statement \n");
}
| IF LPAREN expression RPAREN statement {
    $$ = new SymbolInfo();
    $$->setName("statement");
    $$->allChildren = "IF LPAREN expression RPAREN statement";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->childList.push_back($5);
    $$->startLine = $1->startLine;
    $$->endLine = $5->endLine;

    //fprintf(log, "IF LPAREN expression RPAREN statement \n");
}
| IF LPAREN expression RPAREN statement ELSE statement {
    $$ = new SymbolInfo();
    $$->setName("statement");
    $$->allChildren = "IF LPAREN expression RPAREN statement ELSE statement";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->childList.push_back($5);
    $$->childList.push_back($6);
    $$->childList.push_back($7);
    $$->startLine = $1->startLine;
    $$->endLine = $7->endLine;

    //fprintf(log, "statement : IF LPAREN expression RPAREN statement ELSE statement \n");
}
| WHILE LPAREN expression RPAREN statement {
    $$ = new SymbolInfo();
    $$->setName("statement");
    $$->allChildren = "WHILE LPAREN expression RPAREN statement";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->childList.push_back($5);
    $$->startLine = $1->startLine;
    $$->endLine = $5->endLine;

    //fprintf(log, "statement : WHILE LPAREN expression RPAREN statement \n");
}
| PRINTLN LPAREN ID RPAREN SEMICOLON {
    $$ = new SymbolInfo();
    $$->setName("statement");
    $$->allChildren = "PRINTLN LPAREN ID RPAREN SEMICOLON";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->childList.push_back($5);
    $$->startLine = $1->startLine;
    $$->endLine = $5->endLine;

    //fprintf(log, "statement : PRINTLN LPAREN ID RPAREN SEMICOLON \n");
}
| RETURN expression SEMICOLON {
    $$ = new SymbolInfo();
    $$->setName("statement");
    $$->allChildren = "RETURN expression SEMICOLON";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;

    //fprintf(log, "statement : RETURN expression SEMICOLON\n");
}
;

expression_statement : SEMICOLON {
    $$ = new SymbolInfo();
    $$->setName("expression_statement");
    $$->allChildren = "SEMICOLON";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "expression_statement : SEMICOLON 		 \n");
}
| expression SEMICOLON {
    $$ = new SymbolInfo();
    $$->setName("expression_statement");
    $$->allChildren = "expression SEMICOLON";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->startLine = $1->startLine;
    $$->endLine = $2->endLine;

    //fprintf(log, "expression_statement : expression SEMICOLON 		 \n");
}
;

variable : ID {
    $$ = new SymbolInfo();
    $$->setName("variable");
    $$->setType("ID");

    SymbolInfo* temp = st->find($1->getName());

    if(temp != NULL)
        $$->dataType = temp->dataType;

    $$->allChildren = "ID";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "variable : ID 	 \n");
}
| ID LTHIRD expression RTHIRD {
    $$ = new SymbolInfo();
    $$->setName("variable");
    
    SymbolInfo* res = new SymbolInfo();

    SymbolInfo* temp = st->find($1->getName());
    if(temp != NULL && temp->getType() == "ARRAY") {
        $$->setType(temp->arrayType);
        $$->dataType = $1->dataType;
    }

    $$->allChildren = "ID LSQUARE expression RSQUARE";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->startLine = $1->startLine;
    $$->endLine = $4->endLine;

    //fprintf(log, "variable : ID LSQUARE expression RSQUARE  	 \n");
}
;

expression : logic_expression {
    $$ = new SymbolInfo();
    $$->setName("expression");
    $$->allChildren = "logic_expression";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "expression 	: logic_expression	 \n");
}
| variable ASSIGNOP logic_expression {
    $$ = new SymbolInfo();
    $$->setName("expression");
    $$->allChildren = "variable ASSIGNOP logic_expression";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;

    //fprintf(log, "expression 	: variable ASSIGNOP logic_expression 		 \n");
}
;

logic_expression : rel_expression {
    $$ = new SymbolInfo();
    $$->setName("logic_expression");
    $$->allChildren = "rel_expression";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "logic_expression : rel_expression 	 \n");
}
| rel_expression LOGICOP rel_expression {
    $$ = new SymbolInfo();
    $$->setName("logic_expression");
    $$->allChildren = "rel_expression LOGICOP rel_expression";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;

    //fprintf(log, "logic_expression : rel_expression LOGICOP rel_expression 	 	 \n");
}
;

rel_expression : simple_expression {
    $$ = new SymbolInfo();
    $$->setName("rel_expression");
    $$->allChildren = "simple_expression";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "rel_expression	: simple_expression \n");
}
| simple_expression RELOP simple_expression {
    $$ = new SymbolInfo();
    $$->setName("rel_expression");
    $$->allChildren = "simple_expression RELOP simple_expression";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;
    
    //fprintf(log, "rel_expression	: simple_expression RELOP simple_expression	  \n");
}
;

simple_expression : term {
    $$ = new SymbolInfo();
    $$->setName("simple_expression");
    $$->allChildren = "term";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "simple_expression : term \n");
}
| simple_expression ADDOP term {
    $$ = new SymbolInfo();
    $$->setName("simple_expression");
    $$->allChildren = "simple_expression ADDOP term";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;

    //fprintf(log, "simple_expression : simple_expression ADDOP term  \n");
}
;

term : unary_expression {
    $$ = new SymbolInfo();
    $$->setName("term");
    $$->allChildren = "unary_expression";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "term :	unary_expression \n");
}
| term MULOP unary_expression {
    $$ = new SymbolInfo();
    $$->setName("term");
    $$->allChildren = "term MULOP unary_expression";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;

    //fprintf(log, "term :	term MULOP unary_expression \n");
}
;

unary_expression : ADDOP unary_expression {
    $$ = new SymbolInfo();
    $$->setName("unary_expression");
    $$->allChildren = "ADDOP unary_expression";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->startLine = $1->startLine;
    $$->endLine = $2->endLine;

    //fprintf(log, "unary_expression : ADDOP unary_expression \n");
}
| NOT unary_expression {
    $$ = new SymbolInfo();
    $$->setName("unary_expression");
    $$->allChildren = "NOT unary_expression";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->startLine = $1->startLine;
    $$->endLine = $2->endLine;

    //fprintf(log, "unary_expression : NOT unary_expression \n");
}
| factor {
    $$ = new SymbolInfo();
    $$->setName("unary_expression");
    $$->allChildren = "factor";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "unary_expression : factor \n");
}
;

factor : variable {
    $$ = new SymbolInfo();
    $$->setName("factor");
    $$->allChildren = "variable";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "factor	: variable \n");
}
| ID LPAREN argument_list RPAREN {
    $$ = new SymbolInfo();
    $$->setName("factor");
    $$->allChildren = "ID LPAREN argument_list RPAREN";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->childList.push_back($4);
    $$->startLine = $1->startLine;
    $$->endLine = $4->endLine;

    //fprintf(log, "factor	: ID LPAREN argument_list RPAREN  \n");
}
| LPAREN expression RPAREN {
    $$ = new SymbolInfo();
    $$->setName("factor");
    $$->allChildren = "LPAREN expression RPAREN";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;
    
    //fprintf(log, "factor	: LPAREN expression RPAREN   \n");
}
| CONST_INT {
    $$ = new SymbolInfo();
    $$->setName("factor");
    $$->allChildren = "CONST_INT";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    $$->setType($1->getType());
    $$->dataType = $1->dataType;

    //fprintf(log, "factor	: CONST_INT   \n");
}
| CONST_FLOAT {
    $$ = new SymbolInfo();
    $$->setName("factor");
    $$->allChildren = "CONST_FLOAT";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;
    $$->setType($1->getType());
    $$->dataType = $1->dataType;

    //fprintf(log, "factor	: CONST_FLOAT   \n");
}
| variable INCOP {
    $$ = new SymbolInfo();
    $$->setName("factor");
    $$->allChildren = "variable INCOP";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->startLine = $1->startLine;
    $$->endLine = $2->endLine;

    //fprintf(log, "factor	: variable INCOP   \n");
}
| variable DECOP {
    $$ = new SymbolInfo();
    $$->setName("factor");
    $$->allChildren = "variable DECOP";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->startLine = $1->startLine;
    $$->endLine = $2->endLine;

    //fprintf(log, "factor	: variable DECOP   \n");
}
;

argument_list : arguments {
    $$ = new SymbolInfo();
    $$->setName("argument_list");
    $$->allChildren = "arguments";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "argument_list : arguments  \n");
}
| {
    $$ = new SymbolInfo();
    $$->setName("argument_list");
    $$->allChildren = "";
}
;

arguments : arguments COMMA logic_expression {
    $$ = new SymbolInfo();
    $$->setName("arguments");
    $$->allChildren = "arguments COMMA logic_expression";
    $$->childList.push_back($1);
    $$->childList.push_back($2);
    $$->childList.push_back($3);
    $$->startLine = $1->startLine;
    $$->endLine = $3->endLine;

    //fprintf(log, "arguments : arguments COMMA logic_expression \n");
}
| logic_expression {
    $$ = new SymbolInfo();
    $$->setName("arguments");
    $$->allChildren = "logic_expression";
    $$->childList.push_back($1);
    $$->startLine = $1->startLine;
    $$->endLine = $1->endLine;

    //fprintf(log, "arguments : logic_expression\n");
}
;


%%
int main(int argc,char *argv[]) {
    if(argc != 2) {
        printf("Please provide input file name and try again\n");
        return 0;
    }

    fin = fopen(argv[1],"r");
    if(fin == NULL) {
        printf("Cannot open specified file\n");
        return 0;
    }

    yyin = fin;
    
    yyparse();

    root->printParseTree(0, parse);
    fclose(parse);
    fclose(yyin);
    return 0;

}
