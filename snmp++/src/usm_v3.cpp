/*_############################################################################
  _## 
  _##  usm_v3.cpp  
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
char usm_v3_cpp_version[]="@(#) SNMP++ $Id$";

#ifdef _AIX
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "snmp_pp/config_snmp_pp.h"

#ifdef _SNMPv3

#include "snmp_pp/v3.h"
#include "snmp_pp/usm_v3.h"
#include "snmp_pp/auth_priv.h"
#include "snmp_pp/reentrant.h"
#include "snmp_pp/mp_v3.h"
#include "snmp_pp/asn1.h"
#include "snmp_pp/vb.h"
#include "snmp_pp/pdu.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

// dont use locking
#undef REENTRANT
#define REENTRANT(x) x

#ifndef min
#define min(a,b) ( (a) < (b) ? (a) : (b) )
#endif

#define MAX_LINE_LEN 2048  // Max line length in usm user files

// structure for key update
struct UsmKeyUpdate
{
  OctetStr engineID;
  OctetStr securityName;
  OctetStr newPassword;
  OctetStr newKey;
  int type;
};

/* ------------------------- UsmTimeTable --------------------------*/

/**
 * This class provides a table for the time values (engine boots and
 * engine time) for snmp entities that are identified through their
 * engine id.
 *
 * @author Jochen Katz
 */
class USMTimeTable : public SnmpSynchronized
{
public:

  /**
   * Initialize the usmTimeTable.
   *
   * The usmTimeTable stores for each known engineID the engineBoots
   * and the difference to the local system time
   *
   * @param owner        - Pointer to the USM object that created this table
   * @param engine_boots - The new value for the snmpEngineBoots counter
   * @param result       - OUT: Result of the creation of the table
   */
  USMTimeTable(const USM *owner, const unsigned int engine_boots, int &result);

  ~USMTimeTable();

  /**
   * Add a new entry to the usmTimeTable. The caller is responsible for
   * not adding an engineID twice.
   *
   * @param engine_id    - The engineID of the SNMP entity
   * @param engine_boots - The engine boot counter
   * @param engine_time  - The engine time
   *
   * @return - SNMPv3_USM_ERROR (no memory) or SNMPv3_USM_OK
   */
  int add_entry(const OctetStr &engine_id,
                const long int engine_boots, const long int engine_time);

  /**
   * Return engineBoots and engineTime for a given engineID
   *
   * @param engine_id    - The engineID of the SNMP entity
   * @param engine_boots - OUT: boot counter (0 if not found)
   * @param engine_time  - OUT: engine time (0 if not found)
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_OK (entry found, values are filled)
   *           SNMPv3_USM_UNKNOWN_ENGINEID ( not found)
   */
  int get_time(const OctetStr &engine_id,
               long int &engine_boots, long int &engine_time);

  /**
   * Return the engineBoots and engineTime of this snmp entity.
   *
   * @param engine_boots - OUT: boot counter (0 if not found)
   * @param engine_time  - OUT: engine time (0 if not found)
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_OK (entry found, values are filled)
   */
  int get_local_time(long int &engine_boots, long int &engine_time);

  /**
   * Return the engineBoots value of this snmp entity.
   *
   * @return - engine_boots value if initialized, 0 else
   */
  unsigned long get_local_boots()
    { return (table ? table[0].engine_boots : 0); };

  /**
   * Return the engineTime value of this snmp entity.
   *
   * @return - engine_time value if initialized, 0 else
   */
  unsigned long get_local_time();

  /**
   * Check if the given values for engineBoots and engineTime are
   * in the time window. If the time values are ok, the entry in
   * the usmTimeTable is updated with the given time values.
   *
   * @param engine_id    - The engineID of the SNMP entity
   * @param engine_boots - The boot counter
   * @param engine_time  - The engine time
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_NOT_IN_TIME_WINDOW,
   *           SNMPv3_USM_OK (time ok),
   *           SNMPv3_USM_UNKNOWN_ENGINEID
   */
  int check_time(const OctetStr &engine_id,
                 const long int engine_boots, const long int engine_time);

  /**
   * Check if the given engineID is known: If the USM is in
   * the discovery mode, all engineIDs are accepted and entries
   * in the timeTable are created.
   *
   * @param engine_id - engine id to check
   *
   * @return - SNMPv3_USM_ERROR (not found and no discovery)
   *           SNMPv3_USM_OK (found or discovery set)
   */
  int check_engine_id(const OctetStr &engine_id);

private:
  struct Entry_T
  {
    unsigned char engine_id[MAXLENGTH_ENGINEID];
    int engine_id_len;
    long int engine_boots;
    long int time_diff;
    long int latest_received_time;
  };

  struct Entry_T *table; ///< Array of entries
  const USM *usm;  ///< Pointer to the USM, this table belongs to
  int max_entries; ///< the maximum number of entries
  int entries;     ///< the current amount of entries
};


/* ------------------------- UsmUserNameTable ----------------------*/
class USMUserNameTable : public SnmpSynchronized
{
public:
  USMUserNameTable(int &result);
  ~USMUserNameTable();

  /**
   * Add a new user to the usmUserNameTable. If the userName is already
   * known, the old entry is replaced.
   *
   * It is not recommended to add users with userName != securityName.
   *
   * @param user_name     - Unique userName
   * @param security_name - Unique securityName
   * @param auth_proto    - Possible values are:
   *                        SNMP_AUTHPROTOCOL_NONE,
   *                        SNMP_AUTHPROTOCOL_HMACMD5,
   *                        SNMP_AUTHPROTOCOL_HMACSHA
   * @param priv_proto    - Possible values are:
   *                        SNMP_PRIVPROTOCOL_NONE,
   *                        SNMP_PRIVPROTOCOL_DES,
   *                        SNMP_PRIVPROTOCOL_IDEA
   * @param auth_pass     - Secret password for authentication
   * @param priv_pass     - Secret password for privacy
   *
   * @return - SNMPv3_USM_OK or
   *           SNMP_v3_USM_ERROR (memory error, not initialized)
   */
  int add_entry(const OctetStr& user_name,
		const OctetStr& security_name,
		const long int  auth_proto,
		const long int  priv_proto,
		const OctetStr& auth_pass,
		const OctetStr& priv_pass);

  /**
   * Delete all occurences of the user with the given securityName
   * from the table.
   *
   * @param security_name - the securityName of the user
   *
   * @return - SNMPv3_USM_OK, SNMPv3_USM_ERROR (not initialized)
   */
  int delete_security_name(const OctetStr& security_name);

  /**
   * Private:
   *
   * Get the entry with the given securityName from the usmUserNameTable
   *
   * The caller has to call delete for the returned structure.
   *
   * @param security_name     -
   *
   * @return - pointer to the struct or NULL
   */
  struct UsmUserNameTableEntry* get_entry(const OctetStr &security_name);

  /**
   * Get the securityName from a userName
   *
   * @param user_name         -
   * @param user_name_len     -
   * @param security_name     - Buffer for the securityName
   *
   * @return - SNMPv3_USM_ERROR (not initialized, not found, buffer too small),
   *           SNMPv3_USM_OK
   */
  int get_security_name(const unsigned char *user_name,
			const long int user_name_len,
			OctetStr &security_name);

  /**
   * Get the userName from a securityName
   *
   * @param user_name         - Buffer for the userName
   * @param user_name_len     - Has to be set to the max length of the
   *                            buffer. Is set to the length of the found
   *                            securityName or to 0 if not found.
   * @param security_name     -
   * @param security_name_len -
   *
   * @return - SNMPv3_USM_ERROR (not initialized, not found, buffer too small),
   *           SNMPv3_USM_OK
   */
  int get_user_name(unsigned char *user_name, long int *user_name_len,
		    const unsigned char *security_name,
		    const long int security_name_len);

  /**
   * Save all entries into a file.
   */
  int save_to_file(const char *name, AuthPriv *ap);

  /**
   * Load the table from a file.
   */
  int load_from_file(const char *name, AuthPriv *ap);

  const UsmUserNameTableEntry *peek_first() const
    { if (entries > 0) return table; return 0; };

  const UsmUserNameTableEntry *peek_next(const UsmUserNameTableEntry *e) const;

private:
  struct UsmUserNameTableEntry *table;

  int max_entries; ///< the maximum number of entries
  int entries;     ///< the current amount of entries
};


/* ---------------------------- UsmUserTable ------------------- */

class USMUserTable : public SnmpSynchronized
{
public:
  USMUserTable(int &result);

  ~USMUserTable();

  /**
   * Get the number of valid entries in the table.
   *
   * @return - number of entries
   */
  int size();

  /**
   * Get the userName from a securityName
   *
   * @param user_name     - Buffer for the userName
   * @param user_name_len - Has to be set to the max length of the
   *                        buffer. Is set to the length of the found
   *                        securityName or to 0 if not found.
   * @param sec_name      -
   * @param sec_name_len  -
   *
   * @return - SNMPv3_USM_ERROR (not initialized, not found, buffer too small),
   *           SNMPv3_USM_OK
   */
  int get_user_name(unsigned char *user_name, long int *user_name_len,
                    const unsigned char *sec_name, const long sec_name_len);

  /**
   * Get the sec_name from a userName
   *
   * @param user_name       -
   * @param user_name_len   -
   * @param sec_name        - Object for the security name
   *
   * @return - SNMPv3_USM_ERROR (not initialized, not found, buffer too small),
   *           SNMPv3_USM_OK
   */
  int get_security_name(const unsigned char *user_name,
                        const long user_name_len,
                        OctetStr &sec_name);

  /**
   * Delete all entries of this user from the usmUserTable
   *
   * @param user_name - The userName that should be deleted
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_OK (user deleted or not in table)
   */
  int delete_entries(const OctetStr& user_name);

  /**
   * Delete the entry with the given userName and engineID
   * from the usmUserTable
   *
   * @param engine_id  - The engine id
   * @param user_name - The userName that should be deleted
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_OK (user deleted or not in table)
   */
  int delete_entry(const OctetStr& engine_id, const OctetStr& user_name);

  /**
   * Protected (for agent++):
   *
   * Get the user at the specified position of the usmUserTable.
   *
   * The caller is responsible to delete the entries usmUserEngineID,
   * user_name, usmUserSecurityName of the returned struct and the
   * struct itself.
   *
   * @param number - get the entry at position number (1...)
   *
   * @return - a pointer to the structure or NULL if number is out
   *           of range
   */
  struct UsmUserTableEntry *get_entry(const int number);

  /**
   * Get a user of the usmUserTable.
   *
   * The caller is responsible to delete the returned struct, but
   * not the objects within the struct.
   *
   * @param engine_id - Get a user for this engine id
   * @param sec_name  - Get the user with this security name
   *
   * @return - a pointer to the structure or NULL if the user is not found
   */
  struct UsmUserTableEntry *get_entry(const OctetStr &engine_id,
                                      const OctetStr &sec_name);


  /**
   * Get a user of the usmUserTable.
   *
   * There could be more than one entry with the given
   * sec_name. Always the first entry that is found is returned.
   *
   * The caller is responsible to delete thereturned struct, but
   * not the objects within the struct.
   *
   * @param sec_name - security name to search for
   *
   * @return - a pointer to the structure or NULL if the user is not found
   */
  struct UsmUserTableEntry *get_entry(const OctetStr &sec_name);

  /**
   * Public:
   *
   * Add or replace a user in the usmUserTable. The usmUserTable stores
   * users with their localized keys.
   *
   * @param engine_id     - The engine_id, the key was localized with
   * @param user_name     - The name of the user (in the USM)
   * @param sec_name      - The security name of the user, this name
   *                                is the same for all securityModels
   * @param auth_proto    - Possible values are:
   *                                SNMP_AUTHPROTOCOL_NONE,
   *                                SNMP_AUTHPROTOCOL_HMACMD5,
   *                                SNMP_AUTHPROTOCOL_HMACSHA
   * @param auth_key      - The key used for authentications
   * @param priv_proto    - Possible values are:
   *                                SNMP_PRIVPROTOCOL_NONE,
   *                                SNMP_PRIVPROTOCOL_DES,
   *                                SNMP_PRIVPROTOCOL_IDEA
   * @param priv_key      - The key used for privacy
   *
   * @return - SNMPv3_USM_OK
   *           SNMP_v3_USM_ERROR (not initialized, no memory)
   */
  int add_entry(const OctetStr &engine_id,
		const OctetStr &user_name,  const OctetStr &sec_name,
		const long int  auth_proto, const OctetStr &auth_key,
		const long int  priv_proto, const OctetStr &priv_key);

  /**
   * Replace a localized key of the user and engine_id in the
   * usmUserTable.
   *
   * @param user_name     - The name of the user in the USM
   * @param engine_id     - Change the localized key for the SNMP
   *                        entity with this engine_id
   * @param new_key       - The new key
   * @param key_type      - AUTHKEY, OWNAUTHKEY, PRIVKEY or OWNPRIVKEY
   *
   * @return - SNMPv3_USM_ERROR (no such entry or not initialized),
   *           SNMPv3_USM_OK
   */
  int update_key(const OctetStr &user_name,
                 const OctetStr &engine_id,
                 const OctetStr &new_key,
                 const int key_type);

  /**
   * Save all entries into a file.
   */
  int save_to_file(const char *name, AuthPriv *ap);

  /**
   * Load the table from a file.
   */
  int load_from_file(const char *name, AuthPriv *ap);

  const UsmUserTableEntry *peek_first() const
    { if (entries > 0) return table; return 0; };

  const UsmUserTableEntry *peek_next(const UsmUserTableEntry *e) const;

private:
  void delete_entry(const int nr);

  struct UsmUserTableEntry *table;

  int max_entries; ///< the maximum number of entries
  int entries;     ///< the current amount of entries
};



struct UsmSecurityParameters {
  unsigned char  msgAuthoritativeEngineID[MAXLENGTH_ENGINEID];
  long int       msgAuthoritativeEngineIDLength;
  long int       msgAuthoritativeEngineBoots;
  long int       msgAuthoritativeEngineTime;
  unsigned char  msgUserName[MAXLEN_USMUSERNAME];
  long int       msgUserNameLength;
  unsigned char *msgAuthenticationParameters;
  long int       msgAuthenticationParametersLength;
  unsigned char *msgPrivacyParameters;
  unsigned int   msgPrivacyParametersLength;
};




struct SecurityStateReference
{
  unsigned char  msgUserName[MAXLEN_USMUSERNAME]; int msgUserNameLength;
  unsigned char *securityName;                    int securityNameLength;
  unsigned char *securityEngineID;                int securityEngineIDLength;
  int authProtocol;
  unsigned char* authKey;                         int authKeyLength;
  int privProtocol;
  unsigned char* privKey;                         int privKeyLength;
  int securityLevel;
};




void USM::inc_stats_unsupported_sec_levels()
{
  if (usmStatsUnsupportedSecLevels == MAXUINT32)
    usmStatsUnsupportedSecLevels = 0;
  else
    usmStatsUnsupportedSecLevels++;
}

void USM::inc_stats_not_in_time_windows()
{
  if (usmStatsNotInTimeWindows == MAXUINT32)
    usmStatsNotInTimeWindows = 0;
  else
    usmStatsNotInTimeWindows++;
}

void USM::inc_stats_unknown_user_names()
{
  if (usmStatsUnknownUserNames == MAXUINT32)
    usmStatsUnknownUserNames = 0;
  else
    usmStatsUnknownUserNames++;
}

void USM::inc_stats_unknown_engine_ids()
{
  if (usmStatsUnknownEngineIDs == MAXUINT32)
    usmStatsUnknownEngineIDs = 0;
  else
    usmStatsUnknownEngineIDs++;
}

void USM::inc_stats_wrong_digests()
{
  if (usmStatsWrongDigests == MAXUINT32)
    usmStatsWrongDigests = 0;
  else
    usmStatsWrongDigests++;
}

void USM::inc_stats_decryption_errors()
{
  if (usmStatsDecryptionErrors == MAXUINT32)
    usmStatsDecryptionErrors = 0;
  else
    usmStatsDecryptionErrors++;
}


void USM::delete_sec_state_reference(struct SecurityStateReference *ssr)
{
  if (ssr)
  {
    ssr->msgUserName[0] = 0;
    if (ssr->securityName) delete [] ssr->securityName;
    if (ssr->securityEngineID) delete [] ssr->securityEngineID;
    if (ssr->authKey)
    {
      memset(ssr->authKey, 0, ssr->authKeyLength);
      delete [] ssr->authKey;
    }
    if (ssr->privKey)
    {
      memset(ssr->privKey, 0, ssr->privKeyLength);
      delete [] ssr->privKey;
    }
  }
  delete ssr;
}

struct SecurityStateReference *USM::get_new_sec_state_reference()
{
  struct SecurityStateReference *res = new SecurityStateReference;

  if (!res)
    return NULL;

  memset(res, 0, sizeof(struct SecurityStateReference));
  return res;
}


USM::USM(unsigned int engine_boots, const OctetStr &engine_id,
	 const v3MP *v3_mp,
	 unsigned int *msgID, int &result)
  : local_snmp_engine_id (engine_id),
    v3mp (v3_mp),

    discovery_mode (TRUE),

    usmStatsUnsupportedSecLevels (0),
    usmStatsNotInTimeWindows     (0),
    usmStatsUnknownUserNames     (0),
    usmStatsUnknownEngineIDs     (0),
    usmStatsWrongDigests         (0),
    usmStatsDecryptionErrors     (0),

    usm_add_user_cb (0)
{
  auth_priv = new AuthPriv(result);
  if (result != SNMPv3_USM_OK)
    return;
  auth_priv->add_default_modules();

  usm_user_name_table = new USMUserNameTable(result);
  if (result != SNMPv3_USM_OK)
    return;

  usm_user_table = new USMUserTable(result);
  if (result != SNMPv3_USM_OK)
    return;

#ifdef _TEST
  printf(" Testing DES:\n");
  PrivDES pd;
  pp_uint64 testsalt=0xbabec0de;
  pd.set_salt(&testsalt);

  const char *desplaintext[10];
  desplaintext[0] = "abcdefghijklmnopqrstuvwxyz123456";
  desplaintext[1] = "abcdefghijklmnopqrstuvwxyz1234567";
  desplaintext[2] = "abcdefghijklmnopqrstuvwxyz12345678";
  desplaintext[3] = "abcdefghijklmnopqrstuvwxyz123456789";
  desplaintext[4] = "abcdefghijklmnopqrstuvwxyz123456789A";
  desplaintext[5] = "abcdefghijklmnopqrstuvwxyz123456789AB";
  desplaintext[6] = "abcdefghijklmnopqrstuvwxyz123456789ABC";
  desplaintext[7] = "abcdefghijklmnopqrstuvwxyz123456789ABCD";
  desplaintext[8] = "abcdefghijklmnopqrstuvwxyz123456789ABCDE";
  desplaintext[9] = "abcdefghijklmnopqrstuvwxyz123456789ABCDEF";
  
  unsigned char desencrypted[80];
  unsigned char desdecrypted[80];
  unsigned char desprivparams[8];
  unsigned char deskey[17] = "illegal_des_key!";

  for (int i=0; i<9; i++)
  {
      unsigned int encrypt_len = 80;
      unsigned int decrypt_len = 80;
      unsigned int desprivparamslen = 8;

      memset(desencrypted, 'x', 80);
      memset(desdecrypted, 'y', 80);

      debughexcprintf(1, "Plaintext", (unsigned char*)desplaintext[i],
		      strlen(desplaintext[i]));

      int res = pd.encrypt(deskey, 16,
			   (unsigned char*)desplaintext[i],
			   strlen(desplaintext[i]),
			   desencrypted, &encrypt_len,
			   desprivparams, &desprivparamslen,
			   0x2340abcd, 0);

      printf("%d: Result of encryption is %d\n", i, res);
      debughexcprintf(1, "Encrypted", desencrypted, encrypt_len);

      res = pd.decrypt(deskey, 16,
		       desencrypted, encrypt_len,
		       desdecrypted, &decrypt_len,
		       desprivparams, desprivparamslen,
		       0x2340abcd, 0);
      printf("%d: Result of decryption is %d\n", i, res);
      debughexcprintf(1, "Decrypted", desdecrypted, decrypt_len);

      if (memcmp(desplaintext[i], desdecrypted, strlen(desplaintext[i])))
	  printf("\n********* FAILED **********\n");
      else
	  printf("\nOK\n");

  }



#if 0
  printf(" Testing SHA:\n");

  // test password2key-algor:
  unsigned char keysha[50];
  auth_priv->password_to_key_auth(...(unsigned char*)"maplesyrup",10,
                     (unsigned char*)"\0\0\0\0\0\0\0\0\0\0\0\2",12,keysha);
  printf("Output of PasswordToKey-algorithm for SHA:\n");
  for (int i=0; i< 20; i++) {
    printf("%02X ", keysha[i]);
    if ((i+1)%4==0) printf(" ");
    if ((i+1)%16==0) printf("\n");
  }
  printf("\nOutput should be (draft-ietf-snmpv3-usm-v2-02.txt):\n");
  printf("66 95 fe bc  92 88 e3 62  82 23 5f c7  15 1f 12 84\n97 b3 8f 3f\n");
  printf("\nTesting MD5:\n");
  unsigned char keymd5[50];
  apPasswordToKeyMD5((unsigned char*)"maplesyrup",10,
                     (unsigned char*)"\0\0\0\0\0\0\0\0\0\0\0\2",12,keymd5);
  printf("Output of PasswordToKey-algorithm for MD5:\n");
  for (int i=0; i< 16; i++) {
    printf("%02X ", keymd5[i]);
    if ((i+1)%4==0) printf(" ");
    if ((i+1)%16==0) printf("\n");
  }
  printf("Output ahould be (draft-ietf-snmpv3-usm-v2-02.txt):\n");
  printf("52 6f 5e ed  9f cc e2 6f  89 64 c2 93  07 87 d8 2b\n");

  printf("\nTesting IDEA:\n");
  unsigned char source[35] = "Hallo, das ist ein test!", encrypted[35], decrypted[35], params[8];
  int len_encrypted = 35, len_decrypted = 35;
  apIDEAEncryptData((unsigned char*)"1234567890abcdef",
                    source, 25, encrypted, &len_encrypted, params);

  apIDEADecryptData((unsigned char*)"1234567890abcdef",
                    encrypted, len_encrypted,
                     decrypted, &len_decrypted, params);


  printf("params:\n");
  for (int i=0; i< 8; i++) {
    printf("%02X ", params[i]);
    if ((i+1)%4==0) printf(" ");
    if ((i+1)%16==0) printf("\n");
  }
  printf("\nsource:\n");
  for (int i=0; i< 25; i++) {
    printf("%02X ", source[i]);
    if ((i+1)%4==0) printf(" ");
    if ((i+1)%16==0) printf("\n");
  }
  printf("\n");
  printf("encrypted:\n");
  for (int i=0; i< 25; i++) {
    printf("%02X ", encrypted[i]);
    if ((i+1)%4==0) printf(" ");
    if ((i+1)%16==0) printf("\n");
  }
  printf("\n");
  printf("decrypted:\n");
  for (int i=0; i< 25; i++) {
    printf("%02X ", decrypted[i]);
    if ((i+1)%4==0) printf(" ");
    if ((i+1)%16==0) printf("\n");
  }
  printf("\n");
  for (int i=0; i<25; i++)
    if (source[i]!=decrypted[i]) {
      printf("\n*** source != decrypted ****\n\n");
      break;
    }

  // test keyUpdate md5
  printf("\n Test KeyUpdate Algorithm:\n");
  printf("Test MD5:\n");
  OctetStr oldKey = OctetStr(keymd5, SNMPv3_AP_OUTPUT_LENGTH_MD5);

  unsigned char newkeymd5[50];
  apPasswordToKeyMD5((unsigned char*)"newsyrup",8,
                     (unsigned char*)"\0\0\0\0\0\0\0\0\0\0\0\2",12,newkeymd5);
  printf("Output of PasswordToKey-algorithm for MD5:\n");
  for (int i=0; i< 16; i++) {
    printf("%02X ", newkeymd5[i]);
    if ((i+1)%4==0) printf(" ");
    if ((i+1)%16==0) printf("\n");
  }
  printf("Output ahould be (draft-ietf-snmpv3-usm-v2-02.txt):\n");
  printf("87 02 1d 7b  d9 d1 01 ba  05 ea 6e 3b  f9 d9 bd 4a\n");
  OctetStr result;
  apNewKey(oldKey, OctetStr(newkeymd5, SNMPv3_AP_OUTPUT_LENGTH_MD5),
            result, SNMPv3_usmHMACMD5AuthProtocol);

  // test keyUpdate sha (auth)
  printf("\nTest SHA for authPassword:\n");
  oldKey = OctetStr(keysha, SNMPv3_AP_OUTPUT_LENGTH_SHA);

  unsigned char newkeysha[50];
  apPasswordToKeySHA((unsigned char*)"newsyrup",8,
                     (unsigned char*)"\0\0\0\0\0\0\0\0\0\0\0\2",12,newkeysha);
  printf("Output of PasswordToKey-algorithm for sha:\n");
  for (int i=0; i< SNMPv3_AP_OUTPUT_LENGTH_SHA; i++) {
    printf("%02X ", newkeysha[i]);
    if ((i+1)%4==0) printf(" ");
    if ((i+1)%16==0) printf("\n");
  }
  printf("Output ahould be (draft-ietf-snmpv3-usm-v2-02.txt):\n");
  printf("78 e2 dc ce  79 d5 94 03  b5 8c 1b ba  a5 bf f4 63 \n91 f1 cd 25\n");

  apNewKey(oldKey, OctetStr(newkeysha, SNMPv3_AP_OUTPUT_LENGTH_SHA),
            result, SNMPv3_usmHMACSHAAuthProtocol);
  // test keyUpdate sha (privPassword DES)
  printf("\nTest SHA for privPassword:\n");

  oldKey = OctetStr(keysha, SNMPv3_AP_OUTPUT_LENGTH_MD5);
  apNewKey(oldKey, OctetStr(newkeysha, SNMPv3_AP_OUTPUT_LENGTH_MD5),
            result, SNMPv3_usmHMACSHAAuthProtocol);
  printf("Result should be:\n");
  printf("00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00\n");
  printf("7e f8 d8 a4  c9 cd b2 6b  47 59 1c d8  52 ff 88 b5\n");
#endif
  /* test AES key extension algorithm */
  unsigned char key_sha[SNMPv3_USM_MAX_KEY_LEN];
  unsigned int key_sha_len = SNMPv3_USM_MAX_KEY_LEN;

  int res = auth_priv->password_to_key_priv(
    SNMP_AUTHPROTOCOL_HMACSHA,
    SNMP_PRIVPROTOCOL_AES256,
    (unsigned char*)"maplesyrup", 10,
    (unsigned char*)"\0\0\0\0\0\0\0\0\0\0\0\2", 12,
    key_sha, &key_sha_len);

  debugprintf(0, "aes key extension result %i, key_sha_len = %i.",
	      res, key_sha_len);
  debughexcprintf(0, "key_sha", key_sha, key_sha_len);

  unsigned char pt[56] = "This is a secret message, nobody is allowed to read it!";
  unsigned char *plaintext = pt;
  unsigned char ct[56];
  unsigned char *cipher = ct;
  unsigned int cipherlen = 56;
  unsigned char privpar[8];
  unsigned int privparlen = 8;

  Priv *priv = auth_priv->get_priv(SNMP_PRIVPROTOCOL_AES256);
  pp_uint64 salt = 0;
  priv->set_salt(&salt);
  auth_priv->encrypt_msg(SNMP_PRIVPROTOCOL_AES256,
			 key_sha, key_sha_len,
			 plaintext, 55, cipher, &cipherlen,
			 privpar, &privparlen,
			 0xdeadc0deUL, 0xbeefdedeUL);

  auth_priv->decrypt_msg(SNMP_PRIVPROTOCOL_AES256,
			 key_sha, key_sha_len,
			 cipher, 55, plaintext, &cipherlen,
			 privpar, privparlen,
			 0xdeadc0deUL, 0xbeefdedeUL);

#endif // _TEST

  usm_time_table = new USMTimeTable(this, engine_boots, result);
  if (result != SNMPv3_USM_OK)
    return;

  *msgID = (engine_boots & 0xFFFF) << 16;
}

USM::~USM()
{
  if (usm_time_table)
    delete usm_time_table;
  usm_time_table = NULL;

  if (usm_user_table)
    delete usm_user_table;
  usm_user_table = NULL;

  if (usm_user_name_table)
  {
    delete usm_user_name_table;
    usm_user_name_table = NULL;
  }

  if (auth_priv)
  {
    delete auth_priv;
    auth_priv = NULL;
  }
}



int USM::update_key(const unsigned char* user_name,
		    const long int user_name_len,
		    const unsigned char* engine_id,
		    const long int engine_id_len,
		    const unsigned char* new_key,
		    const long int new_key_len,
		    const int type_of_key)
{
  OctetStr key(new_key, new_key_len);
  int res;
  res = usm_user_table->update_key(OctetStr(user_name, user_name_len),
				   OctetStr(engine_id, engine_id_len),
				   key, type_of_key);
  key.clear();
  return res;
}

int USM::add_localized_user(const OctetStr &engine_id,
			    const OctetStr &user_name,
			    const OctetStr &security_name,
			    const long auth_protocol,
			    const OctetStr &auth_key,
			    const long priv_protocol,
			    const OctetStr &priv_key)
{
   return usm_user_table->add_entry(engine_id, user_name, security_name,
                                    auth_protocol, auth_key,
                                    priv_protocol, priv_key);
}


int USM::add_usm_user(const OctetStr& user_name,
		      const OctetStr& security_name,
		      const long int  auth_protocol,
		      const long int  priv_protocol,
		      const OctetStr& auth_password,
		      const OctetStr& priv_password)
{
  /*  delete localized entries if some exists */
  delete_localized_user(user_name);

  int result = usm_user_name_table->add_entry(user_name,security_name,
					      auth_protocol, priv_protocol,
					      auth_password, priv_password);
  if (result != SNMPv3_USM_OK)
    return result;

  struct UsmUser *dummy;
  dummy = get_user(local_snmp_engine_id, security_name);
  if (dummy) delete dummy;

  return SNMPv3_USM_OK;
}

int USM::add_usm_user(const OctetStr& security_name,
		      const long int  auth_protocol,
		      const long int  priv_protocol,
		      const OctetStr& auth_password,
		      const OctetStr& priv_password)
{
  // usmUserName:     UserName for UserbasedSecurityModel
  // usmSecurityName: UserName for all SecurityModels
  return add_usm_user(security_name, security_name,
		      auth_protocol, priv_protocol,
		      auth_password, priv_password);
}

int USM::delete_localized_user(const OctetStr& usmUserName)
{
  return usm_user_table->delete_entries(usmUserName);
}

int USM::delete_localized_user(const OctetStr& engine_id,
			       const OctetStr& user_name)
{
  return usm_user_table->delete_entry(engine_id, user_name);
}

struct UsmUser *USM::get_user(const OctetStr &engine_id,
			      const OctetStr &security_name)
{
  debugprintf(7,"USM::get_user: user (%s) engine_id (%s)",
              security_name.get_printable(),engine_id.get_printable());

  struct UsmUserNameTableEntry *name_table_entry = NULL;
  struct UsmUserTableEntry *user_table_entry = NULL;

  user_table_entry = usm_user_table->get_entry(engine_id, security_name);
  if (!user_table_entry)
  {
    name_table_entry = usm_user_name_table->get_entry(security_name);
    if (!name_table_entry)
    {
      user_table_entry = usm_user_table->get_entry(security_name);
      if ((user_table_entry) && (engine_id.len() == 0))
      {
        // there is a entry for this security_name in the usmUserTable
        // so return an entry for this user to do engine_id discovery
        struct UsmUser *res = new UsmUser;
        if (!res)
          return NULL;
        res->engineID = NULL;
        res->engineIDLength = 0;
        res->usmUserName = user_table_entry->usmUserName;
        res->usmUserNameLength = user_table_entry->usmUserNameLength;
        res->securityName = user_table_entry->usmUserSecurityName;
        res->securityNameLength = user_table_entry->usmUserSecurityNameLength;
        res->authProtocol = SNMPv3_usmNoAuthProtocol;
        res->authKey = NULL;
        res->authKeyLength = 0;
        res->privProtocol = SNMPv3_usmNoPrivProtocol;
        res->privKey = NULL;
        res->privKeyLength = 0;

        return res;
      }
      else
      {
        debugprintf(1, "USM::get_user: User unknown");
        return NULL;
      }
    }
    if (engine_id.len() == 0)
    {
      // do not add a user
      struct UsmUser *res = new UsmUser;
      if (!res)
      {
	delete name_table_entry;
        return NULL;
      }
      res->engineID           = NULL;
      res->engineIDLength     = 0;
      res->usmUserName        = name_table_entry->usmUserName.data();
      res->usmUserNameLength  = name_table_entry->usmUserName.len();
      res->securityName       = name_table_entry->usmUserSecurityName.data();
      res->securityNameLength = name_table_entry->usmUserSecurityName.len();
      res->authProtocol       = SNMPv3_usmNoAuthProtocol;
      res->authKey            = NULL;
      res->authKeyLength      = 0;
      res->privProtocol       = SNMPv3_usmNoPrivProtocol;
      res->privKey            = NULL;
      res->privKeyLength      = 0;

      delete name_table_entry;
      return res;
    }
    else
    {
      // We can add a new user:
      unsigned char privKey[SNMPv3_USM_MAX_KEY_LEN];
      unsigned char authKey[SNMPv3_USM_MAX_KEY_LEN];
      unsigned int authKeyLength = 0;
      unsigned int privKeyLength = 0;


      int res = auth_priv->password_to_key_auth(
                                     name_table_entry->usmUserAuthProtocol,
                                     name_table_entry->authPassword,
                                     name_table_entry->authPasswordLength,
                                     engine_id.data(), engine_id.len(),
                                     authKey, &authKeyLength);

      if (res != SNMPv3_USM_OK)
      {
        if (res == SNMPv3_USM_UNSUPPORTED_AUTHPROTOCOL)
        {
          debugprintf(0, "Cannot add User: Unsupported authprotocol (%i)",
                      name_table_entry->usmUserAuthProtocol);
        }
        else
        {
          debugprintf(0, "Cannot add User: Error (%i)", res);
        }
	delete name_table_entry;
        return NULL;
      }

      res = auth_priv->password_to_key_priv(
                                     name_table_entry->usmUserAuthProtocol,
                                     name_table_entry->usmUserPrivProtocol,
                                     name_table_entry->privPassword,
                                     name_table_entry->privPasswordLength,
                                     engine_id.data(), engine_id.len(),
                                     privKey, &privKeyLength);

      if (res != SNMPv3_USM_OK)
      {
        if (res == SNMPv3_USM_UNSUPPORTED_PRIVPROTOCOL)
        {
          debugprintf(0, "Cannot add User: Unsupported privprotocol (%i)",
                      name_table_entry->usmUserPrivProtocol);
        }
        else
        {
          debugprintf(0, "Cannot add User: Error (%i)", res);
        }
	delete name_table_entry;
        return NULL;
      }

      OctetStr akey(authKey, authKeyLength);
      OctetStr pkey(privKey, privKeyLength);
      add_localized_user(
        engine_id,
        name_table_entry->usmUserName,
        name_table_entry->usmUserSecurityName,
        name_table_entry->usmUserAuthProtocol, akey,
        name_table_entry->usmUserPrivProtocol, pkey);

      if (usm_add_user_cb)
      {
        // inform agent++ about new user
        debugprintf(5, "Informing agent++ about newly created user");
        usm_add_user_cb(engine_id,
                        name_table_entry->usmUserName,
                        name_table_entry->usmUserSecurityName,
                        name_table_entry->usmUserAuthProtocol, akey,
                        name_table_entry->usmUserPrivProtocol, pkey);
      }
      akey.clear();
      pkey.clear();

      user_table_entry = usm_user_table->get_entry(engine_id, security_name);
    }
    delete name_table_entry;
  }
  struct UsmUser *res = new UsmUser;
  if (!res)
    return NULL;
  res->engineID           = user_table_entry->usmUserEngineID;
  res->engineIDLength     = user_table_entry->usmUserEngineIDLength;
  res->usmUserName        = user_table_entry->usmUserName;
  res->usmUserNameLength  = user_table_entry->usmUserNameLength;
  res->securityName       = user_table_entry->usmUserSecurityName;
  res->securityNameLength = user_table_entry->usmUserSecurityNameLength;
  res->authProtocol       = user_table_entry->usmUserAuthProtocol;
  res->authKey            = user_table_entry->usmUserAuthKey;
  res->authKeyLength      = user_table_entry->usmUserAuthKeyLength;
  res->privProtocol       = user_table_entry->usmUserPrivProtocol;
  res->privKey            = user_table_entry->usmUserPrivKey;
  res->privKeyLength      = user_table_entry->usmUserPrivKeyLength;

  return res;
}

int USM::delete_usm_user(const OctetStr& security_name)
{
  usm_user_name_table->delete_security_name(security_name);

  unsigned char username[MAXLEN_USMUSERNAME + 1];
  long int length = MAXLEN_USMUSERNAME;

  if ((get_user_name(username, &length,
		     security_name.data(), security_name.len()))
      == SNMPv3_USM_OK)
    return delete_localized_user(OctetStr(username, length));
  return SNMPv3_USM_ERROR;
}

int USM::get_security_name(const unsigned char *user_name,
			   const long int user_name_len,
			   OctetStr &security_name)
{
  debugprintf(20,"USM::get_security_name: get  user (%s)",
              OctetStr(user_name,user_name_len).get_printable());

  int result;

  result = usm_user_name_table->get_security_name(user_name, user_name_len,
                                                  security_name);
  if (result == SNMPv3_USM_OK)
    return SNMPv3_USM_OK;

  result = usm_user_table->get_security_name(user_name, user_name_len,
                                             security_name);
  if (result == SNMPv3_USM_OK)
    return SNMPv3_USM_OK;

  debugprintf(1, "USM::get_security_name: User unknown");
  return SNMPv3_USM_ERROR;
}

int USM::get_user_name(unsigned char *user_name, long int *user_name_len,
		       const unsigned char *security_name,
		       const long int security_name_len)
{
  int result;
  long int  buf_len = *user_name_len;

  result = usm_user_name_table->get_user_name(user_name, user_name_len,
                                              security_name,
					      security_name_len);

  if (result == SNMPv3_USM_OK)
    return SNMPv3_USM_OK;

  *user_name_len = buf_len;

  result = usm_user_table->get_user_name(user_name, user_name_len,
                                         security_name, security_name_len);

  if (result == SNMPv3_USM_OK)
    return SNMPv3_USM_OK;

  debugprintf(1, "usmGetUsmUserName: User unknown");
  return SNMPv3_USM_ERROR;
}


void USM::delete_sec_parameters( struct UsmSecurityParameters *usp)
{
  usp->msgAuthoritativeEngineID[0] = 0;
  usp->msgAuthoritativeEngineIDLength = 0;
  usp->msgAuthoritativeEngineBoots = 0;
  usp->msgAuthoritativeEngineTime = 0;
  usp->msgUserName[0] = 0;
  usp->msgUserNameLength = 0;

  if (usp->msgAuthenticationParameters) {
    delete [] usp->msgAuthenticationParameters;
    usp->msgAuthenticationParameters = NULL;
  }
  usp->msgAuthenticationParametersLength = 0;
  if (usp->msgPrivacyParameters) {
    delete [] usp->msgPrivacyParameters;
    usp->msgPrivacyParameters = NULL;
  }
  usp->msgPrivacyParametersLength = 0;
}


struct UsmUserTableEntry *USM::get_user(int number)
{
  return usm_user_table->get_entry(number);
}

struct UsmUserNameTableEntry *USM::get_user(const OctetStr &security_name)
{
  return usm_user_name_table->get_entry(security_name);
}

int USM::get_user_count() const
{
  return usm_user_table->size();
}

DLLOPT void USM::add_user_added_callback(const usm_add_user_callback cb)
{
 usm_add_user_cb = cb;
}

int USM::get_time(const OctetStr &engine_id,
		  long int *engine_boots, long int *engine_time)
{
  return usm_time_table->get_time(engine_id, *engine_boots, *engine_time);
}

int USM::get_local_time(long int *engine_boots, long int *engine_time) const
{
  return usm_time_table->get_local_time(*engine_boots, *engine_time);
}

AuthPriv *USM::get_auth_priv()
{
  return auth_priv;
}

struct UsmKeyUpdate* USM::key_update_prepare(const OctetStr& securityName,
					     SnmpTarget& target,
					     const OctetStr& newPassword,
					     Pdu& pdu, int type,
					     int &status,
					     const OctetStr& oldpass,
					     const OctetStr& oldengid,
					     const OctetStr& newengid)
{
  // check address

  GenAddress genaddress;
  target.get_address(genaddress);
  UdpAddress udp_address(genaddress);
  if (!udp_address.valid()) {
    debugprintf(0, "usmPrepareKeyUpdate: Address invalid.");
    status = SNMPv3_USM_ADDRESS_ERROR;
    return NULL;
  }

  OctetStr engineID = "";
  // get engineID
  if (v3mp->get_from_engine_id_table(engineID,
				     (char*)udp_address.get_printable())
      != SNMPv3_MP_OK ) {
    debugprintf(0, "usmPrepareKeyUpdate: Could not find engineID of given address.");
    status = SNMPv3_USM_ADDRESS_ERROR;
    return NULL;
  }

  // get user
  struct UsmUser* user;
  user = get_user(engineID, securityName);

  if (user == NULL) {
    debugprintf(0, "usmPrepareKeyUpdate: Could not find user in usmTables.");
    status =  SNMPv3_USM_UNKNOWN_SECURITY_NAME;
    return NULL;
  }

  /* set old and new key */
  unsigned char key[SNMPv3_USM_MAX_KEY_LEN];
  unsigned int  key_len;
  OctetStr      newKey;
  OctetStr      oldKey;

  switch (type) {
    case AUTHKEY:
    case OWNAUTHKEY: {
      status = auth_priv->password_to_key_auth(
                                         user->authProtocol,
                                         newPassword.data(), newPassword.len(),
                                         engineID.data(), engineID.len(),
                                         key, &key_len);
      oldKey = OctetStr(user->authKey, user->authKeyLength);
      break;
    }
    case PRIVKEY:
    case OWNPRIVKEY: {
      status = auth_priv->password_to_key_priv(
                                         user->authProtocol,
                                         user->privProtocol,
                                         newPassword.data(), newPassword.len(),
                                         engineID.data(), engineID.len(),
                                         key, &key_len);
      oldKey = OctetStr(user->privKey, user->privKeyLength);
      break;
    }
    default: {
      debugprintf(0, "usmPrepareKeyUpdate: wrong type specified.");
      status = SNMPv3_USM_ERROR;
      return NULL;
    }
  }

  if (status != SNMPv3_USM_OK)
  {
    debugprintf(0, "usmPrepareKeyUpdate: password_to_key failed (code %i).",
                status);
    return NULL;
  }

  newKey = OctetStr(key, key_len);

  /* get value to set and random value */
  OctetStr newValue;
  OctetStr random_value;

  auth_priv->get_keychange_value(user->authProtocol,
                                 oldKey, newKey, newValue);

  char tmp_rand;
  for (int i = 0; i<30; i++) {
    tmp_rand = rand();
    random_value += tmp_rand;
  }

  // Oid in usmUserTable
  Oid userOid = Oid(oidUsmUserEntry);
  Oid publicOid  = Oid(oidUsmUserEntry);

  publicOid += "11";

  switch (type) {
    case AUTHKEY: {
      userOid += "6";
      break;
    }
    case OWNAUTHKEY: {
      userOid += "7";
      break;
    }
    case PRIVKEY: {
       userOid += "9";
       break;
    }
    case OWNPRIVKEY: {
      userOid += "10";
      break;
    }
    default: {
      debugprintf(0, "KeyChange error: wrong type:");
      status = SNMPv3_USM_ERROR;
      return NULL;
    }
  }

  userOid += engineID.len();
  publicOid += engineID.len();

  for (unsigned int j=0; j<engineID.len(); j++) {
    userOid += (engineID)[j];
    publicOid += (engineID)[j];
  }

  OctetStr os = securityName;
  userOid += os.len();
  publicOid += os.len();

  for (unsigned int k=0; k<os.len(); k++) {
    userOid += os[k];
    publicOid += os[k];
  }

  Vb vb;
  vb.set_oid(userOid);
  vb.set_value(newValue);
  pdu += vb;

  vb.set_oid(publicOid);
  vb.set_value(random_value);
  pdu += vb;

  struct UsmKeyUpdate *uku = new struct UsmKeyUpdate;
  uku->engineID = engineID;
  uku->securityName = securityName;
  uku->newPassword = newPassword;
  uku->newKey = newKey;
  uku->type = type;

  delete user;
  status = SNMPv3_USM_OK;
  return uku;
}

void USM::key_update_abort(struct UsmKeyUpdate *uku)
{
  delete uku;
}

int USM::key_update_commit(struct UsmKeyUpdate *uku, int update_type)
{
  if (!uku) return SNMPv3_USM_ERROR;
  int result;
  OctetStr userName;

  switch (update_type) {
    case USM_KeyUpdate: {
      result = update_key(uku->securityName.data(), uku->securityName.len(),
			  uku->engineID.data(), uku->engineID.len(),
			  uku->newKey.data(), uku->newKey.len(),
			  uku->type);
      delete uku;
      return result;
    }
    case USM_PasswordKeyUpdate: {
      result = update_key(uku->securityName.data(), uku->securityName.len(),
			  uku->engineID.data(), uku->engineID.len(),
			  uku->newKey.data(), uku->newKey.len(),
			  uku->type);
      struct UsmUserNameTableEntry *entry;
      entry = usm_user_name_table->get_entry(uku->securityName);
      if (!entry || (result != SNMPv3_USM_OK)) {
        delete uku;
        if (entry)
          delete entry;
        return SNMPv3_USM_ERROR;
      }

      switch (uku->type) {
        case OWNAUTHKEY:
        case AUTHKEY: {
          OctetStr privPass(entry->privPassword, entry->privPasswordLength);
          result = add_usm_user(uku->securityName, entry->usmUserName,
				entry->usmUserAuthProtocol,
				entry->usmUserPrivProtocol,
				uku->newPassword, privPass);
          delete uku;
          return result;
        }
        case OWNPRIVKEY:
        case PRIVKEY: {
          OctetStr authPass(entry->privPassword, entry->privPasswordLength);
          result = add_usm_user(uku->securityName, entry->usmUserName,
				entry->usmUserAuthProtocol,
				entry->usmUserPrivProtocol,
				authPass, uku->newPassword);
          delete uku;
          return result;
        }
        default: {
          delete uku;
          return SNMPv3_USM_ERROR;
        }
      }
    }
    case USM_PasswordAllKeyUpdate: {
      struct UsmUserNameTableEntry *entry;
      entry = usm_user_name_table->get_entry(uku->securityName);
      if (!entry) {
        delete uku;
        return SNMPv3_USM_ERROR;
      }

      switch (uku->type) {
        case OWNAUTHKEY:
        case AUTHKEY: {
          OctetStr privPass = OctetStr(entry->privPassword,
                                       entry->privPasswordLength);
          delete_usm_user(uku->securityName);
          result = add_usm_user(uku->securityName, entry->usmUserName,
				entry->usmUserAuthProtocol,
				entry->usmUserPrivProtocol,
				uku->newPassword, privPass);
          delete uku;
          delete entry;
          return result;
        }
        case OWNPRIVKEY:
        case PRIVKEY: {
          OctetStr authPass = OctetStr(entry->authPassword,
                                       entry->authPasswordLength);
          delete_usm_user(uku->securityName);
          result = add_usm_user(uku->securityName, entry->usmUserName,
				entry->usmUserAuthProtocol,
				entry->usmUserPrivProtocol,
				authPass, uku->newPassword);
          delete uku;
          delete entry;
          return result;
        }
        default: {
          delete uku;
          delete entry;
          return SNMPv3_USM_ERROR;
        }
      }
    }
  }
  delete uku;
  return SNMPv3_USM_ERROR;
}






int USM::generate_msg(
             unsigned char *globalData,       // message header, admin data
             int globalDataLength,
             int maxMessageSize,              // of the sending SNMP entity
             const OctetStr &securityEngineID,// authoritative SNMP entity
             const OctetStr &securityName,    // on behalf of this principal
             int  securityLevel,              // Level of Security requested
             unsigned char  *scopedPDU,       // message (plaintext) payload
             int scopedPDULength,
             struct SecurityStateReference *securityStateReference,
             unsigned char *wholeMsg,         // OUT complete generated message
             int *wholeMsgLength)             // OUT length of generated message
{
  unsigned char buf[MAXLENGTH_BUFFER];
  unsigned char *bufPtr = (unsigned char*)&buf;
  unsigned char buf2[MAXLENGTH_BUFFER];
  unsigned char *buf2Ptr = (unsigned char*)&buf2;
  unsigned char *wholeMsgPtr;
  int startAuthPar = 0;
  struct UsmUser *user = NULL;
  struct UsmSecurityParameters usmSecurityParams;

  int bufLength = 0;
  unsigned int buf2Length = 0;
  int totalLength = 0;             // Bytes encoded
  int restLength = maxMessageSize; // max Bytes left in packet-buffer
  int rc;
  int responseMsg = 0;

  if (securityStateReference) {
    // this is a response message
    responseMsg = 1;
    user = new UsmUser;
    if (!user)
      return SNMPv3_USM_ERROR;
    if (securityStateReference->securityEngineID) {
      user->engineIDLength = securityStateReference->securityEngineIDLength;
      user->engineID       = securityStateReference->securityEngineID;
    } else {
      user->engineIDLength = securityEngineID.len();
      user->engineID       = v3strcpy(securityEngineID.data(),
				      securityEngineID.len());
    }

    user->usmUserName = new unsigned char[MAXLEN_USMUSERNAME + 1];
    if (securityStateReference->securityName)
    {
      user->securityName       = securityStateReference->securityName;
      user->securityNameLength = securityStateReference->securityNameLength;
      memcpy(user->usmUserName, securityStateReference->msgUserName,
	     securityStateReference->msgUserNameLength);
      user->usmUserNameLength  = securityStateReference->msgUserNameLength;
    }
    else
    {
      user->securityNameLength = securityName.len();
      user->securityName = v3strcpy(securityName.data(), securityName.len());
      if (securityStateReference->msgUserNameLength)
      {
        securityStateReference->msgUserName[0] = 0;
        securityStateReference->msgUserNameLength = 0;
      }
      user->usmUserNameLength = MAXLEN_USMUSERNAME;
      get_user_name(user->usmUserName, &user->usmUserNameLength,
		    securityName.data(), securityName.len());
      if ((user->usmUserNameLength == 0) &&
          (securityName.len() <= MAXLEN_USMUSERNAME)) {
        memcpy(user->usmUserName, securityName.data(), securityName.len());
	user->usmUserName[securityName.len()] = 0;
        user->usmUserNameLength = securityName.len();
      }
    }
    user->authProtocol       = securityStateReference->authProtocol;
    user->authKey            = securityStateReference->authKey;
    user->authKeyLength      = securityStateReference->authKeyLength;
    user->privProtocol       = securityStateReference->privProtocol;
    user->privKeyLength      = securityStateReference->privKeyLength;
    user->privKey            = securityStateReference->privKey;

    delete securityStateReference;
    securityStateReference = NULL;
  }
  else
  {
    if (securityEngineID.len() == 0)
    {
      // discovery
      user = new UsmUser;
      if (!user)
	return SNMPv3_USM_ERROR;
      memset(user, 0, sizeof(UsmUser));
    }
    else
    {
      // search for user in usmUserTable
      user = get_user(securityEngineID, securityName);

      if (!user) {
	debugprintf(0, "USM: User unknown!");
	return SNMPv3_USM_UNKNOWN_SECURITY_NAME;
      }
    }
  }

  if (securityEngineID.len() > MAXLENGTH_ENGINEID)
  {
    debugprintf(0, "engine_id too long %i > %i",
		securityEngineID.len(), MAXLENGTH_ENGINEID);
    delete user;
    return SNMPv3_USM_ERROR;
  }

  if (user->usmUserNameLength > MAXLEN_USMUSERNAME)
  {
    debugprintf(0, "user name too long %i > %i",
		user->usmUserNameLength, MAXLEN_USMUSERNAME);
    delete user;
    return SNMPv3_USM_ERROR;
  }

  usmSecurityParams.msgAuthoritativeEngineIDLength = securityEngineID.len();
  usmSecurityParams.msgUserNameLength = user->usmUserNameLength;
  memcpy(usmSecurityParams.msgUserName,
         user->usmUserName, user->usmUserNameLength);
  memcpy(usmSecurityParams.msgAuthoritativeEngineID,
         securityEngineID.data(), securityEngineID.len());

  usmSecurityParams.msgPrivacyParametersLength = 0;
  usmSecurityParams.msgPrivacyParameters = NULL;

  usmSecurityParams.msgAuthenticationParametersLength = 0;
  usmSecurityParams.msgAuthenticationParameters = NULL;

  if (securityLevel >= SNMP_SECURITY_LEVEL_AUTH_NOPRIV)
  {
    // get engineBoots, engineTime
    rc = usm_time_table->get_time(
                           securityEngineID,
                           usmSecurityParams.msgAuthoritativeEngineBoots,
                           usmSecurityParams.msgAuthoritativeEngineTime);
    if (rc == SNMPv3_USM_UNKNOWN_ENGINEID) {
      usm_time_table->add_entry(securityEngineID,
                                usmSecurityParams.msgAuthoritativeEngineBoots,
                                usmSecurityParams.msgAuthoritativeEngineTime);
    }
    if (rc == SNMPv3_USM_ERROR) {
      debugprintf(0, "usm: usmGetTime error.");
      delete user;
      return SNMPv3_USM_ERROR;
    }
  }

  if (securityLevel == SNMP_SECURITY_LEVEL_AUTH_PRIV)
  {
    usmSecurityParams.msgPrivacyParametersLength
             = auth_priv->get_priv_params_len(user->privProtocol);
    usmSecurityParams.msgPrivacyParameters
             = new unsigned char[usmSecurityParams.msgPrivacyParametersLength];

    // encrypt Message
    int enc_result = auth_priv->encrypt_msg(
                               user->privProtocol,
			       user->privKey, user->privKeyLength,
                               scopedPDU, scopedPDULength,
                               buf2Ptr, &buf2Length,
                               usmSecurityParams.msgPrivacyParameters,
                               &usmSecurityParams.msgPrivacyParametersLength,
                               usmSecurityParams.msgAuthoritativeEngineBoots,
			       usmSecurityParams.msgAuthoritativeEngineTime);
    if (enc_result != SNMPv3_USM_OK)
    {
      int return_value;

      if (user->privProtocol == SNMP_PRIVPROTOCOL_NONE)
      {
        debugprintf(0, "usm: Privacy requested, but no UserPrivProtocol");
        return_value = SNMPv3_USM_UNSUPPORTED_SECURITY_LEVEL;
      }
      else
      {
        return_value = SNMPv3_USM_ENCRYPTION_ERROR;
      }

      debugprintf(0, "usm: Encryption error (result %i).", enc_result);

      delete_sec_parameters(&usmSecurityParams);
      if (responseMsg)  delete_user_ptr(user);
      delete user;
      return return_value;
    }

    bufPtr = asn_build_string(bufPtr, &restLength,
                              (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STR),
                              buf2Ptr, buf2Length);
    if (bufPtr==NULL) {
      debugprintf(0, "usm: Encoding Error");
      delete user;
      return SNMPv3_USM_ERROR;
    }
    bufLength = bufPtr - (unsigned char *)&buf;
    totalLength =  bufLength;
    bufPtr = (unsigned char *)&buf;
    memcpy(buf2Ptr, bufPtr, bufLength);
    buf2Length = bufLength;

  } else { // (securityLevel != SNMP_SECURITY_LEVEL_AUTH_PRIV)
    buf2Ptr = scopedPDU;
    buf2Length = scopedPDULength;
    totalLength = scopedPDULength;
  }
  if (bufPtr==NULL) {
    debugprintf(0, "usm: Encoding Error");
    return SNMPv3_USM_ERROR;
  }

  totalLength += bufPtr - (unsigned char*)&buf;
  memcpy(bufPtr, buf2Ptr, buf2Length);
  bufLength = totalLength;

  debugprintf(21, "buf after privacy:");
  debughexprintf(21, buf, bufLength);

  wholeMsgPtr = wholeMsg;

  if (securityLevel >= SNMP_SECURITY_LEVEL_AUTH_NOPRIV)
  {
    /* Build message with authentication */
    usmSecurityParams.msgAuthenticationParametersLength
                         = auth_priv->get_auth_params_len(user->authProtocol);
    usmSecurityParams.msgAuthenticationParameters
      = new unsigned char[usmSecurityParams.msgAuthenticationParametersLength];
    memset((char*)(usmSecurityParams.msgAuthenticationParameters), 0,
           usmSecurityParams.msgAuthenticationParametersLength);

    wholeMsgPtr = build_whole_msg(wholeMsgPtr, &maxMessageSize,
				  globalData, globalDataLength,
				  &startAuthPar, // for MD5, SHA,...
				  usmSecurityParams,
				  (unsigned char*)&buf,
				  bufLength);   // the msgData
    if (wholeMsgPtr == NULL)
    {
      debugprintf(0, "usm: could not generate wholeMsg");
      delete_sec_parameters(&usmSecurityParams);
      if (responseMsg)
        delete_user_ptr(user);
      delete user;
      return SNMPv3_USM_ERROR;
    }
    *wholeMsgLength = wholeMsgPtr - wholeMsg;

    rc = auth_priv->auth_out_msg(user->authProtocol,
                                 user->authKey,
                                 wholeMsg, *wholeMsgLength,
                                 wholeMsg + startAuthPar);

    if (rc!=SNMPv3_USM_OK)
    {
      debugprintf(0, "usm: Authentication error for outgoing message."
                  " error code (%i).", rc);
      delete_sec_parameters(&usmSecurityParams);
      if (responseMsg)
        delete_user_ptr(user);
      delete user;
      return rc;
    }
  }
  else
  {
    //build Message without authentication

    // Set engineBoots and enigneTime to zero!
    usmSecurityParams.msgAuthoritativeEngineBoots = 0;
    usmSecurityParams.msgAuthoritativeEngineTime  = 0;

    usmSecurityParams.msgAuthenticationParametersLength = 0;
    usmSecurityParams.msgAuthenticationParameters = 0;

    wholeMsgPtr = build_whole_msg(wholeMsgPtr, &maxMessageSize,
				  globalData, globalDataLength,
				  &startAuthPar, // dummy ( no auth)
				  usmSecurityParams,
				  (unsigned char*)&buf,
				  bufLength);   // the msgData
    if (wholeMsgPtr == NULL) {
      debugprintf(0, "usm: could not generate wholeMsg");
      delete_sec_parameters(&usmSecurityParams);
      if (responseMsg) {
        delete_user_ptr(user);
      }
      delete user;
      return SNMPv3_USM_ERROR;
    }
    *wholeMsgLength = wholeMsgPtr - wholeMsg;
  }

  debugprintf(21, "Complete Whole Msg:");
  debughexprintf(21, wholeMsg, *wholeMsgLength);

  delete_sec_parameters(&usmSecurityParams);
  if (responseMsg) {
    delete_user_ptr(user);
  }
  delete user;
  return SNMPv3_USM_OK;

}

int USM::process_msg(
            int maxMessageSize,               // of the sending SNMP entity
            unsigned char *securityParameters,// for the received message
            int securityParametersLength,
            int securityParametersPosition,
            long int securityLevel,           // Level of Security
            unsigned char *wholeMsg,          // as received on the wire
            int wholeMsgLength,               // length as received on the wire
            unsigned char *msgData,
            int msgDataLength,
	    OctetStr &security_engine_id,     // authoritative SNMP entity
	    OctetStr &security_name,          //identification of the principal
            unsigned char *scopedPDU,         // message (plaintext) payload
            int *scopedPDULength,
            long *maxSizeResponseScopedPDU, // maximum size of the Response PDU
            struct SecurityStateReference *securityStateReference
                                               // reference to security state
            )                                  // information, needed for response
{
  unsigned char* sp = securityParameters;
  int spLength = securityParametersLength;
  unsigned char type;
  long int engineBoots, engineTime;
  unsigned char authParam[SNMPv3_AP_MAXLENGTH_AUTHPARAM];
  unsigned char privParam[SNMPv3_AP_MAXLENGTH_PRIVPARAM];
  int authParamLength = SNMPv3_AP_MAXLENGTH_AUTHPARAM;
  int privParamLength = SNMPv3_AP_MAXLENGTH_PRIVPARAM;
  unsigned char encryptedScopedPDU[MAXLENGTH_BUFFER];
  int encryptedScopedPDULength = msgDataLength;
  struct UsmUser *user = NULL;
  int rc;
  int notInTime = 0;

  // check securityParameters
  sp = asn_parse_header( sp, &spLength, &type);
  if (sp == NULL){
    debugprintf(0, "bad header of securityParameters");
    return SNMPv3_USM_PARSE_ERROR;
  }
  debugprintf(3, "Parsed securityParametersLength = 0x%x", spLength);

  if (type != (ASN_SEQUENCE | ASN_CONSTRUCTOR)){
    debugprintf(0, "wrong type in header of securityParameters");
    return SNMPv3_USM_PARSE_ERROR;
  }

  // extract security parameters
  {
    int len = MAXLENGTH_ENGINEID + 1;
    unsigned char data[MAXLENGTH_ENGINEID + 1];
    sp = asn_parse_string( sp, &spLength, &type, data, &len);

    debugprintf(3, "Parsed securityEngineID, length = 0x%x", len);
    if (sp==NULL) {
      debugprintf(0, "bad parse of securityEngineID");
      return SNMPv3_USM_PARSE_ERROR;
    }
    security_engine_id.set_data(data, len);
  }

  sp = asn_parse_int(sp, &spLength,
                     &type, &engineBoots, sizeof(engineBoots));
  if ((sp == NULL) || (engineBoots < 0)) {
    debugprintf(0, "bad parse of engineBoots");
    return SNMPv3_USM_PARSE_ERROR;
  }

  sp = asn_parse_int(sp, &spLength,
                     &type, &engineTime, sizeof(engineTime));
  if ((sp == NULL) || (engineTime < 0)) {
    debugprintf(0, "bad parse of engineTime");
    return SNMPv3_USM_PARSE_ERROR;
  }

  debugprintf(3, "Parsed engineBoots(0x%lx), engineTime(0x%lx)",
	      engineTime, engineBoots);

  unsigned char usmUserName[MAXLEN_USMUSERNAME + 1];
  int usmUserNameLength = MAXLEN_USMUSERNAME;

  sp = asn_parse_string( sp, &spLength, &type,
                         (unsigned char*)&usmUserName, &usmUserNameLength);

  if (sp==NULL) {
    debugprintf(0, "bad parse of usmUserName");
    return SNMPv3_USM_PARSE_ERROR;
  }

  sp = asn_parse_string( sp, &spLength, &type,
                         (unsigned char*)&authParam, &authParamLength);

  if (sp==NULL) {
    debugprintf(0, "bad parse of msgAuthenticationParameters");
    return SNMPv3_USM_PARSE_ERROR;
  }
  int authParametersPosition = securityParametersPosition +
                               (sp - securityParameters) - authParamLength;

  sp = asn_parse_string( sp, &spLength, &type,
                         (unsigned char*)&privParam, &privParamLength);

  if (sp==NULL) {
    debugprintf(0, "bad parse of msgPrivacyParameters");
    return SNMPv3_USM_PARSE_ERROR;
  }
  if (spLength !=0) {
    debugprintf(0, "Error Parsing msgPrivacyParameters");
    return SNMPv3_USM_PARSE_ERROR;
  }
  debugprintf(5, "Parsed usmUserName length(0x%x)"
	      " msgAuthenticationParameters length(0x%x)"
	      " msgPrivacyParameters length(0x%x)",
	      usmUserNameLength, authParamLength, privParamLength);

  // prepare securityStateReference
  if (usmUserNameLength > MAXLEN_USMUSERNAME)
  {
    debugprintf(0, "user name too long: %i > %i.",
		usmUserNameLength, MAXLEN_USMUSERNAME);
    return SNMPv3_USM_PARSE_ERROR;
  }

  securityStateReference->msgUserNameLength = usmUserNameLength;
  memcpy(securityStateReference->msgUserName, usmUserName,
         securityStateReference->msgUserNameLength);

  securityStateReference->securityEngineIDLength = security_engine_id.len();
  securityStateReference->securityEngineID =
    new unsigned char [securityStateReference->securityEngineIDLength];
  memcpy(securityStateReference->securityEngineID, security_engine_id.data(),
         securityStateReference->securityEngineIDLength);

  securityStateReference->securityLevel = securityLevel;

  securityStateReference->securityNameLength = 0;
  securityStateReference->securityName = NULL;
  securityStateReference->authProtocol = 1;
  securityStateReference->privProtocol = 1;
  securityStateReference->authKey = NULL;
  securityStateReference->privKey = NULL;

  // in case we return with error,
  // perhaps v3MP can decode it (requestID!!!)
  memcpy(scopedPDU, msgData, msgDataLength);
  *scopedPDULength = msgDataLength;

  if ((security_engine_id.len() == 0) ||
      (usm_time_table->check_engine_id(security_engine_id) != SNMPv3_USM_OK ))
  {
    inc_stats_unknown_engine_ids();

    // *+* REPORT *+*
    securityStateReference->securityLevel = SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV;
    security_name.set_data(usmUserName, usmUserNameLength);

    debugprintf(2, "USM: EngineID unknown");
    return SNMPv3_USM_UNKNOWN_ENGINEID;
  }

  // get securityName:
  if ((usmUserNameLength) ||
      (securityLevel != SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV))
  {
    rc = get_security_name(usmUserName, usmUserNameLength, security_name);
    if (rc != SNMPv3_USM_OK) {
      inc_stats_unknown_user_names();
      security_name.set_data(usmUserName, usmUserNameLength);

      debugprintf(2,"usmProcessMsg: unknown user (%s)",
		  security_name.get_printable());
      return SNMPv3_USM_UNKNOWN_SECURITY_NAME;
    }
  }
  else
  {
    debugprintf(2, "Accepting zero length user/security name.");
    security_name = "";
  }

  securityStateReference->securityNameLength = security_name.len();
  securityStateReference->securityName =
    new unsigned char [securityStateReference->securityNameLength];
  memcpy(securityStateReference->securityName, security_name.data(),
         securityStateReference->securityNameLength);

  // get user from LCD (usmUserTable)
  if (usmUserNameLength)
  {
    user = get_user(security_engine_id, security_name);

    if (!user) {
      inc_stats_unknown_user_names();
      debugprintf(0, "usmProcessMsg: unknown user");
      return SNMPv3_USM_UNKNOWN_SECURITY_NAME;
    }
  }
  else
  {
    user = new UsmUser;
    if (!user)
    {
      debugprintf(0, "Memory error");
      return SNMPv3_USM_ERROR;
    }
    memset(user, 0, sizeof(UsmUser));
  }

  if (((securityLevel > SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV) &&
       (user->authProtocol == SNMPv3_usmNoAuthProtocol)) ||
      ((securityLevel == SNMP_SECURITY_LEVEL_AUTH_PRIV) &&
       (user->privProtocol == SNMPv3_usmNoPrivProtocol))) {
    inc_stats_unsupported_sec_levels();
    debugprintf(0, "usmProcessMsg: unsupported Securitylevel");
    delete user;
    return SNMPv3_USM_UNSUPPORTED_SECURITY_LEVEL;
  }

  if (securityLevel > SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV)
  {
    rc = auth_priv->auth_inc_msg(user->authProtocol,
                                 user->authKey,
                                 wholeMsg, wholeMsgLength,
                                 wholeMsg + authParametersPosition,
				 authParamLength);
    if (rc != SNMPv3_USM_OK)
    {
      switch (rc)
      {
        case SNMPv3_USM_AUTHENTICATION_FAILURE:
          debugprintf(0, "usmProcessMsg: Authentication failure.");
	  inc_stats_wrong_digests();
          /* set securityLevel for Report */
          break;
        case SNMPv3_USM_UNSUPPORTED_AUTHPROTOCOL:
          debugprintf(0, "usmProcessMsg: unknown AuthProtocol");
	  inc_stats_unsupported_sec_levels();
        default:
          debugprintf(0, "usmProcessMsg: error authenticating msg."
                      " Error code (%i).", rc);
          // todo: is it ok to increment this counter?
	  inc_stats_unsupported_sec_levels();
          break;
      }
      securityStateReference->securityLevel= SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV;
      delete user;
      return rc;
    }

    rc = usm_time_table->check_time(security_engine_id,
				    engineBoots, engineTime);
    if (rc == SNMPv3_USM_NOT_IN_TIME_WINDOW) {
      inc_stats_not_in_time_windows();
      debugprintf(2, "***Message not in TimeWindow!");
      notInTime = 1;
    }
    if (rc == SNMPv3_USM_UNKNOWN_ENGINEID) {
      debugprintf(0, "***EngineID not in timeTable!");
      delete user;
      return rc;
    }
  }

  *scopedPDULength = MAXLENGTH_BUFFER;

  // decrypt ScopedPDU if message is in time window
  if ((securityLevel == SNMP_SECURITY_LEVEL_AUTH_PRIV)
      && (!notInTime)) {
    msgData = asn_parse_string( msgData, &msgDataLength, &type,
                                encryptedScopedPDU, &encryptedScopedPDULength);
    if (msgData == NULL){
      debugprintf(0, "usmProcessMsg: bad header of encryptedPDU");
      delete user;
      return SNMPv3_USM_PARSE_ERROR;
    }

    // decrypt Message
    unsigned int tmp_length = *scopedPDULength;
    int dec_result = auth_priv->decrypt_msg(
                                  user->privProtocol,
                                  user->privKey, user->privKeyLength,
                                  encryptedScopedPDU, encryptedScopedPDULength,
                                  scopedPDU, &tmp_length,
                                  (unsigned char*)&privParam, privParamLength,
				  engineBoots, engineTime);
    *scopedPDULength = tmp_length;
    if (dec_result != SNMPv3_USM_OK)
    {
      int return_value;
      if (dec_result == SNMPv3_USM_UNSUPPORTED_PRIVPROTOCOL)
      {
        debugprintf(0, "usmProcessMsg: unknown PrivacyProtocol");
	inc_stats_unsupported_sec_levels();
        return_value = SNMPv3_USM_UNSUPPORTED_PRIVPROTOCOL;
      }
      else // catch all
      {
        debugprintf(0, "usmProcessMsg: Decryption error (result %i).",
		    dec_result);
	inc_stats_decryption_errors();
        return_value = SNMPv3_USM_DECRYPTION_ERROR;
      }
      delete user;
      return return_value;
    }

    debugprintf(21, "scopedPDU(1):");
    debughexprintf(21, scopedPDU, *scopedPDULength);

    // test for decryption error
    // first byte 0x30
    if (scopedPDU[0] != (ASN_CONSTRUCTOR | ASN_SEQUENCE)) {
      debugprintf(0, "Decryption error detected");
      inc_stats_decryption_errors();
      delete user;
      return SNMPv3_USM_DECRYPTION_ERROR;
    }
  }
  else {
    // message was not encrypted
    memcpy(scopedPDU, msgData, msgDataLength);
    *scopedPDULength = msgDataLength;
  }

  *maxSizeResponseScopedPDU = maxMessageSize - (wholeMsgLength - *scopedPDULength);

  security_name.set_data(user->securityName, user->securityNameLength);

  securityStateReference->authProtocol = user->authProtocol;
  securityStateReference->privProtocol = user->privProtocol;

  securityStateReference->authKeyLength = user->authKeyLength;
  securityStateReference->authKey =
    new unsigned char [securityStateReference->authKeyLength];
  memcpy(securityStateReference->authKey, user->authKey,
         securityStateReference->authKeyLength);

  securityStateReference->privKeyLength = user->privKeyLength;
  securityStateReference->privKey =
    new unsigned char [securityStateReference->privKeyLength];
  memcpy(securityStateReference->privKey, user->privKey,
         securityStateReference->privKeyLength);

  // the pointers in user are pointing in usmUserTable,
  // don't delete them!
  delete user;

  if (notInTime)
    return SNMPv3_USM_NOT_IN_TIME_WINDOW;

  return SNMPv3_USM_OK;
}

unsigned char *USM::build_sec_params(unsigned char *outBuf, int *maxLength,
				     struct UsmSecurityParameters sp,
				     int *position)
{
  unsigned char buf[MAXLENGTH_BUFFER];
  unsigned char *bufPtr = (unsigned char*)&buf;
  unsigned char *outBufPtr = outBuf;
  int length = *maxLength;
  int totalLength;

  debugprintf(5, "Coding octstr sp.msgAuthoritativeEngineID, length = 0x%lx",
	      sp.msgAuthoritativeEngineIDLength);
  bufPtr = asn_build_string(bufPtr, &length,
                            (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STR),
                            sp.msgAuthoritativeEngineID,
                            sp.msgAuthoritativeEngineIDLength);
  if (bufPtr == NULL) {
    debugprintf(0, "usmBuildSecurityParameters error coding engineid");
    return NULL;
  }

  debugprintf(5, "Coding int sp.msgAuthoritativeEngineBoots = 0x%lx",
	      sp.msgAuthoritativeEngineBoots);
  bufPtr = asn_build_int(bufPtr, &length,
                         (unsigned char )(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
                         (long*)&sp.msgAuthoritativeEngineBoots,
                         sizeof(sp.msgAuthoritativeEngineBoots));

  if (bufPtr == NULL) {
    debugprintf(0, "usmBuildSecurityParameters error coding engineboots");
    return NULL;
  }

  debugprintf(5, "Coding int sp.msgAuthoritativeEngineTime = 0x%lx",
	      sp.msgAuthoritativeEngineTime);
  bufPtr = asn_build_int(bufPtr, &length,
                         (unsigned char )(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
                         (long*)&sp.msgAuthoritativeEngineTime,
                         sizeof(sp.msgAuthoritativeEngineTime));

  if (bufPtr == NULL) {
    debugprintf(0, "usmBuildSecurityParameters error coding enginetime");
    return NULL;
  }

  debugprintf(5, "Coding octstr sp.msgUserName, length = 0x%lx",
	      sp.msgUserNameLength);
  bufPtr = asn_build_string(bufPtr, &length,
                            (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STR),
                            sp.msgUserName,
                            sp.msgUserNameLength);
  if (bufPtr == NULL) {
    debugprintf(0, "usmBuildSecurityParameters error coding msgusername");
    return NULL;
  }

  *position = bufPtr - buf + 2;

  debugprintf(5, "Coding octstr sp.msgAu..Para.. , length = 0x%lx",
	      sp.msgAuthenticationParametersLength);
  bufPtr = asn_build_string(bufPtr, &length,
                            (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STR),
                            sp.msgAuthenticationParameters,
                            sp.msgAuthenticationParametersLength);

  if (bufPtr == NULL) {
    debugprintf(0, "usmBuildSecurityParameters error coding authparams");
    return NULL;
  }

  debugprintf(5, "Coding octstr sp.msgPr..Para.. , length = 0x%lx",
	      sp.msgPrivacyParametersLength);
  bufPtr = asn_build_string(bufPtr, &length,
                            (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STR),
                            sp.msgPrivacyParameters,
                            sp.msgPrivacyParametersLength);

  if (bufPtr == NULL) {
    debugprintf(0, "usmBuildSecurityParameters error coding privparams");
    return NULL;
  }

  totalLength = bufPtr - (unsigned char *)&buf;

  debugprintf(5, "Coding sequence (securityPar), length = 0x%x", totalLength);
  outBufPtr = asn_build_sequence(outBufPtr, maxLength,
                                 (unsigned char)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
                                 totalLength);

  if (outBufPtr == NULL) {
    debugprintf(0, "usm: usmBuildSecurityParameters error coding secparams");
    return NULL;
  }

  if (*maxLength < totalLength) {
    debugprintf(0, "usm: usmBuildSecurityParameters error (length mismatch)");
    return NULL;
  }
  *position += outBufPtr - outBuf;
  memcpy(outBufPtr, (unsigned char*)&buf, totalLength);
  outBufPtr += totalLength;
  *maxLength -= totalLength;

  debugprintf(21, "bufSecurityData:");
  debughexprintf(21, outBuf, outBufPtr - outBuf);

  return outBufPtr;
}

unsigned char *USM::build_whole_msg(
                         unsigned char *outBuf, int *maxLength,
			 unsigned char *globalData, long int globalDataLength,
			 int *positionAuthPar,
			 struct UsmSecurityParameters  securityParameters,
			 unsigned char *msgData, long int msgDataLength)
{
  unsigned char buf[MAXLENGTH_BUFFER];
  unsigned char *bufPtr = (unsigned char*)&buf;
  unsigned char secPar[MAXLENGTH_BUFFER];
  unsigned char *secParPtr = (unsigned char*)&secPar;
  unsigned char *outBufPtr = outBuf;
  long int secParLength;
  int length = *maxLength;
  int totalLength;

  int dummy = *maxLength;

  secParPtr = build_sec_params(secParPtr, &dummy, securityParameters,
			       positionAuthPar);

  if (secParPtr == NULL)
    return NULL;
  secParLength = secParPtr - (unsigned char*)&secPar;

  long int dummyVersion = 3;
  debugprintf(3, "Coding int snmpVersion = 0x%lx",dummyVersion);
  bufPtr = asn_build_int(bufPtr, &length,
                         (unsigned char )(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
                         (long*)&dummyVersion,
                         sizeof(dummyVersion));
  if (bufPtr == NULL) {
    debugprintf(0, "usmBuildWholeMsg error");
    return NULL;
  }

  // globalData is encoded as sequence
  length -= globalDataLength;
  if (length < 0) {
    debugprintf(0, "usmBuildWholeMsg error");
    return NULL;
  }
  memcpy(bufPtr, globalData, globalDataLength);
  bufPtr += globalDataLength;

  *positionAuthPar += bufPtr - buf +2;
  if (secParLength> 0x7f)
    *positionAuthPar += 2;

  debugprintf(3, "Coding octstr securityParameter, length = 0x%lx",
              secParLength);
  bufPtr = asn_build_string(bufPtr, &length,
                            (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STR),
                            secPar, secParLength);

  if (bufPtr == NULL) {
    debugprintf(0, "usmBuildWholeMsg error2");
    return NULL;
  }

  // msgData (ScopedPduData) is encoded
  length -=msgDataLength;
  if (length < 0) {
    debugprintf(10, "usmBuildWholeMsg error: msgDataLength = %i",
                msgDataLength);
    debugprintf(10, "maxLength = %i, encoded = %i", *maxLength,
                bufPtr - (unsigned char *)&buf);
    return NULL;
  }
  memcpy(bufPtr, msgData, msgDataLength);
  bufPtr += msgDataLength;

  if (bufPtr == NULL) {
    debugprintf(0, "usmBuildWholeMsg error");
    return NULL;
  }

  totalLength = bufPtr - (unsigned char *)&buf;

  debugprintf(3, "Coding sequence (wholeMsg), length = 0x%x", totalLength);

  outBufPtr = asn_build_sequence(outBufPtr, maxLength,
                                 (unsigned char)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
                                 totalLength);

  if (outBufPtr == NULL) {
    debugprintf(0, "usm: usmBuildWholeMsg error");
    return NULL;
  }

  if (*maxLength < totalLength) {
    debugprintf(0, "usm: usmBuildWholeMsg error");
    return NULL;
  }
  *positionAuthPar += outBufPtr - outBuf;
  memcpy(outBufPtr, (unsigned char*)&buf, totalLength);
  outBufPtr += totalLength;
  *maxLength -= totalLength;

  debugprintf(21,"bufWholeMsg:");
  debughexprintf(21, outBuf, outBufPtr - outBuf);

  return outBufPtr;
}

inline void USM::delete_user_ptr(struct UsmUser *user)
{
  if (!user) return;
  if (user->engineID) {
    delete [] user->engineID;
    user->engineID = NULL;
  }
  if (user->usmUserName) {
    delete [] user->usmUserName;
    user->usmUserName = NULL;
  }
  if (user->securityName) {
    delete [] user->securityName;
    user->securityName = NULL;
  }
  if (user->authKey) {
    memset(user->authKey, 0, user->authKeyLength);
    delete [] user->authKey;
    user->authKey = NULL;
  }
  if (user->privKey) {
    memset(user->privKey, 0, user->privKeyLength);
    delete [] user->privKey;
    user->authKey = NULL;
  }
}

// Safe all localized users into a file.
int USM::save_localized_users(const char *file)
{
  return usm_user_table->save_to_file(file, auth_priv);
}

// Load localized users from a file.
int USM::load_localized_users(const char *file)
{
  return usm_user_table->load_from_file(file, auth_priv);
}

// Safe all users with their passwords into a file.
int USM::save_users(const char *file)
{
  return usm_user_name_table->save_to_file(file, auth_priv);
}

// Load users with their passwords from a file.
int USM::load_users(const char *file)
{
  return usm_user_name_table->load_from_file(file, auth_priv);
}

// Get a const pointer to the first entry of the UsmUserNameTable.
const UsmUserNameTableEntry *USM::peek_first_user()
{
  return usm_user_name_table->peek_first();
}

// Get a const pointer to the next entry of the UsmUserNameTable.
const UsmUserNameTableEntry *USM::peek_next_user(const UsmUserNameTableEntry *e)
{
  return usm_user_name_table->peek_next(e);
}


// Get a const pointer to the first entry of the UsmUserTable.
const UsmUserTableEntry *USM::peek_first_luser()
{
  return usm_user_table->peek_first();
}

// Get a const pointer to the next entry of the UsmUserTable.
const UsmUserTableEntry *USM::peek_next_luser(const UsmUserTableEntry *e)
{
  return usm_user_table->peek_next(e);
}


/* ----------------------- class USMTimeTable --------------------*/

USMTimeTable::USMTimeTable(const USM *owner,
			   const unsigned int engine_boots, int &result)
{
  time_t now;

  table = new struct Entry_T[5];

  if (!table)
  {
    debugprintf(0, "USMTimeTable: error constructing table.");
    result = SNMPv3_USM_ERROR;
    return;
  }

  usm = owner;

  /* the first entry always contains the local engine id and time */
  time(&now);

  table[0].time_diff     = -now;
  table[0].engine_boots  = engine_boots;
  table[0].engine_id_len = min(usm->get_local_engine_id().len(),
			       MAXLENGTH_ENGINEID);
  memcpy(table[0].engine_id, usm->get_local_engine_id().data(),
	 table[0].engine_id_len);

  entries = 1;
  max_entries = 5;

  result = SNMPv3_USM_OK;
}

USMTimeTable::~USMTimeTable()
{
  if (table)
  {
    delete [] table;
    table = NULL;
  }
  entries = 0;
  max_entries = 0;
}

int USMTimeTable::add_entry(const OctetStr &engine_id,
                            const long int engine_boots,
                            const long int engine_time)
REENTRANT({
  if (!table)
    return SNMPv3_USM_ERROR;

  debugprintf(10, "USMTimeTable::add_entry: e_id (%s), boot (%i), time(%i)",
              engine_id.get_printable(), engine_boots, engine_time);

  if (entries == max_entries)
  {
    /* resize Table */
    struct Entry_T *tmp = new struct Entry_T[4 * max_entries];

    if (!tmp)
      return SNMPv3_USM_ERROR;

    memcpy(tmp, table, entries * sizeof(Entry_T));

    struct Entry_T *victim = table;
    table = tmp;
    delete [] victim;

    max_entries *= 4;
  }

  time_t now;
  time(&now);

  table[entries].engine_boots = engine_boots;
  table[entries].latest_received_time = engine_time;
  table[entries].time_diff = engine_time - now;
  table[entries].engine_id_len = engine_id.len();
  table[entries].engine_id_len = min(table[entries].engine_id_len,
                                     MAXLENGTH_ENGINEID);
  memcpy(table[entries].engine_id,
	 engine_id.data(), table[entries].engine_id_len);

  entries++;

  return SNMPv3_USM_OK;
})

unsigned long USMTimeTable::get_local_time()
{
  if (!table)
    return 0;

  time_t now;
  time(&now);

  return table[0].time_diff + now;
}

int USMTimeTable::get_local_time(long int &engine_boots,
                                 long int &engine_time)
{
  if (!table)
    return SNMPv3_USM_ERROR;

  time_t now;
  time(&now);

  engine_boots = table[0].engine_boots;
  engine_time  = table[0].time_diff + now;

  debugprintf(10, "get_local_time: returning boots (%i), time (%i)",
              engine_boots, engine_time);

  return SNMPv3_USM_OK;
}

int USMTimeTable::get_time(const OctetStr &engine_id,
                           long int &engine_boots, long int &engine_time)
REENTRANT({
  debugprintf(10, "USMTimeTable::get_ime: Get time for engine_id (%s)",
              engine_id.get_printable());

  if (!table)
    return SNMPv3_USM_ERROR;

  for (int i=0; i < entries; i++)
    if (unsignedCharCompare(table[i].engine_id, table[i].engine_id_len,
                            engine_id.data(), engine_id.len()))
    {
      /* Entry found */
      time_t now;
      time(&now);

      engine_boots = table[i].engine_boots;
      engine_time  = table[i].time_diff + now;

      debugprintf(10, "get_time: found entry with boots (%i), time (%i)",
                  engine_boots, engine_time);

      return SNMPv3_USM_OK;
    }

  /* no entry */
  engine_boots = 0;
  engine_time = 0;

  debugprintf(10, "get_time: no entry found");

  return SNMPv3_USM_UNKNOWN_ENGINEID;
})

int USMTimeTable::check_time(const OctetStr &engine_id,
                             const long int engine_boots,
                             const long int engine_time)

REENTRANT({
  debugprintf(9, "USMTimeTable::check_time: TimeWindowCheck for"
              " engine_id (%s), boot (%i), time(%i)",
              engine_id.get_printable(), engine_boots, engine_time);

  if (!table)
    return SNMPv3_USM_ERROR;

  time_t now;
  time(&now);

  /* table[0] contains the local engine_id and time */
  if (unsignedCharCompare(table[0].engine_id, table[0].engine_id_len,
                          engine_id.data(), engine_id.len()))
  {
    /* Entry found, we are authoritative */
    if ((table[0].engine_boots == 2147483647) ||
        (table[0].engine_boots != engine_boots) ||
        (labs(now + table[0].time_diff - engine_time) > 150))
    {
      debugprintf(1, "check_time: received Message outside "
                  "timewindow (authoritative).");
      return SNMPv3_USM_NOT_IN_TIME_WINDOW;
    }
    else
    {
      debugprintf(9, "check_time: time ok (authoritative)");
      return SNMPv3_USM_OK;
    }
  }

  for (int i=1; i < entries; i++)
    if (unsignedCharCompare(table[i].engine_id, table[i].engine_id_len,
                            engine_id.data(), engine_id.len()))
    {
      /* Entry found we are not authoritative */
      if ((engine_boots < table[i].engine_boots) ||
          ((engine_boots == table[i].engine_boots) &&
           (table[i].time_diff + now > engine_time + 150)) ||
          (table[i].engine_boots == 2147483647))
      {
        debugprintf(1, "check_time: received Message outside "
                    "timewindow (not authoritative).");
        return SNMPv3_USM_NOT_IN_TIME_WINDOW;
      }
      else
      {
        if ((engine_boots > table[i].engine_boots) ||
            ((engine_boots == table[i].engine_boots) &&
             (engine_time > table[i].latest_received_time)))
        {
          /* time ok, update values */
          table[i].engine_boots = engine_boots;
          table[i].latest_received_time  = engine_time;
          table[i].time_diff = engine_time - now;
        }
        debugprintf(9, "check_time: time ok");
        return SNMPv3_USM_OK;
      }
    }
  debugprintf(3, "check_ime: engine_id not found.");
  return SNMPv3_USM_UNKNOWN_ENGINEID;
})


int USMTimeTable::check_engine_id(const OctetStr &engine_id)
REENTRANT({
  if (!table)
    return SNMPv3_USM_ERROR;

  for (int i=0; i < entries; i++)
    if (unsignedCharCompare(table[i].engine_id, table[i].engine_id_len,
                            engine_id.data(), engine_id.len()))
      return SNMPv3_USM_OK;

  /* if in discovery mode:  accept all EngineID's (rfc2264 page 26) */

  if (usm->is_discovery_enabled())
    return add_entry(engine_id, 0, 0);

  return SNMPv3_USM_ERROR;
})



/* ------------------------- USMUserNameTable ----------------------*/

USMUserNameTable::USMUserNameTable(int &result)
{
  /* init Table */
  table = new struct UsmUserNameTableEntry[10];
  if (!table)
  {
    result = SNMPv3_USM_ERROR;
    return;
  }
  max_entries = 10;
  entries = 0;
  result = SNMPv3_USM_OK;
}

USMUserNameTable::~USMUserNameTable()
{
  if (table)
  {
    for (int i=0; i < entries; i++)
    {
      if (table[i].authPassword)
      {
        memset(table[i].authPassword, 0, table[i].authPasswordLength);
        delete [] table[i].authPassword;
      }

      if (table[i].privPassword)
      {
        memset(table[i].privPassword, 0, table[i].privPasswordLength);
        delete [] table[i].privPassword;
      }
    }
    delete [] table;
    table = NULL;
  }
  entries = 0;
  max_entries = 0;
}

int USMUserNameTable::add_entry(const OctetStr& user_name,
                                const OctetStr& security_name,
                                const long int  auth_proto,
                                const long int  priv_proto,
                                const OctetStr& auth_pass,
                                const OctetStr& priv_pass)
REENTRANT({
  if (!table)
    return SNMPv3_USM_ERROR;

  int found=0;
  int i;
  for (i = 0; i < entries; i++)
    if (table[i].usmUserName == user_name)
    {
      found=1;
      break;
    }

  if (found)
  {
    /* replace user */
    table[i].usmUserSecurityName = security_name;
    table[i].usmUserAuthProtocol = auth_proto;
    table[i].usmUserPrivProtocol = priv_proto;

    if (table[i].authPassword)
    {
      memset(table[i].authPassword, 0, table[i].authPasswordLength);
      delete [] table[i].authPassword;
    }
    table[i].authPassword = v3strcpy(auth_pass.data(), auth_pass.len());
    table[i].authPasswordLength = auth_pass.len();

    if (table[i].privPassword)
    {
      memset(table[i].privPassword, 0, table[i].privPasswordLength);
      delete [] table[i].privPassword;
    }
    table[i].privPassword = v3strcpy(priv_pass.data(), priv_pass.len());
    table[i].privPasswordLength = priv_pass.len();
  }
  else
  {
    if (entries == max_entries)
    {
      /* resize Table */
      struct UsmUserNameTableEntry *tmp;
      tmp = new struct UsmUserNameTableEntry[4 * max_entries];
      if (!tmp)
        return SNMPv3_USM_ERROR;
      for (i=0; i < entries; i++)
        tmp[i] = table[i];

      struct UsmUserNameTableEntry *victim = table;
      table = tmp;
      delete [] victim;

      max_entries *= 4;
    }

    table[entries].usmUserName          = user_name;
    table[entries].usmUserSecurityName  = security_name;
    table[entries].usmUserAuthProtocol  = auth_proto;
    table[entries].usmUserPrivProtocol  = priv_proto;

    table[entries].authPasswordLength = auth_pass.len();
    table[entries].authPassword = v3strcpy(auth_pass.data(), auth_pass.len());
    if (!table[entries].authPassword)
      return SNMPv3_USM_ERROR;

    table[entries].privPasswordLength = priv_pass.len();
    table[entries].privPassword = v3strcpy(priv_pass.data(), priv_pass.len());
    if (!table[entries].privPassword)
      return SNMPv3_USM_ERROR;

    entries++;
  }

  return SNMPv3_USM_OK;
})

int USMUserNameTable::delete_security_name(const OctetStr& security_name)
REENTRANT({
  if (!table)
    return SNMPv3_USM_ERROR;

  for (int i = 0; i < entries; i++)
    if (table[i].usmUserSecurityName == security_name)
    {
      memset(table[i].authPassword, 0, table[i].authPasswordLength);
      delete [] table[i].authPassword;
      memset(table[i].privPassword, 0, table[i].privPasswordLength);
      delete [] table[i].privPassword;
      entries--;
      if (entries > i)
        table[i] = table[entries];
      break;
    }
  return SNMPv3_USM_OK;
})

struct UsmUserNameTableEntry* USMUserNameTable::get_entry(
                                          const OctetStr &security_name)
REENTRANT({
  if (!table)
    return NULL;

  for (int i = 0; i < entries; i++)
    if (table[i].usmUserSecurityName == security_name)
    {
      struct UsmUserNameTableEntry *res = new UsmUserNameTableEntry;
      if (!res) return NULL;
      *res = table[i];
      return res;
    }
  return NULL;
})

int USMUserNameTable::get_security_name(const unsigned char *user_name,
					const long int user_name_len,
					OctetStr &security_name)
REENTRANT({
  debugprintf(20,"set_security_name: get user (%s)",
              OctetStr(user_name,user_name_len).get_printable());

  if (!table)
    return SNMPv3_USM_ERROR;

  for (int i = 0; i < entries; i++)
    if (unsignedCharCompare(table[i].usmUserName.data(),
			    table[i].usmUserName.len(),
                            user_name, user_name_len))
    {
      security_name = table[i].usmUserSecurityName;
      return SNMPv3_USM_OK;
    }

  debugprintf(0, "get_security_name: User not in table.");
  return SNMPv3_USM_ERROR;
})

int USMUserNameTable::get_user_name(unsigned char *user_name,
                                    long int *user_name_len,
                                    const unsigned char *security_name,
                                    const long int security_name_len)
REENTRANT({
  unsigned long buf_len = *user_name_len;
  *user_name_len = 0;

  if (!table)
    return SNMPv3_USM_ERROR;

  for (int i = 0; i < entries; i++)
    if (unsignedCharCompare(table[i].usmUserSecurityName.data(),
                            table[i].usmUserSecurityName.len(),
                            security_name, security_name_len))
    {
      if (buf_len < table[i].usmUserName.len())
      {
        debugprintf(0, "Buffer for usmUserName too small!");
        return SNMPv3_USM_ERROR;
      }
      *user_name_len = table[i].usmUserName.len();
      memcpy(user_name, table[i].usmUserName.data(),
	     table[i].usmUserName.len());

      return SNMPv3_USM_OK;
    }

  debugprintf(0, "get_user_name: User not in table");
  return SNMPv3_USM_ERROR;
})

// Save all entries into a file.
int USMUserNameTable::save_to_file(const char *name, AuthPriv *ap)
{
  char encoded[MAX_LINE_LEN * 2];
  FILE *file_out;
  char tmp_file_name[MAXLENGTH_FILENAME];

  if (!name)
  {
    debugprintf(0, "USMUserNameTable::save_to_file: filename is NULL.");
    return SNMPv3_USM_ERROR;
  }

  if (!ap)
  {
    debugprintf(0, "USMUserNameTable::save_to_file: AuthPriv ptr is NULL.");
    return SNMPv3_USM_ERROR;
  }

  sprintf(tmp_file_name, "%s.tmp", name);
  file_out = fopen(tmp_file_name, "w");
  if (!file_out)
  {
    debugprintf(0, "USMUserNameTable::save_to_file: could not open tmpfile"
		" (%s)", tmp_file_name);
    return SNMPv3_USM_FILEOPEN_ERROR;
  }

  bool failed = false;
  for (int i=0; i < entries; ++i)
  {
    debugprintf(4, "Saving user %s to file",
		table[i].usmUserName.get_printable());

    encodeString(table[i].usmUserName.data(), table[i].usmUserName.len(),
		 encoded);
    encoded[2 * table[i].usmUserName.len()] = '\n';
    if (fwrite(encoded, 2 * table[i].usmUserName.len() + 1, 1, file_out) != 1)
    { failed = true; break; }

    encodeString(table[i].usmUserSecurityName.data(),
		 table[i].usmUserSecurityName.len(), encoded);
    encoded[2 * table[i].usmUserSecurityName.len()] = '\n';
    if (fwrite(encoded, 2 * table[i].usmUserSecurityName.len() + 1, 1,
	       file_out) != 1)
    { failed = true; break; }

    encodeString(table[i].authPassword, table[i].authPasswordLength, encoded);
    encoded[2 * table[i].authPasswordLength] = '\n';
    if (fwrite(encoded, 2 * table[i].authPasswordLength + 1, 1, file_out) != 1)
    { failed = true; break; }

    encodeString(table[i].privPassword, table[i].privPasswordLength, encoded);
    encoded[2 * table[i].privPasswordLength] = '\n';
    if (fwrite(encoded, 2 * table[i].privPasswordLength + 1, 1, file_out) != 1)
    { failed = true; break; }
    
    if (table[i].usmUserAuthProtocol == SNMP_AUTHPROTOCOL_NONE)
    {
      if (fwrite("none\n", 5, 1, file_out) != 1)
      { failed = true; break; }
    }
    else
    {
      const Auth *a = ap->get_auth(table[i].usmUserAuthProtocol);
      if (!a) { failed = true; break; }
      sprintf(encoded, "%s\n", a->get_id_string());
      if (fwrite(encoded, strlen(a->get_id_string()) + 1, 1, file_out) != 1)
      { failed = true; break; }
    }

    if (table[i].usmUserPrivProtocol == SNMP_PRIVPROTOCOL_NONE)
    {
      if (fwrite("none\n", 5, 1, file_out) != 1)
      { failed = true; break; }
    }
    else
    {
      const Priv *p = ap->get_priv(table[i].usmUserPrivProtocol);
      if (!p) { failed = true; break; }
      sprintf(encoded, "%s\n", p->get_id_string());
      if (fwrite(encoded, strlen(p->get_id_string()) + 1, 1, file_out) != 1)
      { failed = true; break; }
    }
  }
  fclose(file_out);
  if (failed)
  {
    debugprintf(0, "Failed to write table entries.");
#ifdef WIN32
    _unlink(tmp_file_name);
#else
    unlink(tmp_file_name);
#endif
    return SNMPv3_USM_FILEWRITE_ERROR;
  }
#ifdef WIN32
  _unlink(name);
#else
  unlink(name);
#endif
  if (rename(tmp_file_name, name))
  {
    debugprintf(0, "save_to_file: could not rename tmpfile (%s)"
                  " to file (%s)", tmp_file_name, name);
    return SNMPv3_USM_FILERENAME_ERROR;
  }
  debugprintf(5, "save_to_file: saved all users");
  return SNMPv3_USM_OK;
}

// Load the table from a file.
int USMUserNameTable::load_from_file(const char *name, AuthPriv *ap)
{
  char decoded[MAX_LINE_LEN];
  FILE *file_in;
  unsigned char line[MAX_LINE_LEN * 2];

  if (!name)
  {
    debugprintf(0, "USMUserNameTable::load_from_file: filename is NULL.");
    return SNMPv3_USM_ERROR;
  }

  if (!ap)
  {
    debugprintf(0, "USMUserNameTable::load_from_file: AuthPriv ptr is NULL.");
    return SNMPv3_USM_ERROR;
  }

  file_in = fopen(name, "r");
  if (!file_in)
  {
    debugprintf(0, "load_from_file: Could not open (file): (%s)", name);
    return SNMPv3_USM_FILEOPEN_ERROR;
  }

  int len;
  bool failed = false;
  while (fgets((char*)line, MAX_LINE_LEN * 2, file_in))
  {
    // user_name
    len = strlen((char*)line) - 1;
    decodeString(line, len, decoded);
    OctetStr user_name((unsigned char*)decoded, len / 2);

    // security_name
    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    len = strlen((char*)line) - 1;
    decodeString(line, len, decoded);
    OctetStr user_security_name((unsigned char*)decoded, len / 2);

    // auth password
    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    len = strlen((char*)line) - 1;
    decodeString(line, len, decoded);
    OctetStr auth_pass((unsigned char*)decoded, len / 2);

    // priv password
    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    len = strlen((char*)line) - 1;
    decodeString(line, len, decoded);
    OctetStr priv_pass((unsigned char*)decoded, len / 2);

    // auth protocol
    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    line[strlen((char*)line) - 1] = 0;
    int auth_prot = SNMP_AUTHPROTOCOL_NONE;
    if (strcmp((char*)line, "none") != 0)
    {
      auth_prot = ap->get_auth_id((char*)line);
      if (auth_prot < 0)
      { failed = true; break; }
    }

    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    line[strlen((char*)line) - 1] = 0;
    int priv_prot = SNMP_PRIVPROTOCOL_NONE;
    if (strcmp((char*)line, "none") != 0)
    {
      priv_prot = ap->get_priv_id((char*)line);
      if (priv_prot < 0)
      { failed = true; break; }
    }
    debugprintf(2, "Adding user: %s, %s, ap %i, pp %i",
		user_name.get_printable(),
		user_security_name.get_printable(), auth_prot, priv_prot);
    if (add_entry(user_name, user_security_name, auth_prot, priv_prot,
		  auth_pass, priv_pass) == SNMPv3_USM_ERROR)
    {
      failed = true;
      debugprintf(0, "Error adding user %s", user_name.get_printable());
    }
  }

  fclose(file_in);
  if (failed)
  {
    debugprintf(0, "Failed to read table entries.");
    return SNMPv3_USM_FILEREAD_ERROR;
  }

  debugprintf(5, "load_from_file: loaded all users");
  return SNMPv3_USM_OK;
}

const UsmUserNameTableEntry *USMUserNameTable::peek_next(
                       const UsmUserNameTableEntry *e) const
{
  if (e == 0) return 0;
  if (e - table < 0) return 0;
  if (e - table >= entries - 1) return 0;
  return (e + 1);
}

/* ---------------------------- USMUserTable ------------------- */

USMUserTable::USMUserTable(int &result)
{
  entries = 0;

  table = new struct UsmUserTableEntry[10];
  if (!table)
  {
    result = SNMPv3_USM_ERROR;
    return;
  }
  max_entries = 10;
}

USMUserTable::~USMUserTable()
{
  if (table)
  {
    for (int i = 0; i < entries; i++)
    {
      if (table[i].usmUserEngineID)
	delete [] table[i].usmUserEngineID;
      if (table[i].usmUserName)
	delete [] table[i].usmUserName;
      if (table[i].usmUserSecurityName)
	delete [] table[i].usmUserSecurityName;
      if (table[i].usmUserAuthKey)
      {
	memset(table[i].usmUserAuthKey, 0, table[i].usmUserAuthKeyLength);
	delete [] table[i].usmUserAuthKey;
      }
      if (table[i].usmUserPrivKey)
      {
	memset(table[i].usmUserPrivKey, 0, table[i].usmUserPrivKeyLength);
	delete [] table[i].usmUserPrivKey;
      }
    }
    delete [] table;
    table = NULL;
    max_entries = 0;
    entries = 0;
  }
}

int USMUserTable::size()
{
  return entries;
}

int USMUserTable::get_user_name(unsigned char       *user_name,
				long int            *user_name_len,
				const unsigned char *sec_name,
				const long           sec_name_len)

REENTRANT({
  long buf_len = *user_name_len;
  *user_name_len = 0;

  if (!table)
    return SNMPv3_USM_ERROR;

  for (int i=0; i < entries; i++)
    if (unsignedCharCompare(table[i].usmUserSecurityName,
			    table[i].usmUserSecurityNameLength,
			    sec_name, sec_name_len))
    {
      if (buf_len < table[i].usmUserNameLength)
      {
	debugprintf(0, "Buffer for user_name too small!");
	return SNMPv3_USM_ERROR;
      }
      *user_name_len = table[i].usmUserNameLength;
      memcpy(user_name, table[i].usmUserName, table[i].usmUserNameLength);

      return SNMPv3_USM_OK;
    }

  debugprintf(0, "get_user_name: User not in table.");
  return SNMPv3_USM_ERROR;
})

int USMUserTable::get_security_name(const unsigned char *user_name,
				    const long user_name_len,
				    OctetStr &sec_name)
REENTRANT({
  if (!table)
    return SNMPv3_USM_ERROR;

  for (int i=0; i < entries; i++)
    if (unsignedCharCompare(table[i].usmUserName, table[i].usmUserNameLength,
			    user_name, user_name_len))
    {
      sec_name.set_data(table[i].usmUserSecurityName,
			table[i].usmUserSecurityNameLength);
      return SNMPv3_USM_OK;
    }

  debugprintf(0, "get_security_name: User not in table");
  return SNMPv3_USM_ERROR;
})

int USMUserTable::delete_entries(const OctetStr& user_name)
REENTRANT({
  if (!table)
    return SNMPv3_USM_ERROR;

  for (int i = 0; i < entries; i++)
    if (unsignedCharCompare(table[i].usmUserName, table[i].usmUserNameLength,
			    user_name.data(), user_name.len()))
    {
      /* delete this entry and recheck this position */
      delete_entry(i);
      i--;
    }
  return SNMPv3_USM_OK;
})

int USMUserTable::delete_entry(const OctetStr& engine_id,
			       const OctetStr& user_name)
REENTRANT({
  if (!table)
    return SNMPv3_USM_ERROR;

  for (int i = 0; i < entries; i++)
    if (unsignedCharCompare(table[i].usmUserName, table[i].usmUserNameLength,
			    user_name.data(), user_name.len()))
      if (unsignedCharCompare(table[i].usmUserEngineID,
			      table[i].usmUserEngineIDLength,
			      engine_id.data(), engine_id.len()))
      {
	/* delete this entry and recheck this position*/
	delete_entry(i);
	i--;
      }
  return SNMPv3_USM_OK;
})

struct UsmUserTableEntry *USMUserTable::get_entry(const int number)
REENTRANT({
  if ((entries < number) || (number < 1))
    return NULL;

  struct UsmUserTableEntry *t = &table[number - 1];
  struct UsmUserTableEntry *res = new UsmUserTableEntry;

  if (!res)
    return NULL;

  res->usmUserEngineID       = v3strcpy(t->usmUserEngineID,
					t->usmUserEngineIDLength);
  res->usmUserEngineIDLength = t->usmUserEngineIDLength;
  res->usmUserName           = v3strcpy(t->usmUserName,
					t->usmUserNameLength);
  res->usmUserNameLength     = t->usmUserNameLength;
  res->usmUserSecurityName   = v3strcpy(t->usmUserSecurityName,
					t->usmUserSecurityNameLength);
  res->usmUserSecurityNameLength = t->usmUserSecurityNameLength;
  res->usmUserAuthProtocol   = t->usmUserAuthProtocol;
  res->usmUserAuthKey        = t->usmUserAuthKey;
  res->usmUserAuthKeyLength  = t->usmUserAuthKeyLength;
  res->usmUserPrivProtocol   = t->usmUserPrivProtocol;
  res->usmUserPrivKey        = t->usmUserPrivKey;
  res->usmUserPrivKeyLength  = t->usmUserPrivKeyLength;
  return res;
})

struct UsmUserTableEntry *USMUserTable::get_entry(const OctetStr &engine_id,
						  const OctetStr &sec_name)
REENTRANT({
  if (!table)
    return NULL;

  for (int i = 0; i < entries; i++)
    if (unsignedCharCompare(table[i].usmUserSecurityName,
			    table[i].usmUserSecurityNameLength,
			    sec_name.data(), sec_name.len()))
      if (unsignedCharCompare(table[i].usmUserEngineID,
			      table[i].usmUserEngineIDLength,
			      engine_id.data(), engine_id.len()))
	return &table[i];
  return NULL;
})

struct UsmUserTableEntry *USMUserTable::get_entry(const OctetStr &sec_name)
REENTRANT({
  if (!table)
    return NULL;

  for (int i = 0; i < entries; i++)
    if (unsignedCharCompare(table[i].usmUserSecurityName,
			    table[i].usmUserSecurityNameLength,
			    sec_name.data(), sec_name.len()))
      return &table[i];
  return NULL;
})

int USMUserTable::add_entry(
                      const OctetStr &engine_id,
		      const OctetStr &user_name,  const OctetStr &sec_name,
		      const long int  auth_proto, const OctetStr &auth_key,
		      const long int  priv_proto, const OctetStr &priv_key)
REENTRANT({
  debugprintf(7,"usmAddUser: Adding user (%s) engine_id (%s).",
	      user_name.get_printable(), engine_id.get_printable());

  if (!table)
    return SNMPv3_USM_ERROR;

  if (entries == max_entries)
  {
    /* resize Table */
    struct UsmUserTableEntry *tmp;
    tmp = new struct UsmUserTableEntry[4 * max_entries];
    if (!tmp) return SNMPv3_USM_ERROR;
    for (int i = 0; i < entries; i++)
      tmp[i] = table[i];
    delete [] table;
    table = tmp;
    max_entries *= 4;
  }

  for (int i = 0; i < entries; i++)
    if (unsignedCharCompare(table[i].usmUserName, table[i].usmUserNameLength,
			    user_name.data(), user_name.len()))
      if (unsignedCharCompare(table[i].usmUserEngineID,
			      table[i].usmUserEngineIDLength,
			      engine_id.data(), engine_id.len()))
      {
	/* delete this entry */
	delete_entry(i);
	break;
      }

  /* add user at the last position */
  table[entries].usmUserEngineIDLength = engine_id.len();
  table[entries].usmUserEngineID       = v3strcpy(engine_id.data(),
						  engine_id.len());
  table[entries].usmUserNameLength     = user_name.len();
  table[entries].usmUserName           = v3strcpy(user_name.data(),
						  user_name.len());
  table[entries].usmUserSecurityNameLength = sec_name.len();
  table[entries].usmUserSecurityName   = v3strcpy(sec_name.data(),
						  sec_name.len());
  table[entries].usmUserAuthProtocol   = auth_proto;
  table[entries].usmUserAuthKeyLength  = auth_key.len();
  table[entries].usmUserAuthKey        = v3strcpy(auth_key.data(),
						  auth_key.len());
  table[entries].usmUserPrivProtocol   = priv_proto;
  table[entries].usmUserPrivKeyLength  = priv_key.len();
  table[entries].usmUserPrivKey        = v3strcpy(priv_key.data(),
						  priv_key.len());
  entries++;
  return SNMPv3_USM_OK;
})

int USMUserTable::update_key(const OctetStr &user_name,
			     const OctetStr &engine_id,
			     const OctetStr &new_key,
			     const int key_type)
REENTRANT({
  debugprintf(5,"usmUpdateKey: for user: (%s)@(%s) type (%i)",
	      user_name.get_printable(), engine_id.get_printable(), key_type);

  if (!table)
    return SNMPv3_USM_ERROR;

  for (int i = 0; i < entries; i++)
    if (unsignedCharCompare(table[i].usmUserName, table[i].usmUserNameLength,
			    user_name.data(), user_name.len()))
      if (unsignedCharCompare(table[i].usmUserEngineID,
			      table[i].usmUserEngineIDLength,
			      engine_id.data(), engine_id.len()))
      {
	debugprintf(21, "Set new key to: (will fail if type is wrong!)");
	debughexprintf(21, new_key.data(), new_key.len());

	/* update key: */
	switch (key_type)
	{
	  case AUTHKEY:
	  case OWNAUTHKEY:
	  {
	    debugprintf(2, "updating authkey.");
	    if (table[i].usmUserAuthKey)
	    {
	      memset(table[i].usmUserAuthKey, 0,
		     table[i].usmUserAuthKeyLength);
	      delete [] table[i].usmUserAuthKey;
	    }
	    table[i].usmUserAuthKeyLength = new_key.len();
	    table[i].usmUserAuthKey = v3strcpy(new_key.data(), new_key.len());
	    return SNMPv3_USM_OK;
	  }
	  case PRIVKEY:
	  case OWNPRIVKEY:
	  {
	    debugprintf(2, "updating privkey.");
	    if (table[i].usmUserPrivKey)
	    {
	      memset(table[i].usmUserPrivKey, 0,
		     table[i].usmUserPrivKeyLength);
	      delete [] table[i].usmUserPrivKey;
	    }
	    table[i].usmUserPrivKeyLength = new_key.len();
	    table[i].usmUserPrivKey = v3strcpy(new_key.data(), new_key.len());
	    return SNMPv3_USM_OK;
	  }
	  default:
	  {
	    debugprintf(0, "setting new key failed (wrong type).");
	    return SNMPv3_USM_ERROR;
	  }
	}
      }
  debugprintf(0, "setting new key failed (user not found).");
  return SNMPv3_USM_ERROR;
})

void USMUserTable::delete_entry(const int nr)
REENTRANT({
  if (!table)
    return;

  if (nr >= entries)
    return;

  if (table[nr].usmUserEngineID)
    delete [] table[nr].usmUserEngineID;
  if (table[nr].usmUserName)
    delete [] table[nr].usmUserName;
  if (table[nr].usmUserSecurityName)
    delete [] table[nr].usmUserSecurityName;
  if (table[nr].usmUserAuthKey)
  {
    memset(table[nr].usmUserAuthKey, 0, table[nr].usmUserAuthKeyLength);
    delete [] table[nr].usmUserAuthKey;
  }
  if (table[nr].usmUserPrivKey)
  {
    memset(table[nr].usmUserPrivKey, 0, table[nr].usmUserPrivKeyLength);
    delete [] table[nr].usmUserPrivKey;
  }

  /* We have now one entry less */
  entries--;

  if (entries > nr)
  {
    /* move the last entry to the deleted position */
    table[nr] = table[entries];
  }
})

// Save all entries into a file.
int USMUserTable::save_to_file(const char *name, AuthPriv *ap)
{
  char encoded[MAX_LINE_LEN * 2];
  FILE *file_out;
  char tmp_file_name[MAXLENGTH_FILENAME];

  if (!name)
  {
    debugprintf(0, "USMUserTable::save_to_file: filename is NULL.");
    return SNMPv3_USM_ERROR;
  }

  if (!ap)
  {
    debugprintf(0, "USMUserTable::save_to_file: AuthPriv ptr is NULL.");
    return SNMPv3_USM_ERROR;
  }

  sprintf(tmp_file_name, "%s.tmp", name);
  file_out = fopen(tmp_file_name, "w");
  if (!file_out)
  {
    debugprintf(0, "USMUserTable::save_to_file: could not open tmpfile"
		" (%s)", tmp_file_name);
    return SNMPv3_USM_FILECREATE_ERROR;
  }

  bool failed = false;
  for (int i=0; i < entries; ++i)
  {
    debugprintf(4, "Saving user %s to file",
		OctetStr(table[i].usmUserName, table[i].usmUserNameLength)
		        .get_printable());

    encodeString(table[i].usmUserEngineID, table[i].usmUserEngineIDLength,
		 encoded);
    encoded[2 * table[i].usmUserEngineIDLength] = '\n';
    if (fwrite(encoded, 2 * table[i].usmUserEngineIDLength + 1, 1, file_out)
	!= 1)
    { failed = true; break; }

    encodeString(table[i].usmUserName, table[i].usmUserNameLength, encoded);
    encoded[2 * table[i].usmUserNameLength] = '\n';
    if (fwrite(encoded, 2 * table[i].usmUserNameLength + 1, 1, file_out) != 1)
    { failed = true; break; }

    encodeString(table[i].usmUserSecurityName,
		 table[i].usmUserSecurityNameLength, encoded);
    encoded[2 * table[i].usmUserSecurityNameLength] = '\n';
    if (fwrite(encoded, 2 * table[i].usmUserSecurityNameLength + 1, 1,
	       file_out) != 1)
    { failed = true; break; }

    encodeString(table[i].usmUserAuthKey, table[i].usmUserAuthKeyLength,
		 encoded);
    encoded[2 * table[i].usmUserAuthKeyLength] = '\n';
    if (fwrite(encoded, 2 * table[i].usmUserAuthKeyLength + 1, 1, file_out)
	!= 1)
    { failed = true; break; }

    encodeString(table[i].usmUserPrivKey, table[i].usmUserPrivKeyLength,
		 encoded);
    encoded[2 * table[i].usmUserPrivKeyLength] = '\n';
    if (fwrite(encoded, 2 * table[i].usmUserPrivKeyLength + 1, 1, file_out)
	!= 1)
    { failed = true; break; }
    
    if (table[i].usmUserAuthProtocol == SNMP_AUTHPROTOCOL_NONE)
    {
      if (fwrite("none\n", 5, 1, file_out) != 1)
      { failed = true; break; }
    }
    else
    {
      const Auth *a = ap->get_auth(table[i].usmUserAuthProtocol);
      if (!a) { failed = true; break; }
      sprintf(encoded, "%s\n", a->get_id_string());
      if (fwrite(encoded, strlen(a->get_id_string()) + 1, 1, file_out) != 1)
      { failed = true; break; }
    }

    if (table[i].usmUserPrivProtocol == SNMP_PRIVPROTOCOL_NONE)
    {
      if (fwrite("none\n", 5, 1, file_out) != 1)
      { failed = true; break; }
    }
    else
    {
      const Priv *p = ap->get_priv(table[i].usmUserPrivProtocol);
      if (!p) { failed = true; break; }
      sprintf(encoded, "%s\n", p->get_id_string());
      if (fwrite(encoded, strlen(p->get_id_string()) + 1, 1, file_out) != 1)
      { failed = true; break; }
    }
  }
  fclose(file_out);
  if (failed)
  {
    debugprintf(0, "Failed to write table entries.");
#ifdef WIN32
    _unlink(tmp_file_name);
#else
    unlink(tmp_file_name);
#endif
    return SNMPv3_USM_FILEREAD_ERROR;
  }
#ifdef WIN32
  _unlink(name);
#else
  unlink(name);
#endif
  if (rename(tmp_file_name, name))
  {
    debugprintf(0, "save_to_file: could not rename tmpfile (%s)"
                  " to file (%s)", tmp_file_name, name);
    return SNMPv3_USM_FILERENAME_ERROR;
  }
  debugprintf(5, "save_to_file: saved all users");
  return SNMPv3_USM_OK;
}

// Load the table from a file.
int USMUserTable::load_from_file(const char *name, AuthPriv *ap)
{
  char decoded[MAX_LINE_LEN];
  FILE *file_in;
  unsigned char line[MAX_LINE_LEN * 2];

  if (!name)
  {
    debugprintf(0, "USMUserTable::load_from_file: filename is NULL.");
    return SNMPv3_USM_ERROR;
  }

  if (!ap)
  {
    debugprintf(0, "USMUserTable::load_from_file: AuthPriv ptr is NULL.");
    return SNMPv3_USM_ERROR;
  }

  file_in = fopen(name, "r");
  if (!file_in)
  {
    debugprintf(1, "load_from_file: Could not open (file): (%s)", name);
    return SNMPv3_USM_FILEOPEN_ERROR;
  }

  bool failed = false;
  int len;
  while (fgets((char*)line, MAX_LINE_LEN * 2, file_in))
  {
    // engine_id
    len = strlen((char*)line) - 1;
    decodeString(line, len, decoded);
    OctetStr engine_id((unsigned char*)decoded, len / 2);

    // user_name
    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    len = strlen((char*)line) - 1;
    decodeString(line, len, decoded);
    OctetStr user_name((unsigned char*)decoded, len / 2);

    // security_name
    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    len = strlen((char*)line) - 1;
    decodeString(line, len, decoded);
    OctetStr user_security_name((unsigned char*)decoded, len / 2);

    // auth key
    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    len = strlen((char*)line) - 1;
    decodeString(line, len, decoded);
    OctetStr auth_key((unsigned char*)decoded, len / 2);

    // priv key
    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    len = strlen((char*)line) - 1;
    decodeString(line, len, decoded);
    OctetStr priv_key((unsigned char*)decoded, len / 2);

    // auth protocol
    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    line[strlen((char*)line) - 1] = 0;
    int auth_prot = SNMP_AUTHPROTOCOL_NONE;
    if (strcmp((char*)line, "none") != 0)
    {
      auth_prot = ap->get_auth_id((char*)line);
      if (auth_prot < 0)
      { failed = true; break; }
    }

    if (!fgets((char*)line, MAX_LINE_LEN * 2, file_in))
    { failed = true; break; }
    line[strlen((char*)line) - 1] = 0;
    int priv_prot = SNMP_PRIVPROTOCOL_NONE;
    if (strcmp((char*)line, "none") != 0)
    {
      priv_prot = ap->get_priv_id((char*)line);
      if (priv_prot < 0)
      { failed = true; break; }
    }
    debugprintf(2, "Adding localized user: %s, %s, ap %i, pp %i",
		user_name.get_printable(),
		engine_id.get_printable(), auth_prot, priv_prot);
    if (add_entry(engine_id, user_name, user_security_name,
		  auth_prot, auth_key, priv_prot, priv_key)
	== SNMPv3_USM_ERROR)
    {
      failed = true;
      debugprintf(0, "Error adding user %s", user_name.get_printable());
    }
  }

  fclose(file_in);
  if (failed)
  {
    debugprintf(0, "Failed to read table entries.");
    return SNMPv3_USM_FILEREAD_ERROR;
  }

  debugprintf(5, "load_from_file: loaded all users");
  return SNMPv3_USM_OK;
}

const UsmUserTableEntry *USMUserTable::peek_next(
                   const UsmUserTableEntry *e) const
{
  if (e == 0) return 0;
  if (e - table < 0) return 0;
  if (e - table >= entries - 1) return 0;
  return (e + 1);
}

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 

#endif // _SNMPv3
