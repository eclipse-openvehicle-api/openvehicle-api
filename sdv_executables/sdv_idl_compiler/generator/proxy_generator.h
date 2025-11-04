#ifndef PROXY_GENERATOR_H
#define PROXY_GENERATOR_H

#include "ps_class_generator_base.h"

/**
 * @brief Proxy generator class.
 */
class CProxyGenerator : public CPSClassGeneratorBase
{
public:
    /**
     * @brief Constructor
     * @param[in] pParser Pointer to the parser object.
     */
    CProxyGenerator(sdv::IInterfaceAccess* pParser);

    /**
     * @brief Destructor
     */
    virtual ~CProxyGenerator() override;

private:
    /**
     * @brief Return the name addition to be added to the filename and class definition. Overload of
     * CPSClassGeneratorBase::GetNameAppendix.
     */
    virtual std::string GetNameAppendix() const override;

    /**
     * @brief Get definition file comments to be written in the file header. Overload of
     * CPSClassGeneratorBase::GetClassDefFileComments.
     */
    virtual std::string GetClassDefFileComments() const override;

    /**
     * @brief Get implementation file comments to be written in the file header. Overload of
     * CPSClassGeneratorBase::GetClassImplFileComments.
     */
    virtual std::string GetClassImplFileComments() const override;

    /**
     * @brief Get begin of class definition to be inserted into the header file. Overload of
     * CPSClassGeneratorBase::GetClassDefBegin.
     */
    virtual std::string GetClassDefBegin(CKeywordMap& rmapKeywords) const override;

    /**
     * @brief Get end of class definition to be inserted into the header file. Overload of
     * CPSClassGeneratorBase::GetClassDefEnd.
     */
    virtual std::string GetClassDefEnd(CKeywordMap& rmapKeywords) const override;

    /**
     * @brief Get begin of constructor implementation to be inserted into the cpp file. Overload of
     * CPSClassGeneratorBase::GetConstructImplBegin.
     */
    virtual std::string GetConstructImplBegin(CKeywordMap& rmapKeywords) const override;

    /**
     * @brief Get end of constructor implementation to be inserted into the cpp file. Overload of
     * CPSClassGeneratorBase::GetConstructImplEnd.
     */
    virtual std::string GetConstructImplEnd(CKeywordMap& rmapKeywords) const override;

    /**
     * @brief Get the constructor body for a function (attribute or operation). Overload of
     * CPSClassGeneratorBase::GetConstructFuncImpl.
     */
    virtual std::string GetConstructFuncImpl(const SFuncInfo& rsFunc, CKeywordMap& rmapKeywords) const override;

    /**
     * @brief Get the function definition (attribute or operation). Overload of CPSClassGeneratorBase::GetFuncDef.
     */
    virtual std::string GetFuncDef(const SFuncInfo& rsFunc, CKeywordMap& rmapKeywords) const override;

    /**
    * @brief Get the function implementation (attribute or operation). Overload of
    * CPSClassGeneratorBase::GetFuncImpl.
    */
    virtual std::string GetFuncImpl(const SFuncInfo& rsFunc, CKeywordMap& rmapKeywords, const CExceptionVector& rvecExceptions) const override;

    /**
    * @brief Get parameter initialization of the function implementation (attribute or operation). Overload of
    * CPSClassGeneratorBase::GetFuncImplParamInit.
    */
    virtual std::string GetFuncImplParamInit(const SFuncInfo& rsFunc, const SParamInfo& rsParam, CKeywordMap& rmapKeywords) const override;

    /**
    * @brief Get input parameter streaming of the function implementation (attribute or operation). Overload of
    * CPSClassGeneratorBase::GetFuncImplStreamParamInput.
    */
    virtual std::string GetFuncImplStreamParamInput(const SFuncInfo& rsFunc, const SParamInfo& rsParam, CKeywordMap& rmapKeywords) const override;

    /**
    * @brief Get output parameter streaming of the function implementation (attribute or operation). Overload of
    * CPSClassGeneratorBase::GetFuncImplStreamParamOutput.
    */
    virtual std::string GetFuncImplStreamParamOutput(const SFuncInfo& rsFunc, const SParamInfo& rsParam, CKeywordMap& rmapKeywords) const override;

    /**
    * @brief Get parameter termination of the unpack portion of the function implementation (attribute or operation). Overload of
    * CPSClassGeneratorBase::GetFuncImplParamTerm.
    */
    virtual std::string GetFuncImplParamTerm(const SFuncInfo& rsFunc, const SParamInfo& rsParam, CKeywordMap& rmapKeywords) const override;
};

#endif // !defined(PROXY_GENERATOR_H)