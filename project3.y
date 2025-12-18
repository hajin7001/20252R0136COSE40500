%{
/**********PROLOGUE AREA**********/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "symtab.h"

#define MAX_SCOPE_ERRORS 128
#define MAX_TYPE_ERRORS 128 

NODE *head;
char buf[100];
FILE* fp;

extern FILE *yyin;
extern char *yytext;
extern int yylineno;
char * filename;

typedef union {
    int number;
    char *string;
    NODE *node;
} MyValue;

typedef enum { SYM_TOKEN, SYM_NODE } SymbolKind;
typedef struct {
	SymbolKind kind;
    char* name;  
    MyValue val;
} SymbolSpec;

int yylex();

void yyerror(const char *str){
    fprintf(stderr, "%s:%d: error: %s '%s' token \n", filename, yylineno, str, yytext);
}

int yywrap(){
     return 1; 
}

NODE* CreateTokenNode(char* token_type, char* token_value);
NODE* BuildRuleNode(char* rulename, SymbolSpec* specs, int count);


%}

/**********GRAMMAR AREA**********/
%union { 
    int number; 
    char *string; 
    NODE *node; 
}

/* Tokens */
%token <string> DEFINE
%token <string> INT FLOAT VOID
%token <string> IF FOR ELSE
%token <string> CONTINUE
%token <string> OP_ASSIGN OP_INC OP_DEC OP_ADD OP_MUL OP_LOGIC OP_REL
%token <string> ID
%token <string> NUM
%token <string> NUM_BIN
%token <string> NUM_HEX
%token <string> LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET COMMA SEMICOLON

/* dangling-else handling */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

/* operator precedence (lowest -> highest) */
%left OP_LOGIC
%left OP_REL
%left OP_ADD
%left OP_MUL

%start c_code

/* Non-terminal */
%type <node> c_code code
%type <node> define_header func_def func_arg_dec body body_list 
%type <node> statement assign_stmt continue_stmt
%type <node> decl_list decl_init al_expr rel_expr inc_expr
%type <node> variable value type clause init_stmt test_expr update_stmt
%type <node> number unit

%%
/* CFG */
//todo: build parse tree in actions
c_code:
	code {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } }
        };
        $$ = BuildRuleNode("c_code", specs, 1);
		head = $$;
	}
	| c_code code {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
			{ SYM_NODE, NULL, { .node = $2 } }
        };
        $$ = BuildRuleNode("c_code", specs, 2);
		head = $$;
	};

code:
	define_header {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } }
        };
        $$ = BuildRuleNode("code", specs, 1);
	}

	|func_def  {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } }
        };
        $$ = BuildRuleNode("code", specs, 1);
	};

define_header:
	DEFINE ID number {
		SymbolSpec specs[] = {
            { SYM_TOKEN, "DEFINE", { .string = $1 } },
            { SYM_TOKEN, "ID", { .string = $2 } },
            { SYM_NODE,  NULL, { .node   = $3 } }
        };
        $$ = BuildRuleNode("define_header", specs, 3);
	};

func_def:
	type ID LPAREN func_arg_dec RPAREN LBRACE body_list RBRACE {
		SymbolSpec specs[] = {
			{ SYM_NODE, NULL, {.node = $1} },
			{ SYM_TOKEN, "ID", { .string = $2 } },
			{ SYM_TOKEN, "LPAREN", { .string = $3 } },
			{ SYM_NODE, NULL, { .node = $4 } },
			{ SYM_TOKEN, "RPAREN", { .string = $5 } },
			{ SYM_TOKEN, "LBRACE", { .string = $6 } },
			{ SYM_NODE, NULL, { .node = $7 } },
			{ SYM_TOKEN, "RBRACE", { .string = $8 } },
		};
		$$ = BuildRuleNode("func_def", specs, 8);
	};

func_arg_dec:
	decl_list {
		$$ = MakeNode("func_arg_dec");
		InsertChild($$, $1);
	}
	;

unit:
    clause { 
		$$ = $1;
    }
    | statement { 
		$$ = $1;
    };

/* body:
    unit  { 
		$$ = MakeNode("body");
		InsertChild($$, $1);
    }
    | body unit { 
		$$ = MakeNode("body");
		InsertChild($$, $1);
		InsertSibling($1, $2);
	}; */

body_list:
    unit  {
        $$ = MakeNode("body");
        InsertChild($$, $1);
    }
    | body_list body {
        $$ = MakeNode("body");
        InsertChild($$, $1);
        InsertSibling($1, $2);
    };

body:
    unit  { 
        $$ = MakeNode("body");
        InsertChild($$, $1);
    };

// changed body -> body_list
clause: 
    FOR LPAREN init_stmt test_expr SEMICOLON update_stmt RPAREN LBRACE body_list RBRACE {
		SymbolSpec specs[] = {
			{ SYM_TOKEN, "FOR", {.string = $1} },
			{ SYM_TOKEN, "LPAREN", { .string = $2 } },
			{ SYM_NODE, NULL, { .node = $3 } },
			{ SYM_NODE, NULL, { .node = $4 } },
			{ SYM_TOKEN, "SEMICOLON", { .string = $5 } },
			{ SYM_NODE, "update_stmt", { .node = $6 } },
			{ SYM_TOKEN, "RPAREN", { .string = $7 } },
			{ SYM_TOKEN, "LBRACE", { .string = $8 } },
			{ SYM_NODE, NULL, { .node = $9 } },
			{ SYM_TOKEN, "RBRACE", { .string = $10 } },
		};
		$$ = BuildRuleNode("clause", specs, 10);
	}

    | IF LPAREN test_expr RPAREN LBRACE body_list RBRACE %prec LOWER_THAN_ELSE {
		SymbolSpec specs[] = {
            { SYM_TOKEN, "IF", { .string = $1 } },
            { SYM_TOKEN, "LPAREN", { .string = $2 } },
            { SYM_NODE,  NULL, { .node   = $3 } },
            { SYM_TOKEN, "RPAREN", { .string = $4 } },
            { SYM_TOKEN, "LBRACE", { .string = $5 } },
            { SYM_NODE,  NULL, { .node   = $6 } },
            { SYM_TOKEN, "RBRACE", { .string = $7 } },
        };
        $$ = BuildRuleNode("clause", specs, 7);
	}

    | IF LPAREN test_expr RPAREN statement %prec LOWER_THAN_ELSE {
		SymbolSpec specs[] = {
            { SYM_TOKEN, "IF", { .string = $1 } },
            { SYM_TOKEN, "LPAREN", { .string = $2 } },
            { SYM_NODE,  NULL, { .node   = $3 } },
            { SYM_TOKEN, "RPAREN", { .string = $4 } },
            { SYM_NODE,  NULL, { .node   = $5 } },
        };
        $$ = BuildRuleNode("clause", specs, 5);				
	}
	| IF LPAREN test_expr RPAREN statement ELSE statement {
		SymbolSpec specs[] = {
            { SYM_TOKEN, "IF", { .string = $1 } },
            { SYM_TOKEN, "LPAREN", { .string = $2 } },
            { SYM_NODE,  NULL, { .node   = $3 } },
            { SYM_TOKEN, "RPAREN", { .string = $4 } },
            { SYM_NODE,  NULL, { .node   = $5 } },
            { SYM_TOKEN, "ELSE", { .string = $6 } },
            { SYM_NODE,  NULL, { .node   = $7 } },
        };
        $$ = BuildRuleNode("clause", specs, 7);
	}
	| IF LPAREN test_expr RPAREN LBRACE body_list RBRACE ELSE LBRACE body_list RBRACE {
		SymbolSpec specs[] = {
            { SYM_TOKEN, "IF", { .string = $1 } },
            { SYM_TOKEN, "LPAREN", { .string = $2 } },
            { SYM_NODE,  NULL, { .node   = $3 } },
            { SYM_TOKEN, "RPAREN", { .string = $4 } },
            { SYM_TOKEN, "LBRACE", { .string = $5 } },
            { SYM_NODE,  NULL, { .node   = $6 } },
            { SYM_TOKEN, "RBRACE", { .string = $7 } },
            { SYM_TOKEN, "ELSE", { .string = $8 } },
            { SYM_TOKEN, "LBRACE", { .string = $9 } },
            { SYM_NODE,  NULL, { .node   = $10 } },
            { SYM_TOKEN, "RBRACE", { .string = $11 } },
        };
        $$ = BuildRuleNode("clause", specs, 11);				
	}
	;

statement:
    assign_stmt SEMICOLON {
		SymbolSpec specs[] = {
            { SYM_NODE,  NULL, { .node = $1 } },
            { SYM_TOKEN, "SEMICOLON", { .string = $2 } },
        };
        $$ = BuildRuleNode("statement", specs, 2);				
	}
    |continue_stmt SEMICOLON {
		SymbolSpec specs[] = {
            { SYM_NODE,  NULL, { .node   = $1 } },
            { SYM_TOKEN, "SEMICOLON", { .string = $2 } },
        };
        $$ = BuildRuleNode("statement", specs, 2);
	}

	|decl_list SEMICOLON {
		SymbolSpec specs[] = {
            { SYM_NODE,  NULL, { .node   = $1 } },
            { SYM_TOKEN, "SEMICOLON", { .string = $2 } },
        };
        $$ = BuildRuleNode("statement", specs, 2);				
	}

	|error SEMICOLON
	{
		yyerrok;
	};

init_stmt:
	assign_stmt SEMICOLON {
		SymbolSpec specs[] = {
            { SYM_NODE,  NULL, { .node = $1 } },
            { SYM_TOKEN, "SEMICOLON", { .string = $2 } },
        };
        $$ = BuildRuleNode("init_stmt", specs, 2);				
	}
	|decl_list SEMICOLON {
		SymbolSpec specs[] = {
            { SYM_NODE,  NULL, { .node = $1 } },
            { SYM_TOKEN, "SEMICOLON", { .string = $2 } },
        };
        $$ = BuildRuleNode("init_stmt", specs, 2);				
	}
	;

update_stmt:
	inc_expr {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } }
        };
        $$ = BuildRuleNode("update_stmt", specs, 1);				
	}
	|decl_list {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } }
        };
        $$ = BuildRuleNode("update_stmt", specs, 1);				
	}
	;

assign_stmt:
    variable OP_ASSIGN al_expr {
		SymbolSpec specs[] = {
            { SYM_NODE,  NULL, { .node   = $1 } },
            { SYM_TOKEN, "OP_ASSIGN", { .string = $2 } },
            { SYM_NODE,  NULL, { .node   = $3 } },
        };
        $$ = BuildRuleNode("assign_stmt", specs, 3);				
	}
    ;

continue_stmt:
    CONTINUE {
		SymbolSpec specs[] = {
            { SYM_TOKEN, "CONTINUE", { .string = $1 } }
        };
        $$ = BuildRuleNode("continue_stmt", specs, 1);
					
	}
    ;

test_expr:
	rel_expr {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } }
        };
        $$ = BuildRuleNode("test_expr", specs, 1);				
	}
	;

decl_list:
	decl_init  {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } }
        };
        $$ = BuildRuleNode("decl_list", specs, 1);				
	}

	|decl_list COMMA variable {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_TOKEN, "COMMA", { .string = $2 } },
            { SYM_NODE, NULL, { .node = $3 } },
        };
        $$ = BuildRuleNode("decl_list", specs, 3);				
	}

	|decl_list COMMA decl_init {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_TOKEN, "COMMA", { .string = $2 } },
            { SYM_NODE, NULL, { .node = $3 } },
        };
        $$ = BuildRuleNode("decl_list", specs, 3);
	}
	;

decl_init:
	type variable {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_NODE, NULL, { .node = $2 } },
        };
        $$ = BuildRuleNode("decl_init", specs, 2);					
	}
	;

number:
    NUM {
		$$ = CreateTokenNode("NUM", $1);
		// SymbolSpec specs[] = {
        //     { SYM_TOKEN, "NUM", { .string = $1 } }
        // };
        // $$ = BuildRuleNode("number", specs, 1);
    }
    | NUM_BIN {
		$$ = CreateTokenNode("NUM_BIN", $1);
		// SymbolSpec specs[] = {
        //     { SYM_TOKEN, "NUM_BIN", { .string = $1 } }
        // };
        // $$ = BuildRuleNode("number", specs, 1);
    }
    | NUM_HEX {
		$$ = CreateTokenNode("NUM_HEX", $1);
		// SymbolSpec specs[] = {
        //     { SYM_TOKEN, "NUM_HEX", { .string = $1 } }
        // };
        // $$ = BuildRuleNode("number", specs, 1);
    }
    ;


al_expr:
	number {
		SymbolSpec specs[] = { { SYM_NODE, NULL, { .node = $1 } } };
        $$ = BuildRuleNode("al_expr", specs, 1);				
	}

	| variable {
		SymbolSpec specs[] = { { SYM_NODE, NULL, { .node = $1 } } };
        $$ = BuildRuleNode("al_expr", specs, 1);				
	}

	| al_expr OP_ADD al_expr {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_TOKEN, "OP_ADD", { .string = $2 } },
            { SYM_NODE, NULL, { .node = $3 } },
        };
        $$ = BuildRuleNode("al_expr", specs, 3);				
	}

	| al_expr OP_MUL al_expr {
		SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_TOKEN, "OP_MUL", { .string = $2 } },
            { SYM_NODE, NULL, { .node = $3 } },
        };
        $$ = BuildRuleNode("al_expr", specs, 3);				
	}
	;
rel_expr:
    value {
        SymbolSpec specs[] = { { SYM_NODE, NULL, { .node = $1 } } };
        $$ = BuildRuleNode("rel_expr", specs, 1);
    }
    | rel_expr OP_REL rel_expr {
        SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_TOKEN, "OP_REL", { .string = $2 } },
            { SYM_NODE, NULL, { .node = $3 } },
        };
        $$ = BuildRuleNode("rel_expr", specs, 3);
    }
    | rel_expr OP_LOGIC rel_expr {
        SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_TOKEN, "OP_LOGIC", { .string = $2 } },
            { SYM_NODE, NULL, { .node = $3 } },
        };
        $$ = BuildRuleNode("rel_expr", specs, 3);
    }
;

inc_expr:
    variable OP_INC {
        SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_TOKEN, "OP_INC", { .string = $2 } },
        };
        $$ = BuildRuleNode("inc_expr", specs, 2);
    }
    | variable OP_DEC {
        SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_TOKEN, "OP_DEC", { .string = $2 } },
        };
        $$ = BuildRuleNode("inc_expr", specs, 2);
    }
;

value:
    variable {
        SymbolSpec specs[] = { { SYM_NODE, NULL, { .node = $1 } } };
        $$ = BuildRuleNode("value", specs, 1);
    }
    | number {
        SymbolSpec specs[] = { { SYM_NODE, NULL, { .node = $1 } } };
        $$ = BuildRuleNode("value", specs, 1);
    }
;

variable:
    ID {
        SymbolSpec specs[] = { { SYM_TOKEN, "ID", { .string = $1 } } };
        $$ = BuildRuleNode("variable", specs, 1);
    }
    | variable LBRACKET RBRACKET {
        SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_TOKEN, "LBRACKET", { .string = $2 } },
            { SYM_TOKEN, "RBRACKET", { .string = $3 } },
        };
        $$ = BuildRuleNode("variable", specs, 3);
    }
    | variable LBRACKET al_expr RBRACKET {
        SymbolSpec specs[] = {
            { SYM_NODE, NULL, { .node = $1 } },
            { SYM_TOKEN, "LBRACKET", { .string = $2 } },
            { SYM_NODE, NULL, { .node = $3 } },
            { SYM_TOKEN, "RBRACKET", { .string = $4 } },
        };
        $$ = BuildRuleNode("variable", specs, 4);
    }
;

type:
    VOID {
        SymbolSpec specs[] = { { SYM_TOKEN, "VOID", { .string = $1 } } };
        $$ = BuildRuleNode("type", specs, 1);
    }
    | INT {
        SymbolSpec specs[] = { { SYM_TOKEN, "INT", { .string = $1 } } };
        $$ = BuildRuleNode("type", specs, 1);
    }
    | FLOAT {
        SymbolSpec specs[] = { { SYM_TOKEN, "FLOAT", { .string = $1 } } };
        $$ = BuildRuleNode("type", specs, 1);
    }
;

%%

/**********EPILOGUE AREAR AREA**********/
//THIS AREA WILL BE COPIED TO y.tab.c CODE
NODE* CreateTokenNode(char* token_type, char* token_value) {
    NODE* node = MakeNode(token_type);
    sprintf(buf, "%s: %s", token_type, token_value);
    //free(node->name);
    //node->name = strdup(buf);
    strcpy(node->name, buf);
    return node;
}

NODE* BuildRuleNode(char* rulename, SymbolSpec* specs, int count) {
	NODE* parent = MakeNode(rulename);
    if (count == 0) return parent;

	NODE* first = NULL;
    if (specs[0].kind == SYM_TOKEN) {
        first = CreateTokenNode(specs[0].name, specs[0].val.string);
    } else {
        first = specs[0].val.node;
    }
    InsertChild(parent, first);

	NODE* prev = first;
    for (int i = 1; i < count; i++) {
        NODE* cur = NULL;
        if (specs[i].kind == SYM_TOKEN) {
            cur = CreateTokenNode(specs[i].name, specs[i].val.string);
        } else {
            cur = specs[i].val.node;
        }
        if (cur) {
            InsertSibling(prev, cur);
            cur->parent = parent; // 이거를 추가했어 
            prev = cur;
        }
    }

	return parent;
}

int main(int argc, char **argv){
    if (argc < 2){
        fprintf(stderr, "usage: %s <source.c>\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin){
        fprintf(stderr, "cannot open %s\n", argv[1]);
        return 1;
    }
    
    filename = argv[1];
    //todo
    SYMTAB* rootSymTab = NewSymTab();
    char* scopeErrorIds[MAX_SCOPE_ERRORS];
    int scopeErrorCount = 0;

    char* typeErrorIds[MAX_TYPE_ERRORS];
    int typeErrorCount = 0; 

    yyparse();
    //todo
    
    if (head != NULL) { 
        ConstructSymTab(rootSymTab, head);
        
        printf("--------------------------------------------------------\n");
        PrintSymTab(rootSymTab);
        printf("\n"); 

        ResetVisitCounters(rootSymTab);
        ScopeAnalysis(rootSymTab, head, scopeErrorIds, &scopeErrorCount);
        if (scopeErrorCount > 0) {
            for (int i = 0; i < scopeErrorCount; i++) {
                printf("Undefined Error (%s)\n", scopeErrorIds[i]);
            }
        }

        if (scopeErrorCount == 0){
            ResetVisitCounters(rootSymTab);
            TypeAnalysis(rootSymTab, head, typeErrorIds, &typeErrorCount);
            for (int i = 0; i < typeErrorCount; i++) {
                    bool duplicate = false;
                    for (int j = 0; j < i; j++) {
                        if (strcmp(typeErrorIds[i], typeErrorIds[j]) == 0) {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate) {
                        printf("%s\n", typeErrorIds[i]);
                    }
                }
                for (int i = 0; i < typeErrorCount; i++) {
                    free(typeErrorIds[i]);
            }
        }
        
    } else {
        fprintf(stderr, "Parsing failed. No syntax tree generated.\n");
    } 
    return 0;
}
