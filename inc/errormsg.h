#ifndef ERRORMSG_INC_H
#define ERRORMSG_INC_H

void error(char *fmt, ...);
void error_at(Token* tok, char *msg);

#endif