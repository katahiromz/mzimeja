# vibrato-c

C API bindings for the Vibrato morphological analyzer.

## Overview

This library provides C-compatible FFI bindings for the [Vibrato](https://github.com/daac-tools/vibrato) Rust library, enabling its use in C/C++ projects like MZ-IME Japanese Input.

## Building

### Prerequisites

- Rust 1.70 or later
- Cargo

### Build Instructions

```bash
cd third_party/vibrato-c
cargo build --release
```

The static library will be created at `target/release/libvibrato_c.a`.

## API

### Types

- `VibratoTokenizer`: Opaque tokenizer handle (includes dictionary)
- `VibratoToken`: Token structure with morphological information

### Functions

#### `VibratoTokenizer* vibrato_tokenizer_load(const char* dict_path)`

Load dictionary and create tokenizer.

- **Parameters**: 
  - `dict_path`: UTF-8 encoded path to Vibrato dictionary file
- **Returns**: Tokenizer pointer on success, NULL on error

#### `void vibrato_tokenizer_free(VibratoTokenizer* tokenizer)`

Free tokenizer and its dictionary.

- **Parameters**:
  - `tokenizer`: Tokenizer to free

#### `int vibrato_tokenize(VibratoTokenizer* tokenizer, const char* text, VibratoToken** tokens, size_t* num_tokens)`

Tokenize UTF-8 text.

- **Parameters**:
  - `tokenizer`: Tokenizer to use
  - `text`: UTF-8 encoded text to tokenize
  - `tokens`: Output pointer to token array
  - `num_tokens`: Output number of tokens
- **Returns**: Number of tokens on success, -1 on error

#### `void vibrato_tokens_free(VibratoToken* tokens, size_t num_tokens)`

Free token array.

- **Parameters**:
  - `tokens`: Token array to free
  - `num_tokens`: Number of tokens

## Usage Example

```c
#include <vibrato.h>
#include <stdio.h>

int main() {
    // Load dictionary and create tokenizer
    VibratoTokenizer* tokenizer = vibrato_tokenizer_load("path/to/dict.vibrato");
    if (!tokenizer) {
        fprintf(stderr, "Failed to load dictionary\n");
        return 1;
    }
    
    // Tokenize text
    VibratoToken* tokens = NULL;
    size_t num_tokens = 0;
    int result = vibrato_tokenize(tokenizer, "こんにちは世界", &tokens, &num_tokens);
    
    if (result >= 0) {
        for (size_t i = 0; i < num_tokens; i++) {
            printf("Token %zu: feature=%s\n", i, tokens[i].feature);
        }
        vibrato_tokens_free(tokens, num_tokens);
    }
    
    vibrato_tokenizer_free(tokenizer);
    return 0;
}
```

## License

Same as Vibrato: MIT OR Apache-2.0
