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

class FormatTestLineComment : public testing::Test {
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

TEST_F(FormatTestLineComment, NoSingleComma) {

/*
    This previously would have formatted like the following:
    
out.data.size
      = snprintf( out.data.data, 32, "\x1b[%s;%sR",
               y ? GetText( y ) : "0" // col
               ,
               x ? GetText( x ) : "0" ); // row

   This is incorrect because the comma is on a line all by itself
*/
  FormatStyle Style = getLLVMStyleWithColumnsAndCommaBreak(80);

  Style.ContinuationIndentWidth = 5;
  
  Style.UseTab = FormatStyle::UT_ForIndentation;
  Style.AlignOperands = FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator;
  Style.SpacesInParens = FormatStyle::SIPO_Custom;
  Style.SpacesInParensOptions.Other = true;
  Style.SpacesInParensOptions.InConditionalStatements = true;
  Style.SpaceAfterLogicalNot = false;

  Style.SpacesInSquareBrackets = FormatStyle::SIAS_Always;
  Style.SpaceBeforeParens = FormatStyle::SBPO_Never;
  
  // Style.AlignAfterOpenBracket = FormatStyle::BAS_Align; // default LLVM
  Style.AlignAfterOpenBracket = FormatStyle::BAS_AlignTop;

  Style.BreakBinaryOperations = FormatStyle::BBO_RespectPrecedence;
  Style.BreakBeforeBinaryOperators = FormatStyle::BOS_All;
  Style.FixBinaryOperatorBreak = true; // allow breaks to happen
  Style.BinPackParameters = FormatStyle::BPPS_OnePerLine;
  Style.BinPackArguments = true;

  Style.ColumnLimit = 60;
  EXPECT_EQ("out.data.size = snprintf( out.data.data, 32, \"\x1b[%s;%sR\"\n"
            "                        , y ? GetText( y ) : \"0\"    // col\n"
            "                        , x ? GetText( x ) : \"0\" ); // row\n",
            format("out.data.size\n"
                   "     = snprintf( out.data.data, 32, \"\x1b[%s;%sR\", y ? GetText( y ) : \"0\" // col\n"
                   "               , x ? GetText( x ) : \"0\" );                                // row\n",
                   Style));

  Style.ColumnLimit = 45;
  EXPECT_EQ("out.data.size = snprintf(\n"
            "     out.data.data, 32, \"\x1b[%s;%sR\"\n"
            "     , y ? GetText( y ) : \"0\"    // col\n"
            "     , x ? GetText( x ) : \"0\" ); // row\n",
            format("out.data.size = snprintf(\n"
                   "     out.data.data, 32, \"\x1b[%s;%sR\"\n"
                   "     , y ? GetText( y ) : \"0\"    // col\n"
                   "     , x ? GetText( x ) : \"0\" ); // row\n",
                   Style));

  Style.ColumnLimit = 50;
  EXPECT_EQ("out.data.size\n"
            "     = snprintf( out.data.data, 32, \"\x1b[%s;%sR\"\n"
            "               , y ? GetText( y ) : \"0\"    // col\n"
            "               , x ? GetText( x ) : \"0\" ); // row\n",
            format("out.data.size\n"
                   "     = snprintf( out.data.data, 32, \"\x1b[%s;%sR\", y ? "
                   "GetText( y ) : \"0\" // col\n"
                   "               , x ? GetText( x ) : \"0\" );               "
                   "                 // row\n",
                   Style));

  Style.ColumnLimit = 50;
  EXPECT_EQ(
      "const serverOpts\n"
      "     = { resourcePath : appRoot + \"/ui\"\n"
      "       , npmPath : parentRoot + \"/..\"\n"
      "       , port : Number( process.env.PORT )\n"
      "              || config.port\n"
      "              || 8080\n"
      "       };",
      format("const serverOpts = {\n"
             "\tresourcePath : appRoot + \"/ui\"\n"
             "\t,\n"
             "\tnpmPath : parentRoot + \"/..\",\n"
             "\tport : Number( process.env.PORT ) || config.port || 8080\n"
             "};",
             Style));


  Style.ColumnLimit = 80;
  Style.AlignAfterOpenBracket = FormatStyle::BAS_DontAlign;
  EXPECT_EQ(
      "const serverOpts = {\n"
      "  resourcePath : appRoot + \"/ui\"\n"
      "  , npmPath : parentRoot + \"/..\"\n"
      "  , port : Number( process.env.PORT ) || config.port || 8080\n"
      "};",
      format("const serverOpts = {\n"
             "\tresourcePath : appRoot + \"/ui\"\n"
             "\t,\n"
             "\tnpmPath : parentRoot + \"/..\",\n"
             "\tport : Number( process.env.PORT ) || config.port || 8080\n"
             "};",
             Style));
}

} // namespace format
} // end namespace clang
