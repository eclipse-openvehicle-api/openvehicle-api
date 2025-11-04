#ifndef ATTRIBUTE_ENTITY_H
#define ATTRIBUTE_ENTITY_H

#include "declaration_entity.h"

/**
 * @brief The attribute definition of an IDL file.
 * @details The attribute section of the IDL file defines attribute values.
 */
class CAttributeEntity : public CDeclarationEntity, public sdv::idl::IAttributeEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     * @param[in] bReadOnly When set, the attribute is defined as readonly.
     */
    CAttributeEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, bool bReadOnly);

    /**
     * @brief Destructor
     */
    virtual ~CAttributeEntity() override = default;

    /**
    * @brief Get access to another interface. Overload of sdv::IInterfaceAccess::GetInterface.
    * @param[in] idInterface The interface id to get access to.
    * @return Returns a pointer to the interface or NULL when the interface is not supported.
    */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Get the list of possible exceptions that might be fired during a read operation. Overload of
     * sdv::idl::IAttributeEntity::GetReadExceptions.
     * @return Returns a pointer to the exceptions iterator or NULL when no exceptions were defined.
     */
    virtual sdv::idl::IEntityIterator* GetReadExceptions() override;

    /**
     * @brief Get the list of possible exceptions that might be fired during a write operation. Overload of
     * sdv::idl::IAttributeEntity::GetWriteExceptions.
     * @return Returns a pointer to the exceptions iterator or NULL when no exceptions were defined.
     */
    virtual sdv::idl::IEntityIterator* GetWriteExceptions() override;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns the attribute entity type.
     */
    virtual  sdv::idl::EEntityType GetType() const override { return  sdv::idl::EEntityType::type_attribute; }

    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

protected:
    /**
     * @brief Does the entity support arrays? Overload of CDeclarationEntity::SupportArrays.
     * @return Returns 'true' when the entity supports assignments; 'false' otherwise.
     */
    virtual bool SupportArrays() const override;

    /**
     * @brief Is the entity readonly? Overload of IEntityInfo::IsReadOnly.
     * @return Returns 'true' when the entity defined as readonly; 'false' otherwise.
     */
    virtual bool IsReadOnly() const override;

    /**
     * @brief Does the entity support multiple declarations on one line of code?  Overload of
     * CDeclarationEntity::SupportMultipleDeclarations.
     * @return Returns 'true' when the entity supports multiple declarations; 'false' otherwise.
     */
    virtual bool SupportMultipleDeclarations() const override;

    /**
     * @brief Does the entity support raising exceptions? Overload of CDeclarationEntity::SupportRaiseExceptions.
     * @return Returns 'true' when the entity supports exceptions; 'false' otherwise.
     */
    virtual bool SupportRaiseExceptions() const override;

    /**
     * @brief Does the entity support separate set/get raising exceptions? Overload of
     * CDeclarationEntity::SupportSeparateSetGetRaiseExceptions.
     * @return Returns 'true' when the entity supports separate set/get raise exceptions; 'false' otherwise.
     */
    virtual bool SupportSeparateSetGetRaiseExceptions() const override;

private:
    bool                m_bReadOnly = false;        ///< When set, the attribute is readonly.
    CEntityIterator     m_iteratorReadExceptions;   ///< Exceptions iterator for read exceptions
    CEntityIterator     m_iteratorWriteExceptions;  ///< Exceptions iterator for write exceptions
};



#endif // !defined(ATTRIBUTE_ENTITY_H)