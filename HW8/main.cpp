#include <iostream>
#include <vector>
#include <semaphore>
#include <thread>

class CircularBuffer {
private:
    std::vector<int> buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    std::shared_ptr<std::counting_semaphore<>> emptySlots;
    std::shared_ptr<std::counting_semaphore<>> fullSlots;
    std::shared_ptr<std::mutex> mutexWriter;
    std::shared_ptr<std::mutex> mutexReader;

public:
    CircularBuffer(size_t size) : capacity(size),
                                 buffer(size),
                                 head(0),
                                 tail(0),
                                 emptySlots(std::make_shared<std::counting_semaphore<>>(size)),
                                 fullSlots(std::make_shared<std::counting_semaphore<>>(0)),
                                 mutexWriter(std::make_shared<std::mutex>()),
                                 mutexReader(std::make_shared<std::mutex>()) {}

    void write(int item) {
        emptySlots->acquire();
        std::lock_guard<std::mutex> lock(mutexWriter);

        buffer[tail] = item;
        tail = (tail + 1) % capacity;

        fullSlots->release();
    }

    int read() {
        fullSlots->acquire();
        std::lock_guard<std::mutex> lock(mutexReader);

        int item = buffer[head];
        head = (head + 1) % capacity;

        emptySlots->release();
        return item;
    }
};

void writer(CircularBuffer& cb) {
    for (int i = 0; i < 100; ++i) {
        cb.write(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void reader(CircularBuffer& cb, int readerId) {
    for (int i = 0; i < 50; ++i) {
        int item = cb.read();
        std::cout << "Reader " << readerId << " read: " << item << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    CircularBuffer cb(10);

    std::thread writerThread(writer, std::ref(cb));
    std::thread readerThread1(reader, std::ref(cb), 1);
    std::thread readerThread2(reader, std::ref(cb), 2);

    writerThread.join();
    readerThread1.join();
    readerThread2.join();

    return 0;
}