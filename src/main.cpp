#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iomanip>


struct msg {
    std::size_t id;
    double  msg;
    bool finish;
};

std::size_t numThreads ;
std::queue<msg> shared_queue;
std::mutex mtx;
std::condition_variable cv;


 
void producer()
{
    msg myMsg;
    myMsg.id = std::hash<std::thread::id>{}(std::this_thread::get_id());
    myMsg.finish = false;
    double calcVal = myMsg.id;
  
    while(calcVal >= 1)
    {
            calcVal = calcVal / 10;
            myMsg.msg = calcVal;
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            shared_queue.push(myMsg);
        }

        cv.notify_all();

       std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate some work time
    }

    myMsg.finish = true;
    {
        std::lock_guard<std::mutex> lock(mtx);
        shared_queue.push(myMsg);
    }
    cv.notify_all();
}

// Function for the consumer thread
void consumer()
{

    int countFinish = 0;

    //nums after point in cout
    int precision = 7;

    while (countFinish <  numThreads)
    {

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return  !shared_queue.empty(); });

        msg msg = shared_queue.front();
        shared_queue.pop();

        lock.unlock();

        if (msg.finish)
        {
            std::cout << "Producer " <<  msg.id << " finished "  << std::endl;
            ++countFinish;
            continue;
        }


        std::cout << "Producer " << msg.id << " produced " << std::fixed << std::setprecision(precision) << msg.msg << std::endl;
    }   
}

int main()
{

    numThreads = 2;
    std::vector<std::thread> threads;

    // Create threads
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(producer);
    }

    std::thread consumer_thread(consumer);

    for (auto& thread : threads)
    {
        thread.join();
    }
    consumer_thread.join();

    return 0;
}
