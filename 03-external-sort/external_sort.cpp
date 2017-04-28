#include <cstdlib>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <cstddef>
#include <exception>
#include <vector>
#include <iterator>
#include <iostream>
#include <memory>
#define TEMPORARY_FILE_MASK "temporaryXXXXXX"
#define WRONG_ALIGN_ERROR 666
#define WRONG_WRITE_SIZE 667
#define LOG_LEVEL 10

typedef int INT;

size_t S = 1024 * 4 * sizeof(INT);
size_t B = 1024 * sizeof(INT);

size_t OPEN_FILES_LIMIT = 128;

using std::vector;
using std::exception;
using std::array;
using std::begin;
using std::end;
using std::cout;
using std::endl;
using std::exception_ptr;
using std::shared_ptr;
using std::make_shared;


class TempFileException : public exception
{
    static const size_t MESSAGE_MAX_LENGTH = 128;
private:
    int err_code;
    char* msg;
public:
    TempFileException(int _errcode);

    virtual const char* what() const throw();

    ~TempFileException(){
        if (msg)
        {
            delete msg;
        }
    }

};

TempFileException::TempFileException(int _errcode) : err_code{_errcode}, msg{nullptr}
{
    err_code = _errcode;
    msg = new char[MESSAGE_MAX_LENGTH];
    if (err_code == WRONG_ALIGN_ERROR)
    {
        snprintf(msg, MESSAGE_MAX_LENGTH,
                "Error is %d : size is not multiple of INT", err_code);
    }
    else if (err_code == WRONG_WRITE_SIZE){
        snprintf(msg, MESSAGE_MAX_LENGTH,\
                "Error is %d : wrong write size", err_code);
    }
    else
    {
        snprintf(msg, MESSAGE_MAX_LENGTH, "Error is %d : %s", err_code,\
                strerror(err_code));
    }
}

const char* TempFileException::what() const throw()
{
    return msg;
}

class TempFile
{
    private:
        char path_name[sizeof(TEMPORARY_FILE_MASK)];
        int fd;
        char* buf;
        size_t buf_size;
        size_t buf_pos;
        size_t capacity;
        bool at_end;
        int flags;
    public:
        TempFile(char* _buf=0, size_t _capacity=0);
        ~TempFile();
        char* getPath() {return path_name;}
        void writeNum(INT num);
        void flush();
        ssize_t readNum(INT* num);
        void writeRange(void* range, size_t range_size);
        bool get_at_end() {return at_end;}
        void set_buffer(char* _buf, size_t _capacity){
            buf = _buf; 
            capacity = _capacity; 
            buf_pos = 0;
        }
        void close() 
        {
            int succ = ::close(fd);
            if (succ == -1)
            {
                throw TempFileException(errno);
            }
        }
        void open(int flags);
};


void TempFile::open(int flags)
{ 
    fd = ::open(path_name, flags); 
    flags = flags;
    if (fd == -1)
    {
        throw TempFileException(errno);
    }
}
TempFile::TempFile(char* _buf, size_t _capacity) : buf{_buf},\
                                   capacity{_capacity}, buf_pos{0}, buf_size{0},\
                                   fd{-1}, flags{O_RDWR}, at_end{false}
{
    if (capacity % sizeof(INT))
    {
        throw TempFileException(WRONG_ALIGN_ERROR);
    }

    strcpy(path_name, TEMPORARY_FILE_MASK);
    fd = mkstemp(path_name);
    if (fd == -1)
    {
        throw TempFileException(errno);
    }
}

void TempFile::flush()
{
    if ((flags & (O_WRONLY | O_RDWR)) && buf_pos != 0) 
    {
        ssize_t w_num = write(fd, buf, buf_pos);
        if (w_num == -1)
        {
            throw TempFileException(errno);
        }
        if (w_num != buf_pos)
        {
            throw TempFileException(WRONG_WRITE_SIZE);
        }
        buf_pos = 0;
    }
}

TempFile::~TempFile()
{
    flush();
    ::close(fd);
    unlink(path_name);
}


ssize_t TempFile::readNum(INT* num)
{
    if (at_end)
    {
        return 0;
    } 
    
    if (! buf)
    {
        ssize_t r_num = read(fd, (void*)num, sizeof(INT));
        if (r_num == -1)
        {
            throw TempFileException(errno);
        }

        if (r_num == 0)
        {
            at_end = true;
            return 0;
        }
    }
    else
    {
        if (buf_pos == buf_size)
        {
            buf_pos = 0;
            buf_size = read(fd, buf, capacity);

            if (buf_size == -1)
            {
                throw TempFileException(errno);
            }

            if (buf_size % sizeof(INT) != 0)
            {
                throw TempFileException(WRONG_ALIGN_ERROR); 
            } 
        }

        if (buf_size == 0)
        {
            at_end = true;
            return 0;
        }

        *num = *(INT*)(buf + buf_pos);
        buf_pos += sizeof(INT); 
    }


    return 1;
}

void TempFile::writeNum(INT num)
{
    if (! buf)
    {
        ssize_t w_num = write(fd, &num, sizeof(INT));
        if (w_num == -1)
        {
            throw TempFileException(errno);
        } 
    }

    if (buf_pos == capacity)
    {
        flush();
    }
    *(INT*)(buf + buf_pos) = num; 
    buf_pos += sizeof(INT);
} 

void TempFile::writeRange(void* range, size_t range_size)
{
    flush();
    ssize_t w_num = write(fd, range, range_size);
    if (w_num == -1)
    {
        throw TempFileException(errno); 
    }
}


int cmpfunc (const void* _a, const void* _b)
{
    INT a = *(INT*)_a;
    INT b = *(INT*)_b;

    if (a > b)
    {
        return 1;
    }
    else if (a < b)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

vector<shared_ptr<TempFile>> split(int in_file)
{
    char buffer[S];
    vector<shared_ptr<TempFile>> splitFiles;

    ssize_t in_rnum = read(in_file, buffer, S);
    if (in_rnum == -1)
    {
        fprintf(stderr, "Problems with read from in_file, error %d : %s\n",
                errno, strerror(errno));
        exit(errno);
    }
    while (in_rnum != 0)
    {
        shared_ptr<TempFile> tf = make_shared<TempFile>(nullptr, 0);
        qsort(buffer, in_rnum / sizeof(INT), sizeof(INT), cmpfunc);
        tf->writeRange(buffer, in_rnum);
        tf->close();
        splitFiles.push_back(tf);
        in_rnum = read(in_file, buffer, S);
        if (in_rnum == -1)
        {
            fprintf(stderr, "Problems with read from in_file, error %d : %s\n",
                errno, strerror(errno));
            exit(errno);
        }
    }

    return splitFiles;
}


vector<shared_ptr<TempFile>> merge(vector<shared_ptr<TempFile>>& in_files)
{
    char buffer[S];
    vector<shared_ptr<TempFile>> out_files;
    size_t cur_files_pos = 0;
    size_t merge_files_num;
    size_t file_limit = S / B; 

    if (file_limit > OPEN_FILES_LIMIT)
    {
        file_limit = OPEN_FILES_LIMIT;
    }
    --file_limit; // one for out file and page in buffer for it

    while (cur_files_pos != in_files.size())
    {
        merge_files_num = in_files.size() - cur_files_pos;
        if (merge_files_num > file_limit)
        {
            merge_files_num = file_limit;
        }

        char* cur_buffer_pos = buffer;
        shared_ptr<TempFile> out_file = make_shared<TempFile>(cur_buffer_pos, B);
        cur_buffer_pos += B; 

        for(int i = 0; i < merge_files_num; ++i)
        {
            in_files[i + cur_files_pos]->open(O_RDONLY);
            in_files[i + cur_files_pos]->set_buffer(cur_buffer_pos, B);
            cur_buffer_pos += B; 
        }

        vector<bool> is_ended(merge_files_num);
        std::fill(begin(is_ended), end(is_ended), false);
        std::vector<INT> cur_candidates(merge_files_num);

        //init candidates
        for(int i = 0; i < merge_files_num; ++i)
        {
            int is_read = in_files[i + cur_files_pos]->readNum(&cur_candidates[i]);
            if (! is_read)
            {
                is_ended[i] = true;
            }
        }

        size_t cur_pos = 0;
        while (1)
        {
            while (cur_pos < merge_files_num && is_ended[cur_pos])
            {
                ++cur_pos;
            }

            if (cur_pos == merge_files_num) // all files at end
            {
                break;
            }

            INT min_pos = cur_pos;
            INT min_value = cur_candidates[cur_pos]; 

            for(int i = cur_pos + 1; i < merge_files_num; ++i)
            {
                if (! is_ended[i] && cur_candidates[i] < min_value)
                {
                    min_value = cur_candidates[i];
                    min_pos = i;
                }
            }
            out_file->writeNum(min_value); 

            int is_read = in_files[min_pos + cur_files_pos]->\
                          readNum(&cur_candidates[min_pos]);

            if (! is_read)
            {
                is_ended[min_pos] = true;
            }    
        }


        out_file->flush();
        out_file->close();
        out_files.push_back(out_file);

        for (int i = 0; i < merge_files_num; ++i)
        {
            in_files[i + cur_files_pos]->close();
        }

        cur_files_pos += merge_files_num;
    }


    return out_files;
}


int main(int argNum, char** args)
{
    char const* in_file_name = args[1];
    char const* out_file_name = "out.bin";
    if (argNum == 3)
    {
        out_file_name = args[2];
    }

    vector<shared_ptr<TempFile>> sort_files;
    vector<shared_ptr<TempFile>> new_sort_files;
    
    std::exception_ptr ex_ptr;
    try
    {
        int in_file = open(in_file_name, O_RDONLY);
        if (LOG_LEVEL >= 10)
        {
            cout << "Start splitting file...";
        }
        sort_files = split(in_file);
        if (LOG_LEVEL >= 10)
        {
            cout << " done" << endl;
        }
        close(in_file);

        if (LOG_LEVEL >= 10)
        {
            cout << "Merge is started" << endl;
        }
        size_t phase_id = 0;
        while (sort_files.size() != 1)
        {
            ++phase_id;
            if (LOG_LEVEL >= 10)
            {
                cout << "     Phase " << phase_id << "is started ..."; 
            }
            new_sort_files = merge(sort_files);
            sort_files = new_sort_files;
            if (LOG_LEVEL >= 10)
            {
                cout << "   done" << endl;
            }
        }

        if (LOG_LEVEL >= 10)
        {
            cout << "Merge is finished" << endl;
        }

        unlink(out_file_name);
        int succ_flag = link(sort_files[0]->getPath(), out_file_name);
        //delete sort_files[0];
    }
    catch (const exception& ex)
    {
        std::cout << "Caught exception: " <<  ex.what() << endl;
    }
    return 0;
}
