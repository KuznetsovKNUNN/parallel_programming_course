// Copyright 2019 Kuznetsov Konstantin
// поразрядная сортировка целых положительных чисел
// сортировка всего массива одним потоком

#include <omp.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

void print_data(const std::vector<unsigned int>& data, size_t n = 10) {
    if (data.size() < n) return;

    for (size_t i = 0; i < n; ++i)
        std::cout << data[i] << " ";
    std::cout << std::endl;
}

static std::vector<unsigned int> generate_data(size_t size) {
    using value_type = unsigned int;

    static std::uniform_int_distribution<value_type> distribution(
                std::numeric_limits<value_type>::min(),
                std::numeric_limits<value_type>::max());
    static std::default_random_engine generator;

    std::vector<value_type> data(size);
    std::generate(data.begin(), data.end(), []() {
        return distribution(generator); });

    return data;
}

void create_counters(unsigned int* data,
                     std::vector<std::vector<unsigned int>> counters,
                     size_t size) {
    unsigned char* byte_ptr = reinterpret_cast<unsigned char*>(data);
    unsigned char* data_end = reinterpret_cast<unsigned char*>(data + size);

    size_t i;
    while (byte_ptr != data_end)
        for (i = 0; i < sizeof(int); i++)
            counters[i][*byte_ptr++]++;
}

void byte_sort(unsigned int* data, unsigned int* temp,
               std::vector<unsigned int> counter, size_t byte, size_t size) {
    unsigned char* byte_ptr;
    unsigned int c = 0;

    unsigned int sum = 0;
    for (size_t i = 0; i < 256; ++i) {
        c = counter[i];
        counter[i] = sum;
        sum += c;
    }

    byte_ptr = reinterpret_cast<unsigned char*>(data) + byte;
    for (size_t i = 0; i < size; i++, byte_ptr += sizeof(int))
        temp[counter[*byte_ptr]++] = data[i];
}

// поразрядная сортировка целых положительных чисел
// сортировка выполняется по байтам
// для сортировки по каждому байту используется сортировка подсчетом
void radix_sort(unsigned int* data, size_t size) {
    unsigned int* temp = new unsigned int[size];
    std::vector<std::vector<unsigned int>> counters(
                sizeof(int), std::vector<unsigned int>(256, 0));

    create_counters(data, counters, size);
    for (size_t i = 0; i < sizeof(int); ++i) {
        byte_sort(data, temp, counters[i], i, size);
        std::swap(data, temp);
    }

    delete[] temp;
}

int main(int argc, char* argv[]) {
    size_t data_size = 1000000;
    double t1, t2;
    std::vector<unsigned int> data = generate_data(data_size);

    t1 = omp_get_wtime();
    radix_sort(data.data(), data_size);
    t2 = omp_get_wtime();
    std::cout << "non-parallel time = " << t2 - t1 << std::endl;
    print_data(data);

    return 0;
}

