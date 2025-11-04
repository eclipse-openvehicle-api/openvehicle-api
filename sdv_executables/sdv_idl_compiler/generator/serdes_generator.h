#ifndef SERDES_GENERATOR_H
#define SERDES_GENERATOR_H

#include "definition_generator_base.h"
#include <set>
#include <fstream>

/**
 * @brief Definition stream context.
 */
class CSerdesContext : public CDefEntityContext<CSerdesContext>
{
public:
    /**
     * @brief Constructor assigning the generator context.
     * @param[in] rGenContext Reference to the context to assign.
     * @param[in] pEntity Pointer to the definition entity this context belongs to.
     */
    CSerdesContext(const CGenContext& rGenContext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Copy constructor assigning a new definition entity.
     * @param[in] rcontext Original context to copy from.
     * @param[in] pEntity Pointer to the definition entity this context belongs to.
     * @param[in] rssDeclName Reference to the declaration name to be added to the member scope.
     */
    CSerdesContext(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity, const std::string& rssDeclName = std::string());

    /**
     * @brief Join a context into this context. Overload of CDefEntityContext::operator<<.
     * @param[in] rcontext Reference to the context to join.
     * @return Reference to this context containing the joined result.
     */
    virtual CSerdesContext& operator<<(const CSerdesContext& rcontext) override;

    /**
     * @brief Get the member scoped name when streaming declarations using a container higher in the hierarchy.
     * @return The member scope to use for streaming. Types are separated by a scope-serparator '::' and members are separated by a
     * dot '.' separation character.
     * @param[in] rssDeclName Reference to the string holding the declaration name to use for a full scoped name.
     * @param[in] bFullScope When set, add the container scoped name as well. Otherwise omit the container scoped name.
     */
    std::string ComposeMemberScope(const std::string& rssDeclName, bool bFullScope = false) const;

    /**
     * @brief All following code is part of the serdes namespace.
     */
    void EnableSerDesNamespace();

    /**
     * @brief All following code is not part of the serdes namespace.
     */
    void DisableSerDesNamespace();

    /**
     * @brief Get a reference to the serializer/deserializer code stream.
     * @return Reference to the definition body stream object.
     */
    std::stringstream& GetSerDesCodeStream();

    /**
     * @brief Get serializer/deserializer code.
     * @return Returns a string containing the serializer/deserializer code collected within this context.
     */
    std::string GetSerDesCode() const;

    /**
     * @brief Function part enumeration
     */
    enum class EFuncStreamPart
    {
        header, ///< Function header
        body,   ///< Function body
        footer, ///< Function footer
    };

    /**
     * @brief Get a reference to the size function stream.
     * @param[in] ePart The function part to return.
     * @return Reference to the serializer function stream object.
     */
    std::stringstream& GetSizeFuncStream(EFuncStreamPart ePart = EFuncStreamPart::body);

    /**
     * @brief Get a reference to the serializer function stream.
     * @param[in] ePart The function part to return.
     * @return Reference to the serializer function stream object.
     */
    std::stringstream& GetSerFuncStream(EFuncStreamPart ePart = EFuncStreamPart::body);

    /**
     * @brief Get a reference to the deserializer function stream.
     * @param[in] ePart The function part to return.
     * @return Reference to the deserializer function stream object.
     */
    std::stringstream& GetDesFuncStream(EFuncStreamPart ePart = EFuncStreamPart::body);

    /**
     * @brief Get the size function code.
     * @param[in] ePart The function part to return.
     * @return String with the function code to.
     */
    std::string GetSizeFuncCode(EFuncStreamPart ePart = EFuncStreamPart::body) const;

    /**
     * @brief Get the serializer function code.
     * @param[in] ePart The function part to return.
     * @return String with the function code to.
     */
    std::string GetSerFuncCode(EFuncStreamPart ePart = EFuncStreamPart::body) const;

    /**
     * @brief Get a reference to the deserializer function stream.
     * @param[in] ePart The function part to return.
     * @return String with the function code to.
     */
    std::string GetDesFuncCode(EFuncStreamPart ePart = EFuncStreamPart::body) const;

    /**
     * @brief Join the serialization and deserialization function body content to the body content within this stream.
     * @param[in] rcontext Reference to the context containing of the function bodies.
     * @param[in] bDoNotIncludeNewline When set, do not insert a newline if needed.
     */
    void JoinFuncBodyStreams(const CSerdesContext& rcontext, bool bDoNotIncludeNewline = false);

    /**
     * @brief Stream the class definition with inserted serializer and deserializer functions to the definition stream and clear the
     * serializer and deserializer streams when finished.
     * @param[in] rssClassBegin The class definition part to stream before the serializer functions are streamed.
     * @param[in] rssClassEnd The class definition part to stream after the serializer functions are streamed.
     * @param[in] rmapKeywords Use the map of keywords for keyword replacement within the texts.
     */
    void StreamAndClearSerFuncStreams(const std::string& rssClassBegin, const std::string& rssClassEnd,
        const CGenContext::CKeywordMap& rmapKeywords);

    /**
     * @brief Does the function need variable streaming?
     * @return Returns whether variable streaming is required.
     */
    bool NeedsVariableStreaming() const;

private:
    bool                    m_bSerDesByContainer = false;   ///< When one of the member declarations requires serialization by a
                                                            ///< container that declares this definition, this boolean is set.
    std::stringstream       m_sstreamSerDesCode;            ///< Serializer/deserializer code stream.
    std::stringstream       m_sstreamSizeFuncHdr;           ///< Content of the size function header.
    std::stringstream       m_sstreamSizeFuncFtr;           ///< Content of the size function footer.
    std::stringstream       m_sstreamSizeFunc;              ///< Content of the size function.
    std::stringstream       m_sstreamSerFuncHdr;            ///< Content of the serializer function header.
    std::stringstream       m_sstreamSerFuncFtr;            ///< Content of the serializer function footer.
    std::stringstream       m_sstreamSerFunc;               ///< Content of the serializer function.
    std::stringstream       m_sstreamDesFuncHdr;            ///< Content of the deserializer function header.
    std::stringstream       m_sstreamDesFuncFtr;            ///< Content of the deserializer function footer.
    std::stringstream       m_sstreamDesFunc;               ///< Content of the deserializer function.
    size_t                  m_nTempDeclCnt = 0;             ///< Temporary variable declaration counter. Used to identify generate
                                                            ///< unique temporary variables.
    std::string             m_ssMemberScope;                ///< The member scope to use when streaming declarations.
    bool                    m_bSerDesNamespace = false;     ///< When set, the serdes namespace is enabled.
    bool                    m_bNotStreamable = false;       ///< When set, this definition entity is not streamable directly and
                                                            ///< should be part of the streaming of any container holding the
                                                            ///< declaration of this entity.
};

/**
 * @brief Serializer/deserializer code generator class.
 */
class CSerdesGenerator : public CDefinitionGeneratorBase<CSerdesContext>
{
public:
    /**
     * @brief Constructor
     * @param[in] pParser Pointer to the parser object.
     */
    CSerdesGenerator(sdv::IInterfaceAccess* pParser);

    /**
     * @brief Destructor
     */
    virtual ~CSerdesGenerator() override;

private:
    /**
     * @brief Return the information for target file creation. Overload of CDefinitionGeneratorBase::GetTargetFileInfo.
     * @param[out] rssTargetSubDir Reference to the string containing the target sub-directory to be added to the output directory.
     * Could be empty to target the output directory.
     * @param[out] rssTargetFileEnding Reference to string containing the file ending (file name and extension) to be placed at the
     * end of the source file name replacing the extension.
     */
    virtual void GetTargetFileInfo(std::string& rssTargetSubDir, std::string& rssTargetFileEnding) override;

    /**
     * @brief Return the file header text for automatic file generation. Overload of CDefinitionGeneratorBase::GetFileHeaderText.
     * @return The header text to place into the file.
     */
    virtual std::string GetFileHeaderText() const override;

    /**
     * @brief Stream the code into the file. Called once after processing. Overload of CDefinitionGeneratorBase::StreamIntoFile.
     * @param[in, out] rcontext Reference to the stream context.
     * @param[in, out] rfstream Reference to the file stream to stream into
     */
    virtual void StreamIntoFile(CSerdesContext& rcontext, std::ofstream& rfstream) override;

    /**
     * @brief Stream the include section for the file. Overload of CDefinitionGeneratorBase::StreamIncludeSection.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     */
    virtual void StreamIncludeSection(CSerdesContext& rcontext) override;

    /**
     * @brief Stream the meta entity. Overload of CDefinitionGeneratorBase::StreamMetaEntity.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the interface of the meta entity.
     */
    virtual void StreamMetaEntity(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity) override;

    /**
     * @brief Stream declaration if the entity is a declaration. Overload of CDefinitionGeneratorBase::StreamDeclaration.
     * @param[in, out] rcontext Reference to the stream context.
     * @param[in] pEntity Pointer to the interface of the declaration entity.
     * @return Returns true when the streaming was successful or false when streaming was not successful and should be canceled.
     */
    virtual bool StreamDeclaration(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity) override;

    /**
     * @brief Stream definition if the entity is a definition. Overload of CDefinitionGeneratorBase::StreamDefinition.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @param[in] bInline When set the definition is part of a declaration.
     * @param[in] bAnonymousDecl When set, the definition is part of an anonymous declaration (only valid for unions).
     */
    virtual void StreamDefinition(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity, bool bInline = false,
        bool bAnonymousDecl = false) override;

    /**
     * @brief Stream definition content (the declaration within the definition).
     * @param[in, out] rcontext Reference to the stream context.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @param[in] bSuppressComments When set, do not stream the comment.
     * @return Returns true when the streaming was successful or false when streaming was not successful and should be canceled.
     */
    bool StreamDefinitionContent(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity, bool bSuppressComments = false);

    /**
     * @brief Stream interface definition.
     * @param[in, out] rcontext Reference to the stream context.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     */
    void StreamInterface(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Process the union member that, together with the switch variable, has a mutual container from the entity in the
     * context. Overload of CDefinitionGeneratorBase::ProcessUnionInContainerContext.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] rssMemberScopeUnionDecl Reference to the member scope of the union declaration (could be empty when the union
     * switch is not variable based).
     * @param[in] rssMemberScopeSwitchVar Reference to the member scope of the switch variable (could be empty when the union
     * switch is not variable based).
     * @param[in] pUnionDef Pointer to the union definition entity.
     * @param[in] rvecArrayIndices Reference to the vector containing the array indices of the declaration to use for the detection.
     * If the declaration is not declared as array, the vector should be empty. If the declaration is declared as an array and the
     * amount of dimensions is below the amount of dimensions within the provided vector, the detection function is called for each
     * array dimension extending the vector with the index to do the detection for.
     */
    virtual void ProcessUnionInContainerContext(CSerdesContext& rcontext, std::string rssMemberScopeUnionDecl,
        std::string rssMemberScopeSwitchVar, sdv::IInterfaceAccess* pUnionDef,
        const std::vector<SArrayIterationInfo>& rvecArrayIndices = std::vector<SArrayIterationInfo>()) override;

    /**
    * @brief For a switch variable, process the joint container of both switch variable and union. Start with the highest parent
    * running through all children. Overload of CDefinitionGeneratorBase::ProcessUnionJointContainerForSwitchVar.
    * @param[in, out] rcontext Reference to the definition stream context of the switch variable to stream into.
    * @param[in] pSwitchVarEntity Interface to the switch var declaration.
    * @param[in] pContainerEntity Interface to the container definition.
    */
    virtual void ProcessUnionJointContainerForSwitchVar(CSerdesContext& rcontext,
        sdv::IInterfaceAccess* pSwitchVarEntity, sdv::IInterfaceAccess* pContainerEntity) override;

    std::set<std::string>       m_setHistory;               ///< Set of all the scoped names that have been processed.
    std::set<std::string>       m_setNonStreamableDef;      ///< Set of non-streamable definition entities. Streaming should take
                                                            ///< place in the container implementing the declaration.

    /// Definition entities that cannot be streamed directly (in case of a direct or indirect declaration of a union entity or a
    /// switch variable entity) should be streamed in a container instead. The key in the map represents the definition scoped name.
    /// The value contains a list of container entities (scoped names).
    std::map<std::string, std::list<std::string>>   m_mapInlineDef;
};

#endif // !defined SERDES_GENERATOR_H