// vibrato.h --- C API for Vibrato morphological analyzer
//////////////////////////////////////////////////////////////////////////////
// Vibrato形態素解析エンジンのC API

#ifndef VIBRATO_H
#define VIBRATO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// Opaque tokenizer type (includes dictionary)
typedef struct VibratoTokenizer VibratoTokenizer;

// Token structure
typedef struct VibratoToken {
    size_t surface_start;   // Start position in original text (char index)
    size_t surface_len;     // Length in chars
    const char* feature;    // Part-of-speech feature string (CSV format)
    size_t range_start;     // Start of range (char index)
    size_t range_end;       // End of range (char index)
} VibratoToken;

// Tokenizer functions

/**
 * Load dictionary and create tokenizer
 * @param dict_path UTF-8 encoded dictionary file path
 * @return Tokenizer pointer on success, NULL on error
 */
VibratoTokenizer* vibrato_tokenizer_load(const char* dict_path);

/**
 * Free tokenizer (and its dictionary)
 * @param tokenizer Tokenizer to free
 */
void vibrato_tokenizer_free(VibratoTokenizer* tokenizer);

/**
 * Tokenize text
 * @param tokenizer Tokenizer to use
 * @param text UTF-8 encoded text to tokenize
 * @param tokens Output: array of tokens (must be freed with vibrato_tokens_free)
 * @param num_tokens Output: number of tokens
 * @return Number of tokens on success, -1 on error
 */
int vibrato_tokenize(
    VibratoTokenizer* tokenizer,
    const char* text,
    VibratoToken** tokens,
    size_t* num_tokens
);

/**
 * Free tokens array
 * @param tokens Tokens array to free
 * @param num_tokens Number of tokens
 */
void vibrato_tokens_free(VibratoToken* tokens, size_t num_tokens);

#ifdef __cplusplus
}
#endif

#endif // VIBRATO_H
