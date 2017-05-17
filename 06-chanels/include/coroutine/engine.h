#ifndef COROUTINE_ENGINE_H
#define COROUTINE_ENGINE_H

#include <cstdint>
#include <iostream>
#include <map>
#include <setjmp.h>
#include <string.h>
#include <tuple>
#include <vector>

using std::vector;
using std::map;
using std::pair;

namespace Coroutine {

/**
 * # Entry point of coroutine library
 * Allows to run coroutine and schedule its execution. Not threadsafe
 */
class Engine final {
private:
    /**
     * A single coroutine instance which could be scheduled for execution
     * should be allocated on heap
     */
    struct context;
    typedef struct context {
        // coroutine stack start address
        char* Low = nullptr;

        // coroutine stack end address
        char* Hight = nullptr;

        // coroutine stack copy buffer
        std::tuple<char*, uint32_t> Stack = std::make_tuple(nullptr, 0);

        // Saved coroutine context (registers)
        jmp_buf Environment;

        // Coroutine that has started this one. Once current routine is done, control must
        // be passed back to caller
        struct context* caller = nullptr;

        // Coroutine got control from the current one. Whenever current routine
        // continues self exectution it must transfers control to callee if any
        struct context* callee = nullptr;

        // To include routine in the different lists, such as "alive", "blocked", e.t.c
        struct context* prev = nullptr;
        struct context* next = nullptr;
    } context;

    struct Chanel {
        char* buffer = nullptr;
        size_t begin_offset = 0;
        size_t end_offset = 0;
        size_t cur_size = 0;
        size_t capacity = 0;
        Chanel(size_t _capacity)
            : capacity(_capacity)
            , cur_size(0) {
            buffer = new char[capacity];
        }
        Chanel(const Chanel&) = delete;
        Chanel(Chanel&&) = delete;
        Chanel& operator=(const Chanel&) = delete;
        Chanel& operator=(Chanel&&) = delete;
        ~Chanel();
        ssize_t write(const char* data, size_t size);
        ssize_t read(char* data, size_t max_size);
    };

    enum Chanel_Mode { CHANEL_READ,
        CHANEL_WRITE };

    map<int, Chanel*>* chanel_table; // mapping of descriptors to chanel structures
    map<context*, pair<Chanel*, Chanel_Mode>*>* block_table; //for each context,

    //which chanel it's waiting for, and to write or to read
    //if no such chanel, when chanel* is nullptr

    /**
     * Where coroutines stack begins
     */
    char* StackBottom;

    /**const int&
     * Current coroutine
     */
    context* cur_routine;

    /**
     * List of routines ready to be scheduled. Note that suspended routine ends up here as well
     */
    context* alive;

    /**
     * Context to be returned finally
     */
    context* idle_ctx;

protected:
    /**
     * Save stack of the current coroutine in the given context
     */
    void Store(context& ctx);

    /**
     * Restore stack of the given context and pass control to coroutinne
     */
    void Restore(context& ctx);

    /**
     * Suspend current coroutine execution and execute given context
     */
    //void Enter(context& ctx);

public:
    Engine()
        : StackBottom(0)
        , cur_routine(nullptr)
        , alive(nullptr) {}
    Engine(Engine&&) = delete;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;

    /**
     * Gives up current routine execution and let engine to schedule other one. It is not defined when
     * routine will get execution back, for example if there are no other coroutines then executing could
     * be trasferred back immediately (yield turns to be noop).
     *
     * Also there are no guarantee what coroutine will get execution, it could be caller of the current one or
     * any other which is ready to run
     */
    void yield();

    /**
     * Suspend current routine and transfers control to the given one, resumes its execution from the point
     * when it has been suspended previously.
     *
     * If routine to pass execution to is not specified runtime will try to transfer execution back to caller
     * of the current routine, if there is no caller then this method has same semantics as yield
     */
    void sched(void* routine);

    /**
     * Entry point into the engine. Prepare all internal mechanics and starts given function which is
     * considered as main.
     *
     * Once control returns back to caller of start all coroutines are done execution, in other words,
     * this function doesn't return control until all coroutines are done.
     *
     * @param pointer to the main coroutine
     * @param arguments to be passed to the main coroutine
     */
    template <typename... Ta>
    void start(void (*main)(Ta...), Ta&&... args) {
        // To acquire stack begin, create variable on stack and remember its address
        char StackStartsHere;
        this->StackBottom = &StackStartsHere;
        this->chanel_table = new map<int, Chanel*>;
        this->block_table = new map<context*, pair<Chanel*, Chanel_Mode>*>();
        //for each context,

        // Start routine execution
        void* pc = run(main, std::forward<Ta>(args)...);
        idle_ctx = new context();

        if (setjmp(idle_ctx->Environment) > 0) {
            //Here: correct finish of the coroutine section
            return;
        } else {
            if (pc != nullptr) {
                Store(*idle_ctx);
                sched(pc);
            }
        }

        // Shutdown runtime
        delete idle_ctx;
        this->StackBottom = 0;
    }

    int get_chanel(size_t buf_size);
    ssize_t chanel_write(int cd, const char* buffer, size_t size, bool block = true);
    ssize_t chanel_read(int cd, char* buffer, size_t max_size, bool block = true);

    template <typename... Ta>
    void* run(void (*func)(Ta...), Ta&&... args) {
        if (this->StackBottom == 0) {
            // Engine wasn't initialized yet, BAD
            return nullptr;
        }

        // New coroutine context that carries around all information enough to call function
        context* pc = new context();
        pc->caller = cur_routine;

        // Store current state right here, i.e just before enter new coroutine, later, once it gets scheduled
        // execution starts here. Note that we have to acquire stack of the current function call to ensure
        // that function parameters will be passed along
        // LONGJMP RETURNS HERE!
        if (setjmp(pc->Environment) > 0) {
            // here: only after longjmp
            // Created routine got control in order to start execution. Note that all variables, such as
            // context pointer, arguments and a pointer to the function comes from restored stack

            // invoke routine
            func(std::forward<Ta>(args)...);
            std::cout << "complete: " << pc << ", next: " << pc->caller << std::endl;

            // Routine complete its execution, time to delete it. Note that we should be extremely careful in where
            // to pass control after that. We never want to go backward by stack as that would mean to go backward in
            // time. Function run() has already return once (when setjmp returns 0), so return second return from run
            // would looks a bit awkward
            context* next = pc->caller;
            if (pc->prev != nullptr) {
                pc->prev->next = pc->next;
            }

            if (pc->next != nullptr) {
                pc->next->prev = pc->prev;
            }

            if (pc->caller != nullptr) {
                pc->caller->callee = nullptr;
            }

            if (alive == cur_routine) {
                alive = alive->next;
            }

            // current coroutine finished, and the pointer is not relevant now
            cur_routine = nullptr;

            pc->prev = pc->next = nullptr;
            delete std::get<0>(pc->Stack);
            delete pc;

            // We cannot return here, as this function "returned" once already, so here we must select some other
            // coroutine to run. As current coroutine is completed and can't be scheduled anymore, it is safe to
            // just give up and ask scheduler code to select someone else, control will never returns to this one
            if (next != nullptr) {
                sched(next);
            } else {
                yield();
            }
            Restore(*idle_ctx);
        }

        // setjmp remembers position from which routine could starts execution, but to make it correctly
        // it is neccessary to save arguments, pointer to body function, pointer to context, e.t.c - i.e
        // save stack.
        Store(*pc);

        // Add routine as alive list
        // add to the beginning of the double-linked list
        pc->next = alive;
        alive = pc;
        if (pc->next != nullptr) {
            pc->next->prev = pc;
        }

        return pc;
    }
};

} // namespace Coroutine

#endif // COROUTINE_ENGINE_H
