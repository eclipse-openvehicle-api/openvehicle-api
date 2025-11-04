#include "parsecontext.h"

CContext::CContext(const std::filesystem::path& rpath,
    sdv::idl::IEntityContext::ELocation eLocation /*= sdv::idl::IEntityContext::ELocation::source*/) :
    m_eLocation(eLocation)
{
    if (rpath.is_absolute())
        m_source = rpath;
    else
        m_source = std::filesystem::current_path() / rpath;
    m_code = m_source.GetCodeRef().c_str();
}

CContext::CContext(const char* szCode) :
    m_source(szCode), m_code(m_source.GetCodeRef().c_str()),
    m_eLocation(sdv::idl::IEntityContext::ELocation::source)
{}

CContext::CContext(CContext&& rContext) noexcept:
    m_source(std::move(rContext.m_source)), m_code(m_source.GetCodeRef().c_str())
{}

CContext::~CContext()
{}

CContext& CContext::operator=(CContext&& rContext) noexcept
{
    m_source = std::move(rContext.m_source);
    m_code = std::move(CCodePos(m_source.GetCodeRef().c_str()));
	m_eLocation = rContext.m_eLocation;
	rContext.m_eLocation = sdv::idl::IEntityContext::ELocation::source;
    return *this;
}

bool CContext::operator==(const CContext& rContext) const
{
    if (&rContext == this) return true;
    if (!m_source.GetPathRef().empty() &&
        m_source.GetPathRef() == rContext.m_source.GetPathRef()) return true;
    return m_source.GetCodeRef() == rContext.m_source.GetCodeRef();
}

bool CContext::operator!=(const CContext& rContext) const
{
    return !operator==(rContext);
}

