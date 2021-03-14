#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <cstddef>
#include <vector>
#include <string>

namespace lem {

template<typename T>
class RingBuffer {
public:

    using base_type = T;
    using data_string = std::basic_string<T>;

    RingBuffer(size_t max_size = 1024) :
        buffer(max_size), read_at(max_size - 1),
        write_at(0)
    {

    }

    size_t enqueue(const data_string &_data)
    {
        for (size_t i = 0; i < _data.size(); ++i) {
            if (write_at != read_at) {
                buffer[write_at++] = _data[i];
            } else {
                return i;
            }
        }

        return _data.size();
    }

    size_t dequeue(const data_string &_data)
    {
        for (size_t i = 0; i < _data.size(); ++i) {

        }
    }



private:
    std::vector<T> buffer;
    size_t read_at;
    size_t write_at;
};

}

#endif // RINGBUFFER_H
