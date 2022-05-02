#ifndef ERRORMSG_INC_H
#define ERRORMSG_INC_H

void error_init(char* input);
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);


#endif