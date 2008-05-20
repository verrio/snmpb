/* $Id: shhopt.h 8040 2008-04-14 09:20:35Z schoenw $ */
#ifndef SHHOPT_H
#define SHHOPT_H

#ifdef __cplusplus
extern "C" {
#endif

/* constants for recognized option types. */
typedef enum {
    OPT_END,               /* nothing. used as ending element. */
    OPT_FLAG,              /* no argument following. sets variable to 1. */
    OPT_STRING,            /* string argument. */
    OPT_INT,               /* signed integer argument. */
    OPT_UINT,              /* unsigned integer argument. */
    OPT_LONG,              /* signed long integer argument. */
    OPT_ULONG              /* unsigned long integer argument. */
} optArgType;

/* flags modifying the default way options are handeled. */
#define OPT_CALLFUNC  1    /* pass argument to a function. */

typedef struct {
    char       shortName;  /* short option name. */
    char       *longName;  /* long option name, no including '--'. */
    optArgType type;       /* option type. */
    void       *arg;       /* pointer to variable to fill with argument,
                            * or pointer to function if type == OPT_FUNC. */
    int        flags;      /* modifier flags. */
} optStruct;


extern void optSetFatalFunc(void (*f)(const char *, ...));
extern void optParseOptions(int *argc, char *argv[],
			    optStruct opt[], int allowNegNum);

#ifdef __cplusplus
}
#endif

#endif
