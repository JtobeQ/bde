// bdlmtt_turnstile.t.cpp                                              -*-C++-*-

#include <bdlmtt_turnstile.h>

#include <bdlmtt_barrier.h>
#include <bdlmtt_mutex.h>
#include <bdlmtt_threadutil.h>
#include <bsls_atomic.h>

#include <bdlf_bind.h>
#include <bdlt_datetime.h>
#include <bdlt_currenttime.h>

#include <bsls_stopwatch.h>
#include <bsls_types.h>

#include <bsl_cmath.h>
#include <bsl_cstdio.h>
#include <bsl_cstdlib.h>
#include <bsl_iostream.h>
#include <bsl_sstream.h>
#include <bsl_string.h>
#include <bsl_vector.h>

using namespace BloombergLP;

using bsl::cout;
using bsl::cerr;
using bsl::endl;
using bsl::flush;

//=============================================================================
//                                   TEST PLAN
//-----------------------------------------------------------------------------
//                                   Overview
//                                   --------
//
// The component under test has state, but not value.  State transitions must
// be confirmed explicitly.  The timing functions can be verified to a
// reasonable degree using the 'bsls::Stopwatch::elapsedTime()' function to
// measure intervals.
//-----------------------------------------------------------------------------
// CREATORS
// [ 4] bdlmtt::Turnstile(
//              double                   rate,
//              const bsls::TimeInterval& startTime = bsls::TimeInterval(0));
// [ 1] ~bdlmtt::Turnstile();
//
// MANIPULATORS
// [ 2] bsls::Types::Int64 waitTurn();
// [ 3] void reset(double                   rate,
//                 const bsls::TimeInterval& startTime = bsls::TimeInterval(0));
//
// ACCESSORS
// [ 2] bsls::Types::Int64 lagTime() const;
//-----------------------------------------------------------------------------
// [ 6] USAGE EXAMPLE
// [ 1] BREATHING TEST
// [ 5] MULTI-THREADED TEST
//-----------------------------------------------------------------------------

//=============================================================================
//                        STANDARD BDE ASSERT TEST MACROS
//-----------------------------------------------------------------------------
static int testStatus = 0;

static void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (0 <= testStatus && testStatus <= 100)  ++testStatus;
    }
}

const double EPSILON = 0.035;          // 35 milliseconds

#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
   if (!(X)) { cout << #I << ": " << I << "\n"; aSsErT(1, #X, __LINE__); }}

#define LOOP2_ASSERT(I,J,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\n";\
               aSsErT(1, #X, __LINE__); }}

#define LOOP3_ASSERT(I,J,K,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" \
                    << #K << ": " << K << "\n";                           \
               aSsErT(1, #X, __LINE__); }}

#define LOOP4_ASSERT(I,J,K,L,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" \
                    << #K << ": " << K << "\t" << #L << ": " << L << "\n";\
               aSsErT(1, #X, __LINE__); }}

#define LOOP5_ASSERT(I,J,K,L,M,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" \
                    << #K << ": " << K << "\t" << #L << ": " << L << "\t" \
                    << #M << ": " << M << "\n";                           \
               aSsErT(1, #X, __LINE__); }}

//=============================================================================
//                       SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------
// The following macros facilitate thread-safe streaming to standard output.

static bdlmtt::Mutex coutMutex;

#define COUT  coutMutex.lock(); { bsl::cout << bdlmtt::ThreadUtil::selfIdAsInt()\
                                            << ": "
#define ENDL  bsl::endl;  } coutMutex.unlock()
#define FLUSH bsl::flush; } coutMutex.unlock()

//-----------------------------------------------------------------------------
#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", " << flush; // P(X) without '\n'
#define L_ __LINE__                           // current Line number.
#define T_()  cout << '\t' << flush;          // Print tab w/o newline.

//=============================================================================
//              GLOBAL TYPES, CONSTANTS, AND VARIABLES FOR TESTING
//-----------------------------------------------------------------------------
typedef bdlmtt::Turnstile    Obj;
typedef bsls::Types::Int64 Int64;

enum { USPS = 1000000 };  // microseconds per second

static int verbose = 0;
static int veryVerbose = 0;
static int veryVeryVerbose = 0;
static int veryVeryVeryVerbose = 0;

//=============================================================================
//                      GLOBAL HELPER FUNCTIONS FOR TESTING
//-----------------------------------------------------------------------------
static
void waitTurnAndSleepCallback(
        bdlmtt::Turnstile          *turnstile,
        bsls::AtomicInt           *counter,
        bdlmtt::Barrier            *barrier,
        const bsls::TimeInterval&  sleepInterval,
        const bdlt::Datetime&      stopTime)
{
    barrier->wait();

    do {
        Int64 wt = turnstile->waitTurn();

        int value = ++*counter;

        bdlmtt::ThreadUtil::sleep(sleepInterval);

        ASSERT(0 < turnstile->lagTime());

        if (veryVerbose) {
            COUT << "wt = "      << wt    << ", "
                    "counter = " << value
                 << ENDL;
        }
    } while (bdlt::CurrentTime::utc() < stopTime);
}

static
void waitTurnCallback(
        bdlmtt::Turnstile      *turnstile,
        bsls::AtomicInt       *counter,
        bdlmtt::Barrier        *barrier,
        const bdlt::Datetime&  stopTime)
{
    barrier->wait();

    do {
        Int64 wt = turnstile->waitTurn();

        int value = ++*counter;

        ASSERT(0 == turnstile->lagTime());

        if (veryVerbose) {
            COUT << "wt = "      << wt    << ", "
                    "counter = " << value
                 << ENDL;
        }
    } while (bdlt::CurrentTime::utc() < stopTime);
}

static
void processorWithTurnstile(
        double  rate,
        double  duration,
        double *elapsedTime)
{
    // Write the specified 'message' to the specified 'stream' at the specified
    // 'rate' (given in messages per second) for the specified 'duration'.

    bsls::Stopwatch timer;
    bdlmtt::Turnstile turnstile(rate);
    double          elapsed = 0.0;
    int             numTurns = static_cast<int>(rate * duration);

    ASSERT(static_cast<double>(numTurns) == rate * duration);  // integral

    timer.start();
    for (int i = 0; i < numTurns; ++i) {
        turnstile.waitTurn();
        bsl::sqrt(static_cast<double>(i));
        elapsed = timer.elapsedTime();
    }

    *elapsedTime = elapsed;
}

static
void processorWithSleep(
        double  rate,
        double  duration,
        double *elapsedTime)
{
    // Write the specified 'message' to the specified 'stream' at the specified
    // 'rate' (given in messages per second) for the specified 'duration'.

    bsls::Stopwatch timer;
    int             sleepInterval = static_cast<int>(1000000 / rate);  // usec
    double          elapsed = 0.0;
    int             numTurns = static_cast<int>(rate * duration);

    ASSERT(static_cast<double>(numTurns) == rate * duration);  // integral

    timer.start();
    for (int i = 0; i < numTurns; ++i) {
        bsl::sqrt(static_cast<double>(i));
        if (i + 1 == numTurns) break;
        bdlmtt::ThreadUtil::microSleep(sleepInterval);
        elapsed = timer.elapsedTime();
    }

    *elapsedTime = elapsed;
}

//=============================================================================
//                    FUNCTIONS FOR TESTING USAGE EXAMPLES
//-----------------------------------------------------------------------------
static
void heartbeat(
        bsl::ostream&       stream,
        const bsl::string&  message,
        double              rate,
        double              duration)
{
    // Write the specified 'message' to the specified 'stream' at the specified
    // 'rate' (given in messages per second) for the specified 'duration'.

    bsls::Stopwatch timer;
    timer.start();
    bdlmtt::Turnstile turnstile(rate);

    while (true) {
        turnstile.waitTurn();
        if (timer.elapsedTime() >= duration) {
            break;
        }
        stream << message;
    }
}

//=============================================================================
//                                 MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = (argc > 1) ? bsl::atoi(argv[1]) : 0;

    verbose = (argc > 2);
    veryVerbose = (argc > 3);
    veryVeryVerbose = (argc > 4);
    veryVeryVeryVerbose = (argc > 5);

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    switch (test) { case 0:
      case 7: {
        // --------------------------------------------------------------------
        // TESTING USAGE EXAMPLE 1
        //
        // Concerns:
        //   The usage example provided in the component header file must
        //   compile, link, and execute as shown.
        //
        // Plan:
        //   Incorporate the usage example from the header file into the test
        //   driver.  Make use of existing test apparatus by instantiating
        //   objects with a 'bslma::TestAllocator' object where applicable.
        //   Additionally, replace all calls to 'assert' in the usage example
        //   with calls to 'ASSERT'.  This now becomes the source, which is
        //   then "copied" back to the header file by reversing the above
        //   process.
        //
        // Testing:
        //   USAGE EXAMPLE 1
        // --------------------------------------------------------------------

        if (verbose) {
            cout << "Testing Usage Example" << endl
                 << "=====================" << endl;
        }

        const double RATE = 10;
        const double DURATION = 1.0;

        const bsl::string MESSAGE = "1234567890";
        const bsl::size_t MSGLEN  = MESSAGE.length();

        bsls::Stopwatch    timer;
        bsl::ostringstream oss;
        timer.start();
        heartbeat(oss, MESSAGE, RATE, DURATION - EPSILON);
        timer.stop();

        double       elapsed = timer.elapsedTime();

        // The elapsed time should not be off by more than 15 ms.

        LOOP_ASSERT(elapsed,
                         1.0 - EPSILON <= elapsed && elapsed <= 1.0 + EPSILON);

        // The number of executed events should not be off by more than one.

        LOOP_ASSERT(oss.str().length(),
                               RATE * DURATION * MSGLEN == oss.str().length());

        if (veryVerbose) {
            P_(RATE);    P(RATE * DURATION * (double) MESSAGE.length());
            P_(elapsed); P(oss.str().length());
        }
      }  break;
      case 6: {
        // --------------------------------------------------------------------
        // CONCERN: MULTI-THREADED TEST (LAG)
        //
        // Concerns:
        //   - That the turnstile lags when multiple threads call 'waitTurn' at
        //   lower than the configured rate.
        //
        // Plan:
        //   Create a 'bdlmtt::Turnstile', 'mX', with a rate of 50.  Create
        //   multiple threads bound to a callback that calls 'waitTurn',
        //   increments an atomic counter, and then sleeps.  Start the threads.
        //   Join the threads, and verify that the value of the counter is
        //   less than the expected number of turns.
        //
        // Testing:
        //   Concern: Multi-Threaded Test (Lag)
        // --------------------------------------------------------------------

        if (verbose) {
            cout << "Concern: Multi-Threaded Test (Lag)" << endl
                 << "==================================" << endl;
        }

        const bsls::TimeInterval OFFSET(1.0);        // turnstile start offset
        const double            RATE        = 50.0;
        const int               NUM_TURNS   = 50;
        const int               NUM_THREADS = 3;

        bdlt::Datetime stopTime(bdlt::CurrentTime::utc());
        stopTime.addMilliseconds(2 * OFFSET.totalMilliseconds());

        Obj        mX(RATE, OFFSET);
        const Obj& X = mX;

        bsl::vector<bdlmtt::ThreadUtil::Handle> handles;
        bsls::AtomicInt                        counter;
        bsls::TimeInterval                     sleepInterval(0.075);  // 75ms
        bdlmtt::Barrier                         barrier(NUM_THREADS + 1);
        handles.reserve(NUM_THREADS);

        ASSERT(10 < sleepInterval.totalMilliseconds());
            // Guarantee that the threads will sleep

        for (int i = 0; i < NUM_THREADS; ++i) {
            bdlmtt::ThreadUtil::Handle handle;
            ASSERT(0 == bdlmtt::ThreadUtil::create(&handle,
                                                 bdlf::BindUtil::bind(
                                                     &waitTurnAndSleepCallback,
                                                     &mX,
                                                     &counter,
                                                     &barrier,
                                                     sleepInterval,
                                                     stopTime)));
            handles.push_back(handle);
        }

        barrier.wait();

        // Cleanup threads
        for (int i = 0; i < (int) handles.size(); ++i) {
            ASSERT(0 == bdlmtt::ThreadUtil::join(handles[i]));
        }

        ASSERT(0         < X.lagTime());
        ASSERT(NUM_TURNS > counter);
        if (NUM_TURNS <= counter) {
            P_(NUM_TURNS); P(counter);
        }
        if (veryVerbose) {
            P_(sleepInterval); P_(NUM_TURNS); P(counter);
        }
      }  break;
      case 5: {
        // --------------------------------------------------------------------
        // CONCERN: MULTI-THREADED TEST (NO LAG)
        //
        // Concerns:
        //   - That calling 'waitTurn' from multiple threads is thread-safe.
        //
        //   - That the turnstile does not lag when multiple threads call
        //     'waitTurn' at the configured rate.
        //
        // Plan:
        //   Create a 'bdlmtt::Turnstile', 'mX', with a rate of 50.  Create
        //   multiple threads bound to a callback that calls 'waitTurn', and
        //   then increments an atomic counter.  Start the threads.  Join the
        //   threads, and verify that the value of the counter is at least the
        //   expected number of turns.
        //
        // Testing:
        //   Concern: Multi-Threaded Test (No Lag)
        // --------------------------------------------------------------------

        if (verbose) {
            cout << "Concern: Multi-Threaded Test (No Lag)" << endl
                 << "=====================================" << endl;
        }

        const bsls::TimeInterval OFFSET(1.0);        // turnstile start offset
        const double            RATE        = 8;
        const int               NUM_TURNS   = 8;
        const int               NUM_THREADS = 3;

        bdlt::Datetime stopTime(bdlt::CurrentTime::utc());
        stopTime.addMilliseconds(2 * OFFSET.totalMilliseconds());

        Obj        mX(RATE, OFFSET);
        const Obj& X = mX;

        bsl::vector<bdlmtt::ThreadUtil::Handle> handles;
        bsls::AtomicInt                        counter;
        bdlmtt::Barrier                         barrier(NUM_THREADS + 1);
        handles.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i) {
            bdlmtt::ThreadUtil::Handle handle;
            ASSERT(0 == bdlmtt::ThreadUtil::create(&handle,
                                                 bdlf::BindUtil::bind(
                                                     &waitTurnCallback,
                                                     &mX,
                                                     &counter,
                                                     &barrier,
                                                     stopTime)));
            handles.push_back(handle);
        }

        barrier.wait();

        // Cleanup threads
        for (int i = 0; i < (int) handles.size(); ++i) {
            ASSERT(0 == bdlmtt::ThreadUtil::join(handles[i]));
        }

        Int64 lt = X.lagTime();
        LOOP_ASSERT(lt, 0 == lt);
        LOOP_ASSERT(counter, NUM_TURNS <= counter);
      }  break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING ALTERNATE CONSTRUCTORS
        //
        // Concerns:
        //   - That calling 'waitTurn' after constructing a turnstile with
        //     a specified start time blocks until that time.
        //
        // Plan:
        //   Create a 'bdlmtt::Turnstile', 'mX', with a rate of 1, and a start
        //   time offset of 1 second; and a non-modifiable reference to 'mX'
        //   named 'X'.  Call 'waitTurn' on 'mX' and 'lagTime' on 'X'.  Verify
        //   that the result of both calls is positive, indicating that the
        //   caller is not lagging, and that some wait time is incurred.
        //   Verify that the wait time is within 10ms of the expected maximum
        //   wait time.
        //
        // Testing:
        //   bdlmtt::Turnstile(
        //       double                   rate,
        //       const bsls::TimeInterval& startTime = bsls::TimeInterval(0));
        // --------------------------------------------------------------------

        if (verbose) {
            cout << "Testing Alternate Constructors" << endl
                 << "==============================" << endl;
        }

        const double            RATE = 1.0;
        const bsls::TimeInterval OFFSET(1.0);  // turnstile start offset (1 sec)

        const double WT   = 1.0 / RATE;    // max wait time for each turn
        const Int64  WTUB =
           static_cast<Int64>(USPS * (WT + EPSILON));  // upper bound wait time
        const Int64  WTLB =
           static_cast<Int64>(USPS * (WT - EPSILON));  // lower bound wait time

        Obj        mX(RATE, OFFSET);
        const Obj& X = mX;

        Int64 lt =  X.lagTime();
        Int64 wt = mX.waitTurn();
        ASSERT(0 == lt);  // not lagging since start time is offset
        ASSERT(0 <  wt);  // first turn cannot be taken immediately
        ASSERT(WTLB <= wt && wt <= WTUB);

        if (veryVerbose) {
            P_(WTLB); P_(WTUB); P_(wt); P(lt);
        }

      }  break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING FUNCTION 'reset'
        //
        // Concerns:
        //   - That the next turn may be taken immediately after calling
        //     'reset'.
        //
        //   - That 'reset' does not interrupt threads blocked on 'waitTurn'.
        //
        //   - That calling 'waitTurn' after resetting a turnstile with
        //     a specified start time blocks until that time.
        //
        // Plan:
        //   Create a 'bdlmtt::Turnstile', 'mX', with a rate of 1, and a
        //   non-modifiable reference to 'mX' named 'X'.  Call 'waitTurn' on
        //   'mX' and 'lagTime' on 'X' to establish that the caller waits on
        //   the turnstile and that the caller is not lagging.  Then, call
        //   'reset' with the same rate.  Call 'waitTurn' on 'mX', and verify
        //   that the result is 0.  Sleep for twice the maximum wait time, and
        //   call 'waitTurn' and 'lagTime' again to establish that the caller
        //   is lagging.  Then, call 'reset' again with the same rate.  Verify
        //   that the result of 'waitTurn' is again 0.  Finally, call 'reset'
        //   with the same rate and an offset of 1 second.  Call 'waitTime' on
        //   'mX' and 'lagTime' on 'X'.  Verify that 'waitTime' returns a
        //   positive value, and that the caller is not lagging.
        //
        // Testing:
        //   void reset(
        //      double                   rate,
        //      const bsls::TimeInterval& startTime = bsls::TimeInterval(0));
        // --------------------------------------------------------------------

        if (verbose) {
            cout << "Testing Function 'reset'" << endl
                 << "========================" << endl;
        }

        const double            RATE = 1.0;
        const bsls::TimeInterval OFFSET(1.0);  // turnstile reset offset (1 sec)

        Obj        mX(RATE);
        const Obj& X = mX;

        ASSERT(0 <=  X.lagTime());   // (likely) lagging on the first turn
        ASSERT(0 == mX.waitTurn());  // first turn can be taken immediately
        ASSERT(0 ==  X.lagTime());   // not lagging
        ASSERT(0 <  mX.waitTurn());  // second turn incurs wait
        ASSERT(0 ==  X.lagTime());   // not lagging

        mX.reset(RATE);
        ASSERT(0 <=  X.lagTime());   // (likely) lagging after 'reset'
        ASSERT(0 == mX.waitTurn());  // first turn can be taken immediately
        ASSERT(0 ==  X.lagTime());   // not lagging
        ASSERT(0 <  mX.waitTurn());  // second turn incurs wait
        ASSERT(0 ==  X.lagTime());   // not lagging

        bdlmtt::ThreadUtil::microSleep(3 * USPS);
        ASSERT(0 == mX.waitTurn());
        ASSERT(0 <   X.lagTime());   // lagging after sleep

        mX.reset(RATE);
        ASSERT(0 <=  X.lagTime());   // (likely) lagging after 'reset'
        ASSERT(0 == mX.waitTurn());  // first turn can be taken immediately
        ASSERT(0 ==  X.lagTime());   // not lagging
        ASSERT(0 <  mX.waitTurn());  // second turn incurs wait
        ASSERT(0 ==  X.lagTime());   // not lagging

        mX.reset(RATE, OFFSET);
        ASSERT(0 ==  X.lagTime());   // not lagging after 'reset'
        ASSERT(0 <  mX.waitTurn());  // first turn incurs wait
        ASSERT(0 ==  X.lagTime());   // not lagging
        ASSERT(0 <  mX.waitTurn());  // second turn incurs wait
        ASSERT(0 ==  X.lagTime());   // not lagging
      }  break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING FUNCTION 'waitTurn'
        //
        // Concerns:
        //   - That 'waitTurn' returns a positive value that is within a small
        //     epsilon of the expected wait time when waiting is required.
        //
        //   - That 'lagTime' returns 0 when turns are taken at or above the
        //     specified rate.
        //
        //   - That 'waitTurn' returns 0 when turns are taken more slowly than
        //     the specified rate.
        //
        //   - That 'lagTime' returns a positive value when turns are taken
        //     more slowly than the specified rate.
        //
        // Plan:
        //   Create a 'bdlmtt::Turnstile', 'mX', with a rate of 10.0, and a
        //   non-modifiable reference to 'mX' named 'X'.  In a loop, call
        //   'waitTurn' on 'mX' and 'lagTime' on 'X'.  Verify that the result
        //   of 'waitTime' is within 10ms of the expected maximum wait time,
        //   and that the result of 'lagTime' is 0.  Then, sleep for
        //   twice the maximum wait time, and very that the result of
        //   'waitTime' is 0, and the result of 'lagTime' is positive.
        //
        // Testing:
        //   bsls::Types::Int64 waitTurn();
        //   bsls::Types::Int64 lagTime() const;
        // --------------------------------------------------------------------

        if (verbose) {
            cout << "Testing Function 'waitTurn'" << endl
                 << "===========================" << endl;
        }

        // Turns are taken at or above the specified rate

        const double RATE    = 5.0;
        const double WT      = 1.0 / RATE;  // max wait time for each turn
        const Int64  WTUB    = USPS * (WT + EPSILON);  // upper bound wait time
        const Int64  WTLB    = USPS * (WT - EPSILON);  // lower bound wait time

        if (verbose) {
            P_(RATE)   P_(WT)  P_(WTUB)  P(WTLB);
        }

        Obj        mX(RATE);
        const Obj& X = mX;

        int numTurns = RATE;

        ASSERT(0 <=  X.lagTime());   // (likely) lagging on the first turn
        ASSERT(0 == mX.waitTurn());  // first turn can be taken immediately

        // First, burn a turn.  This incurs a wait, but a smaller wait than the
        // maximum (since some processing has already been done).  After taking
        // two turns, subsequent calls to 'waitTurn' should incur (close to)
        // the maximum wait time.
        mX.waitTurn();
        do {
            Int64 wt = mX.waitTurn();
            Int64 lt =  X.lagTime();
            LOOP3_ASSERT(WTLB, WTUB, wt, WTLB <= wt && wt <= WTUB);
            ASSERT(0 == lt);
            if (veryVerbose) {
                P_(WTLB); P_(WTUB); P_(wt); P(lt);
            }
        } while (--numTurns);

        // Turns are taken more slowly than the specified rate

        // 'X.lagTime()' does not report negative values, but suppose it did.
        // Call 'epsA' the amount of time we've spent doing stuff since the
        // last 'waitTurn'.  Then 'X.lagTime() == - USPS * WT + epsA' at this
        // point.

        // Wait 2.25 times the period time

        int sleepTime = (int) (2.5 * USPS * WT);
        bdlmtt::ThreadUtil::microSleep(sleepTime);

        // At this point.  'X.lagTime() == 1.5 * USPS * WT + epsA'.  Take one
        // turn.

        ASSERT(0 == mX.waitTurn());

        // Now, 'X.lagTime() == 0.5 * USPS * WT + epsA'.  Note that we can't
        // rely on system clocks having a small enough resolution to notice
        // 'epsA'.  Verify that lagTime is positive.

        int lagTime = X.lagTime();
        LOOP_ASSERT(lagTime, 0 < lagTime);

        if (verbose) { P_(sleepTime) P(lagTime); }
      }  break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //
        // Concerns:
        //   Exercise the basic functionality of the 'bdlmtt::Turnstile' class.
        //   Ensure that a turnstile can be instantiated and destroyed.  Also
        //   exercise the primary manipulators and accessors.
        //
        // Plan:
        //   Create a 'bdlmtt::Turnstile', 'mX', with a rate of 1, and a
        //   non-modifiable reference to 'mX' named 'X'.  Call 'waitTurn' on
        //   'mX' and 'lagTime' on 'X'.  Destroy 'mX'.
        //
        // Testing:
        //   Exercise basic functionality
        // --------------------------------------------------------------------

        if (verbose) {
            cout << "BREATHING TEST" << endl
                 << "==============" << endl;
        }

        const double RATE = 1.0;

        Obj        mX(RATE);
        const Obj& X = mX;

        ASSERT(0 <=  X.lagTime());   // (likely) lagging on the first turn
        ASSERT(0 == mX.waitTurn());  // first turn can be taken immediately

      }  break;
      case -1: {
        // --------------------------------------------------------------------
        // COMPARISON OF SLEEP TO TURNSTILE
        //
        // Concerns:
        //   Compare the difference in elapsed time between 'sleep' and
        //   'bdlmtt::Turnstile' implementations of the heartbeat usage example
        //   at various rates.
        //
        // Plan:
        //   Call two functions, 'processWithTurnstile' and 'processWithSleep',
        //   with various rates and durations.  Compare the elapsed time
        //   returned by these functions to the expected elapsed time.  Verify
        //   that the drift resulting from calling "sleep" in the
        //   'processWithSleep' function results in a greater difference from
        //   the expected time that the elapsed time returned by the
        //   'bdlmtt::Turnstile' implementation.
        //
        // Testing:
        //   Comparison of 'sleep' to 'bdlmtt::Turnstile'
        // --------------------------------------------------------------------

        if (verbose) {
            cout << "Comparison of 'sleep' to 'bdlmtt::Turnstile'" << endl
                 << "==========================================" << endl;
        }

        const struct {
            int    d_line;        // test line number
            double d_rate;        // number of turns per second
            double d_duration;    // duration of test in seconds
        } DATA[] = {
            //Line  Rate  Duration
            //----  ----  --------
            { L_,      2,        1, },
            { L_,      4,        1, },
            { L_,      8,        1, },
            { L_,     10,        1, },
            { L_,     10,        2, },
            { L_,     10,        4, },
            { L_,     10,        8, },
            { L_,    100,        1, },
            { L_,    100,        2, },
            { L_,    100,        4, },
            { L_,    100,        8, },
            { L_,    500,        1, },
            { L_,    500,        2, },
            { L_,    500,        4, },
            { L_,    500,        8, },
            { L_,   1000,        1, },
            { L_,   1000,        2, },
            { L_,   1000,        4, },
            { L_,   1000,        8, },
        };
        enum { DATA_SIZE = sizeof DATA / sizeof *DATA };

        for (int i = 0; i < DATA_SIZE; ++i) {
            int    LINE     = DATA[i].d_line;
            double RATE     = DATA[i].d_rate;
            double DURATION = DATA[i].d_duration;

            double elapsed1 = 0.0;
            double elapsed2 = 0.0;

            processorWithTurnstile(RATE, DURATION, &elapsed1);
            processorWithSleep    (RATE, DURATION, &elapsed2);

            bsl::printf("Line = %d, Rate = %f, Duration = %f\n"
                        "    elapsed1 = %.6f\n"
                        "    elapsed2 = %.6f\n\n"
                        , LINE, RATE, DURATION
                        , elapsed1, elapsed2);

            LOOP3_ASSERT(LINE, RATE, DURATION, elapsed1 <= elapsed2);
        }
      }  break;
      case -2: {
        // --------------------------------------------------------------------
        // SYSTEM CLOCK CHANGES DO NOT CAUSE HANGS
        //
        // Concerns:
        //: 1 Before recent changes to this component, a system clock change
        //:   could cause the 'bdlmtt::Turnstile' to appear to hang.
        //
        // Plan:
        //: 1 Create a 'bdlmtt::Turnstile' which prints a message once a second,
        //:   externally change the time, externally verify the messages do
        //:   not stop printing.
        //
        // Testing:
        //   'bdlmtt::Turnstile' behavior is immune to system clock changes.
        // --------------------------------------------------------------------

        if (verbose) {
            cout << "SYSTEM CLOCK CHANGES DO NOT CAUSE HANGS" << endl
                 << "=======================================" << endl;
        }

        cout << endl
             << endl << "Modify your current system time."
             << endl
             << endl << "A message will be printed every second and if the"
             << endl << "messages stop printing before iteration 300 while"
             << endl << "you are changing the system time there is an issue."
             << endl << "You may use CNTRL-C to exit this test at any time."
             << endl << "The test will begin in three seconds; do not change"
             << endl << "the system time until after you see the first printed"
             << endl << "message."
             << endl;

        bdlmtt::ThreadUtil::microSleep(3 * USPS);

        bdlmtt::Turnstile turnstile(1.0);
        unsigned count = 0;
        while (count < 300) {
            turnstile.waitTurn();
            ++count;
            cout << "turnstile iteration: " << count << endl;
        }
      }  break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "."
             << endl;
    }
    return testStatus;
}

// ----------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2007, 2008
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ------------------------------- END-OF-FILE --------------------------------
