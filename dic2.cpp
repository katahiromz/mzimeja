// dic2.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

static WORD table1[] = {0x3000, 0xFF01, 0x201D, 0xFF03, 0xFF04, 0xFF05,
                        0xFF06, 0x2019, 0xFF08, 0xFF09, 0xFF0A, 0xFF0B,
                        0xFF0C, 0xFF0D, 0xFF0E, 0x00F7};
static WORD table2[] = {0xFF1A, 0xFF1B, 0xFF1C, 0xFF1D, 0xFF1E, 0xFF1F};
static WORD table3[] = {0xFF3B, 0xFFE5, 0xFF3D, 0xFF3E, 0xFF3F, 0x2018};
static WORD table4[] = {0xFF5B, 0xFF5C, 0xFF5D, 0xFF5E, 0x0000};
static WORD table5[] = {
    0x3002, 0x300C, 0x300D, 0x3001, 0x30FB, 0x3092, 0x3041, 0x3043, 0x3045,
    0x3047, 0x3049, 0x3083, 0x3085, 0x3087, 0x3063, 0x30FC, 0x3042, 0x3044,
    0x3046, 0x3048, 0x304A, 0x304B, 0x304D, 0x304F, 0x3051, 0x3053, 0x3055,
    0x3057, 0x3059, 0x305B, 0x305D, 0x305F, 0x3061, 0x3064, 0x3066, 0x3068,
    0x306A, 0x306B, 0x306C, 0x306D, 0x306E, 0x306F, 0x3072, 0x3075, 0x3078,
    0x307B, 0x307E, 0x307F, 0x3080, 0x3081, 0x3082, 0x3084, 0x3086, 0x3088,
    0x3089, 0x308A, 0x308B, 0x308C, 0x308D, 0x308F, 0x3093, 0x309B, 0x309C};
static WORD table6[] = {
    0x3002, 0x300C, 0x300D, 0x3001, 0x30FB, 0x30F2, 0x30A1, 0x30A3, 0x30A5,
    0x30A7, 0x30A9, 0x30E3, 0x30E5, 0x30E7, 0x30C3, 0x30FC, 0x30A2, 0x30A4,
    0x30A6, 0x30A8, 0x30AA, 0x30AB, 0x30AD, 0x30AF, 0x30B1, 0x30B3, 0x30B5,
    0x30B7, 0x30B9, 0x30BB, 0x30BD, 0x30BF, 0x30C1, 0x30C4, 0x30C6, 0x30C8,
    0x30CA, 0x30CB, 0x30CC, 0x30CD, 0x30CE, 0x30CF, 0x30D2, 0x30D5, 0x30D8,
    0x30DB, 0x30DE, 0x30DF, 0x30E0, 0x30E1, 0x30E2, 0x30E4, 0x30E6, 0x30E8,
    0x30E9, 0x30EA, 0x30EB, 0x30EC, 0x30ED, 0x30EF, 0x30F3, 0x309B, 0x309C};
static WORD table7[] = {0x306F, 0x3072, 0x3075, 0x3078, 0x307B, 0x30CF,
                        0x30D2, 0x30D5, 0x30D8, 0x30DB, 0x0000};
static WORD table8[] = {0x304B, 0x304D, 0x304F, 0x3051, 0x3053, 0x3055, 0x3057,
                        0x3059, 0x305B, 0x305D, 0x305F, 0x3061, 0x3064, 0x3066,
                        0x3068, 0x306F, 0x3072, 0x3075, 0x3078, 0x307B, 0x30AB,
                        0x30AD, 0x30AF, 0x30B1, 0x30B3, 0x30B5, 0x30B7, 0x30B9,
                        0x30BB, 0x30BD, 0x30BF, 0x30C1, 0x30C4, 0x30C6, 0x30C8,
                        0x30CF, 0x30D2, 0x30D5, 0x30D8, 0x30DB, 0x0000};

/**********************************************************************/
/*                                                                    */
/* HanToZen( code, flag )                                             */
/*                                                                    */
/* A function which converts half size character to full size         */
/*                                                                    */
/* code                                                               */
/* Specify half size character code                                   */
/*                                                                    */
/* flag                                                               */
/* When convert to full size, specify if kana should be converted to  */
/* hiragana or katakana                                               */
/*   0   convert to katakana                                          */
/*   1   convert to hiragana                                          */
/*                                                                    */
/* return                                                             */
/* Return full size character code                                    */
/*                                                                    */
/**********************************************************************/
WORD PASCAL HanToZen(WORD code, WORD KatakanaLetter, DWORD fdwConversion)
{
  BOOL flag;

  flag = !(fdwConversion & IME_CMODE_KATAKANA);

  if (KatakanaLetter) {
    WORD code2, code3;
    code2 = HanToZen(KatakanaLetter, 0, fdwConversion);
    code3 = code;
    if (code3 == 0xFF9E)
      code = ConvTenten(code2);
    else if (code3 == 0xFF9F)
      code = ConvMaru(code2);
    return (code);

  } else {
    if (code >= 0x30 && 0x39 >= code) return (code - 0x30 + 0xFF10);
    if (code >= 0x41 && 0x5a >= code) return (code - 0x41 + 0xFF21);
    if (code >= 0x61 && 0x7a >= code) return (code - 0x61 + 0xFF41);
    if (code >= 0x20 && 0x40 >= code) return (table1[code - 0x20]);
    if (code >= 0x3a && 0x3f >= code) return (table2[code - 0x3a]);
    if (code >= 0x5b && 0x60 >= code) return (table3[code - 0x5b]);
    if (code >= 0x7b && 0x7e >= code) return (table4[code - 0x7b]);
    if (code >= 0xFF61 && 0xFF9F >= code) {
      if (flag)
        return (table5[code - 0xFF61]);
      else
        return (table6[code - 0xFF61]);
    }
    return (code);
  }
}

/**********************************************************************/
/*                                                                    */
/* ZenToHan( code )                                                   */
/*                                                                    */
/* A function which converts full size character to half size         */
/*                                                                    */
/* code                                                               */
/* Specify full size character code                                   */
/*                                                                    */
/* return                                                             */
/* Return half size character code                                    */
/*                                                                    */
/**********************************************************************/
WORD PASCAL ZenToHan(WORD code) {
  int i;
  if (code >= 0xFF10 && code <= 0xFF19) return (code - 0xFF10 + 0x30);
  if (code >= 0xFF21 && code <= 0xFF3A) return (code - 0xFF21 + 0x41);
  if (code >= 0xFF41 && code <= 0xFF5A) return (code - 0xFF41 + 0x61);

  for (i = 0; i < 16; i++)
    if (code == table1[i]) return (0x20 + i);
  for (i = 0; i < 6; i++)
    if (code == table2[i]) return (0x3a + i);
  for (i = 0; i < 6; i++)
    if (code == table3[i]) return (0x5b + i);
  for (i = 0; i < 5; i++)
    if (code == table4[i]) return (0x7b + i);
  for (i = 0; i < 63; i++) {
    if (code == table5[i]) return (0xFF61 + i);
    if (code == table6[i]) return (0xFF61 + i);
  }
  return 0;
}

BOOL PASCAL IsTenten(WORD code) {
  register int i;

  for (i = 0; table8[i]; i++)
    if (table8[i] == code) return (TRUE);
  return (FALSE);
}

WORD PASCAL ConvTenten(WORD code) {
  if (IsTenten(code)) return (code + 1);
  return 0;
}

BOOL PASCAL IsMaru(WORD code) {
  register int i;

  for (i = 0; table7[i]; i++)
    if (table7[i] == code) return (TRUE);
  return (FALSE);
}

WORD PASCAL ConvMaru(WORD code) {
  if (IsMaru(code)) return (code + 2);
  return 0;
}

WORD PASCAL HiraToKata(WORD code) {
  register int i;

  for (i = 0; i < 63; i++) {
    if (code == table5[i]) return table6[i];
  }
  for (i = 0; i < 63; i++) {
    if (code - 1 == table5[i])
      if (IsTenten(table5[i])) return table6[i] + 1;
    if (code - 2 == table5[i])
      if (IsMaru(table5[i])) return table6[i] + 2;
  }
  return code;
}

WORD PASCAL KataToHira(WORD code) {
  register int i;

  for (i = 0; i < 63; i++) {
    if (code == table6[i]) return table5[i];
  }
  for (i = 0; i < 63; i++) {
    if (code - 1 == table6[i])
      if (IsTenten(table6[i])) return table5[i] + 1;
    if (code - 2 == table6[i])
      if (IsMaru(table6[i])) return table5[i] + 2;
  }
  return code;
}

void PASCAL lHanToZen(LPTSTR lpDst, LPTSTR lpSrc, DWORD fdwConversion) {
  WORD code;
  WORD code0;
  WORD code1;

  while (*lpSrc) {
    WORD KatakanaLetter;

    KatakanaLetter = 0;
    code0 = (WORD)*lpSrc;
    code1 = (WORD) * (lpSrc + 1);

    lpSrc++;
    if ((code1 == 0xFF9E) || (code1 == 0xFF9F)) {
      KatakanaLetter = code0;
      code0 = code1;
      lpSrc++;
    }

    code = HanToZen(code0, KatakanaLetter, fdwConversion);
    *lpDst++ = code;
  }
  *lpDst = 0;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
