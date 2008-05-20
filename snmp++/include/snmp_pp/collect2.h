/*_############################################################################
  _## 
  _##  collect2.h  
  _##
  _##  SNMP++v3.2.23
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2007 Jochen Katz, Frank Fock
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
  _##  Stuttgart, Germany, Sun Nov 11 15:10:59 CET 2007 
  _##  
  _##########################################################################*/
// $Id$

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

template <class T> class SnmpCollection
{
  class cBlock
  {
    public:
     cBlock(cBlock *p, cBlock *n) : prev(p), next(n) {};
     T *item[MAXT];
     cBlock *prev;
     cBlock *next;
  };

 public:

  /**
   * Create an empty collection.
   */
  SnmpCollection();

  /**
   * Create a collection using a single template object.
   */
  SnmpCollection(const T &t);

  /**
   * Create a collection with another collection (copy constructor).
   */
  SnmpCollection(const SnmpCollection<T> &c);

  /**
   * Destroy the collection.
   */
  ~SnmpCollection();

  /**
   * Get the size of the collection.
   */
  int size() const;

  /**
   * Append an item to the collection.
   */
  SnmpCollection& operator +=( const T &i);

  /**
   * Assign one collection to another.
   */
  SnmpCollection & operator = ( const SnmpCollection<T> &c);

  /**
   * Access an element in the collection.
   *
   * @return The requestet element or an empty element if out of bounds.
   */
  T operator[](const int p) const;

  /**
   * Set an element in the collection.
   *
   * @return 0 on success and -1 on failure.
   */
  int set_element( const T& i, const int p);

  /**
   * Get an element in the collection.
   *
   * @return 0 on success and -1 on failure.
   */
  int get_element(T& t, const int p) const;

  /**
   * Get a pointer to an element in the collection.
   *
   * @return 0 on success and -1 on failure.
   */
  int get_element(T *&t, const int p) const;

  /**
   * Apply an function to the entire collection, iterator.
   */
  void apply( void f( T&));

  /**
   * Looks for an element in the collection.
   *
   * @return TRUE if found.
   */
  int find( const T& i,int &pos) const;

  /**
   * Delete an element in the collection.
   */
  int remove( const T& i);

  /**
   * Delete all elements within the collection.
   */
  void clear();

 private:
  int count;
  cBlock data;
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 
