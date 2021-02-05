#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H

#include <memory>
#include <mutex>

template<typename T>
class threadsafe_queue
{
public:
    // pay attention to class member's initialization order:'head' is ahead of 'tail'
    threadsafe_queue() : head(new node), tail(head.get()) {}
    threadsafe_queue(const threadsafe_queue &) = delete;
    threadsafe_queue &operator=(const threadsafe_queue &) = delete;
    std::shared_ptr<T>try_pop()
    {
        std::unique_ptr<node>ole_head = pop_head();
        return ole_head ? ole_head->data : std::shared_ptr<T>();
    }
    void push(T val)
    {
        std::shared_ptr<T>new_data = std::make_shared<T>(std::move(val));
        std::unique_ptr<T>ptr(new node);
        node *const new_tail = ptr.get();
        std::lock_guard<std::mutex>tail_lock(tail_mutex);
        tail->data = new_data;
        // note:unique_ptr(const unique_ptr&) = delete;
        tail->next = std::move(ptr);
        tail = new_tail;
    }
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
    node *get_tail()
    {
        std::lock_guard<std::mutex>tail_lock(tail_mutex);
        return tail;
    }

    std::unique_ptr<node>pop_head()
    {
        std::lock_guard<std::mutex>head_lock(head_mutex);
        if(head.get() == get_tail())
            return nullptr;
        std::unique_ptr<node>old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
};

#endif
