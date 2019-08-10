# introduction
In concept(though not always in practice), rvalues correspond to temporary objects returned from functions,
while lvalues correspond to objects you can refer to, either by name or by following a pointer or lvalue reference.

A useful heuristic to determine whether an expression is an lvalue is to ask if you can take its address.
If you can, it typically is. If you can't, it's usually an rvalue.

In a function call, the expressions passed at the call site are the functions's arguemnts. The parameters are used to initialize the function's parameters.
```C++
void someFunc(Widget w);
Widget wid;
someFunc(wid);
someFUnc(std::move(wid));
```
In the first call to someFunc, the argument is wid. In the second call, the argument is std::move(wid). In both calls, the parameter is w.

The distinction between arguments and parameters is important, because parameters are lvalues, but the arguemnts may be rvalues or lvalues.
This is especially relevant during the process of perfect forwarding, whereby an argument passed to a function is passed to a second function call such that
the original argument's rvalueness or lvalueness is preserved.

Well-designed functions are exception safe, meaning they offer at least the basic exception safety guarantee (i.e., the basic guarantee).
Such functions assure calless that even if an exception is thrown, program invariants remain intact (i.e., no data structures are corrupted) and no resources are leaked.
Functions offering the strong exception safety guarantee (i.e., the strong guarantee) assure callers that if an exception arises, the state of the program remains as it was priori to the call.

# deduce types
## template type deduction
If you are willing to overlook a pinch of pseudocode, we can think of a function template as looking like this:
```C++
template <typename T>
void f(ParamType param);
```
A call can look like this:
```C++
f(expr);  // call f with some expression
```
During compilation, compilers use expr to deduce two types: one for T and one for ParamType.
These types are frequently different, because ParamType often contains adornments, e.g., const or reference qualifiers.

### case 1: ParamType is a reference or pointer, but not a universal reference
In this case, type deduction works like this:

1. If expr's type is a reference, ignore the reference part.
1. Then pattern-match expr's type against ParamType to determine T.

```C++
template<typename T>
void f(T& param);  // param is a reference

int x = 27;
const int cx = x;
const int& rx = x;  // rx is reference to x as a const int

f(x);  // T is int, param's type is int&
f(cx);  // T is const int, param's type is const int&
f(rx);  // T is const int, param's type is const int&
```

```C++
template<typename T>
void f(const T& param);  // param is now a ref-to-const

int x = 27;
const int cx = x;
const int& rx = x;

f(x);  // T is int, param's type is const int&
f(cx);  // T is int, param's type is const int&
f(rx):  // T is int, param's type is const int&
```

If parm were a pointer (or a pointer to const) instread of a reference, things would work essentially the same way.
```C++
template<typename T>
void f(T* param);

int x = 27;
const int* px = &x;

f(&x);  // T is int, param's type is int*
f(px);  // T is const int, param's type is const int*
```

### case 2: ParamType is a universal reference
- If expr is an lvalue, both T and ParamType are deduced to be lvalue references. That's doubly unusual.
  1. It's the only situation in template tpe deduction where T is deduced to be a reference.
  1. Although ParamType is declared using the syntax for an rvalue reference, its deduced type is an lvalue reference.
- If expr is an rvalue, case 1 rules apply.

```C++
template<typename T>
void f(T&& param);  // param is now universal reference

int x = 27;
const int cx = x;
const int& rx = x;

f(x);  // x is lvalue, so T is int&, param's type is also int&
f(cx);  // cx is lvalue, so T is const int&, param's type is also const int&
f(rx);  // xx is lvalue, so T is const int&, param's type is also const int&
f(27);  // 27 is rvalue, so T is int, param's type is therefore int&&.
        // 27 is of type int&&, we first ignore the reference part, and we got int, so T is int, and param's type is int&&
```

### case 3: ParamType is neither a pointer nor a reference
```C++
template<typename T>
void f(T param);  // param is now passed by value
```
That means that param will be a copy of whatever is passed in - a completely new object.
The fact that param will be a new object motivates the rules that govern how T is deduced from expr.

1. As before, if expr's type is a reference, ignore the reference part.
1. If, after ignoring expr's reference-ness, expr is const, ignore that, too. If it's volatile, also ignore that.

```C++
int x = 27;
const int cx = x;
const int& rx = x;

f(x);  // T's and param's types are both int
f(cx);  // T's and param's types are again both int
f(rx);  // T's and param's types are still both int
```

It is important to recognize that const (and volatile) is ingored only for by-value-parameters.
As we've seen, for parameters that are references-to- or pointers-to-const, the constness of expr is preserved during type deduction.
```C++
const char* const ptr = "Fun with pointers";  // ptr is const pointer to const object

f(ptr)  // pass arg of type const char* const
```
Here, the right const declares ptr to be const, meaning ptr can not be made to point to a different location, nor can it be set to null.
The left const says that what ptr points to - the character string is cont, hence can not be modified.
When ptr is passed to f, the bits making up the pointer are copied into param.
In accord with the type deduction rule, the constness of ptr will be ignored, and the type deduced for param will be `const char*`, i.e., a modifiable pointer to a const character string.
The constness of what ptr points to is preserved during type deduction.

### array arguments
In many contexts, an array decays into a pointer to its first element. This decay is what permits code like this to compile:
```C++
const char name[] = "J. P. Briggs";  // name's type is const char[13]
const char* ptrToName = name;  // array decays to pointer
```
These types (`const char*` and `const char[13]`) are not the same, but because of the array-to-pointer decay rule, the code compilers.

```C++
template<typename T>
void f(T param);

f(name);  // T is deduced as const char*
```
Because array parameter declarations are treated as if they were pointer parameters, the type of the array that is passed to a template function by value is deduced to be a pointer type.
That means that in the call f(name), its type parameter T is deduct to be `const char*`.

Althouth functions can not declare parameters that are truly arrays, they can declare parameters that are references to arrays!
```C++
template<typename T>
void f(T& param);

f(name);  // T is deduced as const char[13], param's type is const char(&)[13]
```

And interestingly, the ability to declare references to arrays enables creation of a template that deduces the number of elements that an array contains:
```C++
// constexpr makes its result available during compilation.
template<typename T, std::size_t N>
constexpr std::size_t arraySize(T(&)[N]) noexcept {
  return N;
}

int keyVals[] = {1, 3, 5};
int mappedVals[arraySize(keyVals)];
```

### function arguments
Arrays are not the only things in C++ that can decay into pointers.
Function types can decay into function pointers, and everything we've discussed regarding type deduction for array applies to type deduction for functions.
```C++
void someFunc(int, double);  // someFunc's type is void(int, double)

template<typename T>
void f1(T param);

template<typename T>
void f2(T& param);

f1(someFunc);  // param deduced as ptr-to-func, type is void (*)(int, double)
f2(someFunc);  // param deduced as ref-to-func, type is void (&)(int, double)
```

## auto
With only one curious exception, auto type deduction is template type deduction.
There is a direct mapping between template type deduction and auto type deduction.
When a variable is declared using auto, auto plays the role of T in template, and type specifier for the variable acts as ParamType.
So there are also three cases.

- case 1: the type specifier is a pointer or reference, but not a universal reference.
- case 2: the type specifier is a universal reference.
- case 3: the type specifier is neither a pointer nor a reference.

```C++
auto x = 27;  // type specifier for x is auto, case 3
const auto cx = x;  // type specifier for x is const auto, case 3
const auto& rx = x;  // type specifier for x is const auto&, case 1

auto&& uref1 = x;  // x is int and lvalue, so uref1's type is int&
auto&& uref2 = cx;  // cx is const int and lvalue, so uref1's type is const int&
auto&& uref3 = 27;  // 27 is int and rvalue, so uref3's type is int&&

const char name[] = "R. N. Briggs";
auto arr1 = name;  // arr1's type is const char*
auto& arr2 = name;  // arr2's type is const char (&)[13]

void someFunc(int, double);
auto func1 = someFunc;  // func1's type is void(*)(int, double)
auto& func2 = someFunc;  // func2's type is void(&)(int, double)
```

Now let's talk about the one special type deduction for auto.
When the initializer for an auto-declared variable is enclosed in braces, the deduced type is `std::initializer_list`.
```C++
auto x = {11, 23, 9};  // x's type is std::initializer_list<int>

template<typename T>
void f_error(T& param);
f_error({11, 23, 9});  // error, can not deduce type for T

template<typename T>
void f_succ(std::initializer_list<T> param);
f_succ({11, 23, 9});  // T deduced as int, and param's type is std::initializer_list<int>
```

## decltype
Given a name or an expression, decltype tells you the name's or the expression's type.
```C++
int i = 33;
decltype(i) j = i * 2;

auto f = [](int a, int b){return a * b;};
decltype(f) g = f;  // the type of a lambda function is unique and unnamed
```

In C++11, the primary use for decltype is declaring function templates where the functions's return type depends on its paramter types.
For example, suppose we'd like to write a function that takes a container that supports indexing via [] plus an index, then authenticates the user before returning the result.
`oprator[]` on a container of objects of type T typically returns a `T&`.

```C++
// works since C++11, but requires refinement
template<typename Container, typename Index>
auto authAndAccess(Container& c, index i) -> decltype(c[i]) {
  authenticateUser();
  return c[i];
}
```
The use of auto before the function name has nothing to do with the type deduction.
Rather, it indicates that C++11's trailing return type syntax is being used, i.e., that the function's return type will be declared following the parameter list(after the "->").
A trailing return type has the advantage that the function's paramters can be used in the specification of the return type.

```C++
// C++14, works but still requires refinement
template<typename Conatainer, typename Index>
decltype(auto) authAndAccess(Container& c, Index i) {
  authenticateUser();
  return c[i];
}
```
auto sepecifies that the type is to be deduced, and decltype says that decltype rule should be used during the deduction.

The use of `decltype(auto)` is not limited to function return types. It can be also be convenient for declaring variables when you want to apply the decltype type deduction rules.
```C++
Widget w;
const Widget& cw = w;
auto myWidget1 = cw;  // auto type deduction, myWidget1's type is Widget
decltype(auto) myWidget2 = cw;  // decltype type deduction, myWidget2's type is const Widget
```

Look agagin at the declarition for the C++14 version of authAndAccess:
```C++
template<typename Conatainer, typename Index>
decltype(auto) authAndAccess(Container& c, Index i);
```
The container is passed by lvalue-reference-to-non-const, because returning a reference to an element of the container permits clients to modify that container.
But this also means it's not possible to pass rvalue containers to this function.
Admittedly, passing an rvalue container to authAndAccess is an edge case.
An rvalue container, being a temporary object, would typically be destroyed at the end of the statement.
Still, it could make sense to pass a temporary object to authAndAccess. A client might simpy want to make a copy of an element in the temporary container, for example:
```C++
std::deque<std::string> makeStringDeque();  // factory function
// make copy of 5th element of deque returned from makeStringDeque
auto s = authAndAccess(makeStringDeque(), 5);
```

```C++
// final C++14 version
template<typename Container, typename Index>
decltype(auto) authAndAccess(Container&& c, Index i) {
  authenticateUser();
  return std::forward<Container>(c)[i];
}

// final C++11 version
template<typename Container, typename Index>
auto authAndAccess(Container&& c, Index i) -> decltype(std::forward<Container>(c)[i]) {
  authenticateUser();
  return std::forward<Container>(c)[i];
}
```
## know how to view deduced types
### compiler diagnostics
An effective way to get a compiler to show a type it has deduced is to use that type in a way that leads to compilation problems.
The error message reporting the problem is virtually sure to mention the type that's causing it.
```C++
template<typename T>
class TD;  // declaration only for TD(Type Displayer)

const int value = 42;
auto x = value;
auto y = &value;
TD<decltype(x)> xType;
TD<decltype(y)> yType;
```
Attempts to instantiate this template will elicit an error message, because there's no template definition to instantiate.
```C++
error: aggregate 'wcg::TD<int> xType' has incomplete type and cannot be defined
error: aggregate 'wcg::TD<const int*> yType' has incomplete type and cannot be defined
```

### runtime output
Use the Boost TypeIndex library.
See example `code/thirdparty/boost/type_index_test.cc`

# moving to modern C++
## distinguish between () and {} when creating objects
C++98 has no way to directly indicate that an STL container should be created holding a particular set of values.

C++11 introduces **uniform initialization**: a single initialization syntax that can, at least in concept, be used anywhere and express everything.
It's based on braces, and for that reason, I prefer the term **braced initialization**.

A novel feature of braced initialization is that is prohibits implicit narrowing conversions among built-in types.
```C++
double x, y, z;
int sum1(x + y + z);  // ok, value of expression truncated to an int
int sum2{x + y + z};  // error, sum of doubles may not be expressible as int
```

Another noteworthy characteristic of braced initialization is its immunity to C++'s most vexing parse.
A side effect of C++'s rule that anything that can be parsed as a declaration must be interpreted as one.
The most vexing parse most frequently afflicts developers when they want to default-construct an object, but in inadvertently end up declaring a function instead.

The root of the problem is that if you want to call a constructor with an argument, you can do it as usual.
But if you want to call a Widget ctor with zero arguments using the analogous syntax, you declare a function instead of an object:
```C++
Widget w1(10);  // call Widget ctor with argument 10
Widget w2();  // most vexing parse! declares a function named w2 that returns a Widget
Widget w3{};  // calls Widget ctor with no args
```

During ctor overload resolution, braced initializers are matched to `std::initializer_list` parameters if at all possible, even if other ctors offer seemingly better matches.

## prefer alias declarations to typedefs
Alias declarations may be templaized (in which case they're called alias templates), while typedefs cannot.
For example, consider defining a synonym for a linked list that uses a custom allocator, MyAlloc
```C++
// alias declaration
// MyAllocList<T> is synonym for std::list<T, MyAlloc<T>>
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;

MyAllocList<Widget> lw;  // client code
```

With a typedef, you pretty much have to create the cake from sratch:
```C++
// typedef declaration
// MyAllocList<T>::type is synonym for std::list<T, MyAlloc<T>>
template<typename T>
struct MyAllocList {
  typedef std::list<T, MyAlloc<T>> type;
};

MyAllocList<Widget>::type lw;  // client code
```

It gets worse. If you want to use the typedef inside a template for the purpose of creating a linked list holding objects of a type specifiied by a template parameter,
you have to precede the typedef name with typename:
```C++
template<typename T>
class Widget {
 private:
  typename MyAllocList<T>::type list;  // Widget<T> contains a MyAllocList<T> as a data member
};
```
Here, `MyAllocList<T>::type` refers to a type that is dependent on a template type parameter T.
`MyAllocList<T>::type` is thus a dependent type, and one of C++'s many endearing rulels is that **the name of dependent types must be preceded by typename**.

If MyAllocList is defined as an alias template, this need for typenmae vanishes:
```C++
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;

template<typename T>
class Widget {
 private:
  MyAllocList<T> list;  // no typename and no ::type
};
```
To you, `MyAllocList<T>` may look just as dependent on the template parameter T.
But when compilers process the Widget template and encounter the use of `MyAllocList<T>`, they know that `MyAllocList<T>` is the name of a type, because it is an alias template: it must name a type.
`MyAllocList<T>` is thus a non-dependent type, and a typename specifier is neither required nor permitted.

When compilers see `MyAllocList<T>::type` (i.e., use of the nested typedef) in the Widget template, on the other hand, they can not know for sure that it names a type.
Because there might be a specialization of MyAllocList:
```C++
class Wine {};

tempalte<>
class MyAllocList<Wine> {
 private:
  enum class WineType { White, Red, Rose};
  WineType type;  // in this class, type is a data member
};
```
That's why compilers insist on your asserting that it is a type by preceding it with typename.

C++11 gives you the tools to perform some kinds of transformations in the form of type traits, defined in the header `<type_traits>`.
```C++
std::remove_const<T>::type  // yields T from const T
std::remove_reference<T>::type  // yields T from T& and T&&
std::add_lvalue_reference<T>::type  // yields T& from T
```
If you apply these type traits to a type parameter inside a template, you'd also have to precede each use with typename.
The reason is that C++ type traits are implemented as nested typedefs inside templatized structs. That's bad.

So in C++14, alias implementations are included
```C++
std::remove_const<T>::type  // C++11: const T -> T
std::remove_const_t<T>  // C++11 equivalent

std::remove_reference<T>::type  // C++11: T&/T&& -> T
std::remove_reference_t<T>  // C++14 equivalent

std::add_lvalue_reference<T>::type  // C++11: T -> T&
std::add_lvalue_reference_t<T>  // C++14 equivalent
```

## perfer deleted functions to private undefined ones
An important adavantage of deleted functions is that any function may be delted, while only member functions may be private.
For example, suppose we have a non-member function that takes an integer and returns whether it's a lucky number:
```C++
bool isLucky(int number);
bool isLucky(char) = delete;  // reject char
bool isLucky(bool) = delete;  // reject bool
bool isLucky(double) = delete;  // reject double and float
```

Another trick that deleted functions can perform is to prevent use of template instantiations that should be disabled.
For example, suppose you need a template that works with built-in pointers.
```C++
template<typename T>
void processPointer(T* ptr);

template<>
void processPointer<void>(void*) = delete;

template<>
void processPointer<char>(char*) = delete;

template<>
void processPointer<const void>(const void*) = delete;

template<>
void processPointer<const char>(const char*) = delete;
```

Interestingly, if you have a function template inside a class, and you'd like to disable some instantiations by declaring them private(in C++98 style), you can't.
Luckily, this issue doesn't arise for deleted functions, and they can also be deleted outside the class.
```C++
class Widget {
 public:
  template<typename T>
  void processPointer(T* ptr) {}

  template<>
  void processPointer<void>(void*) = delete;
};

template<>
void Widget::processPointer<char>(char*) = delete;  // still public and deleted
```

## declare functions noexcept if they won't emit exceptions
By declaring a function, a method, or a lambda-function as noexcept, you specify that these won't throw an exception and if they throw, you do not care and let the program just crash.

1. move operations (move assignment operator and move constructors)
1. swap operations
1. memory deallocators (operator delete, operator delete[])
1. destructors (though these are implicitly noexcept(true) unless you make them noexcept(false))

These functions should generally be noexcept, and it is most likely that library implementations can make use of the noexcept property.
For example, std::vector can use non-throwing move operations without sacrificing strong exception guarantees.
Otherwise, it will have to fall back to copying elements (as it did in C++98).

This kind of optimization is on the algorithmic level and does not rely on compiler optimizations.
It can have a significant impact, especially if the elements are expensive to copy.
