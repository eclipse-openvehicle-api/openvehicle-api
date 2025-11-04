#include "definition_entity.h"
#include "../exception.h"
#include "../logger.h"
#include "../parser.h"
#include "struct_entity.h"
#include "union_entity.h"
#include "typedef_entity.h"
#include "variable_entity.h"
#include "attribute_entity.h"
#include "operation_entity.h"
#include "enum_entity.h"
#include "exception_entity.h"
#include "interface_entity.h"
#include "meta_entity.h"
#include <iostream>

CDefinitionEntity::CDefinitionEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    CEntity(rptrContext, ptrParent), m_iteratorChildren(GetChildrenVector()), m_iteratorInheritance(m_vecInheritance)
{}

CDefinitionEntity::CDefinitionEntity(CParser& rParser, const CContextPtr& rptrContext) :
    CEntity(rParser, rptrContext), m_iteratorChildren(GetChildrenVector()), m_iteratorInheritance(m_vecInheritance)
{}

sdv::interface_t CDefinitionEntity::GetInterface(sdv::interface_id idInterface)
{
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IDefinitionEntity>())
        return static_cast<sdv::idl::IDefinitionEntity*>(this);
    return CEntity::GetInterface(idInterface);
}

void CDefinitionEntity::ProcessContent()
{
    CLog log("Processing definition content...");

    // Get tokens until no token is returned any more.
    while (true)
    {
        // Prepare proocessing the next token
        CToken token = PeekToken();

        // Add any meta entity
        std::list<CParser::SMetaToken> lstMeta = GetParserRef().GetAndRemoveMeta();
        for (const CParser::SMetaToken& rsMeta : lstMeta)
            AddChild(std::make_shared<CMetaEntity>(rsMeta.tokenMeta.GetContext(), shared_from_this(), rsMeta.tokenMeta,
                rsMeta.lstComments));

        // Process the acquired token
        if (!token) break;
        if (!IsRootEntity() && token == "}")
            break;

        // Determine whether the comments are preceding the token (either on the same line or the line before).
        CTokenList lstPreComments = GetPreCommentTokenList();

        // Check for prefixes
        bool bPrefixConst = false;
        bool bPrefixReadOnly = false;
        bool bPrefixTypedef = false;
        bool bPrefixLocal = false;
        token = GetToken();
        uint32_t uiLineBegin = token.GetLine();
        uint32_t uiColBegin = token.GetCol();
        if (token == "const")   // This must be a const declaration
        {
            if (!Supports(EDefinitionSupport::support_const_variable))
                throw CCompileException(token, "Unexpected keyword 'const'.");
            bPrefixConst = true;
            token = GetToken();
        }
        else if (token == "readonly")   // This must be a readonly attribute
        {
            if (!Supports(EDefinitionSupport::support_attribute))
                throw CCompileException(token, "Unexpected keyword 'readonly'.");
            bPrefixReadOnly = true;
            token = GetToken();
        } else if (token == "typedef")  // This must be a typedef declaration
        {
            if (!Supports(EDefinitionSupport::support_typedef))
                throw CCompileException(token, "Unexpected keyword 'typedef'.");
            bPrefixTypedef = true;
            token = GetToken();
        } else if (token == "local")   // This must be a const declaration
        {
            if (!Supports(EDefinitionSupport::support_interface))
                throw CCompileException(token, "Unexpected keyword 'local'.");
            bPrefixLocal = true;
            token = GetToken();
            // The next token must be interface
            if (token != "interface")
                throw CCompileException(token, "Unexpected token. Only interfaces can be local.");
        }

        // For keywords that could be a definition as well as a declaration, find out whether the statement here is a
        //  - forward declaration
        //  - a definition
        //  - a definition as well as a declaration
        //  - a declaration
        // Check for a definition (first three points).
        auto fnIsDefinition = [&]() -> bool
        {
            // A declaration without definition has the construction
            //      <def type> <def identifier> <decl identifier>;
            // e.g.
            //      struct STest sTest;
            // The <def type> was retrieved already. The next token would be the identifier.
            if (PeekToken(0).GetType() == ETokenType::token_identifier &&
                PeekToken(1).GetType() == ETokenType::token_identifier) return false;

            // All other situations indicate a definition (possibly followed by a declaration).
            return true;
        };

        CEntityPtr ptrDefinitionEntity;
        CToken tokenDefinitionType = token;
        if (token == "module" && fnIsDefinition())
        {
            if (!Supports(EDefinitionSupport::support_module))
                throw CCompileException(token, "Unexpected keyword 'module'.");
            ptrDefinitionEntity = CreateChild<CModuleEntity>(token.GetContext(), this);
        }
        else if (token == "enum" && fnIsDefinition())
        {
            if (fnIsDefinition() && !Supports(EDefinitionSupport::support_enum))
                throw CCompileException(token, "Unexpected keyword 'enum'.");
            ptrDefinitionEntity = CreateChild<CEnumEntity>(token.GetContext(), this);
        }
        else if (token == "exception" && fnIsDefinition())
        {
            if (!Supports(EDefinitionSupport::support_exception))
                throw CCompileException(token, "Unexpected keyword 'exception'.");
            ptrDefinitionEntity = CreateChild<CExceptionEntity>(token.GetContext(), this);
        }
        else if (token == "struct" && fnIsDefinition())
        {
            if (!Supports(EDefinitionSupport::support_struct))
                throw CCompileException(token, "Unexpected keyword 'struct'.");
            ptrDefinitionEntity = CreateChild<CStructEntity>(token.GetContext(), this);
        }
        else if (token == "union" && fnIsDefinition())
        {
            if (!Supports(EDefinitionSupport::support_union))
                throw CCompileException(token, "Unexpected keyword 'union'.");
            ptrDefinitionEntity = CreateChild<CUnionEntity>(token.GetContext(), this);
        }
        else if (token == "interface" && fnIsDefinition())
        {
            if (!Supports(EDefinitionSupport::support_interface))
                throw CCompileException(token, "Unexpected keyword 'interface'.");
            ptrDefinitionEntity = CreateChild<CInterfaceEntity>(token.GetContext(), this, bPrefixLocal);
        }

        // Definition available?
        if (ptrDefinitionEntity)
        {
            // Assign the preceding comments (only if not prefixed)
            if (!bPrefixConst && !bPrefixReadOnly && !bPrefixTypedef && !lstPreComments.empty())
            {
                ptrDefinitionEntity->SetCommentTokens(lstPreComments);
                lstPreComments.clear();
            }

            // Set the location in the source file
            ptrDefinitionEntity->SetBeginPosition(uiLineBegin, uiColBegin);

            // Process the definition
            log << "Detected " << static_cast<std::string>(token) << " definition..." << std::endl;
            ptrDefinitionEntity->Process();
            token = GetToken();

            // Assign any succeeding comments
            ptrDefinitionEntity->ProcessPostCommentTokenList(token.GetLine());
        }

        // A struct allows the definition of types that require a declaration within the struct, but the declaration is anonymous
        // (unnamed and not declared) so the members of the definition appear as if they are part of the struct.
        bool bAnonymousDecl = false;
        if (token == ";" && GetType() == sdv::idl::EEntityType::type_struct && ptrDefinitionEntity &&
            ptrDefinitionEntity->Get<CDefinitionEntity>()->RequireDeclaration())
        {
            if (ptrDefinitionEntity->Get<CDefinitionEntity>()->AllowAutoTransparentDeclaration())
            {
                // Allow processing still
                PrependToken(token);

                // Create a dummy name
                token = CToken(GetParserRef().GenerateAnonymousEntityName("var"));

                bAnonymousDecl = true;
            } else
                throw CCompileException(token,
                    "The definition requires a declaration following the definition content or cannot find a variable within "
                    "current scope - unexpected token ';'.");
        }

        // Statement finished?
        if (token == ";")
        {
            if (bPrefixConst || bPrefixReadOnly || bPrefixTypedef)
                throw CCompileException(token, "Unexpected token ';'.");

            if (ptrDefinitionEntity)
            {
                // Assign any succeeding comments
                ptrDefinitionEntity->ProcessPostCommentTokenList(token.GetLine());

                // Set the end position in the source file.
                ptrDefinitionEntity->SetEndPosition(token.GetLine(), token.GetCol());

                // Declaration needed?
                if (ptrDefinitionEntity->Get<CDefinitionEntity>()->RequireDeclaration())
                    throw CCompileException(token,
                        "The definition requires a declaration following the definition content - unexpected token ';'.");
            }

            continue;
        };

        if (!token && !bPrefixConst && !bPrefixReadOnly && !bPrefixTypedef)
            throw CCompileException(GetLastValidToken(), "Missing ';' following the token.");

        // The declarative part can consist of the following types of declaration
        //  - an attribute - read-only prefix possible
        //  - a typedef
        //  - an operation
        //  - a variable - const prefix possible
        CEntityPtr ptrDeclarationEntity;
        if (bPrefixReadOnly && token != "attribute")
            throw CCompileException(token, "Expecting 'attribute' keyword following 'readonly'.");
        if (token == "attribute")
        {
            if (bPrefixConst || bPrefixTypedef || ptrDefinitionEntity || !Supports(EDefinitionSupport::support_attribute))
                throw CCompileException(token, "Unexpected keyword 'attribute'.");
            ptrDeclarationEntity = CreateChild<CAttributeEntity>(token.GetContext(), this, bPrefixReadOnly);
            if (bPrefixReadOnly)
                log << "Detected read-only attribute declaration..." << std::endl;
            else
                log << "Detected attribute declaration..." << std::endl;
        }
        else if (token == "case" || token == "default")
        {
            if (bPrefixReadOnly || bPrefixConst || bPrefixTypedef || ptrDefinitionEntity || !Supports(EDefinitionSupport::support_case_declaration))
                throw CCompileException(token, "Unexpected keyword 'case'.");
            ptrDeclarationEntity = CreateChild<CCaseEntry>(token.GetContext(), this, token == "default");
            log << "Detected case entry declaration..." << std::endl;
        } else
        {
            // Reinsert the token into the tokenlist
            PrependToken(token);

            // Check whether the declaration is a variable declaration or an operation.
            // An operation can have the following structure:
            //  <keyword> <identifier>();                                   e.g. int32 Func()
            //  <keyword> <keyword> <identifier>();                         e.g. long long Func()
            //  <keyword> <keyword> <keyword> <identifier>();               e.g. unsigned long long Func()
            //  <identifier> <identifier>()                                 e.g. mytype Func()
            // A variable declaration can have the following structure:
            //  <keyword> <identifier>;                                     e.g. int32 iVar;
            //  <keyword> <keyword> <identifier>;                           e.g. long long llVar;
            //  <keyword> <keyword> <keyword> <identifier>;                 e.g. unsigned long long ullVar;
            //  <keyword> <identifier> = <expression>;                      e.g. int32 iVar = 10;
            //  <keyword> <keyword> <identifier> = <expression>;            e.g. long long llVar = 10;
            //  <keyword> <keyword> <keyword> <identifier> = <expression>;  e.g. unsigned long long ullVar = 10;
            // Only variables can be declared with const prefix.
            // A variable declaration without assignment can also be a typedef when declared with typedef prefix.
            // Operations cannot be declared follwoing a definition.

            // As type expecting up to three keywords or one scoped identifier (composition of identifier and scope operator).
            // If there is a definition, then the declaration and definition are combined. The type is there then already.
            size_t nIndex = 0;
            bool bTypeFound = ptrDefinitionEntity ? true : false;
            CToken tokenLocal;
            bool bLongType = false;
            bool bUnsignedType = false;
            while ((tokenLocal = PeekToken(nIndex)).GetType() == ETokenType::token_keyword)
            {
                // Unsigned is allowed only as first keyword
                if (tokenLocal == "unsigned")
                {
                    if (nIndex) break;
                    bUnsignedType = true;
                }

                // Long should be followed by long or double.
                if (bLongType && tokenLocal != "long" && tokenLocal != "double") break;

                // Increase the token index...
                nIndex++;

                // Type found...
                bTypeFound = true;

                // After unsigned, at least one more keyword needs to follow
                if (bUnsignedType && nIndex == 1) continue;

                // After long, another type can follow, but only when this long didn't follow a previous long already.
                if (tokenLocal == "long")
                {
                    if (bLongType) break;   // type was long long.
                    bLongType = true;
                    continue;
                }

                // The type can be a templated type (or even nested templated types are possible).
                tokenLocal = PeekToken(nIndex);
                CToken tokenLocalTemplate;
                if (tokenLocal == "<")
                {
                    size_t nDepth = 0;
                    tokenLocalTemplate = tokenLocal;
                    do
                    {
                        if (tokenLocal == "<") nDepth++;
                        if (tokenLocal == ">") nDepth--;
                        if (tokenLocal == ">>") nDepth-=2;  // This actually is an operator
                        nIndex++;
                        tokenLocal = PeekToken(nIndex);
                    } while (tokenLocal && static_cast<bool>(nDepth));
                }

                // No more types are expected to follow
                break;
            }
            while (!bTypeFound)
            {
                // Check for the scope parameter
                tokenLocal = PeekToken(nIndex);
                if (tokenLocal == "::")
                {
                    nIndex++;
                    tokenLocal = PeekToken(nIndex);
                }

                // Check for the identifier
                if (tokenLocal.GetType() == ETokenType::token_identifier)
                {
                    nIndex++;
                    if (PeekToken(nIndex) != "::")
                        bTypeFound = true;
                    continue;
                }

                // Coming here means, there was no type
                throw CCompileException(tokenLocal, "Invalid token '", static_cast<std::string>(tokenLocal),
                    "' found for type name.");
            }

            // The return value can be an array - but only in case of operations.
            tokenLocal = PeekToken(nIndex);
            CToken tokenLocalArray;
            while (tokenLocal == "[")
            {
                if (!tokenLocalArray) tokenLocalArray = tokenLocal;
                do
                {
                    nIndex++;
                    tokenLocal = PeekToken(nIndex);
                } while (tokenLocal && tokenLocal != "]");
                nIndex++;
                tokenLocal = PeekToken(nIndex);
            }

            // Expecting identfier representing the name
            // If the extension is enabled, this could also be a keyword.
            if (tokenLocal.GetType() != ETokenType::token_identifier &&
                (!GetParserRef().GetEnvironment().ContextDependentNamesExtension() ||
                    tokenLocal.GetType() != ETokenType::token_keyword))
                throw CCompileException(tokenLocal, "Invalid token '", static_cast<std::string>(tokenLocal),
                    "' found for type name (or missing semi-colon ';' ?).");
            nIndex++;

            // An operation has a left bracket
            tokenLocal = PeekToken(nIndex);
            if (tokenLocal == "(") // This indicates a operation
            {
                if (ptrDefinitionEntity || bPrefixConst || bPrefixTypedef || !Supports(EDefinitionSupport::support_operation))
                    throw CCompileException(tokenLocal, "Unexpected left bracket '('.");
                ptrDeclarationEntity = CreateChild<COperationEntity>(token.GetContext(), this);
                log << "Detected operation declaration..." << std::endl;
            }
            else // This could be a variable declaration or a typedef declaration
            {
                // An array is not allowed when not using an operation
                if (tokenLocalArray)
                    throw CCompileException(tokenLocalArray, "Invalid token '[' found for type name.");

                // If there is a definition, add the name of the definition
                if (ptrDefinitionEntity)
                {
                    CToken tokenName(ptrDefinitionEntity->GetName());
                    PrependToken(tokenName);
                }

                // Check for typedef declaration
                if (bPrefixTypedef)
                {
                    ptrDeclarationEntity = CreateChild<CTypedefEntity>(token.GetContext(), this);
                    log << "Detected typedef declaration..." << std::endl;
                }
                else // Variable declaration
                {
                    if (!bPrefixConst && !Supports(EDefinitionSupport::support_variable))
                        throw CCompileException(token, "Variable declaration is not supported.");
                    ptrDeclarationEntity = CreateChild<CVariableEntity>(token.GetContext(), this, bPrefixConst, bAnonymousDecl);
                    if (bPrefixConst)
                        log << "Detected const variable declaration..." << std::endl;
                    else
                        log << "Detected variable declaration..." << std::endl;
                }
            }
        }

        // Process the entity declaration.
        if (!ptrDeclarationEntity) throw CCompileException("Declaration expected.");
        ptrDeclarationEntity->SetBeginPosition(uiLineBegin, uiColBegin);
        ptrDeclarationEntity->Process();

        if (!lstPreComments.empty())
            ptrDeclarationEntity->SetCommentTokens(lstPreComments);

        // Expect ';'
        token = GetToken();
        if (token != ";")
            throw CCompileException(token, "Missing semicolon ';' following the declaration.");
    }

    log << "For definition '" << GetName() << "', processing value assignments within content..." << std::endl;

    // If supported create the value node for the definition (this allows assignments of values to this entity).
    CreateValueNode();

    // If there is a value node (assignment is supported), add the inherited value sub-trees to the type as children. If there are
    // inherited entities with value nodes of their own, of course.
    if (ValueRef())
        CreateInheritanceValueChildNodes();

    // Go through all the type members and do post processing (building value chains, resolving arrays, etc.).
    for (CEntityPtr ptrTypeEntity : m_lstTypeMembers)
    {
        CTypedefEntity* pTypedefEntity = ptrTypeEntity->Get<CTypedefEntity>();
        if (pTypedefEntity)
        {
            log << "Post-processing typedef entity '" << pTypedefEntity->GetName() << "'..." << std::endl;
            pTypedefEntity->PostProcess();
        }
        CUnionEntity* pUnionEntity = ptrTypeEntity->Get<CUnionEntity>();
        if (pUnionEntity)
        {
            log << "Post-processing union entity '" << pUnionEntity->GetName() << "'..." << std::endl;
            pUnionEntity->PostProcess();
        }
    }

    // Go through all the typedef members and do post processing (building value chains, resolving arrays, variable assignment,
    // etc.).
    for (CEntityPtr ptrConstEntity : m_lstConstMembers)
    {
        CVariableEntity* pConstEntity = ptrConstEntity->Get<CVariableEntity>();
        if (!pConstEntity)
            throw CCompileException("Internal error: non-const entities in const entity list.");
        else
        {
            log << "Post-processing const variable entity '" << pConstEntity->GetName() << "'..." << std::endl;
            pConstEntity->PostProcess();
        }
    }

    // Go through all the definition members and do post processing (resolving arrays, creating exception and parameter lists,
    // etc.).
    for (CEntityPtr ptrDefinitionEntity : m_lstAttributesOperation)
    {
        CAttributeEntity* pAttribute = ptrDefinitionEntity->Get<CAttributeEntity>();
        COperationEntity* pOperation = ptrDefinitionEntity->Get<COperationEntity>();
        if (!pAttribute && !pOperation)
            throw CCompileException("Internal error: wrong entities in definition list.");

        if (pAttribute)
        {
            log << "Postprocessing attribute entity '" << pAttribute->GetName() << "'..." << std::endl;
            pAttribute->PostProcess();
        }
        if (pOperation)
        {
            log << "Postprocessing operation entity '" << pOperation->GetName() << "'..." << std::endl;
            pOperation->PostProcess();
        }
    }

    // Go through all the variable members and create the default values for all var members if the value node is a compound type
    // value node.
    for (CEntityPtr ptrDeclEntity : m_lstDeclMembers)
    {
        CVariableEntity *pDeclEntity = ptrDeclEntity->Get<CVariableEntity>();
        if (!pDeclEntity)
            throw CCompileException("Internal error: non-declaration entity in declaration entity list.");
        else
        {
            log << "Postprocess variable entity '" << pDeclEntity->GetName() << "'..." << std::endl;
            pDeclEntity->PostProcess();
        }
    }
}

void CDefinitionEntity::Process()
{
    CLog log("Processing definition of entity...");

    // Check for an identifier.
    // If present, this is the name of the definition.
    CToken token = GetToken();
    std::string ssName;
    m_bAnonymousDefinition = false;
    if (token.GetType() == ETokenType::token_identifier)
    {
        ssName = static_cast<std::string>(token);
        log << "Definition type name '" << ssName << "'..." << std::endl;
    } else
    if (SupportsAnonymous())
    {
        if (!GetParentEntity() || GetParentEntity()->IsRootEntity())
            throw CCompileException(token, "Unnamed definitions are not supported at root level.");
        std::string ssPrefix = "anonymous";
        switch (GetType())
        {
        case sdv::idl::EEntityType::type_enum:      ssPrefix = "enum";          break;
        case sdv::idl::EEntityType::type_struct:    ssPrefix = "struct";        break;
        case sdv::idl::EEntityType::type_union:     ssPrefix = "union";         break;
        case sdv::idl::EEntityType::type_module:    ssPrefix = "namespace";     break;
        case sdv::idl::EEntityType::type_interface: ssPrefix = "interface";     break;
        case sdv::idl::EEntityType::type_exception: ssPrefix = "except";        break;
        case sdv::idl::EEntityType::type_typedef:   ssPrefix = "typedef";       break;
        default:                                                                break;
        }
        ssName = GetParserRef().GenerateAnonymousEntityName(ssPrefix);
        log << "Unnamed definition was automatically assigned name '" << ssName << "'..." << std::endl;
        m_bRequiresContent = true;
        m_bAnonymousDefinition = true;
        PrependToken(token);
    } else
        throw CCompileException(token, "Unnamed definition is not supported.");

    // Process the definition addendum
    ProcessDefinitionAddendum();

    // Expecting curly bracket when for a
    token = GetToken();
    if (RequireContentDefinition() && token != "{")
        throw CCompileException(token, "Expecting curly bracket '{'.");

    // If there is no content, consider the statement as a (forward) declaration.
    SetName(ssName, token != "{");

    // If there is no content, the processing is done.
    if (token != "{")
    {
        log << "Definition type was defined as forward declaration." << std::endl;
        log << "Finished processing definition entity..." << std::endl;
        PrependToken(token);
        return;
    }

    // Definition content...
    ProcessContent();

    // Close the definition
    token = GetToken();
    if (token != "}")
        throw CCompileException(token, "Expecting curly bracket '}'.");

    log << "Finished processing definition entity..." << std::endl;
}

void CDefinitionEntity::ProcessDefinitionAddendum()
{
    // Check for inheritance
    CToken token = GetToken();
    if (token != ":")
    {
        PrependToken(token);
        return;
    }

    if (!SupportsInheritance()) throw CCompileException(token, "Inheritance is not supported.");

    // When an inheritance list is provided, the content should also be provided.
    m_bRequiresContent = true;

    CLog log("Processing inheritance list...");

    // Get the list of base entities
    while (true)
    {
        // Find the base entity.
        std::pair<std::string, CEntityPtr> prBase = ProcessScopedName();
        if (prBase.first.empty() || !prBase.second)
            throw CCompileException("Base not found.");
        log << "Inherited from base entity: " << prBase.first << std::endl;

        if (prBase.second->GetResolvedEntity()->GetType() != GetType())
            throw CCompileException("Cannot inherit from different types.");

        // Check whether the base entity is more than only a declaration.
        if (prBase.second->ForwardDeclaration())
            throw CCompileException("Base type found, but only declared; definition is missing.");

        // TODO: Check whether the entity was already previously inherited directly or indirectly. Inheriting through a base
        // prevents inheriting again.

        // Add the base entity to the base entity list.
        m_vecInheritance.push_back(prBase.second);

        // Get the next base entity or the beginning of the definition.
        token = GetToken();
        if (token != ",")
        {
            PrependToken(token);
            break;
        }
    }
}

void CDefinitionEntity::CreateInheritanceValueChildNodes()
{
    CLog log("Creating values for inherited child nodes...");

    if (!ValueRef()) throw CCompileException("Internal error: cannot create inheritance value child nodes.");

    // For each base entity, copy the value tree
    for (CEntityPtr ptrInheritedEntity : m_vecInheritance)
    {
        // Check for a valid value of the base entity.
        if (!ptrInheritedEntity->GetResolvedEntity()->ValueRef())
            throw CCompileException("Internal error: base entity wasn't processed for assignment yet.");

        log << "Assigning values from base entity '" << ptrInheritedEntity->GetName() << "'." << std::endl;

        // Create a copy of the tree and add as child to own tree.
        ValueRef()->AddChild(ptrInheritedEntity->GetResolvedEntity()->ValueRef()->CreateCopy(shared_from_this(), ValueRef()));
    }
}

sdv::idl::IEntityIterator* CDefinitionEntity::GetChildren()
{
    if (SupportsChildren()) return &m_iteratorChildren;
    return nullptr;
}

sdv::idl::IEntityIterator* CDefinitionEntity::GetInheritance()
{
    if (SupportsInheritance() && !m_vecInheritance.empty()) return &m_iteratorInheritance;
    return nullptr;
}

void CDefinitionEntity::CalcHash(CHashObject& rHash) const
{
    // First calc the hash of the base entity.
    CEntity::CalcHash(rHash);

    // Add the hashes of all inherited entities
    for (const CEntityPtr& ptrEntity : m_vecInheritance)
        ptrEntity->CalcHash(rHash);

    // Add the declarations
    for (const CEntityPtr& ptrEntity : m_lstDeclMembers)
        ptrEntity->CalcHash(rHash);

    // Add the attributes and operations
    for (const CEntityPtr& ptrEntity : m_lstAttributesOperation)
        ptrEntity->CalcHash(rHash);
}

const CEntityList CDefinitionEntity::GetDeclMembers() const
{
    CEntityList lstMembers;

    // Add all declarations from the inherited members
    for (const CEntityPtr& rptrInheritedEntity : m_vecInheritance)
    {
        const CDefinitionEntity* pDefinition = rptrInheritedEntity->GetResolvedEntity()->Get<CDefinitionEntity>();
        if (pDefinition)
        {
            CEntityList lstInheritedEntities = pDefinition->GetDeclMembers();
            for (const CEntityPtr& ptrInheritedMember : lstInheritedEntities)
                lstMembers.push_back(ptrInheritedMember);
        }
    }

    // Add own declarations
    for (const CEntityPtr& rptrMember : m_lstDeclMembers)
        lstMembers.push_back(rptrMember);

    return lstMembers;
}

void CDefinitionEntity::AddChild(CEntityPtr ptrChild)
{
    CLog log;

    // Call base class first
    CEntity::AddChild(ptrChild);

    // Dependable on the type of entity, add the entity to dedicated lists as well.
    if (ptrChild->Get<CTypedefEntity>())
    {
        m_lstTypeMembers.push_back(ptrChild);
        log << "Added typedef declaration to type member list..." << std::endl;
    }
    else if (ptrChild->Get<CAttributeEntity>())
    {
        m_lstAttributesOperation.push_back(ptrChild);
        log << "Added attribute to definition list..." << std::endl;
    } else if (ptrChild->Get<COperationEntity>())
    {
        m_lstAttributesOperation.push_back(ptrChild);
        log << "Added operation to definition list..." << std::endl;
    }
    else if (ptrChild->Get<CDeclarationEntity>())
    {
        if (ptrChild->Get<CDeclarationEntity>()->IsReadOnly())
        {
            m_lstConstMembers.push_back(ptrChild);
            log << "Added const declaration to const member list..." << std::endl;
        }
        else
        {
            m_lstDeclMembers.push_back(ptrChild);
            log << "Added declaration to variable member list..." << std::endl;
        }
    }
    else if (ptrChild->Get<CDefinitionEntity>())
    {
        m_lstTypeMembers.push_back(ptrChild);
        log << "Added definition to type member list..." << std::endl;
    }
    else if (ptrChild->Get<CMetaEntity>())
    {
        GetRootEntity()->Get<CRootEntity>()->AddMeta(ptrChild);
        log << "Added meta data to root based meta list..." << std::endl;
    }
}

std::pair<CEntityPtr, bool> CDefinitionEntity::FindLocal(const std::string& rssName, bool bDeclaration) const
{
    // Call base class implementation first.
    std::pair<CEntityPtr, bool> prEntity = CEntity::FindLocal(rssName, bDeclaration);
    if (prEntity.first) return prEntity;

    // Check through all inherited entities
    for (const CEntityPtr& rptrBaseEntity : m_vecInheritance)
    {
        if (!rptrBaseEntity) continue;
        CEntityPtr ptrResolvedBaseEntity = rptrBaseEntity->GetResolvedEntity();
        if (!ptrResolvedBaseEntity->Get<CDefinitionEntity>()) continue;
        prEntity = ptrResolvedBaseEntity->Get<CDefinitionEntity>()->FindLocal(rssName, bDeclaration);
        if (prEntity.first)
        {
            // Set the inherited flag
            prEntity.second = true;
            return prEntity;
        }
    }

    // No entity found.
    return std::make_pair(CEntityPtr(), false);
}
