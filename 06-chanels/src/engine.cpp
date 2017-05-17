#include <coroutine/engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

namespace Coroutine {
Engine::Engine(): StackBottom(0)
        , cur_routine(nullptr)
        , alive(nullptr) 
{
    this->chanel_table = new map<int, Chanel*>;
    this->block_table = new map<context*, pair<Chanel*, Chanel_Mode>*>();    
}

Engine::~Engine()
{
    delete this->chanel_table;
    delete this->block_table;
}



void Engine::Store(context& ctx) {
    char stackStart;

    ctx.Low = ctx.Hight = this->StackBottom;
    if (&stackStart > ctx.Low) {
        ctx.Hight = &stackStart;
    } else {
        ctx.Low = &stackStart;
    }

    int size = ctx.Hight - ctx.Low;
    if (std::get<1>(ctx.Stack) < size) {
        delete std::get<0>(ctx.Stack);
        std::get<0>(ctx.Stack) = new char[size];
        std::get<1>(ctx.Stack) = size;
    }

    memcpy(std::get<0>(ctx.Stack), ctx.Low, size);
}

void Engine::Restore(context& ctx) {
    char stackStart;
    char* stackAddr = &stackStart;

    if (ctx.Low <= stackAddr && stackAddr <= ctx.Hight) {
        Restore(ctx);
    }

    memcpy(ctx.Low, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    sched(nullptr);
}

void Engine::sched(void* routine_) {
    context* routine = (context*)routine_;

    if (cur_routine != nullptr) {
        if (setjmp(cur_routine->Environment) != 0) {
            return;
        }
        Store(*cur_routine);
    }



    // pass control to the another coroutine
    // if no another coroutine specified, the same semantics as for yield()

    // these lines are required to exit last run() correctly
    if (routine == nullptr && cur_routine == nullptr) {
        if (alive == nullptr) {
            return;
            // no coroutines remain
        } else {
            // find any not blocked routine
            for (context* p = alive; p != nullptr; p = p->next) {
                if (block_table->find(p) == block_table->end()) {
                    routine = p;
                    break;
                }
            }
        }
    }

    if (routine == nullptr && cur_routine != nullptr) {
        // invoke the caller of current coroutine
        if (cur_routine->caller != nullptr && (block_table->find(cur_routine->caller) == block_table->end())) {
            routine = cur_routine->caller;
        }
        // invoke ANY coroutine another than cur_routine
        // if no coroutines remain, pass control back to cur_routine
        else {
            for (context* p = alive; p != nullptr; p = p->next) {
                if ((p != cur_routine) && (block_table->find(p) == block_table->end())) { // find any routine != cur_routine
                    routine = p;
                    break;
                }
            }
            // if only cur_routine remains -> pass back to cur_routine
            if ((routine == nullptr) && (block_table->find(cur_routine) == block_table->end())) {
                routine = cur_routine;
            } else if (routine == nullptr) {
                
                std::cerr << "Dead lock" << std::endl;
                exit(1);
            }
        }
    }

    if (routine->callee != nullptr && routine->callee == cur_routine) {
        routine->callee = routine->callee->caller = nullptr;
    }

    while (routine->callee != nullptr
        && (block_table->find(routine->callee) == block_table->end())) {
        routine = routine->callee;
    }

    // set new caller
    // recall: caller is a coroutine which invoked "routine" via yield() or sched()
    routine->caller = cur_routine;

    cur_routine = routine;
    Restore(*routine);
}

Engine::Chanel::~Chanel() {
    delete[] buffer;
}

ssize_t Engine::Chanel::write(const char* data, size_t size) {
    if (size > capacity - cur_size) {
        return -1;
    }

    if (size <= capacity - end_offset) {
        memcpy(buffer + end_offset, data, size);
        end_offset += size;
        if (end_offset == capacity) {
            end_offset = 0;
        }
    } else {
        size_t to_end_size = capacity - end_offset;
        memcpy(buffer + end_offset, data, to_end_size);
        size_t from_start_size = size - to_end_size;
        memcpy(buffer, data, from_start_size);
        end_offset = from_start_size;
    }

    cur_size += size;
    return size;
}

ssize_t Engine::Chanel::read(char* data, size_t max_size) {
    if (cur_size == 0) {
        return -1;
    }
 
    size_t read_num = std::min(max_size, cur_size);
    if (read_num <= capacity - begin_offset) {
        memcpy(data, buffer + begin_offset, read_num);
        begin_offset += read_num;
        if (begin_offset == capacity) {
            begin_offset = 0;
        }
    } else {
        size_t size_1 = capacity - begin_offset;
        memcpy(data, buffer + begin_offset, size_1);
        size_t size_2 = read_num - size_1;
        memcpy(data + size_1, buffer, size_2);
        begin_offset = size_2;
    }
    cur_size -= read_num;
    return read_num;
}

int Engine::get_chanel(size_t buf_size) {
    static int chanel_id = 0;
    Chanel* chanel = new Chanel(buf_size);
    int key = chanel_id++;
    (*chanel_table)[key] = chanel;
    return key;
}

void Engine::delete_chanel(int cd)
{
    Chanel* chanel = (*chanel_table)[cd];
    delete chanel;
}

ssize_t Engine::chanel_write(int cd, const char* buffer, size_t size, bool block) {
    Chanel* chanel = (*chanel_table)[cd];
    while (true) {
        ssize_t write_num = chanel->write(buffer, size);
        if (write_num < 0) {
            // no enough space in chanel
            if (block) // if blocking access, then block coroutine until we could
            // write to chanel
            {
                pair<Chanel*, Chanel_Mode>* p = new pair<Chanel*, Chanel_Mode>(chanel, CHANEL_WRITE);
                (*block_table)[cur_routine] = p;
                sched(nullptr);
            } else {
                return -1;
            }
        } else {
            // and unblock corutines waiting for reading from it
            vector<context*> unblock_routines;
            for (pair<context*, pair<Chanel*, Chanel_Mode>*> p : (*block_table)) {
                if (p.second->first == chanel && p.second->second == CHANEL_READ) {
                    unblock_routines.push_back(p.first);
                }
            }
            for (context* s : unblock_routines) {
                block_table->erase(s);
            }
            return size;
        }
    }
}

ssize_t Engine::chanel_read(int cd, char* buffer, size_t max_size, bool block) {
    Chanel* chanel = (*chanel_table)[cd];
    while (true) {
        ssize_t read_num = chanel->read(buffer, max_size);
        if (read_num < 0) {
            if (block) // if blocking access, then block coroutine until we can
            // read to chanel
            {
                pair<Chanel*, Chanel_Mode>* p = new pair<Chanel*, Chanel_Mode>(chanel, CHANEL_READ);
                (*block_table)[cur_routine] = p;
                sched(nullptr);
            } else {
                return -1;
            }
        } else {
            // read from chanel and unblock coroutines waiting for write
            vector<context*> unblock_routines;
            for (pair<context*, pair<Chanel*, Chanel_Mode>*> p : (*block_table)) {
                if (p.second->first == chanel && p.second->second == CHANEL_WRITE) {
                    unblock_routines.push_back(p.first);
                }
            }
            for (context* s : unblock_routines) {
                block_table->erase(s);
            }
            return read_num;
        }
    }
}

} // namespace Coroutine
