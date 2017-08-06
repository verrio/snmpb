/*_############################################################################
  _## 
  _##  collect.cpp  
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
/*===================================================================

  Copyright (c) 1999
  Hewlett-Packard Company

  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  Permission to use, copy, modify, distribute and/or sell this software
  and/or its documentation is hereby granted without fee. User agrees
  to display the above copyright notice and this license notice in all
  copies of the software and any documentation of the software. User
  agrees to assume all liability for the use of the software; Hewlett-Packard
  makes no representations about the suitability of this software for any
  purpose. It is provided "AS-IS" without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.


  SNMP++ C O L L E C T . C P P

  COLLECTION CLASS DEFINITION

  DESIGN + AUTHOR:
  Peter E Mellquist

  LANGUAGE:
  ANSI C++

  OPERATING SYSTEMS:
  Win 32
  BSD UNIX

  DESCRIPTION:
  Simple Collection classes for SNMP++ classes.

=====================================================================*/
char collect_cpp_version[]="#(@) SNMP++ $Id$";

#include "snmp_pp/collect.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#ifdef _OLD_TEMPLATE_COLLECTION

// create an empty collection
template <class T>
SnmpCollection<T>::SnmpCollection()
    : count(0), data(0,0) {};

// create a collection using a single template object
template <class T>
SnmpCollection<T>::SnmpCollection(const T &t)
    : count(1), data(0, 0)
  {
    data.item[0] = (T*) (t.clone());
  };

// create a collection with another collection
// copy constructor
template <class T>
SnmpCollection<T>::SnmpCollection( const SnmpCollection<T> &c)
    : count(0), data(0, 0)
  {
    if (c.count == 0) return;

    // load up the new collection
    cBlock *current = &data;
    cBlock *nextBlock;
    int cn = 0;

    while (count < c.count)
    {
      if (cn >= MAXT)
      {
	nextBlock = new cBlock(current, 0);
	current->next = nextBlock;
	current = nextBlock;
	cn=0;
      }
      T *tmp;
      c.get_element(tmp, count);
      current->item[cn] = (T*) (tmp->clone());
      count++;
      cn++;
    }
  };

// destroy the collection
template <class T>
SnmpCollection<T>::~SnmpCollection()
  {
    clear();  // just delete the data
  };

// get the size of the collection
template <class T>
int SnmpCollection<T>::size() const
  {
    return count;
  };

// append an item to the collection
template <class T>
SnmpCollection<T>& SnmpCollection<T>::operator +=( const T &i)
  {
    cBlock *current = &data;
    int cn = (int) count % MAXT;
    while (current->next)
      current = current->next;
    if ((count > 0) && ((count % MAXT) == 0))
    {
      cBlock *add = new cBlock(current, 0);
      if (!add) return *this;
      current->next = add;
      add->item[0] = (T*) (i.clone());
    }
    else
    {
      current->item[cn] = (T*) (i.clone());
    }
    count++;

    return *this;
  };

// assign one collection to another
template <class T>
SnmpCollection<T>& SnmpCollection<T>::operator = ( const SnmpCollection<T> &c)
  {
    if (this == &c) return *this;  // check for self assignment

    clear(); // delete the data

    if (c.count == 0) return *this;

    // load up the new collection
    cBlock *current = &data;
    cBlock *nextBlock;
    int cn = 0;
    count = 0;
    while (count < c.count)
    {
      if (cn >= MAXT)
      {
	nextBlock = new cBlock(current, 0);
	current->next = nextBlock;
	current = nextBlock;
	cn=0;
      }
      T *tmp;
      c.get_element(tmp, count);
      current->item[cn] = (T*) (tmp->clone());
      count++;
      cn++;
    }

    return *this;
  };

// access an element in the collection
template <class T>
T SnmpCollection<T>::operator[](const int p) const
  {
    if ((p < count) && (p >= 0))
    {
      cBlock const *current = &data;
      int bn = (int) (p / MAXT);
      int cn = (int) p % MAXT;
      for (int z=0; z<bn; z++)
	current = current->next;
      return *(current->item[cn]);
    }
    else
    {
      // return an instance of nothing!!
      T t;
      return t;
    }
  };

// set an element in the collection
template <class T>
int SnmpCollection<T>::set_element( const T& i, const int p)
  {
    if ((p < 0) || (p > count)) return -1; // not found!

    cBlock *current = &data;
    int bn = (int) p / MAXT;
    int cn = (int) p % MAXT;
    for (int z=0; z<bn; z++)
      current = current->next;
    delete current->item[cn];
    current->item[cn] = (T*) (i.clone());
    return 0;
  };

// get an element in the collection
template <class T>
int SnmpCollection<T>::get_element(T& t, const int p) const
  {
    if ((p < 0) || (p > count)) return -1; // not found!

    cBlock const *current = &data;
    int bn = (int) p / MAXT;
    int cn = (int) p % MAXT;
    for (int z=0; z<bn; z++)
      current = current->next;
    t = *(current->item[cn]);
    return 0;
  };

// get a pointer to an element in the collection
template <class T>
int SnmpCollection<T>::get_element(T *&t, const int p) const
  {
    if ((p < 0) || (p > count)) return -1; // not found!

    cBlock const *current = &data;
    int bn = (int) p / MAXT;
    int cn = (int) p % MAXT;
    for (int z=0; z<bn; z++)
      current = current->next;
    t = current->item[cn];
    return 0;
  };

// apply an function to the entire collection, iterator
template <class T>
void SnmpCollection<T>::apply( void f( T&))
  {
    T temp;
    for ( int z=0; z<count; z++)
    {
      this->get_element(temp, z);
      f(temp);
    }
  };

// looks for an element in the collection
// returns TRUE if found
template <class T>
int SnmpCollection<T>::find( const T& i,int &pos) const
  {
    T temp;
    for (int z=0; z<count; z++)
    {
      this->get_element(temp, z);
      if ( temp == i) {
	pos = z;
	return TRUE;
      }
    }
    return FALSE;
  };

// delete an element in the collection
template <class T>
int SnmpCollection<T>::remove( const T& i)
  {
    // first see if we have it
    int pos;
    if (find(i, pos))
    {
      SnmpCollection<T> newCollection;

      for (int z=0; z<count; z++)
      {
	if (z != pos)
	{
	  T item;
	  get_element(item, z);
	  newCollection += item;
	}
      }

      // assign new collection to 'this'
      operator =(newCollection);

      return TRUE;
    }
    return FALSE;   // not found thus not removed
  };

// delete all elements within the collection
template <class T>
void SnmpCollection<T>::clear()
  {
    if (count == 0) return;

    cBlock *current = &data;
    int z=0;
    int cn=0;
    while ( z< count)
    {
      if (cn >= MAXT)
      {
	cn =0;
	current = current->next;
      }
      delete current->item[cn];
      cn++;
      z++;
    }

    // delete the blocks
    while (current->next)
      current = current->next;
    while (current->prev)
    {
      current = current->prev;
      delete current->next;
    }

    count = 0;
    data.next=0;
    data.prev=0;
  };

#endif  // _OLD_TEMPLATE_COLLECTION

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 
