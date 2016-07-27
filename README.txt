What is this?
-------------

This is MZ-IME Japanese Input (mzimeja).
This program is a Japanese Input Method Editor (IME) for Windows 2000.

Table of Contents
-----------------

  --------------------+---------------------------------------------------
  (filenames)         | (descriptions)
  --------------------+---------------------------------------------------
  doc\                | the document folder (contains documentation)
  include\            | the include folder (contains C header files)
  projects\           | the projects folder (contains MSVC projects)
  res\                | the resource folder (contains dictionary)
  test\               | the test folder (contains test programs)
  .gitignore          | specify the untracked files Git should ignore
  cand_info.cpp       | candidate info of mzimeja
  comp_str.cpp        | composition string of mzimeja
  config.cpp          | mzimeja configuration
  convert.cpp         | mzimeja kana kanji conversion
  dict_compile.cpp    | dictionary compiler of MZ-IME Japanese Input
  dict_compile.rc     | resource of dict_compile
  imepad.cpp          | IME Pad (helps user input Kanji characters)
  imepad.rc           | resource of IME Pad
  imm.cpp             | IME/IMM related
  immsec.cpp          | IMM security related
  input.cpp           | mzimeja input context and related
  input.h             | mzimeja input context and related
  installer.iss       | Installer Settings for Inno Setup
  installerd.iss      | Installer Settings (Debug Version) for Inno Setup
  kanji.dat           | mzimeja kanji data
  keychar.cpp         | mzimeja keys and characters
  Makefile.msys       | Makefile for MSYS
  mzimeja.cpp         | MZ-IME Japanese Input (mzimeja)
  mzimeja.h           | MZ-IME Japanese Input (mzimeja)
  mzimeja.rc          | resource file of MZ-IME Japanese Input
  mzimeja.rc          | dictionary of MZ-IME Japanese Input
  process.cpp         | mzimeja input process
  radical.dat         | mzimeja kanji radical data
  regword.cpp         | registering words
  setup.cpp           | MZ-IME setup program
  setup.rc            | resource of MZ-IME Japanese Input Setup
  ui.cpp              | mzimeja UI server
  uicand.cpp          | mzimeja candidate window UI
  uicomp.cpp          | mzimeja composition window UI
  uiguide.cpp         | mzimeja guideline window UI
  uistate.cpp         | mzimeja status window UI
  --------------------+---------------------------------------------------

How To Build
------------

Use Visual C++ 2008 SP1 or later, or MSYS + g++.
Use the solution file "projects\mzimeja.sln" or Makefile.msys to build.

/////////////////////////////////////////////////////
// Katayama Hirofumi MZ (katahiromz) [ARMYANT]
// Homepage     http://katahiromz.web.fc2.com/
// BBS          http://katahiromz.bbs.fc2.com/
// E-Mail       katayama.hirofumi.mz@gmail.com
/////////////////////////////////////////////////////
