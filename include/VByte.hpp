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
    int num_encoded_values_ = 0;
    int first_element_ = 0;
    std::vector<dtype> data_;

    uint64_t mask;
    int modulo_mask;
    int divisor_shift;

public:
    VByte(dtype num_values, int bit_block_size = 7, int index_zero = 0)
    : num_values_(num_values), bit_block_size_(bit_block_size), index_zero_(index_zero) {
        num_bits_in_word_ = sizeof(dtype) * 8;
        data_.assign((num_values * (bit_block_size + 1)), 0);
        modulo_mask = num_bits_in_word_ - 1;
        divisor_shift = __builtin_ctzll(num_bits_in_word_);
        bit_block_limit_ = uint64_t(1) << bit_block_size_;
        mask = ~bit_block_limit_;
    };

    void set_first(dtype element) {
        first_element_ = element;
    }

    void VByteEncode(dtype i) {
        while (true) {
            uint64_t b = i & (bit_block_limit_ - 1);
            if (i < bit_block_limit_) {
                // std::cerr << "value " << i << " < " << bit_block_limit_ << " -> " << b+(bit_block_limit_)
                    // << "( " << b << " + " << bit_block_limit_ << " )" << std::endl; 
                set(b + (bit_block_limit_));
                num_encoded_values_++;
                break;
            }
            // std::cerr << "value " << i << " modulo " << bit_block_limit_ << " = " << b << std::endl; 
            set(b);
            i = i >> bit_block_size_;
        }
    }

    void VByteDecode() {
        dtype result = 0;
        int element = 0;
        int shift = 0;
        dtype val;
        while (element < num_blocks_) {
            dtype i = get(element);
            // std::cerr << "i: " << i << std::endl;
            val = i & ((1 << bit_block_size_) - 1);
            // std::cerr << "val: " << val << std::endl;
            result = val << shift | result;
            // std::cerr << "result: " << result << std::endl;
            if (i & (1 << bit_block_size_)) {
                std::cout << result << std::endl;
                element++;
                result = 0;
                continue;
            }
            shift += bit_block_size_;
            element++;
        }
    }

    void VByteDecodeSorted() {
        dtype result = first_element_;
        int sum = first_element_;
        int element = -1;
        int shift = 0;
        dtype val;
        while (element < num_blocks_) {
            // std::cerr << "Element: " << element << " result: " << result << " at the start of loop" << std::endl;
            if (element == -1) {
                std::cout << result << std::endl;
                element++;
                result = 0;
                continue;
            }
            dtype i = get(element);
            if (result == 0) {
                i += sum;
            }
            // std::cerr << "i: " << i << std::endl;
            val = i & ((1 << bit_block_size_) - 1);
            // std::cerr << "val: " << val << std::endl;
            result = val << shift | result;
            // std::cerr << "result: " << result << std::endl;
            if (i & (1 << bit_block_size_)) {
                std::cout << result << std::endl;
                element++;
                sum = result;
                result = 0;
                continue;
            }
            shift += bit_block_size_;
            element++;
        }
    }

    dtype get(int i) {
        uint64_t start_position = i * (bit_block_size_ + 1);
        uint64_t vector_element = start_position >> divisor_shift;
        uint64_t position_in_element = start_position & modulo_mask;
        uint64_t val = data_[vector_element] >> position_in_element;
        return val;
    }

    void set(dtype val) {
        uint64_t start_position = num_blocks_ * (bit_block_size_ + 1);
        uint64_t vector_element = start_position >> divisor_shift;
        uint64_t block = start_position >> __builtin_ctzll(bit_block_size_ + 1);
        uint64_t position_in_element = start_position & modulo_mask;
        data_[vector_element] &= ~(mask << position_in_element);
        data_[vector_element] |= (val << position_in_element);

        // std::cerr << "push_back value: " << val << std::endl;
        num_blocks_++;
    }

    int count() {
        return num_blocks_;
    }

    int count_encoded() {
        return num_encoded_values_;
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
