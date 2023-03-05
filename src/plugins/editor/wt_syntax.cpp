#include "wt_syntax.h"

#include "TextEditor.cpp"

const TextEditor::LanguageDefinition& TextEditor_LanguageDefinition_WTStar() {
    static bool inited = false;
    static TextEditor::LanguageDefinition langDef;
    if (!inited) {
        for (auto& k : wt_keywords)
            langDef.mKeywords.insert(k);

        for (uint i = 0; i < wt_identifiers.size(); ++i)
            langDef.mIdentifiers.insert(
                std::make_pair(wt_identifiers[i], TextEditor::Identifier(wt_identifiers_help[i])));

        langDef.mTokenize = [](const char* in_begin, const char* in_end, const char*& out_begin,
                               const char*& out_end,
                               TextEditor::PaletteIndex& paletteIndex) -> bool {
            paletteIndex = TextEditor::PaletteIndex::Max;

            while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
                in_begin++;

            if (in_begin == in_end) {
                out_begin = in_end;
                out_end = in_end;
                paletteIndex = TextEditor::PaletteIndex::Default;
            } else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
                paletteIndex = TextEditor::PaletteIndex::String;
            else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
                paletteIndex = TextEditor::PaletteIndex::CharLiteral;
            else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
                paletteIndex = TextEditor::PaletteIndex::Identifier;
            else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
                paletteIndex = TextEditor::PaletteIndex::Number;
            else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
                paletteIndex = TextEditor::PaletteIndex::Punctuation;

            return paletteIndex != TextEditor::PaletteIndex::Max;
        };

        langDef.mCommentStart = "/*";
        langDef.mCommentEnd = "*/";
        langDef.mSingleLineComment = "//";

        langDef.mCaseSensitive = true;
        langDef.mAutoIndentation = true;

        langDef.mName = "C++";

        inited = true;
    }
    return langDef;
}
