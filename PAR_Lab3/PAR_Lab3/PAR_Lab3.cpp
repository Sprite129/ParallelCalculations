#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <random>
#include <chrono>

using namespace std;
using namespace std::chrono;

class Task 
{
public:
    int taskId;
    int taskDuration;

    high_resolution_clock::time_point taskCreatedTime;

    Task(int taskId, int taskDuration) : taskId(taskId), taskDuration(taskDuration) 
    {
        taskCreatedTime = high_resolution_clock::now();
    }

    void executeTask() const 
    {
        this_thread::sleep_for(seconds(taskDuration));
    }
};

class TaskQueue 
{
private:
    queue<Task> taskQueue;

    condition_variable queueCV;
    mutable mutex queueMutex;
    atomic<int> queueTotalDuration{ 0 };

    bool isShutdownFlag = false;

public:
    bool tryPushTask(Task task) 
    {
        lock_guard<mutex> lock(queueMutex);
        if (queueTotalDuration + task.taskDuration > 45)
            return false;

        taskQueue.push(task);
        queueTotalDuration += task.taskDuration;
        queueCV.notify_one();
        return true;
    }

    bool popTask(Task& task) 
    {
        unique_lock<mutex> lock(queueMutex);
        queueCV.wait(lock, [&]() { return !taskQueue.empty() || isShutdownFlag; });
        if (isShutdownFlag && taskQueue.empty()) return false;

        task = taskQueue.front();
        taskQueue.pop();
        queueTotalDuration -= task.taskDuration;
        return true;
    }

    void shutdownQueue() 
    {
        lock_guard<mutex> lock(queueMutex);
        isShutdownFlag = true;
        queueCV.notify_all();
    }

    int getTotalDuration() const 
    {
        return queueTotalDuration;
    }

    int getSize() 
    {
        lock_guard<mutex> lock(queueMutex);
        return taskQueue.size();
    }
};

class ThreadPool 
{
private:
    vector<thread> workerThreads;
    TaskQueue taskQueueOne, taskQueueTwo;

    mutable mutex timeMutex;
    mutex pauseMutex;
    condition_variable pauseCv;

    atomic<bool> isShutdown{ false };
    atomic<bool> isPaused{ false };
    atomic<int> taskIdCounter{ 0 };
    atomic<int> rejectedTaskCount{ 0 };
    atomic<int> completedTaskCount{ 0 };

    double totalExecutionTime = 0.0;
    double totalWaitingTime = 0.0;

    void workerThreadLoop(TaskQueue& queueRef, int workerId) 
    {
        while (!isShutdown) 
        {
            Task currentTask(0, 0);
            if (!queueRef.popTask(currentTask)) break;

            {
                unique_lock<mutex> lock(pauseMutex);
                pauseCv.wait(lock, [&]() { return !isPaused || isShutdown; });
                if (isShutdown) break;
            }

            double waitingTime = duration_cast<duration<double>>(high_resolution_clock::now() - currentTask.taskCreatedTime).count();

            auto startTime = high_resolution_clock::now();
            cout << "[Worker " << workerId << "] Task " << currentTask.taskId << " started, duration " << currentTask.taskDuration << "s\n";
            currentTask.executeTask();
            auto endTime = high_resolution_clock::now();

            double executionTime = duration_cast<duration<double>>(endTime - startTime).count();

            completedTaskCount++;

            {
                lock_guard<mutex> lock(timeMutex);
                totalExecutionTime += executionTime;
                totalWaitingTime += waitingTime;
            }

            cout << "[Worker " << workerId << "] Task " << currentTask.taskId << " completed\n";
        }
    }

public:
    ThreadPool() 
    {
        for (int i = 0; i < 2; ++i) 
        {
            workerThreads.emplace_back([this, i] { workerThreadLoop(taskQueueOne, i); });
            workerThreads.emplace_back([this, i] { workerThreadLoop(taskQueueTwo, i + 2); });
        }
    }

    ~ThreadPool() 
    {
        shutdownPool();
    }

    bool submitTask() 
    {
        int taskDuration = 4 + rand() % 7;
        Task newTask(taskIdCounter++, taskDuration);

        bool isAccepted = false;
        if (taskQueueOne.getTotalDuration() <= taskQueueTwo.getTotalDuration()) 
        {
            isAccepted = taskQueueOne.tryPushTask(newTask);
        }
        else 
        {
            isAccepted = taskQueueTwo.tryPushTask(newTask);
        }

        if (!isAccepted) 
        {
            cout << "[Task " << newTask.taskId << "] Rejected (completion time too long)\n";
            rejectedTaskCount++;
        }

        return isAccepted;
    }

    void pausePool() 
    {
        isPaused = true;
        cout << "ThreadPool paused\n";
    }

    void resumePool() 
    {
        {
            lock_guard<mutex> lock(pauseMutex);
            isPaused = false;
        }
        pauseCv.notify_all();
        cout << "ThreadPool resumed\n";
    }

    void shutdownPool() 
    {
        isShutdown = true;
        taskQueueOne.shutdownQueue();
        taskQueueTwo.shutdownQueue();
        pauseCv.notify_all();

        for (auto& t : workerThreads)
            if (t.joinable()) t.join();
    }

    void printStats() const 
    {
        cout << "\n--- Statistics ---\n";
        cout << "Completed tasks: " << completedTaskCount << endl;
        cout << "Rejected tasks: " << rejectedTaskCount << endl;

        if (completedTaskCount > 0) 
        {
            lock_guard<mutex> lock(timeMutex);
            cout << "Avg execution time: " << totalExecutionTime / completedTaskCount << " s\n";
            cout << "Avg wait time: " << totalWaitingTime / completedTaskCount << " s\n";
        }
        cout << "---------------\n";
    }
};

int main() {
    srand(time(NULL));

    ThreadPool threadPool;
    vector<thread> submitterThreads;

    for (int i = 0; i < 3; i++) 
    {
        submitterThreads.emplace_back([&threadPool, i] {
            for (int j = 0; j < 10; ++j) {
                this_thread::sleep_for(milliseconds(300 + rand() % 400));
                threadPool.submitTask();
            }
            });
    }

    this_thread::sleep_for(seconds(5));
    threadPool.pausePool();

    this_thread::sleep_for(seconds(5));
    threadPool.resumePool();

    for (auto& t : submitterThreads) 
    {
        if (t.joinable()) t.join();
    }

    this_thread::sleep_for(seconds(45));
    threadPool.shutdownPool();
    threadPool.printStats();

    return 0;
}
