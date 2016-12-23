// Program Information ////////////////////////////////////////////////////////
/**
 * @file SimpleQueue.h
 *
 * @brief Definition file for SimpleQueue class
 * 
 * @author Michael Leverington
 * 
 * @details Specifies all member methods of the SimpleQueue class
 *
 * @version 1.00
 *          Michael Leverington (13 April 2016)
 *          Original Code
 *
 * @Note None
 */

// Precompiler directives /////////////////////////////////////////////////////

#ifndef CLASS_SIMPLEQUEUE_H
#define CLASS_SIMPLEQUEUE_H

// Header files ///////////////////////////////////////////////////////////////

#include <iostream>
using namespace std;

#include "SimpleList.cpp"

// Class definition  //////////////////////////////////////////////////////////


template <class DataType>
class SimpleQueue : public SimpleList<DataType>
   {
    public:

    static const int INITIAL_CAPACITY = 10;

    // constructors
    SimpleQueue();
    SimpleQueue( const SimpleQueue &copiedQueue ); 

    // destructor
    ~SimpleQueue( );

    // assignment
    const SimpleQueue &operator = ( const SimpleQueue &rhQueue );

    // accessors

       // reports empty queue
       bool isEmpty() const;

    // modifiers

       // enqueues data
       void enqueue( const DataType &enqueueData );

       // dequeues data
       bool dequeue( DataType &dequeueData);

       // allows access to front without removal
       bool peekFront( DataType &peekData );

    private:

       int queueSize;

   };

// Terminating precompiler directives  ////////////////////////////////////////

#endif		// #ifndef CLASS_SIMPLEQUEUE_H

