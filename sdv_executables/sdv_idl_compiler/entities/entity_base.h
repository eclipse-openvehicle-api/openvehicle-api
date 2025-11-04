#ifndef ENTITY_BASE_H
#define ENTITY_BASE_H

#include "../includes.h"
#include "../token.h"
#include "../tokenlist.h"
#include "../exception.h"
#include <memory>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <type_traits>
#include "hash_calc.h"

#define DONT_LOAD_CORE_TYPES
#include <support/interface_ptr.h>

// Forward declaration
class CEntity;
class CParser;
class CEntityValueNode;
class CTypedefEntity;
class CContext;

/**
 * @brief Entity smart pointer.
 */
using CEntityPtr = std::shared_ptr<CEntity>;

/**
 * @brief Map with entities sorted by name.
 * @remarks The key of the entity should be stored in lower case to allow case insensitive searching.
 */
using CEntityMap = std::map<std::string, CEntityPtr>;

/**
 * @brief List with entities in the order they appear in code.
 */
using CEntityList = std::list<CEntityPtr>;

/**
* @brief Vector with entities in the order they appear in code.
*/
using CEntityVector = std::vector<CEntityPtr>;

/**
 * @brief Entity value shared pointer.
 */
using CValueNodePtr = std::shared_ptr<CEntityValueNode>;

/**
 * @brief Entity value vector.
 */
using CEntityValueVector = std::vector<CValueNodePtr>;

/**
 * @brief Is declaration type an integral type?
 * @param[in] eType The declaration type.
 * @return Returns whether the declaration type is an integral type.
 */
inline bool IsIntegralDeclType(sdv::idl::EDeclType eType)
{
    switch (eType)
    {
    case sdv::idl::EDeclType::decltype_short:
    case sdv::idl::EDeclType::decltype_long:
    case sdv::idl::EDeclType::decltype_long_long:
    case sdv::idl::EDeclType::decltype_unsigned_short:
    case sdv::idl::EDeclType::decltype_unsigned_long:
    case sdv::idl::EDeclType::decltype_unsigned_long_long:
    case sdv::idl::EDeclType::decltype_char:
    case sdv::idl::EDeclType::decltype_char16:
    case sdv::idl::EDeclType::decltype_char32:
    case sdv::idl::EDeclType::decltype_wchar:
    case sdv::idl::EDeclType::decltype_boolean:
    case sdv::idl::EDeclType::decltype_native:
    case sdv::idl::EDeclType::decltype_octet:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Is declaration type signed?
 * @param[in] eType The declaration type.
 * @return Returns whether the declaration type is signed.
 */
inline bool IsSignedDeclType(sdv::idl::EDeclType eType)
{
    switch (eType)
    {
    case sdv::idl::EDeclType::decltype_short:
    case sdv::idl::EDeclType::decltype_long:
    case sdv::idl::EDeclType::decltype_long_long:
    case sdv::idl::EDeclType::decltype_char:
    case sdv::idl::EDeclType::decltype_float:
    case sdv::idl::EDeclType::decltype_double:
    case sdv::idl::EDeclType::decltype_long_double:
    case sdv::idl::EDeclType::decltype_fixed:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Is declaration type unsigned?
 * @param[in] eType The declaration type.
 * @return Returns whether the declaration type is unsigned.
 */
inline bool IsUnsignedDeclType(sdv::idl::EDeclType eType)
{
    switch (eType)
    {
    case sdv::idl::EDeclType::decltype_unsigned_short:
    case sdv::idl::EDeclType::decltype_unsigned_long:
    case sdv::idl::EDeclType::decltype_unsigned_long_long:
    case sdv::idl::EDeclType::decltype_char16:
    case sdv::idl::EDeclType::decltype_char32:
    case sdv::idl::EDeclType::decltype_wchar:
    case sdv::idl::EDeclType::decltype_boolean:
    case sdv::idl::EDeclType::decltype_octet:
        return true;
    case sdv::idl::EDeclType::decltype_native:
        return std::is_unsigned_v<size_t>;
    default:
        return false;
    }
}

/**
 * @brief Declaration type name association.
 */
using TDeclTypeAssoc = std::pair<std::string, sdv::idl::EDeclType>;

/**
* @brief Mapping between declaration type and string name.
* @attention Some types have multiple string representatives (e.g. sdv::idl::EDeclType::decltype_short vs. 'short' and
* 'int16').
* @attention Based on the provided extensions on the command line, the types might be extended by: char16, char32, u8string,
* u16string, u32string, pointer, interface_id, interface_t and exception_id
*/
const std::vector<TDeclTypeAssoc> g_vecDeclTypes = {
    {"short", sdv::idl::EDeclType::decltype_short},
    {"unsigned short", sdv::idl::EDeclType::decltype_unsigned_short},
    {"long", sdv::idl::EDeclType::decltype_long},
    {"unsigned long", sdv::idl::EDeclType::decltype_unsigned_long},
    {"long long", sdv::idl::EDeclType::decltype_long_long},
    {"unsigned long long", sdv::idl::EDeclType::decltype_unsigned_long_long},
    {"fixed", sdv::idl::EDeclType::decltype_fixed},
    {"float", sdv::idl::EDeclType::decltype_float},
    {"double", sdv::idl::EDeclType::decltype_double},
    {"long double", sdv::idl::EDeclType::decltype_long_double},
    {"char", sdv::idl::EDeclType::decltype_char},
    {"wchar", sdv::idl::EDeclType::decltype_wchar},
    {"int8", sdv::idl::EDeclType::decltype_char},
    {"int16", sdv::idl::EDeclType::decltype_short},
    {"int32", sdv::idl::EDeclType::decltype_long},
    {"int64", sdv::idl::EDeclType::decltype_long_long},
    {"int", sdv::idl::EDeclType::decltype_long},
    {"uint8", sdv::idl::EDeclType::decltype_octet},
    {"uint16", sdv::idl::EDeclType::decltype_unsigned_short},
    {"uint32", sdv::idl::EDeclType::decltype_unsigned_long},
    {"uint64", sdv::idl::EDeclType::decltype_unsigned_long_long},
    {"uint", sdv::idl::EDeclType::decltype_unsigned_long},
    {"boolean", sdv::idl::EDeclType::decltype_boolean},
    {"native", sdv::idl::EDeclType::decltype_native},
    {"octet", sdv::idl::EDeclType::decltype_octet},
    {"byte", sdv::idl::EDeclType::decltype_octet},
    {"string", sdv::idl::EDeclType::decltype_string},
    {"wstring", sdv::idl::EDeclType::decltype_wstring},
    {"enum", sdv::idl::EDeclType::decltype_enum},
    {"struct", sdv::idl::EDeclType::decltype_struct},
    {"union", sdv::idl::EDeclType::decltype_union},
    {"operation", sdv::idl::EDeclType::decltype_operation},
    {"attribute", sdv::idl::EDeclType::decltype_parameter},
    {"enum_entry", sdv::idl::EDeclType::decltype_enum_entry},
    {"case_entry", sdv::idl::EDeclType::decltype_case_entry},
    {"typedef", sdv::idl::EDeclType::decltype_typedef},
    {"sequence", sdv::idl::EDeclType::decltype_sequence},
    {"map", sdv::idl::EDeclType::decltype_map},
    {"bitset", sdv::idl::EDeclType::decltype_bitset},
    {"bitfield", sdv::idl::EDeclType::decltype_bitfield},
    {"bitmask", sdv::idl::EDeclType::decltype_bitmask},
    {"any", sdv::idl::EDeclType::decltype_any},
    {"void", sdv::idl::EDeclType::decltype_void}
};

/**
 * @brief Type declaration struct containing all the type information of this declaration including dependent type information
 * of templated types and information.
 */
class CTypeDeclaration : public sdv::idl::IDeclarationType, public sdv::IInterfaceAccess
{
public:
	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(sdv::IInterfaceAccess)
		SDV_INTERFACE_ENTRY(sdv::idl::IDeclarationType)
	END_SDV_INTERFACE_MAP()

    /**
     * @brief Return the base type. Overload of sdv::idl::IDeclarationType::GetBaseType.
     * @details The bse type might be a templated type (string, sequence, map, etc.) which means that additional
     * information is needed. Furthermore, the type might be a complex type (struct, union, enum) or a typedef. In these
     * cases the type definition interface is available.
     * @return The base type of this type.
     */
    virtual sdv::idl::EDeclType GetBaseType() const override;

    /**
     * @brief Set the base type.
     * @param[in] eBaseType The base type to set.
     */
	void SetBaseType(sdv::idl::EDeclType eBaseType);

    /**
     * @brief Return the string that described the type in the code. Overload of
     * sdv::idl::IDeclarationType::GetTypeString.
     * @return The type string.
     */
    virtual sdv::u8string GetTypeString() const override;

    /**
	 * @brief Set the type string.
	 * @param[in] rssType Reference to the type string.
	 */
	void SetTypeString(const sdv::u8string& rssType);

    /**
	 * @brief Add a string chunk to the type string.
	 * @param[in] rss Reference to the chunk string to add.
	 */
	void AddTypeString(const sdv::u8string& rss);

	/**
     * @brief Return the type definition for complex types or typedefs.
     * Overload of sdv::idl::IDeclarationType::GetTypeDefinition.
     * @return Pointer to the interface representing the type definition. Will be NULL for all other types.
     */
    virtual sdv::IInterfaceAccess* GetTypeDefinition() const override;

    /**
     * @brief Return the type definition entity pointer.
     * @return The type definition entity pointer.
     */
	CEntityPtr GetTypeDefinitionEntityPtr() const;

    /**
     * @brief Set the definition entity pointer.
     * @param[in] rptrDefinition Reference to the definition entity pointer.
     */
	void SetTypeDefinitionEntityPtr(const CEntityPtr& rptrDefinition);

    /**
     * @brief Fixed length parameter for some templated types. Overload of sdv::idl::IDeclarationType::GetFixedLength.
     * @details Fixed length template parameter for "fixed", "string", "sequence", "pointer", "map" and "bitfields".
     * When not compulsory (for "fixed", "string", "sequence", "pointer" and "map") could be 0 to indicate a dynamic
     * length (or defined over assignment with "fixed"). Bitfields allow a length between 1 and 64 bits.
     * @return Returns the fixed length for some templated types or 0 for all other other types.
     */
    virtual uint32_t GetFixedLength() const override;

    /**
     * @brief Set the fixed length.
     * @param[in] uiFixedLength The fixed length or 0 for dynamic length.
     */
    void SetFixedLength(uint32_t uiFixedLength);

    /**
     * @brief The amount of decimals of the "fixed" data type. Must be equal or smaller than the fixed length. Overload of
     * sdv::idl::IDeclarationType::GetDecimals.
     * @return The amount of decimals for the "fixed" data type or 0 for all other data types.
     */
    virtual uint32_t GetDecimals() const override;

    /**
     * @brief Set the amount of decimals for the fixed type.
     * @param[in] uiDecimals The amount of decimals to set.
     */
	void SetDecimals(uint32_t uiDecimals);

    /**
     * @brief The value type template parameter for the "sequence", "pointer", "map" and "bitfield" data types. Overload of
     * sdv::idl::IDeclarationType::GetValueType.
     * @return Interface to the value data type for some templated types of NULL for all other types.
     */
    virtual sdv::IInterfaceAccess* GetValueType() const override;

    /**
     * @brief Set value type pointer.
     * @param[in] rptrValueType Reference to the value type pointer.
     */
	void SetValueTypePtr(const std::shared_ptr<CTypeDeclaration>& rptrValueType);

    /**
     * @brief The key type template parameter for the "map" data type. Overload of
     * sdv::idl::IDeclarationType::GetKeyType.
     * @return Interface to the key data type for the "map" type of NULL for all other types.
     */
    virtual sdv::IInterfaceAccess* GetKeyType() const override;

    /**
	 * @brief Set key type pointer.
	 * @param[in] rptrKeyType Reference to the key type pointer.
	 */
	void SetKeyTypePtr(const std::shared_ptr<CTypeDeclaration>& rptrKeyType);

private:
    /// The base type of the identifier.
    sdv::idl::EDeclType m_eBaseType = sdv::idl::EDeclType::decltype_unknown;
    std::string m_ssOriginalType;                     ///< The type string that resulted in the base type. This could be a system
                                                      ///< type as well as a scoped name of an enum, struct, union or typedef.
    CEntityPtr m_ptrOriginalType;                     ///< The type entity if the type is defined through an entity.
    uint32_t m_uiFixedLen = 0;                        ///< Fixed length template parameter for "fixed", "string", "sequence",
                                                      ///< "pointer", "map" and "bitfields". When not compulsory (for "fixed",
                                                      ///< "string", "sequence", "pointer" and "map") could be 0 to indicate a
                                                      ///< dynamic length (or defined
                                                      ///< over assignment with "fixed"). Bitfields allow a length between 1 and
                                                      ///< 64 bits.
    uint32_t m_uiDecimals = 0;                        ///< The amount of decimals of the "fixed" data type. Must be equal or
                                                      ///< smaller than the fixed length.
    std::shared_ptr<CTypeDeclaration> m_ptrValueType; ///< The value type template parameter for "sequence", "pointer", "map" and
                                                      ///< "bitfield".
    std::shared_ptr<CTypeDeclaration> m_ptrKeyType;   ///< The key type template parameter for map.
};

// TODO:
// - Const complex entities need to be told, that they are const during assignment.
// - Deal with the case of forward declaration (why is this needed, there are no references and no pointers)?
// - Deal with the case of ambiguity through inheritance:
//     struct S1{ const int32 i1 = 10; };
//     struct S2{ const int32 i1 = 20; };
//     struct S3 : S1, S2 { int32 j1[i1]; }; --> ERROR
// - Not allowed to assign dynamic values when the parent value is a const value.

/**
 * @brief Base of the entities allowing generic access to the entity.
 * @remarks Normally a declaration entity cannot have child entities. It would make sense to manage child entities only by the
 * definition entity. OMG IDL imposes naming restrictions for all the entities. And forward declarations are allowed. This
 * requires that child management is implemented on a base level, even though declarations will not have any children.
 */
class CEntity : public sdv::idl::IEntityInfo, public sdv::idl::IEntityComments, public sdv::idl::IEntityContext,
    public sdv::idl::IForwardDeclarationEntity, public sdv::IInterfaceAccess, public std::enable_shared_from_this<CEntity>
{
    friend CTypedefEntity;
public:
    /**
     * @brief Constructor using the parser to process the code.
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    CEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent);

    /**
     * @brief Constructor using the provided token-list to process the code.
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     * @param[in] rlstTokenList Reference to the token list holding the tokens to process.
     */
    CEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, const CTokenList& rlstTokenList);

protected:
    /**
     * @brief Root entity constructor (name is 'root' and no parent).
     * @param[in] rParser Reference to the parser.
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     */
    CEntity(CParser& rParser, const CContextPtr& rptrContext);
public:

    /**
     * @brief Destructor.
     */
    virtual ~CEntity() = default;

    /**
     * @brief Get access to another interface. Overload of sdv::IInterfaceAccess::GetInterface.
     * @param[in] idInterface The interface id to get access to.
     * @return Returns a pointer to the interface or NULL when the interface is not supported.
     */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Get the type of the entity. Default type is sdv::idl::EEntityType::type_unknown. Overload of
     * sdv::idl::IEntityInfo::GetType.
     * @return Returns the entity type.
     */
    virtual sdv::idl::EEntityType GetType() const override;

    /**
     * @brief Get the entity if this is a forward declared entity. Overload of sdv::idl::IForwardDeclarationEntity.
     * @return Pointer to the entity this forward declaration is referring to. Or NULL when no entity was defined.
     */
    virtual sdv::IInterfaceAccess* GetEntity() override;

    /**
     * @brief Get the declaration type of the entity as string.
     * @attention To get the qualified type including array sizes, use the GetDeclTypeStr of the CEntityValueNode class.
     * @details For declaration entities, the type returned includes the declaration type. For definition entities, the type returned
     * have the for of "<base type> <type identifier>".
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns the entity declaration type string.
     */
   virtual std::string GetDeclTypeStr(bool bResolveTypedef) const;

    /**
     * @{
     * @brief Get the derived entity class.
     * @tparam TEntity The entity class to request a pointer for.
     * @return Returns a pointer to the derived class or NULL when the requested class didn't derive from this entity.
     */
    template <class TEntity>
    TEntity* Get();
    template <class TEntity>
    const TEntity* Get() const;
    /**
     * @}
     */

    /**
     * @brief Process the code. Entity specific processing function. Must be derived.
     * @details The processing of follows the following stages:
     *  - Process definition - determine the type blue-print
     *  - Process declaration - determine the identifier
     *  - Process assignment - build value chains
     * It is not required to process all three steps. A definition can be made without a following declaration. A declaration can
     * be made based on a previous definition.
     * Whether or not a definition is made, is determined from the token sequence (grammar). In principal, if the statements are
     * not specifying a definition, they are specifying a declaration or both (a declaration following the definition). Processing
     * assignments is triggered by the complex processing entities (modules, structs, unions, interfaces, enums). In modules and
     * enums, a declaration statement is processed for assignents directly. In structs and unions, the use of member variables is
     * allowed even if they are defined later and have not been processed yet. Therefore, the declarations of all members in the
     * structs and unions are processed first and the assignments are following after this.
     * Assignments are only possible following a declaration. Therefore, the process assignment function is only implemented in
     * the CDeclarationEntity class implementing typedefs, const declarations and variable declarations. The assignment of values is
     * possible also for enums, structs, unions and interfaces. The overloadable ProcessAsignmentValue function is used for this.
     */
    virtual void Process() = 0;

    /**
     * @brief Get the name of the entity. Overload of IEntityInfo::GetName.
     * @return The name string.
     */
    virtual sdv::u8string GetName() const override;

    /**
     * @brief Get the scoped name of the entity (including the modules separated by the scope separator). Overload of
     * IEntityInfo::GetScopedName.
     * @return The scoped name build from the entities of the parents separated by '::'. An empty name means global scope.
     */
    virtual sdv::u8string GetScopedName() const override;

    /**
     * @brief Replace any identifier in the expression with a relative scope to this entity by a full scoped identifier.
     * @remarks If an entity in the expression cannot be found within the current scope, the entity is not corrected and replaced
     * without change in the retargeted expression.
     * @param[in] rssExpression Reference to the expression to use for retargeting.
     * @return The expression with full scoped identifiers.
     */
    std::string MakeFullScoped(const std::string& rssExpression) const;

    /**
     * @brief Has equal.
     * @param[in] ptrEntity Pointer to the entity to use for the comparison.
     * @return Returns 'true' when the entities have the identical scoped names.
     */
    bool IsEqual(CEntityPtr ptrEntity) { return GetScopedName() == ptrEntity->GetScopedName(); }

    /**
    * @{
    * @brief Get the parse context.
    * @return Returns the parse context.
    */
    CContextPtr GetContext() const;
    CContextPtr GetContext();
    /**
    * @}
    */

    /**
     * @{
     * @brief Get the parent entity of this entity.
     * @return Returns the smart pointer to the parent entity or NULL when this is the root entity.
     */
    CEntityPtr GetParentEntity() const;
    CEntityPtr GetParentEntity();
    /**
     * @}
     */

     /**
      * @brief Get the parent entity of this entity. Overload of IEntityInfo::GetParent.
      * @return Returns an interface to the parent entity or NULL when this is the root parent (when there is no parent).
      */
    virtual sdv::IInterfaceAccess* GetParent() const override;

    /**
     * @{
     * @brief Get the root entity.
     * @return Returns the root entity.
     */
    CEntityPtr GetRootEntity() const;
    CEntityPtr GetRootEntity();
    /**
     * @}
     */

    /**
     * @{
     * @brief Get resolved entity if the entity is a typedef (or multiple typedefs).
     * @return Returns resolved entity.
     */
    CEntityPtr GetResolvedEntity() const;
    CEntityPtr GetResolvedEntity();
    /**
     * @}
     */

    /**
     * @brief Set the comment tokens for this entity.
     * @attention This function doesn't set any comment if there is already a comment assigned (either through a forward
     * declaration or when a comment is provided beforee and after a declaration). Unless, the new comments have additional
     * information on the comment format (brief, java doc or doxygen) and the initial comments didn't have this.
     * @param[in] rlstComments Reference to the comments token list.
     * @param[in] bPreceeding When set, the comments are preceding the next statement; otherwise they are succeeding.
     */
    void SetCommentTokens(const CTokenList& rlstComments, bool bPreceeding = true);

    /**
     * @brief Get the preceding comment token list.
     * @details Preceding comments must be located in the same source file as the next token and must be adjacent and end at the
     * latest one line before the next token.
     * @return Returns the preceding comment token list.
     */
    CTokenList GetPreCommentTokenList();

    /**
     * @brief Process the succeeding comment token list and assign to the current entity.
     * @details Succeeding comments must be located in the same source file as the entity and must be adjacent and start at the
     * provided line.
     * @param[in] uiLine The line to start checking for succeeding comments at.
     */
    void ProcessPostCommentTokenList(uint32_t uiLine);

    /**
     * @brief Get the comment lines for this entity as one string. Overload of IEntityComments::GetComments.
     * @remarks For c-style multi-line comments, the indentation and additional asterisk character at the beginning of each line
     * is removed.
     * @param[out] ruiFlags Reference to the variable receiving the comment flags (a bitmask combination of
     * sdv::idl::IEntityComments::ECommentMask).
     * @return Returns the comment string. If the comment contains multiple lines, each line is ending with a newline.
     */
    virtual sdv::u8string GetComments(uint32_t& ruiFlags) const override;

    /**
     * @brief Get the location. Overload of sdv::idl::IEntityContext::GetLocation.
     * @return Returns the location.
     */
    virtual sdv::idl::IEntityContext::ELocation GetLocation() const override;

    /**
     * @brief Get the path to the file. Overload of sdv::idl::IEntityContext::GetSourcePath.
     * @return Returns the source path string.
     */
    virtual sdv::u8string GetSourcePath() const override;

    /**
     * @brief Get the position in the file. Overload of sdv::idl::IEntityContext::GetPosition.
     * @remarks Not all entities have a position. If no position is available, the position return value has the value 0.
     * @param[out] ruiLineBegin Reference to the variable receiving the line number of the entity beginning.
     * @param[out] ruiColBegin Reference to the variable receiving the column number of the entity beginning.
     * @param[out] ruiLineEnd Reference to the variable receiving the line number of the entity ending.
     * @param[out] ruiColEnd Reference to the variable receiving the column number of the entity ending.
     */
    virtual void GetPosition(uint32_t& ruiLineBegin, uint32_t& ruiColBegin, uint32_t& ruiLineEnd, uint32_t& ruiColEnd) override;

    /**
     * @{
     * @brief Set the position during parsing.
     * @param[in] uiLine The line number.
     * @param[in] uiCol The column number.
     */
    void SetBeginPosition(uint32_t uiLine, uint32_t uiCol) { m_uiLineBegin = uiLine; m_uiColBegin = uiCol; }
    void SetEndPosition(uint32_t uiLine, uint32_t uiCol) { m_uiLineEnd = uiLine; m_uiColEnd = uiCol; }
    /**
     * @}
     */

    /**
     * @brief Calculate the hash of this entity and all encapsulated entities.
     * @param[in, out] rHash Hash object to be filled with data.
     */
    virtual void CalcHash(CHashObject& rHash) const;

    /**
     * @{
     * @brief Get the parser.
     * @return Returns a reference to the parser.
     */
    const CParser& GetParserRef() const;
    CParser& GetParserRef();
    /**
     * @}
     */

protected:
    /**
     * @brief Get a token from the parser with moving the current position.
     * @return Returns the token.
     */
    CToken GetToken();

    /**
    * @brief Get the last valid token.
     * @return Returns the last read token or an empty token when no token was read before.
    */
    CToken GetLastValidToken() const;

    /**
     * @brief Get a token from the parser without moving the current position.
     * @param[in] nIndex The amount of tokens to skip before returning the token.
     * @return Returns the token.
     */
    CToken PeekToken(size_t nIndex = 0);

    /**
     * @brief Insert token for parsing at the front of the parsing list.
     * @param[in] rToken Reference to token object containing the code to insert.
     */
    void PrependToken(CToken& rToken);

    /**
     * @brief Create a child entity and optionally add it to the list of children.
     * @tparam TEntity The entity class to create. This class must derive from CEntity.
     * @tparam TParams Zero or more parameter types to use for the constructor of the entity class.
     * @remarks The parameters defined by the parameter type TParams must correspond to the parameters needed for the constructor
     * of TEntity minus the compulsory ptrParent parameter.
     * @remarks The parse context of the child is the same as the parent.
     * @param[in] rptrContext Reference to the context.
     * @param[in] pParent Pointer to the parent entity.
     * @param[in] tParams Zero or more parameters to use for the constructor of the entity class.
     * @return Returns a smart pointer to the entity.
     */
    template <class TEntity, typename... TParams>
    static CEntityPtr CreateChild(const CContextPtr& rptrContext, CEntity* pParent, TParams... tParams);

public:
    /**
     * @brief Find the entity using a scoped name.
     * @details A scoped name defines an entity name with possible entity parent names. Each name is separated by the scope
     * separator '::'. For example: "parent_1::parent_1_a::parent_1_a_I::entity". The first entity that will be searched for is
     * the uppermost parent (here parent_1). The search starts at the current entity. If the search was not successful, the search
     * is done using the parent and if the search also there was not successful, the parent of the parent is searched for, and so
     * on. The search is successful if all the requested parents match and the entity is found. An exception to the rule of the
     * scoped name is, when the name starts with the scope separator indicating that the search is not relative but should start
     * at the root entity. For example: "::parent::entity".
     * @tparam TEntity The entity type to return.
     * @param[in] rssScopedName Reference to a string object containing the relative or absolute scoped entity name consisting of
     * zero or more parents definitions and one entity name all separated with the scope separator '::' followed by declaration
     * members separated by a member separator '.'. The name must not be empty.
     * @param[in] bCheckParent When set, check the parent(s) as well if the scoped name fits on one of the children of a parent;
     * otherwise the parent(s) is(are) not taken into account.
     * @return The first version returns an entity smart pointer. The second version returns the entity pointer converted to the
     * requested entity type (if the entity is of this type). If the entity doesn't exists or is of the wrong type a NULL pointer
     * is returned.
     */
    CEntityPtr Find(const std::string& rssScopedName, bool bCheckParent = true) const;

    /**
     * @brief Find the entity using a scoped name.
     * @details A scoped name defines an entity name with possible entity parent names. Each name is separated by the scope
     * separator '::'. For example: "parent_1::parent_1_a::parent_1_a_I::entity". The first entity that will be searched for is
     * the uppermost parent (here parent_1). The search starts at the current entity. If the search was not successful, the search
     * is done using the parent and if the search also there was not successful, the parent of the parent is searched for, and so
     * on. The search is successful if all the requested parents match and the entity is found. An exception to the rule of the
     * scoped name is, when the name starts with the scope separator indicating that the search is not relative but should start
     * at the root entity. For example: "::parent::entity".
     * @tparam TEntity The entity type to return.
     * @param[in] rssScopedName Reference to a string object containing the relative or absolute scoped entity name consisting of
     * zero or more parents definitions and one entity name all separated with the scope separator '::' followed by declaration
     * members separated by a member separator '.'. The name must not be empty.
     * @return The first version returns an entity smart pointer. The second version returns the entity pointer converted to the
     * requested entity type (if the entity is of this type). If the entity doesn't exists or is of the wrong type a NULL pointer
     * is returned.
     */
    template <typename TEntity>
    TEntity* Find(const std::string& rssScopedName) const;

    /**
     * @brief Find the value belonging to the scoped name.
     * @details A scoped name defines an entity name with possible entity parent names. Each name is separated by the scope
     * separator '::'. For example: "parent_1::parent_1_a::parent_1_a_I::entity". The first entity that will be searched for is
     * the uppormost parent (here parent_1). The search starts at the current entity. If the search was not successful, the search
     * is done using the parent and if the search also there was not successful, the parent of the parent is searched for, and so
     * on. The search is successful if all the requested parents match and the entity is found. An exception to the rule of the
     * scoped name is, when the name starts with the scope separator indicating that the search is not relative but should start
     * at the root entity. For example: "::parent::entity".
     * Following the scoped name, members and indices can be specified to navigate to the correct value declaration. Members are
     * separated by a dot '.' and indices are specified using square brackets '[...]'. For example "::parent::entity.first[1]".
     * @param[in] rlstScopedNameInclMembers Token list consisting of tokens representing the scoped name as well as any array and
     * member names.
     * @return Returns a value node smart pointer. If the value node doesn't exists or is of the wrong type a NULL pointer is
     * returned.
     */
    CValueNodePtr FindValue(const CTokenList& rlstScopedNameInclMembers) const;

    /**
    * @brief Find the value belonging to the scoped name.
    * @details A scoped name defines an entity name with possible entity parent names. Each name is separated by the scope
    * separator '::'. For example: "parent_1::parent_1_a::parent_1_a_I::entity". The first entity that will be searched for is
    * the uppormost parent (here parent_1). The search starts at the current entity. If the search was not successful, the search
    * is done using the parent and if the search also there was not successful, the parent of the parent is searched for, and so
    * on. The search is successful if all the requested parents match and the entity is found. An exception to the rule of the
    * scoped name is, when the name starts with the scope separator indicating that the search is not relative but should start
    * at the root entity. For example: "::parent::entity".
    * Following the scoped name, members and indices can be specified to navigate to the correct value declaration. Members are
    * separated by a dot '.' and indices are specified using square brackets '[...]'. For example "::parent::entity.first[1]".
    * @param[in] rssScopedNameInclMembers Reference to a string object containing the relative or absolute scoped entity name
    * consisting of zero or more parents and one entity name all separated with the scope separator '::'. The name must not be
    * empty.
    * @return Returns a value node smart pointer. If the value node doesn't exists or is of the wrong type a NULL pointer is
    * returned.
    */
    CValueNodePtr FindValue(const std::string& rssScopedNameInclMembers) const;

    /**
    * @brief Find the value belonging to the scoped name.
    * @details A scoped name defines an entity name with possible entity parent names. Each name is separated by the scope
    * separator '::'. For example: "parent_1::parent_1_a::parent_1_a_I::entity". The first entity that will be searched for is
    * the uppormost parent (here parent_1). The search starts at the current entity. If the search was not successful, the search
    * is done using the parent and if the search also there was not successful, the parent of the parent is searched for, and so
    * on. The search is successful if all the requested parents match and the entity is found. An exception to the rule of the
    * scoped name is, when the name starts with the scope separator indicating that the search is not relative but should start
    * at the root entity. For example: "::parent::entity".
    * Following the scoped name, members and indices can be specified to navigate to the correct value declaration. Members are
    * separated by a dot '.' and indices are specified using square brackets '[...]'. For example "::parent::entity.first[1]".
    * @tparam TValueNode Type of the value node to return.
    * @param[in] rssScopedNameInclMembers Reference to a string object containing the relative or absolute scoped entity name
    * consisting of zero or more parents and one entity name all separated with the scope separator '::'. The name must not be
    * empty.
    * @return Returns the value node pointer converted to the requested value node type (if the value node is of this type). If the
    * value node doesn't exists or is of the wrong type a NULL pointer is returned.
    */
    template <typename TValueNode>
    TValueNode* FindValue(const std::string& rssScopedNameInclMembers) const;

    /**
     * @brief Find the value variant belonging to the scoped name.
     * @details A scoped name defines an entity name with possible entity parent names. Each name is separated by the scope
     * separator '::'. For example: "parent_1::parent_1_a::parent_1_a_I::entity". The first entity that will be searched for is
     * the uppormost parent (here parent_1). The search starts at the current entity. If the search was not successful, the search
     * is done using the parent and if the search also there was not successful, the parent of the parent is searched for, and so
     * on. The search is successful if all the requested parents match and the entity is found. An exception to the rule of the
     * scoped name is, when the name starts with the scope separator indicating that the search is not relative but should start
     * at the root entity. For example: "::parent::entity".
     * Following the scoped name, members and indices can be specified to navigate to the correct value declaration. Members are
     * separated by a dot '.' and indices are specified using square brackets '[...]'. For example "::parent::entity.first[1]".
     * @param[in] rssScopedNameInclMembers Reference to a string object containing the relative or absolute scoped entity name
     * consisting of zero or more parents and one entity name all separated with the scope separator '::'. The name must not be
     * empty.
     * @return On success, return a const variant structure with the value belonging to the requested value node. On failure, a
     * const variant is returned that is initialized with 0 using the boolean data type (the first type and first value).
     */
    CConstVariant FindValueVariant(const std::string& rssScopedNameInclMembers) const;

     /**
     * @brief Get the entity ID. Overload of IEntityInfo::GetId.
     * @return Returns the entity ID.
     */
    virtual uint64_t GetId() const override;

    /**
     * @brief Is the entity defined as (forward) declaration only? Overload of IEntityInfo::ForwardDeclaration.
     * @return Returns whether this entity class instance is defined as forward declaration.
     */
    virtual bool ForwardDeclaration() const override { return m_bForwardDeclaration; }

    /**
     * @brief Is this definition a root entity?
     * @details The root entity is not expecting curly brackets '{...}'.
     * @return Returns whether this entity is the root entity (which is not the case).
     */
    virtual bool IsRootEntity() const { return false; }

    /**
     * @brief Is the entity extendable (can multiple definitions at the same scope exist and do they extend each other)?
     * @details The default implementation doesn't allow extending the entity.
     * @return Returns whether the entity is extendable (which is not the case).
     */
    virtual bool IsExtendable() const { return false; }

    /**
     * @brief Does the entity support children?
     * @details The default implementation doesn't allow children.
     * @return Returns whether the entity supports children (which is not the case).
     */
    virtual bool SupportsChildren() const { return false; }

    /**
     * @{
     * @brief Get a reference to the calculated value. Consider
     * @throw Throws an exception when the index doesn't correspond to the size of the array.
     * @return The value struct smart pointer of the specified type.
     */
    const CValueNodePtr& ValueRef() const { return m_ptrValue; }
    CValueNodePtr& ValueRef() { return m_ptrValue; }
    /**
     * @}
     */

    /**
     * @brief Process the code for a soped name (a name composed of identifiers and scope operators).
     * @param[in] bNoSearchError When set, do not throw exception during search to an entity with the name.
     * @return Returns a pair object with the first element the scoped name and the second element a pointer to the found entity.
     * In case the object doesn't exists, returns a nullptr as second element.
     */
    std::pair<std::string, CEntityPtr> ProcessScopedName(bool bNoSearchError = false);

    /**
     * @brief Process the code for a soped name (a name composed of identifiers and scope operators).
     * @param[in] rlstExpression Reference to the token list holding the expression.
     * @param[in] bNoSearchError When set, do not throw exception during search to an entity with the name.
     * @return Returns a pair object with the first element the scoped name and the second element a pointer to the found entity.
     * In case the object doesn't exists, returns a nullptr as second element.
     */
    std::pair<std::string, CEntityPtr> ProcessScopedName(const CTokenList& rlstExpression, bool bNoSearchError = false) const;

protected:
    /**
     * @brief Calculate the value based on the precedence of operators.
     * @details Parse through the string and calculate a value. Values and arithmetic operators are read and a result is
     * calculated as long as the operator precedence doesn't undercut the current precedence.
     *  - identifiers can be used
     *  - operators allowed for floating and fixed point operations: + - / * < <= > >= == !=
     *  - operators allowed for integral types: + - / % * & | ^ ~ << >> && || ! > >= < <= == !=
     *  - parenthesis are allowed
     *
     * Operator precedence:
     * +------------+-----------------+-------------------------------------------+
     * | precedence | operators       |  description                              |
     * +============+=================+===========================================+
     * |     0      | ~ ! ( ) defined |  Bitwise NOT, logical NOT and parenthesis |
     * +------------+-----------------+-------------------------------------------+
     * |     1      | * / %           |  Multiplication, division, and remainder  |
     * +------------+-----------------+-------------------------------------------+
     * |     2      | + -             |  Addition and subtraction                 |
     * +------------+-----------------+-------------------------------------------+
     * |     3      | << >>           |  Bitwise left shift and right shift       |
     * +------------+-----------------+-------------------------------------------+
     * |     4      | < <= > >=       |  Relational operators                     |
     * +------------+-----------------+-------------------------------------------+
     * |     5      | == !=           |  Equality operators                       |
     * +------------+-----------------+-------------------------------------------+
     * |     6      | &               |  Bitwise AND                              |
     * +------------+-----------------+-------------------------------------------+
     * |     7      | ^               |  Bitwise XOR                              |
     * +------------+-----------------+-------------------------------------------+
     * |     8      | |               |  Bitwise OR                               |
     * +------------+-----------------+-------------------------------------------+
     * |     9      | &&              |  Logical AND                              |
     * +------------+-----------------+-------------------------------------------+
     * |     10     | ||              |  Logical OR                               |
     * +------------+-----------------+-------------------------------------------+
     * @param[in] rlstExpression Reference to the expression token list.
     * @param[in] uiPrecedence [in] Current precedence level (default 100).
     * @return Returns a pair containing the calculated value and a boolean that indicates whether the result is dynamic. In the
     * latter case, the calculated value will be 0.
     */
    std::pair<CConstVariant, bool> ProcessNumericExpression(const CTokenList& rlstExpression, uint32_t uiPrecedence = 100) const;

    /**
     * @brief Process the string expression.
     * @param[in] rlstExpression Reference to the expression token list.
     * @return Returns a pair containing the result string and a boolean that indicates whether the result is dynamic. In the
     * latter case, the result string will be empty.
     */
    std::pair<CConstVariant, bool> ProcessStringExpression(const CTokenList& rlstExpression) const;

    /**
     * @brief Process the code for a type.
     * @param[in] bNoSearchError When set, does not throw an exception during the search of an entity with the defined name.
     * @return The type declaration.
     */
    CTypeDeclaration ProcessType(bool bNoSearchError = false);

    /**
     * @brief Set the name of the entity. This function must be called by the derived class and will check for a duplicate name
     * (case insensitive comparison) and registers the entity under its name.
     * @param[in] rssName String object containing the name of the entity. Cannot be empty.
     * @param[in] bForwardDeclaration When set, the entity is part of forward declaration. When the declaration exists already,
     * this function doesn't do anything. When it doesn't exists, the entity is created and marked as declared.
     * @param[in] bNoInsert When set, do not insert the entity in the child map of the parent. Use only when local entities are
     * needed.
     */
    void SetName(const std::string& rssName, bool bForwardDeclaration = false, bool bNoInsert = false);

    /**
     * @brief In case there are any unassigned system type entities, process the entities.
     */
    void ProcessSystemTypeAssignments();

    /**
     * @brief Add the child to the children list. Called by CreateChild function.
     * @param[in] ptrChild Pointer to the child entity to add.
     */
    virtual void AddChild(CEntityPtr ptrChild);

    /**
     * @brief Find the entity locally by looking in the entity children map and the entity children maps of all the chained
     * entities. The search is done case-insensitive.
     * @param[in] rssName Reference to the string object containing the name of the entity to search for.
     * @param[in] bDeclaration When set, the name belongs to a declaration; otherwise it belongs to a definition. Needed to allow
     * the reuse of names between declarations and definitions.
     * @return Returns a pair object containing an entity pointer if the entity exists or a NULL pointer if not as well as a
     * boolean that indicates that the entity was from an inherited entity.
     */
    virtual std::pair<CEntityPtr, bool> FindLocal(const std::string& rssName, bool bDeclaration) const;

    /**
     * @brief Find the member entity by looking at the entity declaration map of the type.
     * @remarks Only compound entities (struct, union, exception) can have declarations.
     * @param[in] rssScopedName Name of the entity separated by dots (name.name.name) indicating the declaration within a compound
     * entity.
     * @return The declaration entity that is represented by the scoped name or NULL when no entity was found.
     */
    CEntityPtr FindMember(const std::string& rssScopedName) const;

    /**
     * @brief Get the children vector for creation of the iterator.
     * @return Returns the reference to the child vector.
     */
    CEntityVector& GetChildrenVector() { return m_vecChildren; }

    /**
     * @brief Return the declaration type as string.
     * @param[in] eType The declaration type.
     * @return The declaration type as string.
     */
    std::string DeclTypeToString(sdv::idl::EDeclType eType) const;

    /**
     * @brief Return the declaration type as enum type.
     * @param[in] rssType The declaration type as string.
     * @return The declaration type as enum type.
     */
    sdv::idl::EDeclType StringToDeclType(const std::string& rssType) const;

    /**
     * @brief Entity iterator helper class.
     */
    class CEntityIterator : public sdv::idl::IEntityIterator, public sdv::IInterfaceAccess
    {
    public:
        /**
         * @brief Constructor
         * @param[in] rvecEntities Reference to the entity vector to create the iterator for.
        */
        CEntityIterator(CEntityVector& rvecEntities);

        /**
         * @brief Get access to another interface. Overload of IInterfaceAccess::GetInterface.
         * @param[in] idInterface The interface id to get access to.
         * @return Returns a pointer to the interface or NULL when the interface is not supported.
         */
        virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

        /**
        * @brief Get amount of child entities. Overload of IEntityIterator::GetCount.
        * @return Returns the amount of entities in this iterator.
        */
        virtual uint32_t GetCount() const override;

        /**
        * @brief Get entity at supplied index. Overload of IEntityIterator::GetEntityByIndex.
        * @param[in] uiIndex Index of the entity to return.
        * @return Returns interface pointer to the entity or NULL when the index was too large.
        */
        virtual sdv::IInterfaceAccess* GetEntityByIndex(uint32_t uiIndex) override;

    private:
        CEntityVector& m_rvecEntities;    ///< Reference to the entity vector.
    };

private:
    CContextPtr                     m_ptrContext;                   ///< Parse context of the code represented by this entity.
    CEntityPtr                      m_ptrParent;                    ///< Parent entity (could be NULL for the root entity).
    CParser*                        m_pParser = nullptr;            ///< Pointer to the parser.
    CTokenList                      m_lstLocalTokenList;            ///< Local token list to be taken during processing (instead
                                                                    ///< of the parser functions).
    std::string                     m_ssName;                       ///< The entity name.
    CEntityVector                   m_vecChildren;                  ///< Vector with child entities in the order they appear in the
                                                                    ///< code. Some entities allow to be defined in chunks, which
                                                                    ///< means multiple entries with the same name exist.
    CEntityList                     m_lstUnassigned;                ///< List of basic type child entities that were not processed
                                                                    ///< for assignments yet.
    std::shared_ptr<CEntityMap>     m_ptrChildDefMap;               ///< Shared children map containing all the child definitions
                                                                    ///< for the entities with the same scope.
    std::shared_ptr<CEntityMap>     m_ptrChildDeclMap;              ///< Shared children map containing all the child declarations
                                                                    ///< for the entities with the same scope.
    bool                            m_bForwardDeclaration = false;  ///< When set, the entity is only declared and not defined yet.
    CValueNodePtr                   m_ptrValue;                     ///< Smart pointer to the entity value.
    std::string                     m_ssComments;                   ///< Entity comments (could be multiple lines separated by
                                                                    ///< newline).
    uint32_t                        m_uiCommentFlags = 0;           ///< Entity comment flags (bitmask made of
                                                                    ///< sdv::idl::IEntityComments::ECommentMask).
    uint32_t                        m_uiLineBegin = 0;              ///< Begin line position of the entity.
    uint32_t                        m_uiColBegin = 0;               ///< Begin column position of the entity.
    uint32_t                        m_uiLineEnd = 0;                ///< End line position of the entity.
    uint32_t                        m_uiColEnd = 0;                 ///< End column position of the entity.
    std::vector<TDeclTypeAssoc>    m_vecDeclTypes;                 ///< Decl types extended with additional types based on the
                                                                    ///< enabled extension on the command line.
};

template <class TEntity>
TEntity* CEntity::Get()
{
    return dynamic_cast<TEntity*>(this);
}

template <class TEntity>
const TEntity* CEntity::Get() const
{
    return dynamic_cast<const TEntity*>(this);
}

template <class TEntity, typename... TParams>
inline CEntityPtr CEntity::CreateChild(const CContextPtr& rptrContext, CEntity* pParent, TParams... tParams)
{
    // Check to see if TEntity derives from CEntity.
    static_assert(std::is_base_of_v<CEntity, TEntity>);

    // Valid parent entity?
    if (!pParent) throw CCompileException("Internal error: missing parent.");

    // Is the creation of children allowed?
    if (!pParent->SupportsChildren())
        throw CCompileException("Internal error: trying to add an entity to a parent entity that doesn't support this.");

    // Get the shared pointer
    CEntityPtr ptrParent = pParent->shared_from_this();

    // Create the entity class
    CEntityPtr ptrEntity = std::make_shared<TEntity>(rptrContext, ptrParent, tParams...);

    // Add the entity to the parent's children list.
    pParent->AddChild(ptrEntity);

    return ptrEntity;
}

template <typename TEntity>
TEntity* CEntity::Find(const std::string& rssScopedName) const
{
    return dynamic_cast<TEntity*>(Find(rssScopedName).get());
}

template <typename TValueNode>
TValueNode* CEntity::FindValue(const std::string& rssScopedNameInclMembers) const
{
    auto ptrtValueNode = FindValue(rssScopedNameInclMembers);
    return dynamic_cast<TValueNode*>(ptrtValueNode.get());
}

#endif // !defined(ENTITY_BASE_H)
