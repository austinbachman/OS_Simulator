//SimpleList.cpp
//Originally by Michael Leverington
//Implements SimpleList functions
//Modified for CS446 Assignment 2
//by Austin Bachman
//9/24/16

// Precompiler directives /////////////////////////////////////////////////////

#ifndef CLASS_SIMPLELIST_CPP
#define CLASS_SIMPLELIST_CPP

// Header files ///////////////////////////////////////////////////////////////

#include "SimpleList.h"

template <class DataType>
SimpleList<DataType>::SimpleList
       ( 
        int newCapacity
       )
     : listSize( 0 )
   {
    listCapacity = getMax( newCapacity, DEFAULT_CAPACITY );

    listData = new DataType[ listCapacity ];
   }

template <class DataType>
SimpleList<DataType>::SimpleList
       ( 
        const SimpleList<DataType> &copiedList
       )
     : listSize( copiedList.listSize ),
       listCapacity( copiedList.listCapacity ),
       listData( new DataType[ listCapacity ] )
   {
    copyList( listData, copiedList.listData, listCapacity );
   }

template <class DataType>
SimpleList<DataType>::~SimpleList
       (
        // no parameters
       )
   {
    delete [] listData;
   }

template <class DataType>
const SimpleList<DataType> &SimpleList<DataType>::operator =
       (
        const SimpleList<DataType> & rhList
       )
   {
    if( this != &rhList )
       {
        delete [] listData;

        listCapacity = rhList.listCapacity;
        listSize =     rhList.listSize;
        listData = new DataType [ listCapacity ];

        copyList( listData, rhList.listData, listSize );
       }

    return *this; 
   }

template <class DataType>
DataType &SimpleList<DataType>::operator [ ]
       (
        int index
       )
   {
    return listData[ index ];
   }

template <class DataType>
const DataType &SimpleList<DataType>::operator [ ]
       (
        int index
       ) const
   {
    return listData[ index ]; 
   }

template <class DataType>
int SimpleList<DataType>::getSize
       (
        // no parameters
       ) const
   {
    return listSize;
   }

template <class DataType>
int SimpleList<DataType>::getCapacity
       (
        // no parameters
       ) const
   {
    return listCapacity;
   }

template <class DataType>
void SimpleList<DataType>::clear
       (
        // no parameters
       )
   {
    listSize = 0;
   }

template <class DataType>
void SimpleList<DataType>::addItem
       (
        const DataType &item
       )
   {
    listData[ listSize ] = item;

    listSize++;
   }

template <class DataType>
bool SimpleList<DataType>::findData
       (
        DataType &item, 
        bool removeFlag
       )
   {
    int index;
    bool found = false;

    for( index = 0; index < listSize && !found; index++ )
       {
        if( item.compareTo( listData[ index ] ) == 0 )
           {
            getAtIndex( index, item, removeFlag );

            found = true;
           }
       }

    return found;
   }

template <class DataType>
bool SimpleList<DataType>::resize
       (
        int newCapacity
       )
   {
    DataType *newList;

    if( newCapacity > listCapacity )
       {
        newList = new DataType[ newCapacity ];

        copyList( newList, listData, listCapacity );

        delete [] listData;

        listData = newList;

        listCapacity = newCapacity;

        return true;
       }

    return false;
   }

template <class DataType>
void SimpleList<DataType>::getAtIndex
       (
        int index,
        DataType &foundData,
        bool removeFlag
       )
   {
    foundData = listData[ index ];

    if( removeFlag )
       {
        listSize--;

        while( index < listSize )
           {
            listData[ index ] = listData[ index + 1 ];

            index++;
           }
       }
   }

template <class DataType>
void SimpleList<DataType>::copyList
       ( 
        DataType *dest, 
        const DataType *source,
        int copyLength  
       )
   {
    int index;

    for( index = 0; index < copyLength; index++ )
       {
        dest[ index ] = source[ index ];
       }
   }

template <class DataType>
int SimpleList<DataType>::getMax( int one, int other )
   {
    if( other > one )
       {
        return other;
       }

    return one;
   }

// Terminating precompiler directives  ////////////////////////////////////////

#endif		// #ifndef CLASS_SIMPLELIST_CPP







