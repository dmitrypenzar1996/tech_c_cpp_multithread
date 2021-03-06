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

void __png(Coroutine::Engine& engine, int& in_cd, int& out_cd, string& name)
{
    while(true)
    {
        char a[2];
        engine.chanel_read(in_cd, a, 2);
        std::cout << name << std::endl;
        ++a[0];
        engine.chanel_write(out_cd, a, 2);
    }
}


void __main(Coroutine::Engine& engine, int& c12, int& c23, int& c31)
{
    string name1 = "ping";
    string name2 = "pang";
    string name3 = "pong";
    void* ping = engine.run(__png, engine, c31, c12, name1);
    void* pang = engine.run(__png, engine, c12, c23, name2);
    void* pong = engine.run(__png, engine, c23, c31, name3);
    engine.chanel_write(c31, "\1", 1);
    engine.sched(ping);
}

int main()
{
    srand(time(NULL));
    Coroutine::Engine engine;
    int c12 = engine.get_chanel(2); 
    int c23 = engine.get_chanel(2); 
    int c31 = engine.get_chanel(2); 
    engine.start(__main, engine, c12, c23, c31);
    engine.delete_chanel(c12);
    engine.delete_chanel(c23);
    engine.delete_chanel(c31);
    return 0;
}
