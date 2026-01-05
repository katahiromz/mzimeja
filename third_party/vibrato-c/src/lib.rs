// vibrato-c: C API bindings for Vibrato morphological analyzer
// C APIバインディング for Vibrato形態素解析エンジン

use std::ffi::{CStr, CString};
use std::fs::File;
use std::io::BufReader;
use std::os::raw::{c_char, c_int};
use std::ptr;
use vibrato::{Dictionary, Tokenizer};

// Since Tokenizer takes ownership of Dictionary, we need to keep them together
struct TokenizerWithDict {
    _dict: Dictionary,
    tokenizer: Tokenizer,
}

/// Opaque pointer to TokenizerWithDict
#[repr(C)]
pub struct VibratoTokenizer {
    _private: [u8; 0],
}

/// Token structure
#[repr(C)]
pub struct VibratoToken {
    pub surface_start: usize,
    pub surface_len: usize,
    pub feature: *const c_char,
    pub range_start: usize,
    pub range_end: usize,
}

/// Load dictionary and create tokenizer in one step
/// Returns NULL on error
#[no_mangle]
pub unsafe extern "C" fn vibrato_tokenizer_load(
    dict_path: *const c_char,
) -> *mut VibratoTokenizer {
    if dict_path.is_null() {
        return ptr::null_mut();
    }

    let c_str = match CStr::from_ptr(dict_path).to_str() {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };

    let file = match File::open(c_str) {
        Ok(f) => f,
        Err(_) => return ptr::null_mut(),
    };

    let reader = BufReader::new(file);
    let dict = match Dictionary::read(reader) {
        Ok(d) => d,
        Err(_) => return ptr::null_mut(),
    };

    let tokenizer = Tokenizer::new(dict);
    
    // We only need to keep the tokenizer since it owns the dictionary
    Box::into_raw(Box::new(tokenizer)) as *mut VibratoTokenizer
}

/// Free tokenizer (and its dictionary)
#[no_mangle]
pub unsafe extern "C" fn vibrato_tokenizer_free(tokenizer: *mut VibratoTokenizer) {
    if !tokenizer.is_null() {
        let _ = Box::from_raw(tokenizer as *mut Tokenizer);
    }
}

/// Tokenize text
/// Returns number of tokens on success, -1 on error
/// The tokens array must be freed with vibrato_tokens_free
#[no_mangle]
pub unsafe extern "C" fn vibrato_tokenize(
    tokenizer: *mut VibratoTokenizer,
    text: *const c_char,
    tokens: *mut *mut VibratoToken,
    num_tokens: *mut usize,
) -> c_int {
    if tokenizer.is_null() || text.is_null() || tokens.is_null() || num_tokens.is_null() {
        return -1;
    }

    let text_str = match CStr::from_ptr(text).to_str() {
        Ok(s) => s,
        Err(_) => return -1,
    };

    let tokenizer_ref = &mut *(tokenizer as *mut Tokenizer);
    
    let mut worker = tokenizer_ref.new_worker();
    worker.reset_sentence(text_str);
    worker.tokenize();

    let mut token_vec = Vec::new();
    
    for i in 0..worker.num_tokens() {
        let token = worker.token(i);
        
        // Get feature string
        let feature_cstr = match CString::new(token.feature()) {
            Ok(s) => s,
            Err(_) => continue,
        };
        
        token_vec.push(VibratoToken {
            surface_start: token.range_char().start,
            surface_len: token.range_char().end - token.range_char().start,
            feature: feature_cstr.into_raw(),
            range_start: token.range_char().start,
            range_end: token.range_char().end,
        });
    }

    let count = token_vec.len();
    *num_tokens = count;
    
    if count > 0 {
        let boxed_slice = token_vec.into_boxed_slice();
        *tokens = Box::into_raw(boxed_slice) as *mut VibratoToken;
    } else {
        *tokens = ptr::null_mut();
    }

    count as c_int
}

/// Free tokens array
#[no_mangle]
pub unsafe extern "C" fn vibrato_tokens_free(tokens: *mut VibratoToken, num_tokens: usize) {
    if tokens.is_null() {
        return;
    }

    // Free feature strings
    for i in 0..num_tokens {
        let token = &*tokens.add(i);
        if !token.feature.is_null() {
            let _ = CString::from_raw(token.feature as *mut c_char);
        }
    }

    // Free tokens array
    let _ = Box::from_raw(std::slice::from_raw_parts_mut(tokens, num_tokens));
}
