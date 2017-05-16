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


string get_string()
{
    int r = rand() % 10;
    switch (r)
    {
        case 0:
            return "first_value\n";
        case 1:
            return "best_value\n";
        case 2:
            return "good_value\n";
        case 3:
            return "a_very_good_variable\n";
        case 4:
            return "nothing_to_say\n";
        case 5:
            return "help_me_batman\n";
        case 6:
            return "gotham\n";
        case 7:
            return "very_informative_value\n";
        case 8:
            return "best_choice\n";
        case 9:
            return "opa_opa_gavnocode\n";
    }
    return "Just for compiler\n";
}

void __writer(Coroutine::Engine& engine, int& cd)
{
    std::cout << "Writer" << std::endl;
    while (true)
    {
        if (rand() % 2)
        {
            string a = get_string();
            engine.chanel_write(cd, a.c_str(), a.size());
        }
        engine.sched(nullptr);
    }
}

void __reader(Coroutine::Engine& engine, int& cd)
{
    std::cout << "Reader" << std::endl;
    while (true)
    {
        if (rand() % 2)
        {
            char b[100];
            ssize_t read_num = engine.chanel_read(cd, b, 99);
            b[read_num] = '\0';
            std::cout << "Read size: " << read_num << endl << b << endl; 
        }
        engine.sched(nullptr);
    }
}

void __main(Coroutine::Engine& engine)
{
    int cd = engine.get_chanel(2048);
    void* writer = engine.run(__writer, engine, cd);
    void* reader = engine.run(__reader, engine, cd);
    std::cout << "writer" << writer << std::endl;
    std::cout << "reader" << reader << std::endl;
    engine.sched(writer);
}

int main()
{
    srand(time(NULL));
    Coroutine::Engine engine;
    engine.start(__main, engine);
    return 0;
}
