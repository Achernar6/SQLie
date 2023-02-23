#pragma once 

#include "stdint.h"

#define LFQ_MAX_SIZE 16

template <typename T>
class LockFreeQueue
{
public:
    LockFreeQueue(uint32_t size): size_(size) { buffer_ = new T[size]; }
    ~LockFreeQueue()
    {
        delete [] buffer_;
        buffer_ = nullptr;
    }

    inline bool isEmpty() const { return head_ == tail_; }
    inline bool isFull()  const { return head_ == (tail_ + 1) % size_; }

    bool push(const T& value)
    {
        if (isFull()) return false;
        buffer_[head_] = value;
        head_ = (head_ + 1) % size_;
        return true;
    }

    bool pop(T& value)
    {
        if (isEmpty()) return false;
        value = buffer_[tail_];
        tail_ = (tail_ + 1) % size_;
        return true;
    }

    inline uint32_t getSize() const { return size_; }
    inline uint32_t getHead() const { return head_; }
    inline uint32_t getTail() const { return tail_; }

private:
    uint32_t size_;
    uint32_t head_ = 0;
    uint32_t tail_ = 0;
    T*       buffer_;

};