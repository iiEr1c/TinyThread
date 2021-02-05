#include "threadsafe_stack.h"
#include <cassert>
#include <cstring>
#include <thread>
#include <future>

void test_single_thread()
{
    threadsafe_stack<int>s1;
    threadsafe_stack<int>s2(s1);
    assert(s1.empty());
    assert(s2.empty());

    int i;
    s1.push(10);
    s1.pop(i);
    assert(i == 10);

    s1.push(20);
    std::shared_ptr<int>ptr = s1.pop();
    assert(ptr.use_count() == 1);
    s1.pop();
}

void test_multi_thread()
{

}

int main()
{
    test_single_thread();
    return 0;
}
