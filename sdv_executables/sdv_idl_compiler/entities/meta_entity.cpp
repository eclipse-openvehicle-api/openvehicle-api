#include "meta_entity.h"

CMetaEntity::CMetaEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, const CToken& rtokenMeta,
    const CTokenList lstComments) :
    CEntity(rptrContext, ptrParent)
{
    std::string ssContent = rtokenMeta;

    // Skip initial whitespace and store the content if not verbatim text.
    size_t nSkip = 0;
    if (rtokenMeta.GetMetaType() != ETokenMetaType::token_meta_verbatim)
    {
        while (nSkip < ssContent.size() && std::isspace(ssContent[nSkip]))
            ++nSkip;
    }
    m_ssContent = ssContent.substr(nSkip);

    // Store the type
    switch (rtokenMeta.GetMetaType())
    {
    case ETokenMetaType::token_meta_include_local:
        m_eType = sdv::idl::IMetaEntity::EType::include_local;
        break;
    case ETokenMetaType::token_meta_include_global:
        m_eType = sdv::idl::IMetaEntity::EType::include_global;
        break;
    case ETokenMetaType::token_meta_define:
        m_eType = sdv::idl::IMetaEntity::EType::define;
        break;
    case ETokenMetaType::token_meta_undef:
        m_eType = sdv::idl::IMetaEntity::EType::undef;
        break;
    case ETokenMetaType::token_meta_verbatim:
        m_eType = sdv::idl::IMetaEntity::EType::verbatim;
        break;
    default:
        throw CCompileException("Internal error: incomplete meta token received.");
        break;
    }

    // Store comments
    SetCommentTokens(lstComments, true);

    // Set the position in the source file
    SetBeginPosition(rtokenMeta.GetLine(), rtokenMeta.GetCol());
}

sdv::interface_t CMetaEntity::GetInterface(sdv::interface_id idInterface)
{
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IMetaEntity>())
        return static_cast<sdv::idl::IMetaEntity*>(this);
    return CEntity::GetInterface(idInterface);
}

sdv::idl::IMetaEntity::EType CMetaEntity::GetMetaType() const
{
    return m_eType;
}

sdv::u8string CMetaEntity::GetContent() const
{
	return m_ssContent;
}
