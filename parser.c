#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"
#include "ad.h"

int iTk;
Token *consumed;
bool expr();
bool factor();
bool instr();
bool block();

// same as err, but also prints the line of the current token
_Noreturn void tkerr(const char *fmt,...){
	fprintf(stderr,"error in line %d: ",tokens[iTk].line);
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
	}


bool consume(int code){
	if(tokens[iTk].code==code){
		consumed=&tokens[iTk++];
		return true;
		}
	return false;
	}

bool baseType() {
	if (consume(TYPE_INT)) {
		ret.type=TYPE_INT;
		return true;
	}
	if (consume(TYPE_REAL)) {
       ret.type=TYPE_REAL;
		return true;
	}
	if (consume(TYPE_STR)) {
		ret.type=TYPE_STR;
		return true;
	}
	return false;
}

bool defVar() {
	int start = iTk;

	if (consume(VAR)) {
		if (consume(ID)) {
			const char *name=consumed->text;
			Symbol *s=searchInCurrentDomain(name);
			if(s)tkerr("symbol redefinition: %s",name);
			s=addSymbol(name,KIND_VAR);
			s->local=crtFn!=NULL;

			if (consume(COLON)) {
				if (baseType()) {
					s->type=ret.type;
					if (consume(SEMICOLON)) {
						return true;
					} else tkerr("Lipseste ';' !");
				} else tkerr("Nu ai ales tipul de variabila!");
			} else tkerr("Lipseste ':' !");
		} else tkerr("Nu ai dat nume la variabila!");
	}

	// If it didn't start with VAR, just reset and return false (no error)
	iTk = start;
	return false;
}

bool funcParam() {
	if (consume(ID)) {
		const char *name = consumed->text;
		Symbol *s = searchInCurrentDomain(name);
		if (s)tkerr("symbol redefinition: %s", name);
		s = addSymbol(name, KIND_ARG);
		Symbol *sFnParam = addFnArg(crtFn, name);

		if (consume(COLON)) {
			if (baseType()) {
				s->type=ret.type;
				sFnParam->type=ret.type;
				return true;
			} else tkerr("Lipseste tipul parametrului!");
		} else tkerr("Lipseste ':' dupa numele parametrului!");
	}
	return false;
}

bool funcParams() {
	int start = iTk;


	if (funcParam()) {

		while (true) {
			if (consume(COMMA)) {
              if (funcParam()) {

              }else{tkerr("Lipseste parametrul de dupa virgula");}

			}else {break;}

		}
		return true;
	}


	iTk = start;
	return false;
}

bool factor() {
	if (consume(INT)) {
		return true;
	}
	if (consume(REAL)) {
		return true;
	}
	if (consume(STR)) {
		return true;
	}
	if (consume(LPAR)) {
		if (expr()) {
          if (consume(RPAR)) {
	          return true;
          }else{tkerr("Lipseste ')'");}
		}else{tkerr("Lipseste expresia");}

	}
    if (consume(ID)) {

         if (consume(LPAR)) {

         	if (expr()) {
         		while (true) {

         			if (consume(COMMA)) {
         				if (expr()) {

         				}else{tkerr("Lipseste expresia dupa virgula");}

         			}
         			else {break;}
         		}

         	}
         	if (consume(RPAR)) {

         	}
         }
    	return true;
    }

	return false;
}

bool exprPrefix() {
	int start = iTk;
	if (consume(SUB)||consume(NOT)) {}
	if (factor()) {
		return true;
	}
	iTk = start;
	return false;
}

bool exprMul() {

	int start = iTk;
	if (exprPrefix()) {

		while (true) {
			if (consume(MUL) || consume(DIV)) {
				if (exprPrefix()) {

				}
				else {tkerr("Lipseste expresia dupa operatorul aritmetic");}
			}
			else {break;}
		}
		return true;
	}
	iTk = start;
	return false;


}

bool exprAdd() {
	int start = iTk;
if (exprMul()) {

while (true) {
	if (consume(ADD) || consume(SUB)) {
		if (exprMul()) {

		}
		else {tkerr("Lipseste expresia dupa operatorul aritmetic");}
	}
	else {break;}
}
return true;
}
	iTk = start;
	return false;

}


bool exprComp() {
	int start = iTk;
if (exprAdd()) {

	if (consume(LESS) || consume(EQUAL)) {
		if (exprAdd()) {

		}else{tkerr("Lipseste expresia dupa operatorul de comparatie!");}

	}

	return true;
}
	iTk = start;
	return false;
}

bool exprAssign() {
	int start = iTk;

	// ( ID ASSIGN )? exprComp
	if (consume(ID)) {
		if (consume(ASSIGN)) {
			if (exprComp()) {
				return true;
			}
			else{	tkerr("Lipseste expresia dupa '='!");}

		}
		// if there was an ID but no ASSIGN, reset so exprComp can parse it
		iTk = start;
	}

	if (exprComp()) return true;

	iTk = start;
	return false;
}

bool exprLogic() {
	int start = iTk;
	if (exprAssign()){

		while (true) {
			if (consume(AND) || consume(OR)) {
               if (exprAssign()) {

               }
			}else {break;}

		}
		return true;
	}
	iTk = start;
	return false;
}

bool expr() {

	if (exprLogic()) {
		return true;
	}
	return false;
}

bool block()
{
	int start = iTk;
	if (instr()) {
		while (true) {
			if (instr()) {

			}
			else {break;}
		}
		return true;
	}
	iTk = start;
	return false;
}

bool instr() {
    int start = iTk;


    if (expr()) {
        if (consume(SEMICOLON)) {
            return true;
        } else {
            tkerr("Lipseste ';' dupa expresie!");
        }
    }

    iTk = start;


    if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        // optional ELSE block
                        if (consume(ELSE)) {
                            if (block()) {

                            }else {tkerr("Lipseste blocul de cod dupa ELSE");}

                        }
                        if (consume(END)) {
                            return true;
                        } else tkerr("Lipseste END dupa IF!");
                    } else tkerr("Bloc lipsa in IF!");
                } else tkerr("Lipseste ')' dupa expresie IF!");
            } else tkerr("Expresie lipsa in IF!");
        } else tkerr("Lipseste '(' dupa IF!");
    }

    iTk = start; // reset and try next alternative


    if (consume(RETURN)) {
        if (expr()) {
            if (consume(SEMICOLON)) {
                return true;
            } else tkerr("Lipseste ';' dupa RETURN!");
        } else tkerr("Expresie lipsa dupa RETURN!");
    }

    iTk = start;


    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(END)) {
                            return true;
                        } else tkerr("Lipseste END dupa WHILE!");
                    } else tkerr("Bloc lipsa in WHILE!");
                } else tkerr("Lipseste ')' dupa expresie WHILE!");
            } else tkerr("Expresie lipsa in WHILE!");
        } else tkerr("Lipseste '(' dupa WHILE!");
    }


    iTk = start;
    return false;
}




bool defFunc() {
	int start = iTk;

	if (consume(FUNCTION)) {
		if (consume(ID)) {

			const char *name = consumed->text;
			Symbol *s = searchInCurrentDomain(name);
			if (s)tkerr("symbol redefinition: %s", name);
			crtFn = addSymbol(name, KIND_FN);
			crtFn->args = NULL;
			addDomain();

			if (consume(LPAR)) {

				funcParams();

				if (consume(RPAR)) {
					if (consume(COLON)) {
						if (baseType()) {

							crtFn->type=ret.type;

							while (defVar()) {

							}

							if (block()) {
								if (consume(END)) {
									delDomain();
									crtFn=NULL;
									return true;
								} else tkerr("Lipseste 'END' dupa blocul functiei!");
							} else tkerr("Lipseste blocul de cod al functiei!");
						} else tkerr("Lipseste tipul de retur al functiei!");
					} else tkerr("Lipseste ':' dupa lista de parametri!");
				} else tkerr("Lipseste ')' !");
			} else tkerr("Lipseste '(' dupa numele functiei!");
		} else tkerr("Lipseste numele functiei!");
	}


	iTk = start;
	return false;
}


 //program ::= ( defVar | defFunc | block )* FINISH
 bool program(){

addDomain();

 	for(;;){
		if(defVar()){}
		else if(defFunc()){}
		else if(block()){}
 		else break;
 		}
 	if(consume(FINISH)){


 		delDomain();
 		return true;
 		}else tkerr("syntax error");
 	return false;


 	}

 void parse(){
 	iTk=0;
 	program();
	}
