// input.h --- mzimeja input context and related
//////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_H_
#define INPUT_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#include "immdev.h"

#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// ���̓��[�h�B

enum INPUT_MODE {
    IMODE_FULL_HIRAGANA,    // �S�p�Ђ炪�ȁB
    IMODE_FULL_KATAKANA,    // �S�p�J�^�J�i�B
    IMODE_FULL_ASCII,       // �S�p�p���B
    IMODE_HALF_KANA,        // ���p�J�i�B
    IMODE_HALF_ASCII,       // ���p�p���B
    IMODE_DISABLED          // IME�������B
};

BOOL        IsInputModeOpen(INPUT_MODE imode);
INPUT_MODE  InputModeFromConversionMode(BOOL bOpen, DWORD dwConversion);
INPUT_MODE  NextInputMode(INPUT_MODE imode);
INPUT_MODE  GetInputMode(HIMC hIMC);
void        SetInputMode(HIMC hIMC, INPUT_MODE imode);
BOOL        IsRomanMode(HIMC hIMC);
void        SetRomanMode(HIMC hIMC, BOOL bRoman);
UINT        CommandFromInputMode(INPUT_MODE imode);

//////////////////////////////////////////////////////////////////////////////
// ���m�蕶����B

struct LogCandInfo;

// ���m�蕶����̗]����̘_���f�[�^�B
struct LogCompStrExtra {
    // �I�𒆂̕��߂̃C���f�b�N�X�B
    // assert(iClause <= GetClauseCount());
    DWORD iClause;
    // �Ђ炪�ȕ��ߕ�����B
    std::vector<std::wstring>   hiragana_clauses;
    // ���͕��ߕ�����B
    std::vector<std::wstring>   typing_clauses;
    // ���m�蕶����̕��ߕ�����B
    std::vector<std::wstring>   comp_str_clauses;

    LogCompStrExtra() {
        clear();
    }
    void clear();
    DWORD GetTotalSize() const;
}; // struct LogCompStrExtra

// ���m�蕶����̗]����̕����f�[�^�B
struct COMPSTREXTRA {
    DWORD dwSignature;              // �m�F�̂��߁B
    DWORD iClause;                  // ���݂̕��߂̃C���f�b�N�X�B
    DWORD dwHiraganaClauseCount;    // �Ђ炪�ȕ��߂̌��B
    DWORD dwHiraganaClauseOffset;   // �Ђ炪�ȕ��߂̃I�t�Z�b�g�B
    DWORD dwTypingClauseCount;      // ���͕��߂̌��B
    DWORD dwTypingClauseOffset;     // ���͕��߂̃I�t�Z�b�g�B

    BYTE *GetBytes() { return (LPBYTE) this; }  // �o�C�g��̎擾�B
    WCHAR *GetHiraganaClauses(DWORD& dwCount);  // �Ђ炪�ȕ��߂̎擾�B
    WCHAR *GetTypingClauses(DWORD& dwCount);    // ���͕��߂̎擾�B

    void GetLog(LogCompStrExtra& log);          // �����f�[�^����_���f�[�^�ցB
    DWORD Store(const LogCompStrExtra *log);    // �_���f�[�^���畨���f�[�^�ցB
}; // struct COMPSTREXTRA

// ���m�蕶����̘_���f�[�^�B
struct LogCompStr {
    DWORD dwCursorPos;                      // ���݈ʒu�̕����̃C���f�b�N�X�B
    DWORD dwDeltaStart;                     // �ŏ��ɕύX���ꂽ�����̃C���f�b�N�X�B
    std::vector<BYTE>   comp_read_attr;     // we don't use this member: comp_read_attr
    std::vector<DWORD>  comp_read_clause;   // we don't use this member: comp_read_clause
    std::wstring comp_read_str;             // �ǂݕ�����B
    std::vector<BYTE>   comp_attr;          // ���������B
    std::vector<DWORD>  comp_clause;        // ���m�蕶����̕��߃C���f�b�N�X���疢�m�蕶����̕����C���f�b�N�X�ւ̎ʑ��B
    std::wstring comp_str;                  // ���m�蕶����B
    std::vector<DWORD>  result_read_clause; // ���ʓǂݕ��߃C���f�b�N�X���猋�ʕ����C���f�b�N�X�ւ̎ʑ��B
    std::wstring result_read_str;           // ���ʓǂݕ�����B
    std::vector<DWORD>  result_clause;      // ���ʕ��߃C���f�b�N�X���猋�ʕ����C���f�b�N�X�ւ̎ʑ��B
    std::wstring result_str;                // ���ʕ�����B
    LogCompStrExtra extra;                  // �]����B

    LogCompStr() {
        clear();
    }

    // �N���A�B
    void clear();
    void clear_read();
    void clear_comp();
    void clear_result();
    void clear_extra() { extra.clear(); }

    void fix(); // �␳�B
    DWORD GetTotalSize() const; // �����f�[�^�̍��v�T�C�Y�B

    BOOL IsBeingConverted();
    DWORD GetClauseCount() const;
    BOOL CompCharInClause(DWORD iCompChar, DWORD iClause,
                          BOOL bExcludeEnd = FALSE) const;
    BYTE GetClauseAttr(DWORD dwClauseIndex) const;
    void SetClauseAttr(DWORD dwClauseIndex, BYTE attr);
    BOOL IsClauseConverted() const;
    BOOL IsClauseConverted(DWORD dwClauseIndex) const;
    BOOL HasClauseSelected() const;
    std::wstring GetClauseCompString(DWORD dwClauseIndex) const;
    WCHAR PrevCharInClause() const;

    BYTE GetCompCharAttr(DWORD ich) const;
    DWORD GetCompCharCount() const;

    DWORD ClauseToCompChar(DWORD dwClauseIndex) const;
    DWORD CompCharToClause(DWORD iCompChar) const;
    BOOL HasCompStr() const;

    // ���m�蕶����̕�����ϊ��B
    void MakeHiragana();
    void MakeKatakana();
    void MakeHankaku();
    void MakeZenEisuu();
    void MakeHanEisuu();

    void AddCharToEnd(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);
    void InsertChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);
    void AddDakuonChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);

    void AddChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);
    void DeleteChar(BOOL bBackSpace /* = FALSE*/, DWORD dwConv);
    void RevertText();
    void MakeResult();

    BOOL MoveLeft();
    BOOL MoveRight();
    BOOL MoveHome();
    BOOL MoveEnd();

    DWORD GetClauseCompStrLen(DWORD dwClauseIndex) const;
    void SetClauseCompString(DWORD iClause, std::wstring& str);
    void SetClauseCompHiragana(DWORD iClause, std::wstring& str);
    void SetClauseCompHiragana(DWORD iClause, std::wstring& str, BOOL bRoman);
    void UpdateExtraClause(DWORD iClause, DWORD dwConversion);
    void UpdateFromExtra(BOOL bRoman);

    // �f�o�b�O�p�B
    void AssertValid();
    void Dump();

protected:
    void MergeAt(std::vector<std::wstring>& strs, DWORD istr);
    void UpdateCompStr();
}; // struct LogCompStr

inline void SetClause(DWORD *lpdw, DWORD num) {
    *lpdw = 0;
    *(lpdw + 1) = num;
}

// ���m�蕶����̕����f�[�^�B
struct CompStr : public COMPOSITIONSTRING {
    static HIMCC ReCreate(HIMCC hCompStr, const LogCompStr *log = NULL);

    void GetLog(LogCompStr& log);
    DWORD Store(const LogCompStr *log);

    BYTE *GetBytes() {
        return (LPBYTE) this;
    }

    BYTE *GetCompReadAttr() {
        return GetBytes() + dwCompReadAttrOffset;
    }
    DWORD *GetCompReadClause() {
        return (DWORD *)(GetBytes() + dwCompReadClauseOffset);
    }
    WCHAR *GetCompReadStr() {
        return (LPTSTR)(GetBytes() + dwCompReadStrOffset);
    }
    BYTE *GetCompAttr() {
        return GetBytes() + dwCompAttrOffset;
    }
    DWORD *GetCompClause() {
        return (DWORD *)(GetBytes() + dwCompClauseOffset);
    }
    WCHAR *GetCompStr() {
        return (LPTSTR)(GetBytes() + dwCompStrOffset);
    }
    DWORD *GetResultReadClause() {
        return (DWORD *)(GetBytes() + dwResultReadClauseOffset);
    }
    WCHAR *GetResultReadStr() {
        return (LPTSTR)(GetBytes() + dwResultReadStrOffset);
    }
    DWORD *GetResultClause() {
        return (DWORD *)(GetBytes() + dwResultClauseOffset);
    }
    WCHAR *GetResultStr() {
        return (LPTSTR)(GetBytes() + dwResultStrOffset);
    }

    // extension
    COMPSTREXTRA *GetExtra();

    void Dump();

private:
    CompStr();
    CompStr(const CompStr&);
    CompStr& operator=(const CompStr&);
}; // struct CompStr

//////////////////////////////////////////////////////////////////////////////
// �����B

// private data of CANDIDATEINFO
struct CANDINFOEXTRA {
    DWORD dwSignature; // must be 0xDEADFACE
    DWORD iClause; // index of selected clause
};

// ��⃊�X�g�̘_���f�[�^�B
struct LogCandList {
    DWORD dwStyle;
    DWORD dwSelection;
    DWORD dwPageStart;
    DWORD dwPageSize;
    std::vector<std::wstring> cand_strs;

    LogCandList() {
        clear();
    }
    void clear();
    DWORD GetTotalSize() const;

    void MoveNext();
    void MovePrev();
    void PageUp();
    void PageDown();
    void MoveHome();
    void MoveEnd();

    DWORD GetCandCount() const;
    std::wstring GetString() const;
    std::wstring GetString(DWORD iCand) const;
};

// �����̘_���f�[�^�B
struct LogCandInfo {
    std::vector<LogCandList>  cand_lists;
    DWORD iClause;

    LogCandInfo() {
        clear();
    }
    void clear();
    DWORD GetTotalSize() const;
    DWORD GetClauseCount() const;
    BOOL HasCandInfo() const;

    void MoveNext();
    void MovePrev();
    void MoveHome();
    void MoveEnd();
    void PageUp();
    void PageDown();
    BOOL SelectCand(UINT uCandIndex);

    std::wstring GetString() const;
    std::wstring GetString(DWORD iCand) const;

    void Dump();
}; // struct LogCandInfo

// ��⃊�X�g�̕����f�[�^�B
struct CandList : public CANDIDATELIST {
    BYTE *GetBytes()                { return (BYTE *)this; }
    WCHAR *GetCandString(DWORD i)   { return LPTSTR(GetBytes() + dwOffset[i]); }
    WCHAR *GetCurString()           { return GetCandString(dwSelection); }
    DWORD  GetPageEnd() const;
    void GetLog(LogCandList& log);
    DWORD Store(const LogCandList *log);

private:
    // never be implemented
    CandList();
    CandList(const CandList&);
    CandList& operator=(const CandList&);
}; // struct CandList

// �����̕����f�[�^�B
struct CandInfo : public CANDIDATEINFO {
    static HIMCC ReCreate(HIMCC hCandInfo, const LogCandInfo *log = NULL);
    void GetLog(LogCandInfo& log);
    DWORD Store(const LogCandInfo *log);

    BYTE *GetBytes() { return (BYTE *)this; }
    CandList *GetList(DWORD i);
    CANDINFOEXTRA *GetExtra();

    void Dump();

private:
    // never be implemented
    CandInfo();
    CandInfo(const CandInfo&);
    CandInfo& operator=(const CandInfo&);
}; // struct CandInfo

//////////////////////////////////////////////////////////////////////////////
// ���̓R���e�L�X�g�B

struct InputContext : public INPUTCONTEXT {
    void Initialize();

    // ��Ԃ��擾�E�ݒ肷��B
    BOOL& IsOpen()                  { return fOpen; }
    const BOOL& IsOpen() const      { return fOpen; }
    DWORD& Conversion()             { return fdwConversion; }
    const DWORD& Conversion() const { return fdwConversion; }
    DWORD& Sentence()               { return fdwSentence; }
    const DWORD& Sentence() const   { return fdwSentence; }
    BOOL HasStatusWndPos() const    { return (fdwInit & INIT_STATUSWNDPOS); }
    BOOL HasConversion() const      { return (fdwInit & INIT_CONVERSION); }
    BOOL HasSentence() const        { return (fdwInit & INIT_SENTENCE); }
    BOOL HasLogFont() const         { return (fdwInit & INIT_LOGFONT); }
    BOOL HasCompForm() const        { return (fdwInit & INIT_COMPFORM); }
    BOOL HasSoftKbdPos() const      { return (fdwInit & INIT_SOFTKBDPOS); }
    INPUT_MODE GetInputMode() const;
    BOOL IsRomanMode() const;

    // �����B
    BOOL HasCandInfo();
    CandInfo *LockCandInfo();
    void UnlockCandInfo();

    // ���m�蕶����B
    BOOL HasCompStr();
    CompStr *LockCompStr();
    void UnlockCompStr();

    // ���b�Z�[�W�o�b�t�@�B
    LPTRANSMSG LockMsgBuf();
    void UnlockMsgBuf();
    DWORD& NumMsgBuf();
    const DWORD& NumMsgBuf() const;

    // �K�C�h���C���B
    void MakeGuideLine(DWORD dwID);
    LPGUIDELINE LockGuideLine();
    void UnlockGuideLine();

    // actions
    BOOL SelectCand(UINT uCandIndex);
    void AddChar(WCHAR chTyped, WCHAR chTranslated);
    void Escape();
    void CancelText();
    void RevertText();
    void DeleteChar(BOOL bBackSpace = FALSE);
    BOOL Convert(BOOL bShift);
    BOOL OpenCandidate();
    BOOL CloseCandidate(BOOL bClearCandInfo = TRUE);

    void MoveLeft(BOOL bShift);
    void MoveRight(BOOL bShift);
    void MoveUp();
    void MoveDown();
    void MoveHome();
    void MoveEnd();
    void PageUp();
    void PageDown();

    void MakeResult();
    void MakeHiragana();
    void MakeKatakana();
    void MakeHankaku();
    void MakeZenEisuu();
    void MakeHanEisuu();
    BOOL ConvertCode();

    // �_���f�[�^���擾�B
    void GetLogObjects(LogCompStr& comp, LogCandInfo& cand);

    // �f�o�b�O�p�B
    void DumpCandInfo();
    void DumpCompStr();
    void Dump();

private:
    // never be implemented
    InputContext();
    InputContext(const InputContext&);
    InputContext& operator=(const InputContext&);
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef INPUT_H_
