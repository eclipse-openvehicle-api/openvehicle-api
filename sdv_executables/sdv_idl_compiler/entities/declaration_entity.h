#ifndef BASIC_TYPE_ENTITY_H
#define BASIC_TYPE_ENTITY_H

#include "entity_base.h"
#include "entity_value.h"
#include "../constvariant.h"
#include <vector>

/**
 * @brief The base for declaration entity definitions within an IDL file (declarations, typedefs, attributes, operations,
 * parameters, case declarations, enum entries).
 * @details The declaration entity definitions all have a similar setup with small differences in detail. Consider the following
 * structures:
 * @code
 *      <type> <name> = <value>
 *      const <type> <name> = <value>
 *      readonly attribute <name>
 *      <operation_type> <operation_name>(<parameter_type> <parameter_name>) const
 * @endcode
 * The following generalized structure applies to all declaration structures:
 * @code
 *      prefix type name
 *      prefix type name = value
 *      prefix type name(parameters) postfix
 * @endcode
 * The first statement is a declaration. The second statement is a declaration with an assignment. The last statement represents
 * an operation.
 * The prefix is used to provide a specific interpretation to the declaration (in, out, inout, const, attribute, readonly,
 * typedef, struct, union, enum).
 * The type defines the type the declaration represents (either a system type or a typedefed type - scoped name).
 * The name is the defined name o the declaration.
 * The value is the expression used for the assignment.
 * The parameters are a list of zero or more declaration statements.
 * The postfix is used to provide a specific interpretation to the declaration (const).
 * Some declarations might start as a complex type (e.g. struct, union, enum). The might contain the type definition as well as
 * the declaration. For example:
 * @code
 *      struct <def_name> { <definition> } <decl_name>
 *      struct { <definition> } <decl_name>
 * @endcode
 * In the first statement, the struct is defined and declared in one statement. In the second statement, an anonymous struct is
 * defined and declared in one statement.
 * Multiple declarations are possible for many types. The declarations are separated by a comma and follow the same rules as a
 * single declaration starting at the &lt;name&gt;.
 * @code
 *      <type> <name_1> = <value_1>, <name_2> = <value_2>, <name_3>, <name_4> = <value_4>
 * @endcode
 */
class CDeclarationEntity : public CEntity, public sdv::idl::IDeclarationEntity
{
    friend CSimpleTypeValueNode;
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    CDeclarationEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent);

    /**
     * @brief Constructor using the provided token-list to process the code.
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     * @param[in] rlstTokenList Reference to the token list holding the tokens to process.
     */
    CDeclarationEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, const CTokenList& rlstTokenList);

    /**
     * @brief Destructor
     */
    virtual ~CDeclarationEntity() override = default;

    /**
    * @brief Get access to another interface. Overload of sdv::IInterfaceAccess::GetInterface.
    * @param[in] idInterface The interface id to get access to.
    * @return Returns a pointer to the interface or NULL when the interface is not supported.
    */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Get declaration type. Overload of sdv::idl::IDeclarationEntity::GetDeclarationType.
     * @return Interface to the declaration type object.
     */
    virtual sdv::IInterfaceAccess* GetDeclarationType() const override;

    /**
     * @brief Is array? Overload of sdv::idl::IDeclarationEntity::HasArray/HasDynamicArray.
     * @return Retrurns whether the declaration reqpresents an array.
     */
    virtual bool HasArray() const override;

    /**
     * @brief Get the array dimensions (if there are any). Overload of IDeclarationEntity::GetDimensionCount.
     * @return Smart pointer to the sequence of array dimensions.
     */
    virtual sdv::sequence<sdv::idl::SArrayDimension> GetArrayDimensions() const override;

    /**
     * @brief Has assignment? Overload of sdv::idl::IDeclarationEntity::HasAssignment.
     * @return Returns whether the declaration has an assignment.
     */
    virtual bool HasAssignment() const override;

    /**
     * @brief Get assignment string. Overload of sdv::idl::IDeclarationEntity::GetAssignment.
     * @details The assignment can be an algebraic expression composed from constants and variables. If the assignment is an array,
     * the expression is composed like this: {{expr1},{expr2},{expr...}}
     * @return On success, returns the assignment string object or an empty string when no assignment is available.
     */
    virtual sdv::u8string GetAssignment() const override;

    /**
     * @brief Get the base type of the entity.
     * @return Returns the base type.
     */
    virtual sdv::idl::EDeclType GetBaseType() const { return m_typedecl.GetBaseType(); }

    /**
     * @brief Get the type entity if the type is not a system type.
     * @details Complex types (struct, enum, union) and type definitions are based on a type entity.
     * @return Returns a pointer to the type entity if available.
     */
    CEntityPtr GetTypeEntity() const;

    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Process a declaration.
     * @details The processing of a declaration is done through several steps: processing the declaration type, processing the
     * declaration identifier, processing (multi-dimensional) arrays, processing assignments, processing the next declaration
     * identifier and so on.
     * The following statement are examples:
     * @code
     *      <type> <identifier>;                                            // No assignment, one declaration
     *      <type> <identifier>, <identifier>;                              // No assignment, multiple declarations
     *      <type> <identifier>=<expression>;                               // One definition (declaration with assignment)
     *      <type> <identifier>=<expressoin>, <identifier>=<expression>;    // Multiple definitions
     *      <type> <identifier>=<expression>, <identifier>;                 // Mixed expression, definition
     *      <type> <identifier>[]={<expression>, <expression>};             // Array definition
     *      <type> <identifier>[<expression>];                              // Array declaration
     *      <type> <identifier>[<expression>]={<expression>, <expression>}; // Array definition
     *      struct <type> <identifier>;                                     // Struct declaration with explicit struct keyword
     *      struct <type> <identifier>={<expression>, <expression>};        // Struct declaration with assignment
     *      union <type> <identifier>;                                      // Union declaration with explicit union keyword
     *      enum <type> <identifier>;                                       // Enum declaration with explicit enum keyword
     *      enum <type> <identifier>=<exppression>;                         // Enum definition with explicit enum keyword
     * @endcode
     * The 'type' can be a system type (short, int64, float) or a scoped type definition (MyType, \::MyModule\::MyType). Without
     * assignment the type can be 'struct' or 'union', with or without the 'struct' and 'union' type identification (not allowed
     * for const entities, which need an assignment).
     * The 'identifier' is the unique name for the declaration. This name must be case-insensitive unique within the current
     * scope. The identifier can be followed by square-brackets indicating an array. The size of the array can either be retrieved
     * through the assignment or through the expression. If both an assignment and an expression are available, they must match.
     * The array expression must be of an integral type and is not allowed to become negative. Furthermore, it can be created
     * through a mathematical expression existing of constants and/or when not defined to be a constant entity, through
     * declarations made before (this deviates to C/C++, where dynamic arrays are not allowed). In case of a dynamic array, an
     * assignment is not supported.
     * The assignment 'expression' defines a mathematical expression existing of constants and/or when not defined to be a
     * constant entity, through declarations made before.
     * @remarks Array size expressions and assignment expressions are stored as tokenlists to be processed by the
     * ProcessValueAssignment function.
     * @param[in] rTypeDecl Reference to the type identifier (can be a system type as well as a scoped name to a previously
     * defined type).
     */
    void ProcessDeclaration(const CTypeDeclaration& rTypeDecl);

    /**
     * @brief Preprocess the multi-dimensional array declaration.
     * @details Preprocess the potential multi-dimensional array declaration by detecting square brackets and storing the tokens
     * between the brackets. The tokens for each dimension are placed in the m_vecMultiArraySizeTokenList vector.
     */
    void PreprocessArrayDeclaration();

    /**
     * @brief Preprocess a list of comma separated declarations.
     * @details Preprocess a list of tokens separated by the comma ',' separator and place the tokens in the provided vector. The
     * processing is continued until a square bracket ']' or normal bracket ')' or no token exists any more.
     * @param[in] rvecTokenList Reference to the vector containing the token lists to be filled.
     */
    void PreprocessTokenListVector(std::vector<CTokenList>& rvecTokenList);

    /**
     * @brief Postprocess the token lists that where read in the preprocess functions.
     * @pre SupportAssignments returns true and at least m_vecMultiArraySizeTokenList or m_lstAssignmentTokenList is filled.
     * @details For typedef, const and declaration entities, create the value structure containing the arrays and the values of
     * the type entity. For const and declaration entities, fill the value structure using the assignment stored in the assignment
     * expression token list. For attributes and operations build the raising exception lists. For the operations process the
     * parameter list.
     */
    void PostProcess();

    /**
     * @brief Does the entity support assignments (const and variable declarations do, others don't)?
     * @details Determines whether the entity supports assignments. Default value is 'false'.
     * @return Returns 'true' when the entity supports assignments; 'false' otherwise.
     */
    virtual bool SupportAssignments() const { return false; }

    /**
     * @brief Does the entity require an assignment (const declarations do)?
     * @details Determines whether the entity requires an assignment. Default value is is based on the presence of an unbound
     * value in the type.
     * @return Returns 'true' when the entity requires an assignment; 'false' otherwise.
     */
    virtual bool RequiresAssignment() const;

    /**
     * @brief Can the entity be used for assignments of complex types (variable declarations do)?
     * @details Returns whether the entity is defined to be usable for complex type assignments. Default value is 'false'.
     * @return Returns 'true' when the entity defined as declaration; 'false' otherwise.
     */
    virtual bool CanSupportComplexTypeAssignments() const { return false; }

    /**
     * @brief Does the entity support arrays (const and variable declarations, as well as typedefs and attributes do)?
     * @details Determines whether the entity supports arrays. Default value is 'false'.
     * @return Returns 'true' when the entity supports assignments; 'false' otherwise.
     */
    virtual bool SupportArrays() const { return false; }

    /**
     * @brief Is the entity readonly (variable declarations and writable attributes aren't)? Overload of
     * IDeclarationEntity::IsReadOnly.
     * @details Returns whether the entity is readonly by design or whether it is defined readonly by the code. Default value is
     * 'true'.
     * @return Returns 'true' when the entity defined as readonly; 'false' otherwise.
     */
    virtual bool IsReadOnly() const override { return true; }

    /**
     * @brief Is the entity transparent when used in a struct? Overload of IDeclarationEntity::IsAnonymous.
     * @details Returns whether the entity is anonymous when used in a struct/union (unnamed and not declared). This allows its
     * members to appear directly as members within the struct. Default value is 'false'.
     * @return Returns 'true' when the entity defined as anonymous; 'false' otherwise.
     */
    virtual bool IsAnonymous() const override { return false; }

    /**
     * @brief Does the entity support multiple declarations on one line of code (const and var declarations and attributes do)?
     * @details Returns whether the entity supports multiple declarations separated by a comma ','. Default value is 'false'.
     * @return Returns 'true' when the entity supports multiple declarations; 'false' otherwise.
     */
    virtual bool SupportMultipleDeclarations() const { return false; }

    /**
     * @brief Do not enforce next declaration after comma (enums do)?
     * @pre SupportMultipleDeclarations needs to be supported.
     * @details Returns whether the entity supports ending the definition after a comma ','. Default value is 'false'.
     * @return Returns 'true' when not enforcing the next declaration; 'false' otherwise.
     */
    virtual bool DoNotEnfoceNextDeclarationAfterComma() const { return false; }

    /**
    * @brief Does the entity support raising exceptions (attributes and operations do)?
    * @details Returns whether the entity supports exceptions (defined through the keywords: raises, getraises and setraises).
    * Default value is 'false'.
    * @return Returns 'true' when the entity supports exceptions; 'false' otherwise.
    */
    virtual bool SupportRaiseExceptions() const { return false; }

    /**
    * @brief Does the entity support separate set/get raising exceptions (only attributes do)?
    * @details Returns whether the entity supports exceptions (defined through the keywords: getraises and setraises).
    * Default value is 'false'.
    * @return Returns 'true' when the entity supports separate set/get raise exceptions; 'false' otherwise.
    */
    virtual bool SupportSeparateSetGetRaiseExceptions() const { return false; }

    /**
     * @brief Does the entity support an interface as base type (non-const variables, operations and parameters do)?
     * @details Returns whether the entity supports the an interface as base type base type. Default value is 'false'.
     * @return Returns 'true' when the entity supports interfaces as base type; 'false' otherwise.
     */
    virtual bool SupportInterface() const { return false; }

    /**
     * @brief Does the entity support 'void' as base type (operations do)?
     * @details Returns whether the entity supports the 'void' base type. Default value is 'false'.
     * @return Returns 'true' when the entity supports void as base type; 'false' otherwise.
     */
    virtual bool SupportVoid() const { return false; }

    /**
     * @brief Does the entity require parameters (operations do)?
     * @details Returns whether the entity requires parameters. Default value is 'false'.
     * @return Returns 'true' when the entity requires parameters; 'false' otherwise.
     */
    virtual bool RequiresParameters() const { return false; }

    /**
     * @brief Set operation as const (operations only).
     * @details If the declaration requires parameters, the declaration is checked for being defined as const operation. If so,
     * this function is called. Default implementation doesn't do anything.
     * @pre Only called when RequiresParameters is true and the declaration is defined as const.
     */
    virtual void SetOperationAsConst() {}

    /**
     * @brief Calculate the hash of this entity and all encapsulated entities. Overload of CBaseEntity::CalcHash.
     * @param[in, out] rHash Hash object to be filled with data.
     */
    virtual void CalcHash(CHashObject& rHash) const override;

protected:
    /**
     * @brief Get parameter vector.
     * @return Returns the vector with the parameter entities.
     */
    CEntityVector& GetParamVector() { return m_vecParameters; }

    /**
     * @brief Get parameter vector.
     * @return Returns the vector with the parameter entities.
     */
    const CEntityVector& GetParamVector() const { return m_vecParameters; }

    /**
    * @brief Get "raises" exceptions vector.
    * @return Returns the vector with the exception entities.
    */
    CEntityVector& GetExceptionVector() { return m_vecRaisesExceptions; }

    /**
    * @brief Get "get_raises" exceptions vector.
    * @return Returns the vector with the exception entities.
    */
    CEntityVector& GetReadExceptionVector() { return m_vecGetRaisesExceptions; }

    /**
    * @brief Get "set_raises" exceptions vector.
    * @return Returns the vector with the exception entities.
    */
    CEntityVector& GetWriteExceptionVector() { return m_vecSetRaisesExceptions; }

private:

    /**
     * @brief Process assignment state.
     * @details The assignment processing progression state which is used to control the processing of assignments as well as to
     * prevent circular use of assignments.
     */
    enum class EProcessAssignmentProgression
    {
        unprocessed,            ///< Assignment hasn't been processed yet
        currently_processing,   ///< Processing currently takes place
        processed,              ///< Processing has been done
    };

    CTypeDeclaration                m_typedecl;                         ///< The type definition of this declaration.
    std::vector<CTokenList>         m_vecMultiArraySizeTokenList;       ///< The list of tokens for each array dimension to be
                                                                        ///< calculated during the post processing phase.
    CTokenList                      m_lstAssignmentTokenList;           ///< The list of tokens forming the assignment.
    std::vector<CTokenList>         m_vecRaisesExceptionsTokenList;     ///< The list of tokens for each exception to be parsed
                                                                        ///< during the post processing phase.
    std::vector<CTokenList>         m_vecSetRaisesExceptionsTokenList;  ///< The list of tokens for each exception to be parsed
                                                                        ///< during the post processing phase.
    std::vector<CTokenList>         m_vecGetRaisesExceptionsTokenList;  ///< The list of tokens for each exception to be parsed
                                                                        ///< during the post processing phase.
    std::vector<CTokenList>         m_vecParametersTokenList;           ///< The list of tokens for each parameter to be parsed
                                                                        ///< during the post processing phase.
    EProcessAssignmentProgression   m_eProcAssState = EProcessAssignmentProgression::unprocessed;  ///< Processing assignment
                                                                        ///< progression state.
    CEntityVector                   m_vecRaisesExceptions;              ///< Can raise the exceptions while reading/writing.
    CEntityVector                   m_vecGetRaisesExceptions;           ///< Can raise the exceptions while reading.
    CEntityVector                   m_vecSetRaisesExceptions;           ///< Can raise the exceptions while writing.
    CEntityVector                   m_vecParameters;                    ///< Vector of parameters.
};

#endif // !defined(BASIC_TYPE_ENTITY_H)