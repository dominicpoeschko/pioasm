# PIOasm with Kvasir Output

A copy of the pioasm tool from pico_sdk with added Kvasir output format support. This tool assembles PIO (Programmable I/O) programs for the Raspberry Pi Pico/RP2040/RP2350 microcontrollers and generates C++ headers suitable for use with the [Kvasir](https://github.com/kvasir-io/Kvasir) register access library.

## Features

- Additional `kvasir` output format that generates C++ headers with:
  - Constexpr instruction arrays
  - Program metadata (wrap points, clock dividers, FIFO config, etc.)
  - Symbol definitions for labels and constants
  - Type-safe register access compatible with Kvasir

### CMake Integration

To use pioasm in your CMake project, include this directory and use the `pioasm_generate` function:

```cmake
add_subdirectory(pioasm)

pioasm_generate(my_pio_program
    INPUT_FILE my_program.pio
    PIO_VERSION 1  # optional, defaults to 1
)

target_link_libraries(${PROJECT_NAME} my_pio_program)
```

This will:
- Generate `pioasm_generated/my_pio_program/my_program.hpp` in your build directory
- Create an interface library target `my_pio_program` 
- Automatically include the generated header directory

The generated header can then be included in your code:

```cpp
#include "my_pio_program/my_program.hpp"
```
