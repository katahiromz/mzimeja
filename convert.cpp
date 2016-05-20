// convert.cpp --- mzimeja kana kanji conversion
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////

BOOL MZIMEJA::LoadDictionary() {
  // TODO:
  return FALSE;
} // MZIMEJA::LoadDictionary

BOOL MZIMEJA::IsDictionaryLoaded() const {
  // TODO:
  return FALSE;
}

void MZIMEJA::PluralClauseConversion(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  MzConversionResult result;
  std::wstring strHiragana = comp.extra.hiragana_clauses[comp.extra.iClause];
  PluralClauseConversion(strHiragana, result);

  // TODO:
  comp.comp_str.clear();
  comp.extra.clear();
  comp.comp_clause.resize(result.clauses.size() + 1);
  for (size_t k = 0; k < result.clauses.size(); ++k) {
    MzConversionClause& clause = result.clauses[k];
    for (size_t i = 0; i < clause.candidates.size(); ++i) {
      MzConversionCandidate& cand = clause.candidates[i];
      comp.comp_clause[k] = (DWORD)comp.comp_str.size();
      comp.extra.hiragana_clauses.push_back(cand.hiragana);
      std::wstring typing;
      if (bRoman) {
        typing = hiragana_to_typing(cand.hiragana);
      } else {
        typing = hiragana_to_roman(cand.hiragana);
      }
      comp.extra.typing_clauses.push_back(typing);
      comp.comp_str += cand.converted;
      break;
    }
  }
  comp.comp_clause[result.clauses.size()] = (DWORD)comp.comp_str.size();
  comp.comp_attr.assign(comp.comp_str.size(), ATTR_CONVERTED);
  comp.extra.iClause = 0;
  comp.SetClauseAttr(comp.extra.iClause, ATTR_TARGET_CONVERTED);
  comp.dwCursorPos = (DWORD)comp.comp_str.size();
  comp.dwDeltaStart = 0;

  // setting cand
  cand.clear();
  for (size_t k = 0; k < result.clauses.size(); ++k) {
    MzConversionClause& clause = result.clauses[k];
    LogCandList cand_list;
    for (size_t i = 0; i < clause.candidates.size(); ++i) {
      MzConversionCandidate& cand = clause.candidates[i];
      cand_list.cand_strs.push_back(cand.converted);
    }
    cand.cand_lists.push_back(cand_list);
  }
} // MZIMEJA::PluralClauseConversion

void MZIMEJA::PluralClauseConversion(const std::wstring& strHiragana,
                                     MzConversionResult& result)
{
  FOOTMARK();
  // TODO:
  WCHAR sz[64];
  result.clauses.clear();
  for (DWORD iClause = 0; iClause < 5; ++iClause) {
    MzConversionClause clause;
    for (DWORD iCand = 0; iCand < 18; ++iCand) {
      MzConversionCandidate cand;
      ::wsprintfW(sz, L"こうほ%u-%u", iClause, iCand);
      cand.hiragana = sz;
      ::wsprintfW(sz, L"候補%u-%u", iClause, iCand);
      cand.converted = sz;
      clause.candidates.push_back(cand);
    }
    result.clauses.push_back(clause);
  }

  result.clauses[0].candidates[0].hiragana = L"ひらりー";
  result.clauses[0].candidates[0].converted = L"ヒラリー";
  result.clauses[1].candidates[0].hiragana = L"とらんぷ";
  result.clauses[1].candidates[0].converted = L"トランプ";
  result.clauses[2].candidates[0].hiragana = L"さんだーす";
  result.clauses[2].candidates[0].converted = L"サンダース";
  result.clauses[3].candidates[0].hiragana = L"かたやま";
  result.clauses[3].candidates[0].converted = L"片山";
  result.clauses[4].candidates[0].hiragana = L"うちゅうじん";
  result.clauses[4].candidates[0].converted = L"宇宙人";
} // MZIMEJA::PluralClauseConversion

void MZIMEJA::SingleClauseConversion(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  DWORD iClause = comp.extra.iClause;

  MzConversionClause result;
  std::wstring strHiragana = comp.extra.hiragana_clauses[iClause];
  SingleClauseConversion(strHiragana, result);

  comp.SetClauseCompString(iClause, result.candidates[0].converted);
  comp.SetClauseCompHiragana(iClause, result.candidates[0].hiragana, bRoman);

  // setting cand
  LogCandList cand_list;
  for (size_t i = 0; i < result.candidates.size(); ++i) {
    MzConversionCandidate& cand = result.candidates[i];
    cand_list.cand_strs.push_back(cand.converted);
  }
  cand.cand_lists[iClause] = cand_list;
  cand.iClause = 0;

  comp.extra.iClause = 0;
}

void MZIMEJA::SingleClauseConversion(const std::wstring& strHiragana,
                                     MzConversionClause& result)
{
  FOOTMARK();
  result.clear();

  // TODO:
  MzConversionCandidate cand;
  cand.hiragana = L"たんいつぶんせつへんかん";
  cand.converted = L"単一文節変換1";
  result.candidates.push_back(cand);
  cand.hiragana = L"たんいつぶんせつへんかん";
  cand.converted = L"単一文節変換2";
  result.candidates.push_back(cand);
  cand.hiragana = L"たんいつぶんせつへんかん";
  cand.converted = L"単一文節変換3";
  result.candidates.push_back(cand);
} // MZIMEJA::SingleClauseConversion

BOOL MZIMEJA::StretchClauseLeft(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  // TODO:
  return FALSE;
} // MZIMEJA::StretchClauseLeft

BOOL MZIMEJA::StretchClauseRight(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  // TODO:
  return FALSE;
} // MZIMEJA::StretchClauseRight

//////////////////////////////////////////////////////////////////////////////
