#pragma once

enum{
	ID
	// keywords
	,TYPE_INT
	// delimiters
	,COMMA,FINISH
	// operators
	,ASSIGN,EQUAL

	,VAR,FUNCTION,
	IF,ELSE,WHILE,END,
	RETURN,TYPE_REAL,TYPE_STR
	,COLON,SEMICOLON,LPAR,RPAR,
	ADD,SUB,MUL,DIV,AND,OR,NOT,
	NOTEQ,LESS,GREATER,GREATEREQ,
	INT,REAL,STR,


	};

#define MAX_STR		127

typedef struct{
	int code;		// ID, TYPE_INT, ...
	int line;		// the line from the input file
	union{
		char text[MAX_STR+1];		// the chars for ID, STR
		int i;		// the vlaue for INT
		double r;		// the value for REAL
		};
	}Token;

#define MAX_TOKENS		4096
extern Token tokens[];
extern int nTokens;

void tokenize(const char *pch);
void showTokens();
