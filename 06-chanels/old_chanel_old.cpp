#include <stdlib.h>
#include <array>
#include <string>
#include <iostream>
#include <queue>
#include <time.h>

#include <stdlib.h>
#include <coroutine/engine.h>

using std::cout;
using std::endl;
using std::string;

using std::queue;



template <typename ValueType> class Chanel;

template <typename ValueType> class Chanel
{
    private:

    queue<ValueType>* buffer;
    size_t max_buf_size;
    public:
    Chanel(size_t _max_buf_size = 1): max_buf_size(_max_buf_size)
    {
        buffer = new queue<ValueType>;
    }

    ~Chanel()
    {
        delete buffer;
    }

    Chanel(const Chanel&)=delete;
    Chanel(Chanel&&)=delete;
    Chanel& operator=(const Chanel&)=delete;
    Chanel& operator=(Chanel&&)=delete;

    ssize_t read(ValueType& value)
    {
        if (buffer->size())
        {
            value = buffer->front();
            buffer->pop();
            return 1;
        }
        else
        {
            return -1;
        }
    }

    ssize_t write(ValueType& value)
    {
        if (buffer->size() < max_buf_size)
        {
            buffer->push(value);
            return 1;
        }
        else
        {
            return -1;
        }
    }
};

string get_string()
{
    int r = rand() % 10;
    switch (r)
    {
        case 0:
            return "first_value";
        case 1:
            return "best_value";
        case 2:
            return "good_value";
        case 3:
            return "a_very_good_variable";
        case 4:
            return "nothing_to_say";
        case 5:
            return "help_me_batman";
        case 6:
            return "gotham";
        case 7:
            return "very_informative_value";
        case 8:
            return "best_choice";
        case 9:
            return "opa_opa_gavnocode";
    }
    return "Just for compiler";
}

void __producer(Coroutine::Engine& engine,\
        Chanel<string>& chanel)
{
    while (true)
    {
        for (int i = 0; i < 2; ++i)
        {
        string a = get_string();  
        while(true)
        {
            if (chanel.write(a) > 0)
            {
                break;
            }
            else
            {
                engine.sched(nullptr);
            }
        }        
        cout << "Write string " << a << endl;
        }
        engine.sched(nullptr);
    }
}

void __consumer(Coroutine::Engine& engine,
        Chanel<string>& chanel)
{
    while(true)
    {
        string a;
        while(true)
        {
            if (chanel.read(a) > 0)
            {
                break;
            }
            else
            {
                engine.sched(nullptr);
            }
        }
        cout  << "Read string " << a << endl;
        engine.sched(nullptr);
    }
}

void __main(Coroutine::Engine& engine,  Chanel<string>& chanel)
{
    void* producer = engine.run(__producer, engine, chanel); 
    void* consumer = engine.run(__consumer, engine, chanel);
    engine.sched(producer);
}

int main()
{
    srand(time(NULL));
    Coroutine::Engine engine;
    Chanel<string> chanel(2);
    engine.start(__main, engine, chanel);
    return 0;
}
