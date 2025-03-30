%option noyywrap

%{
#include "2005084_SymbolTable.h"
#include<iostream>
#include<fstream>
#include<cstdlib>
#include<cstring>
#include<vector>
#include<string>
#include "y.tab.h"
using namespace std;

extern int lineNo = 1;
extern FILE *yyin;
//extern FILE *log = fopen("2005084_log.txt", "w");
extern int tableCount = 1;

int bucketSize = 11;
extern SymbolTable *st = new SymbolTable(bucketSize);
extern int errors = 0;
int warnings = 0;

%}

letter [A-Za-z]
digit [0-9]
alphaNeum [A-Za-z0-9] 


%%

[ \t\v]+ {}

(\r)?\n {
    lineNo++;
    
}

if {
        ////fprintf(log, "Line# %d: Token <IF> Lexeme %s found\n", lineNo, yytext);
        yylval.symbol = new SymbolInfo(yytext, "IF");
        yylval.symbol->IsLeaf = true;
        yylval.symbol->startLine = lineNo;
        yylval.symbol->endLine = lineNo;
        return IF;
    }

for {
        //fprintf(log, "Line# %d: Token <FOR> Lexeme %s found\n", lineNo, yytext);
        yylval.symbol = new SymbolInfo(yytext, "FOR");
        yylval.symbol->IsLeaf = true;
        yylval.symbol->startLine = lineNo;
        yylval.symbol->endLine = lineNo;
        return FOR; 
    }

int {
        //fprintf(log, "Line# %d: Token <INT> Lexeme %s found\n", lineNo, yytext);
        yylval.symbol = new SymbolInfo(yytext, "INT");
        yylval.symbol->IsLeaf = true;
        yylval.symbol->startLine = lineNo;
        yylval.symbol->endLine = lineNo;
        return INT;
    }

float {
        //fprintf(log, "Line# %d: Token <FLOAT> Lexeme %s found\n", lineNo, yytext);
        yylval.symbol = new SymbolInfo(yytext, "FLOAT");
        yylval.symbol->IsLeaf = true;
        yylval.symbol->startLine = lineNo;
        yylval.symbol->endLine = lineNo;
        return FLOAT; 
    }

void {
        //fprintf(log, "Line# %d: Token <VOID> Lexeme %s found\n", lineNo, yytext);
        yylval.symbol = new SymbolInfo(yytext, "VOID");
        yylval.symbol->IsLeaf = true;
        yylval.symbol->startLine = lineNo;
        yylval.symbol->endLine = lineNo;
        return VOID;
    }

else {
        //fprintf(log, "Line# %d: Token <ELSE> Lexeme %s found\n", lineNo, yytext);
        yylval.symbol = new SymbolInfo(yytext, "ELSE");
        yylval.symbol->IsLeaf = true;
        yylval.symbol->startLine = lineNo;
        yylval.symbol->endLine = lineNo;
        return ELSE; 
    }

while {
        //fprintf(log, "Line# %d: Token <WHILE> Lexeme %s found\n", lineNo, yytext);
        yylval.symbol = new SymbolInfo(yytext, "WHILE");
        yylval.symbol->IsLeaf = true;
        yylval.symbol->startLine = lineNo;
        yylval.symbol->endLine = lineNo;
        return WHILE; 
    }

return {
        //fprintf(log, "Line# %d: Token <RETURN> Lexeme %s found\n", lineNo, yytext);
        yylval.symbol = new SymbolInfo(yytext, "RETURN");
        yylval.symbol->IsLeaf = true;
        yylval.symbol->startLine = lineNo;
        yylval.symbol->endLine = lineNo;
        return RETURN;
    }

println {
        //fprintf(log, "Line# %d: Token <PRINTLN> Lexeme %s found\n", lineNo, yytext);
        yylval.symbol = new SymbolInfo(yytext, "PRINTLN");
        yylval.symbol->IsLeaf = true;
        yylval.symbol->startLine = lineNo;
        yylval.symbol->endLine = lineNo;
        return PRINTLN;
}

{digit}+ {
    //fprintf(log, "Line# %d: Token <CONST_INT> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "CONST_INT");
    yylval.symbol->dataType = "INT";
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return CONST_INT;    
}

(({digit}*\.{digit}+)([Ee][+-]?{digit}+)?)|({digit}+([Ee][+-]?{digit}+)) {
    //fprintf(log, "Line# %d: Token <CONST_FLOAT> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "CONST_FLOAT");
    yylval.symbol->dataType = "FLOAT";
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return CONST_FLOAT;
}


[+-] {
    //fprintf(log, "Line# %d: Token <ADDOP> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "ADDOP");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return ADDOP;
}

"*"|"/"|"%" {
    //fprintf(log, "Line# %d: Token <MULOP> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "MULOP");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return MULOP;
}

"++" {
    //fprintf(log, "Line# %d: Token <INCOP> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "INCOP");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return INCOP;
}

"--" {
    //fprintf(log, "Line# %d: Token <DECOP> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "DECOP");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return DECOP;
}

"<="|"<"|">"|">="|"=="|"!=" {
    //fprintf(log, "Line# %d: Token <RELOP> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "RELOP");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return RELOP;
}

"=" {
    //fprintf(log, "Line# %d: Token <ASSIGNOP> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "ASSIGNOP");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return ASSIGNOP;
}

"&&"|"||" {
    //fprintf(log, "Line# %d: Token <LOGICOP> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "LOGICOP");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return LOGICOP;
}

"&"|"|"|"^"|"<<"|">>" {
    
}

"!" {
    //fprintf(log, "Line# %d: Token <NOT> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "NOT");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return NOT;
}

"(" {
    //fprintf(log, "Line# %d: Token <LPAREN> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "LPAREN");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return LPAREN;
}

")" {
    //fprintf(log, "Line# %d: Token <RPAREN> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "RPAREN");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return RPAREN;
}

"{" {
    
    vector <SymbolInfo*> temp;
    int flag = 1;
    if(st->pendingArg.size() > 0) {
        for(SymbolInfo* par : st->pendingArg) {
            if(temp.size() > 0) {
                for(SymbolInfo* symb : temp) {
                    if(symb->getName() == par->getName()) {
                        flag = 0;
                        break;
                    }
                }
            }
            if(flag) {
                temp.push_back(par);
            }
        }
    }
    st->pendingArg.clear();
    st->createScope(tableCount);
    for(SymbolInfo* para : temp)
        st->insert(para);
    tableCount++;
    //fprintf(log, "Line# %d: Token <LCURL> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "LCURL");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return LCURL;
}

"}" {
    //fprintf(log, "Line# %d: Token <RCURL> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "RCURL");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    //st->exitScope();
    return RCURL;
}

"[" {
    //fprintf(log, "Line# %d: Token <LSQUARE> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "LSQUARE");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return LTHIRD;
}

"]" {
    //fprintf(log, "Line# %d: Token <RSQUARE> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "RSQUARE");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return RTHIRD;
}

"," {
    //fprintf(log, "Line# %d: Token <COMMA> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "COMMA");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return COMMA;
}

";" {
    //fprintf(log, "Line# %d: Token <SEMICOLON> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "SEMICOLON");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return SEMICOLON;
}


({letter}|_)({alphaNeum}|_)* {
    //fprintf(log, "Line# %d: Token <ID> Lexeme %s found\n", lineNo, yytext);
    yylval.symbol = new SymbolInfo(yytext, "ID");
    yylval.symbol->IsLeaf = true;
    yylval.symbol->startLine = lineNo;
    yylval.symbol->endLine = lineNo;
    return ID;
}

. {
    
}

<<EOF>> {
    
    return 0;
}



%%
