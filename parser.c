#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"
#include "ad.h"
#include "at.h"

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

		setRet(TYPE_INT,false);

		return true;
	}
	if (consume(REAL)) {

		setRet(TYPE_REAL,false);

		return true;
	}
	if (consume(STR)) {

		setRet(TYPE_STR,false);

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

    	Symbol *s=searchSymbol(consumed->text);
    	if(!s)tkerr("undefined symbol: %s",consumed->text);

         if (consume(LPAR)) {

         	if(s->kind!=KIND_FN)tkerr("%s cannot be called, because it is not a function",s->name);
         	Symbol *argDef=s->args;

         	if (expr()) {

         		if(!argDef)tkerr("the function %s is called with too many arguments",s->name);
         		if(argDef->type!=ret.type)tkerr("the argument type at function %s call is different from the one given at its definition",s->name);
         		argDef=argDef->next;

         		while (true) {

         			if (consume(COMMA)) {
         				if (expr()) {

         					if(!argDef)tkerr("the function %s is called with too many arguments",s->name);
         					if(argDef->type!=ret.type)tkerr("the argument type at function %s call is different from the one given at its definition",s->name);
         					argDef=argDef->next;

         				}else{tkerr("Lipseste expresia dupa virgula");}

         			}
         			else {break;}
         		}

         	}
         	if (consume(RPAR)) {
         		if(argDef)tkerr("the function %s is called with too few arguments",s->name);
         		setRet(s->type,false);
         		return true;
         	}
         	if(s->kind==KIND_FN)tkerr("the function %s can only be called",s->name); setRet(s->type,true);
         }
    	return true;
    }

	return false;
}

bool exprPrefix() {
	int start = iTk;
	if (consume(SUB)) {

		if (factor()) {

			if(ret.type==TYPE_STR)tkerr("the expression of unary- must be of type int or real");
			ret.lval=false;

			return true;
		}

	}

	if (consume(NOT)) {

		if (factor()) {

			if(ret.type==TYPE_STR)tkerr("the expression of ! must be of type int or real");
			setRet(TYPE_INT,false); }

			return true;
		}

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

				Ret leftType=ret;
				if(leftType.type==TYPE_STR)tkerr("the operands of * or / cannot be of type str");

				if (exprPrefix()) {

					if(leftType.type!=ret.type)tkerr("different types for the operands of * or /");
					ret.lval=false;

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

		Ret leftType=ret;
		if(leftType.type==TYPE_STR)tkerr("the operands of + or- cannot be of type str");

		if (exprMul()) {

			if(leftType.type!=ret.type)tkerr("different types for the operands of + or-"); ret.lval=false;

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
	Ret leftType=ret;
if (exprAdd()) {

	if (consume(LESS) || consume(EQUAL)) {
		if (exprAdd()) {

			if(leftType.type!=ret.type)tkerr("different types for the operands of < or ==");
			setRet(TYPE_INT,false);

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

		const char *name=consumed->text;

		if (consume(ASSIGN)) {
			if (exprComp()) {

				Symbol *s=searchSymbol(name);
				 if(!s)tkerr("undefined symbol: %s",name);
				  if(s->kind==KIND_FN)tkerr("a function (%s) cannot be used as a destination for assignment ",name);
				   if(s->type!=ret.type)tkerr("the source and destination for assignment must have the same type");
				    ret.lval=false;

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

				Ret leftType=ret;
				if(leftType.type==TYPE_STR)tkerr("the left operand of && or || cannot be of type str");

               if (exprAssign()) {

               	if(ret.type==TYPE_STR)tkerr("the right operand of && or || cannot be of type str");
               	setRet(TYPE_INT,false);

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

            	if(ret.type==TYPE_STR)tkerr("the if condition must have type int or real");

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

        	if(!crtFn)tkerr("return can be used only in a function");
        	if(ret.type!=crtFn->type)tkerr("the return type must be the same as the function return type");

            if (consume(SEMICOLON)) {
                return true;
            } else tkerr("Lipseste ';' dupa RETURN!");
        } else tkerr("Expresie lipsa dupa RETURN!");
    }

    iTk = start;


    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {

            	if(ret.type==TYPE_STR)tkerr("the while condition must have type int or real");

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
	addPredefinedFns();

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
