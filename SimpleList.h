// Program Information ////////////////////////////////////////////////////////
/**
 * @file SimpleList.h
 *
 * @brief Definition file for SimpleList class
 * 
 * @author Michael Leverington
 * 
 * @details Specifies all member methods of the SimpleList class
 *
 * @version 1.00
 *          Michael Leverington (13 April 2016)
 *          Original Code
 *
 * @Note None
 */

// Precompiler directives /////////////////////////////////////////////////////

#ifndef CLASS_SIMPLELIST_H
#define CLASS_SIMPLELIST_H

// Header files ///////////////////////////////////////////////////////////////

#include <iostream>

using namespace std;

// Class definition  //////////////////////////////////////////////////////////

const bool REMOVE_ITEM = true;
const bool DONT_REMOVE_ITEM = false;

template <class DataType>
class SimpleList
   {
    public:

    // Internal constant
    static const int DEFAULT_CAPACITY = 10;

    // constructors
    SimpleList( int newCapacity = DEFAULT_CAPACITY );
    SimpleList( const SimpleList &copiedList ); 

    // destructor
    ~SimpleList( );

    // assignment
    const SimpleList &operator = ( const SimpleList &rhList );

    // brackets
    DataType &operator [ ] ( int index );
    const DataType & operator [ ] ( int index ) const;
    
    // accessors

       int getSize() const;
       int getCapacity() const;

    // modifiers

       // resets size
       void clear();

       // appends data to list
       void addItem( const DataType &item );

       // linear search for data
       bool findData( DataType &item, bool removeFlag );

       // allows resizing to increase capacity only
       bool resize( int newCapacity );

 // private:

    // copies data between dynamically allocated arrays
    void copyList( DataType *dest, const DataType *source, int copyLimit );

    // accesses data at specified index, with option to remove
    void getAtIndex( int index, DataType &foundData, bool removeFlag );

    // utility - finds maximum of two values
    int getMax( int one, int other );

    int listSize;
    int listCapacity;
    DataType *listData;
   };

// Terminating precompiler directives  ////////////////////////////////////////

#endif		// #ifndef CLASS_SIMPLELIST_H

