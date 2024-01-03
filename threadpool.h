#include <thread>
#include <queue>
#include <vector>
#include <functional>
#include <condition_variable>
#include <mutex>

class ThreadPool{
public:
    ThreadPool(int num);
    ~ThreadPool();

    template <typename F, typename... Args>
    void enqueue(F&& func, Args... args);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

    void workerThread();

};

// create threads, init "stop"
// using lambda to emplace_back 
// because workerThread is member of the class and used from an instantiated object
ThreadPool::ThreadPool(int num): stop(false){
    for (int i = 0; i < num; i++){
        workers.emplace_back([this]{this->workerThread();});
    }
}

// stop the pool, notify all the threads and join them
ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();

    for (auto& worker: workers){
        worker.join();
    }
}

// push the task into queue
// todo: if the queue is full
// notify one thread to do the task
// need to notice:
//   1. use (*task)() to get the function itself, instead of the share_ptr.
//      but emplace((*task)()) will first execute the function task and then emplace.
//      here the result of (*task)() is void, so emplace((*task)()) is equal to emplace(void)
//   2. why not declare task as the comment do?
//      because when not using shared_ptr, the task might be destroyed before it has completed.
template <typename F, typename... Args>
void ThreadPool::enqueue(F&& func, Args... args){
    // auto task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
    auto task = std::make_shared<std::function<void()>>(std::bind(std::forward<F>(func), std::forward<Args>(args)...));
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop){
            // throw std::runtime_error("the pool is stopped.");
            // or not push task in queue any more
            return; 
        }
        // tasks.emplace(task);
        tasks.emplace([task]{(*task)();});
    }
    condition.notify_one();
}

// do the task for all the time
// need to wait until either the pool is stopped or there is task in queue(tasks)
// if the pool is stopped and there is no task in queue, this function can be released
// if there are still tasks in the queue, get it and do it.
// using lambda in "wait" function, because flag "stop" and queue "tasks" belong to a specific instantiated object
void ThreadPool::workerThread(){
    while (true){
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]{return stop || !tasks.empty();});
            if (stop && tasks.empty()){
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}