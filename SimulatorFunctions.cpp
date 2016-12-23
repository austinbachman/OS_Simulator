// Program Information /////////////////////////////////////////////////////////
/**
 * @file SimulatorFunctions.c
 *
 * @brief Memory Address Locator and Timer Implementations for CS 446 at the 
 *		  Univserity of Nevada, Reno
 * 
 * @details This file includes the necessary function implementations for 
 *			assignment 2 of the CS 446 course simulator.
 *
 * @author Cayler Miley
 *
 * @version 2.0 (17 October 16)
 *          updated by Austin Bachman
 */

// PRECOMPILER DIRECTIVES //////////////////////////////////////////////////////

#ifndef MEM_FUNC_C
#define MEM_FUNC_C

// HEADER FILES ////////////////////////////////////////////////////////////////

#include "SimulatorFunctions.h"

// GLOBAL CONSTANTS ////////////////////////////////////////////////////////////

 // None

/**
 * @brief Memory Adress Return Function
 *
 * @details Reads in an integer representing kilobytes of total memory. Using
 *			the total memory, the function returns the address as a hexadecimal
 *			string.
 *          
 * @param in: total memory in kb (int)
 *
 * @pre None
 *
 * @post Hexadecimal string address returned
 *
 * @exception Requires value > 0
 *
 * @exception Address pointer should be null
 */
unsigned int AllocateMemory( int totMem, int blockSize, int lastLoc )
{	
    //memory uninitialized
	if( lastLoc < 0 )
	{
	    return 0;
	}
	
	//check if previous starting address
	// + space for both previous and new blocks
	// is less than the total memory, else start at 0
	else if( lastLoc + blockSize * 2 < totMem )
	{
	    return lastLoc + blockSize;
    }
    
    else
    {
	    return 0;
	}
}

/**
 * @brief Timer Function
 *
 * @details Calculates time passed through the timeval struct
 *          
 * @param in: a reference time (struct timeval)
 *
 * @pre None
 *
 * @post Returns the microseconds passed as an integer
 */
int timePassed( struct timeval refTime )
{
   struct timeval currentTime;
   int microsec, seconds;

   gettimeofday( &currentTime, NULL );
   seconds = currentTime.tv_sec - refTime.tv_sec;
   microsec = currentTime.tv_usec - refTime.tv_usec;

   if( microsec < 0 )
   {
       microsec += 1000000;
       seconds -= 1;
   }

   if( seconds > 0 )
   {
       microsec = microsec + ( seconds * 1000000 );
   }

   return microsec;
}

#endif // MEM_FUNC_C
