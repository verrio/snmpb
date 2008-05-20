/*
 * dump-sizes.c --
 *
 *      Operations to compute and dump SNMPv3 PDU sizes.
 *
 * Copyright (c) 2003 J. Schoenwaelder, International University Bremen.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-sizes.c 8090 2008-04-18 12:56:29Z strauss $
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "smidump.h"

static int silent = 0;
static int detail = 0;

/*
 * help functions
 */
#define m_abs(a)	((a) < 0 ? -(a) : (a))

typedef struct WellKnowType {
    char *module;
    char *name;
    int max;
    int mean;
    int min;
} WellKnowType;

static WellKnowType specialTypes[] = {
    { "SNMPv2-TC", "PhysAddress", 65535, 6, 0 },
    { "INET-ADDRESS-MIB", "InetAddress", 255, 4, 0 },
    { "IANATn3270eTC-MIB", "IANATn3270eAddress", 255, 4, 0 },
    { NULL, NULL, 0, 0, 0 }
};



typedef enum len_type {
    len_min,
    len_mean,
    len_max
} len_type;



static SmiInteger32
getAbsMinEnum(SmiType *smiType)
{
     SmiNamedNumber *nn;
     SmiInteger32 min = SMI_BASETYPE_INTEGER32_MAX;

     for (nn = smiGetFirstNamedNumber(smiType);
	  nn;
	  nn = smiGetNextNamedNumber(nn)) {
	  if (abs(nn->value.value.integer32) < min) {
	       min = abs(nn->value.value.integer32);
	  }
     }
     return min;
}



static SmiInteger32
getAbsMaxEnum(SmiType *smiType)
{
     SmiNamedNumber *nn;
     SmiInteger32 max = 0;

     for (nn = smiGetFirstNamedNumber(smiType);
	  nn;
	  nn = smiGetNextNamedNumber(nn)) {
	  if (abs(nn->value.value.integer32) > max) {
	       max = abs(nn->value.value.integer32);
	  }
     }
     return max;
}



static SmiInteger32
getAbsMinInteger32(SmiType *smiType)
{
     SmiType *parent;
     SmiRange *range;
     SmiInteger32 min = SMI_BASETYPE_INTEGER32_MAX;

     range = smiGetFirstRange(smiType);
     if (! range) {
	  parent = smiGetParentType(smiType);
	  return parent ? getAbsMinInteger32(parent) : 0;
     }

     for (; range; range = smiGetNextRange(range)) {
	  if (abs(range->minValue.value.integer32) < min) {
	       min = abs(range->minValue.value.integer32);
	  }
     }
     return min;
}



static SmiInteger32
getAbsMaxInteger32(SmiType *smiType)
{
     SmiType *parent;
     SmiRange *range;
     SmiInteger32 max = SMI_BASETYPE_INTEGER32_MIN;

     range = smiGetFirstRange(smiType);
     if (! range) {
	  parent = smiGetParentType(smiType);
	  return parent
	      ? getAbsMaxInteger32(parent) : SMI_BASETYPE_INTEGER32_MAX;
     }

     for (; range; range = smiGetNextRange(range)) {
	  if (abs(range->maxValue.value.integer32) > max) {
	       max = abs(range->maxValue.value.integer32);
	  }
     }
     return max;
}



static SmiUnsigned32
getMinUnsigned32(SmiType *smiType)
{
     SmiType *parent;
     SmiRange *range;
     SmiInteger32 min = 0xffffffff;

     range = smiGetFirstRange(smiType);
     if (! range) {
	  parent = smiGetParentType(smiType);
	  return parent ? getMinUnsigned32(parent) : 0;
     }

     for (; range; range = smiGetNextRange(range)) {
	  if (range->minValue.value.unsigned32 < min) {
	       min = range->minValue.value.unsigned32;
	  }
     }
     return min;
}



static SmiUnsigned32
getMaxUnsigned32(SmiType *smiType)
{
     SmiType *parent;
     SmiRange *range;
     SmiUnsigned32 max = 0;

     range = smiGetFirstRange(smiType);
     if (! range) {
	  parent = smiGetParentType(smiType);
	  return parent ? getMaxUnsigned32(parent) : 0xffffffff;
     }

     for (; range; range = smiGetNextRange(range)) {
	  if (range->maxValue.value.unsigned32 > max) {
	       max = range->maxValue.value.unsigned32;
	  }
     }
     return max;
}



static SmiInteger64
getAbsMinInteger64(SmiType *smiType)
{
     SmiType *parent;
     SmiRange *range;
     SmiInteger64 min = SMI_BASETYPE_INTEGER64_MAX;

     range = smiGetFirstRange(smiType);
     if (! range) {
	  parent = smiGetParentType(smiType);
	  return parent ? getAbsMinInteger64(parent) : min;
     }

     for (; range; range = smiGetNextRange(range)) {
	  if (m_abs(range->minValue.value.integer64) < min) {
	       min = m_abs(range->minValue.value.integer64);
	  }
     }
     return min;
}



static SmiInteger64
getAbsMaxInteger64(SmiType *smiType)
{
     SmiType *parent;
     SmiRange *range;
     SmiInteger64 max = SMI_BASETYPE_INTEGER64_MIN;

     range = smiGetFirstRange(smiType);
     if (! range) {
	  parent = smiGetParentType(smiType);
	  return parent ? getAbsMaxInteger64(parent) : max;
     }

     for (; range; range = smiGetNextRange(range)) {
	  if (m_abs(range->maxValue.value.integer64) > max) {
	       max = m_abs(range->maxValue.value.integer64);
	  }
     }
     return max;
}



static SmiUnsigned64
getMinUnsigned64(SmiType *smiType)
{
     SmiType *parent;
     SmiRange *range;
     SmiInteger64 min = SMI_BASETYPE_UNSIGNED64_MAX;

     range = smiGetFirstRange(smiType);
     if (! range) {
	  parent = smiGetParentType(smiType);
	  return parent ? getMinUnsigned64(parent) : min;
     }

     for (; range; range = smiGetNextRange(range)) {
	  if (range->minValue.value.unsigned64 < min) {
	       min = range->minValue.value.unsigned64;
	  }
     }
     return min;
}



static SmiUnsigned64
getMaxUnsigned64(SmiType *smiType)
{
     SmiType *parent;
     SmiRange *range;
     SmiUnsigned64 max = SMI_BASETYPE_UNSIGNED64_MIN;

     range = smiGetFirstRange(smiType);
     if (! range) {
	  parent = smiGetParentType(smiType);
	  return parent ? getMaxUnsigned64(parent) : max;
     }

     for (; range; range = smiGetNextRange(range)) {
	  if (range->maxValue.value.unsigned64 > max) {
	       max = range->maxValue.value.unsigned64;
	  }
     }
     return max;
}



static int
ber_len_length(int length)
{
     int len;
     
     if (length < 0x80) {
	  return 1;
     }

     for (len = 0; length > 0; len++) {
	  length >>= 8;
     }
     return len;
}



static int
ber_len_subid(SmiSubid subid)
{
     int len = 0;

     do {
	  subid >>= 7;
	  len++;
     } while (subid > 0);
     return len;
}



static int
ber_len_oid(const SmiSubid *oid, unsigned int oidlen)
{
     int len = 0;
     int i;
     
     len += ber_len_subid(oid[1] + oid[0] * 40);
     for (i = 2; i < oidlen; i++) {
	  len += ber_len_subid(oid[i]);
     }

     len += ber_len_length(len);	/* length */
     len += 1;				/* tag */
     return len;
}



static int
ber_len_int32(const SmiInteger32 value)
{
     SmiInteger32 val = value;
     unsigned char ch, sign;
     int lim;
     int len = 0;
     
     if (val < 0) {
	  lim  = -1;
	  sign = 0x80;
     } else {
	  lim  = 0;
	  sign = 0x00;
     }
     do {
	  ch = (unsigned char) val;
	  val >>= 8;
	  len++;
     } while ((val != lim) || (unsigned char) (ch & 0x80) != sign);

     len += ber_len_length(len);	/* length */
     len += 1;				/* tag */
     return len;
}



static int
ber_len_uint32(const SmiUnsigned32 value)
{
     SmiUnsigned32 val = value;
     unsigned char ch;
     int len = 0;
     
     do {
	  ch = (unsigned char) val;
	  val >>= 8;
	  len++;
     } while ((val != 0) || (ch & 0x80) != 0x00);

     len += ber_len_length(len);	/* length */
     len += 1;				/* tag */
     return len;
}



static int
ber_len_int64(const SmiInteger64 value)
{
     SmiInteger64 val = value;
     unsigned char ch, sign;
     int lim;
     int len = 0;
     
     if (val < 0) {
	  lim  = -1;
	  sign = 0x80;
     } else {
	  lim  = 0;
	  sign = 0x00;
     }
     do {
	  ch = (unsigned char) val;
	  val >>= 8;
	  len++;
     } while ((val != lim) || (unsigned char) (ch & 0x80) != sign);

     len += ber_len_length(len);	/* length */
     len += 1;				/* tag */
     return len;
}



static int
ber_len_uint64(const SmiUnsigned64 value)
{
     SmiUnsigned64 val = value;
     unsigned char ch;
     int len = 0;
     
     do {
	  ch = (unsigned char) val;
	  val >>= 8;
	  len++;
     } while ((val != 0) || (ch & 0x80) != 0x00);

     len += ber_len_length(len);	/* length */
     len += 1;				/* tag */
     return len;
}



static int
ber_len_val_oid(SmiType *smiType, len_type flags)
{
    SmiSubid      oid[128];
    unsigned int  oidlen = sizeof(oid)/sizeof(oid[0]);
    int i;
    
    switch (flags) {
    case len_max: 
	oid[0] = 2;
	for (i = 1; i < 128; i++) {
	    oid[i] = 4294967295UL;
	}
	break;
    case len_mean:
	/* see Aiko's measurements */
	for (oidlen = 0; oidlen < 15; oidlen++) {
	    oid[oidlen] = 1;
	}
	break;
    case len_min:
	oid[0] = oid[1] = 0, oidlen = 2;
	break;
    }
    return ber_len_oid(oid, oidlen);
}



static int
ber_len_val_octs(SmiType *smiType, len_type flags)
{
    int len = 0;

    SmiModule *smiModule;

    smiModule = smiGetTypeModule(smiType);
    if (smiModule && smiModule->name && smiType->name) {
	int i;
	for (i = 0; specialTypes[i].module; i++) {
	    if (strcmp(specialTypes[i].module, smiModule->name) == 0
		&& (strcmp(specialTypes[i].name, smiType->name) == 0)) {
		break;
	    }
	}
	if (specialTypes[i].module) {
	    switch (flags) {
	    case len_max:
		return specialTypes[i].max;
		break;
	    case len_mean:
		return specialTypes[i].mean;
		break;
	    case len_min:
		return specialTypes[i].min;
		break;
	    }
	}
    }

    switch (flags) {
    case len_max:
	len = smiGetMaxSize(smiType);
	break;
    case len_mean:
	len = (smiGetMaxSize(smiType) + smiGetMinSize(smiType)) / 2;
	break;
    case len_min:
	len = smiGetMinSize(smiType);
	break;
    }
    return len;
}



static int
ber_len_val_bits(SmiType *smiType, len_type flags)
{
    int len = 0;

    switch (flags) {
    case len_max:
	len = smiGetMaxSize(smiType);
	break;
    case len_mean:
	len = (smiGetMaxSize(smiType) + smiGetMinSize(smiType)) / 2;
	break;
    case len_min:
	len = smiGetMinSize(smiType);
	break;
    }
    return len;
}



static int
ber_len_val_enum(SmiType *smiType, len_type flags)
{
     SmiInteger32 val = 0;

     switch (flags) {
     case len_max:
	 val = getAbsMaxEnum(smiType);
	 break;
     case len_mean:
	 val = (getAbsMaxEnum(smiType) + getAbsMinEnum(smiType)) / 2;
	 break;
     case len_min:
	 val = getAbsMinEnum(smiType);
	 break;
     }
     return ber_len_int32(val);
}



static int
ber_len_val_int32(SmiType *smiType, len_type flags)
{
    SmiInteger32 val = 0;
    
    switch (flags) {
    case len_max:
	val = getAbsMaxInteger32(smiType);
	break;
    case len_mean:
	val = (getAbsMaxInteger32(smiType) + getAbsMinInteger32(smiType)) / 2;
	break;
    case len_min:
	val = getAbsMinInteger32(smiType);
	break;	     
    }
    return ber_len_int32(val);
}



static int
ber_len_val_uint32(SmiType *smiType, len_type flags)
{
    SmiUnsigned32 val = 0;
    
    switch (flags) {
    case len_max:
	val = getMaxUnsigned32(smiType);
	break;
    case len_mean:
	val = (getMaxUnsigned32(smiType) + getMinUnsigned32(smiType)) / 2;
	break;
    case len_min:
	val = getMinUnsigned32(smiType);
	break;
    }
    return ber_len_uint32(val);
}



static int
ber_len_val_int64(SmiType *smiType, len_type flags)
{
    SmiInteger64 val = 0;

    switch (flags) {
    case len_max:
	val = getAbsMaxInteger64(smiType);
	break;
    case len_mean:
	val = (getAbsMaxInteger64(smiType) + getAbsMinInteger64(smiType)) / 2;
	break;
    case len_min:
	val = getAbsMinInteger64(smiType);
	break;
    }
    return ber_len_int64(val);
}



static int
ber_len_val_uint64(SmiType *smiType, len_type flags)
{
    SmiUnsigned64 val = 0;

    switch (flags) {
    case len_max:
	val = getMaxUnsigned64(smiType);
	break;
    case len_mean:
	val = (getMaxUnsigned64(smiType) + getMinUnsigned64(smiType)) / 2;
	break;
    case len_min:
	val = getMinUnsigned64(smiType);
	break;
    }
    return ber_len_uint64(val);
}



static int
ber_len_val(SmiType *smiType, len_type flags)
{
     int len = 0;
     
     switch (smiType->basetype) {
     case SMI_BASETYPE_OBJECTIDENTIFIER:
	 len = ber_len_val_oid(smiType, flags);
	 break;
     case SMI_BASETYPE_OCTETSTRING:
	 len = ber_len_val_octs(smiType, flags);
	 break;
     case SMI_BASETYPE_BITS:
	 len = ber_len_val_bits(smiType, flags);
	 break;
     case SMI_BASETYPE_ENUM:
	 len = ber_len_val_enum(smiType, flags);
	 break;
     case SMI_BASETYPE_INTEGER32:
	 len = ber_len_val_int32(smiType, flags);
	 break;
     case SMI_BASETYPE_UNSIGNED32:
	 len = ber_len_val_uint32(smiType, flags);
	 break;
     case SMI_BASETYPE_INTEGER64:
	 len = ber_len_val_int64(smiType, flags);
	 break;
     case SMI_BASETYPE_UNSIGNED64:
	 len = ber_len_val_uint64(smiType, flags);
	 break;
     default:
	 break;
     }

     return len;
}



static void
append_index(SmiSubid *oid, unsigned int *oidlen,
	     SmiNode *indexNode, len_type flags)
{
     SmiInteger32  int32 = 0;
     SmiUnsigned32 uint32 = 0;
     SmiType *indexType;
     SmiModule *indexModule;
     int i, len = 0;

     if (! indexNode) return;

     indexType = smiGetNodeType(indexNode);
     if (! indexType) return;

     indexModule = smiGetTypeModule(indexType);

     switch (indexType->basetype) {
     case SMI_BASETYPE_OBJECTIDENTIFIER:

	 switch (flags) {
	 case len_max:
	     len = 128 - *oidlen;
	     if (indexNode->implied) len--;
	     break;
	 case len_mean:
	     len = 16;
	     break;
	 case len_min:
	     len = 2;
	     break;
	 }
	 
	 if (! indexNode->implied && *oidlen < 128) {
	     oid[(*oidlen)++] = len;
	 }
	 for (i = 0; i < len && *oidlen < 128; i++) {
	     switch (flags) {
	     case len_max:
		 if (i == 0) {
		     oid[(*oidlen)++] = 2;
		 } else {
		     oid[(*oidlen)++] = 4294967295UL;
		 }
		 break;
	     case len_mean:
		 oid[(*oidlen)++] = i + 1;
		 break;
	     case len_min:
		 oid[(*oidlen)++] = 0;
		 break;
	     }
	 }
	 break;
     case SMI_BASETYPE_OCTETSTRING:
     case SMI_BASETYPE_BITS:
	 switch (flags) {
	 case len_max:
	     len = smiGetMaxSize(indexType);
	     break;
	 case len_mean:
	     len = (smiGetMaxSize(indexType) + smiGetMinSize(indexType) / 2);
	     break;
	 case len_min:
	     len = smiGetMinSize(indexType);
	     break;
	 }

	 if (indexModule && indexModule->name && indexType->name) {
	     int i;
	     for (i = 0; specialTypes[i].module; i++) {
		 if (strcmp(specialTypes[i].module, indexModule->name) == 0
		     && (strcmp(specialTypes[i].name, indexType->name) == 0)) {
		     break;
		 }
	     }
	     if (specialTypes[i].module) {
		 switch (flags) {
		 case len_max:
		     len = specialTypes[i].max;
		     break;
		 case len_mean:
		     len = specialTypes[i].mean;
		     break;
		 case len_min:
		     len = specialTypes[i].min;
		     break;
		 }
	     }
	 }
	 
	 if (! indexNode->implied && *oidlen < 128) {
	     oid[(*oidlen)++] = len;
	 }
	 for (i = 0; i < len && *oidlen < 128; i++) {
	     switch (flags) {
	     case len_max:
		 oid[(*oidlen)++] = 255;
		 break;
	     case len_mean:
		 if (i == 0) {
		     oid[(*oidlen)++] = 1;
		 } else {
		     oid[(*oidlen)++] = (i%2) ? 85 : 170;
		 }
		 break;
	     case len_min:
		 oid[(*oidlen)++] = 0;
		 break;
	     }
	  }
	 break;
     case SMI_BASETYPE_ENUM:
	 switch (flags) {
	 case len_max:
	     int32 = getAbsMaxEnum(indexType);
	     break;
	 case len_mean:
	     int32 = (getAbsMaxEnum(indexType) - getAbsMinEnum(indexType)) / 2;
	     break;
	 case len_min:
	     int32 = getAbsMinEnum(indexType);
	     break;
	 }
	 if (*oidlen < 128) {
	     oid[(*oidlen)++] = int32;
	 }
	 break;
     case SMI_BASETYPE_INTEGER32:
	 switch (flags) {
	 case len_max:
	     int32 = getAbsMaxInteger32(indexType);
	     break;
	 case len_mean:
	     int32 = (getAbsMaxInteger32(indexType)
		      + getAbsMinInteger32(indexType)) / 2;
	     break;
	 case len_min:
	     int32 = getAbsMinInteger32(indexType);
	     break;
	 }
	 if (*oidlen < 128) {
	     oid[(*oidlen)++] = int32;
	 }
	 break;
     case SMI_BASETYPE_UNSIGNED32:
	 switch (flags) {
	 case len_max:
	     uint32 = getMaxUnsigned32(indexType);
	     break;
	 case len_mean:
	     uint32 = (getMaxUnsigned32(indexType)
		       + getMinUnsigned32(indexType)) / 2;
	     break;
	 case len_min:
	     uint32 = getMinUnsigned32(indexType);
	     break;
	 }
	 if (*oidlen < 128) {
	     oid[(*oidlen)++] = uint32;
	 }
	 break;
     case SMI_BASETYPE_UNKNOWN:
     case SMI_BASETYPE_INTEGER64:
     case SMI_BASETYPE_UNSIGNED64:
     case SMI_BASETYPE_FLOAT32:
     case SMI_BASETYPE_FLOAT64:
     case SMI_BASETYPE_FLOAT128:
     case SMI_BASETYPE_POINTER:
	 /* should never really get here */
	 break;
     }
}

#undef DUMP_OID

static int
ber_len_varbind(SmiNode *smiNode, len_type flags)
{
     SmiNode *row;
     SmiSubid oid[128];
     unsigned int oidlen = sizeof(oid)/sizeof(oid[0]);
     int len = 0;
#ifdef DUMP_OID
     int x;
#endif

     switch (smiNode->nodekind) {
     case SMI_NODEKIND_SCALAR:
	  for (oidlen = 0; oidlen < smiNode->oidlen; oidlen++) {
	       oid[oidlen] = smiNode->oid[oidlen];
	  }
	  oid[oidlen++] = 0;
	  break;
     case SMI_NODEKIND_COLUMN:
	  for (oidlen = 0; oidlen < smiNode->oidlen; oidlen++) {
	       oid[oidlen] = smiNode->oid[oidlen];
	  }
	  row = smiGetParentNode(smiNode);
	  if (row) {
	       SmiNode *indexNode = NULL, *iNode;
	       SmiElement *smiElement;
	       
	       switch (row->indexkind) {
	       case SMI_INDEX_INDEX:
	       case SMI_INDEX_REORDER:
		    indexNode = row;
		    break;
	       case SMI_INDEX_EXPAND: /* TODO: we have to do more work here! */
		    break;
	       case SMI_INDEX_AUGMENT:
	       case SMI_INDEX_SPARSE:
		    indexNode = smiGetRelatedNode(row);
		    break;
	       case SMI_INDEX_UNKNOWN:
		    break;
	       }
	       if (indexNode) {
		    for (smiElement = smiGetFirstElement(indexNode);
			 smiElement;
			 smiElement = smiGetNextElement(smiElement)) {
			 iNode = smiGetElementNode(smiElement);
			 append_index(oid, &oidlen, iNode, flags);
		    }
	       }
	  }
	  break;
     default:
	  return 0;
     }

#ifdef DUMP_OID
     fprintf(stderr, "%-32s\t", smiNode->name);
     for (x = 0; x < oidlen; x++) {
	  fprintf(stderr, ".%u", oid[x]);
     }
     fprintf(stderr, "\n");
#endif

     len += ber_len_oid(oid, oidlen);
     len += ber_len_val(smiGetNodeType(smiNode), flags);
     len += ber_len_length(len) + 1;

     return len;
}



static int
isGroup(SmiNode *smiNode)
{
    SmiNode *childNode;

    if (smiNode->nodekind == SMI_NODEKIND_ROW) {
	return 1;
    }
    
    for (childNode = smiGetFirstChildNode(smiNode);
	 childNode;
	 childNode = smiGetNextChildNode(childNode)) {
	if (childNode->nodekind == SMI_NODEKIND_SCALAR
		&& childNode->access > SMI_ACCESS_NOTIFY) {
	    return 1;
	}
    }

    return 0;
}



static void
dumpSizeOfPDU(FILE *f, SmiModule *smiModule, SmiNode *smiNode)
{
     SmiNode *child;
     int worst = 0;
     int best = 0;
     int avg = 0;
     int b, w, a, n = 0;
     
     for (child = smiGetFirstChildNode(smiNode);
	  child;
	  child = smiGetNextChildNode(child)) {
	  if (child->access == SMI_ACCESS_READ_WRITE
	      || child->access == SMI_ACCESS_READ_ONLY) {

	       b = ber_len_varbind(child, len_min);
	       a = ber_len_varbind(child, len_mean);
	       w = ber_len_varbind(child, len_max);
	       
	       best += b, worst += w, avg += a, n++;
	  }
     }

     /* varbind list sequence length and tag */
     best  += ber_len_length(best)  + 1;
     avg   += ber_len_length(avg)   + 1;
     worst += ber_len_length(worst) + 1;

     /* request-id as defined in RFC 3416 */
     best += ber_len_int32(0);
     avg += ber_len_int32(1073741824);
     worst += ber_len_int32(-214783648);
     
     /* error-status as defined in RFC 3416 */
     best += ber_len_int32(0);
     avg += ber_len_int32(0);
     worst += ber_len_int32(18);
     
     /* error-index as defined in RFC 3416 */
     best += ber_len_int32(0);
     avg += ber_len_int32(0);
     worst += ber_len_int32(n-1);

     /* PDU sequence length and tag */
     best  += ber_len_length(best)  + 1;
     avg += ber_len_length(avg) + 1;
     worst += ber_len_length(worst) + 1;
     
     fprintf(f, "%-23s %-23s \t%d\t[%d..%d]\n", smiModule->name, smiNode->name,
	     avg, best, worst);

     if (detail) {
	 for (child = smiGetFirstChildNode(smiNode);
	      child;
	      child = smiGetNextChildNode(child)) {
	     if (child->access == SMI_ACCESS_READ_WRITE
		 || child->access == SMI_ACCESS_READ_ONLY) {
		 
		 b = ber_len_varbind(child, len_min);
		 a = ber_len_varbind(child, len_mean);
		 w = ber_len_varbind(child, len_max);
		 
		 fprintf(f, "%-23s %-23s \t%d\t[%d..%d]\n",
			 "", child->name, a, b, w);
	     }
	 }
     }
}



static void
dumpSizeOfCreatePDU(FILE *f, SmiModule *smiModule, SmiNode *smiNode,
		    int ignoreDefaultColumns)
{
     SmiNode *child;
     SmiType *childType;
     SmiModule *childTypeModule;
     
     int worst = 0;
     int best = 0;
     int avg = 0;
     int b, w, a, n = 0;
     int isRowStatus;
     
     for (child = smiGetFirstChildNode(smiNode);
	  child;
	  child = smiGetNextChildNode(child)) {
	  if (child->access == SMI_ACCESS_READ_WRITE) {

	       /* Ensure RowStatus columns are present even if they
		* have a default value. */

	       childType = smiGetNodeType(child);
	       childTypeModule = childType
		    ? smiGetTypeModule(childType) : NULL;
	       
	       isRowStatus
		    = (childType && childType->name
		       && childTypeModule && childTypeModule->name)
		    ? (strcmp(childType->name, "RowStatus") == 0
		       && strcmp(childTypeModule->name, "SNMPv2-TC") == 0)
		    : 0;

	       /* xxx at least one PDU must be present xxx */

	       if (ignoreDefaultColumns
		   && child->value.basetype != SMI_BASETYPE_UNKNOWN
		   && !isRowStatus) {
		    continue;
	       }

	       b = ber_len_varbind(child, len_min);
	       a = ber_len_varbind(child, len_mean);
	       w = ber_len_varbind(child, len_max);

#if 0
	       fprintf(f, "  %-32s\t[%d..%d] | %d\n", child->name, b, w, a);
#endif
	       
	       best += b, worst += w, avg += a, n++;
	  }
     }

     /* varbind list sequence length and tag */
     best  += ber_len_length(best)  + 1;
     avg   += ber_len_length(avg)   + 1;
     worst += ber_len_length(worst) + 1;

     /* request-id as defined in RFC 3416 */
     best += ber_len_int32(0);
     avg += ber_len_int32(1073741824);
     worst += ber_len_int32(-214783648);
     
     /* error-status as defined in RFC 3416 */
     best += ber_len_int32(0);
     avg += ber_len_int32(0);
     worst += ber_len_int32(18);
     
     /* error-index as defined in RFC 3416 */
     best += ber_len_int32(0);
     avg += ber_len_int32(0);
     worst += ber_len_int32(n-1);

     /* PDU sequence length and tag */
     best  += ber_len_length(best)  + 1;
     avg += ber_len_length(avg) + 1;
     worst += ber_len_length(worst) + 1;
     
     fprintf(f, "%-23s %-23s \t%d\t[%d..%d]\n", smiModule->name, smiNode->name,
	     avg, best, worst);
}



static void
dumpSizeOfNotificationPDU(FILE *f, SmiModule *smiModule, SmiNode *smiNode)
{
     SmiElement *smiElement;
     SmiNode *varNode;
     int worst = 0;
     int best = 0;
     int avg = 0;
     int w, b, a;
     int len = 0;
     static const SmiSubid snmpTrapOid0[]
	  = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
     static const int snmpTrapOid0Len
	  = sizeof(snmpTrapOid0) / sizeof(SmiSubid);
     
     b = 15, w = 19, a = 18;
     best += b, worst += w, avg += a;
     
     len += ber_len_oid(smiNode->oid, smiNode->oidlen);
     len += ber_len_oid(snmpTrapOid0, snmpTrapOid0Len);
     len += ber_len_length(len) + 1;
     b = len, w = len, a = len;
     best += b, worst += w, avg += a;
     
     for (smiElement = smiGetFirstElement(smiNode);
	  smiElement;
	  smiElement = smiGetNextElement(smiElement)) {
	  varNode = smiGetElementNode(smiElement);
	  if (! varNode) continue;

	  b = ber_len_varbind(varNode, len_min);
	  a = ber_len_varbind(varNode, len_mean);
	  w = ber_len_varbind(varNode, len_max);
	  
	  best += b, worst += w, avg += a;
     }

     /* varbind list sequence length and tag */
     best  += ber_len_length(best)  + 1;
     avg   += ber_len_length(avg)   + 1;
     worst += ber_len_length(worst) + 1;
     
     /* request-id as defined in RFC 3416 */
     best += ber_len_int32(0);
     avg += ber_len_int32(1073741824);
     worst += ber_len_int32(-214783648);
     
     /* error-status as defined in RFC 3416 */
     best += ber_len_int32(0);
     avg += ber_len_int32(0);
     worst += ber_len_int32(18);
     
     /* error-index as defined in RFC 3416 */
     best += ber_len_int32(0);
     avg += ber_len_int32(0);
     worst += ber_len_int32(0);

     /* PDU sequence length and tag */
     best  += ber_len_length(best)  + 1;
     avg += ber_len_length(avg) + 1;
     worst += ber_len_length(worst) + 1;
     
     fprintf(f, "%-23s %-23s \t%d\t[%d..%d]\n", smiModule->name, smiNode->name,
	     avg, best, worst);

     if (detail) {
	 b = 15, w = 19, a = 18;
	 fprintf(f, "%-23s %-23s \t%d\t[%d..%d]\n",
		 "", "sysUpTime", a, b, w);
	 
	 len = 0;
	 len += ber_len_oid(smiNode->oid, smiNode->oidlen);
	 len += ber_len_oid(snmpTrapOid0, snmpTrapOid0Len);
	 len += ber_len_length(len) + 1;
	 b = len, w = len, a = len;
	 fprintf(f, "%-23s %-23s \t%d\t[%d..%d]\n",
		 "", "snmpTrapOID", a, b, w);
	 
	 for (smiElement = smiGetFirstElement(smiNode);
	      smiElement;
	      smiElement = smiGetNextElement(smiElement)) {
	     varNode = smiGetElementNode(smiElement);
	     if (! varNode) continue;
	     
	     b = ber_len_varbind(varNode, len_min);
	     a = ber_len_varbind(varNode, len_mean);
	     w = ber_len_varbind(varNode, len_max);
	     
	     fprintf(f, "%-23s %-23s \t%d\t[%d..%d]\n",
		     "", varNode->name, a, b, w);
	 }
     }
}



static void
dumpGroupPduSizes(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
	if (isGroup(smiNode)) {
	    dumpSizeOfPDU(f, smiModule, smiNode);
	}
    }
}



static void
dumpFullRowCreatePduSizes(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ROW);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ROW)) {
	if (smiNode->create) {
	    dumpSizeOfCreatePDU(f, smiModule, smiNode, 0);
	}
    }
}

     

static void
dumpSmallRowCreatePduSizes(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ROW);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ROW)) {
	if (smiNode->create) {
	    dumpSizeOfCreatePDU(f, smiModule, smiNode, 1);
	}
    }
}



static void
dumpNotificationPduSizes(FILE *f, SmiModule *smiModule)
{
    SmiNode *smiNode;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_NOTIFICATION);
	 smiNode;
	 smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_NOTIFICATION)) {
	dumpSizeOfNotificationPDU(f, smiModule, smiNode);
    }
}



static void
dumpSizes(int modc, SmiModule **modv, int flags, char *output)
{
     int       i;
     FILE      *f = stdout;
     
     silent = (flags & SMIDUMP_FLAG_SILENT);
     
     if (output) {
	  f = fopen(output, "w");
	  if (!f) {
	       fprintf(stderr, "smidump: cannot open %s for writing: ", output);
	       perror(NULL);
	       exit(1);
	  }
     }
     
     if (flags & SMIDUMP_FLAG_UNITE) {
	  if (! silent) {
	       int pos = 8888;
	       fprintf(f, "# united module PDU sizes (generated by smidump "
		       SMI_VERSION_STRING ")\n");
	       fprintf(f, "#\n# smidump -u -f sizes");
	       for (i = 0; i < modc; i++) {
		    int len = strlen(modv[i]->name);
		    if (pos + len > 70) {
			 fprintf(f, " \\\n#\t"), pos = 8;
		    }
		    fprintf(f, "%s ", modv[i]->name);
		    pos += len + 1;
	       }
	       fprintf(f, "%s\n", (pos == 8) ? "" : "\n");
	  }
	  fprintf(f, "\n# size of PDUs for groups and rows:\n\n");
	  for (i = 0; i < modc; i++) {
	      dumpGroupPduSizes(f, modv[i]);
	  }
	  fprintf(f, "\n# size of one-shot row creation PDUs including columns with default values:\n\n");
	  for (i = 0; i < modc; i++) {
	      dumpFullRowCreatePduSizes(f, modv[i]);
	  }
	  fprintf(f, "\n# size of one-shot row creation PDUs excluding columns with default values:\n\n");
	  for (i = 0; i < modc; i++) {
	      dumpSmallRowCreatePduSizes(f, modv[i]);
	  }
	  fprintf(f, "\n# size of notification PDUs:\n\n");
	  for (i = 0; i < modc; i++) {
	      dumpNotificationPduSizes(f, modv[i]);
	  }
     } else {
	  for (i = 0; i < modc; i++) {
	       if (! silent) {
		    fprintf(f, "# %s module PDU sizes (generated by smidump "
			    SMI_VERSION_STRING ")\n\n", modv[i]->name);
	       }
	       fprintf(f, "\n# size of PDUs for groups and rows:\n\n");
	       dumpGroupPduSizes(f, modv[i]);
	       fprintf(f, "\n# size of one-shot row creation PDUs including columns with default values:\n\n");
	       dumpFullRowCreatePduSizes(f, modv[i]);
	       fprintf(f, "\n# size of one-shot row creation PDUs excluding columns with default values:\n\n");
	       dumpSmallRowCreatePduSizes(f, modv[i]);
	       fprintf(f, "\n# size of notification PDUs:\n\n");
	       dumpNotificationPduSizes(f, modv[i]);
	  }
     }
     
     if (fflush(f) || ferror(f)) {
	  perror("smidump: write error");
	  exit(1);
     }
     
     if (output) {
	  fclose(f);
     }
}

void
initSizes()
{
    static SmidumpDriverOption opt[] = {
	{ "variables", OPT_FLAG, &detail, 0,
	  "show detailed information the sizes of variables"},
        { 0, OPT_END, 0, 0 }
    };
    
    static SmidumpDriver driver = {
	"sizes",
	dumpSizes,
	SMI_FLAG_NODESCR,
	0,
	"RFC 3416 PDU sizes excluding message / transport headers",
	opt,
	NULL
    };
    
    smidumpRegisterDriver(&driver);
}
