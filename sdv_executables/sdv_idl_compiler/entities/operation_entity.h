#ifndef OPERATION_ENTITY_H
#define OPERATION_ENTITY_H

#include "declaration_entity.h"

/**
 * @brief The operation definition of an IDL file.
 * @details The operation section of the IDL file defines operations.
 */
class COperationEntity : public CDeclarationEntity, public sdv::idl::IOperationEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    COperationEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent);

    /**
     * @brief Destructor
     */
    virtual ~COperationEntity() override = default;

    /**
     * @brief Get access to another interface. Overload of sdv::IInterfaceAccess::GetInterface.
     * @param[in] idInterface The interface id to get access to.
     * @return Returns a pointer to the interface or NULL when the interface is not supported.
     */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Get parameter entity iterator if the definition has any parameters. Overload of
     * sdv::idl::IOperationEntity::GetParameters.
     * @return Returns a pointer to the parameter entity iterator or NULL when not available.
     */
    virtual sdv::idl::IEntityIterator* GetParameters() override;

    /**
     * @brief Get the list of possible exceptions that might be fired for this operation. Overload of
     * sdv::idl::IOperationEntity::GetExceptions.
     * @return Interface pointer to the exception iterator.
     */
    virtual sdv::idl::IEntityIterator* GetExceptions() override;

    /**
     * @brief Is the entity readonly (variable declarations and writable attributes aren't)? Overload of IDeclarationEntity::IsReadOnly.
     * @details Returns whether the entity is readonly by design or whether it is defined readonly by the code. Default value is
     * 'true'.
     * @return Returns 'true' when the entity defined as readonly; 'false' otherwise.
     */
    virtual bool IsReadOnly() const override { return m_bOperationIsConst; }

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns the operation type.
     */
    virtual  sdv::idl::EEntityType GetType() const override { return  sdv::idl::EEntityType::type_operation; }

    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override { CDeclarationEntity::Process(); }

protected:
    /**
     * @brief Does the entity support raising exceptions? Overload of CDeclarationEntity::SupportRaiseExceptions.
     * @return Returns 'true' when the entity defined as attribute; 'false' otherwise.
     */
    virtual bool SupportRaiseExceptions() const override { return true; }

    /**
     * @brief Does the entity support arrays? Overload of CDeclarationEntity::SupportArrays.
     * @return Returns 'true' when the entity supports assignments; 'false' otherwise.
     */
    virtual bool SupportArrays() const override { return true; }

    /**
     * @brief Does the entity support an interface as base type? Overload of CDeclarationEntity::SupportVoid.
     * @return Returns 'true' when the entity supports interfaces as base type; 'false' otherwise.
     */
    virtual bool SupportInterface() const override { return true; }

    /**
     * @brief Does the entity support 'void' as base type? Overload of CDeclarationEntity::SupportVoid.
     * @details Returns whether the entity supports the 'void' base type. Default value is 'false'.
     * @return Returns 'true' when the entity supports void as base type; 'false' otherwise.
     */
    virtual bool SupportVoid() const override { return true; }

    /**
     * @brief Does the entity support parameters? Overload of CDeclarationEntity::RequiresParameters.
     * @details Returns whether the entity supports parameters. Default value is 'false'.
     * @return Returns 'true' when the entity requires parameters; 'false' otherwise.
     */
    virtual bool RequiresParameters() const override { return true; }

    /**
     * @brief Set operation as const. Overload of CDeclarationEntity::SetOperationAsConst.
     */
    virtual void SetOperationAsConst() override { m_bOperationIsConst = true; }

    /**
     * @brief Find the entity locally by looking in the parameter list. Overload of CEntity::FindLocal.
     * @param[in] rssName Reference to the string object containing the name of the entity to search for.
     * @param[in] bDeclaration When set, the name belongs to a declaration; otherwise it belongs to a definition. Needed to allow
     * the reuse of names between declarations and definitions.
     * @return Returns a pair object containing an entity pointer if the entity exists or a NULL pointer if not as well as a
     * boolean that indicates that the entity was from an inherited entity.
     */
    virtual std::pair<CEntityPtr, bool> FindLocal(const std::string& rssName, bool bDeclaration) const override;

    /**
     * @brief Does the entity require an assignment (const declarations do)? Overload of CDeclarationEntity::RequiresAssignment.
     * @details Default processing is done by the declaration function (checking for unbound arrays). Exception: when the
     * parent interface is defined as local, assignment is not required.
     * @return Returns 'true' when the entity requires an assignment; 'false' otherwise.
     */
    virtual bool RequiresAssignment() const override;

private:
    bool                m_bOperationIsConst = false;    ///< When set, the operation is defined as 'const' operation.
    CEntityIterator     m_iteratorParameters;           ///< Parameters iterator
    CEntityIterator     m_iteratorExceptions;           ///< Exceptions iterator
};


#endif // !defined(OPERATION_ENTITY_H)