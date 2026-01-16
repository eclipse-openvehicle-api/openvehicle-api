#ifndef LOCAL_MEM_MGR_H
#define LOCAL_MEM_MGR_H

#define SDV_CORE_H

#define NO_SDV_CORE_FUNC
#include "../export/interfaces/core.h"
#include "../export//support/interface_ptr.h"

#include <support/mem_access.h>
#include <support/interface_ptr.h>

/**
 * @brief Local memory manager class allowing the use of the SDV support classes without having to start the framework.
 * \attention Do not use the local memory manager together with the framework's memory manager.
 */
class CLocalMemMgr : public sdv::core::IMemoryAlloc, public sdv::IInterfaceAccess,
    public sdv::internal::IInternalMemAlloc
{
public:
    /**
     * @brief Constructor assigning this class to the local services.
     */
    CLocalMemMgr()
    {}

    /**
     * @brief Destructor removing this class from the local services.
     */
    ~CLocalMemMgr()
    {}

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IInterfaceAccess)
        SDV_INTERFACE_ENTRY(sdv::core::IMemoryAlloc)
    END_SDV_INTERFACE_MAP()

    /**
    * @brief Allocate a memory block of the provided length. Overload of sdv::core::IMemoryAlloc::Allocate.
    * @param[in] uiLength The length of the memory block to allocate.
    * @return Smart pointer to the allocated memory or NULL when memory allocation was not possible.
    */
    virtual sdv::pointer<uint8_t> Allocate(uint32_t uiLength) override
    {
        return sdv::internal::make_ptr<uint8_t>(this, uiLength);
    }

protected:
    /**
    * @brief Allocate memory. Overload of sdv::internal::IInternalMemAlloc::Alloc.
    * @param[in] nSize The size of the memory to allocate (in bytes).
    * @return Pointer to the memory allocation or NULL when memory allocation failed.
    */
    virtual void* Alloc(size_t nSize) override
    {
        void* p = malloc(nSize);
        return p;
    }

    /**
    * @brief Reallocate memory. Overload of sdv::internal::IInternalMemAlloc::Realloc.
    * @param[in] pData Pointer to a previous allocation or NULL when no previous allocation was available.
    * @param[in] nSize The size of the memory to allocate (in bytes).
    * @return Pointer to the memory allocation or NULL when memory allocation failed.
    */
    virtual void* Realloc(void* pData, size_t nSize) override
    {
        void* p = realloc(pData, nSize);
        return p;
    }

    /**
    * @brief Free a memory allocation. Overload of sdv::internal::IInternalMemAlloc::Free.
    * @param[in] pData Pointer to a previous allocation.
    */
    virtual void Free(void* pData) override
    {
        free(pData);
    }
};

#ifndef NO_SDV_LOCAL_CORE_FUNC
namespace sdv
{
    namespace core
    {
        /**
        * @brief Access to the core.
        * @return Smart pointer to the core services interface.
        */
        inline TInterfaceAccessPtr GetCore()
        {
            static CLocalMemMgr memmgr;
            return &memmgr;
        }

        /**
         * @brief Access to specific interface of the core.
         * @tparam TInterface Type of interface to return.
         * @return Pointer to the interface or NULL when the interface was not exposed.
         */
        template <typename TInterface>
        inline TInterface* GetCore()
        {
            return GetCore().GetInterface<TInterface>();
        }
    }
}
#endif

#endif // !defined(LOCAL_MEM_MGR_H)