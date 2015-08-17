// bdlcfxxxr_vfunc0.t.cpp                                             -*-C++-*-

#include <bdlcfxxxr_vfunc0.h>
#include <bslma_testallocator.h>     // for testing only
#include <bslma_allocator.h>
#include <bslma_default.h>           // for testing only
#include <bdlqq_xxxthread.h>            // for testing only
#include <bdlqq_barrier.h>           // for testing only

#include <bsl_iostream.h>
#include <bsl_c_stdlib.h>     // atoi()

using namespace BloombergLP;
using namespace bsl;  // automatically added by script


//=============================================================================
//                             TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
// 'bdlcfxxxr::Vfunc0' is an abstract class that provides a pure virtual function
// 'execute', a reference counter 'd_count', and methods to modify it.  Each
// instance of this class holds a pointer to the memory allocator used to
// create it.  The class provides a static method to be used to destroy the
// object and subsequently deallocate it.
//
// This test verifies that member functions modify 'd_count' according to their
// specification and that the static 'deleteObject' method destroys the
// derived instance and successfully uses the memory allocator supplied at
// construction to deallocate the memory used by the destroyed object.
//
// We can observe the value of 'd_count' only through the return value of the
// 'decrement' method.  We will use a composition of the 'increment' and
// 'decrement' functions to retrieve the value of 'd_count'.
//
// We will use the API provided by bslma::TestAllocator to verify that
// 'deleteObject' deallocates memory as expected.
//
// In order to test 'bdlcfxxxr::Vfunc0' we have to implement a concrete class
// derived from it.  This class automatically tests the correctness of the
// signature of the pure virtual function 'execute'.  We will use global
// variables to ensure that both 'execute' method and the virtual destructor
// are called.
//
// We use the following abbreviations:
//  'bA'      for 'bdema::Allcoator'
//  'c_A1'    for 'const A1'
//  'c_A1-N   for 'const A1, const A2, ... ,  const AN'
//  'c_A1-N&' for 'const A1&, const A2&, ... , const AN&'
//  'c_D1'    for 'const D1'
//  'c_D1-N'  for 'const D1, const D2, ... , const DN'
//  'c_D1-N&' for 'const D1&, const D2&, ... , const DN&'
//
//-----------------------------------------------------------------------------
// [ 2] bdlcfxxxr::Vfunc0(bslma::Allocator *ba);
// [ 2] void increment();
// [ 2] int decrement();
// [ 2] void execute() const;
// [ 2] ~bdlcfxxxr::Vfunc0();
// [ 2] static void deleteObject(bdlcfxxxr::Vfunc0 *object);
//-----------------------------------------------------------------------------
// [ 1] int getCount(Obj *object);
// [ 3] Thread-safeness test
// [ 4] USAGE EXAMPLE

//=============================================================================
//                  STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
static int testStatus = 0;

static void aSsErT(int c, const char *s, int i) {
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (testStatus >= 0 && testStatus <= 100) ++testStatus;
    }
}

# define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

//=============================================================================
//                  SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------
#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", "<< flush; // P(X) without '\n'
#define L_ __LINE__                           // current Line number

//=============================================================================
//                  CONCRETE DERIVED CLASS FOR TESTING
//-----------------------------------------------------------------------------

static int executeUsageCounter = 0;
static int dtorUsageCounter = 0;
class ConcreteDerivedClass : public bdlcfxxxr::Vfunc0 {
  public:
    ConcreteDerivedClass(bslma::Allocator *basicAllocator)
    : bdlcfxxxr::Vfunc0(basicAllocator) { }

    virtual void execute() const
        // Increment global counter 'testCounter'.
        //
        // This function tests that the 'execute' method of 'bdlcfxxxr::Vfunc0' is
        // indeed virtual.  If it is not virtual, then the 'testCounter' will
        // not be incremented.
    {
        ++executeUsageCounter;
    }

  private:
    virtual ~ConcreteDerivedClass()
        // Destroy the class instance.  Increment a global 'dtorUsageCounter'.
        //
        // Test that a) the destructor in the base class is virtual, b) the
        // destructor is called.
    {
        ++dtorUsageCounter;
    }
};

//=============================================================================
//                  ENVELOPE CLASS FOR USAGE EXAMPLE
//-----------------------------------------------------------------------------

class EnvelopeClass {
    // Provide an object that encapsulates a 'bdlcfxxxr::Vfunc0' object.

    bdlcfxxxr::Vfunc0 *d_rep_p;  // polymorphic functor representation

  public:
    // CREATORS
    EnvelopeClass(bdlcfxxxr::Vfunc0 *rep) : d_rep_p(rep)
        // Create a functor that assumes shared ownership of the specified,
        // dynamically allocated, reference-counted representation.
    {
        if (d_rep_p) {
            d_rep_p->increment();
        }
    }

    ~EnvelopeClass()
        // Decrement the reference count of that internal representation
        // object, and, if the count is now 0, destroy and deallocate the
        // representation using 'deleteObject' method of 'bdlcfxxxr::Vfunc0' class.
    {
        if (0 == d_rep_p->decrement()) {
            bdlcfxxxr::Vfunc0::deleteObject(d_rep_p);
        }
    }


    // ACCESSORS
    void operator()() const
        // Execute this functor.
    {
        d_rep_p->execute();
    }
};

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------
typedef ConcreteDerivedClass DerivedObj;
typedef bdlcfxxxr::Vfunc0 Obj;


struct TestThreadArgs {
    // Arguments used to test the thread-safeness of the the class.
    DerivedObj      *d_obj_p;
    int             d_iterations;
    bdlqq::Barrier   *d_barrier_p;
};

//=============================================================================
//                  GLOBAL HELPER FUNCTIONS FOR TESTING
//-----------------------------------------------------------------------------

static int getCount(Obj *object)
    // Return the current count of the specified 'object'.
{
    // Since we can retrieve the value of the count only by calling the
    // 'decrement' method, we will use a composition of the 'increment' and
    // 'decrement' functions to retrieve the value of 'd_count' yet still
    // preserve its current value.

    object->increment();
    return object->decrement();
}

extern "C" {

void * testIncDecThreadSafeness(void *ptr)
    // This function is used to test the thread safeness of the increment and
    // decrement methods.  It repeatedly increments and decrements the
    // provided object for the specified number of iterations.
{
    TestThreadArgs *args = (TestThreadArgs*)ptr;
    args->d_barrier_p->wait();
    for (int i=0; i < args->d_iterations; ++i) {
        args->d_obj_p->increment();
        args->d_obj_p->decrement();
    }
    return ptr;
}

}

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    int verbose = argc > 2;
    int veryVerbose = argc > 3;
    int veryVeryVerbose = argc > 4;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;
    bslma::TestAllocator testAllocator(veryVeryVerbose);
    switch (test) { case 0:  // Zero is always the leading case.
      case 4: {
        // --------------------------------------------------------------------
        // TESTING USAGE EXAMPLE
        //   The usage example provided in the component header file must
        //   compile, link, and run on all platforms as shown.
        //
        // Plan:
        //   Incorporate usage example from header into driver, remove leading
        //   comment characters, replace 'assert' with 'ASSERT', inserted
        //   'getCount' statements to confirm the value of the reference
        //   counter.
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "Testing Usage Example" << endl
                          << "=====================" << endl;
        {
            executeUsageCounter = 0;
            dtorUsageCounter = 0;

            typedef ConcreteDerivedClass DerivedObj;
            typedef bdlcfxxxr::Vfunc0 Obj;
            bslma::Allocator *myAllocator = bslma::Default::defaultAllocator();

            Obj *x = new(*myAllocator) DerivedObj(myAllocator);
            {
                // The reference counter is 0
                ASSERT(0 == getCount(x));
                EnvelopeClass env1(x);
                env1();        ASSERT(1 == executeUsageCounter);
                // The reference counter is 1
                ASSERT(1 == getCount(x));
                {
                    EnvelopeClass env1(x);
                    // The reference counter is 2
                    ASSERT(2 == getCount(x));
                    env1();    ASSERT(2 == executeUsageCounter);
                }
                // The reference counter is 1
                ASSERT(1 == getCount(x));
                ASSERT(0 == dtorUsageCounter);
            }
            ASSERT(1 == dtorUsageCounter);
        }
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING increment/decrement manipulators thread safeness
        //   Although atomic integer numbers are used for the counters,
        //   we still need to perform some simple multi-threaded,  high
        //   contenation tests to assert that the counter increment and
        //   decrement properly in multi-threaded environments.
        //
        // Plan
        //   Create a series of threads, each of which repeated 'increment'
        //   and 'decrement' the reference count for a common object.  Wait
        //   for all the threads to finish ad assert that the final reference
        //   count for the object is correct.
        // Testing:
        //   Thread safeness
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "Testing thread safeness" << endl
                          << "=======================" << endl;
        {
            const int NTHREADS=3;
            const int NITERATIONS=10000;
            TestThreadArgs args;

            bslma::Allocator *myAllocator = bslma::Default::defaultAllocator();

            args.d_obj_p = new(*myAllocator) DerivedObj(myAllocator);
            args.d_barrier_p = new(*myAllocator) bdlqq::Barrier(NTHREADS);
            args.d_iterations = NITERATIONS;
            args.d_obj_p->increment();

            bdlqq::ThreadUtil::Handle threadHandles[NTHREADS];
            bcemt_Attribute attributes;

            for (int i=0; i<NTHREADS;++i) {
                bdlqq::ThreadUtil::create(&threadHandles[i], attributes,
                                         testIncDecThreadSafeness, &args );
            }

            for (int i=0; i<NTHREADS;++i) {
                bdlqq::ThreadUtil::join(threadHandles[i]);
            }
            ASSERT(0 == args.d_obj_p->decrement());
            myAllocator->deallocate(args.d_obj_p);
            myAllocator->deallocate(args.d_barrier_p);

        }

      } break;

      case 2: {
        // --------------------------------------------------------------------
        // TESTING CREATORS AND MANIPULATORS
        //   The basic concerns are that the constructor and manipulators
        //   'increment' and 'decrement' initialize/manipulate the 'd_count'
        //   correctly, and that 'deleteObject' class method destroys the
        //   object and deallocates it to the held valid memory allocator
        //   supplied at construction.
        //
        //   We have the following specific concerns:
        //    1. The constructor initializes 'd_count' to '0' and initializes
        //       memory allocator to the value that was passed in.
        //    2. 'increment' increases the value of 'd_count' by '1'.
        //       'decrement' decreases the value of 'd_count' by '1'.
        //    3. 'deleteObject' (a) destroys the object (b) frees the memory
        //       using a basic alloator supplied at construction.
        //    4. 'execute' is defined as a virtual method.
        //    5. 'dtor' is defined as a virtual method.
        //
        // Plan
        //   1.   Create a class instance that uses a 'bslma::TestAllocator' to
        //        initialize 'd_allocator_p'.  Check the value of 'd_count'
        //        using global 'getCount' function.
        //   2.   Verify that 'd_count' equals '0' using 'getCount' function.
        //        Check that 'increment' and 'decrement' work correctly when
        //        'd_count' is >= '0'.
        //   3,5. Invoke 'deleteObject'.  (a) Verify that it destroys the
        //        object by calling its destructor.  (b) Verify that the
        //        memory was freed by 'bslma::TestAllocator'.
        //   4.   Invoke 'execute' method.  Verify that the global counter was
        //        incremented.
        //
        // Testing:
        //   bdlcfxxxr::Vfunc0(bslma::Allocator *ba);
        //   void increment();
        //   int decrement();
        //   void execute() const;
        //   static void deleteObject(bdlcfxxxr::Vfunc0 *object);
        //   ~bdlcfxxxr::Vfunc0();
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "Testing Basic Functionality" << endl
                          << "===========================" << endl;

        executeUsageCounter = 0;
        dtorUsageCounter = 0;

        if (veryVerbose) cout <<
            "\nTesting Constructor." << endl;
        {
            ASSERT(0 == testAllocator.numBytesInUse());

            Obj *x = new(testAllocator) DerivedObj(&testAllocator);
            ASSERT(sizeof(Obj) == testAllocator.numBytesInUse());

            if (veryVerbose) { cout << "\t"; P(x);}
            if (veryVerbose)
                cout << "\tTesting that constructor initializes 'd_count' "
                        "correctly." << endl;
            ASSERT(0 == getCount(x));

            if (verbose)
                cout << "\nTesting 'increment' and 'decrement'." << endl;

            if (veryVerbose)
                cout << "\tTesting that 'increment' from '0' works as "
                        "expected." << endl;
            x->increment();
            ASSERT(1 == getCount(x));

            if (veryVerbose)
                cout << "\tTesting that 'increment' from positive works as "
                        "expected." << endl;
            x->increment();
            ASSERT(2 == getCount(x));

            x->decrement();
            ASSERT(1 == getCount(x));

            if (veryVerbose)
                cout << "\tTesting that 'decrement' from '1' works as "
                        "expected." << endl;
            x->decrement();
            ASSERT(0 == getCount(x));

            if (verbose) cout << "\nTesting 'execute'." << endl;

            x->execute();
            ASSERT(1 == executeUsageCounter);
            x->execute();
            ASSERT(2 == executeUsageCounter);

            if (verbose) cout << "\nTesting 'deleteObject'." << endl;

            ASSERT(sizeof(Obj) == testAllocator.numBytesInUse());
            Obj::deleteObject(x);
            ASSERT(0 == testAllocator.numBytesInUse());
            ASSERT(1 == dtorUsageCounter);
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // TESTING 'getCount' HELPER FUNCTION:
        //   We need to confirm that the helper returns current count without
        //   chnging it.
        //
        // Plan:
        //   Construct an object x and verify that 'getCount(x)' always returns
        //   the same value as 'x.decrement()' in the following sequence:
        //       x.increment();  x.decrement();
        //
        // Testing:
        //   int getCount(Obj *object);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "Testing 'getCount' Helper Function" << endl
                          << "==================================" << endl;
        {
            Obj *x = new(testAllocator) DerivedObj(&testAllocator);
            x->increment();
            int expected = x->decrement();
            ASSERT(expected == getCount(x));

            // Do it again with the same value.
            x->increment();
            expected = x->decrement();
            ASSERT(expected == getCount(x));

            // Do it twice with different values.
            x->increment();
            x->increment();
            expected = x->decrement();
            ASSERT(expected == getCount(x));

            x->increment();
            x->increment();
            expected = x->decrement();
            ASSERT(expected == getCount(x));

            Obj::deleteObject(x);
        }
      } break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "." << endl;
    }

    return testStatus;
}

// ----------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2002
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ----------------------------------
