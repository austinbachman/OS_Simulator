//SimpleQueue.cpp
//by Austin Bachman
//4/22/16
//implements SimpleQueue functions
//Modified for CS446 Assignment 2
//9/24/16

// Precompiler directives /////////////////////////////////////////////////////

#ifndef CLASS_SIMPLEQUEUE_CPP
#define CLASS_SIMPLEQUEUE_CPP

// Header files ///////////////////////////////////////////////////////////////

#include "SimpleQueue.h"

using namespace std;

template <class DataType>
SimpleQueue<DataType>::SimpleQueue
   (
   	// no parameters
   )
       : SimpleList<DataType>( INITIAL_CAPACITY ), queueSize( 0 )
{
	// initializers used
}

template <class DataType>
SimpleQueue<DataType>::SimpleQueue
   (
    const SimpleQueue &copiedQueue 
   ) 
       : SimpleList<DataType>( copiedQueue ),
         queueSize( copiedQueue.queueSize )
{
	this->copyList( this->listData, copiedQueue.listData, queueSize );
	//this->listData( copiedQueue.listData );
}

template <class DataType>
SimpleQueue<DataType>::~SimpleQueue
   (
    // no parameters 
   ) 
{
	//parent destructor called
}

template <class DataType>
const SimpleQueue<DataType>& SimpleQueue<DataType>::operator =
   (
    const SimpleQueue &rhQueue
   )
{
	if( this != &rhQueue )
	{
		*this = rhQueue;
	}

	return *this;
}


template <class DataType>
bool SimpleQueue<DataType>::isEmpty
   (
   	// no parameters
   ) const
{
	return ( queueSize == 0 );
}

template <class DataType>
void SimpleQueue<DataType>::enqueue
   (
    const DataType &enqueueData 
   )
{
	if( queueSize == this->getCapacity() )
	{
		this->resize( this->getCapacity() * 1.25 );
	}

	this->operator[]( queueSize ) = enqueueData;
	queueSize++;
}

template <class DataType>
bool SimpleQueue<DataType>::dequeue
   (
    DataType &dequeueData 
   )
{
	int index; 

	if( !isEmpty() )
	{
		queueSize--;
		dequeueData = this->operator []( 0 );

		for( index = 0; index < queueSize; index++ )
		{
			this->operator[]( index ) = this->operator[]( index + 1 );
		}

		return true;
	}

	return false;
}

template <class DataType>
bool SimpleQueue<DataType>::peekFront
   (
    DataType &peekData 
   )
{
	if( !isEmpty() )
	{
		peekData = this->operator[]( 0 );
		return true;
	}

	return false;
}

// Terminating precompiler directives  ////////////////////////////////////////

#endif		// #ifndef CLASS_SIMPLEQUEUE_CPP

