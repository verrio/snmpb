#ifndef BISON_PARSER_SMI_TAB_H
# define BISON_PARSER_SMI_TAB_H

#ifndef YYSTYPE
typedef union {
    char           *text;	  		/* scanned quoted text       */
    char           *id;				/* identifier name           */
    int            err;				/* actually just a dummy     */
    time_t	   date;			/* a date value              */
    Object         *objectPtr;			/* object identifier         */
    SmiStatus      status;			/* a STATUS value            */
    SmiAccess      access;			/* an ACCESS value           */
    Type           *typePtr;
    List           *listPtr;			/* SEQUENCE and INDEX lists  */
    NamedNumber    *namedNumberPtr;		/* BITS or enum item         */
    Range          *rangePtr;			/* type restricting range    */
    SmiValue	   *valuePtr;
    SmiUnsigned32  unsigned32;			/*                           */
    SmiInteger32   integer32;			/*                           */
    SmiUnsigned64  unsigned64;			/*                           */
    SmiInteger64   integer64;			/*                           */
    struct Compl   compl;
    struct Index   index;
    Module	   *modulePtr;
    SubjectCategories *subjectCategoriesPtr;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	DOT_DOT	257
# define	COLON_COLON_EQUAL	258
# define	UPPERCASE_IDENTIFIER	259
# define	LOWERCASE_IDENTIFIER	260
# define	NUMBER	261
# define	NEGATIVENUMBER	262
# define	NUMBER64	263
# define	NEGATIVENUMBER64	264
# define	BIN_STRING	265
# define	HEX_STRING	266
# define	QUOTED_STRING	267
# define	ACCESS	268
# define	AGENT_CAPABILITIES	269
# define	APPLICATION	270
# define	AUGMENTS	271
# define	BEGIN_	272
# define	BITS	273
# define	CHOICE	274
# define	CONTACT_INFO	275
# define	CREATION_REQUIRES	276
# define	COUNTER32	277
# define	COUNTER64	278
# define	DEFINITIONS	279
# define	DEFVAL	280
# define	DESCRIPTION	281
# define	DISPLAY_HINT	282
# define	END	283
# define	ENTERPRISE	284
# define	EXPORTS	285
# define	EXTENDS	286
# define	FROM	287
# define	GROUP	288
# define	GAUGE32	289
# define	IDENTIFIER	290
# define	IMPLICIT	291
# define	IMPLIED	292
# define	IMPORTS	293
# define	INCLUDES	294
# define	INDEX	295
# define	INSTALL_ERRORS	296
# define	INTEGER	297
# define	INTEGER32	298
# define	INTEGER64	299
# define	IPADDRESS	300
# define	LAST_UPDATED	301
# define	MACRO	302
# define	MANDATORY_GROUPS	303
# define	MAX_ACCESS	304
# define	MIN_ACCESS	305
# define	MODULE	306
# define	MODULE_COMPLIANCE	307
# define	MODULE_IDENTITY	308
# define	NOT_ACCESSIBLE	309
# define	NOTIFICATIONS	310
# define	NOTIFICATION_GROUP	311
# define	NOTIFICATION_TYPE	312
# define	OBJECT	313
# define	OBJECT_GROUP	314
# define	OBJECT_IDENTITY	315
# define	OBJECT_TYPE	316
# define	OBJECTS	317
# define	OCTET	318
# define	OF	319
# define	ORGANIZATION	320
# define	OPAQUE	321
# define	PIB_ACCESS	322
# define	PIB_DEFINITIONS	323
# define	PIB_INDEX	324
# define	PIB_MIN_ACCESS	325
# define	PIB_REFERENCES	326
# define	PIB_TAG	327
# define	POLICY_ACCESS	328
# define	PRODUCT_RELEASE	329
# define	REFERENCE	330
# define	REVISION	331
# define	SEQUENCE	332
# define	SIZE	333
# define	STATUS	334
# define	STRING	335
# define	SUBJECT_CATEGORIES	336
# define	SUPPORTS	337
# define	SYNTAX	338
# define	TEXTUAL_CONVENTION	339
# define	TIMETICKS	340
# define	TRAP_TYPE	341
# define	UNIQUENESS	342
# define	UNITS	343
# define	UNIVERSAL	344
# define	UNSIGNED32	345
# define	UNSIGNED64	346
# define	VALUE	347
# define	VARIABLES	348
# define	VARIATION	349
# define	WRITE_SYNTAX	350


#endif /* not BISON_PARSER_SMI_TAB_H */
