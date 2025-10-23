# CMake AVR Build System

This CMake configuration allows you to build C++ projects for both AVR microcontrollers (like Arduino Uno) and native platforms.

## Prerequisites

- Custom AVR toolchain installed at `/Users/alexpoulin/local/avr/`
- CMake 3.10 or later

## Building for AVR (Default)

```bash
mkdir build
cd build
cmake ..
make
```

## Building for Native Platform

```bash
mkdir build-native
cd build-native
cmake -DBUILD_FOR_AVR=OFF ..
make
```

## Using the Toolchain File (Alternative)

```bash
mkdir build-avr
cd build-avr
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/avr-toolchain.cmake ..
make
```

## Available Targets

### For AVR builds:

- `make` - Build the ELF executable
- `make hex` - Generate Intel HEX file for programming
- `make size` - Show program memory usage
- `make listing` - Generate assembly listing
- `make upload` - Upload to Arduino Uno (if connected)

### For Native builds:

- `make` - Build native executable

## Project Structure

```
cpp/
├── CMakeLists.txt          # Main CMake configuration
├── cmake/
│   └── avr-toolchain.cmake # AVR toolchain file
├── include/
│   └── *.h                 # Header files
├── src/
│   └── *.cpp               # Source files
└── build/                  # Build directory (created)
```

## Configuration Options

- `BUILD_FOR_AVR` (ON/OFF) - Build for AVR or native platform
- `MCU` - Target microcontroller (default: atmega328p)
- `F_CPU` - CPU frequency (default: 16000000UL)
- `AVR_TOOLCHAIN_PATH` - Path to AVR toolchain

## Examples

### Build and upload to Arduino:
```bash
mkdir build && cd build
cmake ..
make hex
make upload
```

### Check program size:
```bash
make size
```

### Generate assembly listing:
```bash
make listing
```

## Troubleshooting

1. **Toolchain not found**: Ensure AVR toolchain is installed at `/Users/alexpoulin/local/avr/`
2. **Upload fails**: Check Arduino connection and port (`/dev/cu.usbmodem*`)
3. **Build errors**: Verify source files exist in `src/` directory

## Customization

You can customize the build by setting CMake variables:

```bash
cmake -DMCU=atmega2560 -DF_CPU=16000000UL ..
```

This configuration provides the same functionality as your Makefile but with the flexibility of CMake!