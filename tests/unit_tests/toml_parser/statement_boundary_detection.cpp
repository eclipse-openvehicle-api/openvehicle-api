#include <gtest/gtest.h>
#include <limits>
#include <functional>
#include "../../../sdv_services/core/toml_parser/lexer_toml.h"
#include "../../../sdv_services/core/toml_parser/exception.h"
#include "../../../sdv_services/core/toml_parser/miscellaneous.h"

std::string DumpTokenList(toml_parser::CLexer& rLexer)
{
    auto eNavMode = rLexer.NavigationMode();
    rLexer.NavigationMode(toml_parser::CLexer::ENavigationMode::do_not_skip_anything);
    rLexer.Reset();
    std::stringstream sstream;
    while (!rLexer.IsEnd())
    {
        const auto& rToken = rLexer.Consume();
        switch (rToken.Category())
        {
        case toml_parser::ETokenCategory::token_none:                           sstream << "<none>";                                break;
        case toml_parser::ETokenCategory::token_syntax_assignment:              sstream << "=";                                     break;
        case toml_parser::ETokenCategory::token_syntax_array_open:              sstream << "[";                                     break;
        case toml_parser::ETokenCategory::token_syntax_array_close:             sstream << "]";                                     break;
        case toml_parser::ETokenCategory::token_syntax_table_open:              sstream << "[";                                     break;
        case toml_parser::ETokenCategory::token_syntax_table_close:             sstream << "]";                                     break;
        case toml_parser::ETokenCategory::token_syntax_table_array_open:        sstream << "[{";                                    break;
        case toml_parser::ETokenCategory::token_syntax_table_array_close:       sstream << "}]";                                    break;
        case toml_parser::ETokenCategory::token_syntax_inline_table_open:       sstream << "{";                                     break;
        case toml_parser::ETokenCategory::token_syntax_inline_table_close:      sstream << "}";                                     break;
        case toml_parser::ETokenCategory::token_syntax_comma:                   sstream << ", ";                                    break;
        case toml_parser::ETokenCategory::token_syntax_dot:                     sstream << ".";                                     break;
        case toml_parser::ETokenCategory::token_syntax_new_line:                sstream << std::endl;                               break;
        case toml_parser::ETokenCategory::token_key:                            sstream << rToken.StringValue();                    break;
        case toml_parser::ETokenCategory::token_string:
            sstream << toml_parser::QuoteText(rToken.StringValue(),
                rToken.StringType() == toml_parser::ETokenStringType::literal_string ?
                    toml_parser::EQuoteRequest::quoted_text : toml_parser::EQuoteRequest::literal_text);
            break;
        case toml_parser::ETokenCategory::token_integer:                        sstream << rToken.IntegerValue();                   break;
        case toml_parser::ETokenCategory::token_float:                          sstream << rToken.FloatValue();                     break;
        case toml_parser::ETokenCategory::token_boolean:                        sstream << (rToken.BooleanValue() ? "true" : "false"); break;
        case toml_parser::ETokenCategory::token_time_local:                     sstream << "<time>";                                break;
        case toml_parser::ETokenCategory::token_date_time_offset:               sstream << "<date_time>";                           break;
        case toml_parser::ETokenCategory::token_date_time_local:                sstream << "<date_time>";                           break;
        case toml_parser::ETokenCategory::token_date_local:                     sstream << "<date>";                                break;
        case toml_parser::ETokenCategory::token_whitespace:                     sstream << " ";                                     break;
        case toml_parser::ETokenCategory::token_comment:                        sstream << rToken.RawString();                      break;
        case toml_parser::ETokenCategory::token_error:                          sstream << "<error>";                               break;
        case toml_parser::ETokenCategory::token_empty:                          sstream << "<empty>";                               break;
        case toml_parser::ETokenCategory::token_terminated:                     sstream << "<terminated>";                          break;
        default:                                                                sstream << "<unknown>";                             break;
        }

    }
    rLexer.Reset();
    rLexer.NavigationMode(eNavMode);
    return sstream.str();
}

/**
 * @brief Helper function to determine the statemement boundaries based on a supplied key.
 * @remarks Works only when a token list and a token location has been assigned to the token.
 * @details There are assignment statements and table statements (and table array statements). 
 * @param[in] rKey Reference to the key token.
 * @return Pair of key references containing the begin and end boundary of the complete statement belonging to the key. Or returns
 * a pair of empty tokens when the statement could not be determined.
 */
std::pair<const toml_parser::CToken&, const toml_parser::CToken&> StatementBoundaries(const toml_parser::CToken& rKey)
{
    static toml_parser::CToken tokenNone;

    // Check whether the token is a valid key token.
    if (rKey.Category() != toml_parser::ETokenCategory::token_key)
        return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);
    
    // Check for a table or table array - meaning the token is preceded by a '[' or a '[[' (ignoring whitespace, newlines and
    // comments).
    std::reference_wrapper<const toml_parser::CToken> refBegin = rKey;
    std::reference_wrapper<const toml_parser::CToken> refToken = rKey;
    enum class EType {table, table_array, assignment} eType = EType::assignment;
    bool bScan = true;
    while (bScan)
    {
        refToken = refToken.get().Prev();
        if (!refToken.get()) break;
        switch (refToken.get().Category())
        {
        case toml_parser::ETokenCategory::token_syntax_table_open:
            eType = EType::table;
            refBegin = refToken;
            bScan = false;
            break;
        case toml_parser::ETokenCategory::token_syntax_table_array_open:
            eType = EType::table_array;
            refBegin = refToken;
            bScan = false;
            break;
        case toml_parser::ETokenCategory::token_key:
        case toml_parser::ETokenCategory::token_syntax_dot:
        case toml_parser::ETokenCategory::token_whitespace:
        case toml_parser::ETokenCategory::token_syntax_new_line:
        case toml_parser::ETokenCategory::token_comment:
            break;
        default:
            bScan = false;
            break;
        }
    }

    // Check for the end of the statement
    std::stack<toml_parser::ETokenCategory> stackProcessing;
    std::reference_wrapper<const toml_parser::CToken> refEnd = rKey;
    refToken = rKey;
    bScan = true;
    enum class EAssignmentState {none, key, assignment, value}
        eAssignmentState = (eType == EType::assignment ? EAssignmentState::key : EAssignmentState::none);
    while (bScan)
    {
        refToken = refToken.get().Next();
        if (!refToken.get()) break;
        switch (refToken.get().Category())
        {
        case toml_parser::ETokenCategory::token_syntax_dot:
            // When the assignment has a key followed by a dot, the assignment is actually an inline table with values.
            // Or this is not an assignment, but a parent with child table
            if (eType == EType::assignment && eAssignmentState != EAssignmentState::key)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);
            eAssignmentState = EAssignmentState::none;

            // If there is a table or table-array statement, this is not part of the parent. Re-set the start position.
            refBegin = rKey;

            // This finalizes the statement.
            bScan  = false;
            break;
        case toml_parser::ETokenCategory::token_syntax_assignment:
            // Ignore when assignments are on the stack (array or inline-table).
            if (!stackProcessing.empty())
                break;

            // Check for anything but an assignment (which is an error).
            if (eType != EType::assignment)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Check for the current state; assignment character needed
            if (eAssignmentState != EAssignmentState::key)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Set the current end and the assignment state.
            refEnd = refToken;
            eAssignmentState = EAssignmentState::assignment;
            break;
        case toml_parser::ETokenCategory::token_syntax_inline_table_open:
            // Check for anything but an assignment (which is an error).
            if (eType != EType::assignment)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Check for the current state; value assignment needed
            if (eAssignmentState != EAssignmentState::assignment)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Set the current end.
            refEnd = refToken;

            // Add the category to the stack
            stackProcessing.push(toml_parser::ETokenCategory::token_syntax_inline_table_open);
            break;
        case toml_parser::ETokenCategory::token_syntax_array_open:
            // Check for anything but an assignment (which is an error).
            if (eType != EType::assignment)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Check for the current state; value assignment needed
            if (eAssignmentState != EAssignmentState::assignment)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Set the current end.
            refEnd = refToken;

            // Add the category to the stack
            stackProcessing.push(toml_parser::ETokenCategory::token_syntax_array_open);
            break;
        case toml_parser::ETokenCategory::token_syntax_table_close:
            // Check for anything but a table.
            if (eType != EType::table)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // This finalizes the statement.
            refEnd = refToken;
            bScan  = false;
            break;
        case toml_parser::ETokenCategory::token_syntax_table_array_close:
            // Check for anything but a table array.
            if (eType != EType::table_array)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // This finalizes the statement.
            refEnd = refToken;
            bScan  = false;
            break;
        case toml_parser::ETokenCategory::token_syntax_inline_table_close:
            // Check for a fitting stack; if not existing, this is an error.
            if (stackProcessing.empty() || stackProcessing.top() != toml_parser::ETokenCategory::token_syntax_inline_table_open)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Set the current end.
            refEnd = refToken;

            // Reduce the stack and set the current token.
            stackProcessing.pop();

            // If the stack is empty, the value assignment was made
            if (stackProcessing.empty())
            {
                eAssignmentState = EAssignmentState::value;
                bScan = false;
            }
            break;
        case toml_parser::ETokenCategory::token_syntax_array_close:
            // Check for a fitting stack; if not existing, this is an error.
            if (stackProcessing.empty() || stackProcessing.top() != toml_parser::ETokenCategory::token_syntax_array_open)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Set the current end.
            refEnd = refToken;

            // Reduce the stack and set the current token.
            stackProcessing.pop();

            // If the stack is empty, the value assignment was made
            if (stackProcessing.empty())
            {
                eAssignmentState = EAssignmentState::value;
                bScan = false;
            }
            break;
        case toml_parser::ETokenCategory::token_whitespace:
        case toml_parser::ETokenCategory::token_syntax_new_line:
        case toml_parser::ETokenCategory::token_comment:
            break;
        case toml_parser::ETokenCategory::token_syntax_comma:
        case toml_parser::ETokenCategory::token_key:
            // Check for a fitting stack; if existing, this statement is valid.
            if (!stackProcessing.empty())
                break;

            // The assignment state should indicate a value.
            if (eAssignmentState != EAssignmentState::value)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Indication of the end of the statement (not including this token). 
            bScan = false;
            break;
        case toml_parser::ETokenCategory::token_string:
        case toml_parser::ETokenCategory::token_integer:
        case toml_parser::ETokenCategory::token_float:
        case toml_parser::ETokenCategory::token_boolean:
        case toml_parser::ETokenCategory::token_time_local:
        case toml_parser::ETokenCategory::token_date_time_offset:
        case toml_parser::ETokenCategory::token_date_time_local:
        case toml_parser::ETokenCategory::token_date_local:
            // Ignore when assignments are on the stack (array or inline-table).
            if (!stackProcessing.empty())
                break;

            // Check for anything but an assignment (which is an error).
            if (eType != EType::assignment)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Check for the current state; value needed.
            if (eAssignmentState != EAssignmentState::assignment)
                return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

            // Set the current end.
            refEnd = refToken;

            // The value assignment was made
            eAssignmentState = EAssignmentState::value;

            // End of the statement.
            bScan = false;
            break;
        default:
            // Invalid in this constellation
            return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);
        }
    }

    // There should be no stack entry any more...
    if (!stackProcessing.empty())
        return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

    // Assignment state should be none or value.
    if (eAssignmentState != EAssignmentState::none && eAssignmentState != EAssignmentState::value)
        return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(tokenNone, tokenNone);

    // Return the statement boundaries
    return std::pair<const toml_parser::CToken&, const toml_parser::CToken&>(refBegin.get(), refEnd.get());
}

/**
 * @brief Return the token category as string.
 * @param[in] eCategory The token category to return the text for.
 * @return The string with the category text.
 */
std::string CategoryText(toml_parser::ETokenCategory eCategory)
{
    switch (eCategory)
    {
    case toml_parser::ETokenCategory::token_none:
        return "token_none";
    case toml_parser::ETokenCategory::token_syntax_assignment:
        return "token_syntax_assignment";
    case toml_parser::ETokenCategory::token_syntax_array_open:
        return "token_syntax_array_open";
    case toml_parser::ETokenCategory::token_syntax_array_close:
        return "token_syntax_array_close";
    case toml_parser::ETokenCategory::token_syntax_table_open:
        return "token_syntax_table_open";
    case toml_parser::ETokenCategory::token_syntax_table_close:
        return "token_syntax_table_close";
    case toml_parser::ETokenCategory::token_syntax_table_array_open:
        return "token_syntax_table_array_open";
    case toml_parser::ETokenCategory::token_syntax_table_array_close:
        return "token_syntax_table_array_close";
    case toml_parser::ETokenCategory::token_syntax_inline_table_open:
        return "token_syntax_inline_table_open";
    case toml_parser::ETokenCategory::token_syntax_inline_table_close:
        return "token_syntax_inline_table_close";
    case toml_parser::ETokenCategory::token_syntax_comma:
        return "token_syntax_comma";
    case toml_parser::ETokenCategory::token_syntax_dot:
        return "token_syntax_dot";
    case toml_parser::ETokenCategory::token_syntax_new_line:
        return "token_syntax_new_line";
    case toml_parser::ETokenCategory::token_key:
        return "token_key";
    case toml_parser::ETokenCategory::token_string:
        return "token_string";
    case toml_parser::ETokenCategory::token_integer:
        return "token_integer";
    case toml_parser::ETokenCategory::token_float:
        return "token_float";
    case toml_parser::ETokenCategory::token_boolean:
        return "token_boolean";
    case toml_parser::ETokenCategory::token_time_local:
        return "token_time_local";
    case toml_parser::ETokenCategory::token_date_time_offset:
        return "token_date_time_offset";
    case toml_parser::ETokenCategory::token_date_time_local:
        return "token_date_time_local";
    case toml_parser::ETokenCategory::token_date_local:
        return "token_date_local";
    case toml_parser::ETokenCategory::token_whitespace:
        return "token_whitespace";
    case toml_parser::ETokenCategory::token_comment:
        return "token_comment";
    case toml_parser::ETokenCategory::token_error:
        return "token_error";
    case toml_parser::ETokenCategory::token_empty:
        return "token_empty";
    case toml_parser::ETokenCategory::token_terminated:
        return "token_terminated";
    default:
        return "<unknown>";
    }
}


/**
 * @brief Find routine, extend the line and check for smart boundaries
 * @param[in] rlexer Reference to the lexer to search for the tokens. The position will be reset and consumed until the
 * requested key has been found. Attention: this could be a parent key of a parent-child-composition.
 * @param[in] rssKey The key to search for.
 * @param[in] rvecCategories Reference to a vector containing the categories expected to be found.
 * @return Returns whether the key has been found and fit the list of provided categories.
 */
bool FindAndExtendToken(toml_parser::CLexer& rlexer, const std::string& rssKey,
    const std::vector<toml_parser::ETokenCategory>& rvecCategories)
{
    // Find the key
    rlexer.Reset();
    while (!rlexer.IsEnd())
    {
        const toml_parser::CToken& rToken = rlexer.Consume();
        if (rToken.Category() == toml_parser::ETokenCategory::token_key && rToken.StringValue() == rssKey)
        {
            // Determine the range boundaries and create the range
            auto prBoundaries = StatementBoundaries(rToken);
            EXPECT_TRUE(prBoundaries.first);
            EXPECT_TRUE(prBoundaries.second);
            if (!prBoundaries.first || !prBoundaries.second)
                return false;
            toml_parser::CNodeTokenRange range(toml_parser::CTokenRange(prBoundaries.first, prBoundaries.second.Next()));

            // Auto extend the boundaries
            rlexer.SmartExtendNodeRange(range);
            EXPECT_TRUE(range.ExtendedNode().Begin());
            std::reference_wrapper<const toml_parser::CToken> refToken(range.ExtendedNode().Begin());
            size_t nIndex = 0;
            auto itCategory = rvecCategories.begin();
            do
            {
                if (nIndex == rvecCategories.size())
                    std::cout << "Find " << rssKey << " Index #" << nIndex << ": Received " << CategoryText(refToken.get().Category()) << std::endl;
                EXPECT_LT(nIndex, rvecCategories.size());
                if (itCategory == rvecCategories.end()) return false;
                if (*itCategory != refToken.get().Category())
                    std::cout << "Find " << rssKey << " Index #" << nIndex << ": Expected " << CategoryText(*itCategory) <<
                        ", received " << CategoryText(refToken.get().Category()) << std::endl;
                EXPECT_EQ(*itCategory, refToken.get().Category());
                nIndex++;
                itCategory++;
            } while ((refToken = refToken.get().Next()).get() && refToken.get() != range.ExtendedNode().End());
            return true; // successful
        }
    }
    return false; // When coming here, the key was not found
}

TEST(TOMLLexerStatementBoundaryTests, CheckEmptyRange)
{
    std::string ssEmpty;

    // Process the code
    toml_parser::CLexer lexerEmpty(ssEmpty);

    // Check for empty range
    toml_parser::CNodeTokenRange rangeEmpty(toml_parser::CTokenRange(lexerEmpty.Consume(), lexerEmpty.Peek()));
    EXPECT_FALSE(rangeEmpty.NodeMain().Begin());
    EXPECT_FALSE(rangeEmpty.NodeMain().End());
    EXPECT_THROW(lexerEmpty.SmartExtendNodeRange(rangeEmpty), sdv::toml::XTOMLParseException);
}

TEST(TOMLLexerStatementBoundaryTests, CheckInvalidRange)
{
    std::string ssCode  = R"code(
token_a = 10
)code";
    std::string ssOther = ssCode;

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);
    toml_parser::CLexer lexerOther(ssOther);

    // Check for token of different lexing process
    toml_parser::CNodeTokenRange rangeOther(toml_parser::CTokenRange(lexerOther.Consume(), lexerOther.Peek()));
    EXPECT_TRUE(rangeOther.NodeMain().Begin());
    EXPECT_TRUE(rangeOther.NodeMain().End());
    EXPECT_THROW(lexerCode.SmartExtendNodeRange(rangeOther), sdv::toml::XTOMLParseException);
}

TEST(TOMLLexerStatementBoundaryTests, StandardIntegerAssignment)
{
    std::string ssCode  = R"code(
token_a = 10
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundaries are the space and newlines following the statement.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_a",
        {
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, StandardStringAssignment)
{
    std::string ssCode  = R"code(
token_b = "abc"
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundary is the newline
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_b",
        {toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, AssignmentWithIndependentComment)
{
    std::string ssCode  = R"code(
token_c = 30.1

# middle followed by double lines


token_d = "def"
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundary contains whitespace and the newline, but not the comments
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_c",
        {toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_float,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, AssignmentCommentsNotPartOfIt)
{
    std::string ssCode  = R"code(
token_c = 30.1

# middle followed by double lines


token_d = "def"

# before
# more before
token_e = [10, 20, 30]
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundary doesn't contain the comments before or after, but does contain whitespace and newline.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_d",
        {toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, ArrayAssignmentWithPreceedingComment)
{
    std::string ssCode  = R"code(
token_d = "def"

# before
# more before
token_e = [10, 20, 30]

token_f = "ghi" # after
                # more after
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Array assignment. Extended boundary contains the comments before and whitespace and newline after.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_e",
        {toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_array_open,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_array_close,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, AssignmentWithFollowingComment)
{
    std::string ssCode  = R"code(
token_f = "ghi" # after
                # more after
# belonging to next
[token_g]
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundary includes following comments, whitespace and newlines.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_f",
        {toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, TableAssignmentWithDedicatedComment)
{
    std::string ssCode  = R"code(
token_f = "ghi" # after
                # more after
# belonging to next
[token_g]

)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Table. Extended boundary contains one comment line before (not the other two), and the whitespace and newlines after.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_g",
        {toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_table_open,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_table_close,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, AssignmentWithFollowingCommentWithoutIndentation)
{
    std::string ssCode  = R"code(
token_h = "jkl"#after without whitespace
# more after due to following empty line

[token_i]
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundary includes following comments, whitespace and newlines.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_h",
        {toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, AssignmentCommentsExcluded)
{
    std::string ssCode  = R"code(
token_h = "jkl"#after without whitespace
# more after due to following empty line

[token_i]
# not after
token_j = 20
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Table. Extended boundary contains no comments and no lines after.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_i",
        {toml_parser::ETokenCategory::token_syntax_table_open,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_table_close,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, SmartExtendTokenRange)
{
    std::string ssCode  = R"code(
[token_i]
# not after
token_j = 20
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundaries include the comment before (which is not part of the previous statement) and nothing else.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_j",
        {toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, StandardAssignmentWithIndentation)
{
    std::string ssCode  = R"code(
    token_k = 30

)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundaries are the space and newlines following the statement and the space before.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_k",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, ArrayOfStringsAssignment)
{
    std::string ssCode  = R"code(
    token_l = ["abc", "def", "ghi"]
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Array assignment. Extended boundary is the newline and preceeding space.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_l",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_array_open,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_syntax_array_close,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, InlineTableAssignmentWithIndependentComment)
{
    std::string ssCode  = R"code(
    token_m = {x = 10, str = "gfh"}

    # middle followed by double newlines


    token_n = 100
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Inline table. Extended boundary contains whitespace and the newline, but not the comments; preceeding and following.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_m",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_inline_table_open,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_syntax_inline_table_close,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, IndentedAssignmentNoComments)
{
    std::string ssCode  = R"code(
    # begin followed by double newlines


    token_n = 100

    # before
    # more before
    token_o = 123.456
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundary doesn't contain the comments before or after, but does contain whitespace and newlines.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_n",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, IndentedAssignmentWithCommentsBefore)
{
    std::string ssCode  = R"code(

    # before
    # more before
    token_o = 123.456

)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundary contains the comments and whitespace before and newline after.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_o",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_float,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, IndentedBooleanArrayAssignmentWithDedicatedComments)
{
    std::string ssCode  = R"code(

    token_p = [true, false] # after
            # more after
    # belonging to next
    token_q = "next"
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Array assignment. Extended boundary includes following comments, whitespace and newlines and preceeding whitespace.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_p",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_array_open,
            toml_parser::ETokenCategory::token_boolean,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_boolean,
            toml_parser::ETokenCategory::token_syntax_array_close,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, IndentedAssignmentWithDedicatedCommentsBefore)
{
    std::string ssCode  = R"code(
    token_p = [true, false] # after
            # more after
    # belonging to next
    token_q = "next"

)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundary contains one comment line before (not the other two), and the whitespace and newlines after.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_q",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, IndentedAssignmentFollowedByCommentWithoutSpace)
{
    std::string ssCode  = R"code(
    token_r =987#after without whitespace
    # more after due to following empty line

    [token_s]
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Assignment. Extended boundary includes following comments, whitespace and newlines.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_r",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, IndentedTableWithoutComments)
{
    std::string ssCode  = R"code(
    [token_s]
    # not after
    [token_t]
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Table. Extended boundary contains no comments and no lines after, but whitespace before.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_s",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_table_open,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_table_close,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, IndenteTableWithAndWithoutComments)
{
    std::string ssCode  = R"code(
    [token_s]
    # not after
    [token_t]

# comment before a table member (belongs to token_bb)
token_u.token_aa.token_bb = 10     # table token_u has a table token_aa which has a value token_bb
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Table. Extended boundaries include the comment before (which is not part of the previous statement) and nothing else.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_t",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_table_open,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_table_close,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, InlineTableWithParentChildAssignment)
{
    std::string ssCode  = R"code(
# comment before a table member (belongs to token_bb)
token_u.token_aa.token_bb = 10     # table token_u has a table token_aa which has a value token_bb
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Table value. Extended boundaries include the dot.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_u",
        {toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_dot}));
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_aa",
        {toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_dot,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_dot}));
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_bb",
        {toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_dot,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_dot,
            toml_parser::ETokenCategory::token_key, 
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, ComplexInlineTableWithParentChildAssignmentOfTablesAndMultiDimensionalArrays)
{
    std::string ssCode  = R"code(
# Super inline table with sub-tables and arrays
token_u.token_aa.token_cc = { dd = { ee = 10, ff = 11 },    # this is the comment for dd
                              gg = [{hh = 1, ii = 2},
                                    {hh = 3, ii = 4},
                                    {hh = 5},
                                    {ii = 6}],             # this is the comment for gg
                                                           # and this as well
                              jj = [["abc", "def"], [1, 2, 3], []]}

)code";

    // NOTE EVE 22.10.2025: the value token_u.token_aa.token_cc.jj is the last value in the table. This means that the scope of
    // the value included the comma before the value jj (following value gg). This has the consequence, that the comments, which
    // actually belong to ggh are also part of this and if deleted or inserted before, the comments are at the wrong location.
    // This currently is by design and cannot be changed so easily (needs a different type of parser).

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Complex table value with multiple inline tables and multi-dimensional arrays.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "ee",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_comma}));
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "ff",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_whitespace}));
     EXPECT_TRUE(FindAndExtendToken(lexerCode, "dd",
         {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_inline_table_open,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_inline_table_close,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line}));
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "hh",
        {toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace}));
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "ii",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer}));
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "gg",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,                         // gg
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_array_open,
            toml_parser::ETokenCategory::token_syntax_inline_table_open,
            toml_parser::ETokenCategory::token_key,                         // hh = 1
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,                         // ii = 2
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_inline_table_close,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_inline_table_open,
            toml_parser::ETokenCategory::token_key,                         // hh = 3
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,                         // ii = 4
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_inline_table_close,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_inline_table_open,
            toml_parser::ETokenCategory::token_key,                         // hh = 5
            toml_parser::ETokenCategory::token_whitespace,  
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_inline_table_close,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_inline_table_open,
            toml_parser::ETokenCategory::token_key,                         // ii = 6
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_inline_table_close,
            toml_parser::ETokenCategory::token_syntax_array_close,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line}));
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "jj",
        {toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_assignment,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_array_open,
            toml_parser::ETokenCategory::token_syntax_array_open,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_string,
            toml_parser::ETokenCategory::token_syntax_array_close,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_array_open,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_integer,
            toml_parser::ETokenCategory::token_syntax_array_close,
            toml_parser::ETokenCategory::token_syntax_comma,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_syntax_array_open,
            toml_parser::ETokenCategory::token_syntax_array_close,
            toml_parser::ETokenCategory::token_syntax_array_close}));
}

TEST(TOMLLexerStatementBoundaryTests, ArrayOfTables)
{
    std::string ssCode  = R"code(
[[token_v]]
token_kk = 10
token_ll = 20
[token_w]
token_mm = 30
[[token_v]]
token_kk = 110
token_ll = 120
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Table of arrays.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_v",
        {toml_parser::ETokenCategory::token_syntax_table_array_open,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_table_array_close,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}

TEST(TOMLLexerStatementBoundaryTests, ParentChildTable)
{
    std::string ssCode  = R"code(
[[token_v]]
token_kk = 110
token_ll = 120
# Comments before (belongs ot child only)
[token_x.token_nn] # Comments following (belongs to child only)
)code";

    // Process the code
    toml_parser::CLexer lexerCode(ssCode);

    // Table in implcit table.
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_x",
        {toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_dot}));
    EXPECT_TRUE(FindAndExtendToken(lexerCode, "token_nn",
        {toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line,
            toml_parser::ETokenCategory::token_syntax_table_open,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_dot,
            toml_parser::ETokenCategory::token_key,
            toml_parser::ETokenCategory::token_syntax_table_close,
            toml_parser::ETokenCategory::token_whitespace,
            toml_parser::ETokenCategory::token_comment,
            toml_parser::ETokenCategory::token_syntax_new_line}));
}