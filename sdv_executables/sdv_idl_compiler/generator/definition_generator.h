#ifndef DEFINITION_GENERATOR_H
#define DEFINITION_GENERATOR_H

#include "definition_generator_base.h"
#include <list>
#include <fstream>
#include <set>
#include <sstream>

// TODO: Comment style overwrite: all javadoc, QT, C, Cpp, preceding, succeeding
// TODO: Tabs or spaces
// TODO: Tab size (default 4 characters)
// TODO: Succeeding comments

/**
 * @brief Switch code context specifically for definition creation.
 */
struct SDefinitionSwitchCodeContext : SSwitchCodeContext
{
    std::stringstream   sstreamCode;                    ///< Code used to the actual switching
    std::stringstream   sstreamConstructorImpl;         ///< Constructor content stream. Not applicable if the definition is a
                                                        ///< namespace.
    std::stringstream   sstreamDestructorImpl;          ///< Destructor content stream. Not applicable if the definition is a namespace.
    std::stringstream   sstreamConstructHelperImpl;     ///< Constructor helper function for this switch variable.
    std::stringstream   sstreamCopyConstructHelperImpl; ///< Constructor content stream for copy construction. Not applicable if the
                                                        ///< definition is a namespace.
    std::stringstream   sstreamMoveConstructHelperImpl; ///< Constructor content stream for move construction. Not applicable if the
                                                        ///< definition is a namespace.
    std::stringstream   sstreamDestructHelperImpl;      ///< Constructor helper function for this switch variable.
};

/**
 * @brief Definition stream context.
 */
struct CDefinitionContext : CDefEntityContext<CDefinitionContext>
{
    /**
     * @brief Constructor assigning the generator context.
     * @param[in] rGenContext Reference to the context to assign.
     * @param[in] pEntity Pointer to the definition entity this context belongs to.
     */
    CDefinitionContext(const CGenContext& rGenContext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Copy constructor assigning a new definition entity.
     * @param[in] rcontext Original context to copy from.
     * @param[in] pEntity Pointer to the definition entity this context belongs to.
     */
    CDefinitionContext(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Join a context into this context. Overload of CDefEntityContext::operator<<.
     * @param[in] rcontext Reference to the context to join.
     * @return Reference to this context containing the joined result.
     */
    virtual CDefinitionContext& operator<<(const CDefinitionContext& rcontext) override;

    /**
     * @brief Set the definition access to public (default).
     */
    void SetDefAccessPublic();

    /**
     * @brief Set the definition access to private..
     */
    void SetDefAccessPrivate();

    /**
     * @brief Get a reference to the preface stream.
     * @return Reference to the preface stream object.
     */
    std::stringstream& GetPrefaceStream();

    /**
     * @brief Get a reference to the definition code stream.
     * @return Reference to the definition code stream object.
     */
    std::stringstream& GetDefCodeStream();

    /**
     * @brief Get a reference to the preface or definition code stream dependable on the preface switch.
     * @return Reference to the preface or definition code stream object.
     */
    std::stringstream& GetAutoStream();

    /**
     * @brief Get definition code (this adds both prefacce and definition code stream content).
     * @return Returns a string containing the definition code collected within this context.
     */
    std::string GetDefinitionCode() const;

    /**
     * @brief Returns whether the preface switch is still activated.
     * @return The current state of the preface switch for streaming.
     */
    bool UsePreface() const;

    /**
     * @brief Disable the preface switch.
     */
    void DisablePreface();

    /**
     * @brief Is construction needed?
     * @return Returns whether construction is needed.
     */
    bool NeedsConstruction() const;

    /**
     * @brief Set the construction needed flag.
     */
    void SetConstructionNeeded();

    /**
     * @brief Newline-after-content-flag set?
     * @return Returns whether a newline after the definition content is required.
     */
    bool NeedsNewlineAfterContent() const;

    /**
     * @brief Set the newline-after-content-flag.
     */
    void EnableNewlineAfterContent();

    /**
     * @brief Reset the newline-after-content-flag.
     */
    void DisableNewlineAfterContent();

    /**
     * @brief Dies this entity have any friends?
     * @return Returns whether this entity has any friends in the friend set.
     */
    bool HasFriends() const;

    /**
     * @brief Get the set of friends.
     * @return Returns the reference to the set of friends.
     */
    const std::set<std::string>& GetFriendSet() const;

    /**
     * @brief Assign a the friend to this entity.
     * @param[in] rssScopedName Reference to the scoped name of the friend entity.
     */
    void AddFriend(const std::string& rssScopedName);

private:
    std::stringstream       m_sstreamPreface;                           ///< Preface stream (before the first code).
    std::stringstream       m_sstreamDefCode;                           ///< Definition code stream.
    bool                    m_bPreface = false;                         ///< When set, streaming is done in the preface stream
                                                                        ///< instead of the definition code stream.
    bool                    m_bConstructionCompulsory = false;          ///< Constructor needed even if no content is available
                                                                        ///< (needed wen objects need to be initialized with default
                                                                        ///< initialization). Not applicable if the definition is a
                                                                        ///< namespace (should not occur).
    bool                    m_bCurrentDefAccessPublic = true;           ///< When set, the current definition access is public;
                                                                        ///< private when not set.
    bool                    m_bNewlineAfterContent = false;             ///< The next content that is streamed, should insert a
                                                                        ///< newline.
    sdv::IInterfaceAccess*  m_pDefEntity = nullptr;                     ///< The definition entity that defines this context.
    std::set<std::string>   m_setFriends;                               ///< Friend structures needed to allow access to the
                                                                        ///< private member.
};

/**
 * @brief Definition generator class.
 */
class CDefinitionGenerator : public CDefinitionGeneratorBase<CDefinitionContext>
{
public:
    /**
     * @brief Constructor
     * @param[in] pParser Pointer to the parser object.
     */
    CDefinitionGenerator(sdv::IInterfaceAccess* pParser);

    /**
     * @brief Destructor
     */
    virtual ~CDefinitionGenerator() override;

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
    virtual void StreamIntoFile(CDefinitionContext& rcontext, std::ofstream& rfstream) override;

    /**
     * @brief Stream the include section for the file. Overload of CDefinitionGeneratorBase::StreamIncludeSection.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     */
    virtual void StreamIncludeSection(CDefinitionContext& rcontext) override;

    /**
     * @brief Stream the meta entity. Overload of CDefinitionGeneratorBase::StreamMetaEntity.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the interface of the meta entity.
     */
    virtual void StreamMetaEntity(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity) override;

    /**
     * @brief Compound comment enumerator
     */
    enum class ECommentGroup { none, begin, end };

    /**
     * @brief Stream preceding comments if there are any.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @param[in] eGroup Defines whether the comment groups several statements or comments a single statement.
     */
    void StreamComments(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity,
        ECommentGroup eGroup = ECommentGroup::none);

    /**
     * @brief Stream declaration if the entity is a declaration. Overload of CDefinitionGeneratorBase::StreamDeclaration.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @return Returns true when the streaming was successful or false when streaming was not successful and should be canceled.
     */
    virtual bool StreamDeclaration(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity) override;

    /**
     * @brief Stream definition if the entity is a definition. Overload of CDefinitionGeneratorBase::StreamDefinition.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @param[in] bInline When set the definition is part of a declaration.
     * @param[in] bAnonymousDecl When set, the definition is part of an anonymous declaration (only valid for unions).
     */
    virtual void StreamDefinition(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity, bool bInline = false,
        bool bAnonymousDecl = false) override;

    /**
     * @brief Stream typedef declaration if the entity is a typedef.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     */
    void StreamTypedef(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Stream attribute declaration if the entity is an attribute.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @details Attributes are implemented as getter- and setter-functions.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     */
    void StreamAttribute(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Stream operation declaration if the entity is an operation.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     */
    void StreamOperation(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Stream parameter declaration if the entity is a parameter.
     * @details Stream parameter declarations. Input parameters are provided by-value unless the parameter is a complex parameter
     * (a struct, union, string, pointer, sequence, map or an array); they are provided by-reference (const). Interfaces are
     * provided by C/C++ pointer. Output parameters are always provided by reference. Pointers are defined in IDL as boundless
     * arrays (var[]) and are implemented using a smart-pointer class (hence a complex parameter). Fixed-bound arrays are provided
     * as C-arrays. Dynamic-bound arrays (arrays that have alength defined through a variable) are available only when the length
     * is provided as an input parameter and the variable itself is allocated on input (hence either an input parameter or an in-
     * and output parameter). In case dynamic bound arrays are required as output parameter, use a sequence instead.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @param[in] bInitial When set, this is the first parameter of an operation.
     */
    void StreamParameter(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity, bool bInitial);

    /**
     * @brief Stream enum entry declaration if the entity is an enum entry.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     */
    void StreamEnumEntry(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Stream case entry declaration if the entity is an case entry.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     */
    void StreamCaseEntry(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Stream the declaration type string.
     * details The stream is created inline. In case the declaration contains an anonymous definition, the definition is inserted
     * as well (hence the use of indentation).
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the declaration entity.
     * @param[in] rbDefinitionStreamed When set, a definition was streamed and a newline should be inserted before the next
     * declaration.
     * @param[in] bAnonymousDecl When set, the definition is part of an anonymous declaration (only valid for unions).
     * @param[in] bSkipInitialIndent When set, do not insert an additional indentation before the declaration of the type.
     * @return Returns whether the declaration type was streamed or whether the type was unknown and could not be streamed.
     */
    bool StreamDeclType(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity, bool& rbDefinitionStreamed,
        bool bAnonymousDecl = false, bool bSkipInitialIndent = true);

    /**
     * @brief For a switch variable, process the joint container of both switch variable and union. Start with the highest parent
     * running through all children. Overload of CDefinitionGeneratorBase::ProcessUnionJointContainerForSwitchVar.
     * @param[in, out] rcontext Reference to the definition stream context of the switch variable to stream into.
     * @param[in] pSwitchVarEntity Interface to the switch var declaration.
     * @param[in] pContainerEntity Interface to the container definition.
     */
    virtual void ProcessUnionJointContainerForSwitchVar(CDefinitionContext& rcontext,
        sdv::IInterfaceAccess* pSwitchVarEntity, sdv::IInterfaceAccess* pContainerEntity) override;

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
    virtual void ProcessUnionInContainerContext(CDefinitionContext& rcontext, std::string rssMemberScopeUnionDecl,
        std::string rssMemberScopeSwitchVar, sdv::IInterfaceAccess* pUnionDef,
        const std::vector<SArrayIterationInfo>& rvecArrayIndices = std::vector<SArrayIterationInfo>()) override;

    std::set<std::string>   m_setHistory;                   ///< Set with a history of all added entity definitions.
    std::set<std::string>   m_setForwardDecl;               ///< Set with with forward declared structure definitions.
};

#endif // !defined(DEFINITION_GENERATOR_H)
