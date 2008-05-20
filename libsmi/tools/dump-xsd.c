/*
 * dump-xsd.c --
 *
 *      Operations to dump SMI module information as XML schema definitions.
 *
 * Copyright (c) 2001 J. Schoenwaelder, Technical University of Braunschweig.
 *           (c) 2002 T. Klie, Technical University of Braunschweig.
 *           (c) 2002 F. Strauss, Technical University of Braunschweig.
 *           (c) 2007 T. Klie, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-xsd.c 8090 2008-04-18 12:56:29Z strauss $
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "smidump.h"
#include "fortopat.h"


#define  INDENT		2    /* indent factor */

static int ind = 0;

#ifndef MIN
#define MIN(a,b) ((a)) < ((b)) ? ((a)) : ((b))
#endif /* #ifndef MIN */

static char *schemaLocation = "http://www.ibr.cs.tu-bs.de/projects/libsmi/xsd/";
static int container = 0;
static char *containerBasename = "container";
static int *nestAugmentedTables = 0;
static int *nestSubtables = 0;

typedef struct XmlEscape {
    char character;
    char *escape;
} XmlEscape;

static XmlEscape xmlEscapes [] = {
    { '<',	"&lt;" },
    { '>',	"&gt;" },
    { '&',	"&amp;" },
    { 0,	NULL }
};

typedef struct TypePrefix {
    char *type;
    char *prefix;
    struct TypePrefix *next;
} TypePrefix;

static TypePrefix *typePrefixes = NULL;



/* some forward declarations */
static void fprintElement( FILE *f, SmiNode *smiNode, SmiNode *parentNode );
static char* getTypePrefix( char *typeName );

static char *getStringBasetype(SmiBasetype basetype)
{
    return
        (basetype == SMI_BASETYPE_UNKNOWN)           ? "<UNKNOWN>" :
        (basetype == SMI_BASETYPE_OCTETSTRING)       ? "OctetString" :
        (basetype == SMI_BASETYPE_OBJECTIDENTIFIER)  ? "ObjectIdentifier" :
        (basetype == SMI_BASETYPE_UNSIGNED32)        ? "Unsigned32" :
        (basetype == SMI_BASETYPE_INTEGER32)         ? "Integer32" :
        (basetype == SMI_BASETYPE_UNSIGNED64)        ? "Unsigned64" :
        (basetype == SMI_BASETYPE_INTEGER64)         ? "Integer64" :
        (basetype == SMI_BASETYPE_FLOAT32)           ? "Float32" :
        (basetype == SMI_BASETYPE_FLOAT64)           ? "Float64" :
        (basetype == SMI_BASETYPE_FLOAT128)          ? "Float128" :
        (basetype == SMI_BASETYPE_ENUM)              ? "Enumeration" :
        (basetype == SMI_BASETYPE_BITS)              ? "Bits" :
                                                   "<unknown>";
}

static char* getStringStatus(SmiStatus status)
{
    char *statStr;
    
    switch( status ) {
    case SMI_STATUS_CURRENT:
	statStr = "current";
	break;
    case SMI_STATUS_DEPRECATED:
	statStr = "deprecated";
	break;
    case SMI_STATUS_OBSOLETE:
	statStr = "obsolete";
	break;
    case SMI_STATUS_MANDATORY:
	statStr = "mandatory";
	break;
    case SMI_STATUS_OPTIONAL:
	statStr = "optional";
	break;
    case SMI_STATUS_UNKNOWN:
    default:
	statStr = "unknown";
	break;
    }
    return statStr;
}

static char* getStringAccess( SmiAccess smiAccess )
{
    switch( smiAccess ) {
    case SMI_ACCESS_NOT_IMPLEMENTED: return "not-implemented";
    case SMI_ACCESS_NOT_ACCESSIBLE : return "not-accessible";
    case SMI_ACCESS_NOTIFY         : return "notify";
    case SMI_ACCESS_READ_ONLY      : return "read-only";
    case SMI_ACCESS_READ_WRITE     : return "read-write";
    case SMI_ACCESS_UNKNOWN:
    default: return "unknown";
    }
}
#if 0
static char
*getStringValue(SmiValue *valuePtr, SmiType *typePtr)
{
    static char    s[1024];
    char           ss[9];
    int		   n;
    unsigned int   i;
    SmiNamedNumber *nn;
    SmiNode        *nodePtr;
    
    s[0] = 0;
    
    switch (valuePtr->basetype) {
    case SMI_BASETYPE_UNSIGNED32:
	sprintf(s, "%lu", valuePtr->value.unsigned32);
	break;
    case SMI_BASETYPE_INTEGER32:
	sprintf(s, "%ld", valuePtr->value.integer32);
	break;
    case SMI_BASETYPE_UNSIGNED64:
	sprintf(s, UINT64_FORMAT, valuePtr->value.unsigned64);
	break;
    case SMI_BASETYPE_INTEGER64:
	sprintf(s, INT64_FORMAT, valuePtr->value.integer64);
	break;
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
	break;
    case SMI_BASETYPE_ENUM:
	for (nn = smiGetFirstNamedNumber(typePtr); nn;
	     nn = smiGetNextNamedNumber(nn)) {
	    if (nn->value.value.unsigned32 == valuePtr->value.unsigned32)
		break;
	}
	if (nn) {
	    sprintf(s, "%s", nn->name);
	} else {
	    sprintf(s, "%ld", valuePtr->value.integer32);
	}
	break;
    case SMI_BASETYPE_OCTETSTRING:
	for (i = 0; i < valuePtr->len; i++) {
	    if (!isprint((int)valuePtr->value.ptr[i])) break;
	}
	if (i == valuePtr->len) {
	    sprintf(s, "\"%s\"", valuePtr->value.ptr);
	} else {
            sprintf(s, "0x%*s", 2 * valuePtr->len, "");
            for (i=0; i < valuePtr->len; i++) {
                sprintf(ss, "%02x", valuePtr->value.ptr[i]);
                strncpy(&s[2+2*i], ss, 2);
            }
	}
	break;
    case SMI_BASETYPE_BITS:
	sprintf(s, "(");
	for (i = 0, n = 0; i < valuePtr->len * 8; i++) {
	    if (valuePtr->value.ptr[i/8] & (1 << (7-(i%8)))) {
		if (n)
		    sprintf(&s[strlen(s)], ", ");
		n++;
		for (nn = smiGetFirstNamedNumber(typePtr); nn;
		     nn = smiGetNextNamedNumber(nn)) {
		    if (nn->value.value.unsigned32 == i)
			break;
		}
		if (nn) {
		    sprintf(&s[strlen(s)], "%s", nn->name);
		} else {
		    sprintf(s, "%d", i);
		}
	    }
	}
	sprintf(&s[strlen(s)], ")");
	break;
    case SMI_BASETYPE_UNKNOWN:
	break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
	nodePtr = smiGetNodeByOID(valuePtr->len, valuePtr->value.oid);
	if (nodePtr) {
	    sprintf(s, "%s", nodePtr->name);
	} else {
	    strcpy(s, "");
	    for (i=0; i < valuePtr->len; i++) {
		if (i) strcat(s, ".");
		sprintf(&s[strlen(s)], "%u", valuePtr->value.oid[i]);
	    }
	}
	break;
    }

    return s;
}
#endif /* 0 */

static int smiPow( int base, unsigned int exponent )
{
  unsigned int i;
  int ret = 1;
  
  if( exponent == 0 ) {
    return 1;
  }

  for( i = 0; i < exponent; i++ ) {
    ret *= base;
  }
  return ret;
}

static void fprintSegment(FILE *f, int relindent, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
     
    if ((ind == 0) || (ind + relindent == 0)) {
	ind += relindent;
    } else {
	if (relindent < 0) ind += relindent;
	fprintf(f, "%*c", ind * INDENT, ' ');
	if (relindent > 0) ind += relindent;
    }
    vfprintf(f, fmt, ap);

    va_end(ap);
}



static void fprintMultilineString(FILE *f, const char *s)
{
    int i, j, len;

    fprintSegment(f, 0, "");
    if (s) {
	len = strlen(s);
	for (i=0; i < len; i++) {
	    for (j = 0; xmlEscapes[j].character; j++) {
		if (xmlEscapes[j].character == s[i]) break;
	    }
	    if (xmlEscapes[j].character) {
		fputs(xmlEscapes[j].escape, f);
	    } else {
		putc(s[i], f);
	    }
            if (s[i] == '\n') {
		fprintSegment(f, 0, "");
	    }
	}
    }
}



static void fprintDocumentation(FILE *f, const char *description)
{
    if (description) {
	fprintSegment(f, 1, "<xsd:documentation>\n");
	fprintMultilineString(f, description);
	fprintf(f, "\n");
	fprintSegment(f, -1, "</xsd:documentation>\n");
    }
}

static void fprintNamedNumber( FILE *f, SmiNamedNumber *nn )
{
    fprintSegment( f, 1, "<xsd:enumeration value=\"%s\">\n", nn->name );
    fprintSegment( f, 1, "<xsd:annotation>\n");
    fprintSegment( f, 1, "<xsd:appinfo>\n");
    fprintSegment( f, 0, "<intVal>%d</intVal>\n",
		   (int)nn->value.value.integer32 );
    fprintSegment( f, -1, "</xsd:appinfo>\n");
    fprintSegment( f, -1, "</xsd:annotation>\n");
    fprintSegment( f, -1, "</xsd:enumeration>\n");
}


static void fprintStdRestHead( FILE *f, SmiType *smiType )
{
    char *baseTypeName = getStringBasetype(smiType->basetype);
    char *prefix = getTypePrefix( baseTypeName );
    
    if( prefix ) {
	fprintSegment(f, 1, "<xsd:restriction base=\"%s:%s\">\n",
		      prefix, baseTypeName );
    }
    else {
	fprintSegment(f, 1, "<xsd:restriction base=\"%s\">\n", baseTypeName );
    }
}


static void fprintHexOrAsciiType( FILE *f, SmiType *parent,
				  SmiInteger32 minLength,
				  SmiInteger32 maxLength,
				  char *name, int hex )
{
    char *prefix = parent ? getTypePrefix( parent->name ) : NULL;
    char *typeFlag = hex ? "Hex" : "Ascii";
    
    if( name ) {
	fprintSegment( f, 1, "<xsd:simpleType name=\"%s%s\">\n",
		       name, typeFlag );
    } else {
	fprintSegment( f, 1, "<xsd:simpleType>\n");
    }
    if( prefix ) {
	fprintSegment( f, 1, "<xsd:restriction base=\"%s:%s%s\">\n",
		       prefix, parent->name, typeFlag );
    }
    else {
	fprintSegment( f, 1, "<xsd:restriction base=\"%s%s\">\n",
		       parent->name, typeFlag );
    }
   
    if( minLength > 0 ) {
	fprintSegment( f, 0, "<xsd:minLength value=\"%d\"/>\n",
		       (int)minLength );
    }
    if( maxLength > -1 ) {
	fprintSegment( f, 0, "<xsd:maxLength value=\"%d\"/>\n",
		       (int)maxLength );
    }

    fprintSegment( f, -1, "</xsd:restriction>\n");
    fprintSegment( f, -1, "</xsd:simpleType>\n");
}


static int dhInParent( SmiType *smiType )
{
    SmiType *parent = smiGetParentType( smiType );

    if( smiType->format && parent->format ) {
	return ! strcmp( smiType->format, parent->format );
    }
    return 0;
}

#define MD_DH_INT_NORMAL   1
#define MD_DH_INT_DECIMAL  2
#define MD_DH_INT_BIN      3
#define MD_DH_INT_OCT      4
#define MD_DH_INT_HEX      5

/* parse a (integer) display hint and specify the offset, if used */
static int getIntDHType( char *hint, int *offset )
{
    switch( hint[ 0 ] ) {

    case 'd':
	if( hint[1] ) {
	    *offset = 0;
	    *offset = atoi( &hint[2] );
	    return MD_DH_INT_DECIMAL;
	}
	return MD_DH_INT_NORMAL;

    case 'b':
	/* binary value */
	return MD_DH_INT_BIN;
    case 'o':
	/* octet value */
	return MD_DH_INT_OCT;
    case 'x':
	/* hex value */
	return MD_DH_INT_HEX;
    default:
	/* should not occur */
	return 0;
    }
}


static void fprintRestriction(FILE *f, SmiType *smiType)
{
    SmiRange *smiRange;
    
    /* print ranges etc. */
    switch( smiType->basetype ) {

    case SMI_BASETYPE_INTEGER32:
    {
	SmiInteger32 min = SMI_BASETYPE_INTEGER32_MIN;
	SmiInteger32 max = SMI_BASETYPE_INTEGER32_MAX;
	int offset = 0, useDecPoint = 0;

	if( smiType->format ) {
	  /* we have a display hint here, so check if we have to use
	     a decimal point */
	  useDecPoint =  
	    getIntDHType( smiType->format, &offset ) == MD_DH_INT_DECIMAL; 
	  /* xxx: other display hint types (binary, oct, hex) */
	}

	if( useDecPoint ) {
	  fprintSegment( f, 1, "<xsd:restriction base=\"xsd:decimal\">\n");
	  fprintSegment( f, 0, "<xsd:fractionDigits value=\"%d\"/>\n", offset );
	}
	else {
	  fprintStdRestHead( f, smiType );
	}

	smiRange = smiGetFirstRange( smiType );
	while( smiRange ) {
	    if( min == SMI_BASETYPE_INTEGER32_MIN ||
		smiRange->minValue.value.integer32 < min ) {
		min = smiRange->minValue.value.integer32;
	    }
	    if( max == SMI_BASETYPE_INTEGER32_MAX ||
		smiRange->maxValue.value.integer32 > max ) {
		max = smiRange->maxValue.value.integer32;
	    }
	    smiRange = smiGetNextRange( smiRange );
	}
	
	/* print minimu value */
	if( useDecPoint ) {
	    fprintSegment( f, 0, "<xsd:minInclusive value=\"%d.%d\"/>\n", 
			   (int)min / smiPow( 10, offset ), 
			   abs( (int)min % smiPow( 10, offset ) ) );
	} else {
	    fprintSegment( f, 0, "<xsd:minInclusive value=\"%d\"/>\n",
			   (int)min );
	}

	/* print maximum value */
	if( useDecPoint ) {
	    fprintSegment( f, 0, "<xsd:maxInclusive value=\"%d.%d\"/>\n", 
			   (int)max / smiPow( 10, offset ), 
			   abs( (int)max % smiPow( 10, offset ) ) );
	} else {
	    fprintSegment( f, 0, "<xsd:maxInclusive value=\"%d\"/>\n",
			   (int)max );
	}
	
	fprintSegment(f, -1, "</xsd:restriction>\n");
	break;
    }
    
    case SMI_BASETYPE_OCTETSTRING:
    {
	SmiInteger32  minLength, maxLength;
	unsigned int numSubRanges = 0;
	
	minLength = 0;
	maxLength = -1;

	/* get range details */
	for( smiRange = smiGetFirstRange( smiType );
	     smiRange;
	     smiRange = smiGetNextRange( smiRange ) ) {
	    if( minLength == 0 ||
		smiRange->minValue.value.integer32 < minLength ) {
		minLength = smiRange->minValue.value.integer32;	     
	    }
	    if( smiRange->maxValue.value.integer32 > maxLength ) {
		maxLength = smiRange->maxValue.value.integer32;
	    }
	    numSubRanges++;
	}

	

	if( smiType->format &&
	    ( smiType->decl == SMI_DECL_IMPLICIT_TYPE ||
	      smiType->decl == SMI_DECL_TEXTUALCONVENTION ) &&
	    ! dhInParent( smiType ) ) {
	    /*
	    fprintStringUnion( f, indent, smiType,
			       minLength, maxLength, 0, NULL );
	    */
	    char *pattern;
	    
	    fprintSegment( f, 1, "<xsd:restriction base=\"xsd:string\">\n" );

	    /* create regexp */
	    pattern = smiFormatToPattern(smiType->format,
					 smiGetFirstRange(smiType));
	    if (pattern) {
		fprintSegment( f, 0, "<xsd:pattern value=\"%s\"/>\n", pattern);
		xfree(pattern);
	    }
	    else {
		fprintf( f, "<!-- Warning: repeat in display hint. "
			 "This feature is not supported. -->\n" );
	    }
	    fprintSegment( f, -1, "</xsd:restriction>\n");
	}
	else {
	    SmiType *parent = smiGetParentType( smiType );
	    /*
	    fprintStringUnion( f, indent, smiType,
			       minLength, maxLength, secondTime,
			       smiType->name );
	    */
	    if( parent ) {
		if(  parent->format ) {
		    char *pattern;

		    pattern = smiFormatToPattern(parent->format,
						 smiGetFirstRange(smiType));
		    if (pattern) {
			fprintSegment( f, 1, "<xsd:restriction base=\"xsd:string\">\n" );
			fprintSegment(f, 0, "<xsd:pattern value=\"%s\"/>\n",
				      pattern);
			fprintSegment( f, -1, "</xsd:restriction>\n");
			xfree(pattern);
		    }
		}
		
		
		else if( smiType->name &&
			 ! strcmp( smiType->name, "IpAddress" ) ) {
		    SmiUnsigned32 lengths[] = {4, 4};
		    lengths[0] = 4; lengths[1] = 4;
		    fprintSegment( f, 1, "<xsd:restriction base=\"xsd:string\">\n" );
		    fprintSegment( f, 0, "<xsd:pattern "
				   "value=\"(0|[1-9](([0-9]){0,2}))."
				   "(0|[1-9](([0-9]){0,2}))."
				   "(0|[1-9](([0-9]){0,2}))."
				   "(0|[1-9](([0-9]){0,2}))\"/>\n" );
		    fprintSegment( f, -1, "</xsd:restriction>\n");
		}
		
		else {
		    
		    
		    char *prefix = getTypePrefix( parent->name );

		    if( prefix ) {
			fprintSegment( f, 1, "<xsd:restriction base=\"%s:%s\">\n",
				       prefix, parent->name );
		    } else {
			fprintSegment( f, 1, "<xsd:restriction base=\"%s\">\n",
				       parent->name );
		    }

		    /* print length restriction */
		    if( minLength > 0 )
			fprintSegment( f, 0, "<xsd:minLength value=\"%d\"/>\n",
				       (int)minLength );
		    if( maxLength > -1 )
			fprintSegment( f, 0, "<xsd:maxLength value=\"%d\"/>\n",
				       (int)maxLength );		    
		    fprintSegment( f, -1, "</xsd:restriction>\n");
		}
		    

	    }
	}
	break;
    }

    case SMI_BASETYPE_FLOAT128:
    {
/*	SmiFloat128 min, max; */
	fprintStdRestHead( f, smiType );
	
	/* xxx, only SMIng */
	break;
    }

    case SMI_BASETYPE_FLOAT64:
    {
/*	SmiFloat64 min,max;*/
	fprintStdRestHead( f, smiType );

	/* xxx, only SMIng */
	break;
    }
    
    case SMI_BASETYPE_FLOAT32:
    {
/*	SmiFloat32 min,max;*/
	fprintStdRestHead( f, smiType );
	
	/* xxx, only SMIng */
	break;
    }
    
    case SMI_BASETYPE_INTEGER64:
    {
/*	SmiInteger64 min,max;*/
	fprintStdRestHead( f, smiType );
	
	/* xxx, only SMIng */
	break;
    }

    case SMI_BASETYPE_UNSIGNED64:
    {
	SmiUnsigned64 min, max;

	min = SMI_BASETYPE_UNSIGNED64_MIN;
	max = SMI_BASETYPE_UNSIGNED64_MAX;

	fprintStdRestHead( f, smiType );
	
	smiRange = smiGetFirstRange( smiType );
	while( smiRange ) {
	    if( smiRange->minValue.value.unsigned64 < min ) {
		min = smiRange->minValue.value.unsigned64;
	    }
	    if( smiRange->maxValue.value.unsigned64 > max ) {
		max = smiRange->maxValue.value.unsigned64;
	    }
	    smiRange = smiGetNextRange( smiRange );
	}
	fprintSegment( f, 0, "<xsd:minInclusive value=\"%lu\"/>\n",
		       (unsigned long)min );

	fprintSegment( f, 0, "<xsd:maxInclusive value=\"%lu\"/>\n",
		       (unsigned long)max );
	
	fprintSegment(f, -1, "</xsd:restriction>\n");
	
	break;
    }

    case SMI_BASETYPE_UNSIGNED32:
    {
	SmiUnsigned32 min, max;

	min = 0;
	max = 4294967295UL;

	fprintStdRestHead( f, smiType );
	
	smiRange = smiGetFirstRange( smiType );
	while( smiRange ) {
	    if( smiRange->minValue.value.unsigned32 < min ) {
		min = smiRange->minValue.value.unsigned32;
	    }
	    if( smiRange->maxValue.value.unsigned32 > max ) {
		max = smiRange->maxValue.value.unsigned32;
	    }
	    smiRange = smiGetNextRange( smiRange );
	}
	fprintSegment( f, 0, "<xsd:minInclusive value=\"%u\"/>\n",
		       (unsigned int)min );

	fprintSegment( f, 0, "<xsd:maxInclusive value=\"%u\"/>\n",
		       (unsigned int)max );
	
	fprintSegment(f, -1, "</xsd:restriction>\n");
	break;
    }

    case SMI_BASETYPE_ENUM:
    case SMI_BASETYPE_BITS:
    {
	SmiNamedNumber *nn;
	
	fprintSegment(f, 1, "<xsd:restriction base=\"xsd:NMTOKEN\">\n");

	/* iterate named numbers */
	for( nn = smiGetFirstNamedNumber( smiType );
	     nn;
	     nn = smiGetNextNamedNumber( nn ) ) {
	    fprintNamedNumber( f, nn );
	}
	fprintSegment(f, -1, "</xsd:restriction>\n");
	break;
    }

    case SMI_BASETYPE_OBJECTIDENTIFIER:
	fprintSegment( f, 0,
		       "<xsd:restriction base=\"smi:ObjectIdentifier\"/>\n");
	break;
    case SMI_BASETYPE_UNKNOWN:
	/* should not occur */
	break;
    case SMI_BASETYPE_POINTER:
	/* TODO */
	break;
    }
}
    

static unsigned int getNamedNumberCount( SmiType *smiType )
{
    SmiNamedNumber *nn;
    unsigned int ret = 0;

    for( nn = smiGetFirstNamedNumber( smiType );
	 nn;
	 nn = smiGetNextNamedNumber( nn ) ) {
	ret++;
    }

    return ret;
}


static void fprintBitList( FILE *f, SmiType *smiType )
{
    fprintSegment( f, 1, "<xsd:restriction>\n" );
    fprintSegment( f, 1, "<xsd:simpleType>\n" );
    fprintSegment( f, 1, "<xsd:list>\n" );  
    fprintSegment( f, 1, "<xsd:simpleType>\n" );
    fprintRestriction( f, smiType );
    fprintSegment( f, -1, "</xsd:simpleType>\n" );
    fprintSegment( f, -1, "</xsd:list>\n" );
    fprintSegment( f, -1, "</xsd:simpleType>\n");
    fprintSegment( f, 0, "<xsd:maxLength value=\"%d\"/>\n",
		   getNamedNumberCount( smiType ) );
    fprintSegment( f, -1, "</xsd:restriction>\n");
}
static int getNumSubRanges( SmiType *smiType )
{
    SmiRange *smiRange;
    int num = 0;

    for( smiRange = smiGetFirstRange( smiType );
	 smiRange;
	 smiRange = smiGetNextRange( smiRange ) ) {
	num++;
    }
    
    return num;
}


static void fprintSubRangeType( FILE *f,
				SmiRange *smiRange, SmiType *smiType )
{
    
    switch( smiType->basetype ) {

    case SMI_BASETYPE_UNSIGNED32: {
	SmiUnsigned32 min, max;

	min = 0;
	max = 4294967295UL;

	if( smiRange->minValue.value.unsigned32 < min ) {
	    min = smiRange->minValue.value.unsigned32;
	}
	if( smiRange->maxValue.value.unsigned32 > max ) {
	    max = smiRange->maxValue.value.unsigned32;
	}
	
	fprintSegment( f, 1, "<xsd:simpleType>\n");
	fprintStdRestHead( f, smiType );

	fprintSegment( f, 0, "<xsd:minInclusive value=\"%u\"/>\n",
		       (unsigned int)min );
	
	fprintSegment( f, 0, "<xsd:maxInclusive value=\"%u\"/>\n",
		       (unsigned int)max );

	fprintSegment(f, -1, "</xsd:restriction>\n");
	fprintSegment(f, -1, "</xsd:simpleType>\n");
	break;
    }

    case SMI_BASETYPE_INTEGER32: {
	SmiInteger32 min, max;

	min = SMI_BASETYPE_INTEGER32_MIN;
	max = SMI_BASETYPE_INTEGER32_MAX;

	if( min == SMI_BASETYPE_INTEGER32_MIN ||
	    smiRange->minValue.value.integer32 < min ) {
	    min = smiRange->minValue.value.integer32;
	}
	if( max == SMI_BASETYPE_INTEGER32_MAX ||
	    smiRange->maxValue.value.integer32 > max ) {
	    max = smiRange->maxValue.value.integer32;
	}

	fprintSegment( f, 1, "<xsd:simpleType>\n");
    	fprintStdRestHead( f, smiType );
	
	fprintSegment( f, 0, "<xsd:minInclusive value=\"%d\"/>\n", (int)min );

	fprintSegment( f, 0, "<xsd:maxInclusive value=\"%d\"/>\n", (int)max );

	fprintSegment(f, -1, "</xsd:restriction>\n");	
	fprintSegment(f, -1, "</xsd:simpleType>\n");
	break;
	
    }

    case SMI_BASETYPE_OCTETSTRING: {
	SmiInteger32  minLength, maxLength;
	
	minLength = 0;
	maxLength = -1;
	
	if( smiRange->minValue.value.integer32 < minLength ) {
	    minLength = smiRange->minValue.value.integer32;	     
	}
	if( smiRange->maxValue.value.integer32 > maxLength ) {
	    maxLength = smiRange->maxValue.value.integer32;
	}
	fprintHexOrAsciiType( f, smiType,
			      minLength, maxLength, NULL, 1 );
	break;
    }

    case SMI_BASETYPE_FLOAT128:
    {
/*	SmiFloat128 min, max; 
	xxx, only SMIng */
	break;
    }

    case SMI_BASETYPE_FLOAT64:
    {
/*	SmiFloat64 min,max;
	xxx, only SMIng */
	break;
    }
    
    case SMI_BASETYPE_FLOAT32:
    {
/*	SmiFloat32 min,max;
	xxx, only SMIng */
	break;
    }

    case SMI_BASETYPE_INTEGER64:
    {
/*	SmiInteger64 min,max;
	 xxx, only SMIng */
	break;
    }

    case SMI_BASETYPE_UNSIGNED64:
    {
	SmiUnsigned64 min, max;

	min = SMI_BASETYPE_UNSIGNED64_MIN;
	max = SMI_BASETYPE_UNSIGNED64_MAX;

	if( smiRange->minValue.value.unsigned64 < min ) {
	    min = smiRange->minValue.value.unsigned64;
	}
	if( smiRange->maxValue.value.unsigned32 > max ) {
	    max = smiRange->maxValue.value.unsigned64;
	}
	
	fprintSegment( f, 1, "<xsd:simpleType>\n");
	fprintStdRestHead( f, smiType );

	fprintSegment( f, 0, "<xsd:minInclusive value=\"%lu\"/>\n",
		       (unsigned long)min );

	fprintSegment( f, 0, "<xsd:maxInclusive value=\"%lu\"/>\n",
		       (unsigned long)max );

	fprintSegment(f, -1, "</xsd:restriction>\n");
	fprintSegment(f, -1, "</xsd:simpleType>\n");
	break;
    }

    case SMI_BASETYPE_ENUM:
    case SMI_BASETYPE_BITS:
    case SMI_BASETYPE_OBJECTIDENTIFIER:
    case SMI_BASETYPE_UNKNOWN:
    case SMI_BASETYPE_POINTER:
	/* should not occur */
	break;
	
    }
}

static void fprintDisplayHint( FILE *f, char *format )
{
    fprintSegment( f, 0, "<displayHint>%s</displayHint>\n", format );
}

static void fprintLengths(FILE *f, SmiType *smiType)
{
    SmiRange *smiRange = smiGetFirstRange(smiType);

    if (! smiRange) {
	return;
    }
    
    fprintSegment(f, 1, "<lengths>\n");
    for (smiRange = smiGetFirstRange(smiType);
	 smiRange; smiRange = smiGetNextRange(smiRange)) {
	fprintSegment(f, 0, "<length min=\"%u\" max=\"%u\"/>\n",
		      smiRange->minValue.value.unsigned32,
		      smiRange->maxValue.value.unsigned32);
    }
    fprintSegment( f, -1, "</lengths>\n");
}


static void fprintTypedef(FILE *f, SmiType *smiType, const char *name)
{
    SmiRange *smiRange;
    unsigned int numSubRanges = getNumSubRanges( smiType );
    
    if ( name ) {
	fprintSegment(f, 1, "<xsd:simpleType name=\"%s\">\n", name);
    }

    else {
	/* unnamed simple type */
	fprintSegment(f, 1, "<xsd:simpleType>\n");
    }

    if( smiType->description ) {
	fprintSegment( f, 1, "<xsd:annotation>\n");
	fprintDocumentation(f, smiType->description);
	if( smiType->format ) {
	    fprintSegment( f, 1, "<xsd:appinfo>\n");
	    fprintDisplayHint( f, smiType->format );
	    if( smiType->basetype == SMI_BASETYPE_OCTETSTRING ) {
	      fprintLengths( f, smiType );
	    }
	    fprintSegment( f, -1, "</xsd:appinfo>\n");
	}
	fprintSegment( f, -1, "</xsd:annotation>\n");
    }
        
    if( ( numSubRanges > 1 ) &&
	     ( smiType->basetype != SMI_BASETYPE_OCTETSTRING ) ) {
	
	fprintSegment( f, 1, "<xsd:union>\n");
	
	for( smiRange = smiGetFirstRange( smiType );
	     smiRange;
	     smiRange = smiGetNextRange( smiRange ) ) {
	    fprintSubRangeType( f, smiRange, smiType );
	}
	
	fprintSegment( f, -1, "</xsd:union>\n");
    }
    else if( smiType->basetype == SMI_BASETYPE_BITS ) {
	fprintBitList( f, smiType );
    }
    else {
	fprintRestriction(f, smiType );
    }
    fprintSegment(f, -1, "</xsd:simpleType>\n");

    /* print an empty line after global types */
    if( smiType->decl != SMI_DECL_IMPLICIT_TYPE && name ) {
	fprintf( f, "\n" );
    } 
}


static char* getTypePrefix( char *typeName )
{
    TypePrefix *iterTPr;

    if( !typeName ) {
	return NULL;
    }

    for( iterTPr = typePrefixes; iterTPr; iterTPr = iterTPr->next ) {
	if( ! strcmp( iterTPr->type, typeName ) ) {
	    return iterTPr->prefix;
	}
    }

    return NULL;
}


static void fprintAnnotationElem( FILE *f, SmiNode *smiNode ) {
    int i;
    
    fprintSegment( f, 1, "<xsd:annotation>\n");
    fprintSegment( f, 1, "<xsd:appinfo>\n");

    if( smiNode->nodekind == SMI_NODEKIND_ROW &&
	( smiNode->implied || smiNode->create ) ) {
	fprintSegment( f, 0, "<flags" );
	if( smiNode->implied ) {
	    fprintf( f, " implied=\"yes\"" );
	}
	if( smiNode->create ) {
	    fprintf( f, " create=\"yes\"" );
	}
	fprintf( f, "/>\n" );
    }
    
    fprintSegment( f, 0, "<maxAccess>%s</maxAccess>\n",
		   getStringAccess( smiNode->access ) );
    fprintSegment( f, 0, "<oid>");
    for (i = 0; i < smiNode->oidlen; i++) {
	fprintf(f, i ? ".%u" : "%u", smiNode->oid[i]);
    }
    fprintf( f, "</oid>\n" );

    fprintSegment( f, 0, "<status>%s</status>\n",
		   getStringStatus( smiNode->status ) );
    if( smiNode->value.basetype != SMI_BASETYPE_UNKNOWN ) {
	char *defval = smiRenderValue( &smiNode->value,
				       smiGetNodeType( smiNode ),
				       SMI_RENDER_FORMAT | SMI_RENDER_NAME );
	fprintSegment( f, 0, "<default>%s</default>\n", defval );

    }


    if( smiNode->format ) {
	fprintDisplayHint( f, smiNode->format );
    }

    if( smiNode->units ) {
	fprintSegment( f, 0, "<units>%s</units>\n", smiNode->units );
    }
  
    fprintSegment( f, -1, "</xsd:appinfo>\n");
    fprintDocumentation( f, smiNode->description );
    fprintSegment( f, -1, "</xsd:annotation>\n");
    
}

static int hasChildren( SmiNode *smiNode, SmiNodekind nodekind )
{
    SmiNode *iterNode;
    int childNodeCount = 0; 

    for( iterNode = smiGetFirstChildNode( smiNode );
	 iterNode;
	 iterNode = smiGetNextChildNode( iterNode ) ){
	if( nodekind & iterNode->nodekind ) {
	    childNodeCount++;
	}
    }
    return childNodeCount;
}

static void
fprintTypeWithHint( FILE *f, SmiNode *smiNode, SmiType *smiType, char *hint )
{
    char *pattern;
    
    fprintSegment( f, 1, "<xsd:simpleType>\n");
    fprintSegment( f, 1, "<xsd:annotation>\n");
    fprintSegment( f, 1, "<xsd:appinfo>\n");
    fprintDisplayHint( f, hint );
    fprintSegment( f, -1, "</xsd:appinfo>\n");
    fprintSegment( f, -1, "</xsd:annotation>\n");
    fprintSegment( f, 1, "<xsd:restriction base=\"xsd:string\">\n");

    pattern = smiFormatToPattern(hint, smiGetFirstRange(smiType));
    if (pattern) {
        fprintSegment( f, 0, "<xsd:pattern value=\"%s\"/>\n", pattern);
	xfree(pattern);
    }
    fprintSegment( f, -1, "</xsd:restriction>\n");
    fprintSegment( f, -1, "</xsd:simpleType>\n");
}


static char *getParentDisplayHint( SmiType *smiType )
{
    SmiType *iterType;

    for( iterType = smiGetParentType( smiType );
	 iterType;
	 iterType = smiGetParentType( iterType ) ) {
	if( iterType->format ) {
	    return iterType->format;
	}	
    }
    return NULL;
}


static void fprintIndexAttr( FILE *f, SmiNode *smiNode, SmiNode *augments )
{
    char *typeName, *prefix;
    SmiType *smiType;

    smiType = smiGetNodeType( smiNode );
    if( !smiType ) {
/*	fprint( f, "<!-- error: no type in %s -->\n", smiNode->name );*/
	return;
    }

    typeName = smiType->name ?
	smiType->name :
	getStringBasetype( smiType->basetype );
    prefix = getTypePrefix( typeName );
    

    if( smiType->basetype == SMI_BASETYPE_BITS ) {
	    fprintSegment( f, 1, "<xsd:attribute name=\"%s\" type=\"%s%s\" "
			   "use=\"required\">\n",
			   smiNode->name,
			   smiNode->name,
			   getStringBasetype( smiType->basetype ) );
	    fprintAnnotationElem( f, smiNode );
    }

    else if( smiType->basetype == SMI_BASETYPE_OCTETSTRING ) {	    
	
	if( smiType->decl == SMI_DECL_IMPLICIT_TYPE ) {
	    char *hint = getParentDisplayHint( smiType );
	    
	    fprintSegment( f, 1, "<xsd:attribute name=\"%s\" "
			   "use=\"required\">\n", smiNode->name );
	    fprintAnnotationElem( f, smiNode );
	    if( ! hint ) {
		fprintTypedef( f, smiType, NULL );
	    }
	    else {
		fprintTypeWithHint( f, smiNode, smiType, hint );
	    }
	}
	
	else {
	    if( prefix ) {
		fprintSegment( f, 1, "<xsd:attribute name=\"%s\" "
			       "type=\"%s:%s\" use=\"required\">\n",
			       smiNode->name, prefix, typeName );
	    }
	    else {		    
		fprintSegment( f, 1, "<xsd:attribute name=\"%s\" "
			       "type=\"%s\" use=\"required\">\n",
			       smiNode->name, typeName );
	    }
	    fprintAnnotationElem( f, smiNode );
	}
    }
    
    /* check for other (implicit) types */
    else if( smiType->decl == SMI_DECL_IMPLICIT_TYPE ) {
	fprintSegment( f, 1, "<xsd:attribute name=\"%s\" "
		       "use=\"required\">\n",
		       smiNode->name );
	fprintAnnotationElem( f, smiNode );
	fprintTypedef( f, smiType, NULL );
    }
    
    else {	
	if( prefix ) {
	    fprintSegment( f, 1,"<xsd:attribute name=\"%s\" type=\"%s:%s\" ",
			   smiNode->name, prefix, typeName );
	    fprintf( f, "use=\"required\">\n" );
	}
	else {
	    fprintSegment( f, 1, "<xsd:attribute name=\"%s\" type=\"%s\" ",
			   smiNode->name, typeName );
	    fprintf( f, "use=\"required\">\n" );
	}
	
	if( augments ) {
	    fprintSegment( f, 1, "<xsd:annotation>\n");
	    fprintSegment( f, 1, "<xsd:appinfo>\n");
	    fprintSegment( f, 0, "<augments>%s</augments>\n", augments->name );
	    fprintSegment( f, -1, "</xsd:appinfo>\n");
	    fprintSegment( f, -1, "</xsd:annotation>\n");
	}
	else {
	    fprintAnnotationElem( f, smiNode );
	    }
    }
    fprintSegment( f, -1, "</xsd:attribute>\n"); 
}

static int containsIndex( SmiNode *parentNode, SmiNode *idxNode )
{
    SmiElement *iterElement;


    for( iterElement = smiGetFirstElement( parentNode );
	 iterElement;
	 iterElement = smiGetNextElement( iterElement ) ) {
	SmiNode *iterNode = smiGetElementNode( iterElement );	
	if( iterNode == idxNode )
	    return 1;
    }
    return 0;
}

static void fprintIndex( FILE *f,
			 SmiNode *smiNode, SmiNode *augments, SmiNode *parent )
{
    SmiNode *iterNode;
    SmiElement *iterElem;
  
    /* iterate INDEX columns */
    for( iterElem = smiGetFirstElement( smiNode );
	 iterElem;
	 iterElem = smiGetNextElement( iterElem ) ) {
	iterNode = smiGetElementNode( iterElem );
	if( ! parent || (parent && !containsIndex( parent, iterNode ) ) ) {
	    fprintIndexAttr( f, iterNode, augments );
	}
    }

    /* print AUGMENTS-clause */
    iterNode = smiGetRelatedNode( smiNode );
    if( iterNode ) {
	fprintIndex( f, iterNode, iterNode, NULL );
    }
}

/* counts index elements of a table row node */
static int numIndex( SmiNode *smiNode )
{
    SmiElement *iterElem;
    int ret = 0;

    for( iterElem = smiGetFirstElement( smiNode );
	 iterElem;
	 iterElem = smiGetNextElement( iterElem ) ) {
	ret++;
    }
    return ret;
}


/* checks if the second node is a subtable of the first node */
static int
isSubTable( SmiNode *smiNode, SmiNode *subNode )
{
    SmiElement *iterElement;
    unsigned int numIdx = numIndex( smiNode ), numSubIdx = numIndex( subNode );

    /* compare number of index elements */
    if( numSubIdx <= numIdx ) {
	/* does not have more index elements --> no subtable */
	return 0;
    }

    /* compare all index elements */
    for( iterElement = smiGetFirstElement( smiNode );
	 iterElement;
	 iterElement = smiGetNextElement( iterElement ) ) {
	SmiElement *iterSubElement = smiGetFirstElement( subNode );
	SmiNode *iterSubNode;
	SmiNode *idxNode = smiGetElementNode( iterElement );
	
	for( iterSubElement = smiGetFirstElement( subNode );
	     iterSubElement;
	     iterSubElement = smiGetNextElement( iterSubElement ) ) {
	    
	    iterSubNode = smiGetElementNode( iterSubElement );
	    if( idxNode == iterSubNode ){
		return 1;
	    }
	}
    }
    return 0;
}


static void fprintComplexType( FILE *f, SmiNode *smiNode, const char *name,
			       SmiNode *parent )
{
    SmiNode *iterNode;
    int numChildren;
    
    if( name ) {
	fprintSegment( f, 1, "<xsd:complexType name=\"%sType\">\n",
		       smiNode->name );
    } else {
	fprintSegment( f, 1, "<xsd:complexType>\n" );
    }

/*    fprintAnnotationElem( f, smiNode ); */

    numChildren = hasChildren( smiNode, SMI_NODEKIND_ANY );

    fprintSegment( f, 1, "<xsd:sequence>\n");

    /* print child elements */
    for( iterNode = smiGetFirstChildNode( smiNode );
	 iterNode;
	 iterNode = smiGetNextChildNode( iterNode ) ) {
	
	fprintElement( f, iterNode, NULL );
	
    }

    /* print augmentations */
    if( nestAugmentedTables ) {
	for( iterNode = smiGetFirstNode( smiGetNodeModule( smiNode ),
					 SMI_NODEKIND_ROW );
	     iterNode;
	     iterNode = smiGetNextNode( iterNode, SMI_NODEKIND_ROW ) ) {
	    SmiNode *augmNode = smiGetRelatedNode( iterNode );
	    if( augmNode == smiNode ) {
		SmiNode *augIterNode;
		for( augIterNode = smiGetFirstChildNode( iterNode );
		     augIterNode;
		     augIterNode = smiGetNextChildNode( augIterNode ) ) {
		    
		    fprintElement( f, augIterNode, NULL );
		}
	    }
	}
    }
    
    /* print subtables */
    if( nestSubtables ) {
	for( iterNode = smiGetFirstNode( smiGetNodeModule( smiNode ),
					 SMI_NODEKIND_ROW );
	     iterNode;
	     iterNode = smiGetNextNode( iterNode, SMI_NODEKIND_ROW ) ) {
	    if( isSubTable( smiNode, iterNode ) ) {
/*	    fputs( "<!-- Here BEGIN subtable entry -->\n", f );*/
		fprintElement( f, iterNode, smiNode );
/*	    fputs( "<!-- Here END subtable entry -->\n", f );*/
	    }
	}
    }
    
    fprintSegment( f, -1, "</xsd:sequence>\n");
    fprintIndex( f, smiNode, NULL, parent );
    
    fprintSegment( f, -1, "</xsd:complexType>\n");
    if( name ) {
	/* we are printing out a global type,
	   so let's leave a blank line after it. */
	fprintf( f, "\n" );
    }

    for( iterNode = smiGetFirstChildNode( smiNode );
	 iterNode;
	 iterNode = smiGetNextChildNode( iterNode ) ) {
	if( iterNode->nodekind == SMI_NODEKIND_NODE ) {
	    fprintComplexType( f, iterNode, iterNode->name, NULL );
	}
    }

}    


static void fprintElement( FILE *f, SmiNode *smiNode, SmiNode *parentNode )
{
    switch( smiNode->nodekind ) {
	SmiType *smiType;

    case SMI_NODEKIND_NODE :
	{
	    SmiNode *iterNode;

	    fprintSegment( f, 1, "<xsd:element name=\"%s\">\n", smiNode->name);
	    fprintSegment( f, 1, "<xsd:complexType>\n");
	    fprintSegment( f, 1, "<xsd:sequence>\n");
	    for( iterNode = smiGetFirstChildNode( smiNode );
		 iterNode;
		 iterNode = smiGetNextChildNode( iterNode ) ) {
		if( iterNode->nodekind == SMI_NODEKIND_SCALAR ) {
		    fprintElement( f, iterNode, NULL );
		}
	    }
	    fprintSegment( f, -1, "</xsd:sequence>\n");
	    fprintSegment( f, -1, "</xsd:complexType>\n");
	    fprintSegment( f, -1, "</xsd:element>\n");
	}
	break;
	
    case SMI_NODEKIND_TABLE :
    {
	SmiNode *iterNode;
	
	/* ignore tables and just include their entries */
	for( iterNode = smiGetFirstChildNode( smiNode );
	     iterNode;
	     iterNode = smiGetNextChildNode( iterNode ) ) {
	    fprintElement( f, iterNode, NULL );
	}
	break;
    }

    case SMI_NODEKIND_ROW:
       
	fprintSegment( f, 1, "<xsd:element name=\"%s\" "
		       "minOccurs=\"0\" maxOccurs=\"unbounded\">\n",
		       smiNode->name );
	
	fprintAnnotationElem( f, smiNode );
	
	fprintComplexType( f, smiNode, NULL, parentNode );
	fprintSegment( f, -1, "</xsd:element>\n");	
	break;
    
    case SMI_NODEKIND_SCALAR:
    case SMI_NODEKIND_COLUMN:
    {
	SmiElement *iterElem;
	char *prefix;
	char *typeName;
	
	/* check if we are index column */
	for( iterElem = smiGetFirstElement( smiGetParentNode( smiNode ) ) ;
	     iterElem;
	     iterElem = smiGetNextElement( iterElem ) ) {
	    if( smiNode == smiGetElementNode( iterElem ) ) {
		/* we are index coulumn ==> do not print element */
		return;
	    }
	}
	
	if( smiNode->access < SMI_ACCESS_READ_ONLY ) {
	    /* only print accessible nodes */
	    return;
	}
	
	smiType = smiGetNodeType( smiNode );

	if( smiType->name ) {
	    typeName = smiType->name;
	}
	else {
	    typeName = getStringBasetype( smiType->basetype );
	}
	prefix = getTypePrefix( typeName );

#if 0
	if( smiType->basetype == SMI_BASETYPE_BITS ) {
	    fprintSegment( f, 1, "<xsd:element name=\"%s\" type=\"%s%s\" "
			   "minOccurs=\"0\">\n",
			   smiNode->name,
			   smiNode->name,
			   getStringBasetype( smiType->basetype ) );
	    fprintAnnotationElem( f, smiNode );
	}
	
//	else if( smiType->basetype == SMI_BASETYPE_OCTETSTRING ) {
#endif /* 0 */

	if( smiType->basetype == SMI_BASETYPE_OCTETSTRING ) {
	    if( smiType->decl == SMI_DECL_IMPLICIT_TYPE ) {

		char *hint = getParentDisplayHint( smiType );

		fprintSegment( f, 1, "<xsd:element name=\"%s\" "
			       "minOccurs=\"0\">\n", smiNode->name );
		fprintAnnotationElem( f, smiNode );
		if( ! hint ) {
		    fprintTypedef( f, smiType, NULL );
		}
		else {
		    fprintTypeWithHint( f, smiNode, smiType, hint );
		}
	    }

	    else {
		if( prefix ) {
		    fprintSegment( f, 1, "<xsd:element name=\"%s\" "
				   "type=\"%s:%s\" minOccurs=\"0\">\n",
				   smiNode->name, prefix, typeName );
		}
		else {		    
		    fprintSegment( f, 1, "<xsd:element name=\"%s\" "
				   "type=\"%s\" minOccurs=\"0\">\n",
				   smiNode->name, typeName );
		}
		fprintAnnotationElem( f, smiNode );
	    }	   	   
	}

	else if( smiType->decl == SMI_DECL_IMPLICIT_TYPE ) {
	    fprintSegment( f, 1, "<xsd:element name=\"%s\" minOccurs=\"0\">\n",
			   smiNode->name );
	    fprintAnnotationElem( f, smiNode );
	    fprintTypedef( f, smiType, NULL );
	}
	
	else {
	    if( prefix ) {
		fprintSegment( f, 1, "<xsd:element name=\"%s\" type=\"%s:%s\" "
			       "minOccurs=\"0\">\n",
			       smiNode->name, prefix, typeName );
	    }
	    else {
		fprintSegment( f, 1, "<xsd:element name=\"%s\" type=\"%s\" "
			       "minOccurs=\"0\">\n",
			       smiNode->name, typeName );
	    }
	    fprintAnnotationElem( f, smiNode );
	}
	fprintSegment( f, -1, "</xsd:element>\n"); 
	break;
    }

    case SMI_NODEKIND_NOTIFICATION:
	fprintSegment( f, 0, "<xsd:element name=\"%s\"/>\n", smiNode->name );
	break;

    default:
	fprintf( f, "<!-- Warning! Unhandled Element! No details available!\n");
	fprintf( f, "      Nodekind: %#4x -->\n\n", smiNode->nodekind );
	
    }
}


static void fprintImplicitTypes( FILE *f, SmiModule *smiModule )
{
    SmiNode *iterNode;
    SmiType *smiType;

    for(iterNode = smiGetFirstNode(smiModule,
				   SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN);
	iterNode;
	iterNode = smiGetNextNode(iterNode,
				  SMI_NODEKIND_SCALAR | SMI_NODEKIND_COLUMN)) {
	smiType = smiGetNodeType( iterNode );
	if( smiType ) {
		switch( smiType->basetype ) {
		    
		case SMI_BASETYPE_BITS:
		    if( ! getTypePrefix( smiType->name ) ) {
			fprintTypedef( f, smiType, iterNode->name );
			break;
		    }

		case SMI_BASETYPE_OCTETSTRING:
#if 0
		    if( smiType->decl == SMI_DECL_IMPLICIT_TYPE ) {
			fprintTypedef( f, INDENT, smiType, iterNode->name );
		    }
#endif /* 0 */
		    break;
		default:
		    break;
		}
	}
    }
}


#if 0
static void fprintRows( FILE *f, SmiModule *smiModule )
{
    SmiNode *iterNode;

    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_ROW );
	 iterNode;
	 iterNode = smiGetNextNode( iterNode,  SMI_NODEKIND_ROW ) ) {
	if( hasChildren( iterNode, SMI_NODEKIND_COLUMN | SMI_NODEKIND_TABLE ) ){
	    fprintElement( f, iterNode, NULL );
	}
    }
}
#endif

static void fprintImports( FILE *f, SmiModule *smiModule )
{
    SmiImport *iterImp;
    char *lastModName = "";
    
    fprintSegment( f, 0, "<xsd:import namespace=\"%ssmi\" schemaLocation=\"%ssmi.xsd\"/>\n", schemaLocation, schemaLocation );
    for( iterImp = smiGetFirstImport( smiModule );
	 iterImp;
	 iterImp = smiGetNextImport( iterImp ) ) {
	/* assume imports to be ordered by module names */
	if( strcmp( iterImp->module, lastModName ) ) {
	    fprintSegment( f, 0, "<xsd:import ");
	    fprintf( f, "namespace=\"%s%s\" schemaLocation=\"%s%s.xsd\"/>\n",
		    schemaLocation, iterImp->module,
		    schemaLocation, iterImp->module );
	}
	lastModName = iterImp->module;
    }
    fprintf( f, "\n");
   
}


/*
 * Check if given table io a sub table of another table.
 * If so, its parent table is returned (NULL otherwise).
 */
static SmiNode *isASubTable( SmiNode *smiNode, SmiModule *smiModule )
{
    SmiNode *iterNode;
    int numIdxDiff = -1;
    SmiNode *retNode = NULL;
    
    for( iterNode = smiGetFirstNode( smiModule,
				     SMI_NODEKIND_ROW );
	 iterNode;
	 iterNode = smiGetNextNode( iterNode,
				    SMI_NODEKIND_ROW ) ) {
	
	if( isSubTable( iterNode, smiNode ) ) {

	    if( (numIdxDiff == -1) ||
		((numIndex( smiNode ) - numIndex( iterNode )) < numIdxDiff) ) {
		retNode = iterNode;
		numIdxDiff = numIndex( smiNode ) - numIndex( iterNode );
	    }
	    
	}
    }
    return retNode;
}


static void fprintGroupTypes( FILE *f, SmiModule *smiModule )
{
    SmiNode *iterNode, *iterNode2;

    /* scalar groups */
    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_NODE );
	 iterNode;
	 iterNode = smiGetNextNode( iterNode, SMI_NODEKIND_NODE ) ) {
	if( hasChildren( iterNode, SMI_NODEKIND_SCALAR ) ) {
	    fprintSegment(f, 1, "<xsd:complexType name=\"%sType\">\n",
			  iterNode->name);
	    fprintSegment( f, 1, "<xsd:sequence>\n");
	    for( iterNode2 = smiGetFirstChildNode( iterNode );
		 iterNode2;
		 iterNode2 = smiGetNextChildNode( iterNode2 ) ) {
		if( iterNode2->nodekind == SMI_NODEKIND_SCALAR ) {
		    fprintElement( f, iterNode2, NULL );
		}
	    }
	    fprintSegment( f, -1, "</xsd:sequence>\n");
	    fprintSegment(f, -1, "</xsd:complexType>\n");
	}
    }   

    /* rows */
    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_ROW );
	 iterNode;
	 iterNode = smiGetNextNode( iterNode,  SMI_NODEKIND_ROW ) ) {
	if( hasChildren( iterNode,
			 SMI_NODEKIND_COLUMN | SMI_NODEKIND_TABLE ) ){	   

	    /* skip nested subtables here */
	    if( nestSubtables ){
		if( isASubTable( iterNode, smiModule ) != NULL ) {
		    continue;
		}
	    }

	    /* skip table augmentations here */
	    if( nestAugmentedTables ) {
		if( iterNode->indexkind == SMI_INDEX_AUGMENT ) {
		    continue;
		}
	    }
	    
	    fprintComplexType( f, iterNode, iterNode->name, NULL );
	    
	}
    }   
}


#if 0
static void fprintNotifications( FILE *f, SmiModule *smiModule )
{
    SmiNode *iterNode;
    
    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_NOTIFICATION );
	 iterNode;
	 iterNode = smiGetNextNode( iterNode, SMI_NODEKIND_NOTIFICATION ) ) {
	fprintElement( f, iterNode, NULL );
    }
}
#endif


static void fprintModuleHead(FILE *f, SmiModule *smiModule)
{
    if( smiModule->description ) {
	fprintSegment(f, 1, "<xsd:annotation>\n");
	fprintDocumentation(f, smiModule->description);
	fprintSegment(f, -1, "</xsd:annotation>\n\n");
    }
    
}


static void fprintKey( FILE *f, SmiNode *smiNode )
{
    SmiNode *relNode;
    SmiElement *iterElem;
    
    switch( smiNode->indexkind ) {
	
    case SMI_INDEX_INDEX:
	
	/* print key */
/*	fprintSegment( f, 1, "<xsd:key " );
	fprintf( f, "name=\"%sKey\">\n", smiNode->name );
	fprintSegment( f, 0, "<xsd:selector ");
	fprintf( f, "xpath=\"%s\"/>\n", smiNode->name );
	for( iterElem = smiGetFirstElement( smiNode );
	     iterElem;
	     iterElem = smiGetNextElement( iterElem ) ) {
	    SmiNode *indexNode = smiGetElementNode( iterElem );
	    fprintSegment( f, 0, "<xsd:field ");
	    fprintf( f, "xpath=\"@%s\"/>\n", indexNode->name );
	}
	fprintSegment( f, -1, "</xsd:key>\n\n");*/
	break;
	
    case SMI_INDEX_AUGMENT:
	
	/* print keyref */
	fprintSegment( f, 1, "<xsd:keyref " );
	relNode = smiGetRelatedNode( smiNode );
	fprintf( f, "name=\"%sKeyRef\" ", smiNode->name );
	fprintf( f, "refer=\"%sKey\">\n", relNode->name );
	fprintSegment( f, 0, "<xsd:selector ");
	fprintf( f, "xpath=\"%s\"/>\n", smiNode->name );
	for( iterElem = smiGetFirstElement( relNode );
	     iterElem;
	     iterElem = smiGetNextElement( iterElem ) ) {
	    SmiNode *indexNode = smiGetElementNode( iterElem );
	    fprintSegment( f, 0, "<xsd:field ");
	    fprintf( f, "xpath=\"@%s\"/>\n", indexNode->name );
	}
	fprintSegment( f, -1, "</xsd:keyref>\n");
	
	/* print unique clause */
	fprintSegment( f, 1, "<xsd:unique " );
	fprintf( f, "name=\"%sKeyRefUnique\">\n", smiNode->name );
	fprintSegment( f, 0, "<xsd:selector ");
	fprintf( f, "xpath=\"%s\"/>\n", smiNode->name );
	for( iterElem = smiGetFirstElement( relNode );
	     iterElem;
	     iterElem = smiGetNextElement( iterElem ) ) {
	    SmiNode *indexNode = smiGetElementNode( iterElem );
	    fprintSegment( f, 0, "<xsd:field ");
	    fprintf( f, "xpath=\"@%s\"/>\n", indexNode->name );
	}
	fprintSegment( f, -1, "</xsd:unique>\n\n");
	break;

    case SMI_INDEX_REORDER:
    case SMI_INDEX_SPARSE:
    case SMI_INDEX_EXPAND:
	/* SMIng, not implemented yet */
	break;
	
    default:
	fprintf( f, "<!-- Error: Unknown index type -->\n" );
	break;
    }   
}


static void fprintGroupElements(FILE *f, SmiModule *smiModule)
{
    SmiNode *iterNode;

    /* scalar groups */
    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_NODE );
	 iterNode;
	 iterNode = smiGetNextNode( iterNode, SMI_NODEKIND_NODE ) ) {
	if( hasChildren( iterNode, SMI_NODEKIND_SCALAR ) ) {
	    
	    if (container) {
		fprintSegment(f, 1, "<xsd:element name=\"%s\" "
			      "type=\"%s:%sType\" minOccurs=\"0\">\n",
			      iterNode->name,
			      smiModule->name, iterNode->name);
	    } else {
		fprintSegment(f, 1, "<xsd:element name=\"%s\" "
			      "type=\"%sType\" minOccurs=\"0\">\n",
			      iterNode->name, iterNode->name);
	    }
	    fprintAnnotationElem( f, iterNode );
	    fprintSegment( f, -1, "</xsd:element>\n" );
	}
    }   

    /* rows */
    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_ROW );
	 iterNode;
	 iterNode = smiGetNextNode( iterNode,  SMI_NODEKIND_ROW ) ) {
	if( hasChildren( iterNode, SMI_NODEKIND_COLUMN | SMI_NODEKIND_TABLE ) ){
	    /* skip nested subtables here */
	    if( nestSubtables ){
		if( isASubTable( iterNode, smiModule ) != NULL ) {
		    continue;
		}
	    }

	    /* skip table augmentations here */
	    if( nestAugmentedTables ) {
		if( iterNode->indexkind == SMI_INDEX_AUGMENT ) {
		    continue;
		}
	    }

	    if (container) {
		fprintSegment(f, 1, "<xsd:element name=\"%s\" "
			      "type=\"%s:%sType\" minOccurs=\"0\" "
			      "maxOccurs=\"unbounded\">\n",
			      iterNode->name,
			      smiModule->name, iterNode->name);
		fprintKey( f, iterNode );
	    } else {
		fprintSegment(f, 1, "<xsd:element name=\"%s\" "
			      "type=\"%sType\" minOccurs=\"0\" "
			      "maxOccurs=\"unbounded\">\n",
			      iterNode->name, iterNode->name);
	    }
	    fprintAnnotationElem( f, iterNode );
	    fprintSegment( f, -1, "</xsd:element>\n" );
	}
    }   
}


static char *getSubTableXPath( SmiNode *smiNode, SmiModule *smiModule )
{
    char *ret;
    SmiNode *parentTable = isASubTable( smiNode, smiModule );

    if( parentTable ) {
	smiAsprintf( &ret, "%s/%s",
		     getSubTableXPath( parentTable, smiModule ),
		     smiNode->name );
    }
    else {
	smiAsprintf( &ret, "%s", smiNode->name );
    }
    return ret;   
}


static void fprintKeys( FILE *f, SmiModule *smiModule )
{

    SmiNode *iterNode;

    for( iterNode = smiGetFirstNode( smiModule, SMI_NODEKIND_ROW );
	 iterNode;
	 iterNode = smiGetNextNode( iterNode, SMI_NODEKIND_ROW ) ) {

	SmiElement *iterElem;

	/* print only keys for base tables */
	if( iterNode->indexkind != SMI_INDEX_INDEX ) {
	    continue;
	}
	
	/* print key */
	fprintSegment( f, 1, "<xsd:key name=\"%sKey\">\n", iterNode->name );
	fprintSegment( f, 0, "<xsd:selector ");
	fprintf( f, "xpath=\"%s\"/>\n",
		 nestSubtables ?
		 getSubTableXPath( iterNode, smiModule ) : iterNode->name );
	    
	for( iterElem = smiGetFirstElement( iterNode );
	     iterElem;
	     iterElem = smiGetNextElement( iterElem ) ) {
	    SmiNode *indexNode = smiGetElementNode( iterElem );
	    fprintSegment( f, 0, "<xsd:field ");
	    fprintf( f, "xpath=\"@%s\"/>\n", indexNode->name );
	}
	fprintSegment( f, -1, "</xsd:key>\n\n");
    }
    

}


static void fprintContextHead(FILE *f)
{
    fprintSegment( f, 1, "<xsd:element name=\"snmp-data\">\n");
    fprintSegment( f, 1, "<xsd:complexType>\n");
    fprintSegment( f, 1, "<xsd:sequence>\n");
    fprintSegment( f, 1, "<xsd:element name=\"context\" "
		   "minOccurs=\"0\" maxOccurs=\"unbounded\">\n");
    fprintSegment( f, 1, "<xsd:complexType>\n");
    fprintSegment( f, 1, "<xsd:sequence>\n");
}


static void fprintContextFoot(FILE *f, SmiModule **modv, int modc)
{
    int i;
    
    fprintSegment( f, -1, "</xsd:sequence>\n");
    fprintSegment( f, 0, "<xsd:attribute name=\"ipaddr\" type=\"xsd:NMTOKEN\" use=\"required\"/>\n");
    fprintSegment( f, 0, "<xsd:attribute name=\"hostname\" type=\"xsd:NMTOKEN\"/>\n");
    fprintSegment( f, 0, "<xsd:attribute name=\"port\" type=\"xsd:unsignedInt\" use=\"required\"/>\n");
    fprintSegment( f, 0, "<xsd:attribute name=\"community\" type=\"xsd:NMTOKEN\" use=\"required\"/>\n");
    fprintSegment( f, 0, "<xsd:attribute name=\"caching\" type=\"xsd:NMTOKEN\"/>\n");
    fprintSegment( f, 0, "<xsd:attribute name=\"time\" type=\"xsd:dateTime\" use=\"required\"/>\n");
    fprintSegment( f, -1, "</xsd:complexType>\n");
    fprintSegment( f, -1, "</xsd:element>\n");
    fprintSegment( f, -1, "</xsd:sequence>\n");
    fprintSegment( f, -1, "</xsd:complexType>\n");

    for( i=0; i < modc; i++ ) {
	fprintKeys( f, modv[ i ] );
    }
    fprintSegment( f, -1, "</xsd:element>\n\n");
}


static void fprintTypedefs(FILE *f, SmiModule *smiModule)
{
    int		 i;
    SmiType	 *smiType;
    
    for(i = 0, smiType = smiGetFirstType(smiModule);
	smiType;
	i++, smiType = smiGetNextType(smiType)) {
	fprintf(f, "\n");
	fprintTypedef(f, smiType, smiType->name);
    }
}


static void registerType( char *type, char *module )
{
    TypePrefix *oldTPr = NULL, *iterTPr = NULL;

    for( iterTPr = typePrefixes; iterTPr; iterTPr = iterTPr->next ) {
	oldTPr = iterTPr;
    }
    if( ! oldTPr ) {
	/* type prefixes do not exist yet */
	typePrefixes = xmalloc( sizeof( TypePrefix ) );
	typePrefixes->type = type;
	typePrefixes->prefix = module;
	typePrefixes->next = NULL;
    }
    else {
	/* create new TypePrefix */
	oldTPr->next = xmalloc( sizeof( TypePrefix ) );
	oldTPr->next->type = type;
	oldTPr->next->prefix = module;
	oldTPr->next->next = NULL;
    }
}


static void dumpXsdModules(int modc, SmiModule **modv, int flags, char *output)
{
    int  i;
    FILE *f = stdout;
    SmiImport *iterImp;
    char *lastModName = "";

    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    for (i = 0; i < modc; i++) {

	fprintf(f, "<?xml version=\"1.0\"?>\n"); 
	fprintf(f, "<!-- This module has been generated by smidump "
	       SMI_VERSION_STRING ". Do not edit. -->\n");
	
	fputs( "<!-- WARNING: files located at ", f );
	fprintf(f, "%s ", schemaLocation);
	fputs( "are subject to changes. -->\n\n", f );

	fprintSegment(f, 1, "<xsd:schema ");
	fprintf(f, "targetNamespace=\"%s%s\"\n",
		schemaLocation, modv[i]->name);
    
	fprintf(f, "            xmlns=\"%s%s\"\n",
		schemaLocation, modv[i]->name);
/*      fprintf(f, "            xmlns:xmn=\"http://www.w3.org/XML/1998/namespace\"\n"); */
	fprintf(f, "            xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n");
	fprintf(f, "            xmlns:smi=\"%ssmi\"\n", schemaLocation);
    
	for( iterImp = smiGetFirstImport( modv[i] );
	     iterImp;
	     iterImp = smiGetNextImport( iterImp ) ) {
	    registerType( iterImp->name, iterImp->module );
	    /* assume imports to be ordered by module names */
	    if( strcmp( iterImp->module, lastModName ) ) {
		fprintf( f, "            xmlns:%s=\"%s%s\"\n",
			 iterImp->module, schemaLocation, iterImp->module );
	    }
	    lastModName = iterImp->module;
	}
  
	fprintf(f, "            xml:lang=\"en\"\n");
	fprintf(f, "            elementFormDefault=\"qualified\"\n");
	fprintf(f, "            attributeFormDefault=\"unqualified\">\n\n");

	fprintModuleHead(f, modv[i]);
	fprintImports(f, modv[i]);
	fprintContextHead(f);
	fprintGroupElements(f, modv[i]);
	fprintContextFoot(f, modv, 0);
	fprintGroupTypes(f, modv[i]);
	fprintImplicitTypes(f, modv[i]);
	fprintTypedefs(f, modv[i]);

	fprintSegment(f, -1, "</xsd:schema>\n");
    }

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}


static void dumpXsdContainer(int modc, SmiModule **modv, int flags,
			     char *output)
{
    int  i;
    FILE *f = stdout;

    if (output) {
	f = fopen(output, "w");
	if (!f) {
	    fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	    perror(NULL);
	    exit(1);
	}
    }

    fprintf(f, "<?xml version=\"1.0\"?>\n"); 
    fprintf(f, "<!-- This module has been generated by smidump "
	    SMI_VERSION_STRING ". Do not edit. -->\n");
	
    fputs( "<!-- WARNING: files located at ", f );
    fprintf(f, "%s ", schemaLocation);
    fputs( "are subject to changes. -->\n\n", f );
    
    fprintSegment(f, 1, "<xsd:schema ");
    fprintf(f, "targetNamespace=\"%s%s\"\n", schemaLocation, containerBasename);
    fprintf(f, "            xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n");
    fprintf(f, "            xmlns:smi=\"%ssmi\"\n", schemaLocation);
    for (i = 0; i < modc; i++) {
	fprintf(f, "            xmlns:%s=\"%s%s\"\n",
		modv[i]->name, schemaLocation, modv[i]->name);
    }
    
    fprintf(f, "            xml:lang=\"en\"\n");
    fprintf(f, "            elementFormDefault=\"qualified\"\n");
    fprintf(f, "            attributeFormDefault=\"unqualified\">\n\n");

    /* imports */
    for (i = 0; i < modc; i++) {
	fprintSegment( f, 0, "<xsd:import ");
	fprintf( f, "namespace=\"%s%s\" schemaLocation=\"%s%s.xsd\"/>\n",
		 schemaLocation, modv[i]->name,
		 schemaLocation, modv[i]->name);
    }
    fprintf( f, "\n");
    
    /* context */
    fprintContextHead(f);
    for (i = 0; i < modc; i++) {
	/* per module elements */
	fprintGroupElements(f, modv[i]);
    }
    fprintContextFoot(f, modv, modc);

    fprintSegment(f, -1, "</xsd:schema>\n");

    if (fflush(f) || ferror(f)) {
	perror("smidump: write error");
	exit(1);
    }

    if (output) {
	fclose(f);
    }
}


static void dumpXsd(int modc, SmiModule **modv, int flags, char *output)
{
    /* register smi basetypes */
    registerType( "Integer32", "smi" );
    registerType( "ObjectIdentifier", "smi" );
    registerType( "OctetString", "smi" );
    registerType( "Unsigned32", "smi" );
    registerType( "Unsigned64", "smi" );

    /* make sure url ends with '/' */
    if( schemaLocation[ strlen( schemaLocation ) - 1 ] != '/' ) {
	smiAsprintf( &schemaLocation, "%s%c", schemaLocation, '/');
    }
	
    if (container) {
	dumpXsdContainer(modc, modv, flags, output);
    } else {
	dumpXsdModules(modc, modv, flags, output);
    }

    /* delete type-prefix-mapping */
    free( typePrefixes ); /* XXX: TODO: free all malloced types in a loop */
}


void initXsd()
{

    static SmidumpDriverOption opt[] = {
	{ "schema-url", OPT_STRING, &schemaLocation, 0,
	  "URI prefix for schema definitions and namespaces" },
	{ "container", OPT_FLAG, &container, 0,
	  "generate a container schema" },
	{ "nest-augments", OPT_FLAG, &nestAugmentedTables, 0,
	  "Nest rows of augmented tables in the base tables" },
	{ "nest-subtables", OPT_FLAG, &nestSubtables, 0,
	  "Nest subtables in the base tables" },
	{ 0, OPT_END, 0, 0 }
    };
    
    static SmidumpDriver driver = {
	"xsd",
	dumpXsd,
	0,
	SMIDUMP_DRIVER_CANT_UNITE,
	"XML schema definitions",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
