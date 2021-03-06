
#include "tesseract/training/ligature_table.h"

#include "tesseract/training/commandlineflags.h"
#include "tesseract/training/fileio.h"
#include "tesseract/training/pango_font_info.h"

DECLARE_STRING_PARAM_FLAG(fonts_dir);
DECLARE_STRING_PARAM_FLAG(fontconfig_tmpdir);

namespace {

using tesseract::File;
using tesseract::LigatureTable;
using tesseract::PangoFontInfo;

const char kEngNonLigatureText[] = "fidelity effigy ſteep";
// Same as above text, but with "fi" in the first word and "ffi" in the second
// word replaced with their respective ligatures.
const char kEngLigatureText[] = "ﬁdelity eﬃgy ﬅeep";
// Same as kEngLigatureText but with "fi" in both words replaced with their
// ligature. The test Verdana font does not support the "ffi" or "ſt" ligature.
const char kRenderableEngLigatureText[] = "ﬁdelity efﬁgy ſteep";

class LigatureTableTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    FLAGS_fonts_dir = File::JoinPath(FLAGS_test_srcdir, "testdata");
    FLAGS_fontconfig_tmpdir = FLAGS_test_tmpdir;
  }
  void SetUp() { lig_table_ = LigatureTable::Get(); }
  LigatureTable* lig_table_;
};

TEST_F(LigatureTableTest, DoesFillLigatureTables) {
  EXPECT_GT(lig_table_->norm_to_lig_table().size(), 0);
  EXPECT_GT(lig_table_->lig_to_norm_table().size(), 0);
}

TEST_F(LigatureTableTest, DoesAddLigatures) {
  EXPECT_STREQ(kEngLigatureText,
               lig_table_->AddLigatures(kEngNonLigatureText, nullptr).c_str());
}

TEST_F(LigatureTableTest, DoesAddLigaturesWithSupportedFont) {
  PangoFontInfo font;
  EXPECT_TRUE(font.ParseFontDescriptionName("Verdana"));
  EXPECT_STREQ(kRenderableEngLigatureText,
               lig_table_->AddLigatures(kEngNonLigatureText, &font).c_str());
}

TEST_F(LigatureTableTest, DoesNotAddLigaturesWithUnsupportedFont) {
  PangoFontInfo font;
  EXPECT_TRUE(font.ParseFontDescriptionName("Lohit Hindi"));
  EXPECT_STREQ(kEngNonLigatureText,
               lig_table_->AddLigatures(kEngNonLigatureText, &font).c_str());
}

TEST_F(LigatureTableTest, DoesRemoveLigatures) {
  EXPECT_STREQ(kEngNonLigatureText,
               lig_table_->RemoveLigatures(kEngLigatureText).c_str());
}

TEST_F(LigatureTableTest, TestCustomLigatures) {
  const char* kTestCases[] = {
      "act",       "a\uE003", "publiſh",    "publi\uE006", "ſince",
      "\uE007nce", "aſleep",  "a\uE008eep", "neceſſary",   "nece\uE009ary",
  };
  for (int i = 0; i < ARRAYSIZE(kTestCases); i += 2) {
    EXPECT_STREQ(kTestCases[i + 1],
                 lig_table_->AddLigatures(kTestCases[i], nullptr).c_str());
    EXPECT_STREQ(kTestCases[i],
                 lig_table_->RemoveLigatures(kTestCases[i + 1]).c_str());
    EXPECT_STREQ(kTestCases[i],
                 lig_table_->RemoveCustomLigatures(kTestCases[i + 1]).c_str());
  }
}

TEST_F(LigatureTableTest, TestRemovesCustomLigatures) {
  const char* kTestCases[] = {
      "fiction",
      "ﬁ\uE003ion",
      "ﬁction",
  };
  for (int i = 0; i < ARRAYSIZE(kTestCases); i += 3) {
    EXPECT_STREQ(kTestCases[i + 1],
                 lig_table_->AddLigatures(kTestCases[i], nullptr).c_str());
    EXPECT_STREQ(kTestCases[i + 2],
                 lig_table_->RemoveCustomLigatures(kTestCases[i + 1]).c_str());
  }
}
}  // namespace
