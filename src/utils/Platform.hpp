#pragma once

// Portabilidade minima Linux/Windows (MinGW-w64 UCRT64).
//
// A engine original so' compilava em Linux. O fork precisa rodar tambem no
// Windows para validar o SSAO em GPU real; estas funcoes cobrem as poucas
// chamadas POSIX/C11 que o MinGW nao oferece. Em Linux o comportamento e' o
// mesmo de antes (std::aligned_alloc, std::free, setenv).

#include <cstddef>
#include <cstdlib>

#ifdef _WIN32
#include <malloc.h>
#endif

namespace eruption::platform {

// `size` precisa ser multiplo de `alignment` (mesma regra do aligned_alloc).
inline void* alignedAlloc(std::size_t alignment, std::size_t size) {
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    return std::aligned_alloc(alignment, size);
#endif
}

inline void alignedFree(void* ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

inline void setEnv(const char* name, const char* value) {
#ifdef _WIN32
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}

} // namespace eruption::platform
