////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author or Addison-Welsey Longman make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// Last update: May 19, 2002

#ifdef _MSC_VER
#pragma once

#pragma warning( push )
 
 // 'class1' : base-class 'class2' is already a base-class of 'class3'
#pragma warning( disable : 4584 )

#endif // _MSC_VER

#ifndef HIERARCHYGENERATORS_INC_
#define HIERARCHYGENERATORS_INC_

#include <AidLib/Loki/Typelist.h>
#include <AidLib/Loki/TypeTraits.h>
#include <AidLib/Loki/EmptyType.h>

namespace Loki
{
////////////////////////////////////////////////////////////////////////////////
// class template GenScatterHierarchy
// Generates a scattered hierarchy starting from a typelist and a template
// Invocation (TList is a typelist, Model is a template of one arg):
// GenScatterHierarchy<TList, Model>
// The generated class inherits all classes generated by instantiating the 
// template 'Model' with the types contained in TList 
////////////////////////////////////////////////////////////////////////////////


    template <typename T, template <typename> class Unit>
    class GenScatterHierarchy;

    namespace Private
    {
    // for some reason VC7 needs the base definition altough not in use
    template <typename TListTag> 
    struct GenScatterHierarchyHelper1
    {
        template <typename T, template <typename> class Unit>
        struct In 
        { 
            typedef typename T::ERROR_THIS_INSTANCE_SELECTED Result; 
        };
    };

    template <typename TListTag> 
    struct GenScatterHierarchyHelper2
    {
        template <typename T, template <typename> class Unit>
        struct In 
        { 
            typedef typename T::ERROR_THIS_INSTANCE_SELECTED Result; 
        };
    };

    
    template <> 
    struct GenScatterHierarchyHelper1<TL::Typelist_tag>
    {
        template <class TList, template <typename> class Unit>
        struct In
        {
            typedef GenScatterHierarchy<typename TList::Head, Unit> Result;
        };
    };

    template <> 
    struct GenScatterHierarchyHelper2<TL::Typelist_tag>
    {
        template <class TList, template <typename> class Unit>
        struct In
        {
            typedef GenScatterHierarchy<typename TList::Tail, Unit> Result;
        };
    };
    

    template <> 
    struct GenScatterHierarchyHelper1<TL::NoneList_tag>
    {
        template <typename AtomicType, template <typename> class Unit>
        struct In { typedef Unit<AtomicType> Result; };
    };

    template <> 
    struct GenScatterHierarchyHelper2<TL::NoneList_tag>
    {
        template <typename AtomicType, template <typename> class Unit>
        struct In { struct Result {}; };        
    };


    template <> 
    struct GenScatterHierarchyHelper1<TL::NullType_tag>
    {
        template <class TList, template <typename> class Unit>
        struct In { struct Result {}; };        
    };

    template <> 
    struct GenScatterHierarchyHelper2<TL::NullType_tag>
    {
        template <class TList, template <typename> class Unit>
        struct In { struct Result {}; };        
    };

    } // namespace Private

    template <typename T, template <typename> class Unit>
    class GenScatterHierarchy
        : public Private::GenScatterHierarchyHelper1
          <
            typename TL::is_Typelist<T>::type_tag
          >
          ::template In<T, Unit>::Result

        , public Private::GenScatterHierarchyHelper2
          <
            typename TL::is_Typelist<T>::type_tag
          >
          ::template In<T, Unit>::Result
    {
    public:

        typedef typename Private::GenScatterHierarchyHelper1
        <
            typename TL::is_Typelist<T>::type_tag
        >
        ::template In<T, Unit>::Result LeftBase;

        typedef typename Private::GenScatterHierarchyHelper2
        <
            typename TL::is_Typelist<T>::type_tag
        >
        ::template In<T, Unit>::Result RightBase;
    
    
        typedef typename Select
        <
            TL::is_Typelist<T>::value, T, void
        >
        ::Result TList;

        template <typename T> struct Rebind
        {
            typedef Unit<T> Result;
        };
    };

     
////////////////////////////////////////////////////////////////////////////////
// function template Field
// Accesses a field in an object of a type generated with GenScatterHierarchy
// Invocation (obj is an object of a type H generated with GenScatterHierarchy,
//     T is a type in the typelist used to generate H):
// Field<T>(obj)
// returns a reference to Unit<T>, where Unit is the template used to generate H 
////////////////////////////////////////////////////////////////////////////////

    template <class T, class H>
    typename H::template Rebind<T>::Result &
    Field(H& obj)
    {
        return obj;
    }
     
    template <class T, class H>
    typename H::template Rebind<T>::Result const &
    Field(H const & obj)
    {
        return obj;
    }

////////////////////////////////////////////////////////////////////////////////
// function template TupleUnit
// The building block of tuples 
////////////////////////////////////////////////////////////////////////////////

    template <class T>
    struct TupleUnit
    {
        T value_;
        operator T&() { return value_; }
        operator const T&() const { return value_; }
    };

////////////////////////////////////////////////////////////////////////////////
// class template Tuple
// Implements a tuple class that holds a number of values and provides field 
//     access to them via the Field function (below) 
////////////////////////////////////////////////////////////////////////////////

    template <class TList>
    struct Tuple : public GenScatterHierarchy<TList, TupleUnit>
    {
    };


////////////////////////////////////////////////////////////////////////////////
// helper class template FieldHelper
// See Field below
////////////////////////////////////////////////////////////////////////////////
    
    template <unsigned int i>
    struct FieldHelper
    {            
        template<class H>
        struct In
        {
        private:
            typedef typename TL::TypeAt<typename H::TList, i>::Result ElementType;
            typedef typename H::template Rebind<ElementType>::Result UnitType;
            
            enum { isConst = TypeTraits<H>::isConst };

            typedef typename Select
            <
                isConst,
                const typename H::RightBase,
                typename H::RightBase
            > 
            ::Result RightBase;

            typedef typename Select
            <
                IsSameType<UnitType, TupleUnit<ElementType> >::value, 
                ElementType, 
                UnitType
            >
            ::Result UnqualifiedResultType;

        public:
            typedef typename Select
            <
                isConst,
                const UnqualifiedResultType,
                UnqualifiedResultType
            >
            ::Result ResultType;
            
            static ResultType& Do(H& obj)
            {
                RightBase& rightBase = obj;
                return FieldHelper<i - 1>::template In<RightBase>::Do(rightBase);
            }
        };
    };

    template <>
    struct FieldHelper<0>
    {            
        template<class H>
        struct In
        {
        private:
            typedef typename H::TList::Head ElementType;
            typedef typename H::template Rebind<ElementType>::Result UnitType;
            
            enum { isConst = TypeTraits<H>::isConst };

            typedef typename Select
            <
                isConst,
                const typename H::LeftBase,
                typename H::LeftBase
            > 
            ::Result LeftBase;

            typedef typename Select
            <
                IsSameType<UnitType, TupleUnit<ElementType> >::value, 
                ElementType, 
                UnitType
            >
            ::Result UnqualifiedResultType;

        public:
            typedef typename Select
            <
                isConst,
                const UnqualifiedResultType,
                UnqualifiedResultType
            >
            ::Result ResultType;
            
            static ResultType& Do(H& obj)
            {
                LeftBase& leftBase = obj;
                return leftBase;
            }
        };
    };

////////////////////////////////////////////////////////////////////////////////
// function template Field
// Accesses a field in an object of a type generated with GenScatterHierarchy
// Invocation (obj is an object of a type H generated with GenScatterHierarchy,
//     i is the index of a type in the typelist used to generate H):
// Field<i>(obj)
// returns a reference to Unit<T>, where Unit is the template used to generate H
//     and T is the i-th type in the typelist 
////////////////////////////////////////////////////////////////////////////////

    template <unsigned int i, class H>
    typename FieldHelper<i>::template In<H>::ResultType&
    Field(H& obj)
    {
        return FieldHelper<i>::template In<H>::Do(obj);
    }
                
////////////////////////////////////////////////////////////////////////////////
// class template GenLinearHierarchy
// Generates a linear hierarchy starting from a typelist and a template
// Invocation (TList is a typelist, Model is a template of two args):
// GenLinearHierarchy<TList, Model, Root>
////////////////////////////////////////////////////////////////////////////////
    
    template
    <
        class TList,
        template <class AtomicType, class UnitBase> class Unit,
        class Root = EmptyType
    >
    class GenLinearHierarchy;

    namespace Private
    {
    
    template <typename TListTag> 
    struct GenLinearHierarchyHelper
    {
        template<class TList, template <class, class> class Unit, class Root>
        struct In 
        {
            typedef typename TList::ERROR_THIS_INSTANCE_SELECTED Result; 
        };
    };
    
    template <> 
    struct GenLinearHierarchyHelper<TL::Typelist_tag>
    {
        template<class TList, template <class, class> class Unit, class Root>
        struct In 
        {
        private:
            typedef typename TList::Head Head;
            typedef typename TList::Tail Tail;

        public:
            typedef Unit< Head, GenLinearHierarchy<Tail, Unit, Root> > Result; 
        };
    };

    template <> 
    struct GenLinearHierarchyHelper<TL::NullType_tag>
    {
        template<class TList, template <class, class> class Unit, class Root>
        struct In
        {
        private:
            typedef typename TList::Head Head;

        public:
            typedef Unit<Head, Root> Result;
        };
    };

    } // namespace Private

    template
    <
        class TList,
        template <class AtomicType, class UnitBase> class Unit,
        class Root
    >
    class GenLinearHierarchy
        : public Private::GenLinearHierarchyHelper
          <
            typename TL::is_Typelist<typename TList::Tail>::type_tag
          >
          ::template In<TList, Unit, Root>::Result
    {
        ASSERT_TYPELIST(TList); // TList must not be NullType

    public:
        typedef typename Private::GenLinearHierarchyHelper
        <
            typename TL::is_Typelist<typename TList::Tail>::type_tag
        >
        ::template In<TList, Unit, Root>::Result LinBase;
    };

}   // namespace Loki

////////////////////////////////////////////////////////////////////////////////
// Change log:
// June 20, 2001: ported by Nick Thurn to gcc 2.95.3. Kudos, Nick!!!
// May  10, 2002: ported by Rani Sharoni to VC7 (RTM - 9466)
////////////////////////////////////////////////////////////////////////////////

#endif // HIERARCHYGENERATORS_INC_

#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER

