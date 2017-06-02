// ball_log.h                                                         -*-C++-*-

// ----------------------------------------------------------------------------
//                                   NOTICE
//
// This component is not up to date with current BDE coding standards, and
// should not be used as an example for new development.
// ----------------------------------------------------------------------------

#ifndef INCLUDED_BALL_LOG
#define INCLUDED_BALL_LOG

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide macros and utility functions to facilitate logging.
//
//@CLASSES:
//  ball::Log: namespace for logging utilities (for *internal* use only)
//
//@SEE_ALSO: ball_loggermanager, ball_category, ball_severity, ball_record
//
//@AUTHOR: Hong Shi (hshi2)
//
//@DESCRIPTION: This component provides preprocessor macros and utility
// functions to facilitate use of the 'ball_loggermanager' component.  In
// particular, the macros defined herein greatly simplify the mechanics of
// generating log records.  The utility functions provided in 'ball::Log' are
// intended only for use by the macros and should *not* be called directly.
//
// The macros defined herein pertain to the logger manager singleton only, and
// not to any non-singleton instances of 'ball::LoggerManager'.  In particular,
// the macros do not have any effect unless the logger manager singleton is
// initialized.  Note that the flow of control may pass through a use of any of
// the macros *before* the logger manager singleton has been initialized or
// *after* it has been destroyed; however, control should not pass through any
// macro use *during* logger manager singleton initialization or destruction.
// See {Logger Manager Singleton Initialization} in the 'ball_loggermanager'
// component-level documentation for details on the recommended procedure for
// initializing the singleton.
//
///Thread Safety
///-------------
// All macros defined in this component are thread-safe, and can be invoked
// concurrently by multiple threads.
//
///Macro Reference
///---------------
// This section documents the preprocessor macros defined in this component.
//
// The first three macros described below are used to define categories, at
// either block scope or class scope, to which records are logged by the C++
// stream-based and 'printf'-style logging macros (described further below).
//
///Macros for Defining Categories at Block Scope
///- - - - - - - - - - - - - - - - - - - - - - -
// The following two macros are used to establish logging categories that have
// block scope:
//
//: 'BALL_LOG_SET_CATEGORY(CATEGORY)':
//:     Set a category for logging to the specified 'CATEGORY' (assumed to be
//:     of type convertible to 'const char *').  On the *first* invocation of
//:     this macro in a code block, the 'ball::Log::setCategory' method is
//:     invoked to retrieve the address of an appropriate category structure
//:     for its scope; subsequent invocations will use a cached address of the
//:     category.  (See the function-level documentation of
//:     'ball::Log::setCategory' for more information.)  This macro must be
//:     used at block scope, and can be used at most once in any given block
//:     (or else a compiler diagnostic will result).
//:
//: 'BALL_LOG_SET_DYNAMIC_CATEGORY(CATEGORY)':
//:     Set, *on* *EACH* *invocation*, a category for logging to the specified
//:     'CATEGORY' (assumed to be of type convertible to 'const char *').  On
//:     *EVERY* invocation of this macro in a code block, the
//:     'ball::Log::setCategory' method is invoked to retrieve the address of
//:     an appropriate category structure for its scope; the address returned
//:     from 'ball::Log::setCategory' is *NOT* cached for subsequent calls.
//:     (See the function-level documentation of 'ball::Log::setCategory' for
//:     more information.)  This macro must be used at block scope and can be
//:     used at most once in any given block (or else a compiler diagnostic
//:     will result).  Note that this macro should be used to create categories
//:     that depend on *RUN-TIME* values only (e.g., LUW or UUID).
//
// There can be at most one use of either 'BALL_LOG_SET_CATEGORY' or
// 'BALL_LOG_SET_DYNAMIC_CATEGORY' in any given block (or else a compiler
// diagnostic will result).  Note that categories that are set using these
// macros, including dynamic categories, are not destroyed until the logger
// manager singleton is destroyed.
//
///Macro for Defining Categories at Class Scope
/// - - - - - - - - - - - - - - - - - - - - - -
// The following macro is used to establish logging categories that have class
// scope:
//
//: 'BALL_LOG_SET_CLASS_CATEGORY(CATEGORY)':
//:     Set a category for logging to the specified 'CATEGORY' (assumed to be
//:     of type convertible to 'const char *') in the scope of the class within
//:     which this macro is used.  Similar to 'BALL_LOG_SET_CATEGORY', the
//:     category is set *once* only, the first time that it is accessed (i.e.,
//:     it is not a dynamic category).  This macro must be used, at most once,
//:     within the definition of a class or class template (or else a compiler
//:     diagnostic will result).  Uses of this macro must not be terminated by
//:     a ';'.  Note that use of this macro may occur in either a 'public',
//:     'private', or 'protected' section of a class's interface, although
//:     'private' should be preferred.
//
// Note that similar to block-scope categories (see 'BALL_LOG_SET_CATEGORY' and
// 'BALL_LOG_SET_DYNAMIC_CATEGORY'), class-scope categories are not destroyed
// until the logger manager singleton is destroyed.
//
///Macros for Logging Records
/// - - - - - - - - - - - - -
// The macros defined in this subsection are the ones that are actually used to
// produce log records.  A use of any one of the logging macros requires that a
// logging category (as established by the macros defined above) be in scope at
// the point where the macro is used.  Note that the formatted string that is
// generated for the message attribute of each log record includes the category
// that is in scope and the filename as established by the standard '__FILE__'
// macro.
//
// A set of macros based on C++ streams, 'BALL_LOG_TRACE', 'BALL_LOG_DEBUG',
// 'BALL_LOG_INFO', 'BALL_LOG_WARN', 'BALL_LOG_ERROR', 'BALL_LOG_FATAL', and
// 'BALL_LOG_END', are the ones most commonly used for logging.  They have the
// following usage pattern:
//..
//  BALL_LOG_TRACE << X << Y ... << BALL_LOG_END
//  BALL_LOG_DEBUG << X << Y ... << BALL_LOG_END
//  BALL_LOG_INFO  << X << Y ... << BALL_LOG_END
//  BALL_LOG_WARN  << X << Y ... << BALL_LOG_END
//  BALL_LOG_ERROR << X << Y ... << BALL_LOG_END
//  BALL_LOG_FATAL << X << Y ... << BALL_LOG_END
//      where X, Y, ... represents any sequence of values for which
//      'operator<<' is defined.  The resulting formatted message string is
//      logged with the severity indicated by the name of the initial macro
//      (e.g., 'BALL_LOG_TRACE' ... 'BALL_LOG_END' logs with severity
//      'ball::Severity::e_TRACE').
//..
// Two closely-related macros also based on C++ streams, 'BALL_LOG_STREAM' and
// 'BALL_LOG_STREAM_UNLIKELY', require that the severity be explicitly supplied
// as an argument:
//..
//  BALL_LOG_STREAM(severity)          << X << Y ... << BALL_LOG_END
//  BALL_LOG_STREAM_UNLIKELY(severity) << X << Y ... << BALL_LOG_END
//      where X, Y, ... represents any sequence of values for which
//      'operator<<' is defined.  The resulting formatted message string is
//      logged with the specified 'severity'.  'BALL_LOG_STREAM_UNLIKELY'
//      differs from 'BALL_LOG_STREAM' in that the former provides a hint to
//      the compiler that it is "unlikely" to be executed, and is intended for
//      special-purpose use only.  Note that 'BALL_LOG_TRACE', for example, is
//      equivalent to 'BALL_LOG_STREAM_UNLIKELY(ball::Severity::e_TRACE)'.
//..
// Another set of macros based on C++ streams, similar to 'BALL_LOG_TRACE',
// etc., allow the caller to specify a "callback" function that is passed the
// 'ball::UserFields *' used to represent the user fields of a log record.
// 'BALL_LOGCB_TRACE', 'BALL_LOGCB_DEBUG', 'BALL_LOGCB_INFO',
// 'BALL_LOGCB_WARN', 'BALL_LOGCB_ERROR', 'BALL_LOGCB_FATAL', and
// 'BALL_LOGCB_END' have the following usage pattern:
//..
//  BALL_LOGCB_TRACE(CB) << X << Y ... << BALL_LOGCB_END
//  BALL_LOGCB_DEBUG(CB) << X << Y ... << BALL_LOGCB_END
//  BALL_LOGCB_INFO(CB)  << X << Y ... << BALL_LOGCB_END
//  BALL_LOGCB_WARN(CB)  << X << Y ... << BALL_LOGCB_END
//  BALL_LOGCB_ERROR(CB) << X << Y ... << BALL_LOGCB_END
//  BALL_LOGCB_FATAL(CB) << X << Y ... << BALL_LOGCB_END
//      where X, Y, ... represents any sequence of values for which
//      'operator<<' is defined and 'CB' is a callback taking a
//      'ball::UserFields *' as an argument.  The resulting formatted message
//      string is logged with the severity indicated by the name of the
//      initial macro (e.g., 'BALL_LOGCB_ERROR' ... 'BALL_LOGCB_END' logs with
//      severity 'ball::Severity::e_ERROR').  The generated log record will
//      contain the 'ball::UserFields' representing user fields as populated by
//      'CB'.  Note that the callback supplied to the logging macro must match
//      the prototype 'void (*)(ball::UserFields *)'.
//..
// The remaining macros are based on 'printf'-style format specifications:
//..
//  BALL_LOGVA_TRACE(MSG, ...);
//  BALL_LOGVA_DEBUG(MSG, ...);
//  BALL_LOGVA_INFO( MSG, ...);
//  BALL_LOGVA_WARN( MSG, ...);
//  BALL_LOGVA_ERROR(MSG, ...);
//  BALL_LOGVA_FATAL(MSG, ...);
//      Format the specified '...' optional arguments, if any, according to the
//      'printf'-style format specification in the specified 'MSG' (assumed to
//      be of type convertible to 'const char *') and log the resulting
//      formatted message string with the severity indicated by the name of the
//      macro (e.g., 'BALL_LOGVA_INFO' logs with severity
//      'ball::Severity::e_INFO').  The behavior is undefined unless the number
//      and types of optional arguments are compatible with the format
//      specification in 'MSG'.  Note that each use of these macros must be
//      terminated by a ';'.
//..
//
///Utility Macros
/// - - - - - - -
// The following utility macro is intended for special-purpose use for
// fine-tuning logging behavior.  A use of this macro requires that a logging
// category (as established by the macros defined above) be in scope at the
// point where the macro is used.
//..
//  BALL_LOG_IS_ENABLED(BALL_SEVERITY)
//      Return 'true' if the specified 'BALL_SEVERITY' is at least as severe as
//      any of the threshold levels of the logging category that is in scope,
//      and 'false' otherwise.
//..
//
///Usage
///-----
// The following code fragments illustrate the standard pattern of macro usage.
//
///Example 1: A Basic Logging Example
/// - - - - - - - - - - - - - - - - -
// The following trivial example shows how to use the logging macros to log
// messages at various levels of severity.
//..
//  void exampleFunction()
//  {
//..
// We start by initializing the log category within the context of this
// function.  The logging macros such as 'BALL_LOG_ERROR' will not compile
// unless a category has been specified in the current lexical scope.
//..
//      BALL_LOG_SET_CATEGORY("EXAMPLE.CATEGORY");
//..
// Now we record messages at various levels of severity.  These messages will
// (or will not) be written to the log depending on the current logging
// threshold of the category (configured using the 'ball::LoggerManager'
// singleton).
//..
//      BALL_LOG_FATAL << "Write this message to the log if the log threshold "
//                     << "is above 'ball::Severity::FATAL' (i.e., 32)."
//                     << BALL_LOG_END
//
//      BALL_LOG_TRACE << "Write this message to the log if the log threshold "
//                     << "is above 'ball::Severity::FATAL' (i.e., 192)."
//                     << BALL_LOG_END
//  }
//..
// Note that failing to match the start of a logged message with an
// appropriate 'BALL_LOG_END' will result in a compilation error.
//
///Example 2: Setting the Current Log Category
///- - - - - - - - - - - - - - - - - - - - - -
// This example provides more detail on setting the log category in the
// current lexical scope.  The following macro instantiation sets the category
// for logging to be "EQUITY.NASD" in the enclosing lexical scope:
//..
//  BALL_LOG_SET_CATEGORY("EQUITY.NASD")
//..
// Note that this macro must not be used at file scope and it can be used
// at most once in any given block (or else a compiler diagnostic will result).
// A different category may be established to override one that is in effect,
// but it must occur in a nested scope.  In any case, a use of this macro (or
// of 'BALL_LOG_SET_DYNAMIC_CATEGORY') must be visible from within the lexical
// scope of every use of the log-generating macros.  The following fragment of
// code shows how to set a different category in a nested inner block that
// hides a category set in an enclosing block:
//..
//  void logIt()
//  {
//      BALL_LOG_SET_CATEGORY("EQUITY.NASD")
//
//      // Logging to category "EQUITY.NASD" unless overridden in a nested
//      // block.
//      // [*] ...
//
//      {
//          // [*] ...
//          // Still logging to category "EQUITY.NASD".
//
//          BALL_LOG_SET_CATEGORY("EQUITY.NASD.SUNW")
//
//          // Now logging to category "EQUITY.NASD.SUNW".
//          // [*] ...
//      }
//      // Again logging to category "EQUITY.NASD".
//      // [*] ...
//  }
//..
// Within 'logIt', a requisite logging category is visible at each of the
// locations marked by '[*]'.
//
///Example 3: C++ I/O Streams-Style Logging Macros
///- - - - - - - - - - - - - - - - - - - - - - - -
// The preferred logging method we use, the 'iostream'-style macros such as
// 'BALL_LOG_INFO', allow streaming via the 'bsl::ostream' 'class' and the C++
// stream operator '<<'.  An advantage the C++ streaming style has over the
// 'printf' style output (shown below in example 4) is that complex types often
// have the 'operator<<(ostream&, const TYPE&)' function overloaded so that
// they are able to be easily streamed to output.  We demonstrate this here
// using C++ streaming to stream a 'bdlt::Date' to output:
//..
//  int         lotSize = 400;
//  const char *ticker  = "SUNW";
//  double      price   = 5.65;
//
//  // Trading on a market that settles 3 days in the future.
//
//  bdlt::Date settle = bdlt::CurrentTime::local().date() + 3;
//
//  BALL_LOG_SET_CATEGORY("EQUITY.NASD")
//..
// We are logging with category "EQUITY.NASD", which is configured for a
// pass-through level of 'e_INFO', from here on.  We output a line using the
// 'BALL_LOG_INFO' macro:
//..
//  BALL_LOG_INFO << "[1] " << lotSize << " shares of " << ticker <<
//                               " sold at " << price << " settlement date " <<
//                                                       settle << BALL_LOG_END
//..
// The above results in the following single-line message being output:
//..
//  <ts> <pid> <tid> INFO x.cpp 1161 EQUITY.NASD [1] 400 shares of SUNW sold
//  at 5.65 settlement date 17FEB2017
//..
// '<ts>' is the timestamp, '<pid>' is the process id, '<tid>' is the thread
// id, 'x.cpp' is the expansion of the '__FILE__' macro that is the name of the
// source file containing the call, 1161 is the line number of the call, and
// the trailing date following "settlement date" is the value of 'settle'.
//
// Next, we set the category to "EQUITY.NASD.SUNW", which has been defined with
// 'ball::Administration::addCategory' with its pass-through level set to
// 'e_INFO' and the trigger levels set at or above 'e_ERROR', so a level of
// 'e_WARN' also passes through:
//..
//  {
//      BALL_LOG_SET_CATEGORY("EQUITY.NASD.SUNW")
//
//      // Now logging with category "EQUITY.NASD.SUNW".
//
//      BALL_LOG_WARN << "[2] " << lotSize << " shares of " << ticker <<
//                               " sold at " << price << " settlement date " <<
//                                                       settle << BALL_LOG_END
//  }
//..
// The above results in the following message to category "EQUITY.NASD.SUNW":
//..
//  <ts> <pid> <tid> WARN x.cpp 1185 EQUITY.NASD.SUNW [2] 400 shares of SUNW
//  sold at 5.65 settlement date 17FEB2017
//..
// Now, the category "EQUITY.NASD.SUNW" just went out of scope and category
// "EQUITY.NASD" is visible again, so it applies to the following:
//..
//  BALL_LOG_INFO << "[3] " << lotSize << " shares of " << ticker <<
//                               " sold at " << price << " settlement date " <<
//                                                       settle << BALL_LOG_END
//..
// Finally, the above results in the following single-line message being
// output:
//..
//  <ts> <pid> <tid> INFO x.cpp 1198 EQUITY.NASD [3] 400 shares of SUNW sold
//  at 5.65 settlement date 17FEB2017
//..
// The settlement date was appended to the message as a simple illustration of
// the added flexibility provided by the C++ stream-based macros.  This last
// message was logged to category "EQUITY.NASD" at severity level
// 'ball::Severity::e_INFO'.
//
// The C++ stream-based macros, as opposed to the 'printf'-style macros, ensure
// at compile-time that no run-time format mismatches will occur.  Use of the
// stream-based logging style exclusively will likely lead to clearer, more
// maintainable code with fewer initial defects.
//
// Note that all uses of the log-generating macros, both 'printf'-style and C++
// stream-based, *must* occur within function scope (i.e., not at file scope).
//
///Example 4: 'printf'-Style Output
/// - - - - - - - - - - - - - - - -
// In the following example, we expand the 'logIt' function (defined above)
// to log two messages using the 'BALL_LOGVA_INFO' logging macro provided by
// this component.  This variadic macro takes a format string and a
// variable-length series of arguments, similar to 'printf'.
//..
//  int         lotSize = 400;
//  const char *ticker  = "SUNW";
//  double      price   = 5.65;
//
//  // Trading on a market that settles 3 days in the future.
//
//  bdlt::Date settleDate = bdlt::CurrentTime::local().date() + 3;
//..
// Because we can't easily 'printf' complex types like 'bdlt::Date' or
// 'bsl::string', we have to convert 'settleDate' to a 'const char *'
// ourselves.  Note that all this additional work was unnecessary in Example 3
// when we used the C++ 'iostream'-style, rather than the 'printf'-style,
// macros.
//..
//  bsl::ostringstream  settleOss;
//  settleOss << settleDate;
//  const bsl::string&  settleStr = settleOss.str();
//  const char         *settle    = settleStr.c_str();
//..
// We set logging with category "EQUITY.NASD", which was configured for a
// pass-through severity level of 'e_INFO', and call 'BALL_LOGVA_INFO' to print
// our trade:
//..
//  BALL_LOG_SET_CATEGORY("EQUITY.NASD")
//
//  BALL_LOGVA_INFO("[4] %d shares of %s sold at %f settlement date %s\n",
//                  lotSize, ticker, price, settle);
//..
// The above results in the following single-line message being output to
// category "EQUITY.NASD.SUNW" at severity level 'ball::Severity::e_INFO':
//..
//  <ts> <pid> <tid> INFO x.cpp 1256 EQUITY.NASD [4] 400 shares of SUNW sold
//  at 5.650000 settlement date 17FEB2017
//..
// In the above, '<ts>' is the timestamp, '<pid>' is the process id, '<tid>' is
// the thread id, 'x.cpp' is the expansion of the '__FILE__' macro that is the
// name of the source file containing the call, and 1256 is the line number of
// the call.
//
// Note that the first argument supplied to the 'BALL_LOGVA_INFO' macro is a
// 'printf'-style format specification.
//
// Next, we set the category to "EQUITY.NASD.SUNW", which is configured for a
// pass-through severity level of 'e_INFO':
//..
//  {
//      BALL_LOG_SET_CATEGORY("EQUITY.NASD.SUNW")
//
//      // Now logging with category "EQUITY.NASD.SUNW".
//
//      BALL_LOGVA_WARN("[5] %d shares of %s sold at %f settlement date %s\n",
//                      lotSize, ticker, price, settle);
//  }
//..
// The above results in the following single-line message to category
// "EQUITY.NASD.SUNW":
//..
//  <ts> <pid> <tid> WARN x.cpp 1281 EQUITY.NASD.SUNW [5] 400 shares of SUNW
//  sold at 5.650000 settlement date 17FEB2017
//..
// Now, the category "EQUITY.NASD.SUNW" just went out of scope and category
// "EQUITY.NASD" is visible again, so it applies to the following:
//..
//  BALL_LOGVA_INFO("[6] %d shares of %s sold at %f settlement date %s\n",
//                  lotSize, ticker, price, settle);
//..
// Finally, the above results in the following single-line message being
// output:
//..
//  <ts> <pid> <tid> INFO x.cpp 1294 EQUITY.NASD [6] 400 shares of SUNW sold
//  at 5.650000 settlement date 17FEB2017
//..
//
///Example 5: Dynamic Categories
///- - - - - - - - - - - - - - -
// Logging must sometimes be controlled by parameters that are not available
// until run-time.  The 'BALL_LOG_SET_DYNAMIC_CATEGORY' macro sets a category
// each time it is invoked (unlike 'BALL_LOG_SET_CATEGORY', which sets a
// category only on the first invocation and uses the cached address of the
// category on subsequent invocations).  The category name in the following
// 'processSecurity' function is a combination of a static prefix and the
// (dynamic) 'exchange' argument:
//..
//  void processSecurity(const char *security, const char *exchange)
//  {
//      bsl::string categoryName("EXCHANGE:");
//      categoryName.append(exchange);
//
//      BALL_LOG_SET_DYNAMIC_CATEGORY(categoryName.c_str());
//
//      BALL_LOG_TRACE << "processing: " << security << BALL_LOG_END
//
//      // ...
//  }
//..
// Now logging can be controlled independently for each 'exchange'.
//
// *WARNING*: Along with the added flexibility provided by dynamic categories
// comes the additional overhead of computing and setting a category on each
// invocation.  Consequently, dynamic categories should be used *SPARINGLY* in
// most applications.
//
///Example 6: Rule-Based Logging
///- - - - - - - - - - - - - - -
// The following example demonstrates the use of attributes and rules to
// conditionally enable logging.
//
// We start by defining a function, 'processData', that is passed data in a
// 'vector<char>' and information about the user who sent the data.  This
// example function performs no actual processing, but does log a single
// message at the 'ball::Severity::DEBUG' threshold level.  The 'processData'
// function also adds the user information passed to this function to the
// thread's attribute context.  We will use these attributes later, to create a
// logging rule that enables verbose logging only for a particular user.
//..
//  void processData(int                      uuid,
//                   int                      luw,
//                   int                      terminalNumber,
//                   const bsl::vector<char>& data)
//      // Process the specified 'data' associated with the specified Bloomberg
//      // 'uuid', 'luw', and 'terminalNumber'.
//  {
//      (void)data;  // suppress "unused" warning
//..
// We create a generic "default" attribute container to hold our attributes.
// Note that, in practice, we might choose to define a more efficient
// implementation of the 'ball::AttributeContainer' protocol specifically for
// these three attributes, uuid, luw, and firmNumber.  See the
// 'ball_attributecontainer' component documentation for an example.
//..
//      ball::DefaultAttributeContainer attributes;
//      attributes.addAttribute(ball::Attribute("uuid", uuid));
//      attributes.addAttribute(ball::Attribute("luw", luw));
//      attributes.addAttribute(ball::Attribute("terminalNumber",
//                                              terminalNumber);
//
//      ball::AttributeContext *context = ball::AttributeContext::getContext();
//      ball::AttributeContext::iterator it =
//                                         context->addAttributes(&attributes);
//..
// In this simplified example we perform no actual processing, and simply log
// a message at the 'ball::Severity::DEBUG' level.
//..
//      BALL_LOG_SET_CATEGORY("EXAMPLE.CATEGORY");
//
//      BALL_LOG_DEBUG << "An example message" << BALL_LOG_END
//..
// Because 'attributes' is defined on this thread's stack, it must be removed
// from this thread's attribute context before exiting the function (the
// 'ball_scopedattributes' component provides a proctor for this purpose).
//..
//      context->removeAttributes(it);
//  }
//..
// Next we demonstrate how to create a logging rule that sets the pass-through
// logging threshold to 'ball::Severity::TRACE' (i.e., enables verbose logging)
// for a particular user when calling the 'processData' function defined
// above.
//
// We start by creating the singleton logger manager that we configure with
// the stream observer and a default configuration.  We then call the
// 'processData' function: This first call to 'processData' will not result in
// any logged messages because 'processData' logs its message at the
// 'ball::Severity::DEBUG' level, which is below the default configured logging
// threshold.
//..
//  ball::StreamObserver observer(&bsl::cout);
//  ball::LoggerManagerConfiguration configuration;
//  ball::LoggerManagerScopedGuard lmg(&observer, configuration);
//
//  BALL_LOG_SET_CATEGORY("EXAMPLE.CATEGORY");
//
//  bsl::vector<char> message;
//
//  BALL_LOG_ERROR << "Processing the first message." << BALL_LOG_END
//  processData(3938908, 2, 9001, message);
//..
// Now we add a logging rule, setting the pass-through threshold to be
// 'ball::Severity::TRACE' (i.e., enabling verbose logging) if the thread's
// context contains a "uuid" of '3938908'.  Note that we use the wild-card
// value '*' for the category so that the 'ball::Rule' rule will apply to all
// categories.
//..
//  ball::Rule rule("*", 0, ball::Severity::TRACE, 0, 0);
//  rule.addPredicate(ball::Predicate("uuid", 3938908));
//  ball::LoggerManager::singleton().addRule(rule);
//
//  BALL_LOG_ERROR << "Processing the second message." << BALL_LOG_END
//  processData(3938908, 2, 9001, message);
//..
// The final call to the 'processData' function below, passes a "uuid" of
// '2171395' (not '3938908') so the logging rule we defined will *not* apply
// and no message will be logged.
//..
//  BALL_LOG_ERROR << "Processing the third message." << BALL_LOG_END
//  processData(2171395, 2, 9001, message);
//..
// The resulting logged output for this example looks like the following:
//..
// ERROR example.cpp:105 EXAMPLE.CATEGORY Processing the first message.
// ERROR example.cpp:117 EXAMPLE.CATEGORY Processing the second message.
// DEBUG example.cpp:35 EXAMPLE.CATEGORY An example message
// ERROR example.cpp:129 EXAMPLE.CATEGORY Processing the third message.
//..
//
///Example 7: Logging Using a Callback
///- - - - - - - - - - - - - - - - - -
// The following example demonstrates how to register a logging callback.  The
// C++ stream-based macros that take a callback are particularly useful to
// seamlessly populate the user fields of a record, thus simplifying the
// logging line.
//
// We define a callback function 'populateUsingPoint' that appends to the
// specified 'fields' the attributes of the 'point' to log:
//..
//  void populateUsingPoint(ball::UserFields *fields, const Point& point)
//      // Append to the specified 'list' the name, x value, and y value of
//      // the specified 'point'.
//  {
//      fields->appendString(point.name());
//      fields->appendInt64(point.x());
//      fields->appendInt64(point.y());
//  }
//
//  int validatePoint(const Point& point)
//  {
//      BALL_LOG_SET_CATEGORY("EXAMPLE.CATEGORY");
//..
// We now bind our callback function 'populateUsingPoint' and the supplied
// 'point' to a functor object we will pass to the logging callback.  Note
// that the callback supplied to the logging macro must match the prototype
// 'void (*)(ball::UserFields *)'.
//..
//      bsl::function <void(ball::UserFields *)> callback;
//      callback = bdlf::BindUtil::bind(&populateUsingPoint,
//                                      bdlf::PlaceHolders::_1,
//                                      point);
//
//      int numErrors = 0;
//      if (point.x() > 255) {
//          BALL_LOGCB_ERROR(callback) << "X > 255"  << BALL_LOGCB_END
//          ++numErrors;
//      }
//      if (point.x() < -255) {
//          BALL_LOGCB_ERROR(callback) << "X < -255" << BALL_LOGCB_END
//          ++numErrors;
//      }
//      if (point.y() > 255) {
//          BALL_LOGCB_ERROR(callback) << "Y > 255"  << BALL_LOGCB_END
//          ++numErrors;
//      }
//      if (point.y() < -255) {
//          BALL_LOGCB_ERROR(callback) << "Y < -255" << BALL_LOGCB_END
//          ++numErrors;
//      }
//      return numErrors;
//  }
//..
//
///Example 8: Class-Scope Logging
/// - - - - - - - - - - - - - - -
// The following example demonstrates how to define and use logging categories
// that have class scope.
//
// First, we define a class, 'Thing', for which we want to do class-scope
// logging.  The use of the 'BALL_LOG_SET_CLASS_CATEGORY' macro generates the
// requisite declarations within the definition of the class.  We have used the
// macro in a 'private' section of the interface, which should be preferred,
// but 'public' (or 'protected') is fine, too:
//..
//  // pckg_thing.h
//  namespace pckg {
//
//  class Thing {
//      // ...
//
//    private:
//      // CLASS-SCOPE CATEGORY
//      BALL_LOG_SET_CLASS_CATEGORY("PCKG.THING")
//
//    public:
//      // ...
//
//      // MANIPULATORS
//      void outOfLineMethodThatLogs(bool useClassCategory);
//          // Log to the class-scope category "PCKG.THING" if the specified
//          // 'useClassCategory' flag is 'true', and to the block-scope
//          // category "X.Y.Z" otherwise.
//
//      // ...
//
//      // ACCESSORS
//      void inlineMethodThatLogs() const;
//          // Log a record to the class-scope category "PCKG.THING".
//  };
//..
// Next, we define the 'inlineMethodThatLogs' method 'inline' within the header
// file and log to the class-scope category using 'BALL_LOG_TRACE'.  Since
// there is no other category in scope, the record is necessarily logged to the
// "PCKG.THING" category that is within the scope of the 'Thing' class:
//..
//  // ...
//
//  // ACCESSORS
//  inline
//  void Thing::inlineMethodThatLogs() const
//  {
//      BALL_LOG_TRACE << "log to PCKG.THING" << BALL_LOG_END
//  }
//
//  }  // close namespace pckg
//..
// Now, we define the 'outOfLineMethodThatLogs' method within the '.cpp' file.
// On each invocation, this method logs one record using 'BALL_LOG_TRACE'.  It
// logs to the "PCKG.THING" class-scope category if 'useClassCategory' is
// 'true', and logs to the "X.Y.Z" block-scope category otherwise:
//..
//  // pckg_thing.cpp
//  namespace pckg {
//
//  // ...
//
//  // MANIPULATORS
//  void Thing::outOfLineMethodThatLogs(bool useClassCategory)
//  {
//      if (useClassCategory) {
//          BALL_LOG_TRACE << "log to PCKG.THING" << BALL_LOG_END;
//      }
//      else {
//          BALL_LOG_SET_CATEGORY("X.Y.Z");
//          BALL_LOG_TRACE << "log to X.Y.Z" << BALL_LOG_END
//      }
//  }
//
//  }  // close namespace pckg
//..
// Finally, note that both block-scope and class-scope categories can be logged
// to within the same block.  For example, the following block within a 'Thing'
// method would first log to "PCKG.THING" then log to "X.Y.Z":
//..
//      {
//          BALL_LOG_TRACE << "log to PCKG.THING" << BALL_LOG_END
//
//          BALL_LOG_SET_CATEGORY("X.Y.Z");
//
//          BALL_LOG_TRACE << "log to X.Y.Z" << BALL_LOG_END
//      }
//..

#ifndef INCLUDED_BALSCM_VERSION
#include <balscm_version.h>
#endif

#ifndef INCLUDED_BALL_CATEGORY
#include <ball_category.h>
#endif

#ifndef INCLUDED_BALL_CATEGORYMANAGER
#include <ball_categorymanager.h>
#endif

#ifndef INCLUDED_BALL_LOGGERMANAGER
#include <ball_loggermanager.h>
#endif

#ifndef INCLUDED_BALL_SEVERITY
#include <ball_severity.h>
#endif

#ifndef INCLUDED_BSLS_PERFORMANCEHINT
#include <bsls_performancehint.h>
#endif

#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

#ifndef INCLUDED_BSL_OSTREAM
#include <bsl_ostream.h>
#endif

// BDE_VERIFY pragma: push
// BDE_VERIFY pragma: -AQQ01
// BDE_VERIFY pragma: -AQS01
// BDE_VERIFY pragma: -FB01
// BDE_VERIFY pragma: -KS00
// BDE_VERIFY pragma: -TR04
// BDE_VERIFY pragma: -TR17

                       // =======================
                       // CategoryHolder Accessor
                       // =======================

// HELPER FUNCTIONS
inline
BloombergLP::ball::CategoryHolder *ball_log_cAtEgOrYhOlDeR(
                             BloombergLP::ball::CategoryHolder& categoryHolder)
    // Return the address of the specified 'categoryHolder'.  Note that this
    // function facilitates consistent lookup of block-scope and class-scope
    // category holders (see "Logging Macro Reuse" in the "IMPLEMENTATION
    // NOTES" of the component implementation file for details).
{
    return &categoryHolder;
}

// BDE_VERIFY pragma: pop

                       // =========================
                       // Logging Macro Definitions
                       // =========================

#define BALL_LOG_CATEGORY                                                     \
    (ball_log_cAtEgOrYhOlDeR(BALL_LOG_CATEGORYHOLDER)->category())

#define BALL_LOG_THRESHOLD                                                    \
    (ball_log_cAtEgOrYhOlDeR(BALL_LOG_CATEGORYHOLDER)->threshold())

#define BALL_RECORD (ball_lOcAl_StReAm.record())

#define BALL_STREAM (ball_lOcAl_StReAm.stream())

                       // ===========================
                       // Block-Scope Category Macros
                       // ===========================

#define BALL_LOG_SET_CATEGORY(CATEGORY)                                       \
    static BloombergLP::ball::CategoryHolder BALL_LOG_CATEGORYHOLDER = {      \
        BloombergLP::ball::CategoryHolder::e_UNINITIALIZED_CATEGORY, 0, 0     \
    };                                                                        \
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(!BALL_LOG_CATEGORY)) {          \
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;                                   \
        BloombergLP::ball::Log::setCategory(&BALL_LOG_CATEGORYHOLDER,         \
                                            CATEGORY);                        \
    }

#define BALL_LOG_SET_DYNAMIC_CATEGORY(CATEGORY)                               \
    const BloombergLP::ball::Category *BALL_LOG_DYNAMIC_CATEGORY =            \
                             BloombergLP::ball::Log::setCategory(CATEGORY);   \
    BloombergLP::ball::CategoryHolder BALL_LOG_CATEGORYHOLDER = {             \
        BloombergLP::ball::CategoryHolder::e_DYNAMIC_CATEGORY,                \
        const_cast<BloombergLP::ball::Category *>(BALL_LOG_DYNAMIC_CATEGORY), \
        0                                                                     \
    };

                       // ==========================
                       // Class-Scope Category Macro
                       // ==========================

#define BALL_LOG_SET_CLASS_CATEGORY(CATEGORY)                                 \
    enum { BALL_LOG_CATEGORYHOLDER = 0 };                                     \
    static BloombergLP::ball::CategoryHolder *ball_log_cAtEgOrYhOlDeR(        \
                           BloombergLP::ball::CategoryHolder& categoryHolder) \
    {                                                                         \
        return &categoryHolder;                                               \
    }                                                                         \
    static BloombergLP::ball::CategoryHolder *ball_log_cAtEgOrYhOlDeR(        \
                                                                  const int&) \
    {                                                                         \
        static BloombergLP::ball::CategoryHolder holder = {                   \
            BloombergLP::ball::CategoryHolder::e_UNINITIALIZED_CATEGORY, 0, 0 \
        };                                                                    \
        if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(!holder.category())) {      \
            BSLS_PERFORMANCEHINT_UNLIKELY_HINT;                               \
            BloombergLP::ball::Log::setCategory(&holder, CATEGORY);           \
        }                                                                     \
        return &holder;                                                       \
    }

                       // =======================
                       // C++ stream-based macros
                       // =======================

#define BALL_LOG_STREAM(BALL_SEVERITY) {                                      \
    using namespace BloombergLP;                                              \
    if (BALL_LOG_THRESHOLD >= (BALL_SEVERITY)) {                              \
        if (ball::Log::isCategoryEnabled(                                     \
                            ball_log_cAtEgOrYhOlDeR(BALL_LOG_CATEGORYHOLDER), \
                            BALL_SEVERITY)) {                                 \
            ball::Log_Stream ball_lOcAl_StReAm(BALL_LOG_CATEGORY, __FILE__,   \
                                               __LINE__, BALL_SEVERITY);      \
            BALL_STREAM

#define BALL_LOG_STREAM_UNLIKELY(BALL_SEVERITY) {                             \
    using namespace BloombergLP;                                              \
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(                                \
                                 BALL_LOG_THRESHOLD >= (BALL_SEVERITY))) {    \
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;                                   \
        if (ball::Log::isCategoryEnabled(                                     \
                            ball_log_cAtEgOrYhOlDeR(BALL_LOG_CATEGORYHOLDER), \
                            BALL_SEVERITY)) {                                 \
            ball::Log_Stream ball_lOcAl_StReAm(BALL_LOG_CATEGORY, __FILE__,   \
                                               __LINE__, BALL_SEVERITY);      \
            BALL_STREAM

// We expect TRACE and DEBUG messages not to be logged.

#define BALL_LOG_TRACE BALL_LOG_STREAM_UNLIKELY(ball::Severity::e_TRACE)

#define BALL_LOG_DEBUG BALL_LOG_STREAM_UNLIKELY(ball::Severity::e_DEBUG)

#define BALL_LOG_INFO  BALL_LOG_STREAM(ball::Severity::e_INFO)

#define BALL_LOG_WARN  BALL_LOG_STREAM(ball::Severity::e_WARN)

#define BALL_LOG_ERROR BALL_LOG_STREAM(ball::Severity::e_ERROR)

#define BALL_LOG_FATAL BALL_LOG_STREAM(ball::Severity::e_FATAL)

// We indirectly define the macro for ending a 'BALL_LOG_*' block because the
// 'BALL_LOG_[LEVEL]' macros that most often begin such a block (e.g.,
// 'BALL_LOG_ERROR') indirectly forward to 'BALL_LOG_STREAM'.  This symmetry
// enables Microsoft intellisense to reasonably parse code following a use of
// this macro.

#define BALL_LOG_REAL_END bsl::ends;                                          \
        }                                                                     \
    }                                                                         \
}
#define BALL_LOG_END BALL_LOG_REAL_END

               // ========================================
               // C++ stream-based macros using a callback
               // ========================================

#define BALL_LOGCB_STREAM(BALL_SEVERITY, CB) {                                \
    using namespace BloombergLP;                                              \
    if (BALL_LOG_THRESHOLD >= (BALL_SEVERITY)) {                              \
        if (ball::Log::isCategoryEnabled(                                     \
                            ball_log_cAtEgOrYhOlDeR(BALL_LOG_CATEGORYHOLDER), \
                            BALL_SEVERITY)) {                                 \
            ball::Log_Stream ball_lOcAl_StReAm(BALL_LOG_CATEGORY, __FILE__,   \
                                               __LINE__, BALL_SEVERITY);      \
            CB(&ball_lOcAl_StReAm.record()->customFields());                  \
            BALL_STREAM

#define BALL_LOGCB_TRACE(CB) BALL_LOGCB_STREAM(ball::Severity::e_TRACE, CB)

#define BALL_LOGCB_DEBUG(CB) BALL_LOGCB_STREAM(ball::Severity::e_DEBUG, CB)

#define BALL_LOGCB_INFO(CB) BALL_LOGCB_STREAM(ball::Severity::e_INFO, CB)

#define BALL_LOGCB_WARN(CB) BALL_LOGCB_STREAM(ball::Severity::e_WARN, CB)

#define BALL_LOGCB_ERROR(CB) BALL_LOGCB_STREAM(ball::Severity::e_ERROR, CB)

#define BALL_LOGCB_FATAL(CB) BALL_LOGCB_STREAM(ball::Severity::e_FATAL, CB)

// We indirectly define the macro for ending a 'BALL_LOGCB_*' block because the
// 'BALL_LOGCB_[LEVEL]' macros that most often begin such a block (e.g.,
// 'BALL_LOGCB_ERROR') indirectly forward to 'BALL_LOGCB_STREAM'.  This
// symmetry enables Microsoft intellisense to reasonably parse code following a
// use of this macro.

#define BALL_LOGCB_REAL_END bsl::ends;                                        \
        }                                                                     \
    }                                                                         \
}
#define BALL_LOGCB_END BALL_LOGCB_REAL_END

                       // =====================
                       // 'printf'-style macros
                       // =====================

// ----------------------------------------------------------------------------
// Usage: BALL_LOGVA(severity, msg, arg1, arg2, ... argN);
//
// Log a message with the specified 'severity', formatting the specified 0 or
// more arguments 'arg1, arg2, ... argN' according to the specified
// 'printf'-style format string 'msg'.

#define BALL_LOGVA(BALL_SEVERITY, ...)                                        \
do {                                                                          \
    if (BALL_LOG_THRESHOLD >= (BALL_SEVERITY)) {                              \
        if (BloombergLP::ball::Log::isCategoryEnabled(                        \
                            ball_log_cAtEgOrYhOlDeR(BALL_LOG_CATEGORYHOLDER), \
                            BALL_SEVERITY)) {                                 \
            BloombergLP::ball::Log_Formatter ball_lOcAl_FoRmAtTeR(            \
                                                     BALL_LOG_CATEGORY,       \
                                                     __FILE__, __LINE__,      \
                                                     BALL_SEVERITY);          \
            BloombergLP::ball::Log::format(                                   \
                              ball_lOcAl_FoRmAtTeR.messageBuffer(),           \
                              ball_lOcAl_FoRmAtTeR.messageBufferLen(),        \
                              __VA_ARGS__);                                   \
        }                                                                     \
    }                                                                         \
} while(0)

// ----------------------------------------------------------------------------
// Usage: BALL_LOGVA_{SEVERITY}(msg, arg1, arg2, ... argN);
//
// Log a message with the specified 'ball::Severity::e_{SEVERITY}', formatting
// the specified 0 or more arguments 'arg1, arg2, ... argN' according to the
// specified 'printf'-style format string 'msg'.

#define BALL_LOGVA_TRACE(...)                                                 \
    BALL_LOGVA(BloombergLP::ball::Severity::e_TRACE, __VA_ARGS__)

#define BALL_LOGVA_DEBUG(...)                                                 \
    BALL_LOGVA(BloombergLP::ball::Severity::e_DEBUG, __VA_ARGS__)

#define BALL_LOGVA_INFO( ...)                                                 \
    BALL_LOGVA(BloombergLP::ball::Severity::e_INFO,  __VA_ARGS__)

#define BALL_LOGVA_WARN( ...)                                                 \
    BALL_LOGVA(BloombergLP::ball::Severity::e_WARN,  __VA_ARGS__)

#define BALL_LOGVA_ERROR(...)                                                 \
    BALL_LOGVA(BloombergLP::ball::Severity::e_ERROR, __VA_ARGS__)

#define BALL_LOGVA_FATAL(...)                                                 \
    BALL_LOGVA(BloombergLP::ball::Severity::e_FATAL, __VA_ARGS__)

                       // ==============
                       // Utility Macros
                       // ==============

#define BALL_LOG_IS_ENABLED(BALL_SEVERITY)                                    \
    ((BALL_LOG_THRESHOLD >= (BALL_SEVERITY))                                  \
     && BloombergLP::ball::Log::isCategoryEnabled(                            \
                            ball_log_cAtEgOrYhOlDeR(BALL_LOG_CATEGORYHOLDER), \
                            BALL_SEVERITY))

namespace BloombergLP {

namespace bslmt { class Mutex; }

namespace ball {

class Record;

                         // ==========
                         // struct Log
                         // ==========

struct Log {
    // This 'struct' provides a namespace for a suite of utility functions that
    // simplify usage of the 'ball_loggermanager' component.  The direct use
    // of these utility functions is *strongly* discouraged.

    // CLASS METHODS
    static int format(char        *buffer,
                      bsl::size_t  numBytes,
                      const char  *format, ...);
        // Fill the specified 'buffer' with at most the specified 'numBytes'
        // characters produced by formatting the variable argument list
        // according to the specified 'printf'-style 'format' argument; return
        // the number of characters in the resulting formatted string.  The
        // last character placed into 'buffer' is always a null terminator
        // (leaving at most 'numBytes - 1' bytes of formatted data).  If
        // 'numBytes' is insufficient for the entire formatted string, this
        // method fills 'buffer' with the initial 'numBytes - 1' bytes of
        // formatted data followed by a null terminator and returns -1.  Note
        // that with the exception of the return value, the behavior of this
        // function exactly matches that of the C99 function 'snprintf'.  Also
        // note that 'snprintf' is not part of standard C++-98, so its
        // functionality is provided here.

    static Record *getRecord(const Category *category,
                             const char     *file,
                             int             line);
        // Return the address of a modifiable record having the specified
        // 'file' and 'line' attributes.  The memory for the record will be
        // supplied by the allocator held by the logger manager singleton if
        // the specified 'category' is non-null, or by the currently installed
        // default allocator otherwise.  The behavior is undefined unless the
        // logger manager singleton is initialized when 'category' is non-null.

    static void logMessage(const Category *category,
                           int             severity,
                           const char     *fileName,
                           int             lineNumber,
                           const char     *message);
        // Log a record containing the specified 'message' text, 'fileName',
        // 'lineNumber', 'severity', and the name of the specified 'category'.
        // (See the component-level documentation of 'ball_record' for more
        // information on the additional fields that are logged.)  Store the
        // record in the buffer held by the logger if 'severity' is at least
        // as severe as the current "Record" threshold level of 'category'.
        // Pass the record directly to the registered observer if 'severity'
        // is at least as severe as the current "Pass" threshold level of
        // 'category'.  Publish the entire contents of the buffer of the
        // logger if 'severity' is at least as severe as the current "Trigger"
        // threshold level of 'category'.  Publish the entire contents of all
        // buffers of all loggers if 'severity' is at least as severe as the
        // current "Trigger-All" threshold level of 'category' (i.e., via the
        // callback supplied at construction of the logger manager singleton).
        // This method has no effect if 'category' is 0 or 'severity' is less
        // severe than each of the threshold levels of 'category'.  The
        // behavior is undefined unless 'severity' is in the range '[1 .. 255]'
        // and the logger manager singleton is initialized when 'category' is
        // non-null.

    static void logMessage(const Category *category,
                           int             severity,
                           Record         *record);
        // Log the specified 'record' after setting its category attribute to
        // the specified 'category' and its severity attribute to the specified
        // 'severity'.  (See the component-level documentation of 'ball_record'
        // for more information on the fields that are logged.)  Store the
        // record in the buffer held by the logger if 'severity' is at least
        // as severe as the current "Record" threshold level of 'category'.
        // Pass the record directly to the registered observer if 'severity'
        // is at least as severe as the current "Pass" threshold level of
        // 'category'.  Publish the entire contents of the buffer of the
        // logger if 'severity' is at least as severe as the current "Trigger"
        // threshold level of 'category'.  Publish the entire contents of all
        // buffers of all loggers if 'severity' is at least as severe as the
        // current "Trigger-All" threshold level of 'category' (i.e., via the
        // callback supplied at construction of the logger manager singleton).
        // This method has no effect if 'severity' is less severe than each of
        // the threshold levels of 'category'.  The behavior is undefined
        // unless 'severity' is in the range '[1 .. 255]', 'record' was
        // obtained by a call to 'Log::getRecord', and the logger manager
        // singleton is initialized.

    static char *obtainMessageBuffer(bslmt::Mutex **mutex, int *bufferSize);
        // Block until access to the buffer used for formatting messages in
        // this thread of execution is available.  Return the address of the
        // modifiable buffer to which this thread of execution has exclusive
        // access, load the address of the mutex that protects the buffer into
        // the specified '*mutex' address, and load the size (in bytes) of the
        // buffer into the specified 'bufferSize' address.  The address remains
        // valid, and the buffer remains locked by this thread of execution,
        // until the 'Log::releaseMessageBuffer' method is called.  The
        // behavior is undefined if this thread of execution currently holds a
        // lock on the buffer.  Note that the buffer is intended to be used
        // *only* for formatting log messages immediately before a call to
        // 'Log::logMessage'; other use may adversely affect performance for
        // the entire program.

    static void releaseMessageBuffer(bslmt::Mutex *mutex);
        // Unlock the specified '*mutex' that guards the buffer used for
        // formatting messages in this thread of execution.  The behavior is
        // undefined unless '*mutex' was obtained by a call to
        // 'Log::obtainMessageBuffer' and has not yet been unlocked.

    static const Category *setCategory(const char *categoryName);
        // Return from the logger manager singleton's category registry the
        // address of the non-modifiable category having the specified
        // 'categoryName' if such a category exists, or if a new category
        // having 'categoryName' can be added to the registry (i.e., if the
        // registry has sufficient capacity to accommodate new entries);
        // otherwise, return the address of the non-modifiable *Default*
        // *Category*.  Return 0 if the logger manager singleton is not
        // initialized.

    static void setCategory(CategoryHolder *categoryHolder,
                            const char     *categoryName);
        // Load into the specified 'categoryHolder' the address of the
        // non-modifiable category having the specified 'categoryName' if such
        // a category exists, or if a new category having 'categoryName' can
        // be added to the registry (i.e., if the registry has sufficient
        // capacity to accommodate new entries); otherwise, load the address of
        // the non-modifiable *Default* *Category*.  Also load into
        // 'categoryHolder' the maximum threshold level of the category
        // ultimately loaded into 'categoryHolder'.  This method has no effect
        // if the logger manager singleton is not initialized.

    static bool isCategoryEnabled(const CategoryHolder *categoryHolder,
                                  int                   severity);
        // Return 'true' if logging to the category associated with the
        // specified 'categoryHolder' at the specified 'severity' is enabled,
        // or if 'Severity::e_WARN >= severity' and the logger manager
        // singleton is not initialized; return 'false' otherwise.
};

                        // ================
                        // class Log_Stream
                        // ================

class Log_Stream {
    // This class provides an aggregate of several objects relevant to the
    // logging of a message via the C++ stream-based macros:
    //..
    //  - record to be logged
    //  - category to which to log the record
    //  - severity at which to log the record
    //  - stream to which the user log message is put
    //..
    // As a side-effect of creating an object of this class, the record and
    // stream are also constructed.  As a side-effect of destroying the
    // object, the record is logged.
    //
    // This class should *not* be used directly by client code.  It is an
    // implementation detail of the macros provided by this component.

    // DATA
    const Category *d_category_p;  // category to which record is logged
                                   // (held, not owned)

    Record         *d_record_p;    // logged record (held, not owned)

    const int       d_severity;    // severity at which record is logged

    bsl::ostream    d_stream;      // stream to which log message is put

  private:
    // NOT IMPLEMENTED
    Log_Stream(const Log_Stream&);
    Log_Stream& operator=(const Log_Stream&);

  public:
    // CREATORS
    Log_Stream(const Category *category,
               const char     *fileName,
               int             lineNumber,
               int             severity);
        // Create a logging stream that holds (1) the specified 'category' and
        // 'severity', (2) a record that is created from the specified
        // 'fileName' and 'lineNumber', and (3) an 'bsl::ostream' to which the
        // log message is put.

    ~Log_Stream();
        // Log the record held by this logging stream to the held category
        // (as returned by 'category') at the held severity (as returned by
        // 'severity') and destroy this logging stream.

    // MANIPULATORS
    Record *record();
        // Return the address of the modifiable log record held by this logging
        // stream.  The address is valid until this logging stream is
        // destroyed.

    bsl::ostream& stream();
        // Return a reference to the modifiable stream held by this logging
        // stream.  The reference is valid until this logging stream is
        // destroyed.

    // ACCESSORS
    const Category *category() const;
        // Return the address of the non-modifiable category held by this
        // logging stream.

    const Record *record() const;
        // Return the address of the non-modifiable log record held by this
        // logging stream.  The address is valid until this logging stream is
        // destroyed.

    int severity() const;
        // Return the severity held by this logging stream.
};

                     // ===================
                     // class Log_Formatter
                     // ===================

class Log_Formatter {
    // This class provides an aggregate of several objects relevant to the
    // logging of a message via the 'printf'-style macros:
    //..
    //  - record to be logged
    //  - category to which to log the record
    //  - severity at which to log the record
    //  - buffer in which the user log message is formatted
    //  - mutex mediating exclusive access to the buffer
    //..
    // As a side-effect of creating an object of this class, the record is
    // constructed and the mutex is locked.  As a side-effect of destroying the
    // object, the record is logged and the mutex unlocked.
    //
    // This class should *not* be used directly by client code.  It is an
    // implementation detail of the macros provided by this component.

    // DATA
    const Category *d_category_p;  // category to which record is logged
                                   // (held, not owned)

    Record         *d_record_p;    // logged record (held, not owned)

    const int       d_severity;    // severity at which record is logged

    char           *d_buffer_p;    // buffer for formatted user log message
                                   // (held, not owned)

    int             d_bufferLen;   // length of buffer

    bslmt::Mutex   *d_mutex_p;     // mutex to lock buffer (held, not owned)

  private:
    // NOT IMPLEMENTED
    Log_Formatter(const Log_Formatter&);
    Log_Formatter& operator=(const Log_Formatter&);

  public:
    // CREATORS
    Log_Formatter(const Category *category,
                  const char     *fileName,
                  int             lineNumber,
                  int             severity);
        // Create a logging formatter that holds (1) the specified 'category'
        // and 'severity', (2) a record that is created from the specified
        // 'fileName' and 'lineNumber', and (3) a buffer into which the log
        // message is formatted, and for which a lock is acquired for exclusive
        // access to the buffer.

    ~Log_Formatter();
        // Log the record held by this logging formatter to the held category
        // (as returned by 'category') at the held severity (as returned by
        // 'severity'), release the lock on the held buffer, and destroy this
        // logging formatter.

    // MANIPULATORS
    Record *record();
        // Return the address of the modifiable log record held by this logging
        // formatter.  The address is valid until this logging formatter is
        // destroyed.

    char *messageBuffer();
        // Return the address of the modifiable buffer held by this logging
        // formatter.  The address is valid until this logging formatter is
        // destroyed.

    // ACCESSORS
    const Category *category() const;
        // Return the address of the non-modifiable category held by this
        // logging formatter.

    int messageBufferLen() const;
        // Return the length (in bytes) of the buffer held by this logging
        // formatter.

    const Record *record() const;
        // Return the address of the non-modifiable log record held by this
        // logging formatter.  The address is valid until this logging
        // formatter is destroyed.

    int severity() const;
        // Return the severity held by this logging formatter.
};

// ============================================================================
//                              INLINE DEFINITIONS
// ============================================================================

                     // ----------------
                     // class Log_Stream
                     // ----------------

// MANIPULATORS
inline
Record *Log_Stream::record()
{
    return d_record_p;
}

inline
bsl::ostream& Log_Stream::stream()
{
    return d_stream;
}

// ACCESSORS
inline
const Category *Log_Stream::category() const
{
    return d_category_p;
}

inline
const Record *Log_Stream::record() const
{
    return d_record_p;
}

inline
int Log_Stream::severity() const
{
    return d_severity;
}

                     // -------------------
                     // class Log_Formatter
                     // -------------------

// MANIPULATORS
inline
Record *Log_Formatter::record()
{
    return d_record_p;
}

inline
char *Log_Formatter::messageBuffer()
{
    return d_buffer_p;
}

// ACCESSORS
inline
const Category *Log_Formatter::category() const
{
    return d_category_p;
}

inline
int Log_Formatter::messageBufferLen() const
{
    return d_bufferLen;
}

inline
const Record *Log_Formatter::record() const
{
    return d_record_p;
}

inline
int Log_Formatter::severity() const
{
    return d_severity;
}

}  // close package namespace
}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2017 Bloomberg Finance L.P.
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
