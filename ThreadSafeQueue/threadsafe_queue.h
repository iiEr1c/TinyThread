#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H

#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue
{
public:
    // pay attention to class member's initialization order:'head' is ahead of 'tail'
    threadsafe_queue() : head(new node), tail(head.get()) {}
    threadsafe_queue(const threadsafe_queue &) = delete;
    threadsafe_queue &operator=(const threadsafe_queue &) = delete;
    bool try_pop(T &);
    std::shared_ptr<T>try_pop();
    std::shared_ptr<T>wait_and_pop();
    void wait_and_pop(T &);
    void push(T val);
    bool empty();
private:
    struct node
    {
        std::shared_ptr<T>data;
        std::unique_ptr<node>next;
    };
    std::unique_ptr<node>head;
    std::mutex head_mutex;
    std::mutex tail_mutex;
    node *tail;
    std::condition_variable data_cond;
    node *get_tail()
    {
        std::lock_guard<std::mutex>tail_lock(tail_mutex);
        return tail;
    }

    std::unique_lock<std::mutex>wait_for_data()
    {
        std::unique_lock<std::mutex>head_lock(head_mutex);
        data_cond.wait(head_lock,[&](){return head.get() != get_tail(); });
        // equivalent to: data_cond.wait(head_lock,[this](){return head.get() != get_tail(); });
        // the essence of expression('&' and 'this') is current object
        return std::move(head_lock);
    }

    std::unique_ptr<node>pop_head()
    {
        std::unique_ptr<node>old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

    std::unique_ptr<node>wait_pop_head()
    {
        std::unique_lock<std::mutex>head_lock(wait_for_data());
        return pop_head();
    }

    std::unique_ptr<node>wait_pop_head(T &val)
    {
        std::unique_lock<std::mutex>head_lock(wait_for_data());
        val = std::move(*head->data);
        return pop_head();
    }

    std::unique_ptr<node>try_pop_head()
    {
        std::lock_guard<std::mutex>head_lock(head_mutex);
        if(head.get() == get_tail())
            return std::unique_ptr<node>();
        return pop_head();
    }

    std::unique_ptr<node>try_pop_head(T &val)
    {
        std::lock_guard<std::mutex>head_lock(head_mutex);
        if(head.get() == get_tail())
            return std::unique_ptr<node>();
        val = std::move(*head->data);
        return pop_head();
    }
};


template<typename T>
void threadsafe_queue<T>::push(T val)
{
    std::shared_ptr<T>new_data(std::make_shared<T>(std::move(val)));
    std::unique_ptr<node>ptr(new node);
    {
        node *const new_tail = ptr.get();
        std::lock_guard<std::mutex>tail_lock(tail_mutex);
        tail->data = new_data;
        // note:unique_ptr(const unique_ptr&) = delete;
        tail->next = std::move(ptr);
        tail = new_tail;
    }
    data_cond.notify_one();
}


template<typename T>
std::shared_ptr<T> threadsafe_queue<T>::wait_and_pop()
{
    const std::unique_ptr<node>old_head = wait_pop_head();
    return old_head->data;
}


template<typename T>
void threadsafe_queue<T>::wait_and_pop(T &val)
{
    const std::unique_ptr<node>old_head = wait_pop_head(val);
}


template<typename T>
bool threadsafe_queue<T>::try_pop(T &val)
{
    const std::unique_ptr<node>old_head = try_pop_head(val);
    return (old_head != nullptr);
}


template<typename T>
std::shared_ptr<T> threadsafe_queue<T>::try_pop()
{
    const std::unique_ptr<node>old_head = try_pop_head();
    return old_head ? old_head->data : std::shared_ptr<T>();
}


template<typename T>
bool threadsafe_queue<T>::empty()
{
    std::lock_guard<std::mutex>head_lock(head_mutex);
    return (head.get() == get_tail());
}
#endif
