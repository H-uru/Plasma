## Indentation

Always use spaces, not tabs.  4 spaces indent a block.  Continuation can be aligned to the previous line, or with double-indent:

```c++
DoSomething(arg1, arg2, arg3,
            arg4, arg5, arg6);

DoSomethingElseWithLongFunctionName(
        arg1, arg2, arg3,
        arg4, arg5, arg6);
```


## Blocks

### Functions/Namespaces/Classes

New line for opening brace.  Visibility specifiers are aligned with the container's braces.

```c++
namespace plFoo
{
    class plBar
    {
    public:
        int Baz()
        {
            return 42;
        }
    };
}

static int foo()
{
    return 42;
}
```

### Code blocks

Keep on the same line.  Else is attached to the closing brace

```c++
if (x < y) {
    // stuff
} else {
    // Other stuff
}

if (SomeLongCondition(x, y)
    && SomeOtherLongCondition(z, w)) {
    // Stuff
} else {
    // Other stuff
}
```

### Single-line blocks

Braces are optional.  It is **never** acceptable to put a one-line block on the same line as the if statement.

```c++
if (x < y)
    return 5;
else
    return 10;
```

### Empty blocks

Don't put closing braces on their own line.

```c++
void Foo() { }

void plFoo::Bar(int x)
{ }
```

## Switch statements

Cases align with switch statement.  Braces are indented (excluding the ```break```) and used only when necessary.  Always include default, even if it is empty.

```c++
switch (condition) {
case 1:
    // Code for case 1
    break;
case 2:
    {
        int x;
        // Code for case 2
    }
    break;
default:
    // Nothing to do
    break;
}
```

## Constructors

Initializer lists are indented on the next line

```c++
plMyKlass::plMyKlass()
    : plParent(foo), fZeroInitialized(), fNonZeroInitialized(42)
{
    // Stuff
}

plMyOtherKlass::plMyOtherKlass(int x)
    : MyKlass(), fFirst(), fSecond(42),
      fThird(8.1), fFourth("Hello")
{ }
```

## Type Declarations

Pointers and references bind to the type.  Do not put more than one pointer or reference declaration in the same line.  The first const is on the left of the type.

```c++
unsigned int x;
const unsigned int x;
const unsigned int* x;
const unsigned int& x;
const unsigned int** x;
const unsigned int* const* x;

char* str1;
char* str2;
char  fixedStr3[64];
```

C++11 allows nested templates to be used without an extra space:

```c++
std::vector<std::map<int, char>> x;
```

## Classes

Specify parent(s) on same line

```c++
class plFoo : public plBar, public plBaz
{
public:
    // Contents
};
```

Explicitly call out overridden virtual functions

```c++
class plFoo : public plBar
{
public:
    // New virtual function for plFoo
    virtual void DoFooStuff();
    
    // Overloaded virtual function from plBar
    void DoBarStuff() override;
    
    // Pure virtual function
    virtual void NotImplemented() = 0;
};
```

Use deleted functions rather than declaring disabled functions private:

```c++
class plFoo
{
public:
    plFoo();
    ~plFoo();

    // Disable copying
    plFoo(const plFoo&) = delete;
    void operator=(const plFoo&) = delete;
};
```

## Templates

Template declaration should be on its own line.

```c++
template <typename _T>
void Foo(const _T& x)
{
    // Stuff
}
```

## Naming Convention

Public function names should start with an upper case letter, and then be camelCase.  Classes should have a prefix (usually 'pl') and then be followed by camelCase.

Member variables should start with an 'f' ("field") and then be camelCase.  Global variables should be avoided, but may use 'g' followed by camelCase.  Static variables should start with an 's'.  Local variables and parameters have no prefix.

```c++
class plKlass
{
public:
    void plKlass(int x, const char* y)
        : fX(x), fY(y)
    { }

private:
    int fX;
    const char* fY;
};

void Bar(int x, int y)
{
    int z = x + y;
    return z - 5;
}
```

## Spacing

Function calls and declarations should have no space before parens.  Control flow statements should have a space between the keyword and the parens.  Do not use parens around a ```return``` statement's parameter.

```c++
int Foo(int x)
{
    if (x < 10)
        return 5;

    while (x > 0)
        x += DoMath(x);
    return x;
}
```

Initializer lists should have spaces on both sides.

```c++
std::pair<int> x { 5, 10 };
```

Unary operators should have no space between themselves and their operand.

```c++
if (!fPointer)
    return false;
```

Binary (and ternary) operators should have spaces on both sides.

```c++
return (x < 5 && y > 5) ? -1 : 1;

for (const auto& x : myList)
    Display(x);
```

## Labels and goto

Just Say No (tm).  It is acceptable to use other control flow tricks if absolutely positively necessary.

```c++
do {
    // Stuff

    if (x < y)
        break;

    // Other stuff
} while (false);
```

## Casting

Use C++ casts whenever possible.

```c++
return static_cast<int>(5.0f);
return reinterpret_cast<const void*>("Foo");
```

## Lambdas

Treat lambdas as blocks for brace placement.  Captures should be explicit.

```c++
RegisterCallback(object,
    [this, x](const char *message) {
        ShowMessage(message, x);
    });

RegisterDataProvider(
    [this](plGenerator* gen) -> plData* {
        if (this)
            return gen->GenerateFor(this);
        else
            return nullptr;
    });
```
