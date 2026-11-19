#pragma once
#include <cstdint>

typedef uint8_t Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef uint64_t Uint64;

typedef int8_t Sint8;
typedef int16_t Sint16;
typedef int32_t Sint32;
typedef int64_t Sint64;

typedef unsigned int uint;

const float tol = 1e-6f;
const float tolSq = 1e-12f;
const float NaN = 1.0f / 0.0f;

#pragma pack(push, 1)

struct RGB {
  Uint8 r;
  Uint8 g;
  Uint8 b;
};

struct RGBA {
  union {
    struct {
      Uint8 b;
      Uint8 g;
      Uint8 r;
      Uint8 a;
    };
    Uint8 rgba[4];
  };
};
#pragma pack(pop)

template <class T>
struct Array {
  int size = 0;
  T* data = nullptr;
  Array(int size) : size(size), data(new T[size]) {}
  ~Array() {
    if (data) delete[] data;
    data = nullptr;
    size = 0;
  }
  T& operator[](int index) {
    index = index < 0 ? 0 : (index >= size ? size - 1 : index);
    return data[index];
  }
  const T& operator[](int index) const {
    index = index < 0 ? 0 : (index >= size ? size - 1 : index);
    return data[index];
  }
};
