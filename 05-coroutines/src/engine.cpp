#include <coroutine/engine.h>

#include <setjmp.h>

namespace Coroutine {

void Engine::Store(context& ctx) {
    char stack_start; // get current stack position

    ctx.Low = this->StackBottom;
    ctx.Hight = this->StackBottom;

    if (&stack_start > ctx.Low)
    {
        ctx.Hight = &stack_start;
    }
    else
    {
        ctx.Low = &stack_start;
    }
    int allow_size = ctx.Hight - ctx.Low;
    if (std::get<1>(ctx.Stack) < allow_size)
    {
        delete std::get<0>(ctx.Stack); // free buffer !
        std::get<0>(ctx.Stack) = new char[allow_size];
        std::get<1>(ctx.Stack) = allow_size;
    }
    memcpy(std::get<0>(ctx.Stack), ctx.Low, allow_size);
}

void Engine::Restore(context& ctx) {
    char stack_start;
    if (ctx.Low <= &stack_start && &stack_start <= ctx.Hight)
    {
        Restore(ctx); // while memory is not enough, recursion 
    }
    
    memcpy(ctx.Low, std::get<0>(ctx.Stack),\
            std::get<1>(ctx.Stack));
    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    sched(nullptr);
}

void Engine::sched(void* routine_) {
    context* routine = (context*)routine_;

    if (cur_routine != nullptr)
    {
        if (setjmp(cur_routine->Environment) != 0)
        {
            return;
        }
        Store(*cur_routine);
    }
    
    if (routine == nullptr && cur_routine == nullptr)
    {// last run
        routine = alive;
        if (alive == nullptr)
        {
            return;
        }
    }

    if (routine == nullptr && cur_routine != nullptr)
    {
        if (cur_routine->caller != nullptr)
        {
            routine = cur_routine->caller; // pass to caller
        }
        else
        {
            for (context* p = alive; p != nullptr; p = p->next)
            {
                if (p != cur_routine)
                {
                    routine = p; 
                    break;
                }
            }

            if (routine == nullptr)
            {
                routine = cur_routine;
            }
        }
    }

    if (routine->callee != nullptr \
            && routine->callee == cur_routine)
    {
        routine->callee = routine->callee->callee = nullptr;
    }

    while(routine->callee != nullptr)
    {
        routine = routine->callee;
    }

    routine->caller = cur_routine;
    cur_routine = routine;
    Restore(*routine);
}
} // namespace Coroutine
