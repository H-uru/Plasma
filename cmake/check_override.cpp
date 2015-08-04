class Base
{
public:
    virtual ~Base() { }
    virtual void virtual_function() = 0;
};

class Override : public Base
{
public:
    void virtual_function() override { }
};

class Final : public Override
{
public:
    void virtual_function() final { }
};

int main(int, char **)
{
    Final klass;
    klass.virtual_function();

    return 0;
}
