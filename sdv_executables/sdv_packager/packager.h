#ifndef PACKAGER_H
#define PACKAGER_H

#include "environment.h"

/**
 * @brief Packager class.
*/
class CPackager
{
public:
    /**
     * @brief Constructor
     * @param[in] renv Reference to the environment object.
     */
    CPackager(CSdvPackagerEnvironment& renv);

    /**
     * @brief Execute the packager dependable on the environment settings.
     * @return Returns whether the execution was successful.
     */
    bool Execute();

    /**
     * @brief Was a commandline parse error or a logical error with command line arguments detected?
     * @return Returns the result of the command line parsing. If no error occurs, NO_ERROR is returned.
     */
    int Error() const;

    /**
     * @brief Parse error that might have occurred.
     * @return Returns a reference to the member variable containing the error text.
     */
    const std::string& ArgError() const;

private:
    /**
     * @brief Pack files into a package. Uses the environment settings to create the package.
     * @return Returns whether the packing operation was successful.
     */
    bool Pack();

    /**
     * @brief Unpack an installation package into the target location. Uses the environment settings to unpack the package.
     * @return Returns whether the packing opereration was successful.
     */
    bool Unpack();

    /**
     * @brief Copy files from the source location to the target location (direct installation). Uses the environment settings to
     * copy the files.
     * @return Returns whether the copy operation was successful.
     */
    bool Copy();

    /**
     * @brief Remove an installation from the target location. Uses the environment settings to remove the files.
     * @return Returns whether the removal operation was successful.
     */
    bool Remove();

    /**
     * @brief Check package integrity. Uses the environment settings to check the package.
     * @return Returns whether the integrity operation was successful.
     */
    bool CheckIntegrity();

    /**
     * @brief Show package content. Uses the environment settings to show the package content.
     * @return Returns whether the show operation was successful.
     */
    bool ShowContent();
    
    CSdvPackagerEnvironment                m_env;                              ///< The packager environment
    int                         m_nError = NO_ERROR;                ///< Error code after processing the command line.
    std::string                 m_ssArgError;                       ///< Error text after processing the command line.
};


#endif // !defined PACKAGER_H