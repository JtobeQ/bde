// bslmf_selecttrait.h                                                -*-C++-*-
#ifndef INCLUDED_BSLMF_SELECTTRAIT
#define INCLUDED_BSLMF_SELECTTRAIT

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide clean compile-time dispatch based on multiple traits
//
//@CLASSES:
// bslmf::SelectTrait        Template that selects the first of multiple traits
// bslmf::SelectTraitCase    Template that identifies a selected trait
// bslmf::SelectTraitDefault Class that identifies no traits
//
//@SEE_ALSO:
//
//@DESCRIPTION:
//
///Usage
///-----
// This section illustrates the intended usage of this component.
//
///Example 1: Dispatch on traits
/// - - - - - - - - - - - - - -
// We would like to create a function template,
// 'ScalarPrimitives::copyConstruct', that takes an original object and an
// allocator constructs a copy of 'original' using the most efficient valid
// mechanism.  The function should take into account that the original type
// might be bitwise copyable, or have an allocator that can be different in
// the copy than in the original object, or that the original might be a pair
// type, where the correct method of copying 'first' and 'second' is
// (recursively) governed by the same concerns.
//
// The old (legacy) 'bsls::HasTrait' mechanism has a clumsy mechanism for
// dispatching on multiple traits at once.  For example, the
// 'bslalg::scalarprimitives::copyConstruct', function uses four different
// implementations, depending on the traits of the object being copied.  The
// existing code looks like this:
//..
//  template <class TARGET_TYPE>
//  inline
//  void
//  ScalarPrimitives::copyConstruct(TARGET_TYPE        *address,
//                                  const TARGET_TYPE&  original,
//                                  bslma::Allocator   *allocator)
//  {
//      BSLS_ASSERT_SAFE(address);
//
//      enum {
//          VALUE = HasTrait<TARGET_TYPE,
//                                  TypeTraitUsesBslmaAllocator
//                                 >::VALUE ? Imp::USES_BSLMA_ALLOCATOR_TRAITS
//                : HasTrait<TARGET_TYPE,
//                                  TypeTraitBitwiseCopyable
//                                 >::VALUE ? Imp::BITWISE_COPYABLE_TRAITS
//                : HasTrait<TARGET_TYPE,
//                                  TypeTraitPair
//                                 >::VALUE ? Imp::PAIR_TRAITS
//                : Imp::NIL_TRAITS
//      };
//      Imp::copyConstruct(address, original, allocator,
//                         (bsl::integral_constant<int, VALUE>*)0);
//  }
//..
// We would like to replace the cumbersome chain of '?:' operations with a
// clean mechanism for producing one of four different types based on the
// first matching trait.
//
// First, we create three traits metafunctions to replace the three legacy
// traits used above:
//..
//  template <class TYPE> struct UsesBslmaAllocator : bsl::false_type { };
//  template <class TYPE> struct IsBitwiseCopyable  : bsl::false_type { };
//  template <class TYPE> struct IsPair             : bsl::false_type { };
//..
// Note that these definitions are simplified to avoid excess dependencies; A
// proper traits definition would inherit from 'bslmf::DetectNestedTrait'
// instead of from 'bsl::false_type'.
//
// Next, we forward-declare 'bslma::Allocator' and
// 'bslalg::scalarprimitives::copyConstruct':
//..
//  namespace bslma { class Allocator; }
//
//  namespace bslalg {
//  struct ScalarPrimitives {
//      template <class TARGET_TYPE>
//      static void copyConstruct(TARGET_TYPE        *address,
//                                const TARGET_TYPE&  original,
//                                bslma::Allocator   *allocator);
//  };
//..
// Next, we implement three overloads of 'Imp::copyConstruct', each taking a
// different trait specialization. A fourth overload takes 'false_type'
// instead of a trait specialization, for those types that don't match any
// traits.  For testing purposes, in addition to copying the data member, each
// overload also increments a separate counter.  These implementations are
// slightly simplified for readability:
//..
//  struct Imp {
//
//      // Counters for counting overload calls
//      static int d_noTraitsCounter;
//      static int d_usesBslmaAllocatorCounter;
//      static int d_isPairCounter;
//      static int d_isBitwiseCopyableCounter;
//
//      static void clearCounters() {
//          d_noTraitsCounter = 0;
//          d_usesBslmaAllocatorCounter = 0;
//          d_isPairCounter = 0;
//          d_isBitwiseCopyableCounter = 0;
//      }
//
//      template <class TARGET_TYPE>
//      static void
//      copyConstruct(TARGET_TYPE                                 *address,
//                    const TARGET_TYPE&                           original,
//                    bslma::Allocator                            *allocator,
//                    bslmf::SelectTraitCase<UsesBslmaAllocator>)
//      {
//          new (address) TARGET_TYPE(original, allocator);
//          ++d_usesBslmaAllocatorCounter;
//      }
//
//      template <class TARGET_TYPE>
//      static void
//      copyConstruct(TARGET_TYPE                 *address,
//                    const TARGET_TYPE&           original,
//                    bslma::Allocator            *allocator,
//                    bslmf::SelectTraitCase<IsPair>)
//      {
//          ScalarPrimitives::copyConstruct(&address->first, original.first,
//                                          allocator);
//          ScalarPrimitives::copyConstruct(&address->second, original.second,
//                                          allocator);
//          ++d_isPairCounter;
//      }
//
//      template <class TARGET_TYPE>
//      static void
//      copyConstruct(TARGET_TYPE                             *address,
//                    const TARGET_TYPE&                       original,
//                    bslma::Allocator                        *,
//                    bslmf::SelectTraitCase<IsBitwiseCopyable>)
//      {
//          std::memcpy(address, &original, sizeof(original));
//          ++d_isBitwiseCopyableCounter;
//      }
//
//      template <class TARGET_TYPE>
//      static void
//      copyConstruct(TARGET_TYPE                *address,
//                    const TARGET_TYPE&          original,
//                    bslma::Allocator           *,
//                    bslmf::SelectTraitCase<>)
//      {
//          new (address) TARGET_TYPE(original);
//          ++d_noTraitsCounter;
//      }
//  };
//
//  int Imp::d_noTraitsCounter = 0;
//  int Imp::d_usesBslmaAllocatorCounter = 0;
//  int Imp::d_isPairCounter = 0;
//  int Imp::d_isBitwiseCopyableCounter = 0;
//..
// Then, we implement 'ScalarPrimitives::copyConstruct':
//..
//  template <class TARGET_TYPE>
//  inline void
//  ScalarPrimitives::copyConstruct(TARGET_TYPE        *address,
//                                  const TARGET_TYPE&  original,
//                                  bslma::Allocator   *allocator)
//  {
//..
// We use 'bslmf::SelectTrait' to declare 'Selection' as a specialization
// of the first match of the specified traits:
//..
//      typedef typename bslmf::SelectTrait<TARGET_TYPE,
//                                          UsesBslmaAllocator,
//                                          IsBitwiseCopyable,
//                                          IsPair>::Type Selection;
//..
// Now, we use 'Selection' to choose (at compile time), one of the
// 'Imp::copyConstruct' overloads defined above:
//..
//      Imp::copyConstruct(address, original, allocator, Selection());
//  } // end copyConstruct()
//
//  } // Close namespace bslalg
//..
// Finally, we define three classes, associated with each of the three traits
// of interest, a fourth class associated with more than one trait (to show
// that the selection mechanism respects preference) and a fifth class that is
// not associated with any trait.
//
// The first class is associated with the 'UsesBslmaAllocator' trait:
//..
//  class TypeWithAllocator {
//      int               d_value;
//      bslma::Allocator *d_alloc;
//  public:
//      TypeWithAllocator(int v = 0, bslma::Allocator *a = 0)       // IMPLICIT
//          : d_value(v), d_alloc(a) { }
//      TypeWithAllocator(const TypeWithAllocator& other,
//                        bslma::Allocator *a = 0)
//          : d_value(other.d_value), d_alloc(a) {  }
//
//      int value() const { return d_value; }
//      bslma::Allocator *allocator() const { return d_alloc; }
//  };
//
//  template <> struct UsesBslmaAllocator<TypeWithAllocator>
//      : bsl::true_type { };
//..
// The second class is associated with the 'IsBitwiseCopyable' trait:
//..
//  class BitwiseCopyableType {
//      int d_value;
//  public:
//      BitwiseCopyableType(int v = 0) : d_value(v) { }             // IMPLICIT
//      int value() const { return d_value; }
//  };
//
//  template <> struct IsBitwiseCopyable<BitwiseCopyableType>
//      : bsl::true_type { };
//..
// The third class is associated with the 'IsPair' trait:
//..
//  struct PairType {
//      TypeWithAllocator   first;
//      BitwiseCopyableType second;
//
//      PairType(int a, int b) : first(a), second(b) { }
//  };
//
//  template <> struct IsPair<PairType> : bsl::true_type { };
//..
// The fourth class is associated with both the the 'IsPair' and
// 'IsBitwiseCopyable' traits:
//..
//  struct BitwiseCopyablePairType {
//      BitwiseCopyableType first;
//      BitwiseCopyableType second;
//
//      BitwiseCopyablePairType(int a, int b) : first(a), second(b) { }
//  };
//
//  template <> struct IsPair<BitwiseCopyablePairType> : bsl::true_type { };
//  template <> struct IsBitwiseCopyable<BitwiseCopyablePairType>
//      : bsl::true_type { };
//..
// The fifth class is not associated with any explicit traits:
//..
//  class TypeWithNoTraits {
//      int d_value;
//  public:
//      TypeWithNoTraits(int v = 0) : d_value(v) { }                // IMPLICIT
//      int value() const { return d_value; }
//  };
//..
// We use these classes to instantiate 'ScalarPrimitives::copyConstruct' and
// verify that the most efficient copy operation that is valid for each type
// is applied:
//..
//  int main()
//  {
//      using bslalg::Imp;
//
//      // This buffer is properly aligned and big enough to hold any of the
//      // test types.
//      void *buffer[4];
//      char dummy[2];  // Dummy addresses
//
//      bslma::Allocator *a1 = (bslma::Allocator*) &dummy[0];
//      bslma::Allocator *a2 = (bslma::Allocator*) &dummy[1];
//..
// When we call 'ScalarPrimitives::copyConstruct' for an object of
// 'TypeWithAllocator', we expect that the copy will have the same value but a
// different allocator than the original and that the
// 'UsesBslmaAllocator' copy implementation will be called once:
//..
//      Imp::clearCounters();
//      TypeWithAllocator  twa(1, a1);
//      TypeWithAllocator *twaptr = (TypeWithAllocator*) buffer;
//      bslalg::ScalarPrimitives::copyConstruct(twaptr, twa, a2);
//      assert(1 == Imp::d_usesBslmaAllocatorCounter);
//      assert(1 == twaptr->value());
//      assert(a2 == twaptr->allocator());
//      twaptr->~TypeWithAllocator();
//..
// When we call 'ScalarPrimitives::copyConstruct' for an object of
// 'BitwiseCopyableType', we expect that the 'IsBitwiseCopyable' copy
// implementation will be called once:
//..
//      Imp::clearCounters();
//      BitwiseCopyableType  bct(2);
//      BitwiseCopyableType *bctptr = (BitwiseCopyableType*) buffer;
//      bslalg::ScalarPrimitives::copyConstruct(bctptr, bct, a2);
//      assert(1 == Imp::d_isBitwiseCopyableCounter);
//      assert(2 == bctptr->value());
//      bctptr->~BitwiseCopyableType();
//..
// When we call 'ScalarPrimitives::copyConstruct' for an object of
// 'PairType', we expect that the 'IsPair' copy implementation will be
// called once for the pair as whole and that the
// 'UsesBslmaAllocator' and 'IsBitwiseCopyable' implementations
// will be called for the 'first' and 'second' members, respectively:
//..
//      Imp::clearCounters();
//      PairType  pt(3, 4);
//      PairType *ptptr = (PairType*) buffer;
//      bslalg::ScalarPrimitives::copyConstruct(ptptr, pt, a2);
//      assert(1 == Imp::d_isPairCounter);
//      assert(1 == Imp::d_usesBslmaAllocatorCounter);
//      assert(1 == Imp::d_usesBslmaAllocatorCounter);
//      assert(3 == ptptr->first.value());
//      assert(a2 == ptptr->first.allocator());
//      assert(4 == ptptr->second.value());
//      ptptr->~PairType();
//..
// When we call 'ScalarPrimitives::copyConstruct' for an object of
// 'BitwiseCopyablePairType', the 'IsBitwiseCopyable' trait takes precedence
// over the 'IsPair' trait (because it appears first in the list of traits
// used to instantiate 'SelectTrait').  Therefore, we expect to see the
// 'IsBitwiseCopyable' copy implementation called once for the whole
// pair and the 'IsPair' copy implementation not called at all:
//..
//      Imp::clearCounters();
//      BitwiseCopyablePairType  bcpt(5, 6);
//      BitwiseCopyablePairType *bcptbcptr = (BitwiseCopyablePairType*) buffer;
//      bslalg::ScalarPrimitives::copyConstruct(bcptbcptr, bcpt, a2);
//      // Prefer IsBitwiseCopyable over IsPair trait
//      assert(1 == Imp::d_isBitwiseCopyableCounter);
//      assert(0 == Imp::d_isPairCounter);
//      assert(5 == bcptbcptr->first.value());
//      assert(6 == bcptbcptr->second.value());
//      bcptbcptr->~BitwiseCopyablePairType();
//..
// When we call 'ScalarPrimitives::copyConstruct' for an object of
// 'TypeWithNoTraits', we expect none of the specialized copy implementations
// to be called, thus defaulting to the 'false_type' copy implementation:
//..
//      Imp::clearCounters();
//      TypeWithNoTraits  twnt(7);
//      TypeWithNoTraits *twntptr = (TypeWithNoTraits*) buffer;
//      bslalg::ScalarPrimitives::copyConstruct(twntptr, twnt, a2);
//      assert(1 == Imp::d_noTraitsCounter);
//      assert(7 == twntptr->value());
//      twntptr->~TypeWithNoTraits();
//
//      return 0;
//  }
//..
// Note that using 'SelectTraits' for dispatching using overloading imposes
// little or no overhead, since the compiler typically generates no code for
// the constructor or copy constructor of the 'SelectTraitCase' argument to
// the overloaded functions.  When inlining is in effect, the result is very
// efficient.

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLMF_INTEGRALCONSTANT
#include <bslmf_integralconstant.h>
#endif

#ifndef INCLUDED_BSLMF_SWITCH
#include <bslmf_switch.h>
#endif

namespace BloombergLP {

namespace bslmf {

                        // ========================
                        // struct SelectTrait_False
                        // ========================

template <class>
struct SelectTrait_False : bsl::false_type
{
    // Metafunction that always returns false.
};

                           // ======================
                           // struct SelectTraitCase
                           // ======================

template <template <class> class TRAIT = SelectTrait_False>
struct SelectTraitCase
{
    // This template expresses a class that is unique for the specified
    // (template parameter) 'TRAIT' metafunction.  An instantiation of this
    // template is the "compile-time return value" of 'SelectTrait' (see
    // below).  'SelectTraitCase' acts as a sort of compile-time
    // pointer-to-metafunction that holds the identity of a metafunction
    // similar to the way a pointer-to-function holds (at run-time) the
    // identity of a function.  As in the pointer-to-function case, a
    // 'SelectTraitCase' can also be used indirectly to evaluate 'TRAIT' (at
    // compile time).  Also note that, when 'SelectTraitCase' is specialized
    // with the default 'TRAIT' type parameter, 'SelectTrait_False', it
    // essentially means that none of the traits specified to 'SelectTrait'
    // match.

    template <class TYPE> struct Eval : public TRAIT<TYPE>::type {
        // Evaluates 'TRAIT' for the specified (template parameter) 'T' type.
        // The resulting 'Eval<T>' instantiation is derived from 'true_type'
        // if 'TRAIT<T>' is derived from 'true_type' and 'false_type' if
        // 'TRAIT<T>' is derived from 'false_type'.  (More generally,
        // 'Eval<T>' is derived from 'TRAIT<T>::type'.)
    };

    typedef SelectTraitCase Type;
};

                        // ======================
                        // struct SelectTrait_Imp
                        // ======================


template <class TYPE,
          template <class> class TRAIT1,
          template <class> class TRAIT2,
          template <class> class TRAIT3,
          template <class> class TRAIT4,
          template <class> class TRAIT5,
          template <class> class TRAIT6,
          template <class> class TRAIT7,
          template <class> class TRAIT8,
          template <class> class TRAIT9>
struct SelectTrait_Imp
{
    enum { ORDINAL = (TRAIT1<TYPE>::value ? 1 :
                      TRAIT2<TYPE>::value ? 2 :
                      TRAIT3<TYPE>::value ? 3 :
                      TRAIT4<TYPE>::value ? 4 :
                      TRAIT5<TYPE>::value ? 5 :
                      TRAIT6<TYPE>::value ? 6 :
                      TRAIT7<TYPE>::value ? 7 :
                      TRAIT8<TYPE>::value ? 8 :
                      TRAIT9<TYPE>::value ? 9 : 0) };

    typedef typename Switch<ORDINAL, SelectTraitCase<>,
                            SelectTraitCase<TRAIT1>,
                            SelectTraitCase<TRAIT2>,
                            SelectTraitCase<TRAIT3>,
                            SelectTraitCase<TRAIT4>,
                            SelectTraitCase<TRAIT5>,
                            SelectTraitCase<TRAIT6>,
                            SelectTraitCase<TRAIT7>,
                            SelectTraitCase<TRAIT8>,
                            SelectTraitCase<TRAIT9> >::Type Type;
};

                        // ==================
                        // struct SelectTrait
                        // ==================

template <class TYPE,
          template <class> class TRAIT1,
          template <class> class TRAIT2 = SelectTrait_False,
          template <class> class TRAIT3 = SelectTrait_False,
          template <class> class TRAIT4 = SelectTrait_False,
          template <class> class TRAIT5 = SelectTrait_False,
          template <class> class TRAIT6 = SelectTrait_False,
          template <class> class TRAIT7 = SelectTrait_False,
          template <class> class TRAIT8 = SelectTrait_False,
          template <class> class TRAIT9 = SelectTrait_False>
struct SelectTrait : SelectTrait_Imp<TYPE,   TRAIT1, TRAIT2, TRAIT3, TRAIT4,
                                     TRAIT5, TRAIT6, TRAIT7, TRAIT8, TRAIT9
                                    >::Type
{
    // Instantiate each specified (template parameter) 'TRAIT1' to 'TRAIT9'
    // metafunction using the specified (template parameter) 'TYPE'.  Inherit
    // from 'SelectTraitCase<TRAITx>', where *x* is '1' if
    // 'TRAIT1<TYPE>::value' is true, '2' if 'TRAIT2<TYPE>::value' is true,
    // etc..  If none of the traits evaluates to true, then inherit from
    // 'SelectTraitCase<>', which means that none of the traits match.

public:
    enum {
        ORDINAL = SelectTrait_Imp<TYPE,   TRAIT1, TRAIT2, TRAIT3, TRAIT4,
                                  TRAIT5, TRAIT6, TRAIT7, TRAIT8, TRAIT9
                                 >::ORDINAL
    };

    typedef bsl::integral_constant<int, ORDINAL> OrdinalType;
};

}  // close package namespace

}  // close enterprise namespace

#endif // ! defined(INCLUDED_BSLMF_SELECTTRAIT)

// ----------------------------------------------------------------------------
// Copyright (C) 2013 Bloomberg Finance L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
