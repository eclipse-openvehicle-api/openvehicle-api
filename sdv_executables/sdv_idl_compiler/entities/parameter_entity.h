#ifndef PARAMETER_ENTITY_H
#define PARAMETER_ENTITY_H

#include "declaration_entity.h"

/**
 * @brief The parameter definition of an operation and value type.
 * @details The parameter section contains the definition of the parameter for operations and value types.
 */
class CParameterEntity : public CDeclarationEntity, public sdv::idl::IParameterEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     * @param[in] rlstTokenList Reference to the token list holding the tokens to process.
     * @param[in] bEnforceDirection Enforce parameter direction.
     */
    CParameterEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, const CTokenList& rlstTokenList,
        bool bEnforceDirection = true);

    /**
     * @brief Destructor
     */
    virtual ~CParameterEntity() override = default;

    /**
    * @brief Get access to another interface. Overload of IInterfaceAccess::GetInterface.
    * @param[in] idInterface The interface id to get access to.
    * @return Returns a pointer to the interface or NULL when the interface is not supported.
    */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns the parameter type.
     */
    virtual  sdv::idl::EEntityType GetType() const override { return  sdv::idl::EEntityType::type_parameter; }

    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

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
     * @brief Get the parameter direction. Overload of sdv::idl::IParameterEntity::GetDirection.
     * @return Parameter direction.
     */
    virtual sdv::idl::IParameterEntity::EParameterDirection GetDirection() const override { return m_eDirection; }

    /**
     * @brief Does the entity require an assignment (const declarations do)? Overload of CDeclarationEntity::RequiresAssignment.
     * @details Default processing is done by the declaration function (checking for unbound arrays). Exception: when the
     * parent interface is defined as local, assignment is not required.
     * @return Returns 'true' when the entity requires an assignment; 'false' otherwise.
     */
    virtual bool RequiresAssignment() const override;

    /**
     * @brief Is the entity readonly (variable declarations and writable attributes aren't)? Overload of IDeclarationEntity::IsReadOnly.
     * @details Returns whether the entity is readonly by design or whether it is defined readonly by the code. Default value is
     * 'true'.
     * @return Returns 'true' when the entity defined as readonly; 'false' otherwise.
     */
    virtual bool IsReadOnly() const override { return false; }

    /**
    * @brief Calculate the hash of this entity and all encapsulated entities. Overload of CBaseEntity::CalcHash.
    * @param[in, out] rHash Hash object to be filled with data.
    */
    virtual void CalcHash(CHashObject& rHash) const override;

private:
    bool                m_bEnforceDirection;    ///< When set, the parameter enforces the direction indicator when processing.
    sdv::idl::IParameterEntity::EParameterDirection m_eDirection =
        sdv::idl::IParameterEntity::EParameterDirection::unknown;         ///< The direction type of the parameter.
};

#endif // !defined(PARAMETER_ENTITY_H)