#ifndef DEFINITION_GENERATOR_BASE_H
#define DEFINITION_GENERATOR_BASE_H

#include "context.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <list>

/**
 * @brief Multiple unions can be switched using one switch variable. The switch functions are implemented in the struct or
 * exception containing the switch variable. This struct collects the switch information globally to allow the variable to be
 * used in a switch case even if the union is several layers deep in the child hierarchy.
 */
struct SSwitchVarContext
{
    sdv::IInterfaceAccess*      pVarEntity = nullptr;       ///< The switch variable entity pointer.
    std::string                 ssType;                     ///< Switch var type
    std::string                 ssName;                     ///< Switch var name
    std::string                 ssScopedName;               ///< Scoped name of the switch
    std::vector<std::string>    vecUnionDecl;               ///< Vector containing the union names to switch for.
};

/**
 * @brief Array iteration information contains the variable that is used for iteration and the array dimension expression to
 * determine the upper boundary.
 */
struct SArrayIterationInfo
{
    std::string     ssArrayIterator;        ///< The name of the variable used for the array iteration.
    std::string     ssCountExpression;      ///< The expression used to identify the maximum array elements.
};

/**
 * @brief Switch code context to be processed in the joined parent of both the union with variable based switch and the
 * switch variable.
 */
struct SSwitchCodeContext
{
    std::shared_ptr<SSwitchVarContext>  ptrSwitchVar;               ///< The switch variable context
    std::string                         ssSwitchVarName;            ///< Exact statement of the switch var as scoped member
                                                                    ///< relative to the joint container of both union and switch
                                                                    ///< variable.
    std::string                         ssSwitchValue;              ///< The first switch value (if available).
    std::vector<SArrayIterationInfo>    vecArrayIterationInfo;      ///< The iteration information about any array of
                                                                    ///< declarations for the structures holding unions or for
                                                                    ///< unions themselves.
};

/**
 * @brief Definition stream context.
 * @tparam TDerivedContext The derived context class. Must be deriving from this class.
 */
template <typename TDerivedContext>
class CDefEntityContext
{
public:
    // Forward declaration
    struct SIterator;

    /// Allow the iterator structure to access member variables.
    friend SIterator;

    /**
     * @brief Constructor assigning the generator context.
     * @param[in] rContext Reference to the context to assign.
     * @param[in] pEntity Pointer to the definition entity this context belongs to.
     */
    CDefEntityContext(const CGenContext& rContext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Copy constructor assigning a new definition entity.
     * @param[in] rcontext Original context to copy from.
     * @param[in] pEntity Pointer to the definition entity this context belongs to.
     */
    CDefEntityContext(CDefEntityContext& rcontext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Virtual destructor allowing the destruction of derived class members as well.
     */
    virtual ~CDefEntityContext();

    /**
     * @brief Join a context into this context.
     * @param[in] rContext Reference to the context to join.
     * @return Reference to this context containing the joined result.
     */
    virtual TDerivedContext& operator<<(const TDerivedContext& rContext);

    /**
     * @brief Return the scope of the members of the definition entity (which is the scoped name of the definition).
     * @return The scope if existing or empty if not.
     */
    std::string GetScope() const;

    /**
     * @brief Current scope part of compound structure?
     * @return Returns whether the current structure is part of a compound structure.
     */
    bool IsCompound() const;

    /**
     * @brief Is this definition structural (struct, exception, union)?
     * @return Returns whether the definition is structural.
     */
    bool IsStructural() const;

    /**
     * @brief Get the stored indentation.
     * @param[in] bDefBody When set, the indentation is for the definition body, keeping the deep indentation flag in consideration.
     * @param[in] bFuncImpl When set, the indentation is for the function implementation, increasing the indentation.
     * @return The current indentation.
     */
    std::string GetIndent(bool bDefBody = true, bool bFuncImpl = false) const;

    /**
     * @brief Enable (if not enabled) and increase the indentation.
     */
    void IncrIndent();

    /**
     * @brief Enable (if not enabled) and decrease the indentation (if possible).
     */
    void DecrIndent();

    /**
     * @brief Disable the indentation if enabled.
     */
    void DisableIndent();

    /**
     * @brief Enable the indentation if disabled.
     */
    void EnableIndent();

    /**
     * @brief Enable deep indentation (one more indentation in definition part).
     */
    void EnableDeepIndent();

    /**
     * @brief Get the interface to the definition entity.
     * @return The interface to the definition or NULL when the context is representing the root definition.
     */
    sdv::IInterfaceAccess* GetDefEntity() const;

    /**
     * @brief Templated function for getting an interface to the definition entity.
     * @tparam TInterface The interface to get.
     * @return Pointer to the interface or NULL when the definition is not available or doesn#t expose the interface.
     */
    template <typename TInterface>
    TInterface* GetDefEntity() const;

    /**
     * @brief Iterator structure (used for entity iteration).
     */
    struct SIterator
    {
        /// The context class is allowed to access content directly.
        friend CDefEntityContext<TDerivedContext>;

    private:
        /**
         * @brief Constructor
         * @param[in] rContextParam Reference to the entity context that holds the iterator list.
         */
        SIterator(CDefEntityContext<TDerivedContext>& rContextParam);

    public:

        /**
         * @brief No copy constructor
         * @param[in] rsIterator Reference to the itertor to copy from.
         */
        SIterator(const SIterator& rsIterator) = delete;

        /**
         * @brief Move constructor
         * @param[in] rsIterator Reference to the itertor to move from.
         */
        SIterator(SIterator&& rsIterator);

        /**
         * @brief Destructor
         */
        ~SIterator();

        /**
         * @brief Releases the iterator.
        */
        void Release();

        /**
         * @brief Increase the iteration value by 1.
         * @return Returns reference to the iterator object after incrementation.
         */
        SIterator& operator++();

        /**
         * @brief Increase the iteration value by 1.
         * @param[in] iVal Value is ignored.
         * @return Returns the iteration value before incrementation.
         */
        uint32_t operator++(int iVal);

        /**
         * @brief Get the current iteration value.
         * @return The iteration index.
         */
        operator uint32_t() const;

    private:
        CDefEntityContext<TDerivedContext>&     rContext;       ///< Reference to the entity context holding the iterator list.
        bool                                    bValid = false; ///< The iterator is only valid when set.
        typename std::list<uint32_t>::iterator  itPos{};        ///< Iterator position in the context iteration object list.
    };

    /**
     * @brief Create a new iterator object and make it current.
     * @remarks The iterator object is deleted automatically when out of scope or when explicitly triggered to release the iterator.
     * @return The iterator object.
    */
    SIterator CreateIterator();

    /**
     * @brief Get the iteration value of the current iterator.
     * @return The value of the current iteration or 0 when no iterator was created.
     */
    uint32_t GetCurrentIteration();

    /**
     * @brief Assign a switch variable context to the definition containing the switch variable.
     * @param[in] rptrSwitchVarContext Reference to the smart pointer holding the context structure for the switch variable.
     */
    void AssignSwitchVarContext(const std::shared_ptr<SSwitchVarContext>& rptrSwitchVarContext);

    /**
     * @brief Create or get a switch code context for the switch variable supplied.
     * @tparam TSwitchCodeContext Type of switch code context structure. Must derive from SSwitchCodeContext.
     * @param[in] rssSwitchVarName Reference to the scoped member name of the switch variable including array brackets.
     * @param[in] rptrSwitchVar Reference to the smart pointer holding the switch variable context.
     * @param[in] rvecArrayIndices Reference to the iterator information of any arrays being part of the switch variable name.
     * @return Returns a shared pointer to an existing or a new switch code context.
     */
    template <typename TSwitchCodeContext = SSwitchCodeContext>
    std::shared_ptr<TSwitchCodeContext> GetOrCreateSwitchCodeContext(const std::string& rssSwitchVarName,
        const std::shared_ptr<SSwitchVarContext>& rptrSwitchVar, const std::vector<SArrayIterationInfo>& rvecArrayIndices);

    /**
     * @brief Are multiple switch code contexts available?
     * @return Returns whether more than one switch code contexts are available.
     */
    bool HasMultipleSwitchCodeContexts() const;

    /**
     * @brief Get a vector with all the switch code contexts.
     * @tparam TSwitchCodeContext Type of switch code context structure. Must derive from SSwitchCodeContext.
     * @param[in] rssScopedSwitchVar Reference to the string containing the scoped variable name that should be used as a filter. If
     * not set, all contexts are returned.
     * @return Returns a vector with the stored switch code contexts.
     */
    template <typename TSwitchCodeContext = SSwitchCodeContext>
    std::vector<std::shared_ptr<TSwitchCodeContext>> GetSwitchCodeContexts(
        const std::string& rssScopedSwitchVar = std::string()) const;

private:
    /**
     * @brief Create an iterator object and return the current position to the object.
     * @return The iterator position in the iterator list.
     */
    std::list<uint32_t>::iterator CreateIteratorObject();

    /**
     * @brief Remove the iterator object from the iterator list.
     * @remarks This function is not protected by providing faulty positions.
     * @param[in] itPos Iterator position in the iterator list.
     */
    void RemoveIteratorObject(std::list<uint32_t>::iterator itPos);

    const CGenContext&      m_rGenContext;              ///< Reference to te generator context.
    sdv::IInterfaceAccess*  m_pDefEntity = nullptr;     ///< The definition entity that defines this context.
    std::string             m_ssIndent;                 ///< Current indentation for definition body (needs to be indented once more
                                                        ///< for constructor and destructor implementations).
    std::string             m_ssIndentBackup;           ///< Stored indent for disabled indentation support.
    bool                    m_bDeepDefIndent = false;   ///< When set, the definitions are one level deeper than the provided
                                                        ///< indentation. This doesn't count for the functions. The boolean is
                                                        ///< needed when the definition is implemented using two levels of
                                                        ///< definitions, as is the case with unions using a type based switch.
    uint32_t                m_uiItIdx = 0;              ///< Current iteration index during iteration.

    /// Contained switch var contexts in this definition.
    std::vector<std::shared_ptr<SSwitchVarContext>> m_vecSwitchVars;

    /// Switch code implemented in this definition entity.
    using TSwitchCodeMap = std::map<std::string, std::shared_ptr<SSwitchCodeContext>>;

    /// Shareable switch code map.
    std::shared_ptr<TSwitchCodeMap> m_ptrSwitchCodeMap;

    /// List with iterators. The bottom iterator (latest added) is the current iterator.
    std::list<uint32_t> m_lstIterators;
};

/**
 * @brief Definition code generator base.
 * @tparam TDefEntityContext Type of definition entity context to use. Must derive from CDefEntityContext.
 */
template <typename TDefEntityContext>
class CDefinitionGeneratorBase : public CGenContext
{
    static_assert(std::is_base_of_v<CDefEntityContext<TDefEntityContext>, TDefEntityContext>);
public:
    /**
     * @brief Constructor
     * @param[in] pParser Pointer to the parser object.
     */
    CDefinitionGeneratorBase(sdv::IInterfaceAccess* pParser);

    /**
     * @brief Generate the definition code.
     * @return Returns whether generation was successful.
     */
    virtual bool Generate();

protected:
    /**
     * @brief Return the information for target file creation.
     * @param[out] rssTargetSubDir Reference to the string containing the target sub-directory to be added to the output directory.
     * Could be empty to target the output directory.
     * @param[out] rssTargetFileEnding Reference to string containing the file ending (file name and extension) to be placed at the
     * end of the source file name replacing the extension.
     */
    virtual void GetTargetFileInfo(std::string& rssTargetSubDir, std::string& rssTargetFileEnding) = 0;

    /**
     * @brief Return the file header text for automatic file generation.
     * @return The header text to place into the file.
     */
    virtual std::string GetFileHeaderText() const = 0;

    /**
     * @brief Stream the code into the file. Called once after processing.
     * @param[in, out] rcontext Reference to the stream context.
     * @param[in, out] rfstream Reference to the file stream to stream into
     */
    virtual void StreamIntoFile(TDefEntityContext& rcontext, std::ofstream& rfstream);

    /**
     * @brief Stream the include section for the file.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     */
    virtual void StreamIncludeSection(TDefEntityContext& rcontext);

    /**
     * @brief Process the entities.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pIterator Pointer to the iterator interface.
     * @return Returns true when the streaming of declarations was successful or false when streaming of declarations was not
     * successful and should be canceled.
     */
    virtual bool ProcessEntities(TDefEntityContext& rcontext, sdv::idl::IEntityIterator* pIterator);

    /**
     * @brief Stream the meta entity.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the interface of the meta entity.
     */
    virtual void StreamMetaEntity(TDefEntityContext& rcontext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Stream declaration if the entity is a declaration.
     * @param[in, out] rcontext Reference to the stream context.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @return Returns true when the streaming was successful or false when streaming was not successful and should be canceled.
     */
    virtual bool StreamDeclaration(TDefEntityContext& rcontext, sdv::IInterfaceAccess* pEntity);

    /**
     * @brief Stream definition if the entity is a definition.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] pEntity Pointer to the IInterfaceAccess pointer of the entity.
     * @param[in] bInline When set the definition is part of a declaration.
     * @param[in] bAnonymousDecl When set, the definition is part of an anonymous declaration (only valid for unions).
     */
    virtual void StreamDefinition(TDefEntityContext& rcontext, sdv::IInterfaceAccess* pEntity, bool bInline = false,
        bool bAnonymousDecl = false);

    /**
     * @brief Check for the existence of the switch variable and add the variable if not existing.
     * @param[in] pSwitchVarEntity Interface pointer to the switch variable entity.
     * @return Smart pointer to the switch variable.
     */
    std::shared_ptr<SSwitchVarContext> GetOrCreateVarBasedSwitch(sdv::IInterfaceAccess* pSwitchVarEntity);

    /**
     * @brief Detect a declaration of a union using a variable based switch case. If the switch case variable is within the scope
     * of the provided context, stream the functions needed to initialize and use the switch case. If the declaration is not a
     * fitting union but is of a compound structure (struct, exception or union), go through the declaration members for further
     * detection.
     * @param[in, out] rcontext Reference to the stream context to stream into.
     * @param[in] rssMemberScope Reference to the string containing the declarative member scope. Each declaration is separated by
     * a dot separator.
     * @param[in] pDeclEntity Interface pointer to the declarative entity to check.
     * @param[in] rvecArrayIndices Reference to the vector containing the array indices of the declaration to use for the detection.
     * If the declaration is not declared as array, the vector should be empty. If the declaration is declared as an array and the
     * amount of dimensions is below the amount of dimensions within the provided vector, the detection function is called for each
     * array dimension extending the vector with the index to do the detection for.
     */
    virtual void DetectUnionContainerForProcessing(TDefEntityContext& rcontext, const std::string& rssMemberScope,
        sdv::IInterfaceAccess* pDeclEntity, const std::vector<SArrayIterationInfo>& rvecArrayIndices = {});

    /**
     * @brief Process the union member that, together with the switch variable, has a mutual container from the entity in the
     * context.
     * @details Union definitions and declarations are to be treated differently, dependable on the switch type and the way of the
     * integration. The following differences are to be distingueshed:
     *  - Unions can be defined as named or as unnamed unions. Unnamed unions can only occur when followed by a declaration. They
     *    cannot be declared at global level due to the missing possibility to initialize/uninitialize using a member
     *    constructor/destructor.
     *  - Union declarations can be explicit or anonymous (not existing, but due to the unnamed nature of the union implicitly
     *    present). Anonymous declaration only occur for unnamed unions and are not allowed at root level. For named unions,
     *    there are no anonymous declarations.
     *
     * The following table defines the different types:
     * +------------------+---------------------------------------------------------------------+---------------------------------------------------------------------+
     * |                  | switch type base                                                    | switch var based                                                    |
     * |                  +----------------------------------+----------------------------------+----------------------------------+----------------------------------+
     * |                  | named                            | unnamed                          | named                            | unnamed                          |
     * |                  +----------------+-----------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+
     * |                  | explicit decl. | anonymous decl. | explicit decl. | anonymous decl. | explicit decl. | anonymous decl. | explicit decl. | anonymous decl. |
     * +==================+================+=================+================+=================+================+=================+================+=================+
     * | definition code  | Use struct to  | Not existing    | Use struct to  | Not allowed/    | Define named   | Not existing    | Must be        | Define unnamed  |
     * |                  | encapsulate    |                 | encapsulate    | not occurring.  | union as       |                 | followed by    | union as part   |
     * |                  | switch var and |                 | switch var and |                 | member.        |                 | decl. Cannot   | of struct.      |
     * |                  | union def.     |                 | union def.     |                 |                |                 | be global.     | Will not be     |
     * |                  |                |                 | Auto generate  |                 |                |                 |                | followed by     |
     * |                  |                |                 | struct name.   |                 |                |                 |                | decl.           |
     * +------------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+
     * | declaration code | Normal decl.   | N.a.            | Following def. | N.a.            | Normal         | N.a.            | Following def. | Absent          |
     * +------------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+
     * | switch variable  | Part of struct | N.a.            | Part of struct | N.a.            | Part of        | N.a.            | Part of        | Part of         |
     * |                  |                |                 |                |                 | container      |                 | container      | container       |
     * +------------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+
     * | constructor      | Part of struct | N.a.            | Part of struct | N.a.            | Part of        | N.a.            | Part of        | Part of         |
     * |                  |                |                 |                |                 | container      |                 | container      | container       |
     * +------------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+
     * | destructor       | Part of struct | N.a.            | Part of struct | N.a.            | Part of        | N.a.            | Part of        | Part of         |
     * |                  |                |                 |                |                 | container      |                 | container      | container       |
     * +------------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+
     * | switch function  | Part of struct | N.a.            | Part of struct | N.a.            | Part of        | N.a.            | Part of        | Part of         |
     * |                  |                |                 |                |                 | container      |                 | container      | container       |
     * +------------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+
     * | element access   | Internal       | N.a.            | Internal       | N.a.            | Over member    | N.a.            | Over member    | Direct access   |
     * |                  |                |                 |                |                 | var            |                 | var            | to elements     |
     * +------------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+----------------+-----------------+
     *
     * In case of a switch case based on a variable, this variable might be defined in a sub-structure of a mutual container of
     * a union. The union content depends on the switch variable. Both need to be processed together in the same context of one of
     * the containers.
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
    virtual void ProcessUnionInContainerContext(TDefEntityContext& rcontext, std::string rssMemberScopeUnionDecl,
        std::string rssMemberScopeSwitchVar, sdv::IInterfaceAccess* pUnionDef,
        const std::vector<SArrayIterationInfo>& rvecArrayIndices = std::vector<SArrayIterationInfo>());

    /**
     * @brief For a switch variable, detect the joint container of both switch variable and union. Start with the highest parent
     * running through all children.
     * @param[in, out] rcontext Reference to the definition stream context of the switch variable to stream into.
     * @param[in] pSwitchVarEntity Interface to the switch var declaration.
     * @param[in] pEntity Interface to the definition to detect for the container.
     */
    void DetectUnionJointContainerForSwitchVar(TDefEntityContext& rcontext, sdv::IInterfaceAccess* pSwitchVarEntity,
        sdv::IInterfaceAccess* pEntity);

    /**
     * @brief For a switch variable, process the joint container of both switch variable and union. Start with the highest parent
     * running through all children.
     * @param[in, out] rcontext Reference to the definition stream context of the switch variable to stream into.
     * @param[in] pSwitchVarEntity Interface to the switch var declaration.
     * @param[in] pContainerEntity Interface to the container definition.
     */
    virtual void ProcessUnionJointContainerForSwitchVar(TDefEntityContext& rcontext, sdv::IInterfaceAccess* pSwitchVarEntity,
        sdv::IInterfaceAccess* pContainerEntity);

private:
    /// Map with union switch variables. The switch variable scoped name is the key for the map.
    std::map<std::string, std::shared_ptr<SSwitchVarContext>> m_mapSwitchFunc;
};

// Include the code as well
#include "definition_generator_base.inl"

#endif // !defined DEFINITION_GENERATOR_BASE_H