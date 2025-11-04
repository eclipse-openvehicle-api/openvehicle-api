#ifndef CMAKE_GENERATOR_H
#define CMAKE_GENERATOR_H

#include <filesystem>
#include <string>
#include "codegen_base.h"

/**
* @brief CMake generator class.
*/
class CDbcUtilCMakeGenerator : public CCodeGeneratorBase
{
public:
    /**
    * @brief Constructor
    * @param[in] rpathOutputDir Reference to the outpur directory.
    * @param[in] rssComponent Current component this CMake file is for.
    */
    CDbcUtilCMakeGenerator(const std::filesystem::path& rpathOutputDir, const std::string& rssComponent);

    /**
    * @brief Destructor
    */
    virtual ~CDbcUtilCMakeGenerator();

private:
    std::filesystem::path       m_pathProject;      ///< Project file path
};

#endif // !defined CMAKE_GENERATOR_H