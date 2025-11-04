#include "hash_calc.h"

CHashObject::CHashObject()
{
	// Create a xxHash state
	m_state = XXH64_createState();

	/* Initialize state with selected seed */
	XXH64_hash_t seed = 0;   /* or any other value */
	if (XXH64_reset(m_state, seed) == XXH_ERROR)
	{
		XXH64_freeState(m_state);
		m_state = nullptr;
	}
}

CHashObject::~CHashObject()
{
	if (m_state) XXH64_freeState(m_state);
}

CHashObject& CHashObject::operator<<(const std::string& rssString)
{
	if (!m_state) return *this;
	if (rssString.empty()) return *this;
	if (XXH64_update(m_state, rssString.c_str(), rssString.size()) == XXH_ERROR)
	{
		XXH64_freeState(m_state);
		m_state = nullptr;
	}
	return *this;
}

uint64_t CHashObject::GetHash() const
{
	if (!m_state) return 0;
	XXH64_hash_t hash = XXH64_digest(m_state);
	return hash;
}
