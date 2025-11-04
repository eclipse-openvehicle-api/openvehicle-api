#ifndef PS_CPP_GENERATOR_H
#define PS_CPP_GENERATOR_H

#include "../../../global/ipc_named_mutex.h"

/**
 * @brief Prox/stub CPP file generator class.
 */
class CPsCppGenerator : public CGenContext
{
public:
    /**
    * @brief Constructor
    * @param[in] pParser Pointer to the parser object.
    */
    CPsCppGenerator(sdv::IInterfaceAccess* pParser);

    /**
    * @brief Destructor
    */
    virtual ~CPsCppGenerator() override;

    /**
    * @brief Generate the definition.
    * @return Returns whether the generation was successful.
    */
    bool Generate();

private:
    ipc::named_mutex        m_mtx;      ///< Guarantee exclusive access while writing the PS file.
};

#endif // !defined PS_CPP_GENERATOR_H