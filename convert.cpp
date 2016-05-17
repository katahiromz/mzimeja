// convert.cpp --- mzimeja kana kanji conversion
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

void MZIMEJA::FreeClauseConversion(LogCompStr& comp, LogCandInfo& cand) {
  MzConversionResult result;
  std::wstring strHiragana = comp.extra.hiragana_clauses[comp.extra.iClause];
  FreeClauseConversion(strHiragana, result);

  // TODO: support multiple clauses
  // setting comp
  comp.comp_str.clear();
  comp.extra.clear();
  comp.comp_clause.resize(result.clauses.size() + 1);
  for (size_t k = 0; k < result.clauses.size(); ++k) {
    MzConversionClause& clause = result.clauses[k];
    for (size_t i = 0; i < clause.candidates.size(); ++i) {
      MzConversionCandidate& cand = clause.candidates[i];
      comp.comp_clause[k] = (DWORD)comp.comp_str.size();
      comp.extra.hiragana_clauses.push_back(cand.hiragana);
      std::wstring typing = hiragana_to_typing(cand.hiragana);
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
} // MZIMEJA::FreeClauseConversion

void MZIMEJA::FreeClauseConversion(const std::wstring& strHiragana,
                                   MzConversionResult& result)
{
  // TODO:
  MzConversionClause clause;
  MzConversionCandidate cand;
  cand.hiragana = L"これは";
  cand.converted = L"これは";
  clause.candidates.push_back(cand);
  cand.hiragana = L"てすと";
  cand.converted = L"テスト";
  clause.candidates.push_back(cand);
  cand.hiragana = L"です。";
  cand.converted = L"です。";
  clause.candidates.push_back(cand);
  result.clauses.push_back(clause);
} // MZIMEJA::FreeClauseConversion

void MZIMEJA::FixedClauseConversion(const std::wstring& strHiragana,
                                    MzConversionClause& result)
{
  // TODO:
} // MZIMEJA::FixedClauseConversion
