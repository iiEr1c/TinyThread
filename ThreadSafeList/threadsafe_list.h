#include <memory>
#include <mutex>

template<typename T>
class threadsafe_list
{
public:
    threadsafe_list() {}
    ~threadsafe_list()
    {
        remove_if([](const T &){ return true; });
    }
    threadsafe_list(const threadsafe_list &) = delete;
    threadsafe_list &operator=(const threadsafe_list &) = delete;
    void push_front(const T &);

    template<typename Function>void for_each(Function f)
    {
        node *current = &head;
        std::unique_lock<std::mutex>lk(head._mutex);
        while (node *const next = current->next.get())
        {
            std::unique_lock<std::mutex>next_lk(next->_mutex);
            lk.unlock();
            f(*next->data);
            current = next;
            lk = std::move(next_lk);
        }
    }

    template<typename Predicate>
    void remove_if(Predicate p)
    {
        node *current = &head;
        std::unique_lock<std::mutex>lk(head._mutex);
        while(node *const next = current->next.get())
        {
            std::unique_lock<std::mutex>next_lk(next->_mutex);
            if(p(*next->data))
            {
                std::unique_ptr<node> old_next = std::move(current->next);
                current->next = std::move(next->next);
                next_lk.unlock();
            }
            else
            {
                lk.unlock();
                current = next;
                lk = std::move(next_lk);
            }
        }
    }

    template<typename Predicate>
    std::shared_ptr<T>find_first_if(Predicate p)
    {
        node *current = &head;
        std::unique_lock<std::mutex>lk(head._mutex);
        while(node *const next = current->next.get())
        {
            std::unique_lock<std::mutex>next_lk(next->_mutex);
            lk.unlock();
            if(p(*next->data))
                return next->data;
            current = next;
            lk = std::move(next_lk);
        }
        return std::shared_ptr<T>();
    }

private:
    struct node
    {
        std::mutex _mutex;
        std::shared_ptr<T>data;
        std::unique_ptr<node>next;
        node() : next() {}
        node(const T &val) : data(std::make_shared<T>(val)) {}
    };

    node head;
};


template<typename T>
void threadsafe_list<T>::push_front(const T &val)
{
    std::unique_ptr<node>new_node(new node(val));
    std::lock_guard<std::mutex>lk(head._mutex);
    new_node->next = std::move(head.next);
    head.next = std::move(new_node);
}
