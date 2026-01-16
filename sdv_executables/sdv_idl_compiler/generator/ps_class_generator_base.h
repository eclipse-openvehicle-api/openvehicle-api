#ifndef PS_CLASS_GENERATOR_BASE_H
#define PS_CLASS_GENERATOR_BASE_H

#include "context.h"
#include <vector>

/**
 * @brief Proxy/stub class generator base implementation.
 */
class CPSClassGeneratorBase : public CGenContext
{
public:
    /**
     * @brief Constructor
     * @param[in] pParser Pointer to the parser object.
     */
    CPSClassGeneratorBase(sdv::IInterfaceAccess* pParser);

    /**
    * @brief Generate the proxy.
    * @return Returns whether generation was successful.
    */
    virtual bool Generate();

protected:
    /**
     * @brief Return the name addition to be added to the filename and class definition.
     * @return The name appendix string.
     */
    virtual std::string GetNameAppendix() const = 0;

    /**
     * @brief Get definition file comments to be written in the file header.
     * @return String with class definition comments.
     */
    virtual std::string GetClassDefFileComments() const = 0;

    /**
     * @brief Get implementation file comments to be written in the file header.
     * @return String with class implementation comments.
     */
    virtual std::string GetClassImplFileComments() const = 0;

    /**
     * @brief Get begin of class definition to be inserted into the header file.
     * @remarks The following keywords are defined: %class_name% and %interface_name%.
     * @param[inout] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
     * code snippet.
     * @return String with class definition begin.
     */
    virtual std::string GetClassDefBegin(CKeywordMap& rmapKeywords) const = 0;

    /**
     * @brief Get end of class definition to be inserted into the header file.
     * @remarks The following keywords are defined: %class_name% and %interface_name%.
     * @param[inout] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
     * code snippet.
     * @return String with class definition end.
     */
    virtual std::string GetClassDefEnd(CKeywordMap& rmapKeywords) const = 0;

    /**
     * @brief Get begin of constructor implementation to be inserted into the cpp file.
     * @remarks The following keywords are defined: %class_name% and %interface_name%.
     * @param[in, out] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
     * code snippet.
     * @return String with constructor implementation begin.
     */
    virtual std::string GetConstructImplBegin(CKeywordMap& rmapKeywords) const = 0;

    /**
     * @brief Get end of constructor implementation to be inserted into the cpp file.
     * @remarks The following keywords are defined: %class_name% and %interface_name%.
     * @param[inout] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
     * code snippet.
     * @return String with constructor implementation end.
     */
    virtual std::string GetConstructImplEnd(CKeywordMap& rmapKeywords) const = 0;

    /**
     * @brief Function information structure
     */
    struct SFuncInfo
    {
        std::string                     ssName;                 ///< Function name.
        std::string                     ssDecl;                 ///< Return value declaration type (could be void).
        std::string                     ssDeclType;             ///< Return value declaration base type.
        std::string                     ssDefRetValue;          ///< Default return value (or empty if decl type is void).
        bool                            bIsConst = false;       ///< Set when the function is declared as const function.
        size_t                          nInputParamCnt = 0;     ///< Input parameter count.
        size_t                          nOutputParamCnt = 0;    ///< Output parameter count.
    };

    /**
     * @brief Parameter information structure
     */
    struct SParamInfo : SCDeclInfo
    {
        std::string                             ssName;                 ///< Parameter name
        std::string                             ssDefaultValue;         ///< Parameter default value (or empty for void return value)
        std::string                             ssSize;                 ///< Parameter size
        enum class EDirection                                           ///< Parameter direction or return value
        { in, out, inout, ret, ignored } eDirection = EDirection::ignored;   ///< Parameter direction or return value
        enum class EAllocType                                           ///< Parameter allocation type
        { direct, indirect, ifc}         eAllocType = EAllocType::direct;    ///< Parameter allocation type
    };

    /**
     * @brief Get the constructor body for a function (attribute or operation).
     * @remarks The following keywords are defined: %class_name%, %interface_name%, %func_decl_type%,
     * %func_default_ret_value%, %func_name%, %func_index%, %param_pack_def%, %param_pack_use%, %total_param_cnt%, %in_param_cnt%,
     * and %out_param_cnt%.
     * @param[in] rsFunc Reference to the function information structure.
     * @param[inout] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
     * code snippet.
     * @return String with constructor function body.
     */
    virtual std::string GetConstructFuncImpl(const SFuncInfo& rsFunc, CKeywordMap& rmapKeywords) const = 0;

    /**
     * @brief Get the function definition (attribute or operation).
     * @remarks The following keywords are defined: %class_name%, %interface_name%, %func_decl_type%,
     * %func_default_ret_value%, %func_name%, %func_index%, %param_pack_def%, %param_pack_use%, %total_param_cnt%, %in_param_cnt%,
     * and %out_param_cnt%.
     * @remarks The %out_param_cnt% includes the return parameter.
     * @param[in] rsFunc Reference to the function information structure.
     * @param[inout] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
     * code snippet.
     * @return String with the function definition.
     */
    virtual std::string GetFuncDef(const SFuncInfo& rsFunc, CKeywordMap& rmapKeywords) const = 0;

    /**
    * @brief Get the function implementation (attribute or operation).
    * @details The function implementation uses the specific keywords %param_init%, %stream_input_param%, %stream_output_param%,
    * and %param_term% to insert code from the parameter streaming functions.
    * @remarks The following keywords are defined: %class_name%, %interface_name%, %func_decl_type%,
    * %func_default_ret_value%, %func_name%, %func_index%, %param_pack_def%, %param_pack_use%, %total_param_cnt%, %in_param_cnt%,
    * and %out_param_cnt%.
    * @remarks The %out_param_cnt% includes the return parameter.
    * @param[in] rsFunc Reference to the function information structure.
    * @param[inout] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
    * code snippet.
    * @param[in] rvecExceptions Vector containing the exceptions defined for this function.
    * @return String with the function implementation.
    */
    virtual std::string GetFuncImpl(const SFuncInfo& rsFunc, CKeywordMap& rmapKeywords, const CExceptionVector& rvecExceptions) const = 0;

    /**
     * @brief Get parameter initialization of the function implementation (attribute or operation). The content of this function
     * is paced in the keyword %param_init% of the GetFuncImpl function.
     * @remarks The following keywords are defined: %class_name%, %interface_name%, %func_decl_type%,
     * %func_default_ret_value%, %func_name%, %func_index%, %param_pack_def%, %param_pack_use%, %total_param_cnt%, %in_param_cnt%,
     * %out_param_cnt%, %param_name%, %param_decl_type%, %param_index%, %param_default_val%, %param_size% and %param_cnt%.
     * @remarks The %out_param_cnt% includes the return parameter.
     * @remarks The %param_index% is the parameter index incl. optional return value at index 0.
     * @param[in] rsFunc Reference to the function information structure.
     * @param[in] rsParam Reference to a parameter information structure.
     * @param[inout] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
     * code snippet.
     * @return String with the function parameter initialization.
     */
    virtual std::string GetFuncImplParamInit(const SFuncInfo& rsFunc, const SParamInfo& rsParam, CKeywordMap& rmapKeywords) const = 0;

    /**
    * @brief Get input parameter streaming of the function implementation (attribute or operation). The content of this function
    * is paced in the keyword %stream_param_input% of the GetFuncImpl function.
    * @remarks The following keywords are defined: %class_name%, %interface_name%, %func_decl_type%,
    * %func_default_ret_value%, %func_name%, %func_index%, %param_pack_def%, %param_pack_use%, %total_param_cnt%, %in_param_cnt%,
    * %out_param_cnt%, %param_name%, %param_decl_type%, %param_index%, %param_default_val%, %param_size% and %param_cnt%.
    * @remarks The %out_param_cnt% includes the return parameter.
    * @remarks The %param_index% is the parameter index incl. optional return value at index 0.
    * @param[in] rsFunc Reference to the function information structure.
    * @param[in] rsParam Reference to a parameter information structure.
    * @param[inout] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
    * code snippet.
    * @return String with the function input parameters.
    */
    virtual std::string GetFuncImplStreamParamInput(const SFuncInfo& rsFunc, const SParamInfo& rsParam, CKeywordMap& rmapKeywords) const = 0;

    /**
    * @brief Get output parameter streaming of the function implementation (attribute or operation). The content of this function
    * is paced in the keyword %stream_param_output% of the GetFuncImpl function.
    * @remarks The following keywords are defined: %class_name%, %interface_name%, %func_decl_type%,
    * %func_default_ret_value%, %func_name%, %func_index%, %param_pack_def%, %param_pack_use%, %total_param_cnt%, %in_param_cnt%,
    * %out_param_cnt%, %param_name%, %param_decl_type%, %param_index%, %param_default_val%, %param_size% and %param_cnt%.
    * @remarks The %out_param_cnt% includes the return parameter.
    * @remarks The %param_index% is the parameter index incl. optional return value at index 0.
    * @param[in] rsFunc Reference to the function information structure.
    * @param[in] rsParam Reference to a parameter information structure.
    * @param[inout] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
    * code snippet.
    * @return String with the function output parameters.
    */
    virtual std::string GetFuncImplStreamParamOutput(const SFuncInfo& rsFunc, const SParamInfo& rsParam, CKeywordMap& rmapKeywords) const = 0;

    /**
    * @brief Get parameter termination of the function implementation (attribute or operation). The content of this function
    * is paced in the keyword %param_term% of the GetFuncImpl function.
    * @remarks The following keywords are defined: %class_name%, %interface_name%, %func_decl_type%,
    * %func_default_ret_value%, %func_name%, %func_index%, %param_pack_def%, %param_pack_use%, %total_param_cnt%, %in_param_cnt%,
    * %out_param_cnt%, %param_name%, %param_decl_type%, %param_index%, %param_default_val%, %param_size% and %param_cnt%.
    * @remarks The %out_param_cnt% includes the return parameter.
    * @remarks The %param_index% is the parameter index incl. optional return value at index 0.
    * @param[in] rsFunc Reference to the function information structure.
    * @param[in] rsParam Reference to a parameter information structure.
    * @param[inout] rmapKeywords Reference to the keyword map. This allows inserting additional keywords local for the returned
    * code snippet.
    * @return String with the function parameter termination.
    */
    virtual std::string GetFuncImplParamTerm(const SFuncInfo& rsFunc, const SParamInfo& rsParam, CKeywordMap& rmapKeywords) const = 0;

private:
    /**
     * @brief Process the entities.
     * @param[in] rstreamHdr Reference to the stream of the target header file.
     * @param[in] rstreamCpp Reference to the stream of the target C++ file.
     * @param[in] pIterator Pointer to the iterator interface.
     */
    void ProcessEntities(std::ostream& rstreamHdr, std::ostream& rstreamCpp, sdv::idl::IEntityIterator* pIterator);

    /**
     * @brief Stream definition and implementation if the entity is an interface definition.
     * @param[in] rstreamHdr Reference to the stream of the target header file.
     * @param[in] rstreamCpp Reference to the stream of the target C++ file.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     */
    void StreamInterface(std::ostream& rstreamHdr, std::ostream& rstreamCpp, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Stream attributes and operations of an interface definition and all derived interface definitions.
     * @param[in] rstreamClassDef Reference to the class definition stream.
     * @param[in] rstreamConstrBody Reference to the constructor body stream.
     * @param[in] rstreamClassImpl Reference to the class implementation stream.
     * @param[in] rmapKeywords Map with keywords to replace. Keyword "class" and "interface" are defined.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @param[in, out] ruiFuncCnt Reference to the function counter used to set the index. Will be updated.
     */
    void StreamInterfaceContent(std::ostream& rstreamClassDef, std::ostream& rstreamConstrBody, std::ostream& rstreamClassImpl,
        const CKeywordMap& rmapKeywords, sdv::IInterfaceAccess* pEntity, uint32_t& ruiFuncCnt);

    /**
     * @brief Stream attribute declaration if the entity is an attribute.
     * attention Comments are not streamed for parameters.
     * @param[in] rstreamClassDef Reference to the class definition stream.
     * @param[in] rstreamConstrBody Reference to the constructor body stream.
     * @param[in] rstreamClassImpl Reference to the class implementation stream.
     * @param[in] rmapKeywords Map with keywords to replace. Keyword "class" and "interface" are defined.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @param[in, out] ruiFuncCnt Reference to the function counter used to set the index. Will be updated.
     */
    void StreamAttribute(std::ostream& rstreamClassDef, std::ostream& rstreamConstrBody, std::ostream& rstreamClassImpl,
        const CKeywordMap& rmapKeywords, sdv::IInterfaceAccess* pEntity, uint32_t& ruiFuncCnt);

    /**
     * @brief Stream operation declaration if the entity is an operation.
     * @param[in] rstreamClassDef Reference to the class definition stream.
     * @param[in] rstreamConstrBody Reference to the constructor body stream.
     * @param[in] rstreamClassImpl Reference to the class implementation stream.
     * @param[in] rmapKeywords Map with keywords to replace. Keyword "class" and "interface" are defined.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @param[in, out] ruiFuncCnt Reference to the function counter used to set the index. Will be updated.
     */
    void StreamOperation(std::ostream& rstreamClassDef, std::ostream& rstreamConstrBody, std::ostream& rstreamClassImpl,
        const CKeywordMap& rmapKeywords, sdv::IInterfaceAccess* pEntity, uint32_t& ruiFuncCnt);

    /**
     * @brief Stream function declaration (for attributes and operations).
     * @details Functions are streamed in this order:
     * - Constructor function impl
     * - Function definition
     * - Function impl begin
     * - For each parameter, param init impl
     * - Pass #1 impl begin
     * - For each parameter, pass #1 impl
     * - Pass #1 impl end
     * - Pass #2 impl begin
     * - For each parameter, pass #2 impl
     * - Pass #2 impl end
     * - Function impl end
     * @param[in] rstreamClassDef Reference to the class definition stream.
     * @param[in] rstreamConstrBody Reference to the constructor body stream.
     * @param[in] rstreamClassImpl Reference to the class implementation stream.
     * @param[in] rmapKeywords Map with keywords to replace. Additional keyword "func_name" is defined.
     * @param[in] pRetParam Pointer to the IInterfaceAccess pointer of the entity holding the return parameter.
     * @param[in, out] ruiFuncCnt Reference to the function counter used to set the index. Will be updated.
     * @param[in] bConst When set, the function is marked as a const-function.
     * @param[in] rvecParams Reference to the vector containing the parameter entities defined for this function.
     * @param[in] rvecExceptions Reference to the vector containing the exceptions defined for this function.
     */
    void StreamFunction(std::ostream& rstreamClassDef, std::ostream& rstreamConstrBody, std::ostream& rstreamClassImpl,
        const CKeywordMap& rmapKeywords, sdv::IInterfaceAccess* pRetParam, uint32_t& ruiFuncCnt, bool bConst,
        const std::vector<sdv::IInterfaceAccess*>& rvecParams, const CExceptionVector& rvecExceptions);

    /**
     * @brief Get parameter information.
     * @param[in] pParam Entity representing the parameter (not necessarily a parameter entity). If NULL, expecting it to be a
     * "void" return value.
     * @param[in] bIsRetValue When set, the parameter is marked as return value.
     * @return The parameter information structure.
    */
    SParamInfo GetParamInfo(sdv::IInterfaceAccess* pParam, bool bIsRetValue = false) const;

    /**
     * @brief Stream SMarshall declaration.
     * @param[in] rstream Reference to the stream of the target C++ file.
     * @param[in] rmapKeywords Map with keywords to replace.
     * @param[in] uiFuncIndex Index of the function within the interface.
     * @param[in] uiParamCnt Parameter count.
     */
    void StreamMarshallDecl(std::ofstream& rstream, const CKeywordMap& rmapKeywords, uint32_t uiFuncIndex, uint32_t uiParamCnt);

    /**
     * @brief Does the provided entity or any contained entities data types where the interpretation can only be done during
     * runtime?
     * @details Data types that require runtime processing are dynamic arrays (arrays where the size is determined by another
     * entity), interfaces, strings and sequences.
     * @param[in] pEntity Pointer to the entity to check for runtime processing requirements.
     * @return Returns whether runtime processing is required.
     */
    static bool RuntimeProcessingRequired(sdv::IInterfaceAccess* pEntity);


};


#endif // !defined(PS_CLASS_GENERATOR_BASE_H)
