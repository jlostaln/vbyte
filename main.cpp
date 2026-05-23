#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "include/NewVByte.hpp"
#include "include/VByte.hpp"
#include "include/timer.hpp"

struct Config {
    bool debug = false;
    bool timing = false;
    bool sorted = false;
    bool generalized = false;
    bool index_query = false;
    int bit_block_size = 7;
};

void help() {
    std::cout << R"(
Help instructions to be added.
    )" << std::endl;
}

template <class query_structure, bool debug = false, bool sorted = false, bool timing = false>
void run_ops(query_structure& qs, std::istream& in, uint64_t n, bool index_query) {
    uint64_t value;

    if constexpr (sorted) {
        {
            pfp::Timer<timing> timer("Insert sorted");
            uint64_t temp;
            in.read(reinterpret_cast<char*>(&temp), sizeof(temp));
            qs.set_first(temp);
            for (int i = 1; i < n; i++) {
                in.read(reinterpret_cast<char*>(&value), sizeof(value));
                qs.VByteEncode(value-temp);

                if constexpr (debug) std::cerr << "Set i:th difference: " << value - temp
                                               << "\nValue: " << value 
                                               << "\nTemp: " << temp 
                                               << "\nMemory in bits: " << qs.memory_in_bits() 
                                               << "\nCount: " << qs.count()
                                               << "\nSize: " << qs.size() << std::endl;
                temp = value;

            }
        }

        std::cerr << qs.count() + 1 << std::endl;

        if (index_query)  {
            {
                uint64_t q;
                in.read(reinterpret_cast<char*>(&q), sizeof(q));

                pfp::Timer<timing> timer("Index query from sorted");
                for (size_t i = 0; i < q; i++) {
                    in.read(reinterpret_cast<char*>(&value), sizeof(value));
                    std::cout << qs.VByteDecodeSorted(value) << std::endl;
                }
            }
        } else {
            {
                pfp::Timer<timing> timer("Decode sorted");
                qs.VByteDecodeSorted();
            }
        }

    } else {
        {
            pfp::Timer<timing> timer("Insert");
            for (int i = 0; i < n; i++) {
                in.read(reinterpret_cast<char*>(&value), sizeof(value));
                qs.VByteEncode(value);

                if constexpr (debug) std::cerr << "Set i:th value: " << value
                                               << "\nMemory in bits: " << qs.memory_in_bits() 
                                               << "\nCount: " << qs.count()
                                               << "\nSize: " << qs.size() << std::endl;
            }
        }
        std::cerr << qs.count() << std::endl;

        if (index_query) {
            {
                uint64_t q;
                in.read(reinterpret_cast<char*>(&q), sizeof(q));

                pfp::Timer<timing> timer("Index query");
                for (size_t i = 0; i < q; i++) {
                    in.read(reinterpret_cast<char*>(&value), sizeof(value));
                    std::cout << qs.VByteDecode(value) << std::endl;
                }
            }

        } else {
            {
                pfp::Timer<timing> timer("Decode");
                qs.VByteDecode();
            }
        }
    }
}

template <class query_structure, bool debug = false, bool timing = false>
void run_query_ops(query_structure& qs, std::istream& in, uint64_t n) {
    uint64_t value;

    {
        pfp::Timer<timing> timer("Insert");
        for (int i = 0; i < n; i++) {
            in.read(reinterpret_cast<char*>(&value), sizeof(value));
            qs.VByteEncode(value);

            // if constexpr (debug) std::cerr << "Set i:th value: " << value
            //                                << "\nMemory in bits: " << qs.memory_in_bits() 
            //                                << "\nCount: " << qs.count()
            //                                << "\nSize: " << qs.size() << std::endl;
        }
    }

    // std::cerr << "After insert" << std::endl;
    qs.precompute();
    // std::cerr << qs.count() << std::endl;
    // std::cerr << "After precompute" << std::endl;
    uint64_t q;
    in.read(reinterpret_cast<char*>(&q), sizeof(q));

    {
        pfp::Timer<timing> timer("Index query");
        for (size_t i = 0; i < q; i++) {
            // std::cerr << "inside query: " << i << std::endl;

            in.read(reinterpret_cast<char*>(&value), sizeof(value));
            std::cout << qs.VByteDecode(value) << std::endl;
        }
    }
}


template<bool timing>
pfp::VByte<uint64_t> make_vbyte(uint64_t n, int bit_block_size) {
    if constexpr (timing) {
        pfp::Timer<timing> constructor_timer("Construction time");
        return pfp::VByte<uint64_t>(n, bit_block_size);
    } else {
        return pfp::VByte<uint64_t>(n, bit_block_size);
    }
}

template <bool debug = false, bool sorted = false, bool timing = false>
void select_operation(Config& cfg, std::istream& in) {
    uint64_t n;
    pfp::Timer<timing> t("Total time");
    in.read(reinterpret_cast<char*>(&n), sizeof(n));

    if constexpr (debug) 
        std::cout << "Debug: " << debug << std::endl
        << "Sorted: " << sorted << std::endl
        << "Time: " << timing << std::endl
        << "Bit block size: " << cfg.bit_block_size << std::endl
        << "Generalized: " << cfg.generalized << std::endl
        << "Index queries: " << cfg.index_query << std::endl
        << "n: " << n << std::endl
        << std::endl;

    if (!cfg.index_query) {
        pfp::VByte<uint64_t> vb = make_vbyte<timing>(n, cfg.bit_block_size);
        run_ops<pfp::VByte<uint64_t>, debug, sorted, timing>(vb, in, n, cfg.index_query);
    } else {
        pfp::NewVByte<uint64_t> nb(n, cfg.bit_block_size);
        run_query_ops<pfp::NewVByte<uint64_t>, debug, timing>(nb, in, n);
    }

}

template <bool timing = false>
void run_program(Config& cfg, std::istream& in) {
    if (cfg.debug) {
        if (cfg.sorted) {
            select_operation<true, true, timing>(cfg, in);
        } else {
            select_operation<true, false, timing>(cfg, in);
        }
    } else {
        if (cfg.sorted) {
            select_operation<false, true, timing>(cfg, in);
        } else {
            select_operation<false, false, timing>(cfg, in);
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
        } else if (s.compare("-n") == 0) {
            cfg.index_query = true;
        } else if (s.compare("-q") == 0) {
            cfg.index_query = true;
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
