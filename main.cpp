#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "include/VByte.hpp"
#include "include/timer.hpp"

struct Config {
    bool debug = false;
    bool timing = false;
    bool sorted = false;
    bool generalized = false;
    int bit_block_size = 7;
};

void help() {
    std::cout << R"(
Help instructions to be added.
    )" << std::endl;
}

template <class query_sturcure, bool debug = false, bool sorted = false, bool timing = false>
void run_ops(query_sturcure& qs, std::istream& in, uint64_t n) {
    uint64_t value;

    if constexpr (sorted) {
        uint64_t temp;
        in.read(reinterpret_cast<char*>(&temp), sizeof(temp));
        qs.set_first(temp);
        for (int i = 1; i < n; i++) {
            in.read(reinterpret_cast<char*>(&value), sizeof(value));
            qs.VByteEncode(value-temp);

            if constexpr (debug) std::cerr << "Set i:th difference: " << value - temp
                                           << "\nMemory in bits: " << qs.memory_in_bits() 
                                           << "\nCount: " << qs.count()
                                           << "\nSize: " << qs.size() << std::endl;
        }

        std::cerr << qs.count() << std::endl;
        qs.VByteDecodeSorted();
    } else {
        for (int i = 0; i < n; i++) {
            in.read(reinterpret_cast<char*>(&value), sizeof(value));
            qs.VByteEncode(value);

            if constexpr (debug) std::cerr << "Set i:th value: " << value
                                           << "\nMemory in bits: " << qs.memory_in_bits() 
                                           << "\nCount: " << qs.count()
                                           << "\nSize: " << qs.size() << std::endl;
        }

        std::cerr << qs.count() << std::endl;
        qs.VByteDecode();
    }
}

template <bool debug = false, bool sorted = false, bool timing = false>
void select_operation(int bit_block_size, bool generalized, std::istream& in) {
    uint64_t n;

    in.read(reinterpret_cast<char*>(&n), sizeof(n));

    if constexpr (debug) 
        std::cout << "Debug: " << debug << std::endl
        << "Sorted: " << sorted << std::endl
        << "Time: " << timing << std::endl
        << "Bit block size: " << bit_block_size << std::endl
        << "Generalized: " << generalized << std::endl
        << "n: " << n << std::endl
        << std::endl;

    pfp::VByte<uint64_t> vb(n, bit_block_size);
    run_ops<pfp::VByte<uint64_t>, debug, sorted, timing>(vb, in, n);

}

template <bool timing = false>
void run_program(Config& cfg, std::istream& in) {
    if (cfg.debug) {
        if (cfg.sorted) {
            select_operation<true, true, timing>(cfg.bit_block_size, cfg.generalized, in);
        } else {
            select_operation<true, false, timing>(cfg.bit_block_size, cfg.generalized, in);
        }
    } else {
        if (cfg.sorted) {
            select_operation<false, true, timing>(cfg.bit_block_size, cfg.generalized, in);
        } else {
            select_operation<false, false, timing>(cfg.bit_block_size, cfg.generalized, in);
        }
    }

}

int main(int argc, char const* argv[]) {

    Config cfg;
    int input_file = 0;
    int i = 1;
    while (i < argc) {
        std::string s(argv[i++]);
        if (s.compare("-s") == 0) {
            cfg.sorted = true;
        } else if (s.compare("-k") == 0) {
            cfg.generalized = true;
            cfg.bit_block_size = std::stoull(argv[i++]);
        } else if (s.compare("-h") == 0) {
            help();
            exit(0);
        } else if (s.compare("-d") == 0) {
            cfg.debug = true;
        } else if (s.compare("-t") == 0) {
            cfg.timing = true;
        } else {
            input_file = i - 1;
        }
    }

    if (input_file > 0) {
        std::ifstream in(argv[input_file]);
        if (cfg.timing) {
            run_program<true>(cfg, in);
        } else {
            run_program<false>(cfg, in);
        }
    } else {
        if (cfg.timing) {
            run_program<true>(cfg, std::cin);
        } else {
            run_program<false>(cfg, std::cin);
        }
    }
    return 0;
}
