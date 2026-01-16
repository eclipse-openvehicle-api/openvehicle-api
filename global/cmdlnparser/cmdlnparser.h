#ifndef CMDLN_PARSER_H
#define CMDLN_PARSER_H


#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <list>
#include <set>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <vector>
#include <memory>
#include <optional>
#include <queue>
#include <support/pointer.h>
#include <support/string.h>
#include <support/sequence.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

/**
 * @brief Helper namespace
 */
namespace helper
{
    /**
     * @brief Right trim the string from any whitespace characters.
     * @param[in] rss Reference to the string to trim.
     * @return The trimmed string.
     */
    inline std::string rtrim(const std::string& rss)
    {
        const std::string ssWhitespace = " \t\n\r\f\v";
        std::string ssResult = rss;
        ssResult.erase(ssResult.find_last_not_of(ssWhitespace) + 1);
        return ssResult;
    }

    /**
     * @brief Left trim the string from any whitespace characters.
     * @param[in] rss Reference to the string to trim.
     * @return The trimmed string.
     */
    inline std::string ltrim(const std::string& rss)
    {
        const std::string ssWhitespace = " \t\n\r\f\v";
        std::string ssResult = rss;
        ssResult.erase(0, ssResult.find_first_not_of(ssWhitespace));
        return ssResult;
    }

    /**
     * @brief Left and right trim the string from any whitespace characters.
     * @param[in] rss Reference to the string to trim.
     * @return The trimmed string.
     */
    inline std::string trim(const std::string& rss)
    {
        return ltrim(rtrim(rss));
    }
} // namespace helper

// Forward declaration
class CArgumentDefBase;

/**
* @brief Prototype of CArgumentDefT<>.
* @tparam TVar The type of the argument variable.
* @tparam TEnable The type to allow argument specific specialization of this class.
*/
template <typename TVar, typename TEnable = void>
class CArgumentDefT;

/**
 * @brief Group definition.
 */
struct SGroupDef
{
    std::string     ssTitle;        ///< Title of the group.
    std::string     ssDescription;  ///< Description.
};

/**
 * @brief Argument iterator class, allowing iteration of the second until last arguments (the first argument is skipped, since
 * it represents the application name).
 */
class CArgumentIterator
{
public:
    /**
     * @brief Constructor allowing iteration through the list of arguments.
     * tparam Character type.
     * @param[in] nArgs The amount of arguments.
     * @param[in] rgszArgs Array of arguments.
     */
    template <typename TCharType>
    CArgumentIterator(size_t nArgs, const TCharType** rgszArgs);

    /**
     * @brief Get the next argument (if existing).
     * @return Returns the next argument (or false when there is no argument).
     */
    std::optional<std::string> GetNext();

    /**
     * @brief Returns the index of the last provided argument.
     * @return The index of the last provided argument.
     */
    size_t GetIndexOfLastArg() const;

private:
    size_t                      m_nCounter = 0;         ///< Counter that returns the index of the last provided argument.
    std::queue<std::string>     m_queueArguments;       ///< Vector with the provided arguments.
};

/**
 * @brief The class provides a generic implementation of command line argument parsing. Instantiate this class and define which
 * arguments are supported. The parsing is done by calling the Parse function. Parse errors are reported with std::exception.
 * @details This class allows the generic implementation of a command line parser by defining the arguments and the variables
 * connected to the arguments. The parsing of the command line is done by calling the function CCommandLine::Parse, which
 * automatically fills the corresponding variables when an argument is supplied on the command line.
 *
 * The following command line argument types are supported:<br>
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | Variable              | Description      | Source code example                                             | Argument Example  |
 * +=======================+==================+=================================================================+===================+
 * | bool                  | Boolean variable | bool bHelp = false;                                             | -help             |
 * |                       |                  | commandline.DefineOption("help", bHelp, "Show help.");          |                   |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | int8_t                | Integer variable | int iTest = 0;                                                  | -test_int=10      |
 * | uint8_t               |                  | commandline.DefineOption("test_int", iTest, "Example.");        |                   |
 * | int16_t               |                  | double dTest = 0.0;                                             |                   |
 * | uint16_t              |                  | commandline.DefineOption("test_d", dTest, "Example.");          | -test_d=1.1       |
 * | int32_t               |                  |                                                                 |                   |
 * | uint32_t              |                  |                                                                 |                   |
 * | int64_t               |                  |                                                                 |                   |
 * | uint64_t              |                  |                                                                 |                   |
 * | float                 |                  |                                                                 |                   |
 * | double                |                  |                                                                 |                   |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | std::vector           | Vector container | std::vector<int> vec; sdv::sequence<int> seq;                   | -vec_int=10,20    |
 * | std::list             | List container   | commandline.DefineOption("vec_int", vec, "Example");            |                   |
 * | sdv::sequence         | Sequence         | commandline.DefineOption("vec_int", seq, "Example");            |                   |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | bool                  | Flag variable    | bool bExample = false;                                          | -flag+            |
 * |                       |                  | commandline.DefineFlagOption("flag", bExample, "Example.");     | -flag-            |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | enum                  | Enum variable    | enum ETest { test1, test2, test3 };                             | -e_tst=tst2       |
 * |                       |                  | SEnumArgumentAssoc<ETest> rgsAssociations[] =                   |                   |
 * |                       |                  |     { { test1, "tst1", "Test the #1" },                         |                   |
 * |                       |                  |       { test2, "tst2", "Test the #2" },                         |                   |
 * |                       |                  |       { test3, "tst3", "Test the #3" } };                       |                   |
 * |                       |                  | ETest eTest = test3;                                            |                   |
 * |                       |                  | CArgumentDefT<ETest>& rEnumTest =                               |                   |
 * |                       |                  |     commandline.DefineOption("e_tst", eTest, "Test.");          |                   |
 * |                       |                  | rEnumTest.AddAssociations(rgsAssociations);                     |                   |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | std::vector           | Enum vector      | enum ETest { test1, test2, test3 };                             | -e_tst=tst2,tst3  |
 * | std::list             | Enum list        | SEnumArgumentAssoc<ETest> rgsAssociations[] =                   |                   |
 * | sdv::sequence         | Enum sequence    |     { { test1, "tst1", "Test the #1" },                         |                   |
 * |                       |                  |       { test2, "tst2", "Test the #2" },                         |                   |
 * |                       |                  |       { test3, "tst3", "Test the #3" } };                       |                   |
 * |                       |                  | std::vector&lt;ETest&gt; vecEnumArg;                            |                   |
 * |                       |                  | CArgumentDefT<std::vector<ETest>>& rEnumTest =                  |                   |
 * |                       |                  |     commandline.DefineOption("e_tst", vecEnumArg, "Test.");     |                   |
 * |                       |                  | rEnumTest.AddAssociations(rgsAssociations);                     |                   |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | std::string           | String variable  | std::string ssTest;                                             | -str=abc          |
 * |                       |                  | commandline.DefineOption("str", ssTest, "Example.");            | -str="abc def"    |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | std::vector           | String vector    | std::list<std::string> lst; sdv::sequence<sdv::u8string> seq    | -str=abc,"def"    |
 * | std::list             | String list      | commandline.DefineOption("lst_str", lst, "Example.");           |                   |
 * | sdv::sequence         | String sequence  | commandline.DefineOption("lst_str", seq, "Example.");           |                   |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | std::filesystem::path | Path variable    | std::filesystem::path pathTest;                                 | -path="test/file  |
 * |                       |                  | commandline.DefineOption("path", pathTest, "Example.");         |                   |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | std::vector           | Path vector      | std::list<std::filesystem::path> lstPathTest;                   | -pth="f1.tx","f2" |
 * | std::list             | Path list        | commandline.DefineOption("pth", lstPathTest, "Example.");       |                   |
 * | sdv::sequence         | Path sequence    |                                                                 |                   |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 * | any assignment        | Default argument | std::vector<std::string> args;                                  | abc def           |
 * |                       |                  | commandline.DefineDefaultArgument(vec, "text chunks");          |                   |
 * +-----------------------+------------------+-----------------------------------------------------------------+-------------------+
 *
 * The command line class holds a list of argument definitions. Dependable on the provided arguments, the argument definitions
 * receive the parsed values from the command line. Since so many different types of arguments are possible, the implementation
 * uses a construct of class derivation implementing portions of the arguments (e.g. value assignment, containers, flags, etc.).
 *
 * Implementing a new argument type, it is necessary to provide a definition and a value class for the type.
 */
class CCommandLine
{
    /// Friend class allowing to add additional names
    friend CArgumentDefBase;

public:
    /**
     * @brief Parsing flags.
     */
    enum class EParseFlags : uint32_t
    {
        assignment_character = 0x10,        ///< Default argument assignment: -option=ARG and --suboption=ARG.
        no_assignment_character = 0x20,     ///< When set, assignment arguments do not use the assignment character. This allows
                                            ///< values to be glueed directly to the (sub-)option as it is used by many
                                            ///< programs. -optionARG and --suboptionARG
        assignment_next_arg = 0x40,         ///< When set, assignment arguments provide the arguments as separate arguments:
                                            ///< -option ARG --suboption ARG. This is also a common way to provide options. NOTE:
                                            ///< although this is a very common way to provide options, arrays cannot be provided.
                                            ///< If multiple values need to be provided, each value has to be repeated using the
                                            ///< option tag.
    };

    /**
     * @brief Constructor
     * @param[in] uiFlags Zero or more flags of EParseFlags (default using assignment character).
     */
    CCommandLine(uint32_t uiFlags = static_cast<uint32_t>(EParseFlags::assignment_character));

    /**
     * @brief Destructor
     */
    ~CCommandLine();

    /**
     * @brief Return the application path.
     * @return Returns the application path.
     */
    std::filesystem::path GetApplicationPath() const;

    /**
     * @brief Get the parse flags that were supplied to the parse function.
     * @return The parse flags of zero or more flags of EParseFlags.
     */
    uint32_t GetParseFlags() const { return m_uiParseFlags; }

    /**
     * @brief Check whether the parse flag has been set.
     * @param[in] eParseFlag The parse flag to check for.
     * @return Returns whether the flag has been set, or not.
     */
    bool CheckParseFlag(EParseFlags eParseFlag) const { return m_uiParseFlags & static_cast<uint32_t>(eParseFlag); }

    /**
     * @brief Parse the command line, to be called after the command line class has been filled with argument definitions. This
     * function will trow exceptions using the std::exception mechanism.
     * tparam Character type.
     * @param[in] nArgs The amount of arguments.
     * @param[in] rgszArgs Array of arguments.
     */
    template <typename TCharType>
    void Parse(size_t nArgs, const TCharType** rgszArgs);

    /**
     * @brief Define a group that cover the options following.
     * @param[in] rssTitle Zero terminated string containing the title of the group.
     * @param[in] rssDescription The description of the group. This parameter is optional. If not supplied, can be NULL.
     */
    void DefineGroup(const std::string& rssTitle, const std::string& rssDescription = std::string{});

    /**
     * @brief Argument definition for a default argument, which is an argument without option (-/) character.
     * @tparam TVar Variable type of the argument to define.
     * @param[in, out] rtVar Reference to the argument that receives the content of the command line after parsing.
     * @param[in] rssHelpText Pointer to the zero terminated text to present when printing help information about the argument.
     * @return CArgumentDefT<TVar>& Reference to the argument command class, which can be used to set additional attributes.
     */
    template <typename TVar>
    CArgumentDefT<TVar>& DefineDefaultArgument(TVar& rtVar, const std::string& rssHelpText);

    /**
     * @brief Option definition, which is an argument with option (-/) character.
     * @tparam TVar Variable type of the argument to define.
     * @tparam TArgumentGroup Types of additional argument groups for showing argument specific help.
     * @param[in] rssArgument Pointer to the zero terminated string with the argument name.
     * @param[in, out] rtVar Reference to the argument that receives the content of the command line after parsing.
     * @param[in] rssHelpText Pointer to the zero terminated text to present when printing help information about the argument.
     * @param[in] bCaseSensitive Boolean telling whether the argument should be compared case sensitive or case insensitive.
     * @param[in] nArgumentGroup During the generation of the help test, to optionally show the options based on the arguments
     * supplied, argument groups can be used. The options are assigned to one or more argument groups and during the
     * PrintHelp function, the group to show will be supplied. The default group is group #0.
     * @param[in] nAdditionalGroups Zero or more additional group scan be supplied.
     * @return CArgumentDefT<TVar>& Reference to the argument command class, which can be used to set additional attributes.
     */
    template <typename TVar, typename... TArgumentGroup>
    CArgumentDefT<TVar>& DefineOption(const std::string& rssArgument, TVar& rtVar, const std::string& rssHelpText,
        bool bCaseSensitive = true, size_t nArgumentGroup = 0, TArgumentGroup... nAdditionalGroups);

    /**
     * @brief Sub-option definition, which is an argument with sub-option (--) characters.
     * @tparam TVar Variable type of the argument to define.
     * @tparam TArgumentGroup Types of additional argument groups for showing argument specific help.
     * @param[in] rssArgument Pointer to the zero terminated string with the argument name.
     * @param[in, out] rtVar Reference to the argument that receives the content of the command line after parsing.
     * @param[in] rssHelpText Pointer to the zero terminated text to present when printing help information about the argument.
     * @param[in] bCaseSensitive Boolean telling whether the argument should be compared case sensitive or case insensitive.
     * @param[in] nArgumentGroup During the generation of the help test, to optionally show the options based on the arguments
     * supplied, argument groups can be used. The options are assigned to one or more argument groups and during the
     * PrintHelp function, the group to show will be supplied. The default group is group #0.
     * @param[in] nAdditionalGroups Zero or more additional group scan be supplied.
     * @return CArgumentDefT<TVar>& Reference to the argument command class, which can be used to set additional attributes.
     */
    template <typename TVar, typename... TArgumentGroup>
    CArgumentDefT<TVar>& DefineSubOption(const std::string& rssArgument, TVar& rtVar, const std::string& rssHelpText,
        bool bCaseSensitive = true, size_t nArgumentGroup = 0, TArgumentGroup... nAdditionalGroups);

    /**
     * @brief Flag option definition, which is an argument with option (-/) character.
     * @tparam TArgumentGroup Types of additional argument groups for showing argument specific help.
     * @param[in] rssArgument Pointer to the zero terminated string with the argument name.
     * @param[in, out] rbFlag Reference to the argument that receives the content of the command line after parsing.
     * @param[in] rssHelpText Pointer to the zero terminated text to present when printing help information about the argument.
     * @param[in] bCaseSensitive Boolean telling whether the argument should be compared case sensitive or case insensitive.
     * @param[in] nArgumentGroup During the generation of the help test, to optionally show the options based on the arguments
     * supplied, argument groups can be used. The options are assigned to one or more argument groups and during the
     * PrintHelp function, the group to show will be supplied. The default group is group #0.
     * @param[in] nAdditionalGroups Zero or more additional group scan be supplied.
     * @return CArgumentDefT<TVar>& Reference to the argument command class, which can be used to set additional attributes.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT<bool>& DefineFlagOption(const std::string& rssArgument, bool& rbFlag, const std::string& rssHelpText,
        bool bCaseSensitive = true, size_t nArgumentGroup = 0, TArgumentGroup... nAdditionalGroups);

    /**
     * @brief Flag sub-option definition, which is an argument with option (-/) character.
     * @tparam TArgumentGroup Types of additional argument groups for showing argument specific help.
     * @param[in] rssArgument Pointer to the zero terminated string with the argument name.
     * @param[in, out] rbFlag Reference to the argument that receives the content of the command line after parsing.
     * @param[in] rssHelpText Pointer to the zero terminated text to present when printing help information about the argument.
     * @param[in] bCaseSensitive Boolean telling whether the argument should be compared case sensitive or case insensitive.
     * @param[in] nArgumentGroup During the generation of the help test, to optionally show the options based on the arguments
     * supplied, argument groups can be used. The options are assigned to one or more argument groups and during the
     * PrintHelp function, the group to show will be supplied. The default group is group #0.
     * @param[in] nAdditionalGroups Zero or more additional group scan be supplied.
     * @return CArgumentDefT<TVar>& Reference to the argument command class, which can be used to set additional attributes.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT<bool>& DefineFlagSubOption(const std::string& rssArgument, bool& rbFlag, const std::string& rssHelpText,
        bool bCaseSensitive = true, size_t nArgumentGroup = 0, TArgumentGroup... nAdditionalGroups);

    /**
     * @brief Set or remove a fixed with printing of the help text.
     * @param[in] nWidth The width of the printable area. Can be 0, then the width should be dynamically taken from the console
     * width the application is running in.
     */
    void PrintFixedWidth(size_t nWidth);

    /**
     * @brief Get the current fixed width for printing help. If no fixed width has been selected, returns 0.
     * @return Returns the fixed width selected to print help or 0 for dynamic width.
     */
    size_t PrintFixedWidth() const;

    /**
     * @brief Set the maximum width of printing the help text when the console width is larger. This will automatically select
     * dynamic width.
     * @param[in] nWidth The maximum width of the printing text if the console is larger.
     */
    void PrintMaxWidth(size_t nWidth);

    /**
     * @brief Get the maximum width of printing the help text. Could be 0 when there is no limit.
     * @return Returns the maximum width of the printing the help text.
     */
    size_t PrintMaxWidth() const;

    /**
     * @brief Enable or disable syntax explanation during printing of the help text.
     * @param[in] bEnable When set, enables the syntax printing; otherwise disables syntax printing.
     */
    void PrintSyntax(bool bEnable);

    /**
     * @brief Get the current syntax printing state.
     * @return The syntax printing state; either enabled, or disabled.
     */
    bool PrintSyntax() const;

    /**
     * @brief Print help information based on the configured arguments.
     * @tparam TGroup Zero or more help groups to show. If none are supplied, the default group #0 is shown.
     * @param[in] rstream The stream to be used to print the help.
     * @param[in] rssHelpText A description of the application formatted to fit the console width.
     * @param[in] nArgumentGroup During the generation of the help test, to optionally show the options based on the arguments
     * supplied, argument groups can be used. The options are assigned to one or more argument groups and during the
     * PrintHelp function, the group to show will be supplied. The default group is group #0.
     */
    void PrintHelp(std::ostream& rstream, const std::string& rssHelpText = std::string{}, size_t nArgumentGroup = 0) const;

    /**
     * @brief Print help text information (not printing the arguments).
     * @param[in] rstream The stream to be used to print the help.
     * @param[in] rssHelpText A description of the application formatted to fit the console width.
     * @param[in] nPrintWidth The width of the printable area. Can be 0, then the width is taken from the console width the
     * application is running in.
     */
    static void PrintHelpText(std::ostream& rstream, const std::string& rssHelpText, size_t nPrintWidth = 0);

    /**
     * @brief Dump command line arguments.
     * @param[in, out] rstream Reference to the stream to dump the arguments to.
     * @param[in] bAll When set, dump all arguments; otherwise only the ones that we provided over the command line.
     */
    void DumpArguments(std::ostream& rstream, bool bAll = true) const;

    /**
     * @brief Get a list of incompatible supplied arguments base don the supplied argument group.
     * @details Options can be assigned one or more groups. This can be useful to selective enable/disable options based on the
     * state on the command line (e.g. based on a command provided on the command line). Not all options are compatible to the
     * currently active group and the compatibility can be checked using this function. Provided options belonging to group 0 are
     * always compatible. For all other provided options, the compatibility is checked with the provided argument group number.
     * @param[in] nArgumentGroup Argument group that currently defines the compatibility.
     * @param[in] bFull When set, returns the complete supplied argument text. If not, returns the option label only.
     * @return List of incompatible provided arguments.
     */
    std::vector<std::string> IncompatibleArguments(size_t nArgumentGroup, bool bFull = true) const;

private:
    uint32_t                                        m_uiParseFlags = 0;     ///< The parse flags supplied to the parse function.
    std::shared_ptr<CArgumentDefBase>               m_ptrDefaultArg;        ///< Default argument (if available).
    std::list<std::shared_ptr<CArgumentDefBase>>    m_lstOptionArgs;        ///< List of configured option arguments (in order of definition).
    std::map<std::string, CArgumentDefBase&, std::greater<std::string>> m_mapSortedOptions;     ///< Map with sorted options.
    std::map<std::string, CArgumentDefBase&, std::greater<std::string>> m_mapSortedSubOptions;  ///< Map with sorted sub-options.
    std::shared_ptr<SGroupDef>                      m_ptrCurrentGroup;      ///< Current group to assign the options to.
    std::list<std::pair<std::reference_wrapper<CArgumentDefBase>, std::string>> m_lstSupplied;  ///< List of supplied arguments.
    size_t                                          m_nFixedWidth = 0;      ///< Fixed with limit (or 0 for dynamic width).
    size_t                                          m_nMaxWidth = 0;        ///< Max with when dynamic (or 0 for no max).
    bool                                            m_bSyntaxPrint = true;  ///< Enable/disable syntax printing.
};

/**
 * @brief Parse exception structure based on std::exception.
 */
struct SArgumentParseException : std::exception
{
    /**
     * @brief Constructor
     * @param[in] rssDescription Reference to the description explaining what caused this exception.
     */
    SArgumentParseException(const std::string& rssDescription) : m_ssDescription(rssDescription)
    {
        Compose();
    }

	/**
	 * @brief Return a textual description.
	 * @return Returns the composed text.
	 */
	virtual const char* what() const noexcept
	{
		return m_ssWhat.c_str();
	}

    /**
     * @brief Add the argument index to the exception.
     * @param[in] nIndex The argument index.
     */
    void AddIndex(size_t nIndex)
    {
        m_nIndex = nIndex; Compose();
    }

    /**
     * @brief Add the argument string to the exception.
     * @param[in] rssArg The argument string.
     */
    void AddArgument(const std::string& rssArg)
    {
        m_ssArgument = rssArg; Compose();
    }

private:
    /**
     * @brief Compose an exception string.
     */
    void Compose()
    {
        std::stringstream sstream;
        if (m_nIndex != static_cast<size_t>(-1))
            sstream << "Argument #" << m_nIndex;
        if (!m_ssArgument.empty())
        {
            if (!sstream.str().empty()) sstream << " ";
            sstream << "'" << m_ssArgument << "'";
        }
        if (!sstream.str().empty())
            sstream << ": ";
        sstream << m_ssDescription;
        m_ssWhat = std::move(sstream.str());
    }

    std::string     m_ssWhat;                               ///< Holding string for the what function.
    std::string     m_ssDescription;                        ///< Exception description.
    std::string     m_ssArgument;                           ///< The argument.
    size_t          m_nIndex = static_cast<size_t>(-1);     ///< The argument index or -1 when no index is available.
};

/**
 * @brief Print a block of text indenting nStartOfTextBlock spaces, starting at nCurrentPosition and aligning at nMaxPosition.
 * @param[in] rstream Reference to the output stream.
 * @param[in] rssText The text to print.
 * @param[in] nIndentPos The indentation of the text.
 * @param[in] nCurrentPos The start position of the text block.
 * @param[in] nMaxPos The last position of the text block.
 */
void PrintBlock(std::ostream& rstream, const std::string& rssText, size_t nIndentPos, size_t nCurrentPos, size_t nMaxPos);

/**
 * @brief Interface to generalize the access to argument implementation using virtual functions.
 * @remarks This interface contains only function prototypes.
 */
struct IArgumentProvide
{
    /**
     * @brief Assign the value from the string (override for assignment values)
     * @param[in] rssValue Reference to the string containing the value to be assigned.
     */
    virtual void ArgumentAssign(const std::string& rssValue) = 0;

    /**
     * @brief Get the markup string for the argument type. Overload in derived class.
     * @return std::string Returns the markup (default empty).
     */
    virtual std::string GetArgumentOptionMarkup() = 0;

    /**
     * @brief Get the option details for the argument type. Overload in derived class.
     * @param[in] nMaxStringLen The maximum length of the string in characters; after the length is reached a newline should be
     * added.
     * @remarks The nMaxStringLen could be ignored if no other alignment is needed; the print function will align the words based on
     * their whitespace.
     * @return std::string Returns the option details (default empty).
     */
    virtual std::string GetArgumentOptionDetails(size_t nMaxStringLen) = 0;

    /**
     * @brief Get the value of the variable
     * @return std::string Returns the value of the variable.
     */
    virtual std::string GetArgumentValueString() = 0;

    /**
     * @brief Is the argument assigned?
     * @return bool Returns 'true' when the argument was assigned; otherwise 'false'.
     */
    virtual bool IsArgumentAssigned() = 0;

    /**
     * @brief Allow multi assignments?
     * @return bool Returns 'true' when multiple assignments are allowed; otherwise 'false'.
     */
    virtual bool AllowMultiArgumentAssign() = 0;
};

/**
* @brief Command line argument type bitmask values
*/
enum class EArgumentFlags : uint32_t
{
    default_argument = 1,           ///< The argument is the default argument on the command line.
    option_argument = 0x10,         ///< The argument is a command line option.
    sub_option_argument = 0x20,     ///< The argument is a command line sub-option.
    bool_option = 0x100,            ///< When set, the argument type variable is a bool (no value assignment).
    flag_option = 0x200,            ///< When set, the argument type is a flag and the variable is a bool.
    case_sensitive = 0x1000,         ///< When set, the argument name is treated as case-sensitive name.
};

/**
 * @brief Command line argument containing the name of the argument, the default value and the reference to the variable receiving
 * the target value.
 */
class CArgumentDefBase
{
protected:
    /**
     * @brief Constructor for singular arguments
     * @tparam TVar The type of the argument variable.
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename TVar, typename... TArgumentGroup>
    CArgumentDefBase(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, TVar& rtVar, TArgumentGroup... nArgumentGroup);

    /**
     * @brief Destructor
     * @attention Since the structure is inherited by many derived classes and it is used with polymorphism to delete the derived
     * class, the destructor has to be virtual.
     */
    virtual ~CArgumentDefBase();

public:
    /**
     * @brief Argument entry (multiple argument names are possible for one argument).
     */
    struct SOptionName
    {
        std::string     ssName;         ///< Argument name.
        uint32_t        uiFlags = 0;    ///< Argument flags with the option/sub-option bits set.
    };

    /**
     * @brief Get access to the command line parser.
     * @return Reference to the parser.
     */
    const CCommandLine& GetCLParser() const { return m_rCLParser; }

    /**
     * @brief Compare the argument name and assign the argument value when the name corresponds.
     * @remarks The comparison occurs either case sensitive or case insensitive (lower case) dependable on the argument
     * configuration.
     * @param[in] rargit Reference to the argument iterator, allowing additional arguments to be retrieved during the assignment.
     * This will only be the case when the command line parser was initiated with the EParseFlags::assignment_next_arg parser flag.
     * @param[in] rssArgument Reference to the string containing the argument.
     * @param[in] rssOptionName The name to compare the argument with (must be one of the registered names of the option or no name
     * if the argument is a default value).
     * @param[in] bPartial When set, the argument definition name could be part of the supplied argument string (in case assignment
     * characters are not used).
     * @return bool Returns true when the comparison was successful and the variable was updated; other returns false.
     */
    bool CompareNameAndAssign(CArgumentIterator& rargit, const std::string& rssArgument, const std::string& rssOptionName,
        bool bPartial) const;

    /**
     * @brief Add the examples to be printed in the help text.
     * @param[in] rssExample Zero terminated string containing the example assignment.
     */
    void AddExample(const std::string& rssExample);

    /**
     * @brief Get the argument option names.
     * @return Reference to a vector with argument option names.
     */
    const std::vector<SOptionName>& GetOptionNames() const { return m_vecOptionNames; }

    /**
     * @brief Get the help text.
     * @return Reference to a string containing the help text.
     */
    const std::string& GetHelpText() const { return m_ssHelpText; }

    /**
     * @brief Get the argument variable access interface.
     * @return Reference to the pointer containing the object exposing the argument variable access interface.
     */
    const std::shared_ptr<IArgumentProvide>& GetArgumentVar() const { return m_ptrArgProvide; }

    /**
     * @brief Get the examples.
     * @return Reference to a list with the examples.
     */
    const std::list<std::string>& GetExamples() const { return m_lstExamples; }

    /**
     * @brief Check for flag to have been set.
     * @param[in] eFlag The flag to check.
     * @return Returns the result.
     */
    bool CheckFlag(EArgumentFlags eFlag) const { return m_uiFlags & static_cast<uint32_t>(eFlag); }

    /**
     * @brief Add additional option name.
     * @param[in] rssOption The name to add for this option.
     */
    void AddOptionName(const std::string& rssOption)
    {
        SOptionName sOption{ rssOption, static_cast<uint32_t>(EArgumentFlags::option_argument) };
        m_vecOptionNames.push_back(sOption);
        m_rCLParser.m_mapSortedOptions.emplace(rssOption, *this);
    }

    /**
    * @brief Add additional sub-option name.
    * @param[in] rssSubOption The name to add for this option.
    */
    void AddSubOptionName(const std::string& rssSubOption)
    {
        SOptionName sSubOption{ rssSubOption, static_cast<uint32_t>(EArgumentFlags::sub_option_argument) };
        m_vecOptionNames.push_back(sSubOption);
        m_rCLParser.m_mapSortedSubOptions.emplace(rssSubOption, *this);
    }

    /**
     * @brief Get the current logical group if existing.
     * @return Shared pointer to the group this argument is assigned to. Or NULL when no group assignment is available.
     */
    std::shared_ptr<SGroupDef> GetGroup() const { return m_ptrGroup; }

    /**
     * @brief Is the option compatible to the argument specific group? Options assigned argument group #0 are always compatible.
     * @param[in] nGroup The argument specific group to show arguments for.
     * @return Returns whether the argument is part of the argument group.
     */
    bool PartOfArgumentGroup(size_t nGroup) const
    {
        return m_setArgumentGroups.find(nGroup) != m_setArgumentGroups.end() ||
            m_setArgumentGroups.find(0) != m_setArgumentGroups.end();
    }

    /**
     * @brief Is the option available on the command line?
     * @details In case an option is provided, but zhe string assignment is empty. This might be valid for several options where
     * the presence of a string following the option can be optional. To detect whether the option is provided, this function can
     * be used.
     * @return Returns whether the option was available at the command line.
     */
    bool OptionAvailableOnCommandLine() const { return m_bAvailableOnCommandLine; }

private:
    CCommandLine&                       m_rCLParser;                        ///< Reference to the command line parser.
    std::shared_ptr<SGroupDef>          m_ptrGroup;                         ///< Group this argument is assigned to.
    std::vector<SOptionName>            m_vecOptionNames;                   ///< The argument option names that are assigned.
    std::string                         m_ssHelpText;                       ///< The argument help text
    std::shared_ptr<IArgumentProvide>   m_ptrArgProvide;                    ///< Pointer to the argument variable to be updated.
    std::list<std::string>              m_lstExamples;                      ///< Examples for the help text
    uint32_t                            m_uiFlags = 0;                      ///< Bitmask (zero or more from EArgumentFlags).
                                                                            ///< Additional args can be added with specific flags.
    std::set<size_t>                    m_setArgumentGroups;                ///< Argument groups to show argument specific help.
    mutable bool                        m_bAvailableOnCommandLine = false;  ///< Will be updated when available on the command line.
                                                                            ///< Needed to detect empty strings following option.
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TEnum The type of the argument variable - when it is an arithmetic type.
 */
template <typename TVar>
class CArgumentDefT<TVar, typename std::enable_if_t<std::is_arithmetic<TVar>::value>> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, TVar& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is an arithmetic vector.
 */
template <typename TVar>
class CArgumentDefT<std::vector<TVar>, typename std::enable_if_t<std::is_arithmetic<TVar>::value>> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::vector<TVar>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is an arithmetic vector.
 */
template <typename TVar>
class CArgumentDefT<sdv::sequence<TVar>, typename std::enable_if_t<std::is_arithmetic<TVar>::value>> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, sdv::sequence<TVar>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is an arithmetic list.
 */
template <typename TVar>
class CArgumentDefT<std::list<TVar>, typename std::enable_if_t<std::is_arithmetic<TVar>::value>> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::list<TVar>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is a path.
 */
template <>
class CArgumentDefT<std::filesystem::path, void> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::filesystem::path& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is a path vector.
 */
template <>
class CArgumentDefT<std::vector<std::filesystem::path>, void> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::vector<std::filesystem::path>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is a path vector.
 */
template <>
class CArgumentDefT<sdv::sequence<std::filesystem::path>, void> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, sdv::sequence<std::filesystem::path>& rtVar,
        TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is a path list.
 */
template <>
class CArgumentDefT<std::list<std::filesystem::path>, void> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::list<std::filesystem::path>& rtVar,
        TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is a string.
 */
template <>
class CArgumentDefT<std::string, void> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::string& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is a string vector.
 */
template <>
class CArgumentDefT<std::vector<std::string>, void> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::vector<std::string>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is a string vector.
 */
template <>
class CArgumentDefT<sdv::sequence<std::string>, void> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, sdv::sequence<std::string>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TVar The type of the argument variable - when it is a string list.
 */
template <>
class CArgumentDefT<std::list<std::string>, void> : public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::list<std::string>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
* @brief Specialization of CArgumentDefT
* @tparam TVar The type of the argument variable - when it is an sdv-string.
*/
template <>
class CArgumentDefT<sdv::u8string, void> : public CArgumentDefBase
{
public:
    /**
    * @brief Constructor for singular arguments
    * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
    * @param[in] rCLParser Reference to the command line parser.
    * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
    * @param[in] rptrGroup Reference to the group this argument is assigned to.
    * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
    * @param[in] uiFlags The argument flags.
    * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
    * operation.
    * @param[in] nArgumentGroup One or more argument group to assign the argument to.
    */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, sdv::u8string& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
* @brief Specialization of CArgumentDefT
* @tparam TVar The type of the argument variable - when it is a string vector.
*/
template <>
class CArgumentDefT<std::vector<sdv::u8string>, void> : public CArgumentDefBase
{
public:
    /**
    * @brief Constructor for singular arguments
    * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
    * @param[in] rCLParser Reference to the command line parser.
    * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
    * @param[in] rptrGroup Reference to the group this argument is assigned to.
    * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
    * @param[in] uiFlags The argument flags.
    * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
    * operation.
    * @param[in] nArgumentGroup One or more argument group to assign the argument to.
    */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::vector<sdv::u8string>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
* @brief Specialization of CArgumentDefT
* @tparam TVar The type of the argument variable - when it is a string sequence.
*/
template <>
class CArgumentDefT<sdv::sequence<sdv::u8string>, void> : public CArgumentDefBase
{
public:
    /**
    * @brief Constructor for singular arguments
    * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
    * @param[in] rCLParser Reference to the command line parser.
    * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
    * @param[in] rptrGroup Reference to the group this argument is assigned to.
    * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
    * @param[in] uiFlags The argument flags.
    * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
    * operation.
    * @param[in] nArgumentGroup One or more argument group to assign the argument to.
    */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, sdv::sequence<sdv::u8string>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
* @brief Specialization of CArgumentDefT
* @tparam TVar The type of the argument variable - when it is a string list.
*/
template <>
class CArgumentDefT<std::list<sdv::u8string>, void> : public CArgumentDefBase
{
public:
    /**
    * @brief Constructor for singular arguments
    * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
    * @param[in] rCLParser Reference to the command line parser.
    * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
    * @param[in] rptrGroup Reference to the group this argument is assigned to.
    * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
    * @param[in] uiFlags The argument flags.
    * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
    * operation.
    * @param[in] nArgumentGroup One or more argument group to assign the argument to.
    */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::list<sdv::u8string>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Value text association structure for enumeration arguments
 * @tparam TEnum The enumerator type to define the associations for.
 */
template <typename TEnum>
struct SEnumArgumentAssoc
{
    TEnum           eValue;             ///< The value of the enumerator
    std::string     ssValueText;        ///< The value in text. Cannot be empty.
    std::string     ssDescription;      ///< The description of the enumerator value. Can be empty.
};

/**
 * @brief Argument implementation for enumerator argument specialization. This class is used for single and multiple enum arguments.
 */
template <typename TEnum>
class CEnumArgumentDefBase
{
public:
    /**
     * @brief Add an enumerator association.
     * @param[in] eEnumValue The value of the enumerator.
     * @param[in] rssValueText The text belonging to the value. Cannot be NULL.
     * @param[in] rssDescription The description of the enumerator value. Can be NULL, which means that it will be the same as the
     * value text.
     */
    void AddAssociation(TEnum eEnumValue, const std::string& rssValueText, const std::string& rssDescription)
    {
        if (rssValueText.empty()) return;   // At least a value text is needed
        SEnumAssociation sEnumAssociation;
        sEnumAssociation.eValue = eEnumValue;
        sEnumAssociation.ssValueText = rssValueText;
        sEnumAssociation.ssDescr = rssDescription.empty() ? rssValueText : rssDescription;
        m_lstEnumAssociations.push_back(sEnumAssociation);
    }

    /**
     * @brief Add an enumerator association.
     * @param[in] rsAssociation Reference to an enumerator association structure.
     */
    void AddAssociation(const SEnumArgumentAssoc<TEnum>& rsAssociation)
    {
        if (rsAssociation.ssValueText.empty()) return;	// At least a value text is needed
        SEnumAssociation sEnumAssociation;
        sEnumAssociation.eValue = rsAssociation.eValue;
        sEnumAssociation.ssValueText = rsAssociation.ssValueText;
        sEnumAssociation.ssDescription =
            rsAssociation.ssDescription.empty() ? rsAssociation.ssValueText : rsAssociation.ssDescription;
        m_lstEnumAssociations.push_back(sEnumAssociation);
    }

    /**
     * @brief Add a array of enumerator associations.
     * @tparam nSize Amount of associations in the array.
     * @param[in] rgsAssociations Array of enumerator associations.
     */
    template <size_t nSize>
    void AddAssociations(const SEnumArgumentAssoc<TEnum>(& rgsAssociations)[nSize])
    {
        for (size_t n = 0; n < nSize; n++)
            AddAssociation(rgsAssociations[n]);
    }

    /**
     * @brief Internal enumerator association structure.
     */
    struct SEnumAssociation
    {
        TEnum           eValue;             ///< The value of the enumerator
        std::string     ssValueText;        ///< The value in text
        std::string     ssDescription;      ///< The description of the enumerator value
    };

    /**
     * @brief Return a reference to the the stored associations.
     * @return std::list<SEnumAssociation>& Reference to the list of associations.
     */
    const std::list<SEnumAssociation>& GetAssociations() const
    {
        return m_lstEnumAssociations;
    }

private:
    std::list<SEnumAssociation>     m_lstEnumAssociations;      ///< List with enumerator to value associations
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TEnum The type of the argument variable - when it is an enum.
 */
template <typename TEnum>
class CArgumentDefT <TEnum, typename std::enable_if_t<std::is_enum<TEnum>::value>> :
    public CEnumArgumentDefBase<TEnum>, public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, TEnum& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TEnum The type of the argument variable - when it is an enum vector.
 */
template <typename TEnum>
class CArgumentDefT<std::vector<TEnum>, typename std::enable_if_t<std::is_enum<TEnum>::value>> :
    public CEnumArgumentDefBase<TEnum>, public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::vector<TEnum>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TEnum The type of the argument variable - when it is an enum sequence.
 */
template <typename TEnum>
class CArgumentDefT<sdv::sequence<TEnum>, typename std::enable_if_t<std::is_enum<TEnum>::value>> :
    public CEnumArgumentDefBase<TEnum>, public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, sdv::sequence<TEnum>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Specialization of CArgumentDefT
 * @tparam TEnum The type of the argument variable - when it is an enum list.
 */
template <typename TEnum>
class CArgumentDefT<std::list<TEnum>, typename std::enable_if_t<std::is_enum<TEnum>::value>> :
    public CEnumArgumentDefBase<TEnum>, public CArgumentDefBase
{
public:
    /**
     * @brief Constructor for singular arguments
     * @tparam TArgumentGroup Types of argument groups for showing argument specific help.
     * @param[in] rCLParser Reference to the command line parser.
     * @param[in] rssArgument Pointer to the zero terminated string containing the name of the argument.
     * @param[in] rptrGroup Reference to the group this argument is assigned to.
     * @param[in] rssHelpText Pointer to the zero terminated string containing the help text of the argument.
     * @param[in] uiFlags The argument flags.
     * @param[in, out] rtVar Reference to the argument variable to be changed. This variable will be updated during the parse
     * operation.
     * @param[in] nArgumentGroup One or more argument group to assign the argument to.
     */
    template <typename... TArgumentGroup>
    CArgumentDefT(CCommandLine& rCLParser, const std::string& rssArgument, const std::shared_ptr<SGroupDef>& rptrGroup,
        const std::string& rssHelpText, uint32_t uiFlags, std::list<TEnum>& rtVar, TArgumentGroup... nArgumentGroup) :
        CArgumentDefBase(rCLParser, rssArgument, rptrGroup, rssHelpText, uiFlags, rtVar, nArgumentGroup...)
    {}
};

/**
 * @brief Argument container containing default function implementations.
 * @tparam TVar The value this argument is used with.
 */
template <typename TVar>
class CArgValueImpl
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line definition.
     */
    CArgValueImpl(CArgumentDefT<TVar>& rArgumentDef) : m_rArgumentDef(rArgumentDef)
    {
    }

    /**
     * @brief Parse the value from the string (override for assignment values)
     * @param[in] rtArgument Reference to the argument.
     * @param[in] rssValue Reference to the string containing the value to be assigned.
     */
    void Parse([[maybe_unused]] TVar& rtArgument, [[maybe_unused]] const std::string& rssValue) {}

    /**
     * @brief Get the markup string for the argument type. Overload in derived class.
     * @return std::string Returns the markup (default empty).
     */
    std::string GetArgumentOptionMarkup() { return std::string(); }

    /**
     * @brief Get the option details for the argument type. Overload in derived class.
     * @remarks The nMaxStringLen could be ignored if no other alignment is needed; the print function will align the words based on
     * their whitespace.
     * @return std::string Returns the option details (default empty).
     */
    std::string GetOptionDetails(size_t /*nMaxStringLen*/) { return std::string(); }

    /**
     * @brief Get the value of the variable
     * @return std::string Returns the value of the variable.
     */
    std::string GetArgumentValueString(const TVar& /*rtArgument*/) { return std::string(); }

    /**
     * @brief Is this a multi argument assignments?
     * @return bool Returns 'true' when multiple assignments are allowed; otherwise 'false'.
     * @remarks Default implementation is to support argument assignments only once.
     */
    bool MultiArgument() { return false; }

protected:
    CArgumentDefT<TVar>&    m_rArgumentDef;     ///< Accessible command line argument definition for derived classes.
};

/**
 * @brief Assignment argument implementation.
 * @tparam TBase The base class where this argument is derived of. This base class should be derived of CArgValueImpl<> and can
 * implement several overloaded functions.
 * @tparam TVar The value this argument is used with.
 */
template <class TBase, typename TVarClass, typename TVar = TVarClass>
class CValueAssignment : public TBase
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CValueAssignment(CArgumentDefT<TVarClass>& rArgumentDef) :
        TBase(rArgumentDef),
        m_bNoAssignChar(rArgumentDef.GetOptionNames().empty() ||
            rArgumentDef.GetCLParser().CheckParseFlag(CCommandLine::EParseFlags::no_assignment_character) ||
            rArgumentDef.GetCLParser().CheckParseFlag(CCommandLine::EParseFlags::assignment_next_arg))
    {}

    /**
     * @brief Parse the value from the string (override for assignment values)
     * @details This function uses the command line parse flags to determine whether the assignment is using an assignment
     * character and if so checks and removes the character from the value. This function passes through the value assignment to
     * the derived class TBase.
     * @param[in] rtArgument Reference to the argument.
     * @param[in] rssValue Reference to the string value to be assigned.
     */
    void Parse(TVarClass& rtArgument, const std::string& rssValue)
    {
        // Value existing?
        if (!m_bNoAssignChar && !rssValue.size())
            throw SArgumentParseException("Incorrect value (assignment expected)!");

        // Assignment expected?
        if (!m_bNoAssignChar && rssValue[0] != '=' && rssValue[0] != ':')
            throw SArgumentParseException("Incorrect value (assignment expected)!");

        // Forward the request to the base
        TBase::Parse(rtArgument, rssValue.substr(m_bNoAssignChar ? 0 : 1));
    }

    /**
     * @brief Get the markup string for the IP address.
     * @return std::string Returns the markup string.
     */
    std::string GetArgumentOptionMarkup()
    {
        // Request the markup of the argument(s)
        // Option markup is composed of ':' with the markup of the argument type(s)
        return (m_bNoAssignChar ? "" : ":") + TBase::GetArgumentOptionMarkup();
    }

private:
    bool m_bNoAssignChar = false;        ///< When set, no assigning character (':' or '=') is used.
};

/**
 * @brief Container argument implementation (used with std::vector, std::list and sdv::sequence).
 * @tparam TContainer The container type to use with this argument being among others std::vector, std::list and sdv::sequence.
 * @tparam TBase The base class where this argument is derived of. This base class should be derived of CArgValueImpl<> and can
 * implement several overloaded functions.
 * @tparam TVar The value this argument is used with.
 * @remarks This class can only be used with an assignment argument CValueAssignment<>.
 */
template <class TContainer, class TBase, typename TVar>
class CContainerArgValue : public TBase
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CContainerArgValue(CArgumentDefT<TContainer>& rArgumentDef) : TBase(rArgumentDef)	{}

    /**
     * @brief Parse the value from the string (override for assignment values)
     * @param[in] rvectArgument Reference to the argument container.
     * @param[in] rssValue Reference to the string value to be assigned.
     */
    void Parse(TContainer& rvectArgument, const std::string& rssValue)
    {
        // Parse through the value ('\\' and '"' are escape characters)
        std::size_t nPos = 0;
        std::string ssValue;
        enum class EState { value_or_quote, value_or_comma, quote_string, comma_or_end } eState = EState::value_or_quote;
        bool bEscape = false;
        size_t nDepth = 0;

        do
        {
            bool bSkip = false;
            bool bProcess = false;

            // Use the '\0' to identify the end of the string
            char ch = nPos >= rssValue.size() ? '\0' : rssValue[nPos];

            // interpret the character
            switch (ch)
            {
            case '(':
            case '{':
                if (!bEscape && eState != EState::quote_string)
                    nDepth++;
                break;
            case ')':
            case '}':
                if (nDepth && !bEscape && eState != EState::quote_string)
                    nDepth--;
                break;
            case '\\':
                if (bEscape)	// Keep this character
                {
                    bEscape = false;
                    break;
                }
                // Escape character
                bEscape = true;
                bSkip = true;
                break;
            case '"':
                if (bEscape)	// Keep this character
                {
                    bEscape = false;
                    break;
                }
                if (eState == EState::value_or_comma || eState == EState::comma_or_end) // Invalid state, quoted string cannot start in the middle
                    throw SArgumentParseException("Incorrect value (quoted string cannot start in the middle of another string)!");
                if (eState == EState::value_or_quote) // Start of quoted string
                {
                    eState = EState::quote_string;
                    bSkip = true;
                }
                else if (eState == EState::quote_string) // End of quoted string
                {
                    eState = EState::comma_or_end;
                    bSkip = true;
                }
                break;
            case ',':
            case ';':
                if (bEscape) // Keep this character
                {
                    bEscape = false;
                    break;
                }
                if (eState == EState::quote_string) // Comma is allowed in quoted strings
                    break;
                if (nDepth) // Comma is allowed when within brackets
                    break;
                if (eState == EState::value_or_quote)
                    throw SArgumentParseException("Incorrect value (string part cannot start with a comma)!");
                if (eState == EState::value_or_comma || eState == EState::comma_or_end) // Invalid state, quoted string cannot start in the middle
                {
                    bSkip = true;
                    bProcess = true;
                    eState = EState::value_or_quote;
                }
                break;
            case '\0':
                bProcess = true;
                bSkip = true;
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\a':
            default:
                if (bEscape)
                {
                    bEscape = false;	// When the value previously was escaped... this was not wanted
                    nPos--;
                    ch = '\\';
                }
                if (eState != EState::quote_string)
                    eState = EState::value_or_comma;
                break;
            }


            // Add the character to the current string
            if (!bSkip)
                ssValue += ch;

            // Process the current value
            if (bProcess)
            {
                // To allow proper initialization, the value is first added
                // to the container and then assigned.
                TVar tVar{};
                rvectArgument.push_back(tVar);
                TBase::Parse(rvectArgument.back(), ssValue);
                ssValue.clear();
            }
        } while (++nPos <= rssValue.size());
    }

    /**
     * @brief Get the markup string for the IP address.
     * @return std::string Returns the markup string.
     */
    std::string GetArgumentOptionMarkup()
    {
        // Request the markup of the argument(s)
        // Option markup is composed of ':' with the markup of the argument type(s)
        return TBase::GetArgumentOptionMarkup() + "[," + TBase::GetArgumentOptionMarkup() + "]";
    }

    /**
     * @brief Get the value of the variable
     * @param[in] rvectArgument Reference to the argument container.
     * @return std::string Returns the value of the variable.
     */
    std::string GetArgumentValueString(const TContainer& rvectArgument)
    {
        std::string ssValue;
        for (const TVar& tVar : rvectArgument)
        {
            if (ssValue.size())
                ssValue += ",";
            ssValue += TBase::GetArgumentValueString(tVar);
        }
        return ssValue;
    }

    /**
     * @brief Allow multiple assignments?
     * @return bool Returns 'true' when multiple assignments are allowed; otherwise 'false'.
     * @remarks Using vectors, multiple argument assignments are supported.
     */
    bool MultiArgument() { return true; }
};

/**
 * @brief Arguments of std::string type derived from CArgValueImpl<>.
 */
template <typename TVarBase = std::string, typename TVar = std::string>
class CStdStringValue : public CArgValueImpl<TVarBase>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CStdStringValue(CArgumentDefT<TVarBase>& rArgumentDef) : CArgValueImpl<TVarBase>(rArgumentDef)	{}

    /**
     * @brief Parse the value from the string(override for assignment values)
     * @param[in] rssArgument Reference to the string argument.
     * @param[in] rssValue Reference to the string value to be assigned.
     */
    void Parse(TVar& rssArgument, const std::string& rssValue) const
    {
        // Assign the string
        // Does the string have a quote at the beginning and at the end?
        if (rssValue.size() >= 2 && rssValue.front() == '\"' && rssValue.back() == '\"')
            rssArgument = rssValue.substr(1, rssValue.size() - 2);
        else
            rssArgument = rssValue;
    }

    /**
     * @brief Get the markup string for the string.
     * @return std::string Returns the markup string.
     */
    std::string GetArgumentOptionMarkup() const
    {
        return "<string>";
    }

    /**
     * @brief Get the value of the variable
     * @param[in] rssArgument Reference to the argument.
     * @return std::string Returns the value of the variable.
     */
    std::string GetArgumentValueString(const TVar& rssArgument) const
    {
        return rssArgument;
    }
};

/**
* @brief Arguments of std::string type derived from CArgValueImpl<>.
*/
template <typename TVarBase = sdv::u8string, typename TVar = sdv::u8string>
class CSdvStringValue : public CArgValueImpl<TVarBase>
{
public:
    /**
    * @brief Constructor
    * @param[in] rArgumentDef Reference to the command line argument definition.
    */
    CSdvStringValue(CArgumentDefT<TVarBase>& rArgumentDef) : CArgValueImpl<TVarBase>(rArgumentDef)	{}

    /**
    * @brief Parse the value from the string(override for assignment values)
    * @param[in] rssArgument Reference to the string argument.
    * @param[in] rssValue Reference to the string value to be assigned.
    */
    void Parse(TVar& rssArgument, const std::string& rssValue) const
    {
        // Assign the string
        // Does the string have a quote �t the beginning and at the end?
        if (rssValue.size() >= 2 && rssValue.front() == '\"' && rssValue.back() == '\"')
            rssArgument = rssValue.substr(1, rssValue.size() - 2);
        else
            rssArgument = rssValue;
    }

    /**
    * @brief Get the markup string for the string.
    * @return std::string Returns the markup string.
    */
    std::string GetArgumentOptionMarkup() const
    {
        return "<string>";
    }

    /**
    * @brief Get the value of the variable
    * @param[in] rssArgument Reference to the argument.
    * @return std::string Returns the value of the variable.
    */
    std::string GetArgumentValueString(const TVar& rssArgument) const
    {
        return rssArgument;
    }
};

/**
 * @brief Arguments of std::filesystem::path type derived from CArgValueImpl<>.
 */
template <typename TVarBase = std::filesystem::path>
class CPathArgValue : public CArgValueImpl<TVarBase>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CPathArgValue(CArgumentDefT<TVarBase>& rArgumentDef) : CArgValueImpl<TVarBase>(rArgumentDef){}

    /**
     * @brief Parse the value from the string(override for assignment values)
     * @param[in] rssArgument Reference to the string argument.
     * @param[in] rssValue Reference to the string value to be assigned.
     */
    void Parse(std::filesystem::path& rssArgument, const std::string& rssValue) const
    {
        // Assign the string
        // Does the string have a quote �t the beginning and at the end?
        if (rssValue.size() >= 2 && rssValue.front() == '\"' && rssValue.back() == '\"')
            rssArgument = rssValue.substr(1, rssValue.size() - 2);
        else
            rssArgument = rssValue;
    }

    /**
     * @brief Get the markup string for the string.
     * @return std::string Returns the markup string.
     */
    std::string GetArgumentOptionMarkup() const
    {
        return "<path>";
    }

    /**
     * @brief Get the value of the variable
     * @param[in] rssArgument Reference to the argument.
     * @return std::string Returns the value of the variable.
     */
    std::string GetArgumentValueString(const ::std::filesystem::path& rssArgument) const
    {
        return rssArgument.u8string();
    }
};

/**
 * @brief Arguments of numeric type derived from CArgValueImpl<>.
 */
template <typename TVarBase, typename TVar = TVarBase>
class CNumericArgValue : public CArgValueImpl<TVarBase>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CNumericArgValue(CArgumentDefT<TVarBase>& rArgumentDef) : CArgValueImpl<TVarBase>(rArgumentDef)	{}

    /**
     * @brief Parse the value from the string(override for assignment values)
     * @param[in] rtArgument Reference to the numeric argument.
     * @param[in] rssValue Reference to the string value to be assigned.
     */
    void Parse(TVar& rtArgument, const std::string& rssValue) const
    {
        // Assign the number
        std::istringstream sstream;
        sstream.str(rssValue);

        // Skip whitespace
        while (!sstream.str().empty() && std::isspace(sstream.str()[0]))
            sstream.str().erase(0, 1);

        // To support int8_t and uint8_t as well (which are implemented using the char data type), stream into a 64-bit
        // integer and assign separately. Streaming into a character will not interpret the value as number, but as a single
        // character.
        if constexpr (std::is_integral_v<TVar>)
        {
            if constexpr (std::is_signed_v<TVar>)
            {
                int64_t iVal = 0;
                if (!sstream.str().empty() && !std::isdigit(sstream.str()[0]) && sstream.str()[0] != '-')
                    throw SArgumentParseException("Value is not a number!");
                sstream >> iVal;
                if (iVal < std::numeric_limits<TVar>().min())
                    throw SArgumentParseException("Value too small!");
                if (iVal > std::numeric_limits<TVar>().max())
                    throw SArgumentParseException("Value too large!");
                rtArgument = static_cast<TVar>(iVal);
            }
            else
            {
                uint64_t uiVal = 0;
                if (!sstream.str().empty() && !std::isdigit(sstream.str()[0]))
                    throw SArgumentParseException("Value is not a number!");
                sstream >> uiVal;
                if (uiVal > std::numeric_limits<TVar>().max())
                    throw SArgumentParseException("Value too large!");
                rtArgument = static_cast<TVar>(uiVal);
            }
        }
        else
        {
            if (!sstream.str().empty() && !std::isdigit(sstream.str()[0]) && sstream.str()[0] != '-' && sstream.str()[0] != '.')
                throw SArgumentParseException("Value is not a number!");
            sstream >> rtArgument;
        }
    }

    /**
     * @brief Get the markup string for the integer.
     * @return std::string Returns the markup string.
     */
    std::string GetArgumentOptionMarkup() const
    {
        return "<number>";
    }

    /**
     * @brief Get the value of the variable
     * @param[in] rtArgument Reference to the argument.
     * @return std::string Returns the value of the variable.
     */
    std::string GetArgumentValueString(const TVar& rtArgument) const
    {
        std::ostringstream sstream;
        sstream << rtArgument;
        return sstream.str();
    }
};

/**
 * @brief Arguments of numeric type derived from CArgValueImpl<>.
 */
template <typename TVarBase, typename TVar = TVarBase>
class CEnumArgValue : public CArgValueImpl<TVarBase>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CEnumArgValue(CArgumentDefT<TVarBase>& rArgumentDef) : CArgValueImpl<TVarBase>(rArgumentDef)	{}

    /**
     * @brief Parse the value from the string(override for assignment values)
     * @param[in] rtArgument Reference to the numeric argument.
     * @param[in] rssValue Reference to the string value to be assigned.
     */
    void Parse(TVar& rtArgument, const std::string& rssValue) const
    {
        // Find the value
        bool bFound = false;
        for(const typename CArgumentDefT<TVarBase>::SEnumAssociation& rsEnumAssociation :
            CArgValueImpl<TVarBase>::m_rArgumentDef.GetAssociations())
        {
            if (rssValue == rsEnumAssociation.ssValueText)
            {
                rtArgument = rsEnumAssociation.eValue;
                bFound = true;
                break;
            }
        }

        // Error?
        if (!bFound)
            throw SArgumentParseException("Incorrect value!");
    }

    /**
     * @brief Get the markup string for the integer.
     * @return std::string Returns the markup string.
     */
    std::string GetArgumentOptionMarkup() const
    {
        return "<...>";
    }

    /**
     * @brief Get the option details for the argument type. Overload in derived class.
     * @param[in] nMaxStringLen The maximum length of the string in characters; after the length is reached a newline should be
     * added.
     * @remarks The nMaxStringLen could be ignored if no other alignment is needed; the print function will align the words based on
     * their whitespace.
     * @return std::string Returns the option details (default empty).
     */
    std::string GetOptionDetails(size_t nMaxStringLen)
    {
        size_t nMaxNameLen = 0;
        for (const typename CArgumentDefT<TVarBase>::SEnumAssociation& rsEnumAssociation :
            CArgValueImpl<TVarBase>::m_rArgumentDef.GetAssociations())
        {
            nMaxNameLen = std::max(rsEnumAssociation.ssValueText.size(), nMaxNameLen);
        }
        std::string ssResult;
        bool bFirst = true;
        for (const typename CArgumentDefT<TVarBase>::SEnumAssociation& rsEnumAssociation :
            CArgValueImpl<TVarBase>::m_rArgumentDef.GetAssociations())
        {
            // Insert a newline starting with the second enum value
            if (!bFirst) ssResult += "\n";
            bFirst = false;

            // A space
            ssResult += " ";

            // The enum value
            ssResult += rsEnumAssociation.ssValueText;

            // Extra space
            ssResult.insert(ssResult.end(), nMaxNameLen - rsEnumAssociation.ssValueText.size(), ' ');

            // Separator
            ssResult += " - ";

            // Start position of text
            size_t nStartTextPos = nMaxNameLen + 4;	// Include ' ' and ' - '

            // Create a text block
            std::stringstream sstream;
            PrintBlock(sstream, rsEnumAssociation.ssDescription, nStartTextPos, nStartTextPos, nMaxStringLen);

            // Text
            ssResult += sstream.str();
        }
        return ssResult;
    }

    /**
     * @brief Get the value of the variable
     * @param[in] rtArgument Reference to the argument.
     * @return std::string Returns the value of the variable.
     */
    std::string GetArgumentValueString(const TVar& rtArgument) const
    {
        // Find the value
        for (const typename CArgumentDefT<TVarBase>::SEnumAssociation& rsEnumAssociation :
            CArgValueImpl<TVarBase>::m_rArgumentDef.GetAssociations())
        {
            if (rtArgument == rsEnumAssociation.eValue)
                return rsEnumAssociation.ssValueText;
        }
        return std::string();
    }
};

/**
 * @brief CArgumentProvideImpl<> class prototype.
 * @tparam TArgument The argument type this class is wrapping.
 * @tparam TEnable The argument type to allow type dependent specialization.
 */
template <typename TArgument, typename TEnable = void>
class CArgumentProvideImpl;

/**
* @brief Specialization of CArgumentProvideImpl<> with the argument definition for
* the std::string argument.
*/
template <> class CArgumentProvideImpl<std::string, void> :
    public CValueAssignment<CStdStringValue<>, std::string>
{
public:
    /**
    * @brief Constructor
    * @param[in] rArgumentDef Reference to the command line argument definition.
    */
    CArgumentProvideImpl(CArgumentDefT<std::string>& rArgumentDef) :
        CValueAssignment<CStdStringValue<>, std::string>(rArgumentDef)
    {}
};


/**
* @brief Specialization of CArgumentProvideImpl<> with the argument definition for
* the std::string argument.
*/
template <> class CArgumentProvideImpl<sdv::u8string, void> :
    public CValueAssignment<CSdvStringValue<>, sdv::u8string>
{
public:
    /**
    * @brief Constructor
    * @param[in] rArgumentDef Reference to the command line argument definition.
    */
    CArgumentProvideImpl(CArgumentDefT<sdv::u8string>& rArgumentDef) :
        CValueAssignment<CSdvStringValue<>, sdv::u8string>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for the std::filesystem::path argument.
 */
template <> class CArgumentProvideImpl<std::filesystem::path, void> :
    public CValueAssignment<CPathArgValue<>, std::filesystem::path>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<std::filesystem::path>& rArgumentDef) :
        CValueAssignment<CPathArgValue<>, std::filesystem::path>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition of numeric types (short, long, int, float, double,
 * signed, unsigned).
 * @tparam TArgument The argument type this class is wrapping.
 */
template <typename TArgument>
class CArgumentProvideImpl<TArgument, typename std::enable_if_t<std::is_arithmetic<TArgument>::value>> :
    public CValueAssignment<CNumericArgValue<TArgument>, TArgument>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<TArgument>& rArgumentDef) :
        CValueAssignment<CNumericArgValue<TArgument>, TArgument>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for a std::vector<> of numeric types (short, long,
 * int, float, double, signed, unsigned).
 * @tparam TArgument The argument type this class is wrapping.
 */
template <typename TArgument>
class CArgumentProvideImpl<std::vector<TArgument>, typename std::enable_if_t<std::is_arithmetic<TArgument>::value>> :
    public CValueAssignment<
        CContainerArgValue<std::vector<TArgument>, CNumericArgValue<std::vector<TArgument>, TArgument>, TArgument>,
        std::vector<TArgument>>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<std::vector<TArgument>>& rArgumentDef) :
        CValueAssignment<
            CContainerArgValue<std::vector<TArgument>, CNumericArgValue<std::vector<TArgument>, TArgument>, TArgument>,
            std::vector<TArgument>>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for a sdv::sequence<> of numeric types (short, long,
 * int, float, double, signed, unsigned).
 * @tparam TArgument The argument type this class is wrapping.
 */
template <typename TArgument>
class CArgumentProvideImpl<sdv::sequence<TArgument>, typename std::enable_if_t<std::is_arithmetic<TArgument>::value>> :
    public CValueAssignment<
        CContainerArgValue<sdv::sequence<TArgument>, CNumericArgValue<sdv::sequence<TArgument>, TArgument>, TArgument>,
    sdv::sequence<TArgument>>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<sdv::sequence<TArgument>>& rArgumentDef) :
        CValueAssignment<
            CContainerArgValue<sdv::sequence<TArgument>, CNumericArgValue<sdv::sequence<TArgument>, TArgument>, TArgument>,
        sdv::sequence<TArgument>>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for a std::ist<> of numeric types (short, long, int,
 * float, double, signed, unsigned).
 * @tparam TArgument The argument type this class is wrapping.
 */
template <typename TArgument>
class CArgumentProvideImpl<std::list<TArgument>, typename std::enable_if_t<std::is_arithmetic<TArgument>::value>> :
    public CValueAssignment<CContainerArgValue<std::list<TArgument>, CNumericArgValue<std::list<TArgument>, TArgument>, TArgument>,
        std::list<TArgument>>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<std::list<TArgument>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<std::list<TArgument>, CNumericArgValue<std::list<TArgument>, TArgument>, TArgument>,
            std::list<TArgument>>(rArgumentDef)
    {}
};

/**
* @brief Specialization of CArgumentProvideImpl<> with the argument definition for a std::vector<> of the std::string argument.
*/
template <> class CArgumentProvideImpl<std::vector<std::string>, void> :
    public CValueAssignment<CContainerArgValue<std::vector<std::string>, CStdStringValue<std::vector<std::string>>, std::string>,
    std::vector<std::string>>
{
public:
    /**
    * @brief Constructor
    * @param[in] rArgumentDef Reference to the command line argument definition.
    */
    CArgumentProvideImpl(CArgumentDefT<std::vector<std::string>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<std::vector<std::string>, CStdStringValue<std::vector<std::string>>, std::string>,
        std::vector<std::string>>(rArgumentDef)
    {}
};

/**
* @brief Specialization of CArgumentProvideImpl<> with the argument definition for a sdv::sequence<> of the std::string argument.
*/
template <> class CArgumentProvideImpl<sdv::sequence<std::string>, void> :
    public CValueAssignment<CContainerArgValue<sdv::sequence<std::string>, CStdStringValue<sdv::sequence<std::string>>, std::string>,
    sdv::sequence<std::string>>
{
public:
    /**
    * @brief Constructor
    * @param[in] rArgumentDef Reference to the command line argument definition.
    */
    CArgumentProvideImpl(CArgumentDefT<sdv::sequence<std::string>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<sdv::sequence<std::string>, CStdStringValue<sdv::sequence<std::string>>, std::string>,
        sdv::sequence<std::string>>(rArgumentDef)
    {}
};

/**
* @brief Specialization of CArgumentProvideImpl<> with the argument definition for a std::list<> of the std::string argument.
*/
template <> class CArgumentProvideImpl<std::list<std::string>, void> :
    public CValueAssignment<CContainerArgValue<std::list<std::string>, CStdStringValue<std::list<std::string>>, std::string>,
    std::list<std::string>>
{
public:
    /**
    * @brief Constructor
    * @param[in] rArgumentDef Reference to the command line argument definition.
    */
    CArgumentProvideImpl(CArgumentDefT<std::list<std::string>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<std::list<std::string>, CStdStringValue<std::list<std::string>>, std::string>,
        std::list<std::string>>(rArgumentDef)
    {}
};

/**
* @brief Specialization of CArgumentProvideImpl<> with the argument definition for a std::vector<> of the sdv::u8string argument.
*/
template <> class CArgumentProvideImpl<std::vector<sdv::u8string>, void> :
    public CValueAssignment<CContainerArgValue<std::vector<sdv::u8string>, CSdvStringValue<std::vector<sdv::u8string>>, sdv::u8string>,
    std::vector<sdv::u8string>>
{
public:
    /**
    * @brief Constructor
    * @param[in] rArgumentDef Reference to the command line argument definition.
    */
    CArgumentProvideImpl(CArgumentDefT<std::vector<sdv::u8string>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<std::vector<sdv::u8string>, CSdvStringValue<std::vector<sdv::u8string>>, sdv::u8string>,
        std::vector<sdv::u8string>>(rArgumentDef)
    {}
};

/**
* @brief Specialization of CArgumentProvideImpl<> with the argument definition for a sdv::sequence<> of the sdv::u8string argument.
*/
template <> class CArgumentProvideImpl<sdv::sequence<sdv::u8string>, void> :
    public CValueAssignment<CContainerArgValue<sdv::sequence<sdv::u8string>, CSdvStringValue<sdv::sequence<sdv::u8string>>, sdv::u8string>,
    sdv::sequence<sdv::u8string>>
{
public:
    /**
    * @brief Constructor
    * @param[in] rArgumentDef Reference to the command line argument definition.
    */
    CArgumentProvideImpl(CArgumentDefT<sdv::sequence<sdv::u8string>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<sdv::sequence<sdv::u8string>, CSdvStringValue<sdv::sequence<sdv::u8string>>, sdv::u8string>,
        sdv::sequence<sdv::u8string>>(rArgumentDef)
    {}
};

/**
* @brief Specialization of CArgumentProvideImpl<> with the argument definition for a std::list<> of the sdv::u8string argument.
*/
template <> class CArgumentProvideImpl<std::list<sdv::u8string>, void> :
    public CValueAssignment<CContainerArgValue<std::list<sdv::u8string>, CSdvStringValue<std::list<sdv::u8string>>, sdv::u8string>,
    std::list<sdv::u8string>>
{
public:
    /**
    * @brief Constructor
    * @param[in] rArgumentDef Reference to the command line argument definition.
    */
    CArgumentProvideImpl(CArgumentDefT<std::list<sdv::u8string>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<std::list<sdv::u8string>, CSdvStringValue<std::list<sdv::u8string>>, sdv::u8string>,
        std::list<sdv::u8string>>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for a std::vector<> of the
 * std::filesystem::path argument.
 */
template <> class CArgumentProvideImpl<std::vector<std::filesystem::path>, void> :
    public CValueAssignment<CContainerArgValue<std::vector<std::filesystem::path>,
        CPathArgValue<std::vector<std::filesystem::path>>, std::filesystem::path>, std::vector<std::filesystem::path>>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<std::vector<std::filesystem::path>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<std::vector<std::filesystem::path>,
            CPathArgValue<std::vector<std::filesystem::path>>, std::filesystem::path>,
            std::vector<std::filesystem::path>>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for a sdv::sequence<> of the
 * std::filesystem::path argument.
 */
template <> class CArgumentProvideImpl<sdv::sequence<std::filesystem::path>, void> :
    public CValueAssignment<CContainerArgValue<sdv::sequence<std::filesystem::path>,
        CPathArgValue<sdv::sequence<std::filesystem::path>>, std::filesystem::path>, sdv::sequence<std::filesystem::path>>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<sdv::sequence<std::filesystem::path>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<sdv::sequence<std::filesystem::path>,
            CPathArgValue<sdv::sequence<std::filesystem::path>>, std::filesystem::path>,
        sdv::sequence<std::filesystem::path>>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for a std::list<> of the std::filesystem::path
 * argument.
 */
template <> class CArgumentProvideImpl<std::list<std::filesystem::path>, void> :
    public CValueAssignment<CContainerArgValue<std::list<std::filesystem::path>,
        CPathArgValue<std::list<std::filesystem::path>>, std::filesystem::path>, std::list<std::filesystem::path>>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<std::list<std::filesystem::path>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<std::list<std::filesystem::path>, CPathArgValue<std::list<std::filesystem::path>>,
            std::filesystem::path>, std::list<std::filesystem::path>>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for the TEnum argument.
 * @tparam TEnum The enum type to use for this specicialization.
 */
template <typename TEnum>
class CArgumentProvideImpl<TEnum, typename std::enable_if_t<std::is_enum<TEnum>::value>> :
    public CValueAssignment<CEnumArgValue<TEnum>, TEnum>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<TEnum>& rArgumentDef) :
        CValueAssignment<CEnumArgValue<TEnum>, TEnum>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for a std::vector<> of enumeration types.
 * @tparam TEnum The argument type this class is wrapping.
 */
template <typename TEnum>
class CArgumentProvideImpl<std::vector<TEnum>, typename std::enable_if_t<std::is_enum<TEnum>::value>> :
    public CValueAssignment<CContainerArgValue<std::vector<TEnum>, CEnumArgValue<std::vector<TEnum>, TEnum>, TEnum>,
        std::vector<TEnum>>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<std::vector<TEnum>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<std::vector<TEnum>, CEnumArgValue<std::vector<TEnum>, TEnum>, TEnum>,
            std::vector<TEnum>>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for a sdv::sequence<> of enumeration types.
 * @tparam TEnum The argument type this class is wrapping.
 */
template <typename TEnum>
class CArgumentProvideImpl<sdv::sequence<TEnum>, typename std::enable_if_t<std::is_enum<TEnum>::value>> :
    public CValueAssignment<CContainerArgValue<sdv::sequence<TEnum>, CEnumArgValue<sdv::sequence<TEnum>, TEnum>, TEnum>,
    sdv::sequence<TEnum>>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<sdv::sequence<TEnum>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<sdv::sequence<TEnum>, CEnumArgValue<sdv::sequence<TEnum>, TEnum>, TEnum>,
        sdv::sequence<TEnum>>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of CArgumentProvideImpl<> with the argument definition for a std::list<> of enumeration types.
 * @tparam TEnum The argument type this class is wrapping.
 */
template <typename TEnum>
class CArgumentProvideImpl<std::list<TEnum>, typename std::enable_if_t<std::is_enum<TEnum>::value>> :
    public CValueAssignment<CContainerArgValue<std::list<TEnum>, CEnumArgValue<std::list<TEnum>, TEnum>, TEnum>, std::list<TEnum>>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<std::list<TEnum>>& rArgumentDef) :
        CValueAssignment<CContainerArgValue<std::list<TEnum>, CEnumArgValue<std::list<TEnum>, TEnum>, TEnum>,
            std::list<TEnum>>(rArgumentDef)
    {}
};

/**
 * @brief Specialization of the CArgumentProvideImpl class for the bool type.
 * @remarks This class cannot be used in combination with the assignment CValueAssignment<> argument.
 */
template <> class CArgumentProvideImpl<bool, void> : public CArgValueImpl<bool>
{
public:
    /**
     * @brief Constructor
     * @param[in] rArgumentDef Reference to the command line argument definition.
     */
    CArgumentProvideImpl(CArgumentDefT<bool>& rArgumentDef) : CArgValueImpl<bool>(rArgumentDef) {}

    /**
     * @brief Parse the value from the string (override for assignment values)
     * @param[out] rbArgument Reference to the argument.
     * @param[in] rssValue Reference to the value string to parse.
     */
    void Parse(bool& rbArgument, const std::string& rssValue) const
    {
        // Differentiate between a flag and a boolean.
        if (m_rArgumentDef.CheckFlag(EArgumentFlags::flag_option))
        {
            // Interpret the value (either + or -).
            if (rssValue == "+")
                rbArgument = true;
            else if (rssValue == "-")
                rbArgument = false;
            else
                throw SArgumentParseException("Incorrect value: + or - expected!");
        }
        else
        {
            // The value can be ignored...
            rbArgument = true;
        }
    }

    /**
    * @brief Get the markup string for the flag.
    * @return std::string Returns the markup string.
    */
    std::string GetArgumentOptionMarkup() const
    {
        if (m_rArgumentDef.CheckFlag(EArgumentFlags::flag_option))
            return "<+|->";
        else
            return std::string();
    }

    /**
     * @brief Get the value of the variable
     * @param[in] rbArgument Reference to the argument.
     * @return std::string Returns the value of the variable.
     */
    std::string GetArgumentValueString(const bool& rbArgument) const
    {
        if (m_rArgumentDef.CheckFlag(EArgumentFlags::flag_option))
            return rbArgument ? "signalled (+)" : "not signalled (-)";
        else
            return rbArgument ? "true" : "false";
    }
};

/**
 * @brief Default template for the templatized specific argument. This class implements the IArgumentProvide prototype functions and
 * forwards the calls to the specialized argument implementations (which is the base of this class through the
 * CArgumentProvideImpl<> class).
 * @tparam TArgument Template type used with a specialized implementation of CArgumentProvideImpl.
 */
template <typename TArgument>
class CArgumentProvide : public CArgumentProvideImpl<TArgument>, public IArgumentProvide
{
public:
    /**
     * @brief Constructor assigning the reference to the argument to the member function.
     * @param[in] rArgumentDef Referetnce to the argument definition.
     * @param[in] rtArgument Reference to the argument variable.
     */
    CArgumentProvide(CArgumentDefT<TArgument>& rArgumentDef, TArgument& rtArgument) :
        CArgumentProvideImpl<TArgument>(rArgumentDef), m_rtArgument(rtArgument), m_bArgumentAssigned(false)
    {};

    /**
     * @brief Assign the value from the string (override for assignment values)
     * @param[in] rssValue Reference to the string containing the value to be assigned.
     */
    virtual void ArgumentAssign(const std::string& rssValue) override
    {
        CArgumentProvideImpl<TArgument>::Parse(m_rtArgument, rssValue);
        m_bArgumentAssigned = true;
    }

    /**
     * @brief Get the markup string for the argument type. Overload in derived class.
     * @return std::string Returns the markup (default empty).
     */
    virtual std::string GetArgumentOptionMarkup() override
    {
        return CArgumentProvideImpl<TArgument>::GetArgumentOptionMarkup();
    }

    /**
     * @brief Get the option details for the argument type. Overload in derived class.
     * @param[in] nMaxStringLen The maximum length of the string in characters; after the length is reached a newline should be
     * added.
     * @remarks The nMaxStringLen could be ignored if no other alignment is needed; the print function will align the words based on
     * their whitespace.
     * @return std::string Returns the option details (default empty).
     */
    virtual std::string GetArgumentOptionDetails(size_t nMaxStringLen) override
    {
        return CArgumentProvideImpl<TArgument>::GetOptionDetails(nMaxStringLen);
    }

    /**
     * @brief Get the value of the variable
     * @return std::string Returns the value of the variable.
     */
    virtual std::string GetArgumentValueString() override
    {
        return CArgumentProvideImpl<TArgument>::GetArgumentValueString(m_rtArgument);
    }

    /**
     * @brief Is the argument assigned?
     * @return bool Returns 'true' when the argument was assigned; otherwise 'false'.
     */
    virtual bool IsArgumentAssigned() override
    {
        return m_bArgumentAssigned;
    }

    /**
     * @brief Allow multi assignments?
     * @return bool Returns 'true' when multiple assignments are allowed; otherwise 'false'.
     */
    virtual bool AllowMultiArgumentAssign() override
    {
        return CArgumentProvideImpl<TArgument>::MultiArgument();
    }

private:
    TArgument&  m_rtArgument;           ///< The argument reference
    bool        m_bArgumentAssigned;    ///< Boolean keeping track of argument assignment.
};

template <typename TVar, typename... TArgumentGroup>
CArgumentDefBase::CArgumentDefBase(CCommandLine& rCLParser, const std::string& rssArgument,
    const std::shared_ptr<SGroupDef>& rptrGroup,const std::string& rssHelpText, uint32_t uiFlags, TVar& rtVar,
    TArgumentGroup... nArgumentGroup) :
    m_rCLParser(rCLParser), m_ptrGroup(rptrGroup), m_setArgumentGroups({ static_cast<size_t>(nArgumentGroup)... })
{
    // Set the flags, but remove the option/sub-option information.
    m_uiFlags = uiFlags &
        ~static_cast<uint32_t>(EArgumentFlags::option_argument) &
        ~static_cast<uint32_t>(EArgumentFlags::sub_option_argument);
    if (!rssArgument.empty())
    {
        std::string ssArgument = rssArgument;
        size_t nPos = 0;
        do
        {
            size_t nSeparator = ssArgument.find('|', nPos);
            std::string ssArgumentPart = helper::trim(ssArgument.substr(nPos, nSeparator - nPos));
            if (!uiFlags && !ssArgumentPart.empty())
                for (char& rc : ssArgumentPart)
                    rc = static_cast<char>(std::tolower(rc));
            if (ssArgumentPart.size())
            {
                SOptionName sOptionName{ ssArgumentPart, uiFlags };
                m_vecOptionNames.push_back(sOptionName);
            }
            if (nSeparator != std::string::npos)
                nSeparator++;
            nPos = nSeparator;
        } while (nPos < ssArgument.size());
    }
    if (!rssHelpText.empty()) m_ssHelpText = rssHelpText;
    m_ptrArgProvide = std::make_shared<CArgumentProvide<TVar>>(static_cast<CArgumentDefT<TVar>&>(*this), rtVar);
}

template <typename TCharType>
CArgumentIterator::CArgumentIterator(size_t nArgs, const TCharType** rgszArgs)
{
    // Start with the second argument (skipping the app name).
    for (size_t nArg = 1; rgszArgs && nArg < nArgs; nArg++)
    {
        // Create an UTF-8 string from the argument
        std::string ssArg = sdv::MakeUtf8String(rgszArgs[nArg]);
        if (ssArg.empty())
        {
            SArgumentParseException exception("Invalid argument!");
            exception.AddIndex(nArg);
            throw exception;
        }

        // Add to the queue
        m_queueArguments.push(ssArg);
    }
}

inline std::optional<std::string> CArgumentIterator::GetNext()
{
    if (m_queueArguments.empty()) return {};
    std::string ssArg = std::move(m_queueArguments.front());
    m_queueArguments.pop();
    m_nCounter++;
    return ssArg;
}

inline size_t CArgumentIterator::GetIndexOfLastArg() const
{
    return m_nCounter;
}

template <typename TCharType>
inline void CCommandLine::Parse(size_t nArgs, const TCharType** rgszArgs)
{
    switch (m_uiParseFlags)
    {
    case static_cast<uint32_t>(EParseFlags::assignment_character):
    case static_cast<uint32_t>(EParseFlags::no_assignment_character):
    case static_cast<uint32_t>(EParseFlags::assignment_next_arg):
        break;
    default:
        throw (SArgumentParseException("Invalid parse mode!"));
    }

    if (!nArgs || !rgszArgs[0])
    {
        SArgumentParseException exception("Missing arguments!");
        exception.AddIndex(0);
        throw exception;
    }

    CArgumentIterator argit(nArgs, rgszArgs);

    // Iterate through the arguments. Skip the first argument, which points to the name of the application.
    while (true)
    {
        // Get the next argument
        auto optArg = argit.GetNext();
        if (!optArg) return;

        // Create an UTF-8 string from the argument
        std::string ssArg = *optArg;

        if (ssArg.empty())
            throw (SArgumentParseException("Invalid argument!"));

        // Check for the (sub-)option character '-' (or under Windows '/').
        bool bSubOption = false;
        bool bOption = false;
        if (ssArg.size() >= 2 && ssArg[0] == '-' && ssArg[1] == '-')
            bSubOption = true;
        else if (ssArg.size() >= 2 && ssArg[0] == '-')
            bOption = true;
#ifdef _WIN32
        else if (ssArg.size() >= 2 && ssArg[0] == '/')
            bOption = true;
#endif

        // Find argument function
        auto fnFindAndAssign = [&](CArgumentDefBase& rArgument, const std::string& rssOptionName) -> bool
        {
            // Compare the argument name
            try
            {
                bool bRet = rArgument.CompareNameAndAssign(argit, ssArg, rssOptionName,
                    CheckParseFlag(EParseFlags::no_assignment_character) || CheckParseFlag(EParseFlags::assignment_next_arg));
                if (bRet)
                {
                    std::string ssOptionName(rssOptionName);
                    m_lstSupplied.emplace_back(std::ref(rArgument), ssArg);
                }
                return bRet;
            }
            catch (SArgumentParseException& rexception)
            {
                rexception.AddIndex(argit.GetIndexOfLastArg());
                rexception.AddArgument(ssArg);
                throw;
            }
        };

        // Find the argument
        bool bFound = false;
        if (bOption)
        {
            for (auto& rvtOption : m_mapSortedOptions)
            {
                bFound = fnFindAndAssign(rvtOption.second, rvtOption.first);
                if (bFound) break;
            }
        }
        else if (bSubOption)
        {
            for (auto& rvtOption : m_mapSortedSubOptions)
            {
                bFound = fnFindAndAssign(rvtOption.second, rvtOption.first);
                if (bFound) break;
            }
        } else // Default argument
            bFound = m_ptrDefaultArg ? fnFindAndAssign(*m_ptrDefaultArg.get(), {}) : false;

        if (!bFound)
        {
            SArgumentParseException exception("Argument unknown");
            exception.AddIndex(argit.GetIndexOfLastArg());
            exception.AddArgument(ssArg);
            throw exception;
        }
    }
}

template <typename TVar>
inline CArgumentDefT<TVar>& CCommandLine::DefineDefaultArgument(TVar& rtVar, const std::string& rssHelpText)
{
    uint32_t uiFlags = static_cast<uint32_t>(EArgumentFlags::default_argument);
    m_ptrDefaultArg = std::make_shared<CArgumentDefT<TVar>>(*this, "", m_ptrCurrentGroup, rssHelpText, uiFlags, rtVar);
    return static_cast<CArgumentDefT<TVar>&>(*m_ptrDefaultArg.get());
}

template <typename TVar, typename... TArgumentGroup>
inline CArgumentDefT<TVar>& CCommandLine::DefineOption(const std::string& rssArgument, TVar& rtVar, const std::string& rssHelpText,
    bool bCaseSensitive /*= true*/, size_t nArgumentGroup /*= 0*/, TArgumentGroup... nAdditionalGroups)
{
    uint32_t uiFlags = static_cast<uint32_t>(EArgumentFlags::option_argument);
    if (bCaseSensitive) uiFlags |= static_cast<uint32_t>(EArgumentFlags::case_sensitive);
    if constexpr (std::is_same_v<TVar, bool>)
        uiFlags |= static_cast<uint32_t>(EArgumentFlags::bool_option);
    auto ptrOption = std::make_shared<CArgumentDefT<TVar>>(*this, rssArgument, m_ptrCurrentGroup, rssHelpText, uiFlags, rtVar,
        nArgumentGroup, nAdditionalGroups...);
    m_lstOptionArgs.push_back(ptrOption);
    m_mapSortedOptions.emplace(rssArgument, *ptrOption.get());
    CArgumentDefT<TVar>& rArg = static_cast<CArgumentDefT<TVar>&>(*ptrOption.get());
    return rArg;
}

template <typename TVar, typename... TArgumentGroup>
inline CArgumentDefT<TVar>& CCommandLine::DefineSubOption(const std::string& rssArgument, TVar& rtVar, const std::string& rssHelpText,
    bool bCaseSensitive /*= true*/, size_t nArgumentGroup /*= 0*/, TArgumentGroup... nAdditionalGroups)
{
    uint32_t uiFlags = static_cast<uint32_t>(EArgumentFlags::sub_option_argument);
    if (bCaseSensitive) uiFlags |= static_cast<uint32_t>(EArgumentFlags::case_sensitive);
    if constexpr (std::is_same_v<TVar, bool>)
        uiFlags |= static_cast<uint32_t>(EArgumentFlags::bool_option);
    auto ptrOption = std::make_shared<CArgumentDefT<TVar>>(*this, rssArgument, m_ptrCurrentGroup, rssHelpText, uiFlags, rtVar,
        nArgumentGroup, nAdditionalGroups...);
    m_lstOptionArgs.push_back(ptrOption);
    m_mapSortedSubOptions.emplace(rssArgument, *ptrOption.get());
    CArgumentDefT<TVar>& rArg = static_cast<CArgumentDefT<TVar>&>(*ptrOption.get());
    return rArg;
}

template <typename... TArgumentGroup>
inline CArgumentDefT<bool>& CCommandLine::DefineFlagOption(const std::string& rssArgument, bool& rbFlag, const std::string& rssHelpText,
    bool bCaseSensitive /*= true*/, size_t nArgumentGroup /*= 0*/, TArgumentGroup... nAdditionalGroups)
{
    uint32_t uiFlags = static_cast<uint32_t>(EArgumentFlags::option_argument);
    uiFlags |= static_cast<uint32_t>(EArgumentFlags::flag_option);
    if (bCaseSensitive) uiFlags |= static_cast<uint32_t>(EArgumentFlags::case_sensitive);
    auto ptrOption = std::make_shared<CArgumentDefT<bool>>(*this, rssArgument, m_ptrCurrentGroup, rssHelpText, uiFlags, rbFlag,
        nArgumentGroup, nAdditionalGroups...);
    m_lstOptionArgs.push_back(ptrOption);
    m_mapSortedOptions.emplace(rssArgument, *ptrOption.get());
    return static_cast<CArgumentDefT<bool>&>(*ptrOption.get());
}

template <typename... TArgumentGroup>
inline CArgumentDefT<bool>& CCommandLine::DefineFlagSubOption(const std::string& rssArgument, bool& rbFlag, const std::string& rssHelpText,
    bool bCaseSensitive /*= true*/, size_t nArgumentGroup /*= 0*/, TArgumentGroup... nAdditionalGroups)
{
    uint32_t uiFlags = static_cast<uint32_t>(EArgumentFlags::sub_option_argument);
    uiFlags |= static_cast<uint32_t>(EArgumentFlags::flag_option);
    if (bCaseSensitive) uiFlags |= static_cast<uint32_t>(EArgumentFlags::case_sensitive);
    auto ptrOption = std::make_shared<CArgumentDefT<bool>>(*this, rssArgument, m_ptrCurrentGroup, rssHelpText, uiFlags, rbFlag,
        nArgumentGroup, nAdditionalGroups...);
    m_lstOptionArgs.push_back(ptrOption);
    m_mapSortedSubOptions.emplace(rssArgument, *ptrOption.get());
    CArgumentDefT<bool>& rArg = static_cast<CArgumentDefT<bool>&>(*ptrOption.get());
    return rArg;
}

#endif // !defined CMDLN_PARSER_H
