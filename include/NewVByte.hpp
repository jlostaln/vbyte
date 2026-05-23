#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <iostream>

namespace pfp {

template <class dtype>
class NewVByte {
private:
    int num_values_;
    int bit_block_size_;
    int index_zero_;
    uint64_t bit_block_limit_;

    struct BitArray {
        int block_size_;
        int num_bits_in_word_;
        int modulo_mask;
        std::vector<uint64_t> data_;
        int count = 0;

        BitArray(int bit_block_size = 0)
        : block_size_(bit_block_size) {
            num_bits_in_word_ = sizeof(dtype) * 8;
            modulo_mask = num_bits_in_word_ - 1;
        }

        void ensure_capacity(int entries) {
            uint64_t total_bits = uint64_t(entries) * block_size_;
            uint64_t words_needed = (total_bits + num_bits_in_word_ - 1) / num_bits_in_word_;
            if (words_needed > data_.size())
                data_.resize(words_needed, 0);
        }

        void set(int i, uint64_t val) {
            ensure_capacity(i + 1);

            uint64_t start_position = uint64_t(i) * block_size_;
            uint64_t vector_element = start_position / num_bits_in_word_;
            uint64_t position_in_element = start_position & modulo_mask;
            uint64_t mask = ((uint64_t(1) << block_size_) - 1);
            data_[vector_element] &= ~(mask << position_in_element);
            data_[vector_element] |= (val << position_in_element);
            uint64_t spill = position_in_element + block_size_;
            if (spill > num_bits_in_word_) {
                uint64_t spill_bits = spill - num_bits_in_word_;
                uint64_t spill_mask = (uint64_t(1) << spill_bits) - 1;
                data_[vector_element + 1] &= ~spill_mask;
                data_[vector_element + 1] |= (val >> (block_size_ - spill_bits));
            }

            count++;
        }

        uint64_t get(int i) const {
            uint64_t start_position = uint64_t(i) * block_size_;
            uint64_t vector_element = start_position / num_bits_in_word_;
            uint64_t position_in_element = start_position & modulo_mask;
            uint64_t val = data_[vector_element] >> position_in_element;
            uint64_t spill = position_in_element + block_size_;
            if (spill > num_bits_in_word_) {
                uint64_t spill_bits = spill - num_bits_in_word_;
                uint64_t upper = data_[vector_element + 1] & ((uint64_t(1) << spill_bits) - 1);
                val |= (upper << (block_size_ - spill_bits));
            }
            return val & ((uint64_t(1) << block_size_) - 1);
        }

        void push(uint64_t val) {
            set(count, val);
        }
    };

    std::vector<BitArray> A;
    std::vector<BitArray> B;

    std::vector<std::vector<int>> rankB;
    bool precomputed = false;

    void ensure_layer(size_t layer) {
        if (layer >= A.size()) {
            A.emplace_back(bit_block_size_);
            B.emplace_back(1);
            rankB.emplace_back();
        }
    }

    BitArray A_flat;
    BitArray B_flat;
    std::vector<size_t> layer_offset;
    std::vector<size_t> layer_count;
    std::vector<int> rankB_flat;
    std::vector<size_t> rankB_layer_offset;
    bool flattened = false;

    void flatten() {
        size_t num_layers = A.size();
        layer_offset.resize(num_layers);
        layer_count.resize(num_layers);
        rankB_layer_offset.resize(num_layers);
        size_t sum = 0;
        for (size_t layer = 0; layer < num_layers; layer++) {
            layer_offset[layer] = sum;
            layer_count[layer] = A[layer].count;
            sum += layer_count[layer];
        }

        for (size_t layer = 0; layer < num_layers; layer++) {
            size_t start = layer_offset[layer];
            size_t count = layer_count[layer];
            for (size_t i = 0; i < count; i++) {
                uint64_t a_value = A[layer].get(i);
                uint64_t b_value = B[layer].get(i);
                std::cerr << "start: " << start << std::endl;
                std::cerr << "i: " << i << std::endl;
                A_flat.set(start + i, a_value);
                B_flat.set(start + i, b_value);
                std::cerr << "here start: " << start << std::endl;
                std::cerr << "here i: " << i << std::endl;
            }
        }
        flattened = true;
    }

public:
    NewVByte(dtype num_values, int bit_block_size = 7, int index_zero = 0)
    : num_values_(num_values), bit_block_size_(bit_block_size), index_zero_(index_zero) {
        bit_block_limit_ = uint64_t(1) << bit_block_size_;
    }

    void VByteEncode(dtype i) {
        size_t layer = 0;
        while (true) {
            uint64_t data = i & (bit_block_limit_ - 1);
            bool stop = (i < bit_block_limit_);
            ensure_layer(layer);
            B[layer].push(stop);
            A[layer].push(data);
            if (stop)
                break;
            i = i >> bit_block_size_;
            layer++;
        }
    }

    void precompute() {
        for (size_t layer = 0; layer < B.size(); layer++) {
            int n = B[layer].count;
            rankB[layer].resize(n);
            int sum = 0;
            for (int i = 0; i < n; i++) {
                sum += B[layer].get(i);
                rankB[layer][i] = sum;
            }
        }
        precomputed = true;
        flatten();
    }

    dtype VByteDecode(int i) {
        // if (!flattened) [[ unlikely ]] flatten();
        size_t layer = 0;
        size_t position = i;
        dtype result = 0;
        while (true) {
            bool stop = B[layer].get(position);
            uint64_t data = A[layer].get(position);
            result |= (data << (layer * bit_block_size_));
            if (stop)
                return result;
            int ones = rankB[layer][position];
            position = position - ones;
            layer++;
        }
    }
};

} // namespace pfp

