// vibrato_engine.cpp --- Vibrato morphological analysis engine implementation
//////////////////////////////////////////////////////////////////////////////
// (Japanese, UTF-8)
// Vibrato形態素解析エンジンの実装

#include "vibrato_engine.h"
#include <codecvt>
#include <locale>

// Vibratoがコンパイル時に有効な場合のみ、実際のVibrato APIを使用
#ifdef HAVE_VIBRATO
    #include <vibrato.h>
#endif

//////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor

VibratoEngine::VibratoEngine()
    : initialized_(FALSE)
    , tokenizer_(nullptr)
{
}

VibratoEngine::~VibratoEngine()
{
#ifdef HAVE_VIBRATO
    if (tokenizer_) {
        vibrato_tokenizer_free(tokenizer_);
        tokenizer_ = nullptr;
    }
#endif
    initialized_ = FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// Initialization

BOOL VibratoEngine::Initialize(const std::wstring& dict_path)
{
    DPRINTW(L"VibratoEngine::Initialize: %s\n", dict_path.c_str());
    
#ifdef HAVE_VIBRATO
    // Vibratoが有効な場合の初期化処理
    
    // 辞書パスの検証
    if (dict_path.empty()) {
        DPRINTW(L"VibratoEngine: Dictionary path is empty\n");
        return FALSE;
    }
    
    // UTF-8に変換
    std::string dict_path_utf8 = WideToUTF8(dict_path);
    
    // 辞書の読み込みとトークナイザーの初期化
    tokenizer_ = vibrato_tokenizer_load(dict_path_utf8.c_str());
    if (!tokenizer_) {
        DPRINTW(L"VibratoEngine: Failed to load dictionary or create tokenizer\n");
        return FALSE;
    }
    
    initialized_ = TRUE;
    DPRINTW(L"VibratoEngine: Initialized successfully\n");
    return TRUE;
    
#else
    // Vibratoが無効な場合
    DPRINTW(L"VibratoEngine: Vibrato support not compiled in\n");
    return FALSE;
#endif
}

//////////////////////////////////////////////////////////////////////////////
// Conversion methods

BOOL VibratoEngine::AnalyzeToLattice(const std::wstring& text, Lattice& lattice)
{
    if (!initialized_) {
        DPRINTW(L"VibratoEngine::AnalyzeToLattice: Not initialized\n");
        return FALSE;
    }
    
#ifdef HAVE_VIBRATO
    // Vibratoを使用した形態素解析
    
    // テキストをUTF-8に変換
    std::string text_utf8 = WideToUTF8(text);
    
    // Vibratoで形態素解析を実行
    VibratoToken* tokens = nullptr;
    size_t num_tokens = 0;
    
    int result = vibrato_tokenize(tokenizer_, text_utf8.c_str(), &tokens, &num_tokens);
    if (result < 0 || tokens == nullptr) {
        DPRINTW(L"VibratoEngine::AnalyzeToLattice: Tokenization failed\n");
        return FALSE;
    }
    
    // Lattice構造を初期化
    lattice.m_pre = text;
    lattice.m_chunks.clear();
    lattice.m_chunks.resize(text.size() + 1);
    
    // 先頭ノードと末端ノードを作成
    lattice.m_head = LatticeNodePtr(new LatticeNode());
    lattice.m_head->bunrui = HB_HEAD;
    lattice.m_head->deltaCost = 0;
    lattice.m_head->subtotal_cost = 0;
    
    lattice.m_tail = LatticeNodePtr(new LatticeNode());
    lattice.m_tail->bunrui = HB_TAIL;
    lattice.m_tail->deltaCost = 0;
    
    // Vibratoトークンをラティスノードに変換
    for (size_t i = 0; i < num_tokens; ++i) {
        LatticeNodePtr node_ptr(new LatticeNode());
        VibratoTokenToLatticeNode(&tokens[i], text, *node_ptr);
        
        // ノードを適切なチャンクに追加
        size_t start_pos = tokens[i].range_start;
        if (start_pos < lattice.m_chunks.size()) {
            lattice.m_chunks[start_pos].push_back(node_ptr);
        }
    }
    
    // トークンを解放
    vibrato_tokens_free(tokens, num_tokens);
    
    DPRINTW(L"VibratoEngine::AnalyzeToLattice: Success (%zu tokens)\n", num_tokens);
    return TRUE;
    
#else
    return FALSE;
#endif
}

BOOL VibratoEngine::ConvertMultiClause(const std::wstring& text, MzConvResult& result)
{
    if (!initialized_) {
        DPRINTW(L"VibratoEngine::ConvertMultiClause: Not initialized\n");
        return FALSE;
    }
    
#ifdef HAVE_VIBRATO
    // ラティスを作成
    Lattice lattice;
    if (!AnalyzeToLattice(text, lattice)) {
        return FALSE;
    }
    
    // TODO: ビタビアルゴリズムで最適パスを見つけてMzConvResultに変換
    // 現時点では基本的な実装のみ
    
    DPRINTW(L"VibratoEngine::ConvertMultiClause: Not fully implemented\n");
    return FALSE;
#else
    return FALSE;
#endif
}

BOOL VibratoEngine::ConvertSingleClause(const std::wstring& text, MzConvResult& result)
{
    if (!initialized_) {
        DPRINTW(L"VibratoEngine::ConvertSingleClause: Not initialized\n");
        return FALSE;
    }
    
#ifdef HAVE_VIBRATO
    // 単文節変換は文節変換の特殊ケース
    return ConvertMultiClause(text, result);
#else
    return FALSE;
#endif
}

//////////////////////////////////////////////////////////////////////////////
// Private helper methods

void VibratoEngine::VibratoTokenToLatticeNode(const VibratoToken* token, const std::wstring& text, LatticeNode& node)
{
#ifdef HAVE_VIBRATO
    if (!token) return;
    
    // 表層形（変換前）を取得
    size_t start = token->range_start;
    size_t len = token->surface_len;
    if (start + len <= text.size()) {
        node.pre = text.substr(start, len);
        node.post = node.pre;  // デフォルトでは変換後も同じ
    }
    
    // 品詞情報を変換
    if (token->feature) {
        std::string feature_str(token->feature);
        node.bunrui = ConvertPartOfSpeech(token->feature);
        
        // タグとして保存
        node.tags = UTF8ToWide(feature_str);
    } else {
        node.bunrui = HB_UNKNOWN;
    }
    
    // コストの初期化（TODO: 実際のコストを計算）
    node.deltaCost = 100;  // デフォルトコスト
    node.subtotal_cost = MAXLONG;
    node.gyou = GYOU_NN;
    node.katsuyou = SHUUSHI_KEI;
#endif
}

HinshiBunrui VibratoEngine::ConvertPartOfSpeech(const char* pos)
{
#ifdef HAVE_VIBRATO
    // MeCab/Vibratoの品詞をMZ-IMEjaの品詞分類に変換
    // 
    // MeCab IPA辞書の品詞体系（CSV形式）:
    // 品詞,品詞細分類1,品詞細分類2,品詞細分類3,活用型,活用形,原形,読み,発音
    
    if (!pos) return HB_UNKNOWN;
    
    std::string pos_str(pos);
    
    // CSVの最初のフィールドを取得
    size_t comma_pos = pos_str.find(',');
    std::string main_pos = (comma_pos != std::string::npos) 
                          ? pos_str.substr(0, comma_pos) 
                          : pos_str;
    
    // 品詞マッピング
    if (main_pos.find("名詞") != std::string::npos) {
        return HB_MEISHI;
    }
    else if (main_pos.find("動詞") != std::string::npos) {
        // TODO: 五段/一段/カ変/サ変の判定
        return HB_GODAN_DOUSHI;
    }
    else if (main_pos.find("形容詞") != std::string::npos) {
        return HB_IKEIYOUSHI;
    }
    else if (main_pos.find("形容動詞") != std::string::npos) {
        return HB_NAKEIYOUSHI;
    }
    else if (main_pos.find("連体詞") != std::string::npos) {
        return HB_RENTAISHI;
    }
    else if (main_pos.find("副詞") != std::string::npos) {
        return HB_FUKUSHI;
    }
    else if (main_pos.find("接続詞") != std::string::npos) {
        return HB_SETSUZOKUSHI;
    }
    else if (main_pos.find("感動詞") != std::string::npos) {
        return HB_KANDOUSHI;
    }
    else if (main_pos.find("助詞") != std::string::npos) {
        // 助詞の細分類をチェック
        if (pos_str.find("格助詞") != std::string::npos) {
            return HB_KAKU_JOSHI;
        } else if (pos_str.find("接続助詞") != std::string::npos) {
            return HB_SETSUZOKU_JOSHI;
        } else if (pos_str.find("副助詞") != std::string::npos) {
            return HB_FUKU_JOSHI;
        } else if (pos_str.find("終助詞") != std::string::npos) {
            return HB_SHUU_JOSHI;
        }
        return HB_KAKU_JOSHI;  // デフォルト
    }
    else if (main_pos.find("助動詞") != std::string::npos) {
        return HB_JODOUSHI;
    }
    else if (main_pos.find("接頭詞") != std::string::npos) {
        return HB_SETTOUJI;
    }
    else if (main_pos.find("接尾") != std::string::npos) {
        return HB_SETSUBIJI;
    }
    else if (main_pos.find("記号") != std::string::npos) {
        if (pos_str.find("句点") != std::string::npos) {
            return HB_PERIOD;
        } else if (pos_str.find("読点") != std::string::npos) {
            return HB_COMMA;
        }
        return HB_SYMBOL;
    }
    
    return HB_UNKNOWN;
#else
    return HB_UNKNOWN;
#endif
}

//////////////////////////////////////////////////////////////////////////////
// UTF-8/UTF-16 conversion utilities

std::string VibratoEngine::WideToUTF8(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    
#if defined(_MSC_VER) && _MSC_VER >= 1900
    // Visual Studio 2015以降
    // Note: std::wstring_convert is deprecated in C++17 and removed in C++20
    // This code uses it for backward compatibility. When upgrading to C++20,
    // use Windows APIs directly (WideCharToMultiByte) or a modern conversion library.
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    try {
        return converter.to_bytes(wstr);
    } catch (...) {
        return std::string();
    }
#else
    // それ以前のコンパイラまたはGCC/Clang
    // WideCharToMultiByte を使用（Windows専用）
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), 
                                          (int)wstr.size(), NULL, 0, NULL, NULL);
    if (size_needed <= 0) return std::string();
    
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), 
                       &result[0], size_needed, NULL, NULL);
    return result;
#endif
}

std::wstring VibratoEngine::UTF8ToWide(const std::string& str)
{
    if (str.empty()) return std::wstring();
    
#if defined(_MSC_VER) && _MSC_VER >= 1900
    // Visual Studio 2015以降
    // Note: std::wstring_convert is deprecated in C++17 and removed in C++20
    // This code uses it for backward compatibility. When upgrading to C++20,
    // use Windows APIs directly (MultiByteToWideChar) or a modern conversion library.
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    try {
        return converter.from_bytes(str);
    } catch (...) {
        return std::wstring();
    }
#else
    // それ以前のコンパイラまたはGCC/Clang
    // MultiByteToWideChar を使用（Windows専用）
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), 
                                          (int)str.size(), NULL, 0);
    if (size_needed <= 0) return std::wstring();
    
    std::wstring result(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), 
                       &result[0], size_needed);
    return result;
#endif
}
