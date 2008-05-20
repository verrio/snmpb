/*
 * fortopat.c --
 *
 *      Implementation of format string to xsd pattern conversion.
 *
 * Copyright (c) 2001 J. Schoenwaelder, Technical University of Braunschweig.
 *           (c) 2002 T. Klie, Technical University of Braunschweig.
 *           (c) 2002 F. Strauss, Technical University of Braunschweig.
 *           (c) 2007 T. Klie, Technical University of Braunschweig.
 *	     (c) 2007 J. Schoenwaelder, Jacobs University Bremen.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-xsd.c 7372 2007-10-17 15:26:40Z tklie $
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


#ifndef MIN
#define MIN(a,b) ((a)) < ((b)) ? ((a)) : ((b))
#endif


/* definition of commonly used display hints */
#define LIBSMI_DH_TP_IPV4Z_ADDR "1d.1d.1d.1d%4d:2d"
#define LIBSMI_DH_UDP_IPV4 "1d.1d.1d.1d/2d"
#define LIBSMI_DH_IPV4Z_ADDR "1d.1d.1d.1d%4d"
#define LIBSMI_DH_IPV4_ADDR "1d.1d.1d.1d"
#define LIBSMI_DH_LDP_ID "1d.1d.1d.1d:2d"
#define LIBSMI_DH_IPV6_ADDR "2x:2x:2x:2x:2x:2x:2x:2x"
#define LIBSMI_DH_IPV6Z_ADDR "2x:2x:2x:2x:2x:2x:2x:2x%4d"
#define LIBSMI_DH_TP_IPV6_ADDR "0a[2x:2x:2x:2x:2x:2x:2x:2x]0a:2d"
#define LIBSMI_DH_TP_IPV6Z_ADDR "0a[2x:2x:2x:2x:2x:2x:2x:2x%4d]0a:2d"
#define LIBSMI_DH_HEX "1x"
#define LIBSMI_DH_HEX_COLON "1x:"
#define LIBSMI_DH_DOUBLE_HEX_COLON "2x:"
#define LIBSMI_DH_ASCII "255a"
#define LIBSMI_DH_UTF8 "255t"
#define LIBSMI_DH_ASCII_CHAR "1a"
#define LIBSMI_DH_DATETIME "2d-1d-1d,1d.1d.1d,1a1d:1d"
#define LIBSMI_DH_DATETIME2 "2d-1d-1d,1d:1d:1d.1d,1a1d:1d"
#define LIBSMI_DH_IPX_ADDR "4x.1x:1x:1x:1x:1x:1x.2d"


/* list of parts of a display hint */
typedef struct DH {
    unsigned int number;
/*    int repeat;    //  repeat not yet supported */
    char type;
    char separator[3];
/*    char repTerm;  // repeat not yet supported */
    struct DH *next;
} DH;





static int
smiPow(int base, unsigned int exponent)
{
    unsigned int i;
    int ret = 1;
    
    if (exponent == 0) {
	return 1;
    }
    
    for (i = 0; i < exponent; i++) {
	ret *= base;
    }
    return ret;
}



static unsigned int
numDigits(unsigned int val)
{
    int ret = 1;

    for (; val / 10; val = val / 10) {
	ret++;
    }
    
    return ret;
}



static int
getBracketLevel(char *bracketString)
{
    int level = 0;
    char *c = bracketString;

    while(level >= 0 && *c != '\0') {
	switch(*c) {
	case '(':
	    level++;
	    break;
	case ')':
	    level--;
	    break;
	default:
	    break;
	}
	c++;
    }

    return level;
}



/* parse a display hint and create a list of DHs */
static struct DH*
parseDH(const char *hint)
{
    struct DH *iterDH = (struct DH *)malloc( sizeof( struct DH ) );
    struct DH *ret = iterDH;
    struct DH *oldIterDH = iterDH;
    unsigned int pos = 0;
    
    if (!ret) {
	return NULL;
    }
    
    memset(iterDH, 0, sizeof(struct DH));
    
    while( pos < strlen( hint ) ) {

	if( ! iterDH ) {
	    iterDH = (struct DH *)malloc( sizeof( struct DH ) );
	    if( ! iterDH ) return NULL;
	    memset(iterDH, 0, sizeof(struct DH));
	    oldIterDH->next = iterDH;
	    oldIterDH = iterDH;
	}

	switch( hint[ pos ] ) {
	    
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9': {
	    unsigned int endPos;
	    char *strNum;
	    
	    /* find end of number */
	    for( endPos = pos; isdigit( hint[ endPos ] ); endPos++ );
	    
	    /* parse number */
	    strNum = (char *)malloc( endPos - pos );
	    strncpy( strNum, &hint[ pos ], endPos - pos );
	    strNum[ endPos - pos ] = '\0';
	    iterDH->number = atoi( strNum );
	    free( strNum );
	    
	    /* forward the position pointer */
	    pos = endPos;
	    break;
	}
	
	case 'a':
	case 'b':
	case 'd':
	case 'o':
	case 't':
	case 'x':
	    iterDH->type = hint[ pos++ ];
	    if( isdigit( hint[ pos ] ) || hint[ pos ] == '*' ) {
		iterDH = iterDH->next;
	    }
	    break;
	    
	case '*':
	    /* repeat not supported */
	    return NULL;
	    
	default:
	    if( iterDH->separator ) {
		/* iterDH->repTerm = hint[ pos++ ]; // repeat not supported */
	      pos++;
	    }
	    else {
              iterDH->separator[0] = hint[ pos++ ];
	      if (iterDH->separator[0] == '.' ) {
		iterDH->separator[0] = '\\';
		iterDH->separator[1] = '.';
	      }
	    }
	    if( isdigit( hint[ pos ] ) || hint[ pos ] == '*' ) {
		iterDH = iterDH->next;
	    }
	    break;
	}	
    }
    return ret;
}

static char*
getSimpleCharFacet_XSD(char dhType, unsigned int number, 
		       unsigned int minL, unsigned int maxL)
{
    char *ret;
    char *baseRegExp;
    
    /* determine base regular expression, depending on display hint type */
    switch( dhType ) {
    case 'a':
	baseRegExp = "\\p{IsBasicLatin}";
	break;
    case 't':
	baseRegExp = ".";
	break;
    default:
	/* XXX - fix this to use the normal error reporting mechanism XXX */
	fprintf(stderr, 
		"displayHint.c: Error: unknown display hint type\n");
	return NULL;
    } 
    
    smiAsprintf( &ret, "%s{%d,%d}", baseRegExp, minL, MIN( number, maxL));
    return ret;
}


/* check if the hint is known and (if so) return the XSD translation */
static char*
getKnownDisplayHint_XSD(const char *hint,
			SmiUnsigned32 *lengths, unsigned int numSubranges)
{
    int i;
    char *ret = "";
    
    if( ! strcmp( hint, LIBSMI_DH_TP_IPV4Z_ADDR ) )
	/* this is a simplyfied restriction. 
	 * We could restrict more, 
	 * but then, the pattern will be very hard to read / understand */
	return xstrdup("(\\d{1,3}.){3}\\d{1,3})%\\d{1,10}:\\d{1,5}");
    
    if( ! strcmp( hint, LIBSMI_DH_UDP_IPV4 ) ) 
	return xstrdup("(\\d{1,3}.){3}\\d{1,3}/\\d{1,5}");
    
    if( ! strcmp( hint, LIBSMI_DH_IPV4Z_ADDR ) )
	return xstrdup("(\\d{1,3}.){3}\\d{1,3}%\\d{1,10}");
    
    if( ! strcmp( hint, LIBSMI_DH_IPV4_ADDR ) )
	return xstrdup("(\\d{1,3}.){3}\\d{1,3}");
    
    if( ! strcmp( hint, LIBSMI_DH_LDP_ID ) )
	return xstrdup("(\\d{1,3}.){3}\\d{1,3}:\\d{1,5}");
    
    if( ! strcmp( hint, LIBSMI_DH_IPV6_ADDR ) )
	return xstrdup("([\\dA-Fa-f]{2}:){7}[\\dA-Fa-f]{2}");
    
    if( ! strcmp( hint, LIBSMI_DH_IPV6Z_ADDR ) )
	return xstrdup("([\\dA-Fa-f]{2}:){7}[\\dA-Fa-f]{2}%\\d{1,10}");
    
    if( ! strcmp( hint, LIBSMI_DH_TP_IPV6_ADDR ) )
	return xstrdup("\\[([\\dA-Fa-f]{2}:){7}[\\dA-Fa-f]{2}\\]:\\d{1,5}");
    
    if( ! strcmp( hint, LIBSMI_DH_TP_IPV6Z_ADDR ) )
	return xstrdup("\\[([\\dA-Fa-f]{2}:){7}[\\dA-Fa-f]{2}%\\d{1,10}\\]:\\d{1,5}");
    
    if( ! strcmp( hint, LIBSMI_DH_HEX ) )
	return xstrdup("[0-9A-Fa-f]{2}");
    
    if( ! strcmp( hint, LIBSMI_DH_HEX_COLON ) )
	return xstrdup("[0-9A-Fa-f]{2}:");
    
    if( ! strcmp( hint, LIBSMI_DH_DOUBLE_HEX_COLON ) )
	return xstrdup("[0-9A-Fa-f]{4}:");
    
    if( ! strcmp( hint, LIBSMI_DH_ASCII ) ) {
	switch( numSubranges ) { 		
	case 0: 
	    return getSimpleCharFacet_XSD( 'a', 255, 0, 255 );
	case 1: 			
	    return getSimpleCharFacet_XSD( 'a', 255, lengths[0], lengths[1] );
	default:
	    ret = "("; 
	    for( i=0; i < numSubranges * 2; i+=2 ) {
		if( i ) smiAsprintf(&ret, "%s)|(", ret);
		smiAsprintf(&ret, "%s%s", ret,
			    getSimpleCharFacet_XSD('a', 255, 
						   lengths[i], lengths[i+1]));
		
	    }
	    smiAsprintf(&ret, "%s)", ret);
	    return ret;
	}
    }
    
    if( ! strcmp(hint, LIBSMI_DH_UTF8 )) {
	switch( numSubranges ) { 		
	case 0: 
	    return getSimpleCharFacet_XSD( 't' , 255, 0, 255 );
	case 1: 			
	    return getSimpleCharFacet_XSD( 't' , 255, lengths[0], lengths[1]);
	default:
	    ret = "("; 
	    for( i=0; i < numSubranges * 2; i+=2 ) {
		if( i ) smiAsprintf( &ret, "%s)|(", ret );
		smiAsprintf(&ret, "%s%s", 
			    ret, getSimpleCharFacet_XSD('t', 255,
							lengths[i], 
							lengths[i+1]));
	    }
	    smiAsprintf( &ret, "%s)", ret );
	    return ret;
	}
    }
    
    if( ! strcmp( hint, LIBSMI_DH_ASCII_CHAR ))
	return xstrdup("\\p{IsBasicLatin}{0,1}");
    
    /* we assume here date-time-value, i.e. the number of digits are fixed */
    if( ! strcmp( hint, LIBSMI_DH_DATETIME ) )
	return xstrdup("\\d{4}-\\d{2}-\\d{2},(\\d{2}.){2}\\d{2},\\p{IsBasicLatin}\\d{2}:\\d{2}");
    
    if( ! strcmp( hint, LIBSMI_DH_DATETIME2 ) ) {
	if( numSubranges >1 )
	    /* there are more subranges, 
	     * assume: lengths = 8 | lengths = 11 
	     * thus: last 3 octets (time zone info) is optional */ 
	    return xstrdup("\\d{4}-\\d{2}-\\d{2},(\\d{2}:){2}\\d{2}(,[\\+\\-]\\d{2}:\\d{2}){0,1}");
	return xstrdup("\\d{4}-\\d{2}-\\d{2},(\\d{2}:){2}\\d{2},[\\+\\-]\\d{2}:\\d{2}");
    }
    if( !strcmp( hint, LIBSMI_DH_IPX_ADDR ) )
	return xstrdup("[0-9A-Fa-f]{8}.([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}.\\d{1,4}");
    
    else return NULL;		
}



/* build a regexp pattern from a display hint 
 * 
 * Some documentation: First of all, it is checked whether the display hint 
 * matches againts one of the well known display hints. If so, a direct mapping 
 * is applied. Note that most cases ignore any additional length information 
 * (except DATETIME and ASCII / UTF8).
 * If no match can be found, the display hint is parsed and seperated 
 * into a list of blocks (DH struct). A display hint block consists of 
 * o the number of octets to be used (unsigned int "number"),
 * o a charakter representing the type (char "type"),
 * o the separator (char[3] "separator"),
 * o a pointer to the next display hint block in the list.
 * 
 * Then, it is checked whether it is a "simple" display hint, 
 * i.e. is hint that consists only of a single block and 
 * ASCII / UTF-8 restriction (type 'a' or 't'; as in 100a, 127t).
 * (Maybe this can be extended for other simple hints.)
 * 
 * If all of the above matches fail, the hints blocks are converted 
 * step by step using the these Mappings:
 * o ASCII: \p{IsBasicLatin}
 * o UTF-8: .
 * o Binary: (0|1){8} (8 digits (0 or 1))
 * o Decimal: [0-9]{3} (3 decimal digits)
 * o Octal: [0-7]{3} (3 octal digits)
 * o Hexadecimal: [0-9A-Fa-f]{2} (2 hexadecimal digits)
 * The conversion is not straight forward, 
 * because there may be also (even several) length restrictions 
 * and we have to count (or try to estimate) the used octets. Thus, 
 * it is sometimes necessary to generate multiple patterns 
 * with different length specification.
 * 
 * Unfortunately, the conversion still is quite difficult to understand 
 * although I added some comments. The good news is, that all IETF MIBs 
 * (and all other MIBs coming with libsmi) can be converted "the easy way" 
 * (i.e. without the difficult conversion loop).*/

char* getStrDHType(const char *hint,
		   SmiUnsigned32 *lengths, unsigned int numSubranges)
{
    unsigned int i = 0;
    char *ret = lengths[ i ] ? "(" : "((";
    char *r2 = getKnownDisplayHint_XSD( hint, lengths, numSubranges );
    DH *dh;
    int bl;
    
    r2 = NULL;
  
    if( r2 ) {
    /*	if( lengths[ i ] && lengths[i+1] < 65535 ) {    		
    		fputs( "LENGTH: ", stdout );
    		fprintf( stdout, "min: %d, max: %d ; ", lengths[i], lengths[i+1] );
    		fprintf( stdout, "hint was: %s\n", hint );
    	} 
    	if( numSubranges >  1 ) fputs( "MORE_SUBRANGES\n", stdout );*/
    	return r2;	
    }
    dh = parseDH( hint );
    bl = 0;
    
    if(! dh ) return NULL;
    
    /* check if we have a "simple" display hint (e.g. "100a") */
    if( !dh->next && ( dh->type == 'a' || dh->type == 't' ) ) {
    	switch( numSubranges ) { 		
	case 0: 
	    return getSimpleCharFacet_XSD( dh->type , dh->number, 0, dh->number );
	case 1: 			
	    return getSimpleCharFacet_XSD( dh->type , dh->number, lengths[0], lengths[1]);
	default:
	    ret = "("; 
	    for( i=0; i < numSubranges * 2; i+=2 ) {
		if( i ) smiAsprintf( &ret, "%s)|(", ret );
		smiAsprintf( &ret, "%s%s", 
			     ret, getSimpleCharFacet_XSD(dh->type, dh->number,
							 lengths[i], 
							 lengths[i+1]));
	    }
	    smiAsprintf(&ret, "%s)", ret);
	    return ret;
	}   		
    }

    /* no "easy match was possible, so start the "hard" loop */
    do {
	unsigned int octetsUsed = 0;
	DH *iterDH;
	
	for( iterDH = dh; iterDH; iterDH = iterDH->next ) {
	    char *baseRegexp = NULL;
	    
	    switch( iterDH->type ) {
		
	    case 'a':
		/* ascii character */
		baseRegexp = "(\\p{IsBasicLatin})";
		break;
		
	    case 't':
		/* utf-8 character */
		baseRegexp = ".";
		break;
		
	    case 'b':
		/* binary number */
		baseRegexp = "((0|1){8})";
		break;
		
	    case 'd':
		/* decimal number */
		baseRegexp = "([0-9]{3})";
		break;
		
	    case 'o':
		/* octal number */
		baseRegexp = "([0-7]{3})";
		break;
		
	    case 'x':
		/* hexadecimal number */
		baseRegexp = "([0-9A-Fa-f]{2})";
		break;
		
	    default:
		fputs( "smidump: Warning: unknown type of display-hint",
		       stderr );
	    }	   
	    
	    
	    if( iterDH->number < lengths[ i ] ) {
		/* there are more octets to come */
		if( iterDH->type == 'd' ) {
		    /* decimal number needs to be treated differently */
		    if( iterDH->next ){
			/* we still have another diplay hint block coming */
			smiAsprintf( &ret, "%s(0|[1-9](([0-9]){0,%d}))",
				     ret,
				     numDigits(smiPow(255,
						      iterDH->number ))-1 );
			
			/* adjust number of used digits */
			octetsUsed += iterDH->number;
			
			if( octetsUsed >= lengths[ i + 1 ] ) {
			    /* maximum number of octets used,
			       we must exit the loop */			    
			    break;
			}
			
			/* add separator char */
			if( iterDH->separator ) {
			    smiAsprintf( &ret, "%s%s",ret, iterDH->separator );
			}
		    }
		    else {
			/* no orther display hint coming up. 
			 * we are at the last iteration */		
			smiAsprintf( &ret, "%s((0|[1-9](([0-9]){0,%d})",
				     ret,
				     numDigits( smiPow( 255,
							iterDH->number ) ) - 1 );			
			/* add separator char */
			if( iterDH->separator ) {
			    smiAsprintf( &ret, "%s%s",
					 ret, iterDH->separator );
			}
			if( lengths[ i+1 ] - 1 - octetsUsed ) {
			    /* not all digits for maximum string length (length[i+1 ])
			     * have been used, so we have to add some digits */
			    smiAsprintf(&ret,
					"%s){%u,%u})(0|[1-9](([0-9]){0,%d}))",
					ret, lengths[ i ] - 1 - octetsUsed,
					lengths[ i+1 ] - 1 - octetsUsed,
					numDigits(
					    smiPow( 255, iterDH->number ))- 1 );							
			}
			else {
			    /* maximum number of digets have been used,
			     * so let's terminate the pattern for this round*/
			    smiAsprintf( &ret, "%s)", ret );
			}
			
			/* adjust the used digit counter */
			octetsUsed += iterDH->number;
			
			if( octetsUsed >= lengths[ i + 1 ] ) {
			    /* maximum number of octets used, we must exit the loop */
			    break;
			}
		    }
		}
		else {
		    /* type other than decimal */
		    if( iterDH->next ){
			/* there will be another display hint block */
			smiAsprintf( &ret, "%s(%s{%d})",
				     ret,
				     baseRegexp, iterDH->number );
			
			/* adjust number of used octets */
			octetsUsed += iterDH->number;
			if( octetsUsed >= lengths[ i + 1 ] ) {
			    /* maximum number of octets used,
			       we must exit the loop */			    
			    break;
			}
			
			/* add separator char */
			if( iterDH->separator ) {
			    smiAsprintf( &ret, "%s%s", ret, iterDH->separator );
			}
		    }
		    else {
			/* we are the last display hint block */			
			smiAsprintf( &ret, "%s(%s",
				     ret, baseRegexp );
			
			/* add separator char */
			if( iterDH->separator ) {
			    smiAsprintf( &ret, "%s%s", ret, iterDH->separator );
			}
			
			smiAsprintf( &ret, "(%s){%u,%u})%s",
				     ret, lengths[ i ] - 1, lengths[ i+1 ] - 1,
				     baseRegexp );
			
			/* adjust the number of used octets */
			octetsUsed += iterDH->number;
			if( octetsUsed >= lengths[ i + 1 ] ) {
			    /* maximum number of octets used,
			       we must exit the loop */			    
			    break;
			}
		    }
		}
	    }
	    else {
		/* might be the last one */
		
		if( iterDH->type == 'd' ) {
		    /* decimal number needs to be treated differently */
		    if( iterDH->number < lengths[ i+1 ] ) {
			/* we are not using the maximun number of octets */
			smiAsprintf( &ret, "%s(0|[1-9]([0-9]{0,%d}))",
				     ret,
				     numDigits( smiPow( 255, iterDH->number ) ) );
			
			/* adjust the number of used octets */
			octetsUsed += lengths[ i ];
			if( octetsUsed >= lengths[ i + 1 ] ) {
			    /* the maximum number of octets have been reached,
			       we must exit the loop */
			    break;
			}
			
			/* add separator char */
			if( iterDH->separator ) {
			    smiAsprintf( &ret, "%s%s", ret, iterDH->separator );
			}						
		    }
		    else {
			/* we have used the maximum number of octets */
			smiAsprintf( &ret, "%s(0|[1-9]([0-9]{0,%d})",
				     ret,
				     numDigits( smiPow( 255, lengths[ i+1 ] ) ) );
		    }
		}
		else {
		    /* type is not decimal */
		    smiAsprintf( &ret, "%s(%s",  ret, baseRegexp );
		    if( iterDH->next ) {
			/* there will be another display hint block */
			if( iterDH->separator ) {
			    smiAsprintf( &ret, "%s%s", ret, iterDH->separator );
			}
			if( ! lengths[ i ] && lengths[ i+1 ] == 65535 ) {
			    /* minLength = 0, maxLength = 65535, 
			     * i.e. no restriction at all */
			    smiAsprintf( &ret, "%s)*",ret );
			}
			else{
			    /* we have a different length restriction */
			    smiAsprintf( &ret, "%s){%u,%u}",ret, lengths[ i ],
					 MIN( iterDH->number,
					      lengths[ i + 1] ) - 1 );
			}
			
			/* adjust the number of used octets */
			octetsUsed += lengths[ i ];
			if( octetsUsed >= lengths[ i + 1 ] ) {
			    /* the maximum number of octets have been reached,
			       we must exit the loop */
			    break;
			}						
		    }
		    else {
			/* we are the ast display hint block */
			octetsUsed += lengths[ i ];
			if( iterDH->separator &&
			    octetsUsed < lengths[ i + 1 ] ) {
			    /* we have a separator char and 
			     * still not reached the maximum number of octets */
			    if( ! lengths[ i ] && lengths[ i+1 ] == 65535 ) {
				/* we have no length restriction */
				smiAsprintf( &ret, "%s%s)*%s",
					     ret, iterDH->separator, baseRegexp );
			    }
			    else {
				/* we have a length restriction */
				smiAsprintf( &ret, "%s%s){%u,%u}%s",
					     ret, iterDH->separator,
					     lengths[ i ], lengths[ i + 1] - 1,
					     baseRegexp );
			    }
			}
			else {
			    /* we don't have a separator char or
			     * have used the maximum number of octets */
			    if( ! lengths[ i ] && lengths[ i+1 ] == 65535 ) {
				/* no lengths restriction */
				smiAsprintf( &ret, "%s)*%s",
					     ret, iterDH->separator ); 
				/* TBD: what, if there is no separator ??? */
			    }
			    else {
				/* we have a length restriction */
				smiAsprintf( &ret, "%s){%u,%u}%s",
					     ret, lengths[ i ],
					     lengths[ i + 1],
					     iterDH->separator );
				/* TBD: what, if there is no separator ??? */
			    }			    
			}
		    }
		}
		
		if( octetsUsed >= lengths[ i + 1 ] ) {
		    /* the maximum number of octets have been reached,
		       we must exit the loop */
		    break;
		}
	    }
	}
	/* adjust the "pointer" for the lenghts array */
	i += 2;
	
	if( i < numSubranges  * 2 ) {
	    /* we are not the last subrange, so we have to extend the pattern */
	    smiAsprintf( &ret, "%s)|(", ret );
	}
	else {
	    /* we are the last subrange */
	    smiAsprintf( &ret, "%s)", ret );
	    if( ! lengths[ 0 ] ) {
		smiAsprintf( &ret, "%s){0,1}", ret );
	    }
	}
    } while( i < numSubranges * 2 );
    
    /* check if all brackets have been closed */
    if( getBracketLevel( ret ) ) {
	bl = getBracketLevel( ret );
      	fprintf( stderr, "%d\n", bl );
      	if( bl > 0 ) {
	    /* TODO: add a warning that brackets have been added */
	    for(; bl; bl--) {
		smiAsprintf( &ret, "%s)", ret );
	    }
      	}
      	else {
	    /* TODO: some error handling */
      	}
    }
    return ret;
}

/*
 * Perhaps we should introduce another parameter (flags) which
 * controls whether shortcuts are taken and such things...
 */

extern char*
smiFormatToPattern(const char *format, SmiRange *smiRange)
{
    SmiRange *range = smiRange;
    SmiUnsigned32 *lengths = NULL;
    int num = 0, lp;
    char *pattern;

    for (range = smiRange; range; range = smiGetNextRange(range)) {
	num++;
    }

    /* copy ranges to array (no clue why this is being done) */
    
    if (num) {
	lengths = xmalloc(2 * num * sizeof(SmiUnsigned32));
	for (range = smiRange, lp = 0; range; range = smiGetNextRange(range)) {
	    lengths[lp++] = range->minValue.value.unsigned32;
	    lengths[lp++] = range->maxValue.value.unsigned32;
	}
    } else {
	lengths = xmalloc( 2 * sizeof(SmiUnsigned32));
	lengths[0] = 0;
	lengths[1] = 65535;
    }

    pattern = getStrDHType(format, lengths, num);
    xfree(lengths);

    return pattern;
}
