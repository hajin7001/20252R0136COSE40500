#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "node.h"

/* Node/Leaf labels produced by project3.y (edit if you changed grammar) */
#define T_DEFINE_HEADER "define_header"
#define T_FUNC_DEF      "func_def"
#define T_FUNC_ARG_DEC  "func_arg_dec"
#define T_DECL_LIST     "decl_list"
#define T_DECL_INIT     "decl_init"
#define T_ASSIGN_STMT   "assign_stmt"
#define T_AL_EXPR       "al_expr"
#define T_REL_EXPR      "rel_expr"
#define T_INC_EXPR      "inc_expr"
#define T_VARIABLE      "variable"
#define T_CLAUSE        "clause"
#define T_BODY          "body"
#define T_VALUE         "value"
#define T_NUMBER        "number"

#define P_ID        "ID:"
#define P_LBRACE    "LBRACE:"
#define P_RBRACE    "RBRACE:"
#define P_LBRACKET  "LBRACKET:"
#define P_RBRACKET  "RBRACKET:"
#define P_NUM       "NUM:"
#define P_NUM_BIN   "NUM_BIN:"
#define P_NUM_HEX   "NUM_HEX:"
#define P_INT       "INT:"
#define P_VOID      "VOID:"
#define P_FLOAT     "FLOAT:"

#define MAX_SCOPE_ERRORS 128
#define MAX_TYPE_ERRORS 128 

/* =====PROBLEM1===== */

/* Structure for a symbol table */
typedef struct SYMTAB {
    struct SYMTAB* parent;
    struct SYMTAB* child[16];   /* maximum child num = 16 */
    int num_child;              /* real child num */
    struct SYMBOL* entry[64];   /* maximum number of symbol table entries = 64 */
    int num_entry;              /* real entry num */

    /* Optional: use this cursor when aligning block scopes during analyses */
    int visit_i;
} SYMTAB;

/* Structure for a symbol table entry */
typedef struct SYMBOL {
    char name[64];  
    int  kind;      /* 0: func, 1: param, 2: var */
    int  type[8];   /* 0: void, 1: int, 2: float */
    int  num_type;  /* if symbol is param or var, num_type = 1. 
                       else if symbol is func, num_type can be larger than 1. (return type + args type) */    
} SYMBOL;

/* Generate a symbol table (Example implementation) */
static inline SYMTAB* NewSymTab(void) {
    SYMTAB* t = (SYMTAB*)malloc(sizeof(SYMTAB));
    t->parent = NULL;
    t->num_child = 0;
    t->num_entry = 0;
    t->visit_i = 0;
    for (int i=0;i<16;i++) t->child[i] = NULL;
    for (int i=0;i<64;i++) t->entry[i] = NULL;
    return t;
}

/* Add a symbol table to another symbol table as a child (Example) */
static inline void AddSymTab(SYMTAB* parent_symtab, SYMTAB* child_symtab) {
    parent_symtab->child[parent_symtab->num_child] = child_symtab;
    child_symtab->parent = parent_symtab;
    parent_symtab->num_child += 1;
}

static inline const char* getKindString(int kind) {
    switch (kind) {
        case 0: return "func";
        case 1: return "param";
        case 2: return "var";
        default: return "unknown";
    }
}

static inline const char* getTypeString(int typeCode) {
    switch (typeCode) {
        case 0: return "void";
        case 1: return "int";
        case 2: return "float";
        default: return "unknown";
    }
}

static inline void PrintSymTab(SYMTAB* symtab) {
    if (symtab == NULL || symtab->num_entry == 0) {
        return;
    }

    printf("%-10s | %-7s | %s\n", "name", "kind", "type");
    printf("-----------|---------|----------------------------------\n");

    for (int i = 0; i < symtab->num_entry; i++) {
        SYMBOL* s = symtab->entry[i];
        
        printf("%-10s | %-7s | ", s->name, getKindString(s->kind));

        if (s->kind == 0) { 
            
            printf("%s", getTypeString(s->type[0]));
            printf(" /");
            for (int j = 1; j < s->num_type; j++) {
                printf(" %s", getTypeString(s->type[j]));
            }
        } else { 
            printf("%s", getTypeString(s->type[0]));
        }
        printf("\n");
    }
    printf("-----------|---------|----------------------------------\n");

    for (int i = 0; i < symtab->num_child; i++) {
        PrintSymTab(symtab->child[i]);
    }
}

/* Generate a new element for symbol table */
static inline SYMBOL* NewSymbol(const char* name, int kind, int* type_list, int num_type) {
    SYMBOL *s = (SYMBOL*)malloc(sizeof(SYMBOL));
    strcpy(s->name, name);
    s->kind = kind; 
    for(int i=0; i<num_type; i++) {
        s->type[i] = type_list[i];
    }
    s->num_type = num_type;
    return s;
}

/* Insert an element to symbol table entry */
static inline void AddSymbol(SYMTAB* symtab, SYMBOL* symbol) {
    if(symtab->num_entry >= 64) {
        printf("SymbolTable is Full!");
        return;
    } 
    symtab->entry[symtab->num_entry] = symbol;
    symtab->num_entry += 1;
}

/* Find an element with corresponding name from symbol table, considering scope */
static inline SYMBOL* FindSymbol(SYMTAB* currentScope, const char* name) {
    SYMTAB* scope = currentScope;

    while (scope != NULL) {
        for (int i = 0; i < scope->num_entry; i++) {
            SYMBOL* sym = scope->entry[i];
            
            if (sym != NULL && strcmp(sym->name, name) == 0) {
                return sym;
            }
        }
        
        scope = scope->parent; 
    }
    return NULL; 
}

// 주어진 type node의 type을 가지고 오는 함수 
static inline int GetTypeCode(NODE* typeNode) {
    if (typeNode == NULL || typeNode->child == NULL) return -1; 
    if (strstr(typeNode->child->name, P_VOID)) return 0;
    if (strstr(typeNode->child->name, P_INT)) return 1;
    if (strstr(typeNode->child->name, P_FLOAT)) return 2; 
    return -1; 
}

static inline NODE* GetIdNodeFromVariable(NODE* varNode) {
    if (varNode == NULL) return NULL; 
    if (varNode->child && !strncmp(varNode->child->name, P_ID, strlen(P_ID))) {
        return varNode->child;
    } 
    else if (varNode->child && !strcmp(varNode->child->name, T_VARIABLE)) {
        return GetIdNodeFromVariable(varNode->child);
    }
    return NULL; 
}

static inline char* GetNameFromIdNode(NODE* idNode) {
    if (idNode == NULL) {
        fprintf(stderr, "Error: Tried to get name from NULL ID node.\n");
        return NULL; 
    }
    char* colon = strchr(idNode->name, ':');
    if (colon && *(colon + 1) == ' ') {
        return colon + 2; 
    }
    return idNode->name;
}

static inline int ProcessDeclarations(NODE* declListNode, SYMTAB* currentScope, int kind, int* typeArray, int* numTypes, bool updateFuncTypes) {
    // declListNode에서 타고 내려가면서 decl_init을 발견하면 그 아래에 있는 type-variable을 새로 더하고 
    // funcTypes와 numTypes를 update하기 

    // decl_list -> decl_init | decl_list COMMA variable | decl_list COMMA decl_init 
    if (declListNode == NULL || declListNode -> child == NULL) return -1; 

    NODE* child = declListNode -> child; 

    // base case 
    if (!strcmp(child->name, T_DECL_INIT)) {
        NODE* typeNode = child -> child; 
        NODE* varNode = typeNode -> next;

        int typeCode = GetTypeCode(typeNode); 
        NODE* idNode = GetIdNodeFromVariable(varNode);
        if (idNode == NULL) {
            fprintf(stderr, "Parse Error: Declaration missing variable name.\n");
            return -1; // 에러 
        }
        char* idName = GetNameFromIdNode(idNode);
        int symbolTypeArray[1] = { typeCode }; 

        AddSymbol(currentScope, NewSymbol(idName, kind, symbolTypeArray, 1));

        if (updateFuncTypes) {
            typeArray[*numTypes] = typeCode;
            (*numTypes)++;
        }

        return typeCode; 
    } 
    if (!strcmp(child->name, T_DECL_LIST)) {
        // decl_list -> decl_list COMMA variable | decl_list COMMA decl_init 에서 각각 앞의 decl_list에 해당하는 type을 baseType으로 
        int baseType = ProcessDeclarations(child, currentScope, kind, typeArray, numTypes, updateFuncTypes);
        int baseTypeArray[1] = { baseType };

        NODE* siblingNode = child -> next -> next; // variable 이나 decl_init 둘 중 하나 
        if (!strcmp(siblingNode->name, T_VARIABLE)) {
            // baseType을 가지고 variable을 처리 
            int typeCode = baseType; 
            NODE* idNode = GetIdNodeFromVariable(siblingNode);
            if (idNode == NULL) {
                fprintf(stderr, "Parse Error: Declaration missing variable name.\n");
                return -1; // 에러 
            }
            char* idName = GetNameFromIdNode(idNode);
            int symbolTypeArray[1] = { typeCode };

            AddSymbol(currentScope, NewSymbol(idName, kind, symbolTypeArray, 1));
            if (updateFuncTypes) {
                typeArray[*numTypes] = typeCode;
                (*numTypes)++;
            }

            return typeCode; 
        } else if (!strcmp(siblingNode->name, T_DECL_INIT)) {
            // baseType을 가지고 올 필요가 없다 
            NODE* typeNode = siblingNode -> child; 
            NODE* varNode = typeNode->next;

            int typeCode = GetTypeCode(typeNode); 
            NODE* idNode = GetIdNodeFromVariable(varNode);
            if (idNode == NULL) {
                fprintf(stderr, "Parse Error: Declaration missing variable name.\n");
                return -1; // 에러 
            }
            char* idName = GetNameFromIdNode(idNode);

            int symbolTypeArray[1] = { typeCode };

            AddSymbol(currentScope, NewSymbol(idName, kind, symbolTypeArray, 1));
            if (updateFuncTypes) {
                typeArray[*numTypes] = typeCode;
                (*numTypes)++;
            }
            return typeCode; 

        } 
    }
    return -1; 
}

/* Construct a symbol table tree using parse tree */
static inline void ConstructSymTab(SYMTAB* currentScope, NODE* head) {
    if (head == NULL) {
        return; // 현재 노드가 NULL이면 즉시 종료
    }
    //printf("[DEBUG] Visiting Node: %s\n", head->name);
    SYMTAB* nextScope = currentScope;  // 나중에 새로운 scope으로 이동할 경우 변경 여기서가 문제일 수 있나보다...

    if (!strcmp(head->name, T_DEFINE_HEADER)) {
        // define_header -> DEFINE ID number 
        NODE* idNode = head -> child -> next;
        char* name = GetNameFromIdNode(idNode);
        
        int typeArray[1] = {1};

        SYMBOL* newSymbol = NewSymbol(name, 2, typeArray, 1); // kind : 2 = var, type : 1 = int 
        AddSymbol(currentScope, newSymbol); 
    }
    else if (!strcmp(head->name, T_FUNC_DEF)) {
        // func_def -> type ID LPAREN func_arg_dec RPAREN LBRACE body_list RBRACE
        NODE* typeNode = head -> child;                     
        NODE* idNode = typeNode -> next;                     
        NODE* funcArgNode = idNode -> next -> next;  
        NODE* bodyListNode = funcArgNode -> next -> next -> next;
        
        char* funcName = GetNameFromIdNode(idNode);
        int funcTypeArray[8];
        int numTypes = 0; 
        funcTypeArray[numTypes++] = GetTypeCode(typeNode);

        // printf("FOR DEBUGGING2"); 
        if (funcArgNode -> child != NULL) {
            ProcessDeclarations(funcArgNode->child, currentScope, 1, 
                        funcTypeArray, &numTypes, true);
        } 
        AddSymbol(currentScope, NewSymbol(funcName, 0, funcTypeArray, numTypes));

        SYMTAB* funcBodyScope = NewSymTab();
        AddSymTab(currentScope, funcBodyScope);
        nextScope = funcBodyScope;
    } 

    else if (!strcmp(head->name, T_CLAUSE)) {
        // clause가 body_list를 지녔는지 확인을 하고, 지녔으면 새로운 table을 만들기 
        // 참고로 모든 LBRACE 뒤에는 body_list가 와! 
        // clause -> FOR LPAREN init_stmt test_expr SEMICOLON update_stmt RPAREN LBRACE body_list RBRACE | ... 
        bool createNewScope = false; 
        NODE* child = head -> child; 
        while (child != NULL) {
            //printf("child name in clause: %s\n", child->name);
            if (!strcmp(child->name, T_BODY)) {
                createNewScope = true;
                break;
            }
            child = child->next;
        }

        if (createNewScope) {
            SYMTAB* blockScope = NewSymTab();
            AddSymTab(currentScope, blockScope);
            nextScope = blockScope;
        }
    }

    else if (!strcmp(head->name, "statement")) {
        // statement -> assign_stmt SEMICOLON | continue_stmt SEMICOLON | decl_list SEMICOLON
        if (head->child && !strcmp(head->child->name, "decl_list")) {
            ProcessDeclarations(head->child, currentScope, 2, NULL, NULL, false);
        }
    }

    else if (!strcmp(head->name, "init_stmt")) {
        // init_stmt -> assign_stmt SEMICOLON | decl_list SEMICOLON
        if (head->child && !strcmp(head->child->name, "decl_list")) {
            ProcessDeclarations(head->child, currentScope, 2, NULL, NULL, false);
        }
    }

    else if (!strcmp(head->name, "update_stmt")) {
        // update_stmt -> inc_expr | decl_list 
        if (head->child && !strcmp(head->child->name, "decl_list")) {
            ProcessDeclarations(head->child, currentScope, 2, NULL, NULL, false);
        }
    }
    ConstructSymTab(nextScope, head->child);
    ConstructSymTab(currentScope, head->next);
}

/* =====PROBLEM2===== */
/*  Do scope-analysis for every variables in the code
    for detecting undefined variables */

static inline void ResetVisitCounters(SYMTAB* symtab) {
    if (symtab == NULL) return;
    symtab->visit_i = 0;
    for (int i = 0; i < symtab->num_child; i++) {
        ResetVisitCounters(symtab->child[i]);
    }
}

static inline void ScopeAnalysis(SYMTAB* currentScope, NODE* head, char* errorIdNames[], int* errorCount) {
    if (head == NULL)
        return; 
    SYMTAB* nextScope = currentScope;
    //  ID를 만날 수 있는 모든 Production Rule에 대한 고려를 하고 
    // 새로운 nextScope으로 넘어가게되는 시점을 생각해보자 
    if (!strcmp(head->name, T_FUNC_DEF)) {
        if (currentScope->visit_i < currentScope->num_child) {
            nextScope = currentScope->child[currentScope->visit_i];
            currentScope->visit_i++;
        }
        else {
            printf("ERROR: FUNC_DEF SYMBOL TABLE ACCESS FAILURE");
            return; 
        }
    }
    else if (!strcmp(head->name, T_CLAUSE)) {
        bool createNewScope = false; 
        NODE* child = head -> child; 

        while (child != NULL) {
            //printf("child name in clause: %s\n", child->name);
            if (!strcmp(child->name, T_BODY)) {
                createNewScope = true;
                break;
            }
            child = child->next;
        }

        if (createNewScope) {
            if (currentScope->visit_i < currentScope->num_child) {
                nextScope = currentScope->child[currentScope->visit_i];
                currentScope->visit_i++;
            } else {
                printf("ERROR: CLAUSE SYMBOL TABLE ACCESS FAILURE");
                return;
            }
        }
        
    }
    else if (!strcmp(head->name, T_VARIABLE)) {
        NODE* idNode = GetIdNodeFromVariable(head);
        if (idNode) {
            char* idName = GetNameFromIdNode(idNode);
            SYMBOL* foundSymbol = FindSymbol(currentScope, idName);
            if (foundSymbol == NULL && *errorCount < MAX_SCOPE_ERRORS) {
                bool exists = false; 
                // errorIdName을 순회하면서 Id가 저장된 적이 있는지 확인 - 이는 Id Name들은 서로 겹치지 않는다는 전제가 붙음 
                for (int i=0; i<*errorCount; i++) {
                    if (!strcmp(errorIdNames[i], idName)) {
                        exists = true; 
                    }
                }
                if (!exists) errorIdNames[(*errorCount)++] = idName;
                // printf("Symbol %s does not exist", idName);
            }
        }
    }
    ScopeAnalysis(nextScope, head->child, errorIdNames, errorCount);
    ScopeAnalysis(nextScope, head->next, errorIdNames, errorCount);
}

/* ======PROBLEM3===== */
/*  Do type-analysis for every arithmetic & logic expressions in the code
    for detecting type error
     1. array index should be integer
     2. float number cannot be stored in integer variable */

static const char *errorFormats[] = {
    "Type error: %s number cannot be stored in %s variable!",
    "Type error: void type cannot be added or multiplied",
    "Type error: %s and %s cannot be compared together",
    "Type error: cannot increment or decrement 'void' type",
    "Type error: array index is not an integer"
};

static inline int GetExprType(SYMTAB* currentScope, NODE* head) {
    if (head == NULL) return -1; 
    if (!strcmp(head->name, T_VARIABLE)) {
        NODE* idNode = GetIdNodeFromVariable(head);
        if (idNode) {
            char* idName = GetNameFromIdNode(idNode);
            SYMBOL* foundSymbol = FindSymbol(currentScope, idName); 
            if (foundSymbol == NULL) return -1; 
            return foundSymbol->type[0]; // function이 아닌 이상 type array는 0-entry만 차게 되어있어 
        }
    }
    else if (strstr(head->name, P_NUM) || strstr(head->name, P_NUM_BIN) || strstr(head->name, P_NUM_HEX)) {       // "NUM:"
        return 1; // int
    }
    else if (!strcmp(head->name, T_NUMBER)) {
        // number -> NUM | NUM_BIN | NUM_HEX이므로 어떤 경우든지 int이다 
        return 1; 
    }
    else if (!strcmp(head->name, T_VALUE)) {
        // value -> number | variable 
        return GetExprType(currentScope, head->child);
    }
    else if (!strcmp(head->name, T_AL_EXPR)) {
        // al_expr -> number | variable | al_expr OP_ADD al_expr | al_expr OP_MUL al_expr
        NODE* child = head->child; 
        // 첫번째 child가 al_expr로 시작하는 경우 
        if (child->next != NULL) {
            // 양변의 T_AL_EXPR에서 받아오고 그 중에서 더 높은 type으로 승격 (type casting)
            int type1 = GetExprType(currentScope, child); 
            int type2 = GetExprType(currentScope, child->next->next);
            // 사실 둘의 type이 맞지 않으면 에러가 뜰거지만 GetExprType에서는 일단 더 높은 type만 가져오고 error handling은 TypeAnalysis 가서 처리 
            if (type1 == 2 || type2 == 2) return 2; // 하나라도 float면 결과는 float
            if (type1 == 1 && type2 == 1) return 1; // 둘 다 int면 결과는 int
            return -1; // void 연산 등
        }
        // 첫번째 child가 al_expr로 시작하지 않는 경우, number or variable인 경우 
        else return GetExprType(currentScope, child);
    }
    else if (!strcmp(head->name, T_REL_EXPR)) {
        // rel_expr -> value | rel_expr OP_REL rel_expr | rel_expr OP_LOGIC rel_expr 
        // OP_REL : <=, >=, ==, <, > 
        // OP_LOGIC: &&, || 
        NODE* child = head->child;
        if(child->next != NULL) {
            int type1 = GetExprType(currentScope, child);
            int type2 = GetExprType(currentScope, child->next->next);
            if (type1 == 0 || type2 == 0) return -1;
            return 1; 
        }
        return 1; // value인 경우 이게 rel_expr으로 해석되는 것이므로 1로 반환 
    }
    else if (!strcmp(head->name, T_INC_EXPR)) {
        return GetExprType(currentScope, head->child);
    }
    return -1;  // 모르는 경우에 대해서는 일단 -1을 반환하기 
}
     
static inline void TypeAnalysis(SYMTAB* currentScope, NODE* head, char* errorIdNames[], int* errorCount) {
    if (head == NULL) return; 
    SYMTAB* nextScope = currentScope; 

    // nextScope을 정확히 설정해 주는 일을 먼저 하고 
    if (!strcmp(head->name, T_FUNC_DEF)) {
        if (currentScope->visit_i < currentScope->num_child) {
            nextScope = currentScope->child[currentScope->visit_i];
            currentScope->visit_i++;
        }
        else {
            printf("ERROR: FUNC_DEF SYMBOL TABLE ACCESS FAILURE\n");
            return; 
        }
    }
    else if (!strcmp(head->name, T_CLAUSE)) {
        bool createNewScope = false; 
        NODE* child = head -> child; 

        while (child != NULL) {
            //printf("child name in clause: %s\n", child->name);
            if (!strcmp(child->name, T_BODY)) {
                createNewScope = true;
                break;
            }
            child = child->next;
        }

        if (createNewScope) {
            if (currentScope->visit_i < currentScope->num_child) {
                nextScope = currentScope->child[currentScope->visit_i];
                currentScope->visit_i++;
            } else {
                printf("ERROR: CLAUSE SYMBOL TABLE ACCESS FAILURE\n");
                return;
            }
        }
    }

    // Type Analysis 시작 
    if (!strcmp(head->name, T_ASSIGN_STMT)) {
        // assign_stmt -> variable OP_ASSIGN al_expr 
        int lhsType = GetExprType(nextScope, head->child);
        int rhsType = GetExprType(nextScope, head->child->next->next);
        //printf("%s : %s\n", head->child->name, getTypeString(lhsType));
        if (!((lhsType == 1 && rhsType == 1) || (lhsType == 2 && rhsType == 2) || (lhsType == 2 && rhsType == 1))) {
            // printf("Type error: %s number cannot be stored in %s variable!\n", 
            //     getTypeString(rhsType),  
            //     getTypeString(lhsType));
            char buffer[256];
            snprintf(buffer, sizeof(buffer), errorFormats[0],
                     getTypeString(rhsType), getTypeString(lhsType));
            errorIdNames[*errorCount] = strdup(buffer);
            (*errorCount)++;
        }
    }
    else if (!strcmp(head->name, T_AL_EXPR)) {
        NODE* child = head -> child;
        if (child->next != NULL) {
            // al_expr -> number | variable | al_expr OP_ADD al_expr | al_expr OP_MUL al_expr
            int type1 = GetExprType(nextScope, child);
            int type2 = GetExprType(nextScope, child->next->next);
            // int + float를 허용하고 있어  
            if (type1 == 0 || type2 == 0) {
                // printf("Type error: void type cannot be added or multiplied\n");
                errorIdNames[*errorCount] = strdup(errorFormats[1]);
                (*errorCount)++;
            }
        }
    }
    else if (!strcmp(head->name, T_REL_EXPR)) {
        // rel_expr -> value | rel_expr OP_REL rel_expr | rel_expr OP_LOGIC rel_expr
        // 지금 GetExprType에서는 어떤 경우든지 rel_expr이면 그냥 int를 가지고 오는 걸로 설정을 했는데 
        NODE* child = head -> child;
        if (child->next != NULL) {
            int type1 = GetExprType(nextScope, child);
            int type2 = GetExprType(nextScope, child->next->next);
            if (type1 != type2) {
                // printf("Type error: %s and %s cannot be compared together\n", getTypeString(type1), getTypeString(type2));
                char buffer[256];
                snprintf(buffer, sizeof(buffer), errorFormats[2],
                         getTypeString(type1), getTypeString(type2));
                errorIdNames[*errorCount] = strdup(buffer);
                (*errorCount)++;
            }
        }
    }
    else if (!strcmp(head->name, T_INC_EXPR)) {
        int type = GetExprType(nextScope, head->child);
        if (type == 0) {
            // printf("Type error: cannot increment or decrement 'void' type\n");
            errorIdNames[*errorCount] = strdup(errorFormats[3]);
            (*errorCount)++;
        }
    }
    else if (!strcmp(head->name, T_VARIABLE)) {
        // LBRACKET이 있고, 그 다음이 RBRACKET이 아닌 경우 (var[expr])
        if (head->child->next != NULL && 
            strstr(head->child->next->name, P_LBRACKET) &&
            !strstr(head->child->next->next->name, P_RBRACKET)) 
        {
            NODE* indexNode = head->child->next->next;
            int indexType = GetExprType(nextScope, indexNode);
            if (indexType != 1) {
                // printf("Type error: array index is not an integer\n");
                errorIdNames[*errorCount] = strdup(errorFormats[4]);
                (*errorCount)++;
            }
        }
    }
    TypeAnalysis(nextScope, head->child, errorIdNames, errorCount);
    TypeAnalysis(nextScope, head->next, errorIdNames, errorCount);
}

#endif 
