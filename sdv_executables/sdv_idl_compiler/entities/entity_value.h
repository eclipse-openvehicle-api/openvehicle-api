#ifndef ENTITIY_VALUE_H
#define ENTITIY_VALUE_H

#include "entity_base.h"
#include "../constvariant.h"
#include "../token.h"
#include "../tokenlist.h"
#include <vector>
#include <memory>

// Forward declarations
class CDeclarationEntity;

/**
 * @brief The entity value node base class.
 * @details Entity values form a value tree made of all the values that are part of one assignment. Each value has its special
 * value grammar when used in a assignment. All entities have their own personal linked list, which can be formed from other
 * entities when they are defined as type of a declaration. Variable declarations can also be part of a larger linked list. Arrays
 * form their own links within the linked list
 * Examples:
 * @code
 *      // Value tree for i: CSimpleTypeValueNode assigning the value 10.
 *      int32 i = 10;
 *
 *      // Value tree for S1: CCompoundTypeValueNode with child CSimpleTypeValueNode assigning the value 20.
 *      struct S1
 *      {
 *          const int32 m_i1 = 10;      // Value node for m_i1: CSimpleTypeValueNode assigning the value 10.
 *          int32 m_i2 = 20;            // Part of the value tree of S1.
 *      };
 *
 *      // Value tree for s1: sub-tree of S1 assigning the value 200 to m_i2.
 *      S1 s1 = {200};
 *
 *      // Value tree for S2: CCompoundTypeValueNode with children sub-tree of S1 and CSimpleTypeValueNode assigning the value 5.
 *      struct S2 : S1
 *      {
 *          int32 m_i3 = 5;             // Part of value tree of S2.
 *      };
 *
 *      // Value tree for s2: sub-tree of S2 assigning the value 150 to S1 and 100 to m_i3.
 *      s2 = {{150}, 100};
 *
 *      // Value tree for rgi: CArrayValueNode having two children assigning the value 10 and 20.
 *      int32 rgi[2] = {10, 20};
 *
 *      // Value tree for rgs3: CarrayValueNode having two children S1 assigning the values 1000 and 2000.
 *      S1 rgs3[2] = {{1000}, {2000}};
 * @endcode
 */
class CEntityValueNode : public std::enable_shared_from_this<CEntityValueNode>
{
public:
    /**
     * @brief Constructor
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Can only be nullptr for const and declaration entities or when
     * creating a value node for a definition which will be copied in a larger declaration at a later stage.
     */
    CEntityValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Copy constructor with new parent and entity.
     * @param[in] rValueNode Reference to the value node to copy from.
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     */
    CEntityValueNode(const CEntityValueNode& rValueNode, CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Default destructor
     * @remarks This constructor needs to be virtual to allow derived classes to be destroyed correctly.
     *
     */
    virtual ~CEntityValueNode() = default;

    /**
     * @brief Create a copy of the value. Prototype to be implemented from derived class.
     * @param[in] ptrEntity Smart pointer to the entity receiving the copy of the value node.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     * @return Returns a smart pointer of the copy of the value.
     */
    virtual CValueNodePtr CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const = 0;

    /**
     * @brief Process the assignment. Prototype, to be implemented by derived value class.
     * @param[in] rlstExpression Reference to the expression token list.
     */
    virtual void ProcessValueAssignment(const CTokenList& rlstExpression) = 0;

    /**
     * @brief Process the child nodes.
     * @param[in] rlstExpression Reference to the expression token list.
     */
    virtual void ProcessChildNodes(const CTokenList& rlstExpression);

    /**
     * @{
     * @brief Get the derived value class.
     * @tparam TValue The value class to request a pointer for.
     * @return Returns a pointer to the derived class or NULL when the requested class didn't derive from this value.
     */
    template <class TValue>
    TValue* Get();
    template <class TValue>
    const TValue* Get() const;
    /**
     * @}
     */

    /**
     * @brief Add a child node
     * @param[in] ptrChild Pointer to the child value node.
     */
    void AddChild(CValueNodePtr ptrChild);

    /**
     * @brief Get the parent node.
     * @return Returns a reference to the smart pointer of the parent node (if assigned).
     */
    const CValueNodePtr& GetParentNode() const { return m_ptrParent; }

    /**
     * @{
     * @brief Get the attached declaration entity.
     * @return Returns the smart pointer to the entity.
     */
    const CDeclarationEntity* GetDeclEntity() const { return m_ptrEntity->Get<CDeclarationEntity>(); }
    CDeclarationEntity* GetDeclEntity() { return m_ptrEntity->Get<CDeclarationEntity>(); }
    /**
     * @}
     */

    /**
     * @brief Get the declaration type build from the value chain.
     * @details The declaration type consists of "<base type> <type identifier> <arrays>".
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns a string with entity type.
     */
    virtual std::string GetDeclTypeStr(bool bResolveTypedef) const;

    /**
     * @{
     * @brief Information functions.
     * @details Each function is implemented by an entity value class. The IsConst function traverses through the parents to find
     * out whether or not a value is a const value (this is the case when the entity holding the top most value is a const
     * entity.)
     * @return Returns the information.
     */
    virtual bool IsArray() const { return false; }
    virtual bool IsUnbound() const { return false; }
    virtual bool IsConst() const;
    virtual bool IsDeclaration() const;
    virtual bool IsDynamic() const { return false; }
    virtual bool IsLiteral() const { return false; }
    virtual bool IsComplex() const { return false; }
    virtual bool HasParent() const{ return m_ptrParent ? true : false; }
    virtual bool HasChildren() const{ return !m_vecChildren.empty(); }
    /**
     * @}
     */

protected:
    CEntityPtr                  m_ptrEntity;        ///< The entity implementing the value type.
    const CValueNodePtr         m_ptrParent;        ///< Parent value node - can be nullptr for root node.
    std::vector<CValueNodePtr>  m_vecChildren;      ///< Child nodes.
};

/**
 * @brief Entity declaration value (used as const variable assignment as well as default declaration assignment).
 */
class CSimpleTypeValueNode : public CEntityValueNode
{
public:
    /**
     * @brief Constructor
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Can only be nullptr for upper most value level.
     */
    CSimpleTypeValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Copy constructor with new parent and entity.
     * @param[in] rValueNode Reference to the value node to copy from.
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     */
    CSimpleTypeValueNode(const CSimpleTypeValueNode& rValueNode, CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Default destructor
     * @remarks This constructor needs to be virtual to allow derived classes to be destroyed correctly.
     *
     */
    virtual ~CSimpleTypeValueNode() override = default;

    /**
     * @brief Create a copy of the value. Overload of CEntityValueNode::CreateCopy.
     * @param[in] ptrEntity Smart pointer to the entity receiving the copy of the value node.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     * @return Returns a smart pointer of the copy of the value.
     */
    virtual CValueNodePtr CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const override;

    /**
     * @brief Process the assignment. Overload of CEntityValueNode::ProcessValueAssignment.
     * @param[in] rlstExpression Reference to the expression token list.
     */
    virtual void ProcessValueAssignment(const CTokenList& rlstExpression) override;

    /**
     * @brief Set to a fixed value.
     * @remarks In case the entity is not a const entity, the value is considered to be a default value.
     * @param[in] rvarValue Reference to the fixed value.
     * @param[in] rlstExpression Reference to the expression list used to calculate the value.
     */
    void SetFixedValue(const CConstVariant& rvarValue, const CTokenList& rlstExpression);

    /**
     * @brief Set to a dynamic value
     * @remarks This function can only be called when the entity of any of the parent entities is not a const entity.
     * @param[in] rlstExpression Reference to the expression list used to calculate the value.
     */
    void SetDynamicValue(const CTokenList& rlstExpression);

    /**
     * @brief Is the value defined?
     * @return Returns 'true' when the value is defined; 'false' otherwise.
     */
    virtual bool IsDefined() const { return m_eValueDef != EValueDef::not_defined; }

    /**
     * @brief Is this value dynamic (will it be defined through a non-const variable definition)?
     * @return Returns 'true' when the value is dynamic; 'false' otherwise.
     */
    virtual bool IsDynamic() const override { return m_eValueDef != EValueDef::dynamic; }

    /**
     * @brief Get access to the underlying const variant.
     * @return The const variant storing the calculated value.
     */
    const CConstVariant &Variant() const { return m_varValue; }

private:
    /**
     * @brief The definition of the size is either fixed or not fixed. When it is fixed, it could be derived from assigning its
     * values.
     */
    enum class EValueDef
    {
        not_defined,            ///< Currently not defined yet.
        fixed,                  ///< The value is fixed; could be used for const and declaration entities.
        dynamic,                ///< The value is dependable on a variable declaration. Only to be used with a declaration entity.
    };

    CConstVariant   m_varValue;                             ///< The calculated value.
    CTokenList      m_lstExpression;                        ///< List of tokens holding the value expression.
    EValueDef       m_eValueDef = EValueDef::not_defined;   ///< Dynamic value based on variables.
};

/**
 * @brief Entity array value (containing an array of value pointers).
 * @remarks When the array contains non-allocated value pointers, the array has been declared, but the values haven't been
 * assigned.
 */
class CArrayValueNode : public CEntityValueNode
{
public:
    /**
     * @brief Constructor
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     */
    CArrayValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Copy constructor with new parent and entity.
     * @param[in] rValueNode Reference to the value node to copy from.
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     */
    CArrayValueNode(const CArrayValueNode& rValueNode, CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Default destructor
     * @remarks This constructor needs to be virtual to allow derived classes to be destroyed correctly.
     *
     */
    virtual ~CArrayValueNode() override = default;

    /**
     * @brief Create a copy of the value. Overload of CEntityValueNode::CreateCopy.
     * @param[in] ptrEntity Smart pointer to the entity receiving the copy of the value node.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     * @return Returns a smart pointer of the copy of the value.
     */
    virtual CValueNodePtr CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const override;

    /**
     * @brief Process the assignment. Overload of CEntityValueNode::ProcessValueAssignment.
     * @param[in] rlstExpression Reference to the expression token list.
     */
    virtual void ProcessValueAssignment(const CTokenList& rlstExpression) override;

    /**
	 * @brief Process the child nodes. Overload of CEntityValueNode::ProcessChildNodes.
	 * @param[in] rlstExpression Reference to the expression token list.
	 */
	void ProcessChildNodes(const CTokenList& rlstExpression) override;

    /**
	 * @brief Process the string nodes. This is an array processing function based on a string as character array.
	 * @param[in] rToken Reference to the string literal token.
	 */
	void ProcessStringNode(const CToken& rToken);

	/**
     * @brief Set the fixed size of the array when available during declaration.
     * @attention Must not be called after the value has been set to dynamic or set to a delayed fixed size.
     * @param[in] nSize The calculated size of the array.
     * @param[in] rlstExpression Reference to the expression list used to calculate the value.
     */
    void SetFixedSize(size_t nSize, const CTokenList& rlstExpression);

    /**
     * @brief Set the array to a dynamic size based on a variable declaration. This is only allowed for entities that are not
     * declared as const entity (where all the parent values are not defined as const).
     * @attention Must not be called after the value has been set to a fixed size (delayed or not).
     * @param[in] nSize The calculated size of the array.
     * @param[in] rlstExpression Reference to the expression list used to calculate the size.
     */
    void SetDynamicSize(size_t nSize, const CTokenList& rlstExpression);

    /**
     * @brief Set the array to a fixed size through its value assignment. This is only allowed for entities that are declared as
     * const entity (where one of the parents was defined as const).
     * @attention Must not be called after the value has been set to a fixed size (not delayed) or a dynamic size.
     */
    void SetFixedSizeUnbound();

    /**
     * @{
     * @brief Element access of the array.
     * @param[in] nIndex The index of the element. For dynamic arrays, only an index of 0 is allowed.
     * @return Returns a reference to the element.
     */
    const CValueNodePtr& operator[](size_t nIndex) const;
    CValueNodePtr& operator[](size_t nIndex);
    /**
     * @}
     */

    /**
     * @brief Get the size of the array.
     * @return The size of the array.
     */
    size_t GetSize() const;

    /**
     * @brief This is an array entity.
     * @return Returns 'true'.
     */
    virtual bool IsArray() const override;

    /**
     * @brief Is the size of the array defined through the assignment?
     * @return Returns 'true' when the size of the array has to be defined through the assignment.
     */
    virtual bool IsUnbound() const override;

    /**
     * @brief Get the size expression.
     * @return The size expression string.
     */
	std::string GetSizeExpression() const;

    /**
     * @brief Get the declaration type build from the value chain. Overload of CEntityValueNode::GetDeclTypeStr.
     * @details The declaration type consists of "<base type> <type identifier> <arrays>".
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns a string with entity type.
     */
    virtual std::string GetDeclTypeStr(bool bResolveTypedef) const override;

private:
    /**
     * @brief The definition of the size is either fixed or not fixed. When it is fixed, it could be derived from assigning its
     * values.
     */
    enum class ESizeDef
    {
        not_defined,            ///< Currently not defined yet.
        fixed,                  ///< The size is fixed; could be used for const and declaration entities.
        dynamic,                ///< The size is dependable on a variable declaration. Only to be used with a declaration entity.
        fixed_unbound,          ///< The size is fixed, but gets defined during value assignment. Only to be used with a const
                                ///< entity.
    };

    CTokenList          m_lstArraySizeExpression;               ///< List of tokens holding the array size expression.
    ESizeDef            m_eSizeDef = ESizeDef::not_defined;     ///< The size definition.
 };

 /**
 * @brief Entity struct value containing two arrays of value pointers, one array for the member const declarations and one array
 * for the member variable declarations.
 */
class CCompoundTypeValueNode : public CEntityValueNode
{
public:
    /**
     * @brief Constructor
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Can only be nullptr when creating a value node for a definition
     * which will be copied in a larger declaration at a later stage.
     */
    CCompoundTypeValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Copy constructor with new parent and entity.
     * @param[in] rValueNode Reference to the value node to copy from.
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     */
    CCompoundTypeValueNode(const CCompoundTypeValueNode& rValueNode, CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Default destructor
     * @remarks This constructor needs to be virtual to allow derived classes to be destroyed correctly.
     *
     */
    virtual ~CCompoundTypeValueNode() override = default;

    /**
     * @brief Create a copy of the value. Overload of CEntityValueNode::CreateCopy.
     * @param[in] ptrEntity Smart pointer to the entity receiving the copy of the value node.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     * @return Returns a smart pointer of the copy of the value.
     */
    virtual CValueNodePtr CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const override;

    /**
     * @brief Process the assignment. Overload of CEntityValueNode::ProcessValueAssignment.
     * @param[in] rlstExpression Reference to the expression token list.
     */
    virtual void ProcessValueAssignment(const CTokenList& rlstExpression) override;

    /**
     * @brief Return the value for the supplied member.
     * @param[in] rssName Name Reference to the string object containing the name of the member.
     * @return Returns the value node smart pointer of the requested member, or nullptr when member doesn't have a value or
     * doesn't exist.
     */
    CValueNodePtr Member(const std::string& rssName) const;
};

/**
 * @brief Interface value node.
 * @note The only assignable values of an interface are "null" and "0". No variable is allowed to be assigned and an interface
 * cannot be const.
 */
class CInterfaceValueNode : public CEntityValueNode
{
public:
    /**
     * @brief Constructor
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Can only be nullptr for upper most value level.
     */
    CInterfaceValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Copy constructor with new parent and entity.
     * @param[in] rValueNode Reference to the value node to copy from.
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     */
    CInterfaceValueNode(const CInterfaceValueNode& rValueNode, CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Default destructor
     * @remarks This constructor needs to be virtual to allow derived classes to be destroyed correctly.
     *
     */
    virtual ~CInterfaceValueNode() override = default;

    /**
     * @brief Create a copy of the value. Overload of CEntityValueNode::CreateCopy.
     * @param[in] ptrEntity Smart pointer to the entity receiving the copy of the value node.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     * @return Returns a smart pointer of the copy of the value.
     */
    virtual CValueNodePtr CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const override;

    /**
     * @brief Process the assignment. Overload of CEntityValueNode::ProcessValueAssignment.
     * @param[in] rlstExpression Reference to the expression token list.
     */
    virtual void ProcessValueAssignment(const CTokenList& rlstExpression) override;

    /**
     * @brief Is this value dynamic (will it be defined through a non-const variable definition)? This is always the case.
     * @return Returns 'true' when the value is dynamic; 'false' otherwise.
     */
    virtual bool IsDynamic() const override { return true; }
};

/**
 * @brief Enum entity declaration value.
 */
class CEnumValueNode : public CEntityValueNode
{
public:
    /**
     * @brief Constructor
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Can only be nullptr for upper most value level.
     */
    CEnumValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Copy constructor with new parent and entity.
     * @param[in] rValueNode Reference to the value node to copy from.
     * @param[in] ptrEntity Smart pointer to the entity implementing the type. Cannot be nullptr.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     */
    CEnumValueNode(const CEnumValueNode& rValueNode, CEntityPtr ptrEntity, const CValueNodePtr ptrParent);

    /**
     * @brief Default destructor
     * @remarks This constructor needs to be virtual to allow derived classes to be destroyed correctly.
     *
     */
    virtual ~CEnumValueNode() override = default;

    /**
     * @brief Create a copy of the value. Overload of CEntityValueNode::CreateCopy.
     * @param[in] ptrEntity Smart pointer to the entity receiving the copy of the value node.
     * @param[in] ptrParent Smart pointer to the parent value. Cannot be nullptr.
     * @return Returns a smart pointer of the copy of the value.
     */
    virtual CValueNodePtr CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const override;

    /**
     * @brief Process the assignment. Overload of CEntityValueNode::ProcessValueAssignment.
     * @param[in] rlstExpression Reference to the expression token list.
     */
    virtual void ProcessValueAssignment(const CTokenList& rlstExpression) override;

    /**
     * @brief Is the value defined?
     * @return Returns 'true' when the value is defined; 'false' otherwise.
     */
    virtual bool IsDefined() const { return m_ptrEntryVal ? true : false; }

    /**
     * @brief Get access to the assigned const variant of the enum entry.
     * @return The const variant storing the value.
     */
    const CConstVariant& Variant() const;

    /**
     * @brief Get access to the assigned name string of the enum entry.
     * @return The name string of the enum entry.
     */
    std::string String() const;

private:
    CEntityPtr      m_ptrEntryVal;      ///< The enum entry value when assigned.
    CTokenList      m_lstExpression;    ///< List of tokens holding the value expression.
};

template <class TValue>
TValue* CEntityValueNode::Get()
{
    return dynamic_cast<TValue*>(this);
}

template <class TValue>
const TValue* CEntityValueNode::Get() const
{
    return dynamic_cast<const TValue*>(this);
}

#endif // !define(ENTITY_VALUE_H)