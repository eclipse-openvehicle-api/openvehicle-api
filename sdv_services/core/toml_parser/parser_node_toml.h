#ifndef PARSER_NODE_TOML_H
#define PARSER_NODE_TOML_H

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <interfaces/toml.h>
#include <support/interface_ptr.h>

// Forward declaration
class CArray;
class CTable;

/**
 * @brief Find the first separator character. Do not include string content (single/or double quoted) and escape characters.
 * @param[in] rss Reference to the string.
 * @param[in] rssSeparator One of the characters to find in the string. Must not be an empty string!
 * @return The position of the first separator character or std::string::npos when none has found.
 */
size_t FindFirst(const std::string& rss, const std::string& rssSeparator = ".");

/**
 * @brief Find the last separator character. Do not include string content (single/or double quoted) and escape characters.
 * @param[in] rss Reference to the string.
 * @param[in] rssSeparator One of the characters to find in the string. Must not be an empty string!
 * @return The position of the last separator character or std::string::npos when none has found.
 */
size_t FindLast(const std::string& rss, const std::string& rssSeparator = ".");

/**
 * @brief Compare both string ignoring the quotes at the first position and last position.
 * @param[in] rss1 Reference to the first string.
 * @param[in] rss2 Reference to the second string.
 * @return The comparison result.
 */
bool CompareEqual(const std::string& rss1, const std::string& rss2);

/**
 * @brief Escape a string using escape characters and UTF values.
 */
std::string EscapeString(const std::string& rssString, const char cQuoteType = '\"');

/**
 * @brief Node to build up the parse tree
 */
class CNode : public std::enable_shared_from_this<CNode>, public sdv::IInterfaceAccess, public sdv::toml::INodeInfo
{
protected:
    /**
     * @brief Constructs a new node object representing a table or an array.
     * @param[in] rssName Reference to the name of the node.
     */
    CNode(const std::string& rssName);

public:
    /**
     * @brief Deleted since Nodes should only be handled via smart-pointer
     * @{
     */
    CNode(const CNode&) = delete;
    CNode& operator=(const CNode&) = delete;
    CNode(const CNode&&) = delete;
    CNode& operator=(const CNode&&) = delete;
    /**
     * @}
     */

    /**
     * @brief Destroy the node object
     */
    ~CNode();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::toml::INodeInfo)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Get the node name. Overload of sdv::toml::INodeInfo::GetName.
     * @return String containing the name of the node.
     */
    virtual sdv::u8string GetName() const override;

    /**
     * @brief The node value. Overload of sdv::toml::INodeInfo::GetValue.
     * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
     * returns empty.
     */
    virtual sdv::any_t GetValue() const override;

    /**
     * @brief The node value. Overload of sdv::toml::INodeInfo::GetTOML.
     * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
     * returns empty.
     */
    virtual sdv::u8string GetTOML() const override;

    /**
     * @brief Gets the array value of a node
     * @return Returns a shared pointer of the array value stored in the node if the stored type is array
     */
    std::shared_ptr<const CArray> GetArray() const;

    /**
     * @brief Gets the array value of a node
     * @return Returns a shared pointer of the array value stored in the node if the stored type is array
     */
    std::shared_ptr<CArray> GetArray();

    /**
     * @brief Gets the table value of a node
     * @return Returns a shared pointer of the table value stored in the node if the stored type is table
     */
    std::shared_ptr<const CTable> GetTable() const;

    /**
     * @brief Gets the table value of a node
     * @return Returns a shared pointer of the table value stored in the node if the stored type is table
     */
    std::shared_ptr<CTable> GetTable();

protected:
    /**
     * @brief Gets the Parent Node
     * @return Returns the parent Node
     * @attention Beware of expiring pointers
     */
    std::weak_ptr<const CNode> GetParent() const;

public:
    /**
     * @brief Set the parent node.
     * @param[in] rptrParent Reference to the node to assign to this node as a parent.
     */
    void SetParent(const std::shared_ptr<CNode>& rptrParent);

    /**
     * @brief Accesses a node by its key in the parse tree.
     * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
     * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and traversed
     * by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'. These access
     * conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
     * @attention Array indexing starts with 0!
     * @param[in] rssPath The path of the node to searched for.
     * @return Returns a shared pointer to the wanted Node if it was found or a node with invalid content if it was not found.
     */
    virtual std::shared_ptr<CNode> GetDirect(const std::string& rssPath) const;

    /**
     * @brief Create the TOML text based on the content using an optional parent node.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @return The string containing the TOML text.
     */
    std::string CreateTOMLText(const std::string& rssParent = std::string()) const;

    /**
     * @brief Get the TOML text based on the content.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @param[in] rssLastPrintedTable Reference to the string containing the last printed table. This might be necessary in case a
     * different table was printed in between.
     * @param[in] bFirst When set, this is the first entry in an array or table.
     * @param[in] bEmbedded When set, this is an embedded definition in an array or table.
     * @param[in] bAssignment When set, this is a table assignment.
     * @param[in] bRoot Only for table entries, when set this is the root entry (suppress the table name).
     * @return The string containing the TOML text.
     */
    virtual std::string CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst = true,
        bool bEmbedded = false, bool bAssignment = true, bool bRoot = false) const = 0;

private:
    std::weak_ptr<CNode>                m_ptrParent;    ///< Weak pointer to the parent node (if existing).
    std::string                         m_ssName;       ///< Name of the node.

public:
    /**
     * @brief Searches the subtree of the node for a node at the given location using the provided path.
     * @remarks The path elements of arrays and tables are separated by a dot.
     * @param[in] rssPath Reference to the string containing the path of the node to find.
     * @return Returns a shared pointer to the wanted Node if it is found or an error-Node if it is not
     */
    virtual std::shared_ptr<CNode> Find(const std::string& rssPath) const;

    /**
     * @brief Adds a given node to a given path in the tree
     * @remarks The path elements of arrays and tables are separated by a dot.
     * @param[in] rssPath Reference to the string containing the path in the tree of the location to the new node to be inserted.
     * @param[in] rptrNode Reference to the smart pointer containing the new node to be added.
     * @param[in] bDefinedExplicitly If a table that is created to create the path of the node to be added is defined explicitly.
     * @throw XInvalidAccessException Throws an XInvalidAccessException if a ancestor node is not open to add children.
     * @throw XDuplicateNameException Throws a XDuplicateNameException if a node with the same path as the node to
     * be added is already defined explicitly
     */
    virtual void Add(const std::string& rssPath, const std::shared_ptr<CNode>& rptrNode, bool bDefinedExplicitly = true);
};

/**
 * @brief Boolean value node.
 */
class CBooleanNode : public CNode
{
public:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the string containing the name of the node.
     * @param[in] bVal The value to assign.
     */
    CBooleanNode(const std::string& rssName, bool bVal);

    /**
     * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
     * @return Type of the node.
     */
    virtual sdv::toml::ENodeType GetType() const override;

    /**
     * @brief The node value. Overload of sdv::toml::INodeInfo::GetValue.
     * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
     * returns empty.
     */
    virtual sdv::any_t GetValue() const override;

    /**
     * @brief Get the TOML text based on the content. Overload of CNode::CreateTOMLText.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @param[in] rssLastPrintedTable Reference to the string containing the last printed table. This might be necessary in case a
     * different table was printed in between.
     * @param[in] bFirst When set, this is the first entry in an array or table.
     * @param[in] bEmbedded When set, this is an embedded definition in an array or table.
     * @param[in] bAssignment When set, this is a table assignment.
     * @param[in] bRoot Only for table entries, when set this is the root entry (suppress the table name).
     * @return The string containing the TOML text.
     */
    virtual std::string CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
        bool bAssignment, bool bRoot) const override;

private:
    bool    m_bVal;    ///< Value in case of boolean node.
};

/**
 * @brief Integer value node.
 */
class CIntegerNode : public CNode
{
public:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the string containing the name of the node.
     * @param[in] iVal The value to assign.
     */
    CIntegerNode(const std::string& rssName, int64_t iVal);

    /**
     * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
     * @return Type of the node.
     */
    virtual sdv::toml::ENodeType GetType() const override;

    /**
     * @brief The node value. Overload of sdv::toml::INodeInfo::GetValue.
     * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
     * returns empty.
     */
    virtual sdv::any_t GetValue() const override;

    /**
     * @brief Get the TOML text based on the content. Overload of CNode::CreateTOMLText.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @param[in] rssLastPrintedTable Reference to the string containing the last printed table. This might be necessary in case a
     * different table was printed in between.
     * @param[in] bFirst When set, this is the first entry in an array or table.
     * @param[in] bEmbedded When set, this is an embedded definition in an array or table.
     * @param[in] bAssignment When set, this is a table assignment.
     * @param[in] bRoot Only for table entries, when set this is the root entry (suppress the table name).
     * @return The string containing the TOML text.
     */
    virtual std::string CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
        bool bAssignment, bool bRoot) const override;

private:
    int64_t     m_iVal;     ///< Value in case of integer node.
};

/**
 * @brief Floating point value node.
 */
class CFloatingPointNode : public CNode
{
public:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the string containing the name of the node.
     * @param[in] dVal The value to assign.
     */
    CFloatingPointNode(const std::string& rssName, double dVal);

    /**
     * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
     * @return Type of the node.
     */
    virtual sdv::toml::ENodeType GetType() const override;

    /**
     * @brief The node value. Overload of sdv::toml::INodeInfo::GetValue.
     * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
     * returns empty.
     */
    virtual sdv::any_t GetValue() const override;

    /**
     * @brief Get the TOML text based on the content. Overload of CNode::CreateTOMLText.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @param[in] rssLastPrintedTable Reference to the string containing the last printed table. This might be necessary in case a
     * different table was printed in between.
     * @param[in] bFirst When set, this is the first entry in an array or table.
     * @param[in] bEmbedded When set, this is an embedded definition in an array or table.
     * @param[in] bAssignment When set, this is a table assignment.
     * @param[in] bRoot Only for table entries, when set this is the root entry (suppress the table name).
     * @return The string containing the TOML text.
     */
    virtual std::string CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
        bool bAssignment, bool bRoot) const override;

private:
    double      m_dVal;     ///< Value in case of floating point node.
};

/**
 * @brief String value node.
 */
class CStringNode : public CNode
{
public:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the string containing the name of the node.
     * @param[in] rssVal The value to assign.
     */
    CStringNode(const std::string& rssName, const std::string& rssVal);

    /**
     * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
     * @return Type of the node.
     */
    virtual sdv::toml::ENodeType GetType() const override;

    /**
     * @brief The node value. Overload of sdv::toml::INodeInfo::GetValue.
     * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
     * returns empty.
     */
    virtual sdv::any_t GetValue() const override;

    /**
     * @brief Get the TOML text based on the content. Overload of CNode::CreateTOMLText.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @param[in] rssLastPrintedTable Reference to the string containing the last printed table. This might be necessary in case a
     * different table was printed in between.
     * @param[in] bFirst When set, this is the first entry in an array or table.
     * @param[in] bEmbedded When set, this is an embedded definition in an array or table.
     * @param[in] bAssignment When set, this is a table assignment.
     * @param[in] bRoot Only for table entries, when set this is the root entry (suppress the table name).
     * @return The string containing the TOML text.
     */
    virtual std::string CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
        bool bAssignment, bool bRoot) const override;

private:
    std::string     m_ssVal;        ///< Value in case of string or illegal (error) node.
};

/**
 * @brief Base structure for arrays and tables.
 */
class CNodeCollection : public CNode, public sdv::toml::INodeCollection
{
protected:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the name of the node.
     */
    CNodeCollection(const std::string& rssName);

public:
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::toml::INodeCollection)
        SDV_INTERFACE_CHAIN_BASE(CNode)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Returns the amount of nodes. Overload of sdv::toml::INodeCollection::GetCount.
     * @return The amount of nodes.
     */
    virtual uint32_t GetCount() const override;

    /**
     * @brief Get the node. Overload of sdv::toml::INodeCollection::GetNode.
     * @param[in] uiIndex Index of the node to get.
     * @return Interface to the node object.
     */
    virtual IInterfaceAccess* GetNode(/*in*/ uint32_t uiIndex) const override;

    /**
     * @brief Get the node.
     * @param[in] uiIndex Index of the node to get.
     * @return Smart pointer to the node object.
     */
    std::shared_ptr<CNode> Get(uint32_t uiIndex) const;

    /**
     * @brief Searches a node by its key in the parse tree
     * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child
     * name. E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be
     * accessed and traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of
     * the array 'array'. These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
     * @attention Array indexing starts with 0!
     * @param[in] ssPath The path of the node to searched for.
     * @return Returns an interface the requested node if available.
     */
    virtual sdv::IInterfaceAccess* GetNodeDirect(/*in*/ const sdv::u8string& ssPath) const override;

    /**
     * @brief Add an element to the collection.
     * @param[in] rptrNode Reference to the node element smart pointer.
     * @param[in] bUnique When set, check prevents adding an element with the same name.
     * @return Returns whether the element addition was successful.
     */
    bool AddElement(const std::shared_ptr<CNode>& rptrNode, bool bUnique = false);

private:

    std::vector<std::shared_ptr<CNode>>     m_vecContent;                   ///< Vector holding the child elements

public:
    bool                                    m_bDefinedExplicitly = true;    ///< WHen set, the array/table is defined explicitly
                                                                            ///< (not internal).
};

/**
 * @brief A dynamic table structure that allows mixed data in form of key value pairs
 */
class CTable : public CNodeCollection
{
protected:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the name of the node.
     */
    CTable(const std::string& rssName);

public:
    /**
     * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
     * @return Type of the node.
     */
    virtual sdv::toml::ENodeType GetType() const override;

    /**
     * @brief Accesses a node by its key in the parse tree. Overload of CNode::GetDirect.
     * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
     * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and traversed
     * by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'. These access
     * conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
     * @attention Array indexing starts with 0!
     * @param[in] rssPath The path of the node to searched for.
     * @return Returns a shared pointer to the wanted Node if it was found or a node with invalid content if it was not found.
     */
    virtual std::shared_ptr<CNode> GetDirect(const std::string& rssPath) const override;

    /**
     * @brief Get the TOML text based on the content. Overload of CNode::CreateTOMLText.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @param[in] rssLastPrintedTable Reference to the string containing the last printed table. This might be necessary in case a
     * different table was printed in between.
     * @param[in] bFirst When set, this is the first entry in an array or table.
     * @param[in] bEmbedded When set, this is an embedded definition in an array or table.
     * @param[in] bAssignment When set, this is a table assignment.
     * @param[in] bRoot Only for table entries, when set this is the root entry (suppress the table name).
     * @return The string containing the TOML text.
     */
    virtual std::string CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
        bool bAssignment, bool bRoot) const override;

    /**
     * @brief Adds a given node to a given path in the tree. Overload of CNode::Add.
     * @param[in] rssPath Reference to the path in the tree where the new node is to be added
     * @param[in] rptrNode Reference to the smart pointer holding the node.
     * @param[in] bDefinedExplicitly If a table that is created to create the path of the node to be added is
     * defined explicitly
     * @throw XInvalidAccessException Throws an XInvalidAccessException if a ancestor node is not open to add
     * children
     * @throw XDuplicateNameException Throws a XDuplicateNameException if a node with the same path as the node to
     * be added is already defined explicitly
     */
    virtual void Add(const std::string& rssPath, const std::shared_ptr<CNode>& rptrNode, bool bDefinedExplicitly) override;

    /**
     * @brief Searches the subtree of the node for a node at the given location using the provided path. Overload of CNode::Find.
     * @remarks The path elements of arrays and tables are separated by a dot.
     * @param[in] rssPath Reference to the path in the tree where the new node is to be added
     * @return Returns a shared pointer to the wanted Node if it is found or an error-Node if it is not
     */
    virtual std::shared_ptr<CNode> Find(const std::string& rssPath) const override;

    bool        m_bOpenToAddChildren = true;                    ///< If internal table, the table can be extended until the table
                                                                ///< is closed.

};

/**
 * @brief A dynamic array structure that allows mixed data
 * @details The definition of an array in TOML differentiate massively from the syntax to access the elements. For example an array
 * in TOML could be defined by:
 * @code
 * integers = [ 1, 2, 3 ]
 * nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
 * [[products]]
 * name = "Hammer"
 * sku = 738594937
 * @endcode
 * The first two examples define the complete array at once. The third example defines one element to be added to an array. Random
 * access to previous definitions is not required.
 * The access functions need random access to each element. The GetDirect function uses the syntax similar to C++:
 * @code
 * integers[1]                  --> gives: 2
 * nested_mixed_array[1][2]     --> gives: "c"
 * products[0].sku              --> gives: 738594937
 * @endcode
 * To find array elements, the path names are composed of elements separated by a dot. The Add and Find functions use the following
 * syntax:
 * @code
 * integers.1                   --> stores: 2
 * nested_mixed_array.1.2       --> stores: "c"
 * products.0.sku               --> stores: 738594937
 * @endcode
 */
class CArray : public CNodeCollection
{
protected:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the name of the node.
     */
    CArray(const std::string& rssName);

public:
    /**
     * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
     * @return Type of the node.
     */
    virtual sdv::toml::ENodeType GetType() const override;

    /**
     * @brief Accesses a node by its key in the parse tree. Overload of CNode::GetDirect.
     * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
     * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and traversed
     * by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'. These access
     * conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
     * @attention Array indexing starts with 0!
     * @param[in] rssPath Reference to the path of the node to searched for.
     * @return Returns a shared pointer to the wanted Node if it was found or a node with invalid content if it was not found.
     */
    virtual std::shared_ptr<CNode> GetDirect(const std::string& rssPath) const override;

    /**
     * @brief Get the TOML text based on the content. Overload of CNode::CreateTOMLText.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @param[in] rssLastPrintedTable Reference to the string containing the last printed table. This might be necessary in case a
     * different table was printed in between.
     * @param[in] bFirst When set, this is the first entry in an array or table.
     * @param[in] bEmbedded When set, this is an embedded definition in an array or table.
     * @param[in] bAssignment When set, this is a table assignment.
     * @param[in] bRoot Only for table entries, when set this is the root entry (suppress the table name).
     * @return The string containing the TOML text.
     */
    virtual std::string CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
        bool bAssignment, bool bRoot) const override;

    /**
     * @brief Adds a given node to a given path in the tree. Overload of CNode::Add.
     * @param[in] rssPath Reference to the path in the tree where the new node is to be added.
     * @param[in] rptrNode Reference top the smart pointer holding the node.
     * @param[in] bDefinedExplicitly If a table that is created to create the path of the node to be added is
     * defined explicitly
     * @throw XInvalidAccessException Throws an XInvalidAccessException if a ancestor node is not open to add
     * children
     * @throw XDuplicateNameException Throws a XDuplicateNameException if a node with the same path as the node to
     * be added is already defined explicitly
     */
    virtual void Add(const std::string& rssPath, const std::shared_ptr<CNode>& rptrNode, bool bDefinedExplicitly) override;

    /**
     * @brief Searches the subtree of the node for a node at the given location using the provided path. Overload of CNode::Find.
     * @remarks The path elements of arrays and tables are separated by a dot.
     * @param[in] rssPath Reference to the path of the node to find.
     * @return Returns a shared pointer to the wanted Node if it is found or an error-Node if it is not
     */
    virtual std::shared_ptr<CNode> Find(const std::string& rssPath) const override;
};

/**
 * @brief Normal table
 */
class CNormalTable : public CTable
{
public:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the name of the node.
     */
    CNormalTable(const std::string& rssName) : CTable(rssName) {}
};

/**
 * @brief Inline table
 */
class CInlineTable : public CTable
{
public:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the name of the node.
     */
    CInlineTable(const std::string& rssName) : CTable(rssName) {}
};

/**
 * @brief Normal array
 */
class CNormalArray : public CArray
{
public:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the name of the node.
     */
    CNormalArray(const std::string& rssName) : CArray(rssName) {}
};

/**
 * @brief Array of tables
 */
class CTableArray : public CArray
{
public:
    /**
     * @brief Constructor
     * @param[in] rssName Reference to the name of the node.
     */
    CTableArray(const std::string& rssName) : CArray(rssName) {}

    /**
     * @brief Get the TOML text based on the content. Overload of CNode::CreateTOMLText.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @param[in] rssLastPrintedTable Reference to the string containing the last printed table. This might be necessary in case a
     * different table was printed in between.
     * @param[in] bFirst When set, this is the first entry in an array or table.
     * @param[in] bEmbedded When set, this is an embedded definition in an array or table.
     * @param[in] bAssignment When set, this is a table assignment.
     * @param[in] bRoot Only for table entries, when set this is the root entry (suppress the table name).
     * @return The string containing the TOML text.
     */
    virtual std::string CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
        bool bAssignment, bool bRoot) const override;

    /**
    * @brief Adds a given node to a given path in the tree. Overload of CNode::Add.
    * @param[in] rssPath Reference to the path in the tree where the new node is to be added.
    * @param[in] rptrNode Reference top the smart pointer holding the node.
    * @param[in] bDefinedExplicitly If a table that is created to create the path of the node to be added is
    * defined explicitly
    * @throw XInvalidAccessException Throws an XInvalidAccessException if a ancestor node is not open to add
    * children
    * @throw XDuplicateNameException Throws a XDuplicateNameException if a node with the same path as the node to
    * be added is already defined explicitly
    */
    virtual void Add(const std::string& rssPath, const std::shared_ptr<CNode>& rptrNode, bool bDefinedExplicitly) override;

    /**
     * @brief Searches the subtree of the node for a node at the given location using the provided path. Overload of CNode::Find.
     * @remarks The path elements of arrays and tables are separated by a dot.
     * @param[in] rssPath Reference to the string containing the path of the node to find.
     * @return Returns a shared pointer to the wanted Node if it is found or an error-Node if it is not
     */
    virtual std::shared_ptr<CNode> Find(const std::string& rssPath) const override;
};

/**
 * @brief Root table
 */
class CRootTable : public CNormalTable
{
public:
    /**
     * @brief Constructor
     */
    CRootTable() : CNormalTable("root") {}

    /**
     * @brief Get the TOML text based on the content. Overload of CNode::CreateTOMLText.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @param[in] rssLastPrintedTable Reference to the string containing the last printed table. This might be necessary in case a
     * different table was printed in between.
     * @param[in] bFirst When set, this is the first entry in an array or table.
     * @param[in] bEmbedded When set, this is an embedded definition in an array or table.
     * @param[in] bAssignment When set, this is a table assignment.
     * @param[in] bRoot Only for table entries, when set this is the root entry (suppress the table name).
     * @return The string containing the TOML text.
     */
    virtual std::string CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
        bool bAssignment, bool bRoot) const override;
};

#endif // !defined PARSER_NODE_TOML_H