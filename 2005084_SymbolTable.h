#include<iostream>
#include<fstream>
#include <sstream>
#include <vector>
using namespace std;

class SymbolInfo {
private:
     string name;
     string type;
      
public:
     int startLine, endLine;
     bool IsLeaf;
     string returnType;
     int arraySize;
     int stackUsed;
     string arrayType;
     vector<SymbolInfo*> argList;
     vector<SymbolInfo*> childList;
     SymbolInfo* next;
     string allChildren;
     bool IsCondition;
     bool IsWhileCond;
     bool IsMain;
     bool arrayFlag;
     string dataType;
     SymbolInfo* parent;
     int scope;
     string nextLabel, trueLabel, falseLabel;

     SymbolInfo() {
          next = NULL;
          returnType = "Invalid";
          arraySize = -1;
          IsLeaf = false;
          stackUsed = 0;
          parent = NULL;
          IsCondition = false;
          IsWhileCond = false;
          nextLabel = "#";
          trueLabel = "#"; 
          falseLabel = "#";
          IsMain = false;
          arrayFlag = false;
     }

     SymbolInfo(string name, string type) {
          this->name = name;
          this->type = type;
          returnType = "Invalid";
          arraySize = -1;
          next = NULL;
          IsLeaf = false;
          stackUsed = 0;
          parent = NULL;
          IsCondition = false;
          IsWhileCond = false;
          nextLabel = "#";
          trueLabel = "#"; 
          falseLabel = "#";
          IsMain = false;
          arrayFlag = false;
     }

     ~SymbolInfo() {
          if(next != NULL)
               delete next;
     }

     string getName() { return name; }

     void setName(string name) { this->name = name; }

     string getType() { return type; }

     void setType(string type) { this->type = type; }

     void printParseTree(int spaceCount, FILE* parseFile) {

          for(int i = 0; i < spaceCount; i++) 
               fprintf(parseFile," ");
          
          if(IsLeaf) {
               fprintf(parseFile,"%s : %s	<Line: %d>\n",getType().c_str(), getName().c_str(), startLine);
          }
          else {
               fprintf(parseFile,"%s : %s 	<Line: %d-%d>\n",getName().c_str(), allChildren.c_str(), startLine, endLine);
               for(SymbolInfo* sym : childList) {
                    sym->printParseTree(spaceCount+1, parseFile);
               }
          }

     }

};



class ScopeTable {
public:
     int bucketSize;
     int id;
     int totalChild;
     SymbolInfo** hashTables;
     ScopeTable* parentScope;


     ScopeTable(int size) {
          bucketSize = size;
          hashTables = new SymbolInfo*[bucketSize];

          for(int i = 0; i < bucketSize; i++) {
               hashTables[i] = nullptr;
          }
          parentScope = NULL;
          totalChild = 0;
     }

     ~ScopeTable() {
          for(int i = 0; i < bucketSize; i++) {
               if(hashTables[i] != NULL)
                    delete hashTables[i];
          }
          delete[] hashTables;

          if(parentScope != NULL) 
               delete parentScope;
     }



     unsigned long long sdbm_hash(const char *str) {
         unsigned long long hash = 0;

         while (*str) {
             // equivalent to: hash = 65599 * hash + (*str++);
             hash = (*str++) + (hash << 6) + (hash << 16) - hash;
         }

         return hash%bucketSize;
     }

     SymbolInfo* find(string name) {
          const char* charPtr = name.c_str();
          unsigned long long hashVal = sdbm_hash(charPtr);
          SymbolInfo *temp = hashTables[hashVal];
          int counter = 1;
          while(temp != NULL) {
               if(temp->getName() == name) {
                    return temp;
               }
               counter++;
               temp = temp->next;
          }
          return NULL;
     }


     SymbolInfo* lookUp(string name) {
          SymbolInfo* res = find(name);
          if(res != NULL)
               return res;
          ScopeTable *temp = parentScope;
          while(temp != NULL) {
               res = temp->find(name);
               if(res != NULL)
                    return res;
               temp = temp->parentScope;
          }
          cout<<"\t'"<<name<<"' not found in any of the ScopeTables"<<endl;
          return NULL;
     }

     bool insert(SymbolInfo* s1) {
          const char* charPtr = s1->getName().c_str();
          unsigned long long hashVal = sdbm_hash(charPtr);
          bool flag = true;
          int counter = 1;
          string name = s1->getName();
          
          if(hashTables[hashVal] == NULL) {
               hashTables[hashVal] = s1;
               flag = true;
          }
          else {
               SymbolInfo *temp = hashTables[hashVal];

               if(temp->getName() == name && s1->returnType == "Invalid") {
                    flag = false;
               }
               counter++;
               if(flag) {
                    while(temp->next != NULL) {
                         if(temp->next->getName() == name && s1->returnType == "Invalid") {
                              flag = false;
                         }
                         temp = temp->next;
                         counter++;
                    }
                    if(flag)
                         temp->next = s1;
               }
          }

          return flag;
     }



     bool insert(string name, string type) {
          const char* charPtr = name.c_str();
          unsigned long long hashVal = sdbm_hash(charPtr);
          SymbolInfo *s1 = new SymbolInfo(name, type);
          bool flag = true;
          int counter = 1;
          if(hashTables[hashVal] == NULL) {
               hashTables[hashVal] = s1;
               flag = true;
          }
          else {
               SymbolInfo *temp = hashTables[hashVal];

               if(temp->getName() == name) {
                    flag = false;
               }
               counter++;
               if(flag) {
                    while(temp->next != NULL) {
                         if(temp->next->getName() == name) {
                              flag = false;
                         }
                         temp = temp->next;
                         counter++;
                    }
                    if(flag)
                         temp->next = s1;
               }
          }
          return flag;
     }



     bool Delete(string name) {
          const char* charPtr = name.c_str();
          unsigned long long hashVal = sdbm_hash(charPtr);
          SymbolInfo *temp = hashTables[hashVal];
          int counter = 0;
          bool flag = false;
          if(temp == NULL)
               flag = false;
          else {
               if(temp->getName() == name) {
                    hashTables[hashVal] = temp->next;
                    counter++;
                    flag = true;
               }
               else {
                    while(temp->next != NULL && !flag) {
                         if(temp->next->getName() == name) {
                              if(temp->next->next == NULL)
                                   temp->next = NULL;
                              else
                                   temp->next = temp->next->next;
                              flag = true;
                         }
                         counter++;
                         temp = temp->next;
                    }
               }
          }
          return flag;
     }

     void print(string fileName) {
          FILE *file = fopen(fileName.c_str(), "a");
          fprintf(file, "\tScopeTable# %d\n",id);
          for(int i = 0; i < bucketSize; i++) {
               if(hashTables[i] == NULL) {
               }
               else {
                    SymbolInfo* temp = hashTables[i];
                    fprintf(file, "\t%d",i+1);
                    while(temp != NULL) {
                         if(temp->returnType == "Invalid")
                              fprintf(file, "--> <%s,%s> ",temp->getName().c_str(), temp->dataType.c_str());
                         else 
                              fprintf(file, "--> <%s,%s,%s> ",temp->getName().c_str(), temp->getType().c_str(), temp->returnType.c_str());
                         temp = temp->next;
                    }
                    fprintf(file, "\n");
               }
          }
          fclose(file);
     }


};


class SymbolTable {
public:
     ScopeTable* current;
     int bucketSize;
     vector <SymbolInfo*> pendingArg;
     

     SymbolTable(int bucketSize) {
          this->bucketSize = bucketSize;
          current = new ScopeTable(bucketSize);
          current->id = 1;
     }

     ~SymbolTable() {
          if(current != NULL)
               delete current;
     }

     void createScope(int tableCount) {
          ScopeTable* st = new ScopeTable(bucketSize);
          current->totalChild++;
          st->id = tableCount+1;
          st->parentScope = current;
          current = st;
     }

     void exitScope() {
          if(current->id == 1) {
               return;
          }
          ScopeTable* temp = current->parentScope;
          current = temp;
     }

     bool insert(string name, string type) {
          return current->insert(name, type);
     }

     bool insert(SymbolInfo* s1) {
          return current->insert(s1);
     }

     SymbolInfo* lookUp(string name) {
          return current->lookUp(name);
     }

     SymbolInfo* find(string name) {
          return current->find(name);
     }

     bool remove(string name) {
          return current->Delete(name);
     }

     void printCurrent(string fileName) {
          current->print(fileName);
     }

     void printAll(string fileName) {
          ScopeTable* temp = current;
          while(temp != NULL) {
               temp->print(fileName);
               temp = temp->parentScope;
          }
     }

     int getCurrentID() {
          return current->id;
     }


};








