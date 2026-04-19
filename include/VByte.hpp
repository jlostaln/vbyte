#pragma once

#include <cstdint>
#include <vector>
namespace pfp {

template <class dtype>
class VByte {
private:
    int num_values_;
    int bit_block_size_;
    int num_blocks_ = 0;
    std::vector<dtype> data_;

public:
    VByte(dtype num_values, int bit_block_size = 7)
    : num_values_(num_values), bit_block_size_(bit_block_size) {
        data_.reserve(num_values);
    };
};

} // namespace pfp
