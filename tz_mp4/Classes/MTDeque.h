//
//  MTDEQUE.H
//
//  Source code from:
//
//  Serial Communications: A C++ Developer's Guide, 2nd Edition
//  by Mark Nelson, M&T Books, 1999
//
//  Please see the book for information on usage.
//
// This file contains the definition for class MTDeque, a
// double ended queue of type char that is multithread safe.
// This class is used to implemnet the I/O buffers for class
// Win32Port.
//

#ifndef MTDEQUE_H
#define MTDEQUE_H

#include <deque>
#include <string>
using namespace std;

#include "MutexInteger.h"
using namespace vfc;
//
// Class MTdeque is a class used in this program to provide an
// implementation of the standard continer deque<T> that is
// safe in a multithread program. This is done by creating a
// semaphore that wraps any member functions that access the
// object. This class is used in the input and output buffers
// for the Win32Port class. deque<T> is a double ended queue,
// and provides fairly fast access to elements in the queueu
// from both ends. While this may not be as fast as a custom
// container designed specifically for this app, using standard
// library classes should give you a nice safe feeling.
//

class MTdeque
{
//
// All three data members of this class are protectd; anything
// the end user wants to know needs to come from a member
// function. The deque<BYTE> member is where the data is
// actually stored. m_iMaxSize is the suggested maximum size.
// The CRITICAL_SECTION member protects access to the queue
// against the danger of simultaneous calls from multiple
// threads
//
protected :
    int m_iMaxSize;
    deque<byte> m_Queue;
    CVorxMutex m_Lock;
	

    //
    // Since the m_iMaxSize is a const member, we need to
    // assign it a value in a member initialization list.
    // We can accept the default constructor for the
    // deque<BYTE> member, as it just creates an empty
    // container. And the critical section needs to be
    // initialized using a call to the Win32 API.
    //
public :
    MTdeque( int max_size ) : m_iMaxSize( max_size )
    {
        //::InitializeCriticalSection( &m_Lock );
    }
    //
    // The destructor for the m_Queue member will take care
    // of properly cleaning up all of its data. We have to
    // make sure we use the API call to properly free up
    // the CRITICAL_SECTION object. And that's all!
    //
    ~MTdeque()
    {
		Clear();
        //::DeleteCriticalSection( &m_Lock );
    }
    //
    // This is a multithread safe version of the function
    // that tells us how much free space is in the queue.
    // There is a belt and suspenders check to make sure we
    // don't return a negative number if the user has
    // somehow exceeded the maximum size of the queue. This
    // shouldn't happen, but if it does for some reason, we
    // are protected against confusion.
    //
    int SpaceFree()
    {
		CAutoMutex am(&m_Lock);
		//////////////////////////
		int size = 0;
        //m_Lock.Lock();
        size = m_iMaxSize - m_Queue.size();
        //m_Lock.UnLock();
        return ( size < 0 ) ? 0 : size;
    }
    //
    // This returns the amount of space used in the queue.
    // Like the previous member, it uses our private
    // critical section object to protect against the
    // possible ravages that may occur if we use this
    // function while a different thread is accessing the
    // queue.
    //
    int SpaceUsed()
    {
		CAutoMutex am(&m_Lock);
		//////////////////////////
		int size = 0;
        //m_Lock.Lock();
        size = m_Queue.size();
        //m_Lock.UnLock();
        return size;
    }
    //
    // This is a thread safe version of a function that
    // inserts a single character into the queue. It
    // protects against exceeding the desired size of the
    // queue, and like some of the old stdio C functions,
    // it returns the character inserted on success, and
    // negative number upon failure. This function is used
    // in Win32Port by the write_byte() member function.
    //
    int Insert( BYTE c )
    {
		CAutoMutex am(&m_Lock);
		//////////////////////////

        int return_value;
        //m_Lock.Lock();
        if ( m_Queue.size() < m_iMaxSize ) {
            m_Queue.push_back( c );
            return_value = c & 0xff;
        } else
            return_value = -1;
        //m_Lock.UnLock();
        return return_value;
    }
    //
    // This overloaded version of Insert() is used to
    // insert an entire buffer of characters into the
    // deque<char> member of the class. It uses the
    // critical section created in the constructor to
    // protect us from multithread conflicts during the
    // operation. It returns the actual count of characters
    // inserted to the caller. If the caller doesn't
    // attempt to exceed the maximum character size the
    // function will always succeed.
    //
    int Insert( BYTE *data, int count )
    {
		CAutoMutex am(&m_Lock);
		//////////////////////////

        //m_Lock.Lock();
        int actual = m_iMaxSize - m_Queue.size() ;
        if ( actual < 0 )
            actual = 0;
        if ( count < actual )
            actual = count;
        for ( int i = 0 ; i < actual ; i++ )
            m_Queue.push_back( *data++ );
        //m_Lock.UnLock();
        return actual;
    }
    //
    // This function is used to extract a buffer full of
    // characters from the deque<char> member of this
    // class. The user passes a pointer to a buffer and a
    // maximum count of characters desired. In turn, this
    // function extracts as many characters as possible
    // from the the deque<char> and inserts them into the
    // user buffer. It then returns the count of characters
    // to the caller. This function is used by the
    // read_bytes() member of Win32Port.
    //
    int Extract( BYTE *data, int max )
    {
		CAutoMutex am(&m_Lock);
		//////////////////////////

        int i = 0;
        //m_Lock.Lock();
        while ( i < max && m_Queue.size() ) {
            data[ i++ ] = m_Queue.front();
            m_Queue.pop_front();
        }
        //m_Lock.UnLock();
        return i;
    }
	int Extract(int max)
	{
		CAutoMutex am(&m_Lock);
		//////////////////////////

		int i = 0;
        //m_Lock.Lock();
        while ( i < max && m_Queue.size() ) {
            m_Queue.pop_front();
			i++;
        }
        //m_Lock.UnLock();
        return i;
	}
    //
    // Peek is used to look into the buffer without
    // actually removing the characters. Naturally it looks
    // an awful lot like Extract().
    //
    int Peek( BYTE *data, int max )
    {
		CAutoMutex am(&m_Lock);
		//////////////////////////

        //m_Lock.Lock();
        if ( max > m_Queue.size() )
            max = m_Queue.size();
        for ( int i = 0 ; i < max ; i++ )
            data[ i ] = m_Queue.begin()[ i ];
        //m_Lock.UnLock();
        return i;
    }
    //
    // This overloaded version of Extract() is used by the
    // read_byte() member of Win32Port to read a single
    // character at a time from the deque<char> member of
    // this class. If a character is available, it is
    // returned directly to the caller in the lower eight
    // bits of the int return value. This function is
    // called by the read_byte() member function of the
    // Win32Port class.
    //
    int Extract()
    {
		CAutoMutex am(&m_Lock);
		//////////////////////////

        int ret_val = -1;
        //m_Lock.Lock();
        if ( m_Queue.size() ) {
            ret_val = m_Queue.front() & 0xff;
            m_Queue.pop_front();
        }
        //m_Lock.UnLock();
        return ret_val;
    }
    //
    // This member just empties the queue.
    //
    void Clear()
    {
		CAutoMutex am(&m_Lock);
		//////////////////////////

        //m_Lock.Lock();
        m_Queue.clear();
        //m_Lock.UnLock();
    }

	//////////////////////
	void printfList()
	{
		CAutoMutex am(&m_Lock);
		//////////////////////////

		static byte BUFF[512];
		//m_Lock.Lock();
		int size= m_Queue.size();
		Peek(BUFF,size); 
		//m_Lock.UnLock();
		TRACE("\nqueue data::");
		for (int i= 0; i<size; i++) 
			TRACE("%02x",BUFF[i]);
	}
};

#endif

// EOF MTDeque.h
