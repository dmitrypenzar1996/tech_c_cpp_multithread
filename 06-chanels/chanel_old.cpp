#include <stdlib.h>
#include <array>
#include <string>
#include <iostream>

#include <coroutine/engine.h>

using std::cout;
using std::endl;
using std::array;
using std::string;




template <typename ValueType> class Chanel;
template <typename ValueType> void chanel_write(Chanel<ValueType>& chanel,\
        ValueType& value);
template <typename ValueType> void chanel_read(Chanel<ValueType>& chanel,\
        ValueType& value);

template <typename ValueType> class Chanel
{
    private:

    friend void _write(Chanel& chanel, ValueType& value);
    ValueType* messages;
    size_t* cur_buf_size;
    size_t max_buf_size;
    Coroutine::Engine& engine;
    public:
    Chanel(Coroutine::Engine& _engine, size_t _max_buf_size = 1):\
                                                      max_buf_size(_max_buf_size),\
                                                      engine(_engine)
    {
        messages = new ValueType[max_buf_size]; 
        cur_buf_size = new size_t[1];
        *cur_buf_size = 0;
    }

    ~Chanel()
    {
        delete[] messages;
        delete cur_buf_size;
    }

    Chanel(const Chanel&)=delete;
    Chanel(Chanel&&)=delete;
    Chanel& operator=(const Chanel&)=delete;
    Chanel& operator=(Chanel&&)=delete;

    void __read(ValueType& value)
    {
        std::cout << *cur_buf_size << std::endl;
        while (true)
        {
            if (*cur_buf_size > 0)
            {
                value = messages[--(*cur_buf_size)];
                break;
            }
            engine.sched(nullptr); // pass to any other coroutine
        }
    }

    ValueType read()
    {
        ValueType* value = new ValueType;
        void* rc = engine.run(chanel_read, *this, *value);
        engine.sched(rc); // return here only then value will be read
        ValueType result = *value;
        delete value;
        return result;
    }

    void write(ValueType& value)
    {
        void* wc = engine.run(chanel_write, *this, value);
        engine.sched(wc); // return here only then value will be written 
    }

    void __write(ValueType& value)
    {
        while (true)
        {
            if (*cur_buf_size < max_buf_size)
            {
                messages[(*cur_buf_size)++] = value;
                break;
            }
            else
            {
                engine.sched(nullptr); 
            }
        }
    }

};


template <typename ValueType> void chanel_write(Chanel<ValueType>& chanel,\
        ValueType& value)
{
    chanel.__write(value);
}

template <typename ValueType> void chanel_read(Chanel<ValueType>& chanel,\
        ValueType& value)
{
    chanel.__read(value);
}

void __producer(Chanel& ch)
{
    while (True)
    {
        cout << ch.read << endl;
    }
}

void __main(Coroutine::Engine& en)
{
    string a = "asdsd";
    string b = "hii";
    string answer;
    Chanel<string> chanel(en, 1);
    chanel.write(a);
    answer = chanel.read();
    chanel.write(b);
    answer = chanel.read();
    chanel.write(a);
    answer = chanel.read();
    answer = chanel.read();
}

int main()
{
    Coroutine::Engine engine;
    engine.start(__main, engine);
    return 0;
}
