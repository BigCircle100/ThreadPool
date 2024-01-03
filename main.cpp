#include <iostream>
#include "threadpool.h"

int main(){
    ThreadPool pool(4);
    for (int i = 0; i < 100; i++){
        pool.enqueue([i]{std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Task " << i << " completed by thread " << std::this_thread::get_id() << std::endl;});
    }
    return 0;
}