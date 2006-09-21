/*
 * win.c --
 *
 *      Some helper functions to make this compile with vc++ for win32
 *
 * Copyright (c) 2000 E. Schoenfelder, Gaertner Datensysteme Braunschweig.
 * Copyright (c) 2000 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: win.c 1107 2001-03-08 14:49:09Z schoenw $
 */

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>

#include "win.h"

/*
 * There are currently no Win32 specific compatibility functions since
 * we replaced the getopt option parser with another option parser
 * implementation. But we will keep this file around since I am sure
 * we will need it again at some point in time.
 */
