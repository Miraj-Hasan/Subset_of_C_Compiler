//#include "2005084_SymbolTable.h"
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <string>
#include <cstdlib>
#include <unordered_map>

using namespace std;

FILE *assembly;
vector<SymbolInfo*> globals;
int labelNo;
unordered_map< string, int > localVars;
int offset;
bool InMain;
string returnLabel;

void generateAssembly (SymbolInfo* root);

void printLibrary();

void optimizer();

void initialize (SymbolInfo* root, FILE* fp, vector<SymbolInfo*> globalVars) {
    assembly = fp;
    globals = globalVars;
    InMain = false;
    returnLabel = "";
    fprintf(assembly, ".MODEL SMALL\n");
    fprintf(assembly, ".STACK 1000H\n");
    fprintf(assembly, ".Data\n");
    fprintf(assembly, "\tnumber DB \"00000$\"\n");
    for (SymbolInfo* sym : globalVars) {
        if(sym->getType() == "ARRAY")
            fprintf(assembly, "\t%s DW %d DUP (0000H)\n", sym->getName().c_str(), sym->arraySize);
        else
            fprintf(assembly, "\t%s DW 1 DUP (0000H)\n", sym->getName().c_str());
    }
    fprintf(assembly, ".CODE\n");

    labelNo = 0;

    generateAssembly(root);

    printLibrary();

    fprintf(assembly, "END main\n");

    fclose(fp);


    optimizer();


}

bool IsGlobal (string name) {
    for (SymbolInfo* sym : globals) {
        if (sym->getName() == name)
            return true;
    }
    return false;
}

string createLabel() {
    labelNo++;
    string label = "L" + to_string(labelNo);
    return label;
}

void mainProcStart() {
    fprintf(assembly, "main PROC\n");
    fprintf(assembly, "\tMOV AX, @DATA\n");
    fprintf(assembly, "\tMOV DS, AX\n");
    fprintf(assembly, "\tPUSH BP\n");
    fprintf(assembly, "\tMOV BP, SP\n");
}

void mainProcEnd() {
    fprintf(assembly, "\tPOP BP\n");
    fprintf(assembly, "\tMOV AX,4CH\n");
    fprintf(assembly, "\tINT 21H\n");
    fprintf(assembly, "main ENDP\n");
}








void generateAssembly (SymbolInfo* root) {


    if(root->allChildren == "program") {
        generateAssembly(root->childList[0]);
    }

    if(root->allChildren == "program unit") {
        generateAssembly(root->childList[0]);
        generateAssembly(root->childList[1]);
    }

    if(root->allChildren == "unit") {
        generateAssembly(root->childList[0]);
    }

    if(root->getName() == "statement" && root->allChildren == "var_declaration") {
        root->childList[0]->IsMain = root->IsMain;

        root->childList[0]->IsCondition = root->IsCondition;
        
        generateAssembly(root->childList[0]);
        root->nextLabel = root->childList[0]->nextLabel;

        root->stackUsed = root->childList[0]->stackUsed;
    }

    if(root->allChildren == "func_definition") {
        generateAssembly(root->childList[0]);
    }

    if(root->allChildren == "type_specifier ID LPAREN parameter_list RPAREN compound_statement") {
        
        fprintf(assembly, "%s PROC\n", root->childList[1]->getName().c_str());
        fprintf(assembly, "\tPUSH BP\n");
        fprintf(assembly, "\tMOV BP, SP\n");

        generateAssembly(root->childList[3]);

        offset = 2;

        SymbolInfo* decList = root->childList[3];
        int totalArg = decList->argList.size();
        offset += totalArg*2 + 2;
        for (SymbolInfo* sym : decList->argList) {
            if (sym->getType() != "ARRAY") {
                offset -= 2;
                localVars[sym->getName()] = -offset;
                root->stackUsed += 2;
            }
            else {
                offset -= 2*sym->arraySize;
                localVars[sym->getName()] = -offset;
                root->stackUsed += sym->arraySize*2;
            }
        }

        offset = 0;

        generateAssembly(root->childList[5]);

        fprintf(assembly, "%s:\n", returnLabel.c_str());
        returnLabel = "";

        if(root->childList[5]->stackUsed > 0)
            fprintf(assembly, "\tADD SP, %d\n", root->childList[5]->stackUsed);
        fprintf(assembly, "\tPOP BP\n");
        fprintf(assembly, "\tRET %d\n", root->stackUsed);
        fprintf(assembly, "%s ENDP\n", root->childList[1]->getName().c_str());
     
        localVars.clear();
        offset = 0;
    
    }

    if(root->allChildren == "type_specifier ID LPAREN RPAREN compound_statement") {
        if(root->childList[1]->getName() == "main") {
            mainProcStart();
            InMain = true;
            root->childList[4]->IsMain = true;
        }
        else {
            fprintf(assembly, "%s PROC\n", root->childList[1]->getName().c_str());
            fprintf(assembly, "\tPUSH BP\n");
            fprintf(assembly, "\tMOV BP, SP\n");
        }

        generateAssembly(root->childList[4]);

        fprintf(assembly, "%s:\n", returnLabel.c_str());
        returnLabel = "";

        root->stackUsed = root->childList[4]->stackUsed;

        fprintf(assembly, "%s:\n", createLabel().c_str());   
        if(root->childList[4]->stackUsed > 0)
            fprintf(assembly, "\tADD SP, %d\n", root->childList[4]->stackUsed);
        if(root->childList[1]->getName() == "main") {
            mainProcEnd();
            InMain = false;
        }
        else {
            fprintf(assembly, "\tPOP BP\n");
            fprintf(assembly, "\tRET\n");
            fprintf(assembly, "%s ENDP\n", root->childList[1]->getName().c_str());
        }

        localVars.clear();
        offset = 0;
        
    }

    if(root->allChildren == "parameter_list COMMA type_specifier ID") {
        generateAssembly(root->childList[0]);
    }

    if(root->allChildren == "type_specifier ID") {

    }

    if(root->allChildren == "LCURL statements RCURL") {
        root->childList[1]->IsCondition = root->IsCondition;
        root->childList[1]->IsMain = root->IsMain;

        generateAssembly(root->childList[1]);
        root->stackUsed = root->childList[1]->stackUsed;

        root->nextLabel = root->childList[1]->nextLabel;
        root->trueLabel = root->childList[1]->trueLabel;
        root->falseLabel = root->childList[1]->falseLabel;
    }

    if(root->allChildren == "type_specifier declaration_list SEMICOLON") {
        generateAssembly(root->childList[1]);
        SymbolInfo* decList = root->childList[1];
        for (SymbolInfo* sym : decList->argList) {
            if (sym->getType() != "ARRAY") {
                offset += 2;
                localVars[sym->getName()] = offset;
                fprintf(assembly, "\tSUB SP, 2\n");
                root->stackUsed += 2;
            }
            else {
                offset += 2*sym->arraySize;
                localVars[sym->getName()] = offset;
                fprintf(assembly, "\tSUB SP, %d\n", sym->arraySize*2);
                root->stackUsed += sym->arraySize*2;
            }
        }

        if(root->IsCondition) {
            fprintf(assembly, "\tJMP %s\n", root->nextLabel.c_str());
            fprintf(assembly, "%s:\n", root->falseLabel.c_str());
        }
        else {
            root->nextLabel = createLabel();
            fprintf(assembly, "%s:\n", root->nextLabel.c_str());
        }

    }

    if(root->allChildren == "declaration_list COMMA ID") {
        generateAssembly(root->childList[0]);
    }

    if(root->allChildren == "declaration_list COMMA ID LSQUARE CONST_INT RSQUARE") {
        generateAssembly(root->childList[0]);
    }

    if(root->getName() == "declaration_list" && root->allChildren == "ID") {

    }

    if(root->getName() == "declaration_list" && root->allChildren == "ID LSQUARE CONST_INT RSQUARE") {

    }

    if(root->allChildren == "statement") {
        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[0]->IsMain = root->IsMain;

        if(!root->IsCondition) 
            fprintf(assembly, "%s:\n", createLabel().c_str());

        generateAssembly(root->childList[0]);
        root->stackUsed = root->childList[0]->stackUsed;

        root->nextLabel = root->childList[0]->nextLabel;
        root->trueLabel = root->childList[0]->trueLabel;
        root->falseLabel = root->childList[0]->falseLabel;
    }

    if(root->allChildren == "statements statement") {
        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[0]->IsMain = root->IsMain;
        root->childList[1]->IsMain = root->IsMain;

        generateAssembly(root->childList[0]);
        root->stackUsed = root->childList[0]->stackUsed;

        generateAssembly(root->childList[1]);
        root->stackUsed += root->childList[1]->stackUsed;

        root->nextLabel = root->childList[1]->nextLabel;
        root->trueLabel = root->childList[1]->trueLabel;
        root->falseLabel = root->childList[1]->falseLabel;
    }   

    if(root->allChildren == "expression_statement") {
        root->childList[0]->IsCondition = root->IsCondition;

        generateAssembly(root->childList[0]);

        root->nextLabel = root->childList[0]->nextLabel;
        root->trueLabel = root->childList[0]->trueLabel;
        root->falseLabel = root->childList[0]->falseLabel;
    }

    if(root->allChildren == "compound_statement") {
        root->childList[0]->IsCondition = root->IsCondition;
        
        generateAssembly(root->childList[0]);

        root->nextLabel = root->childList[0]->nextLabel;
        root->trueLabel = root->childList[0]->trueLabel;
        root->falseLabel = root->childList[0]->falseLabel;
    }

    if(root->allChildren == "FOR LPAREN expression_statement expression_statement expression RPAREN statement") {
        root->childList[2]->IsCondition = true;
        root->childList[3]->IsCondition = true;

        generateAssembly(root->childList[2]);

        string tempLabel = "L" + to_string(labelNo);

        generateAssembly(root->childList[3]);

        root->nextLabel = root->childList[3]->nextLabel;
        root->trueLabel = root->childList[3]->trueLabel;
        root->falseLabel = root->childList[3]->falseLabel;



        fprintf(assembly, "\tCMP AX, 0\n");
        fprintf(assembly, "\tJNE %s\n", root->childList[3]->trueLabel.c_str());
        fprintf(assembly, "\tJMP %s\n", root->childList[3]->falseLabel.c_str());


        fprintf(assembly, "%s:\n", root->childList[3]->trueLabel.c_str());
        
        generateAssembly(root->childList[6]);

        generateAssembly(root->childList[4]);


        fprintf(assembly, "\tJMP %s\n", tempLabel.c_str());

        fprintf(assembly, "%s:\n", root->childList[3]->falseLabel.c_str());


        root->nextLabel = root->childList[6]->nextLabel;
    }

    if(root->allChildren == "IF LPAREN expression RPAREN statement") {
        root->childList[2]->IsCondition = true;
        
        generateAssembly(root->childList[2]);

        root->nextLabel = root->childList[2]->nextLabel;
        root->trueLabel = root->childList[2]->trueLabel;
        root->falseLabel = root->childList[2]->falseLabel;


        fprintf(assembly, "\tCMP AX, 0\n");
        fprintf(assembly, "\tJNE %s\n", root->childList[2]->trueLabel.c_str());
        fprintf(assembly, "\tJMP %s\n", root->childList[2]->falseLabel.c_str());
        

        fprintf(assembly, "%s:\n", root->childList[2]->trueLabel.c_str());

        generateAssembly(root->childList[4]);

        fprintf(assembly, "\tJMP %s\n", root->childList[2]->nextLabel.c_str());
        fprintf(assembly, "%s:\n", root->childList[2]->falseLabel.c_str());

        root->nextLabel = root->childList[4]->nextLabel;

        fprintf(assembly, "%s:\n", root->childList[2]->nextLabel.c_str());
    }

    if(root->allChildren == "IF LPAREN expression RPAREN statement ELSE statement") {
        root->childList[2]->IsCondition = true;

        generateAssembly(root->childList[2]);

        fprintf(assembly, "\tCMP AX, 0\n");
        fprintf(assembly, "\tJNE %s\n", root->childList[2]->trueLabel.c_str());
        fprintf(assembly, "\tJMP %s\n", root->childList[2]->falseLabel.c_str());



        fprintf(assembly, "%s:\n", root->childList[2]->trueLabel.c_str());

        generateAssembly(root->childList[4]);

        fprintf(assembly, "\tJMP %s\n", root->childList[2]->nextLabel.c_str());
        fprintf(assembly, "%s:\n", root->childList[2]->falseLabel.c_str());

        generateAssembly(root->childList[6]);

        fprintf(assembly, "\tJMP %s\n", root->childList[2]->nextLabel.c_str());
        
        fprintf(assembly, "%s:\n", root->childList[2]->nextLabel.c_str());
        
        root->nextLabel = root->childList[6]->nextLabel;
    }

    if(root->allChildren == "WHILE LPAREN expression RPAREN statement") {
        root->childList[2]->IsCondition = true;
        root->childList[2]->IsWhileCond = true;
        
        string tempLabel = "L" + to_string(labelNo);


        generateAssembly(root->childList[2]);


        fprintf(assembly, "\tCMP AX, 0\n");
        fprintf(assembly, "\tJNE %s\n", root->childList[2]->trueLabel.c_str());
        fprintf(assembly, "\tJMP %s\n", root->childList[2]->falseLabel.c_str());


        fprintf(assembly, "%s:\n", root->childList[2]->trueLabel.c_str());

        generateAssembly(root->childList[4]);


        fprintf(assembly, "\tJMP %s\n", tempLabel.c_str());

        fprintf(assembly, "%s:\n", root->childList[2]->falseLabel.c_str());

        root->nextLabel = root->childList[4]->nextLabel;
    }

    if(root->allChildren == "PRINTLN LPAREN ID RPAREN SEMICOLON") {
        string name = root->childList[2]->getName();
        if( localVars.find(name) == localVars.end() )
            fprintf(assembly, "\tMOV AX, %s         ; Line %d\n", name.c_str(), root->startLine);
        else {
            if(localVars[name] > 0)
                fprintf(assembly, "\tMOV AX, [BP-%d]         ; Line %d\n", localVars[name], root->startLine);
            else
                fprintf(assembly, "\tMOV AX, [BP+%d]         ; Line %d\n", -localVars[name], root->startLine);
        }
            

        fprintf(assembly, "\tCALL print_output\n");
        fprintf(assembly, "\tCALL new_line\n");

       
        root->nextLabel = createLabel();
        fprintf(assembly, "%s:\n", root->nextLabel.c_str());
   
    }

    if(root->allChildren == "RETURN expression SEMICOLON") {
        root->childList[0]->IsCondition = root->IsCondition;
        generateAssembly(root->childList[1]);

        root->nextLabel = createLabel();
        returnLabel = createLabel();
        fprintf(assembly, "\tJMP %s\n", returnLabel.c_str());
        fprintf(assembly, "%s:\n", root->nextLabel.c_str());
    }

    if(root->allChildren == "expression SEMICOLON") {
        root->childList[0]->IsCondition = root->IsCondition;

        generateAssembly(root->childList[0]);

        root->nextLabel = root->childList[0]->nextLabel;
        root->trueLabel = root->childList[0]->trueLabel;
        root->falseLabel = root->childList[0]->falseLabel;
    }

    if(root->getName() == "variable" && root->allChildren == "ID") {
        string varName = root->childList[0]->getName();

        if( localVars.find(varName) == localVars.end() ) 
            fprintf(assembly, "\tMOV AX, %s         ; Line %d\n", root->childList[0]->getName().c_str(), root->startLine);
        else {
            if(localVars[varName] > 0) 
                fprintf(assembly, "\tMOV AX, [BP-%d]         ; Line %d\n", localVars[varName], root->startLine);
            else 
                fprintf(assembly, "\tMOV AX, [BP+%d]         ; Line %d\n", -localVars[varName], root->startLine);
        } 

        if(root->IsCondition) {
            string finalTrue = createLabel();
            string finalFalse = createLabel();
            string finalNext = createLabel();

            root->nextLabel = finalNext;
            root->trueLabel = finalTrue;
            root->falseLabel = finalFalse;
        }
 
    }

    if(root->allChildren == "ID LSQUARE expression RSQUARE") {

        generateAssembly(root->childList[2]);

        root->arrayFlag = true;

        string varName = root->childList[0]->getName();
        if( localVars.find(varName) == localVars.end() ) {
            fprintf(assembly, "\tMOV BX, AX\n");
            fprintf(assembly, "\tMOV AX, 2\n");
            fprintf(assembly, "\tMUL BX\n");
            fprintf(assembly, "\tMOV BX, AX\n");
            fprintf(assembly, "\tMOV AX, %s[BX]         ; Line %d\n", root->childList[0]->getName().c_str(), root->startLine);
        }
        else {
            fprintf(assembly, "\tMOV BX, AX\n");
            fprintf(assembly, "\tMOV AX, 2\n");
            fprintf(assembly, "\tMUL BX\n");
            fprintf(assembly, "\tMOV BX, AX\n");
            fprintf(assembly, "\tMOV AX, %d\n", localVars[varName]);
            fprintf(assembly, "\tSUB AX, BX\n");
            fprintf(assembly, "\tMOV BX, AX\n");
            fprintf(assembly, "\tMOV SI, BX\n");
            fprintf(assembly, "\tNEG SI\n");

            fprintf(assembly, "\tMOV AX, [BP+SI]         ; Line %d\n", root->startLine);
        }

        if(root->IsCondition) {
            string finalTrue = createLabel();
            string finalFalse = createLabel();
            string finalNext = createLabel();

            root->nextLabel = finalNext;
            root->trueLabel = finalTrue;
            root->falseLabel = finalFalse;
        }


    }

    if(root->getName() == "expression" && root->allChildren == "logic_expression") {
        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[0]->IsWhileCond = root->IsWhileCond;

        generateAssembly(root->childList[0]);

        root->nextLabel = root->childList[0]->nextLabel;
        root->trueLabel = root->childList[0]->trueLabel;
        root->falseLabel = root->childList[0]->falseLabel;

        root->arrayFlag = root->childList[0]->arrayFlag;
    }

    if(root->allChildren == "variable ASSIGNOP logic_expression") {
        root->childList[2]->IsCondition = root->IsCondition;

        generateAssembly(root->childList[0]);

        string assignVar = root->childList[0]->childList[0]->getName();

        if(root->childList[0]->arrayFlag) {
            if( localVars.find(assignVar) == localVars.end() ) {
                fprintf(assembly, "\tPUSH BX\n");
            }
            else {
                fprintf(assembly, "\tPUSH SI\n");
            }
        }

        generateAssembly(root->childList[2]);

        if(root->childList[0]->arrayFlag)
            fprintf(assembly, "\tPOP BX\n");
    
        if( localVars.find(assignVar) == localVars.end() ) {
            if( root->childList[0]->arrayFlag ) {
                fprintf(assembly, "\tMOV %s[BX], AX\n", assignVar.c_str());
            }
            else
                fprintf(assembly, "\tMOV %s, AX\n", assignVar.c_str());
        }
        else {
            if( root->childList[0]->arrayFlag ) {
                fprintf(assembly, "\tMOV SI, BX\n");
                fprintf(assembly, "\tMOV [BP+SI], AX\n");
            }
            else {
                if(localVars[assignVar] > 0)
                    fprintf(assembly, "\tMOV [BP-%d], AX\n", localVars[assignVar]);
                else
                    fprintf(assembly, "\tMOV [BP+%d], AX\n", -localVars[assignVar]);
            }
            
        }        
        

        root->nextLabel = createLabel();
        fprintf(assembly, "%s:\n", root->nextLabel.c_str());
        
    }

    if(root->allChildren == "rel_expression") {
        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[0]->IsWhileCond = root->IsWhileCond;

        generateAssembly(root->childList[0]);

        if(root->childList[0]->IsCondition) {
            root->trueLabel = root->childList[0]->trueLabel;
            root->falseLabel = root->childList[0]->falseLabel;
        }

        root->nextLabel = root->childList[0]->nextLabel;

        root->arrayFlag = root->childList[0]->arrayFlag;
    }

    if(root->allChildren == "rel_expression LOGICOP rel_expression") {

        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[2]->IsCondition = root->IsCondition;

        generateAssembly(root->childList[0]);

        string midLabel = createLabel();
        string finalTrue = createLabel();
        string finalFalse = createLabel();
        string labelNext = createLabel();

        fprintf(assembly, "\tCMP AX, 0\n");

        if(root->childList[1]->getName() == "||") {
            fprintf(assembly, "\tJNE %s\n", finalTrue.c_str());
            fprintf(assembly, "\tJMP %s\n", midLabel.c_str());
        }
        else if(root->childList[1]->getName() == "&&") {
            fprintf(assembly, "\tJNE %s\n", midLabel.c_str());
            fprintf(assembly, "\tJMP %s\n", finalFalse.c_str());
        }


        fprintf(assembly, "%s:\n", midLabel.c_str());

        generateAssembly(root->childList[2]);

        fprintf(assembly, "\tCMP AX, 0\n");
        fprintf(assembly, "\tJNE %s\n", finalTrue.c_str());
        fprintf(assembly, "\tJMP %s\n", finalFalse.c_str());

        fprintf(assembly, "%s:\n", finalTrue.c_str());
        fprintf(assembly, "\tMOV AX, 1         ; Line %d\n", root->startLine);
        fprintf(assembly, "\tJMP %s\n", labelNext.c_str());

        fprintf(assembly, "%s:\n", finalFalse.c_str());
        fprintf(assembly, "\tMOV AX, 0         ; Line %d\n", root->startLine);

        fprintf(assembly, "%s:\n", labelNext.c_str());

        root->trueLabel = root->childList[2]->trueLabel;
        root->falseLabel = root->childList[2]->falseLabel;
        root->nextLabel = root->childList[2]->nextLabel;

    }

    if(root->allChildren == "simple_expression") {
        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[0]->IsWhileCond = root->IsWhileCond;

        generateAssembly(root->childList[0]);

        root->trueLabel = root->childList[0]->trueLabel;
        root->falseLabel = root->childList[0]->falseLabel;
        root->nextLabel = root->childList[0]->nextLabel;

        root->arrayFlag = root->childList[0]->arrayFlag;
    }

    if(root->allChildren == "simple_expression RELOP simple_expression") {
        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[2]->IsCondition = root->IsCondition;

        generateAssembly(root->childList[2]);

        fprintf(assembly, "\tPUSH AX\n");

        generateAssembly(root->childList[0]);

        fprintf(assembly, "\tPOP CX\n");
        fprintf(assembly, "\tCMP AX, CX\n");

        string labelTrue = createLabel();
        string labelFalse = createLabel();
        string labelNext = createLabel();
        

        if(root->childList[1]->getName() == "<=") {
            fprintf(assembly, "\tJLE %s\n", labelTrue.c_str());
        }
        else if(root->childList[1]->getName() == "<") {
            fprintf(assembly, "\tJL %s\n", labelTrue.c_str());
        }
        else if(root->childList[1]->getName() == ">") {
            fprintf(assembly, "\tJG %s\n", labelTrue.c_str());
        }
        else if(root->childList[1]->getName() == ">=") {
            fprintf(assembly, "\tJGE %s\n", labelTrue.c_str());
        }
        else if(root->childList[1]->getName() == "==") {
            fprintf(assembly, "\tJE %s\n", labelTrue.c_str());
        }
        else if(root->childList[1]->getName() == "!=") {
            fprintf(assembly, "\tJNE %s\n", labelTrue.c_str());
        }


        fprintf(assembly, "\tJMP %s\n", labelFalse.c_str());

        fprintf(assembly, "%s:\n", labelTrue.c_str());
        fprintf(assembly, "\tMOV AX, 1         ; Line %d\n", root->startLine);
        fprintf(assembly, "\tJMP %s\n", labelNext.c_str());

        fprintf(assembly, "%s:\n", labelFalse.c_str());
        fprintf(assembly, "\tMOV AX, 0         ; Line %d\n", root->startLine);

        fprintf(assembly, "%s:\n", labelNext.c_str());


        if(root->IsCondition == true) {
            string finalTrue = createLabel();
            string finalFalse = createLabel();
            string finalNext = createLabel();

            root->nextLabel = finalNext;
            root->trueLabel = finalTrue;
            root->falseLabel = finalFalse;
        }

    }

    if(root->allChildren == "term") {
        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[0]->IsWhileCond = root->IsWhileCond;

        generateAssembly(root->childList[0]);

        root->trueLabel = root->childList[0]->trueLabel;
        root->falseLabel = root->childList[0]->falseLabel;
        root->nextLabel = root->childList[0]->nextLabel;

        root->arrayFlag = root->childList[0]->arrayFlag;
    }

    if(root->allChildren == "simple_expression ADDOP term") {
        root->childList[0]->IsCondition = root->IsCondition;

        generateAssembly(root->childList[2]);

        fprintf(assembly, "\tPUSH AX\n");      

        generateAssembly(root->childList[0]);

        fprintf(assembly, "\tPOP CX\n");
        if(root->childList[1]->getName() == "+")
            fprintf(assembly, "\tADD AX, CX\n");
        else
            fprintf(assembly, "\tSUB AX, CX\n");

        root->nextLabel = createLabel();
        fprintf(assembly, "%s:\n", root->nextLabel.c_str());
    }   

    if(root->allChildren == "unary_expression") {
        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[0]->IsWhileCond = root->IsWhileCond;

        generateAssembly(root->childList[0]);

        root->trueLabel = root->childList[0]->trueLabel;
        root->falseLabel = root->childList[0]->falseLabel;
        root->nextLabel = root->childList[0]->nextLabel;

        root->arrayFlag = root->childList[0]->arrayFlag;
    }

    if(root->allChildren == "term MULOP unary_expression") {
        generateAssembly(root->childList[2]);

        fprintf(assembly, "\tPUSH AX\n");

        generateAssembly(root->childList[0]);
        
        fprintf(assembly, "\tPOP CX\n");
        fprintf(assembly, "\tCWD\n");

        if(root->childList[1]->getName() == "*")
            fprintf(assembly, "\tMUL CX\n");
        else {
            fprintf(assembly, "\tDIV CX\n");
        }

        if(root->childList[1]->getName() == "%") 
            fprintf(assembly, "\tMOV AX, DX\n");
        
        root->nextLabel = createLabel();
        fprintf(assembly, "%s:\n", root->nextLabel.c_str());

    }

    if(root->allChildren == "ADDOP unary_expression") {
        generateAssembly(root->childList[1]);
        if(root->childList[0]->getName() == "-") {
            fprintf(assembly, "\tNEG AX\n");
        }

        root->nextLabel = createLabel();
        fprintf(assembly, "%s:\n", root->nextLabel.c_str());
    }

    if(root->allChildren == "NOT unary_expression") {
        root->childList[1]->IsCondition = root->IsCondition;

        generateAssembly(root->childList[1]);

        string finalTrue = createLabel();
        string finalFalse = createLabel();
        string labelNext = createLabel();


        fprintf(assembly, "\tCMP AX, 0\n");
        fprintf(assembly, "\tJE %s\n", finalTrue.c_str());
        fprintf(assembly, "\tJMP %s\n", finalFalse.c_str());

        fprintf(assembly, "%s:\n", finalTrue.c_str());
        fprintf(assembly, "\tMOV AX, 1         ; Line %d\n", root->startLine);
        fprintf(assembly, "\tJMP %s\n", labelNext.c_str());

        fprintf(assembly, "%s:\n", finalFalse.c_str());
        fprintf(assembly, "\tMOV AX, 0         ; Line %d\n", root->startLine);

        fprintf(assembly, "%s:\n", labelNext.c_str());

        root->trueLabel = root->childList[1]->trueLabel;
        root->falseLabel = root->childList[1]->falseLabel;
        root->nextLabel = root->childList[1]->nextLabel;

    }

    if(root->allChildren == "factor") {
        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[0]->IsWhileCond = root->IsWhileCond;

        generateAssembly(root->childList[0]);

        root->trueLabel = root->childList[0]->trueLabel;
        root->falseLabel = root->childList[0]->falseLabel;
        root->nextLabel = root->childList[0]->nextLabel;
    }

    if(root->allChildren == "variable") {
        root->childList[0]->IsCondition = root->IsCondition;
        generateAssembly(root->childList[0]);

        root->trueLabel = root->childList[0]->trueLabel;
        root->falseLabel = root->childList[0]->falseLabel;
        root->nextLabel = root->childList[0]->nextLabel;
        root->arrayFlag = root->childList[0]->arrayFlag;
    }

    if(root->allChildren == "ID LPAREN argument_list RPAREN") {
        generateAssembly(root->childList[2]);
        fprintf(assembly, "\tCALL %s\n", root->childList[0]->getName().c_str());
        
        root->nextLabel = createLabel();
        fprintf(assembly, "%s:\n", root->nextLabel.c_str());
    }

    if(root->allChildren == "LPAREN expression RPAREN") {
        root->childList[1]->IsCondition = root->IsCondition;
        generateAssembly(root->childList[1]);

        root->trueLabel = root->childList[1]->trueLabel;
        root->falseLabel = root->childList[1]->falseLabel;
        root->nextLabel = root->childList[1]->nextLabel;
    }

    if(root->allChildren == "CONST_INT") {
        fprintf(assembly, "\tMOV AX, %s         ; Line %d\n", root->childList[0]->getName().c_str(), root->startLine);
    }

    if(root->allChildren == "variable INCOP") {

        root->childList[0]->IsCondition = root->IsCondition;
        root->childList[0]->IsWhileCond = root->IsWhileCond;

        generateAssembly(root->childList[0]);

        string name = root->childList[0]->childList[0]->getName();

        
        fprintf(assembly, "\tPUSH AX\n");
        fprintf(assembly, "\tINC AX\n");



        if( localVars.find(name) == localVars.end() ) {
            if( root->childList[0]->arrayFlag ) {
                fprintf(assembly, "\tMOV %s[BX], AX\n", name.c_str());
            }
            else
                fprintf(assembly, "\tMOV %s, AX\n", name.c_str());
        }
        else {
            if( root->childList[0]->arrayFlag ) {
                fprintf(assembly, "\tMOV [BP+SI], AX\n");
            }
            else {
                if(localVars[name] > 0)
                    fprintf(assembly, "\tMOV [BP-%d], AX\n", localVars[name]);
                else
                    fprintf(assembly, "\tMOV [BP+%d], AX\n", -localVars[name]);
            }
            
        }

        fprintf(assembly, "\tPOP AX\n");

        if(root->IsCondition) {
            root->trueLabel = createLabel();
            root->falseLabel = createLabel();
            root->nextLabel = createLabel();
        }
        
    }

    if(root->allChildren == "variable DECOP") {

        string name = root->childList[0]->childList[0]->getName();

        generateAssembly(root->childList[0]);
        
        fprintf(assembly, "\tPUSH AX\n");
        fprintf(assembly, "\tDEC AX\n");


        if( localVars.find(name) == localVars.end() ) {
            if( root->childList[0]->arrayFlag ) {
                fprintf(assembly, "\tMOV %s[BX], AX\n", name.c_str());
            }
            else
                fprintf(assembly, "\tMOV %s, AX\n", name.c_str());
        }
        else {
            if( root->childList[0]->arrayFlag ) {
                fprintf(assembly, "\tMOV [BP+SI], AX\n");
            }
            else {
                if(localVars[name] > 0)
                    fprintf(assembly, "\tMOV [BP-%d], AX\n", localVars[name]);
                else
                    fprintf(assembly, "\tMOV [BP+%d], AX\n", -localVars[name]);
            }
            
        }

        fprintf(assembly, "\tPOP AX\n");

        if(root->IsCondition) {
            root->trueLabel = createLabel();
            root->falseLabel = createLabel();
            root->nextLabel = createLabel();
        } 
    }

    if(root->allChildren == "arguments") {
        generateAssembly(root->childList[0]);

        root->nextLabel = root->childList[0]->nextLabel;
    }

    if(root->allChildren == "arguments COMMA logic_expression") {
        generateAssembly(root->childList[0]);
        generateAssembly(root->childList[2]);

        fprintf(assembly, "\tPUSH AX\n");

        root->nextLabel = root->childList[2]->nextLabel;
    }

    if(root->getName() == "arguments" && root->allChildren == "logic_expression") {
        generateAssembly(root->childList[0]);

        fprintf(assembly, "\tPUSH AX\n");

        root->nextLabel = root->childList[0]->nextLabel;
    }






}












void printLibrary() {
    fprintf(assembly, "new_line proc\n");
    fprintf(assembly, "\tpush ax\n");
    fprintf(assembly, "\tpush dx\n");
    fprintf(assembly, "\tmov ah,2\n");
    fprintf(assembly, "\tmov dl,0Dh\n");
    fprintf(assembly, "\tint 21h\n");
    fprintf(assembly, "\tmov ah,2\n");
    fprintf(assembly, "\tmov dl,0Ah\n");
    fprintf(assembly, "\tint 21h\n");
    fprintf(assembly, "\tpop dx\n");
    fprintf(assembly, "\tpop ax\n");
    fprintf(assembly, "\tret\n");
    fprintf(assembly, "\tnew_line endp\n");
    fprintf(assembly, "print_output proc\n");
    fprintf(assembly, "\tpush ax\n");
    fprintf(assembly, "\tpush bx\n");
    fprintf(assembly, "\tpush cx\n");
    fprintf(assembly, "\tpush dx\n");
    fprintf(assembly, "\tpush si\n");
    fprintf(assembly, "\tlea si,number\n");
    fprintf(assembly, "\tmov bx,10\n");
    fprintf(assembly, "\tadd si,4\n");
    fprintf(assembly, "\tcmp ax,0\n");
    fprintf(assembly, "\tjnge negate\n");
    fprintf(assembly, "\tprint:\n");
    fprintf(assembly, "\txor dx,dx\n");
    fprintf(assembly, "\tdiv bx\n");
    fprintf(assembly, "\tmov [si],dl\n");
    fprintf(assembly, "\tadd [si],'0'\n");
    fprintf(assembly, "\tdec si\n");
    fprintf(assembly, "\tcmp ax,0\n");
    fprintf(assembly, "\tjne print\n");
    fprintf(assembly, "\tinc si\n");
    fprintf(assembly, "\tlea dx,si\n");
    fprintf(assembly, "\tmov ah,9\n");
    fprintf(assembly, "\tint 21h\n");
    fprintf(assembly, "\tpop si\n");
    fprintf(assembly, "\tpop dx\n");
    fprintf(assembly, "\tpop cx\n");
    fprintf(assembly, "\tpop bx\n");
    fprintf(assembly, "\tpop ax\n");
    fprintf(assembly, "\tret\n");
    fprintf(assembly, "\tnegate:\n");
    fprintf(assembly, "\tpush ax\n");
    fprintf(assembly, "\tmov ah,2\n");
    fprintf(assembly, "\tmov dl,'-'\n");
    fprintf(assembly, "\tint 21h\n");
    fprintf(assembly, "\tpop ax\n");
    fprintf(assembly, "\tneg ax\n");
    fprintf(assembly, "\tjmp print\n");
    fprintf(assembly, "\tprint_output endp\n"); 
}








void optimizer() {
    ifstream input("2005084_assembly.asm");
    // ifstream input("temp.asm");
    ofstream output("optimized.asm");
    // ofstream temp("temp.asm");

    vector<string> allLines;
    vector<string> finalLines;
    vector< vector<string> > words;

    string line;
    int i = 0;
    while( getline(input, line) ) {
        allLines.push_back(line);

        vector<string> lineWords;
        string token;
        stringstream lineStream(line);

        while( getline(lineStream, token, ' ') ) {
            if(token == "")
                continue;
            while (!token.empty() && token.back() == '\r') {
                token.pop_back(); 
            }
            lineWords.push_back(token);
        }

        words.push_back(lineWords);
        lineWords.clear();
        
    }

    int flag = 0;

    for(int i = 0; i < words.size()-1; i++) {
        
        if(flag <= 0)
            flag = 0;
        else {
            flag--;
            continue;
        }
        
        string str1, str2;

        if(words[i].size() >= 3 && words[i+1].size() >= 3) {
            str1 = words[i][1];
            str1.pop_back();
            str2 = words[i+1][1];
            str2.pop_back();
        }
        
        /* 
            MOV AX, a
            MOV a, AX
        */

        if( words[i].size() >= 3 && words[i+1].size() >= 3
            && words[i][0] == "\tMOV" && words[i+1][0] == "\tMOV"
            && str1 == words[i+1][2] && words[i][2] == str2 ) {
            str1 = "";
            str2 = "";
            flag = 1;
            finalLines.push_back(allLines[i]);
            continue;
        }


        /* 
            MOV AX, 2
            MOV AX, 1
        */

        if( words[i].size() >= 2 && words[i+1].size() >= 2
            && words[i][0] == "\tMOV" && words[i+1][0] == "\tMOV"
            && words[i][1] == words[i+1][1] ) {
            continue;
        }


        /* 
            PUSH AX
            POP AX
        */

        if( words[i].size() >= 2 && words[i+1].size() >= 2
            && words[i][0] == "\tPUSH" && words[i+1][0] == "\tPOP"
            && words[i][1] == words[i+1][1] ) {
            flag = 1;
            continue;
        }


        // ADD AX, 0

        if( words[i].size() >= 3
            && words[i][0] == "\tADD" && words[i][2] == "0" ) {
            continue;
        }


        // MUL 1

        if( words[i].size() >= 2
            && words[i][0] == "\tMUL" && words[i][1] == "1" ) {
            continue;
        }



        // conditional jump optimization

        if( allLines[i] == "\tCMP AX, CX") {
            string st = words[i+3][0];
            st.pop_back();

            if( st == words[i+1][1] 
                && words[i+4][0] == "\tMOV" && words[i+4][1] == "AX," && words[i+4][2] == "1"
                && words[i+7][0] == "\tMOV" && words[i+7][1] == "AX," && words[i+7][2] == "0" 
                && allLines[i+9] == "\tCMP AX, 0"
            ) {
                finalLines.push_back(allLines[i]);
                string newLine = words[i+1][0] + " " + words[i+10][1];
                finalLines.push_back(newLine);
                finalLines.push_back(allLines[i+11]);
                flag = 11;
                continue;
            }

        }






        finalLines.push_back(allLines[i]);
        
    }

    finalLines.push_back(allLines[allLines.size()-1]);




    for(string line : finalLines) {
        output<<line<<endl;
    }


}