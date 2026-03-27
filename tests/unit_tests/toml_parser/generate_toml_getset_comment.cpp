/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#include <gtest/gtest.h>

#include "../../../sdv_services/core/toml_parser/parser_node_toml.h"
#include "../../../sdv_services/core/toml_parser/parser_toml.h"

TEST(GenerateTOML, GetCommentRoot)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(# This is a comment)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_before);
    EXPECT_TRUE(ssComment.empty());
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_behind);
    EXPECT_TRUE(ssComment.empty());
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before);
    EXPECT_TRUE(ssComment.empty());
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind);
    EXPECT_EQ(ssComment, "This is a comment");
}

TEST(GenerateTOML, GetMultiLineCommentRoot)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(
# This is a comment that stretches
# more than one line.

# And here's another comment of more
# than one line.
)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_before);
    EXPECT_TRUE(ssComment.empty());
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_behind);
    EXPECT_TRUE(ssComment.empty());
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before);
    EXPECT_TRUE(ssComment.empty());
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind);
    EXPECT_EQ(ssComment, R"code(This is a comment that stretches more than one line.
And here's another comment of more than one line.)code");
}

TEST(GenerateTOML, SetCommentRoot)
{
    toml_parser::CParser parser;

    // This will test the SetComment function and overwriting the existing comment.

    std::string ssTOMLInput = R"code(# This is a double line comment
# This is line two of the double line comment)code";
    std::string ssTOMLOutput = R"code(# This is comment way before

# This is comment before
# This is comment way behind
)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::comment_before , "This is comment before");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::comment_behind , "This is comment behind");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before , "This is comment way before");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind , "This is comment way behind");

    std::string ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_before);
    EXPECT_EQ(ssComment, "This is comment before");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_behind);
    EXPECT_EQ(ssComment, "This is comment behind");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before);
    EXPECT_EQ(ssComment, "This is comment way before");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind);
    EXPECT_EQ(ssComment, "This is comment way behind");

    std::string ssGenerated = parser.Root().GenerateTOML();
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, SetMultiLineCommentRoot)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput  = R"code(# This is a double line comment
# This is line two of the double line comment)code";
    std::string ssTOMLOutput = R"code(# This is comment way before

# And surprise, also a second line

# This is comment before
#
# With a second line
# This is comment way behind

# And final, also a second line
)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::comment_before, R"(This is comment before
With a second line)");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::comment_behind, R"(This is comment behind
Also with a second line)");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before, R"(This is comment way before
And surprise, also a second line)");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind, R"(This is comment way behind
And final, also a second line)");

    std::string ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_before);
    EXPECT_EQ(ssComment, R"(This is comment before
With a second line)");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_behind);
    EXPECT_EQ(ssComment, R"(This is comment behind
Also with a second line)");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before);
    EXPECT_EQ(ssComment, R"(This is comment way before
And surprise, also a second line)");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind);
    EXPECT_EQ(ssComment, R"(This is comment way behind
And final, also a second line)");

    std::string ssGenerated = parser.Root().GenerateTOML();
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, RemoveCommentRoot)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput  = R"code(# This is a double line comment
# This is line two of the double line comment)code";
    std::string ssTOMLOutput = R"code()code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind, std::string());

    std::string ssGenerated = parser.Root().GenerateTOML();
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, GetCommentRootValue)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(# This is a separate comment

# This is a comment before the value
value = "this is the value text" # Comment following the value

# This is also a separate comment)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));
    sdv::TInterfaceAccessPtr ptrValue = parser.Root().GetNodeDirect("value");
    ASSERT_TRUE(ptrValue);
    sdv::toml::INodeInfo* pComment = ptrValue.GetInterface<sdv::toml::INodeInfo>();
    ASSERT_NE(pComment, nullptr);

    std::string ssComment = pComment->GetComment(sdv::toml::INodeInfo::ECommentType::comment_before);
    EXPECT_EQ(ssComment, "This is a comment before the value");
    ssComment = pComment->GetComment(sdv::toml::INodeInfo::ECommentType::comment_behind);
    EXPECT_EQ(ssComment, "Comment following the value");
    ssComment = pComment->GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before);
    EXPECT_EQ(ssComment, "This is a separate comment");
    ssComment = pComment->GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind);
    EXPECT_EQ(ssComment, "This is also a separate comment");
}

TEST(GenerateTOML, GetMultiLineCommentRootValue)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(


# This is a separate comment with several line-breaks before.
# Followed by this text on the same line.
  
# Note: there was a space after the empty comment line.
# And another separate comment on a next line.

# This is a comment before the value.
# And another comment before the value at the same line.
# 
# Note: there was a space after the empty comment line.
# But that should not influence the comment lines.
value = "this is the value text" # Comment following the value.
                                 # More comment following the value.
                                 # This becomes one line.
                                 #
                                 # But this is a separate line.

# This is also a separate comment.
# Followed by this text on the same line.

# And another text on a separate line.)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));
    sdv::TInterfaceAccessPtr ptrValue = parser.Root().GetNodeDirect("value");
    ASSERT_TRUE(ptrValue);
    sdv::toml::INodeInfo* pComment = ptrValue.GetInterface<sdv::toml::INodeInfo>();
    ASSERT_NE(pComment, nullptr);

    std::string ssComment = pComment->GetComment(sdv::toml::INodeInfo::ECommentType::comment_before);
    EXPECT_EQ(ssComment, R"code(This is a comment before the value. And another comment before the value at the same line.
Note: there was a space after the empty comment line. But that should not influence the comment lines.)code");
    ssComment = pComment->GetComment(sdv::toml::INodeInfo::ECommentType::comment_behind);
    EXPECT_EQ(ssComment, R"code(Comment following the value. More comment following the value. This becomes one line.
But this is a separate line.)code");
    ssComment = pComment->GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before);
    EXPECT_EQ(ssComment, R"code(This is a separate comment with several line-breaks before. Followed by this text on the same line.
Note: there was a space after the empty comment line. And another separate comment on a next line.)code");
    ssComment = pComment->GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind);
    EXPECT_EQ(ssComment, R"code(This is also a separate comment. Followed by this text on the same line.
And another text on a separate line.)code");
}

TEST(GenerateTOML, SetCommentRootValue)
{
    toml_parser::CParser parser;

    // This will test the SetComment function and overwriting the existing comment.

    std::string ssTOMLInput = R"code(# This is a double line comment
# This is line two of the double line comment)code";
    std::string ssTOMLOutput = R"code(# This is comment #3

# This is comment #1
# This is comment #4
)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::comment_before , "This is comment #1");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::comment_behind , "This is comment #2");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before , "This is comment #3");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind , "This is comment #4");

    std::string ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_before);
    EXPECT_EQ(ssComment, "This is comment #1");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_behind);
    EXPECT_EQ(ssComment, "This is comment #2");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before);
    EXPECT_EQ(ssComment, "This is comment #3");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind);
    EXPECT_EQ(ssComment, "This is comment #4");

    std::string ssGenerated = parser.Root().GenerateTOML();
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, SetMultiLineCommentRootValue)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput  = R"code(# This is a double line comment
# This is line two of the double line comment)code";
    std::string ssTOMLOutput = R"code(# This is comment #3

# And surprise, also a second line

# This is comment #1
#
# With a second line
# This is comment #4

# And final, also a second line
)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::comment_before, R"(This is comment #1
With a second line)");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::comment_behind, R"(This is comment #2
Also with a second line)");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before, R"(This is comment #3
And surprise, also a second line)");
    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind, R"(This is comment #4
And final, also a second line)");

    std::string ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_before);
    EXPECT_EQ(ssComment, R"(This is comment #1
With a second line)");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::comment_behind);
    EXPECT_EQ(ssComment, R"(This is comment #2
Also with a second line)");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before);
    EXPECT_EQ(ssComment, R"(This is comment #3
And surprise, also a second line)");
    ssComment = parser.Root().GetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind);
    EXPECT_EQ(ssComment, R"(This is comment #4
And final, also a second line)");

    std::string ssGenerated = parser.Root().GenerateTOML();
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, RemoveCommentRootValue)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput  = R"code(# This is a double line comment
# This is line two of the double line comment)code";
    std::string ssTOMLOutput = R"code()code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    parser.Root().SetComment(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind, std::string());

    std::string ssGenerated = parser.Root().GenerateTOML();
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}
