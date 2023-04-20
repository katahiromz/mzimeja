// mzimeja.h --- MZ-IME Japanese Input (mzimeja)
//////////////////////////////////////////////////////////////////////////////
// (Japanese, UTF-8)

#pragma once

#define _CRT_SECURE_NO_WARNINGS   // use fopen

#include "../targetver.h"   // target Windows version

#ifndef _INC_WINDOWS
    #include <windows.h>    // Windows
#endif
#include <tchar.h>          // for Windows generic text
#include <shlwapi.h>        // Shell Light-Weight API
#include <strsafe.h>        // for StringC... functions

#include <string>           // for std::string, std::wstring, ...
#include <vector>           // for std::vector
#include <unordered_set>    // for std::unordered_set
#include <unordered_map>    // for std::unordered_map
#include <memory>           // for std::weak_ptr

#include "indicml.h"        // for system indicator
#include "immdev.h"         // for IME/IMM development
#include "input.h"          // for INPUT_MODE and InputContext

#include "../dict.hpp"      // for dictionary
#include "../str.hpp"       // for str_*

//////////////////////////////////////////////////////////////////////////////
// _countof macro --- get the number of elements in an array

#ifndef _countof
    #define _countof(array)   (sizeof(array) / sizeof(array[0]))
#endif

//////////////////////////////////////////////////////////////////////////////
// For debugging.
// デバッグ用。

#ifndef NDEBUG
    extern "C" {
        extern BOOL g_bTrace;
        void DebugPrintA(const char *lpszFormat, ...);
        void DebugPrintW(const WCHAR *lpszFormat, ...);
        void DebugAssert(const char *file, int line, const char *exp);
    } // extern "C"
    #define DPRINTA(fmt, ...) DebugPrintA("%s (%d): " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
    #define DPRINTW(fmt, ...) DebugPrintW(L"%hs (%d): " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
    #ifdef UNICODE
        #define DPRINT DebugPrintW
        #define DebugPrint DebugPrintW
    #else
        #define DPRINT DebugPrintA
        #define DebugPrint DebugPrintA
    #endif
    #define ASSERT(exp) ((exp) ? (void)0 : DebugAssert(__FILE__, __LINE__, #exp))
    #define TRACE_ON()    do { g_bTrace = TRUE; } while (0)
    #define TRACE_OFF()   do { g_bTrace = FALSE; } while (0)

    /* ArrayAt for std::vector */
    template <typename T_ITEM>
    T_ITEM& Array_At(std::vector<T_ITEM>& array, size_t index, const char *file, int line) {
        if (index >= array.size())
            DebugAssert(file, line, "index >= array.size()");
        return array[index];
    }
    template <typename T_ITEM>
    const T_ITEM& Array_At(const std::vector<T_ITEM>& array, size_t index, const char *file, int line) {
        if (index >= array.size())
            DebugAssert(file, line, "index >= array.size()");
        return array[index];
    }

    /* ArrayAt for raw array */
    template <typename T_ITEM, size_t t_size>
    T_ITEM& Array_At(T_ITEM (&array)[t_size], size_t index, const char *file, int line) {
        if (index >= t_size)
            DebugAssert(file, line, "index >= t_size");
        return array[index];
    }
    template <typename T_ITEM, size_t t_size>
    const T_ITEM& Array_At(const T_ITEM (&array)[t_size], size_t index, const char *file, int line) {
        if (index >= t_size)
            DebugAssert(file, line, "index >= t_size");
        return array[index];
    }

    #define ARRAY_AT(array, index) \
        Array_At((array), (index), __FILE__, __LINE__)
    #define ARRAY_AT_AT(array, index0, index1) \
        ARRAY_AT(ARRAY_AT((array), (index0)), (index1))
#else
    #define DPRINT(fmt, ...)
    #define DPRINTA(fmt, ...)
    #define DPRINTW(fmt, ...)
    #define ASSERT(exp)
    #define TRACE_ON()
    #define TRACE_OFF()
    #define ARRAY_AT(array, index)         ((array)[index])
    #define ARRAY_AT_AT(array, index0, index1) ((array)[index0][index1])
#endif

#define FootmarkDebugPrint DPRINTA
#include "footmark.hpp"   // for footmark++

//////////////////////////////////////////////////////////////////////////////

// For limit of MZ-IME.
// MZ-IMEの制限。
#define MAXCOMPWND  10  // maximum number of composition windows
#define MAXGLCHAR   32  // maximum number of guideline characters

// Special messages.
// 特別なメッセージ。
#define WM_UI_UPDATE      (WM_USER + 500)
#define WM_UI_HIDE        (WM_USER + 501)
#define WM_UI_STATEMOVE   (WM_USER + 601)
#define WM_UI_DEFCOMPMOVE (WM_USER + 602)
#define WM_UI_CANDMOVE    (WM_USER + 603)
#define WM_UI_GUIDEMOVE   (WM_USER + 604)

// Escape Functions
#define IME_ESC_PRI_GETDWORDTEST (IME_ESC_PRIVATE_FIRST + 0)

// Special style.
// 特別なウィンドウスタイル。
#define WS_COMPDEFAULT    (WS_DISABLED | WS_POPUP)
#define WS_COMPNODEFAULT  (WS_DISABLED | WS_POPUP)

// The IDs of guideline entry.
// ガイドラインエントリーのID。
#define MYGL_NODICTIONARY   0
#define MYGL_TYPINGERROR    1
#define MYGL_TESTGUIDELINE  2

// window extra of child UI windows
#define FIGWL_MOUSE 0
#define FIGWLP_SERVERWND        (FIGWL_MOUSE + sizeof(LONG))
#define FIGWLP_FONT             (FIGWLP_SERVERWND + sizeof(LONG_PTR))
#define FIGWL_COMPSTARTSTR      (FIGWLP_FONT + sizeof(LONG_PTR))
#define FIGWL_COMPSTARTNUM      (FIGWL_COMPSTARTSTR + sizeof(LONG))
#define FIGWLP_STATUSBMP        (FIGWL_COMPSTARTNUM + sizeof(LONG))
#define FIGWLP_CLOSEBMP         (FIGWLP_STATUSBMP + sizeof(LONG_PTR))
#define FIGWL_PUSHSTATUS        (FIGWLP_CLOSEBMP + sizeof(LONG_PTR))
#define FIGWLP_CHILDWND         (FIGWL_PUSHSTATUS + sizeof(LONG))
#define UIEXTRASIZE             (FIGWLP_CHILDWND + sizeof(LONG_PTR))

// The flags of FIGWL_MOUSE.
// FIGWL_MOUSEのフラグ。
#define FIM_CAPUTURED 0x01
#define FIM_MOVED 0x02

// The flags of the close button.
// 閉じるボタンのフラグ。
#define PUSHED_STATUS_CLOSE 0x08

// Statue Close Button.
#define STCLBT_X  (BTX * 2 + 3)
#define STCLBT_Y  1
#define STCLBT_DX 12
#define STCLBT_DY 12

// The style flags of ImeRegisterWord
#define MZIME_REGWORD_STYLE (IME_REGWORD_STYLE_USER_FIRST | 0x40000000)

static inline HinshiBunrui StyleToHinshi(DWORD dwStyle) {
    ASSERT((dwStyle & MZIME_REGWORD_STYLE) == MZIME_REGWORD_STYLE);
    HinshiBunrui ret = (HinshiBunrui)((dwStyle & ~MZIME_REGWORD_STYLE) + HB_MEISHI);
    ASSERT(HB_MEISHI <= ret && ret <= HB_MAX);
    return ret;
}

static inline DWORD HinshiToStyle(HinshiBunrui hinshi) {
    ASSERT(HB_MEISHI <= hinshi && hinshi <= HB_MAX);
    return ((DWORD)hinshi - HB_MEISHI) | MZIME_REGWORD_STYLE;
}

//////////////////////////////////////////////////////////////////////////////
// additional GCS bits

#define GCS_COMPALL \
        (GCS_COMPSTR | GCS_COMPATTR | GCS_COMPREADSTR | GCS_COMPREADATTR | \
         GCS_COMPCLAUSE | GCS_COMPREADCLAUSE)

#define GCS_RESULTALL \
        (GCS_RESULTSTR | GCS_RESULTREADSTR | GCS_RESULTCLAUSE | GCS_RESULTREADCLAUSE)

//////////////////////////////////////////////////////////////////////////////
// Structures

// UIEXTRA
struct UIEXTRA {
    HIMC hIMC;
    HWND hwndStatus;
    HWND hwndCand;
    POINT ptCand;
    DWORD dwCompStyle;
    HFONT hFont;
    BOOL bVertical;
    HWND hwndDefComp;
    HWND hwndComp[MAXCOMPWND];
    RECT rcComp[MAXCOMPWND];
    HWND hwndGuide;
};

// MZGUIDELINE
struct MZGUIDELINE {
    DWORD dwLevel;
    DWORD dwIndex;
    DWORD dwStrID;
    DWORD dwPrivateID;
};

//////////////////////////////////////////////////////////////////////////////

// ウィンドウクラス名。
extern const WCHAR szUIServerClassName[];   // UIサーバー。
extern const WCHAR szCompStrClassName[];    // 未確定文字列。
extern const WCHAR szCandClassName[];       // 候補ウィンドウ。
extern const WCHAR szStatusClassName[];     // 状態ウィンドウ。
extern const WCHAR szGuideClassName[];      // ガイドラインウィンドウ。

// ガイドラインテーブル。
extern const MZGUIDELINE glTable[];

//////////////////////////////////////////////////////////////////////////////

extern "C" {

// convert.c
extern std::unordered_map<WCHAR,Dan>  g_hiragana_to_dan;  // 母音写像。
extern std::unordered_map<WCHAR,Gyou> g_hiragana_to_gyou; // 子音写像。
void MakeLiteralMaps(); // 子音の写像と母音の写像を作成する。
LPCWSTR BunruiToString(HinshiBunrui bunrui);
LPCTSTR HinshiToString(HinshiBunrui hinshi);
HinshiBunrui StringToHinshi(LPCTSTR str);

// notify.c
BOOL PASCAL NotifyUCSetOpen(HIMC hIMC);
BOOL PASCAL NotifyUCConversionMode(HIMC hIMC);
BOOL PASCAL NotifyUCSetCompositionWindow(HIMC hIMC);

// ui.c
LRESULT CALLBACK MZIMEWndProc(HWND, UINT, WPARAM, LPARAM);
LONG NotifyCommand(HIMC hIMC, HWND hWnd, WPARAM wParam, LPARAM lParam);
LONG ControlCommand(HIMC hIMC, HWND hWnd, WPARAM wParam, LPARAM lParam);
void DrawUIBorder(LPRECT lprc);
void DragUI(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int GetCompFontHeight(UIEXTRA *lpUIExtra);
BOOL IsImeMessage(UINT message);
BOOL IsImeMessage2(UINT message);

// uistate.c
HWND StatusWnd_Create(HWND hWnd, UIEXTRA *lpUIExtra);
LRESULT CALLBACK StatusWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void StatusWnd_Update(UIEXTRA *lpUIExtra);

// uicand.c
LRESULT CALLBACK CandWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void CandWnd_Paint(HWND hCandWnd);
void CandWnd_Create(HWND hUIWnd, UIEXTRA *lpUIExtra, InputContext *lpIMC);
void CandWnd_SetFont(UIEXTRA *lpUIExtra);
void CandWnd_Resize(UIEXTRA *lpUIExtra, InputContext *lpIMC);
void CandWnd_Hide(UIEXTRA *lpUIExtra);
void CandWnd_Move(UIEXTRA *lpUIExtra, InputContext *lpIMC);

// uicomp.c
LRESULT CALLBACK CompWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void CompWnd_Paint(HWND hCompWnd);
void CompWnd_Create(HWND hUIWnd, UIEXTRA *lpUIExtra, InputContext *lpIMC);
void CompWnd_Move(UIEXTRA *lpUIExtra, InputContext *lpIMC);
void CompWnd_Hide(UIEXTRA *lpUIExtra);
void CompWnd_SetFont(UIEXTRA *lpUIExtra);
HWND GetCandPosHintFromComp(UIEXTRA *lpUIExtra, InputContext *lpIMC,
                            DWORD iClause, LPPOINT ppt, LPSIZE psizText);

// uiguide.c
LRESULT CALLBACK GuideWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void GuideWnd_Paint(HWND hGuideWnd, HDC hDC, LPPOINT lppt, DWORD dwPushedGuide);
void GuideWnd_Button(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void GuideWnd_Update(UIEXTRA *lpUIExtra);
LRESULT CALLBACK LineWndProc(HWND, UINT, WPARAM, LPARAM);

// config.c
HKEY Config_OpenAppKey(VOID);
HKEY Config_CreateAppKey(VOID);
DWORD Config_GetDWORD(LPCTSTR name, DWORD dwDefault);
BOOL Config_SetDWORD(LPCTSTR name, DWORD dwValue);
BOOL Config_GetData(LPCTSTR name, LPVOID pvData, DWORD cbData);
BOOL Config_SetData(LPCTSTR name, DWORD dwType, LPCVOID pvData, DWORD cbData);
BOOL Config_GetSz(LPCTSTR name, std::wstring& str);
BOOL Config_SetSz(LPCTSTR name, LPCTSTR psz);
INT_PTR CALLBACK WordListDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RegWordDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ChooseDictDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GeneralDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DebugOptionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// immsec.cpp
SECURITY_ATTRIBUTES *CreateSecurityAttributes(void);
void FreeSecurityAttributes(SECURITY_ATTRIBUTES *psa);
BOOL IsNT(void);

// mzimeja.cpp
void      RepositionWindow(HWND hWnd);
HGLOBAL   GetUIExtraFromServerWnd(HWND hwndServer);
void      SetUIExtraToServerWnd(HWND hwndServer, HGLOBAL hUIExtra);
UIEXTRA * LockUIExtra(HWND hwndServer);
void      UnlockUIExtra(HWND hwndServer);
void      FreeUIExtra(HWND hwndServer);

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
// keychar.cpp

// conversion between roman and hiragana
std::wstring hiragana_to_roman(std::wstring hiragana);
std::wstring roman_to_hiragana(std::wstring roman);
std::wstring roman_to_hiragana(std::wstring roman, size_t ichTarget);
// conversion between roman and katakana
std::wstring roman_to_katakana(std::wstring roman);
std::wstring roman_to_katakana(std::wstring roman, size_t ichTarget);
// conversion between roman and halfwidth katakana
std::wstring roman_to_halfwidth_katakana(std::wstring roman);
std::wstring roman_to_halfwidth_katakana(std::wstring roman, size_t ichTarget);

// character map for kana input
WCHAR vkey_to_hiragana(BYTE vk, BOOL bShift);
// character map for typing keys
WCHAR typing_key_to_char(BYTE vk, BOOL bShift, BOOL bCapsLock);
// dakuon (voiced consonant) processor
WCHAR dakuon_shori(WCHAR ch0, WCHAR ch1);
// locale-dependent string conversion
std::wstring lcmap(const std::wstring& str, DWORD dwFlags);
// convert hiragana to typing characters
std::wstring hiragana_to_typing(std::wstring hiragana);
// convert fullwidth ascii to halfwidth
std::wstring fullwidth_ascii_to_halfwidth(const std::wstring& str);

// is the character hiragana?
BOOL is_hiragana(WCHAR ch);
// is the character fullwidth katakana?
BOOL is_fullwidth_katakana(WCHAR ch);
// is the character halfwidth katakana?
BOOL is_halfwidth_katakana(WCHAR ch);
// is the character fullwidth ascii?
BOOL is_fullwidth_ascii(WCHAR ch);
// is the character kanji?
BOOL is_kanji(WCHAR ch);
// is the character the education kanji?
BOOL is_education_kanji(WCHAR ch);
// is the character the common use kanji?
BOOL is_common_use_kanji(WCHAR ch);
// is the character fullwidth ASCII?
BOOL is_fullwidth_ascii(WCHAR ch);
// are all the characters numeric?
BOOL are_all_chars_numeric(const std::wstring& str);
// convert to kansuuji (Kanji number)
std::wstring convert_to_kansuuji_1(wchar_t ch, size_t digit_level);
std::wstring convert_to_kansuuji_4(const std::wstring& halfwidth);
std::wstring convert_to_kansuuji(const std::wstring& str);
std::wstring convert_to_kansuuji_brief(const std::wstring& str);
std::wstring convert_to_kansuuji_formal(const std::wstring& str);
std::wstring convert_to_kansuuji_brief_formal(const std::wstring& str);
// ピリオドか？
BOOL is_period(WCHAR ch);
// カンマか？
BOOL is_comma(WCHAR ch);
// ハイフンか？
BOOL is_hyphen(WCHAR ch);
// 設定に応じて文字を変換する。
WCHAR translateChar(WCHAR ch);
WCHAR translateChar(WCHAR ch, BOOL bCommaPeriod, BOOL bNoZenkakuAscii);
// 設定に応じて文字列を変換する。
std::wstring translateString(const std::wstring& str);

//////////////////////////////////////////////////////////////////////////////
// LatticeNode and Lattice.
// 言語学でよく扱われるラティス構造を実現する。

struct LatticeNode;
typedef std::shared_ptr<LatticeNode>  LatticeNodePtr;

// ラティス（lattice）ノード。
struct LatticeNode {
    std::wstring pre;                       // 変換前。
    std::wstring post;                      // 変換後。
    std::wstring tags;                      // タグ。
    HinshiBunrui bunrui;                    // 分類。
    INT deltaCost = 0;                      // コスト差分。
    INT subtotal_cost = MAXLONG;            // 部分合計コスト。
    INT marked = 0;                         // マーキング。
    Gyou gyou;                              // 活用の行。
    KatsuyouKei katsuyou;                   // 動詞活用形。
    DWORD linked = 0;                       // リンク数。
    // 枝分かれ。
    std::vector<LatticeNodePtr> branches;
    // 逆向き枝分かれ。
    std::unordered_set<LatticeNode*> reverse_branches;

    bool IsDoushi() const;      // 動詞か？
    bool IsJoshi() const;       // 助詞か？
    bool IsJodoushi() const;    // 助動詞か？
    bool IsKeiyoushi() const;   // 形容詞か？

    // 指定したタグがあるか？
    bool HasTag(const wchar_t *tag) const {
        return tags.find(tag) != std::wstring::npos;
    }
};
typedef std::vector<LatticeNodePtr>   LatticeChunk;

// ラティス。
struct Lattice {
    std::wstring                    m_pre;    // 変換前。
    LatticeNodePtr                  m_head;   // 先頭ノード。
    LatticeNodePtr                  m_tail;   // 末端ノード。
    std::vector<LatticeChunk>       m_chunks; // インデックス位置に対するノード集合。
    // m_pre.size() + 1 == m_chunks.size().

    BOOL AddNodesForMulti(const std::wstring& pre);
    BOOL AddNodesForSingle(const std::wstring& pre);
    void AddExtraNodes();

    BOOL AddNodesFromDict(size_t index, const WCHAR *dict_data);
    BOOL AddNodesFromDict(const WCHAR *dict_data);
    void ResetLatticeInfo();
    void UpdateLinksAndBranches();
    BOOL OptimizeMarking(LatticeNode *ptr0);
    void AddComplement();
    void AddComplement(size_t index, size_t min_size, size_t max_size);
    void CutUnlinkedNodes();
    void MakeReverseBranches(LatticeNode *ptr0);
    INT CalcSubTotalCosts(LatticeNode *ptr1);
    size_t GetLastLinkedIndex() const;

    void Dump(int num = 0);
    void Fix(const std::wstring& pre);
    void AddNode(size_t index, const LatticeNode& node);

protected:
    void DoFields(size_t index, const WStrings& fields, INT deltaCost = 0);
    void DoMeishi(size_t index, const WStrings& fields, INT deltaCost = 0);
    void DoIkeiyoushi(size_t index, const WStrings& fields, INT deltaCost = 0);
    void DoNakeiyoushi(size_t index, const WStrings& fields, INT deltaCost = 0);
    void DoGodanDoushi(size_t index, const WStrings& fields, INT deltaCost = 0);
    void DoIchidanDoushi(size_t index, const WStrings& fields, INT deltaCost = 0);
    void DoKahenDoushi(size_t index, const WStrings& fields, INT deltaCost = 0);
    void DoSahenDoushi(size_t index, const WStrings& fields, INT deltaCost = 0);
    void DoFukushi(size_t index, const WStrings& fields, INT deltaCost = 0);
};

//////////////////////////////////////////////////////////////////////////////

// 変換候補。
struct MzConvCandidate {
    std::wstring pre;              // ひらがな。
    std::wstring post;             // 変換後。
    INT cost = 0;                  // コスト。
    INT word_cost = 0;             // 単語コスト。
    std::unordered_set<HinshiBunrui>  bunruis;    // 品詞分類集合。
    std::wstring tags;             // タグ。
    HinshiBunrui bunrui;           // 品詞分類。
    KatsuyouKei katsuyou;          // 活用形。

    void clear() {
        pre.clear();
        post.clear();
        cost = 0;
        bunruis.clear();
        tags.clear();
    }
};

// 変換文節。
struct MzConvClause {
    std::vector<MzConvCandidate> candidates;    // 候補群。
    void sort();                                // ソートする。
    void add(const LatticeNode *node);          // ノードを追加する。

    void clear() {
        candidates.clear();
    }
};

// 変換結果。
struct MzConvResult {
    std::vector<MzConvClause> clauses;      // 文節群。
    void sort();                            // ソートする。
    void clear() { clauses.clear(); }       // クリアする。
    std::wstring get_str(bool detailed = false) const;
};

//////////////////////////////////////////////////////////////////////////////
// dictionary - 辞書

class Dict {
public:
    Dict();
    ~Dict();

    // 辞書を読み込む。
    BOOL Load(const wchar_t *file_name, const wchar_t *object_name);
    // 辞書をアンロードする。
    void Unload();

    BOOL IsLoaded() const;  // 読み込み済みか？
    DWORD GetSize() const;  // サイズを取得する。

    wchar_t *Lock();            // ロックして読み込みを開始する。
    void Unlock(wchar_t *data); // ロックを解除して読み込みを終了する。

protected:
    std::wstring m_strFileName;     // ファイル名。
    std::wstring m_strObjectName;   // 複数の辞書を使うので、オブジェクト名で区別する。
    HANDLE m_hMutex;                // 排他制御用。
    HANDLE m_hFileMapping;          // ファイルマッピング。
};

//////////////////////////////////////////////////////////////////////////////
// MZ-IME

extern Dict g_basic_dict;
extern Dict g_name_dict;

class MzIme {
public:
    HINSTANCE m_hInst;
    HKL m_hMyKL;
    BOOL m_bWinLogOn;
    LPTRANSMSGLIST m_lpCurTransKey;
    UINT m_uNumTransKey;
    BOOL m_fOverflowKey;

public:
    MzIme();

    // 初期化。
    BOOL Init(HINSTANCE hInstance);
    // ウィンドウクラスの登録。
    BOOL RegisterClasses(HINSTANCE hInstance);
    void UnregisterClasses();
    // 逆初期化。
    VOID Uninit(VOID);

    // リソースからビットマップを読み込む。
    HBITMAP LoadBMP(LPCTSTR pszName);
    HBITMAP LoadBMP(UINT nID) { return LoadBMP(MAKEINTRESOURCE(nID)); }
    // リソースから文字列を読み込む。
    WCHAR *LoadSTR(INT nID);

    void UpdateIndicIcon(HIMC hIMC);    // インジケーターアイコンを更新する。

    HKL GetHKL(VOID);                   // キーボードレイアウトのハンドルを取得する。

    InputContext *LockIMC(HIMC hIMC);   // 入力コンテキストをロックする。
    VOID UnlockIMC(HIMC hIMC);          // 入力コンテキストのロックを解除する。

    // メッセージの生成。
    BOOL GenerateMessage(LPTRANSMSG lpGeneMsg);
    BOOL GenerateMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
    BOOL GenerateMessageToTransKey(LPTRANSMSG lpGeneMsg);

    BOOL DoCommand(HIMC hIMC, DWORD dwCommand);     // コマンドを実行する。

    // 基本辞書。
    BOOL LoadBasicDict();                   // 基本辞書を読み込む。
    BOOL IsBasicDictLoaded() const;         // 基本辞書が読み込まれたか？
    WCHAR *LockBasicDict();                 // 基本辞書をロックする。
    void UnlockBasicDict(WCHAR *data);      // 基本辞書のロックを解除する。

    // make result
    void MakeResultOnFailure(MzConvResult& result, const std::wstring& pre);
    void MakeResultForMulti(MzConvResult& result, Lattice& lattice);
    void MakeResultForSingle(MzConvResult& result, Lattice& lattice);
    int CalcCost(const std::wstring& tags) const;

    // 変換。
    BOOL ConvertMultiClause(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
    BOOL ConvertMultiClause(const std::wstring& str, MzConvResult& result);
    BOOL ConvertSingleClause(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
    BOOL ConvertSingleClause(const std::wstring& str, MzConvResult& result);
    BOOL StretchClauseLeft(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
    BOOL StretchClauseRight(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
    BOOL ConvertCode(const std::wstring& strTyping, MzConvResult& result);
    BOOL ConvertCode(LogCompStr& comp, LogCandInfo& cand);
    BOOL StoreResult(const MzConvResult& result, LogCompStr& comp, LogCandInfo& cand);

protected:
    // 入力コンテキスト（input context）
    HIMC m_hIMC;
    InputContext *  m_lpIMC;

    // 辞書。
    BOOL LoadDict();
    void UnloadDict();
}; // class MzIme

extern MzIme TheIME;

// postal.cpp
std::wstring normalize_postal_code(const std::wstring& str);
std::wstring convert_postal_code(const std::wstring& code);

//////////////////////////////////////////////////////////////////////////////
