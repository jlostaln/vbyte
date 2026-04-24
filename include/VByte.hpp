#pragma once

#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>
namespace pfp {

template <class dtype>
class VByte {
private:
    int num_values_;
    int bit_block_size_;
    int index_zero_;
    int num_bits_in_word_;
    uint64_t bit_block_limit_;
    int num_blocks_ = 0;
    std::vector<dtype> data_;

    uint64_t mask;
    int modulo_mask;
    int divisor_shift;

public:
    VByte(dtype num_values, int bit_block_size = 7, int index_zero = 0)
    : num_values_(num_values), bit_block_size_(bit_block_size), index_zero_(index_zero) {
        num_bits_in_word_ = sizeof(dtype) * 8;
        // data_.reserve((num_values * (bit_block_size + 1) + num_bits_in_word_ - 1) / num_bits_in_word_);
        data_.assign(((num_values * (bit_block_size + 1) + num_bits_in_word_ - 1) / num_bits_in_word_), 0);
        modulo_mask = num_bits_in_word_ - 1;
        divisor_shift = __builtin_ctzll(num_bits_in_word_);
        bit_block_limit_ = uint64_t(1) << bit_block_size_;
        mask = ~bit_block_limit_;
    };

    void VByteEncode(dtype i) {
        while (true) {
            uint64_t b = i & (bit_block_limit_ - 1);
            if (i < bit_block_limit_) {
                std::cerr << "value " << i << " < " << bit_block_limit_ << " -> " << b+(bit_block_limit_)
                    << "( " << b << " + " << bit_block_limit_ << " )" << std::endl; 
                set(b + (bit_block_limit_));
                break;
            }
            std::cerr << "value " << i << " modulo " << bit_block_limit_ << " = " << b << std::endl; 
            set(b);
            i = i >> bit_block_size_;
        }
    }

    dtype VByteDecode(int idx) {
        dtype i = get(idx);
        std::cerr << "Value " << i << ", mask " << mask << std::endl;
        return i & ((1 << bit_block_size_) - 1);
    }

    dtype get(int i) {
        uint64_t start_position = i * (bit_block_size_ + 1);
        uint64_t vector_element = start_position / num_bits_in_word_;
        uint64_t position_in_element = start_position % num_bits_in_word_;
        uint64_t val = data_[vector_element] >> position_in_element;
        std::cerr << "sp " << start_position << " ve " << vector_element << " pos " << position_in_element << std::endl;
        return val;
    }
    // uint64_t get(uint64_t i) {
    //     uint64_t start_position = i * bits_per_int;
    //     uint64_t word = start_position / word_in_bits;
    //     uint64_t position_in_word = start_position % word_in_bits;
    //     uint64_t val = parray[word] >> position_in_word;
    //     uint64_t spill = position_in_word + bits_per_int;
    //     if (spill > word_in_bits) {
    //         uint64_t spill_bits = spill - word_in_bits;
    //         uint64_t upper = parray[word + 1] & ((uint64_t(1) << spill_bits) - 1);
    //         val |= (upper << (bits_per_int - spill_bits));
    //     }
    //     return val & mask;
    // }

    void set(dtype val) {
        uint64_t start_position = num_blocks_ * (bit_block_size_ + 1);
        uint64_t vector_element = start_position / num_bits_in_word_;
        uint64_t block = start_position / (bit_block_size_ + 1);
        uint64_t position_in_element = start_position % num_bits_in_word_;
        data_[vector_element] &= ~(mask << position_in_element);
        data_[vector_element] |= (val << position_in_element);

        std::cerr << "push_back value: " << val << std::endl;
        num_blocks_++;
    }

    // void set(uint64_t i, uint64_t val) {
    //     val &= mask;
    //
    //     uint64_t start_position = i * bits_per_int;
    //     uint64_t word = start_position / word_in_bits;
    //     uint64_t position_in_word = start_position % word_in_bits;
    //
    //     parray[word] &= ~(mask << position_in_word);
    //     parray[word] |= (val << position_in_word);
    //     uint64_t spill = position_in_word + bits_per_int;
    //     if (spill > word_in_bits) {
    //         uint64_t spill_bits = spill - word_in_bits;
    //         uint64_t spill_mask = (uint64_t(1) << spill_bits) - 1;
    //         parray[word + 1] &= ~spill_mask;
    //         parray[word + 1] |= (val >> (bits_per_int - spill_bits));
    //     }
    // }

    int count() {
        return num_blocks_;
    }

    int size() {
        return data_.size();
    }

    int memory_in_bits() {
        std::cerr << "size: " << data_.size() << " elements, word in bytes: " << sizeof(dtype) << std::endl;
        return data_.size() * 8;
    }
};

} // namespace pfp
