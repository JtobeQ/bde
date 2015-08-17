// btlmt_asyncchannel.t.cpp                                           -*-C++-*-

#include <btlmt_asyncchannel.h>

#include <btlb_blob.h>
#include <bsls_timeinterval.h>
#include <bslma_testallocator.h>

#include <bsl_iostream.h>

#include <bsl_cstdlib.h>     // atoi()

using namespace BloombergLP;
using namespace bsl;  // automatically added by script

//=============================================================================
//                             TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
// We are testing a pure protocol class.  We need to verify that a concrete
// derived class compiles and links.  We create a sample derived class that
// provides a dummy implementation of the base class virtual methods.  We then
// verify that when a method is called through a base class instance pointer
// the appropriate method in the derived class instance is invoked.
//-----------------------------------------------------------------------------
// [ 1] ~btlmt::AsyncChannel(...);
// [ 1] int read(...);
// [ 1] int timedRead(...);
// [ 1] int write(const btlb::Blob&, int);
// [ 1] void cancelRead()
// [ 1] void close()
// [ 1] btlso::IPv4Address localAddress() const;
// [ 1] btlso::IPv4Address peerAddress() const;
//-----------------------------------------------------------------------------
// [ 1] PROTOCOL TEST - Make sure derived class compiles and links.
//=============================================================================

//=============================================================================
//                      STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
static int testStatus = 0;

static void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (0 <= testStatus && testStatus <= 100) ++testStatus;
    }
}

#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

//=============================================================================
//                  STANDARD BDE LOOP-ASSERT TEST MACROS
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
   if (!(X)) { cout << #I << ": " << I << "\n"; aSsErT(1, #X, __LINE__); }}

#define LOOP2_ASSERT(I,J,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " \
              << J << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP3_ASSERT(I,J,K,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" \
              << #K << ": " << K << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP4_ASSERT(I,J,K,L,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" << \
       #K << ": " << K << "\t" << #L << ": " << L << "\n"; \
       aSsErT(1, #X, __LINE__); } }

#define LOOP5_ASSERT(I,J,K,L,M,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" << \
       #K << ": " << K << "\t" << #L << ": " << L << "\t" << \
       #M << ": " << M << "\n"; \
       aSsErT(1, #X, __LINE__); } }

#define LOOP6_ASSERT(I,J,K,L,M,N,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" << \
       #K << ": " << K << "\t" << #L << ": " << L << "\t" << \
       #M << ": " << M << "\t" << #N << ": " << N << "\n"; \
       aSsErT(1, #X, __LINE__); } }

//=============================================================================
//                  SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------
#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define NL() cout << endl;                    // End of line
#define P_(X) cout << #X " = " << (X) << ", "<< flush; // P(X) without '\n'
#define T_()  cout << '\t' << flush;          // Print tab w/o newline
#define L_ __LINE__                           // current Line number

//=============================================================================
//                  SUPPORT CLASSES AND FUNCTIONS USED FOR TESTING
//-----------------------------------------------------------------------------

//=============================================================================
//          USAGE example from header(with assert replaced with ASSERT)
//-----------------------------------------------------------------------------

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

class TestAsyncChannel : public btlmt::AsyncChannel {

    // DATA
    int *d_funcCode_p;  // code of the function, held

  private:
    // NOT IMPLEMENTED
    TestAsyncChannel(const btlmt::AsyncChannel&);
    TestAsyncChannel& operator=(const btlmt::AsyncChannel&);

  public:
    // CREATORS
    TestAsyncChannel(int *funcCode)
    : d_funcCode_p (funcCode)
    {
        ASSERT(d_funcCode_p);
        *d_funcCode_p = 0;
    }

    ~TestAsyncChannel()
    {
        *d_funcCode_p = 1;
    }

    // MANIPULATORS
    int read(int, const BlobBasedReadCallback&)
    {
        *d_funcCode_p = 3;
        return 0;
    }

    int timedRead(int,
                  const bsls::TimeInterval&,
                  const BlobBasedReadCallback&)
    {
        *d_funcCode_p = 5;
        return 0;
    }

    int write(const btlb::Blob&, int)
    {
        *d_funcCode_p = 6;
        return 0;
    }

    int setSocketOption(int, int, int)
    {
        *d_funcCode_p = 10;
        return 0;
    }

    void cancelRead()
    {
        *d_funcCode_p = 11;
    }

    void close()
    {
        *d_funcCode_p = 12;
    }

    btlso::IPv4Address localAddress() const
    {
        *d_funcCode_p = 13;
        return btlso::IPv4Address();
    }

    btlso::IPv4Address peerAddress() const
    {
        *d_funcCode_p = 14;
        return btlso::IPv4Address();
    }
};

void myBlobBasedReadCallback(int,
                             int *,
                             btlb::Blob *,
                             int)
{
}

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    int verbose = argc > 2;
    int veryVerbose = argc > 3;
    int veryVeryVerbose = argc > 4;
    bslma::TestAllocator testAllocator(veryVeryVerbose);

    cout << "TEST " << __FILE__ << " CASE " << test << endl;;

    switch (test) { case 0:  // Zero is always the leading case.
      case 1: {
       // --------------------------------------------------------------------
       // PROTOCOL TEST:
       //   All we need to do is make sure that a concrete subclass of the
       //   'btlsc::CbChannel' class compiles and links when all
       //   virtual functions are defined.
       //
       // Testing:
       //   ~btlmt::AsyncChannel(...);
       //   int read(...);
       //   int timedRead(...);
       //   void cancelRead()
       //   void close()
       //   btlso::IPv4Address localAddress() const;
       //   btlso::IPv4Address peerAddress() const;
       //
       //   PROTOCOL TEST - Make sure derived class compiles and links.
       // --------------------------------------------------------------------

       if (verbose) bsl::cout << "BREATHING TEST." << bsl::endl
                              << "===============" << bsl::endl;

       if (veryVerbose) bsl::cout <<
                "\tTesting 'btlmt::AsyncChannel': protocol test." << bsl::endl;
       {
           int opCode   = -1;
           int numBytes = 1;
           int timeout  = 600;

           TestAsyncChannel mX(&opCode);
           ASSERT(0 == opCode);

           btlmt::AsyncChannel& asyncChannel = mX;

           btlmt::AsyncChannel::BlobBasedReadCallback blobCallback =
                                                      &myBlobBasedReadCallback;
           asyncChannel.read(numBytes, blobCallback);
           ASSERT(3 == opCode);

           asyncChannel.timedRead(numBytes,
                                  bsls::TimeInterval(timeout),
                                  blobCallback);
           ASSERT(5 == opCode);

           asyncChannel.write(btlb::Blob());
           ASSERT(6 == opCode);

           asyncChannel.setSocketOption(1, 2, 3);
           ASSERT(10 == opCode);

           asyncChannel.cancelRead();
           ASSERT(11 == opCode);

           asyncChannel.close();
           ASSERT(12 == opCode);

           asyncChannel.localAddress();
           ASSERT(13 == opCode);

           asyncChannel.peerAddress();
           ASSERT(14 == opCode);
       }

       if (veryVerbose) bsl::cout
        << "\tTesting 'btlmt::AsyncChannel': destructor test." << bsl::endl;
       {
           int opCode = -1;

           btlmt::AsyncChannel *asyncChannel =
                                  new(testAllocator) TestAsyncChannel(&opCode);
           ASSERT(0 == opCode);

           testAllocator.deleteObjectRaw(asyncChannel);
           ASSERT(1 == opCode);
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
// Copyright 2015 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
