#ifndef DEFINITION_ENTITY_H
#define DEFINITION_ENTITY_H

#include "entity_base.h"

/**
 * @brief Support flags for the content of a definition (the part between the curly brackets '{...}').
 */
enum class EDefinitionSupport : uint32_t
{
    support_variable,               ///< Support variable declarations.
    support_const_variable,         ///< Support const variable declarations.
    support_case_declaration,       ///< Support case declarations.
    support_enum_entry,             ///< Support enumerator entry.
    support_module,                 ///< Support module definitions.
    support_typedef,                ///< Support typedef declarations.
    support_interface,              ///< Support interface definitions.
    support_struct,                 ///< Support struct definitions.
    support_union,                  ///< Support union definitions.
    support_enum,                   ///< Support enum definitions.
    support_exception,              ///< Support exception definitions.
    support_attribute,              ///< Support attribute declarations.
    support_operation,              ///< Support operation declarations.
};

/**
 * @brief The base for definition entity definitions within an IDL file (struct, union, exception, interface, enum).
 * @details The definition entity definitions all have a similar setup with small differences in detail. Consider the following
 * structures:
 * @code
 *      struct <name | anonymous> : <inheritance list> { <member list> }
 *      union <name | anonymous> switch(<type>) { <case member list> }
 *      interface <name> : <inheritance list> { <member list> }
 *      exception <name> { <member list> }
 *      enum <name> : <type> { <item list> }
 * @endcode
 * The following generalized structure applies to all complex structures:
 * @code
 *      keyword name
 *      prefix keyword name postfix { definition }
 * @endcode
 * The first statement is a forward declaration. The second represents the type definition.
 * The prefix is used to provide a specific interpretation to the definition.
 * The keyword defines the type of definition (can be struct, union, interface, exception, enum).
 * The name is the defined name of the definition. For types that allow anonymous names, this part is optionally.
 * The postfix allows the specification of additional information needed for the definition (inheritynce list, switch list).
 * The definition defines the content of the type.
 * Some declarations might start as a type definition (e.g. struct, union, enum) followed with a declaration. For example:
 * @code
 *      struct <def_name> { <definition> } <decl_name>
 *      struct { <definition> } <decl_name>
 * @endcode
 * In the first statement, the struct is defined and declared in one statement. In the second statement, an anonymous struct is
 * defined and declared in one statement.
 */
class CDefinitionEntity : public CEntity, public sdv::idl::IDefinitionEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    CDefinitionEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent);

protected:
    /**
     * @brief Root entity constructor (name is 'root' and no parent).
     * @param[in] rParser Reference to the parser.
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     */
    CDefinitionEntity(CParser& rParser, const CContextPtr& rptrContext);
public:

    /**
     * @brief Destructor
     */
    virtual ~CDefinitionEntity() override = default;

    /**
    * @brief Get access to another interface. Overload of IInterfaceAccess::GetInterface.
    * @param[in] idInterface The interface id to get access to.
    * @return Returns a pointer to the interface or NULL when the interface is not supported.
    */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Process the content of the definition.
     * @details Process the content of the definition. This function parses through the content until a closing curly bracket
     * has been detected. First the function checks for prefixes. Then the function determines whether the statement is a
     * declaration or a definition. It then creates the corresponding entity and let the entity process itself.
     */
    virtual void ProcessContent();

    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Process the definition addendum.
     * @details Process the definition addendum following the definition statement before the content definition. The default
     * implementation checks for an inheritance list.
     */
    virtual void ProcessDefinitionAddendum();

    /**
     * @brief Create a values for inherited child nodes.
     */
    void CreateInheritanceValueChildNodes();

    /**
     * \brief Does the entity have an Unnamed definition. Overload of IDefinitionEntity::Unnamed.
     * @return Returns 'true' when the definition supports unnamed definition; 'false' otherwise.
     */
    virtual bool IsUnnamed() const override { return m_bAnonymousDefinition; };

    /**
     * @brief Request whether the definition supports the content. This function must be implemented by the derived entity.
     * @param[in] eSupport The type of support that is requested.
     * @return Returns 'true' when the definition supports the content; 'false' otherwise.
     */
    virtual bool Supports(EDefinitionSupport eSupport) const = 0;

    /**
     * @brief Does the entity support children? Overload of CEntity::SupportsChildren.
     * @details Complex types support children per default.
     * @return Returns whether the entity supports children (which is the case).
     */
    virtual bool SupportsChildren() const override { return true; }

    /**
     * @brief Does the entity support inheritance?
     * @details The default implementation is that inheritance is not supported.
     * @return Returns whether the entity supports inheritance (which is not the case).
     */
    virtual bool SupportsInheritance() const { return false; }

    /**
     * @brief Does the entity support anonymous naming?
     * @details The default implementation is that anonymous naming is not supported.
     * @return Returns whether the entity supports inheritance (which is not the case).
     */
    virtual bool SupportsAnonymous() const { return false; }

    /**
     * @brief Does the complex entity support attributes in its content?
     * @details The default implementation doesn't support attributes (they are specific to interfaces).
     * @return Returns whether the entity supports attributes (which is not the case).
     */
    virtual bool SupportContentAttributes() const { return false; }

    /**
     * @brief Does the complex entity support operations in its content?
     * @details The default implementation doesn't support operations (they are specific to interfaces).
     * @return Returns whether the entity supports operations (which is not the case).
     */
    virtual bool SupportContentOperations() const { return false; }

    /**
     * @brief Does the definition entity require a content definition?
     * @details In certain cases, it is required that the content definition is following the definition statement. For example,
     * when an inheritance list is provided. The default implementation checks the m_bRequiresContent variable.
     * @return Returns whether the content definition should be defined following the definition statement.
     */
    virtual bool RequireContentDefinition() const { return m_bRequiresContent; }

    /**
     * @brief Does the definition require a declaration?
     * @details In certain cases, it is required that the definition is followed by a declaration. For example,
     * when the definition was made anonymously or when the definition is dependent on a variable within the same struct (e.g. with
     * unions).
     * @return Returns whether the definition requires a declaration.
     */
    virtual bool RequireDeclaration() const { return IsUnnamed(); }

    /**
     * @brief Does the definition allow automatic transparent declaration if not present?
     * @details When set an automatic transparent declaration is allowed without an explicit variable declaration. Currently this
     * is only the case with unions with a variable based switch type.
     * @return Returns whether the definition allows an automatic transparent declaration.
     */
    virtual bool AllowAutoTransparentDeclaration() const { return false; }

    /**
     * @brief Get child entity iterator if children are available and supported by the definition. Overload of
     * sdv::idl::IDefinitionEntity::GetChildren.
     * @return Returns a pointer to the child entity iterator or NULL when not available.
     */
    virtual sdv::idl::IEntityIterator* GetChildren() override;

    /**
     * @brief Get inheritance entity iterator if the definition was inheriting from other entities. Overload of
     * sdv::idl::IDefinitionEntity::GetInheritance.
     * @return Returns a pointer to the inheritance entity iterator or NULL when not available.
     */
    virtual sdv::idl::IEntityIterator* GetInheritance() override;

    /**
     * @brief Calculate the hash of this entity and all encapsulated entities. Overload of CBaseEntity::CalcHash.
     * @param[in, out] rHash Hash object to be filled with data.
     */
    virtual void CalcHash(CHashObject& rHash) const override;

    /**
     * @brief Get the declaration members.
     * @return List of declaration members.
     */
    const CEntityList GetDeclMembers() const;

protected:
    /**
     * @brief Add the child to the children list. Called by CreateChild function. Overload of CEntity::AddChild.
     * @param[in] ptrChild Pointer to the child entity to add.
     */
    virtual void AddChild(CEntityPtr ptrChild) override;

    /**
     * @brief Find the entity locally by looking in the entity children map and the entity children maps of all the chained
     * entities. The search is done case-insensitive. Overload of CEntity::FindLocal.
     * @param[in] rssName Reference to the string object containing the name of the entity to search for.
     * @param[in] bDeclaration When set, the name belongs to a declaration; otherwise it belongs to a definition. Needed to allow
     * the reuse of names between declarations and definitions.
     * @return Returns a pair object containing an entity pointer if the entity exists or a NULL pointer if not as well as a
     * boolean that indicates that the entity was from an inherited entity.
     */
    virtual std::pair<CEntityPtr, bool> FindLocal(const std::string& rssName, bool bDeclaration) const override;

    /**
     * @brief Create the content value node. Overridable when supporting content values.
     * @details When supporting value assignments, create the value node and assign the value node to the ValueRef() reference. For
     * definitions that do not support value assignments, do nothing (default).
     */
    virtual void CreateValueNode() {};

protected:
    CEntityVector           m_vecInheritance;               ///< List of base entities in the order of appearance.
    CEntityList             m_lstTypeMembers;               ///< List of typedef member declarations and type definitions.
    CEntityList             m_lstConstMembers;              ///< List of const member declarations.
    CEntityList             m_lstDeclMembers;               ///< List of variable member declarations.
    CEntityList             m_lstAttributesOperation;       ///< List with attributes and operations.
    bool                    m_bRequiresContent = false;     ///< When set, the definition statement requires content to follow.
    bool                    m_bAnonymousDefinition = false; ///< When set, the entity has an anonymous generated name.
    CEntityIterator         m_iteratorChildren;             ///< Children iterator
    CEntityIterator         m_iteratorInheritance;          ///< Inheritance iterator
};


#endif // !defined(DEFINITION_ENTITY_H)
