/*_############################################################################
  _## 
  _##  v3.cpp  
  _##
  _##  SNMP++v3.2.14
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2004 Jochen Katz, Frank Fock
  _##
  _##  This software is based on SNMP++2.6 from Hewlett Packard:
  _##  
  _##    Copyright (c) 1996
  _##    Hewlett-Packard Company
  _##  
  _##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  _##  Permission to use, copy, modify, distribute and/or sell this software 
  _##  and/or its documentation is hereby granted without fee. User agrees 
  _##  to display the above copyright notice and this license notice in all 
  _##  copies of the software and any documentation of the software. User 
  _##  agrees to assume all liability for the use of the software; 
  _##  Hewlett-Packard and Jochen Katz make no representations about the 
  _##  suitability of this software for any purpose. It is provided 
  _##  "AS-IS" without warranty of any kind, either express or implied. User 
  _##  hereby grants a royalty-free license to any and all derivatives based
  _##  upon this software code base. 
  _##  
  _##  Stuttgart, Germany, Tue Sep  7 21:25:32 CEST 2004 
  _##  
  _##########################################################################*/

char v3_cpp_version[]="#(@) SNMP++ $Id$";

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#ifndef _MSC_VER
#ifndef __BCPLUSPLUS__
#include <unistd.h>
#endif
#endif

#include "snmp_pp/v3.h"
#include "snmp_pp/octet.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#define MAX_LINE_LEN 100

const char *logfilename = NULL;
int debug_level = 19;

void debug_set_logfile(const char *filename)
{
  logfilename = filename;
}

// Set the amount of log messages you want to get.
void debug_set_level(const int db_level)
{
  debug_level = db_level;
}

#ifdef _DEBUG

void debughexcprintf(int db_level, const char *comment,
                     const unsigned char *data, const unsigned int len)
{
  if (db_level > debug_level)
    return;

  FILE    *logfile = NULL;

  if (logfilename)
  {
    logfile = fopen(logfilename, "a");
  }
  if (logfile == NULL)
    logfile = stdout;

  if (comment)
    fprintf(logfile, "%s (length %i): \n", comment, len);

  for (unsigned int i=0; i<len; i++) {
    fprintf(logfile, "%02X ", data[i]);
    if ((i+1)%4==0) fprintf(logfile, " ");
    if ((i+1)%16==0) fprintf(logfile, "\n");
  }
  fprintf(logfile, "\n");

  if (logfile != stdout)
    fclose(logfile);
}

void debugprintf(int db_level, const char *format, ...)
{
  if (db_level > debug_level)
    return;

  va_list  args;
  FILE    *logfile = NULL;

  if (logfilename)
  {
    logfile = fopen(logfilename, "a");
  }
  if (logfile == NULL)
    logfile = stdout;

  va_start(args, format);

  vfprintf(logfile, format, args);
  fprintf(logfile, "\n");

  va_end(args);

  if (logfile != stdout)
    fclose(logfile);
}

#else
#if (defined (__STRICT_ANSI__) || !defined (__GNUC__)) && !defined (_MSC_VER)
void debugprintf(int, const char*, ...)
{
}
#endif

#endif

#ifdef _SNMPv3

unsigned char *v3strcpy(const unsigned char *src, const int srclen)
{
  unsigned char *res = new unsigned char[srclen+1];
  if (!res) return NULL;
  memcpy(res, src, srclen);
  res[srclen] = '\0';
  return res;
}


int unsignedCharCompare(const unsigned char *str1, const long int ptr1len,
                        const unsigned char *str2, const long int ptr2len)
{
  if (ptr1len != ptr2len) return 0;

  const unsigned char *ptr1 = str1;
  const unsigned char *ptr2 = str2;

  for (int i=0; i < ptr1len; ++i)
    if (*ptr1++ != *ptr2++) return 0;

  return 1;
}

// Encode the given string into the output buffer.
void encodeString(const unsigned char* in, const int in_length, char* out)
{
  char* out_ptr = out;
  const unsigned char* in_ptr = in;

  for (int i=0; i<in_length; i++)
  {
    *out_ptr++ = 64 + ((*in_ptr >> 4) & 0xF);
    *out_ptr++ = 64 + (*in_ptr++ & 0xF);
  }
}

// Decode the given encoded string into the output buffer.
void decodeString(const unsigned char* in, const int in_length, char* out)
{
  char* out_ptr = out;
  const unsigned char* in_ptr = in;

  if ((in_length % 2) || (in_length < 0))
  {
    debugprintf(0, "Illegal length: %i", in_length);
    *out = 0;
    return;
  }

  for (int i= in_length / 2; i > 0; i--)
  {
    *out_ptr = (*in_ptr++ & 0xF) << 4;
    *out_ptr++ |= (*in_ptr++ & 0xF);
  }
  *out_ptr = 0; // make sure it is null terminated
}

// Read the bootCounter of the given engineID stored in the given file.
int getBootCounter(const char *fileName,
                   const OctetStr &engineId, unsigned int &boot)
{
  char line[MAX_LINE_LEN];
  char encoded[MAXLENGTH_ENGINEID * 2 + 2];
  int len = engineId.len();

  FILE *file;

  boot = 0;
  file = fopen(fileName, "r");
  if (file)
  {
    if (len > MAXLENGTH_ENGINEID)
    {
      debugprintf(2, "getBootCounter: engineId too long (len): (%i)", len);
      return SNMPv3_TOO_LONG_ERROR;
    }
    encodeString(engineId.data(), len, encoded);
    encoded[2*len]=' ';
    encoded[2*len + 1] = 0;

    while (fgets(line, MAX_LINE_LEN, file))
    {
      line[MAX_LINE_LEN - 1] = 0;
      /* ignore comments */
      if (line[0]=='#')
        continue;

      if (!strncmp(encoded, line, len*2 + 1))
      {
        /* line starts with engineId */
        char* ptr = line;
        /* skip until first space */
        while (*ptr != 0 && *ptr != ' ')
          ptr++;

        if (*ptr == 0)
        {
          fclose(file);
          debugprintf(1,"getBootCounter: Wrong line in file (file) (line):"
                      "(%s) (%s)", fileName, line);
          return SNMPv3_FILE_ERROR;
        }
        boot = atoi(ptr);
        fclose(file);
        debugprintf(5,"getBootCounter: found (bootCounter): (%i)", boot);
        return SNMPv3_OK;
      }
    }
    fclose(file);
    debugprintf(5,"getBootCounter: found no entry for engineId (%s)",
                OctetStr(engineId).get_printable());
    return SNMPv3_NO_ENTRY_ERROR;
  }

  debugprintf(1,"getBootCounter: Could not open (file): (%s)", fileName);
  return SNMPv3_FILEOPEN_ERROR;
}

// Store the bootCounter of the given engineID in the given file.
int saveBootCounter(const char *fileName,
                    const OctetStr &engineId, const unsigned int boot)
{
  char line[MAX_LINE_LEN];
  char tmpFileName[MAXLENGTH_FILENAME];
  char encoded[MAXLENGTH_ENGINEID * 2 + 2];
  int found = FALSE;
  int len = engineId.len();
  FILE *file_in, *file_out;

  tmpFileName[0] = 0;
  sprintf(tmpFileName, "%s.tmp",fileName);
  if (len > MAXLENGTH_ENGINEID) {
    debugprintf(1,"EngineID too long, ignoring last bytes.");
    len = MAXLENGTH_ENGINEID;
  }
  file_in  = fopen(fileName, "r");
  if (!file_in)
  {
    file_in  = fopen(fileName, "w");
    if (!file_in)
    {
      debugprintf(2,"saveBootCounter: could not create file (%s)",fileName);
      return SNMPv3_FILECREATE_ERROR;
    }
    debugprintf(5,"saveBootCounter: created new file (%s)",fileName);
    fputs("# \n",file_in);
    fputs("# This file was created by an SNMP++v3 application,\n", file_in);
    fputs("# it is used to store the snmpEngineBoots counters.\n", file_in);
    fputs("# \n",file_in);
    fputs("# Lines starting with '#' are comments.\n", file_in);
    fputs("# The snmpEngineBoots counters are stored as\n", file_in);
    fputs("# <encoded snmpEngineId> <bootCounter>\n", file_in);
    fputs("# \n", file_in);
    fclose(file_in);
    file_in = fopen(fileName, "r");
  }

  file_out = fopen(tmpFileName, "w");

  if ((file_in) && (file_out))
  {
    encodeString(engineId.data(), len, encoded);
    encoded[len*2] = ' ';
    encoded[len*2 + 1] = 0;

    while (fgets(line, MAX_LINE_LEN, file_in))
    {
      line[MAX_LINE_LEN - 1] = 0;
      if (!strncmp(encoded, line, len*2 + 1))
      {
        if (found)
        {
          debugprintf(2,"saveBootCounter: found engineId more than one time,"
                      " deleting this line (%s).", line);
          continue;
        }
        sprintf(line,"%s%i\n", encoded, boot);
        fputs(line, file_out);
        found = TRUE;
        continue;
      }
      fputs(line, file_out);
    }
    if (!found)
    {
      sprintf(line, "%s%i\n", encoded, boot);
      fputs(line, file_out);
    }
    fclose(file_in);
    fclose(file_out);
#ifdef WIN32
    _unlink(fileName);
#endif
    if (rename(tmpFileName, fileName))
    {
      debugprintf(2,"saveBootCounter: could not rename tmpfile (%s)"
                  " to file (%s)", tmpFileName, fileName);
      return SNMPv3_FILERENAME_ERROR;
    }
    debugprintf(5,"saveBootCounter: saved snmpEngineBoots counter");
    return SNMPv3_OK;
  }
  debugprintf(2,"saveBootCounter: could not open tmpfile (%s)"
              " or file (%s)", tmpFileName, fileName);
  return SNMPv3_FILEOPEN_ERROR;
}

#endif

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 
