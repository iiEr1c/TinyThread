#ifndef THREADSAFE_STACK_H
#define THREADSAFE_STACK_H

#include <exception>
#include <stack>
#include <mutex>
#include <memory>

// ToDo: exception
struct empty_stack : std::exception
{
    const char *what() const throw() override;
};

const char * empty_stack::what() const throw()
{
    return "empty stack";
}

template<typename T>
class threadsafe_stack
{
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack &);
    threadsafe_stack &operator=(const threadsafe_stack &) = delete;
    void push(T);
    void pop(T &);
    std::shared_ptr<T>pop();
    bool empty() const;
private:
    std::stack<T>data;
    mutable std::mutex mut;
};

template<typename T>
threadsafe_stack<T>::threadsafe_stack(const threadsafe_stack &t)
{
    std::lock_guard<std::mutex>lk(t.mut);
    data = t.data;
}

template<typename T>
bool threadsafe_stack<T>::empty() const
{
    std::lock_guard<std::mutex>lk(mut);
    return data.empty();
}

template<typename T>
void threadsafe_stack<T>::push(T val)
{
    std::lock_guard<std::mutex>lk(mut);
    data.push(std::move(val));
}

template<typename T>
void threadsafe_stack<T>::pop(T &val)
{
    std::lock_guard<std::mutex>lk(mut);
    if(data.empty())
        throw empty_stack();
    val = std::move(data.top());
    data.pop();
}

template<typename T>
std::shared_ptr<T> threadsafe_stack<T>::pop()
{
    std::lock_guard<std::mutex>lk(mut);
    if(data.empty())
        throw empty_stack();
    const std::shared_ptr<T> res = std::make_shared<T>(std::move(data.top()));
    data.pop();
    return res;
}

#endif
