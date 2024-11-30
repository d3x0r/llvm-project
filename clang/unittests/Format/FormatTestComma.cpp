//===- unittest/Format/FormatTestJS.cpp - Formatting unit tests for JS ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FormatTestUtils.h"
#include "clang/Format/Format.h"
#include "llvm/Support/Debug.h"
#include "gtest/gtest.h"

#define DEBUG_TYPE "format-test"

namespace clang {
namespace format {

class FormatTestComma : public testing::Test {
protected:
  static std::string format(StringRef Code, unsigned Offset, unsigned Length,
                            const FormatStyle &Style) {
    LLVM_DEBUG(llvm::errs() << "---\n");
    LLVM_DEBUG(llvm::errs() << Code << "\n\n");
    std::vector<tooling::Range> Ranges(1, tooling::Range(Offset, Length));
    FormattingAttemptStatus Status;
    tooling::Replacements Replaces =
        reformat(Style, Code, Ranges, "<stdin>", &Status);
    EXPECT_TRUE(Status.FormatComplete);
    auto Result = applyAllReplacements(Code, Replaces);
    EXPECT_TRUE(static_cast<bool>(Result));
    LLVM_DEBUG(llvm::errs() << "\n" << *Result << "\n\n");
    return *Result;
  }

  static std::string format(
      StringRef Code,
      const FormatStyle &Style = getGoogleStyle(FormatStyle::LK_JavaScript)) {
    return format(Code, 0, Code.size(), Style);
  }

  static FormatStyle getLLVMStyleWithColumnsAndCommaBreak(unsigned ColumnLimit) {
    FormatStyle Style = getLLVMStyle(FormatStyle::LK_None);
    Style.ColumnLimit = ColumnLimit;
    Style.BreakBeforeComma = true;
    Style.FixBinaryOperatorBreak = true;

    return Style;
  }

  static void verifyFormat(
      StringRef Code,
      const FormatStyle &Style = getGoogleStyle(FormatStyle::LK_JavaScript)) {
    EXPECT_EQ(Code.str(), format(Code, Style)) << "Expected code is not stable";
    std::string Result = format(test::messUp(Code), Style);
    EXPECT_EQ(Code.str(), Result) << "Formatted:\n" << Result;
  }

  static void verifyFormat(
      StringRef Expected, StringRef Code,
      const FormatStyle &Style = getGoogleStyle(FormatStyle::LK_JavaScript)) {
    EXPECT_EQ(Expected.str(), format(Expected, Style))
        << "Expected code is not stable";
    std::string Result = format(Code, Style);
    EXPECT_EQ(Expected.str(), Result) << "Formatted:\n" << Result;
  }
};

TEST_F(FormatTestComma, WillWrap) {

  EXPECT_EQ("aaaaa(aaaaa\n"
            "      , bbbbb(bbbbb,\n"
            "              ccccc(ccccc)))",
            format("aaaaa(aaaaa,bbbbb(bbbbb,ccccc(ccccc)))",
                   getLLVMStyleWithColumnsAndCommaBreak(28)));

  EXPECT_EQ("aaaaa(aaaaa\n"
            "      , bbbbb(bbbbb\n"
            "              , ccccc(ccccc)))",
            format("aaaaa(aaaaa,bbbbb(bbbbb,ccccc(ccccc)))",
                   getLLVMStyleWithColumnsAndCommaBreak(32)));
}

TEST_F(FormatTestComma, AlignTopInteraction) {

  FormatStyle Style31t = getLLVMStyleWithColumnsAndCommaBreak(27);
  Style31t.AlignAfterOpenBracket = FormatStyle::BAS_AlignTop;

  EXPECT_EQ("aaaaa(aaaaa\n"
            "     , bbbbb(bbbbb,\n"
            "            ccccc(ccccc)))",
            format("aaaaa(aaaaa,bbbbb(bbbbb,ccccc(ccccc)))", Style31t));

  FormatStyle Style32t = getLLVMStyleWithColumnsAndCommaBreak(28);
  Style32t.AlignAfterOpenBracket = FormatStyle::BAS_AlignTop;

  EXPECT_EQ("aaaaa(aaaaa\n"
            "     , bbbbb(bbbbb\n"
            "            , ccccc(ccccc)))",
            format("aaaaa(aaaaa,bbbbb(bbbbb,ccccc(ccccc)))", Style32t));
}

TEST_F(FormatTestComma, AlignTopWithSpaces) {
  FormatStyle Style28t2 = getLLVMStyleWithColumnsAndCommaBreak(28);
  Style28t2.AlignAfterOpenBracket = FormatStyle::BAS_AlignTop;
  Style28t2.SpacesInParens = FormatStyle::SpacesInParensStyle::SIPO_Custom;
  Style28t2.SpacesInParensOptions.InConditionalStatements = true;
  Style28t2.SpacesInParensOptions.Other = true;
  Style28t2.UseTab = FormatStyle::UT_ForIndentation;
  // Style28t2.AlignOperands =
  //    FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator;

  FormatStyle Style32t2 = getLLVMStyleWithColumnsAndCommaBreak(34);
  Style32t2.AlignAfterOpenBracket = FormatStyle::BAS_AlignTop;
  Style32t2.SpacesInParens = FormatStyle::SpacesInParensStyle::SIPO_Custom;
  Style32t2.SpacesInParensOptions.InConditionalStatements = true;
  Style32t2.SpacesInParensOptions.Other = true;
  Style32t2.UseTab = FormatStyle::UT_ForIndentation;
  // Style32t2.AlignOperands =
  //     FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator;

  // not enough spaces, so ',' at the end is cheaper than breaking by
  // 1 character.
  EXPECT_EQ("aaaaa( aaaaa,\n"
            "     bbbbb( bbbbb,\n"
            "          ccccc( ccccc ) ) )",
            format("aaaaa(aaaaa,bbbbb(bbbbb,ccccc(ccccc)))", Style28t2));

  // with just enough space, the comma is wrapped, and the extra character can
  // still fit.
  EXPECT_EQ("aaaaa( aaaaa\n"
            "     , bbbbb( bbbbb\n"
            "            , ccccc( ccccc ) ) )",
            format("aaaaa(aaaaa,bbbbb(bbbbb,ccccc(ccccc)))", Style32t2));
}

} // namespace format
} // end namespace clang
