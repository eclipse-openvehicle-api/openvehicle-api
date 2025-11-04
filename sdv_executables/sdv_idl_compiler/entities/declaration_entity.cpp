#include "declaration_entity.h"
#include "../exception.h"
#include "../logger.h"
#include "../token.h"
#include "../tokenlist.h"
#include "../support.h"
#include "../lexer.h"
#include "../constvariant.inl"
#include "../parser.h"
#include "struct_entity.h"
#include "interface_entity.h"
#include "typedef_entity.h"
#include "variable_entity.h"
#include "attribute_entity.h"
#include "operation_entity.h"
#include "exception_entity.h"
#include "parameter_entity.h"
#include "enum_entity.h"
#include "union_entity.h"
#include <functional>
#include <type_traits>
#include <limits>

CDeclarationEntity::CDeclarationEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    CEntity(rptrContext, ptrParent)
{}

CDeclarationEntity::CDeclarationEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent,
    const CTokenList& rlstTokenList) :
    CEntity(rptrContext, ptrParent, rlstTokenList)
{}

sdv::interface_t CDeclarationEntity::GetInterface(sdv::interface_id idInterface)
{
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IDeclarationEntity>())
        return static_cast<sdv::idl::IDeclarationEntity*>(this);
    return CEntity::GetInterface(idInterface);
}

sdv::IInterfaceAccess* CDeclarationEntity::GetDeclarationType() const
{
    return const_cast<CTypeDeclaration*>(&m_typedecl);
}

CEntityPtr CDeclarationEntity::GetTypeEntity() const
{
    return m_typedecl.GetTypeDefinitionEntityPtr();
}

bool CDeclarationEntity::HasArray() const
{
    return m_vecMultiArraySizeTokenList.size() ? true : false;
}

sdv::sequence<sdv::idl::SArrayDimension> CDeclarationEntity::GetArrayDimensions() const
{
    sdv::sequence<sdv::idl::SArrayDimension> seqArrayDimensions;
    if (!HasArray()) return seqArrayDimensions;

    // Traverse through each array entry
    std::function<void(const CValueNodePtr& ptrValue)> fnCollectArrayDimensions =
        [&](const CValueNodePtr& ptrValue)
    {
        // Is the value node an array at all?
        CArrayValueNode* pArrayValue = ptrValue->Get<CArrayValueNode>();
        if (!pArrayValue) return;

        // Fill the array dimension struct
        sdv::idl::SArrayDimension sArrayDimension{};
        sArrayDimension.eType = sdv::idl::SArrayDimension::EDimensionType::bound;
        if (pArrayValue->IsUnbound())
            sArrayDimension.eType = sdv::idl::SArrayDimension::EDimensionType::unbound;
        sArrayDimension.ssExpression = MakeFullScoped(pArrayValue->GetSizeExpression());

        // Store in sequence
        seqArrayDimensions.push_back(sArrayDimension);

        // Process the next array dimension
        if (pArrayValue->GetSize() != 0)
            fnCollectArrayDimensions((*pArrayValue)[0]);
    };

    // Collect the array dimensions.
    fnCollectArrayDimensions(ValueRef());

    return seqArrayDimensions;
}

bool CDeclarationEntity::HasAssignment() const
{
    return !m_lstAssignmentTokenList.empty();
}

sdv::u8string CDeclarationEntity::GetAssignment() const
{
    std::stringstream sstreamAssignment;
    bool bInitial = true;
    for (const CToken& rToken : m_lstAssignmentTokenList)
    {
        if (!bInitial) sstreamAssignment << " ";
        bInitial = false;
        sstreamAssignment << static_cast<std::string>(rToken);
    }
    return sstreamAssignment.str();
}

void CDeclarationEntity::Process()
{
    CLog log("Processing declaration (preparation)...");

    // Determine whether the comments are preceding the token (either on the same line or the line before).
    CTokenList lstPreComments = GetPreCommentTokenList();
    if (!lstPreComments.empty()) SetCommentTokens(lstPreComments);

    // Process the type
    CTypeDeclaration sTypeDecl = ProcessType();

    // Check for the support of interface and void types
    if (sTypeDecl.GetBaseType() == sdv::idl::EDeclType::decltype_interface && !SupportInterface())
        throw CCompileException("The declaration of interfaces is not supported.");
    if (sTypeDecl.GetBaseType() == sdv::idl::EDeclType::decltype_void && !SupportVoid())
        throw CCompileException("The use of 'void' as type is not supported.");

    // Preprocess potential array declaration (only for operations).
    if (GetType() == sdv::idl::EEntityType::type_operation)
        PreprocessArrayDeclaration();

    // Process the declaration
    ProcessDeclaration(sTypeDecl);
}

void CDeclarationEntity::ProcessDeclaration(const CTypeDeclaration& rTypeDecl)
{
    // Store the type
    m_typedecl = rTypeDecl;

    CLog log("Processing declaration...");

    // Expecting an identifier.
    CToken token = GetToken();
    if (token.GetType() == ETokenType::token_keyword)
    {
        // Keywords as names are allowed if the extension is enabled.
        if (!GetParserRef().GetEnvironment().ContextDependentNamesExtension())
            throw CCompileException(token, "The identifier cannot be a reserved keyword.");
    }
    else if (token.GetType() != ETokenType::token_identifier)
        throw CCompileException(token, "Expecting an identifier.");
    SetName(token);
    log << "Declaration name '" << GetName() << "'..." << std::endl;

    // Preprocess potential array declaration (not for operations).
    if (GetType() != sdv::idl::EEntityType::type_operation)
        PreprocessArrayDeclaration();

    // Further processing...
    token = GetToken();

    // Requires parameters?
    if (RequiresParameters())
    {
        // Expect a left bracket
        if (token != "(") throw CCompileException(token, "Expected left bracket '('.");

        log << "Reading parameter list..." << std::endl;
        PreprocessTokenListVector(m_vecParametersTokenList);

        // Expect a right bracket
        token = GetToken();
        if (token != ")")
            throw CCompileException(token, "Expected right bracket ')'.");

        // Get the next token...
        token = GetToken();

        // Check for the 'const' keyword. If set, the operation is defined as const-operation.
        if (token == "const")
        {
            SetOperationAsConst();
            token = GetToken();
        }
    }

    // Supports exceptions
    if (SupportRaiseExceptions())
    {
        while (true)
        {
            // Check for the 'raises' keyword.
            enum class EExceptType {raises, getraises, setraises, none} eExceptType = EExceptType::none;
            if (token == "raises")
                eExceptType = EExceptType::raises;
            else if (token == "getraises")
                eExceptType = EExceptType::getraises;
            else if (token == "setraises")
                eExceptType = EExceptType::setraises;
            if (eExceptType == EExceptType::none)
                break;

            // Check for validity
            if (!SupportSeparateSetGetRaiseExceptions() && eExceptType == EExceptType::getraises)
                throw CCompileException(token,
                    "Cannot set a separate 'get-raises' exception list; use the 'raises' keyword instead.");
            if (!SupportSeparateSetGetRaiseExceptions() && eExceptType == EExceptType::setraises)
                throw CCompileException(token,
                    "Cannot set a separate 'set-raises' exception list; use the 'raises' keyword instead.");
            if (SupportSeparateSetGetRaiseExceptions() && eExceptType == EExceptType::raises && IsReadOnly())
                eExceptType = EExceptType::getraises;
            if (eExceptType == EExceptType::setraises && IsReadOnly())
                throw CCompileException(token, "Cannot set a set-raises exception list for a readonly type.");
            if ((eExceptType == EExceptType::raises) && !m_vecRaisesExceptionsTokenList.empty())
                throw CCompileException(token, "Multiple definitions of 'raises' exceptions are not allowed.");
            if ((eExceptType == EExceptType::setraises) && !m_vecSetRaisesExceptionsTokenList.empty())
                throw CCompileException(token, "Multiple definitions of 'set-raises' exceptions are not allowed.");
            if ((eExceptType == EExceptType::getraises) && !m_vecGetRaisesExceptionsTokenList.empty())
                throw CCompileException(token, "Multiple definitions of 'get-raises' exceptions are not allowed.");

            // Expect a left bracket
            token = GetToken();
            if (token != "(") throw CCompileException(token, "Expected left bracket '('.");

            // Processes raises exception list.
            switch (eExceptType)
            {
            case EExceptType::raises:
                log << "Reading 'raises' exception list..." << std::endl;
                PreprocessTokenListVector(m_vecRaisesExceptionsTokenList);
                if (m_vecRaisesExceptionsTokenList.empty()) throw CCompileException(token, "Missing exception types.");
                break;
            case EExceptType::setraises:
                log << "Reading 'setraises' exception list..." << std::endl;
                PreprocessTokenListVector(m_vecSetRaisesExceptionsTokenList);
                if (m_vecSetRaisesExceptionsTokenList.empty()) throw CCompileException(token, "Missing exception types.");
                break;
            case EExceptType::getraises:
                log << "Reading 'getraises' exception list..." << std::endl;
                PreprocessTokenListVector(m_vecGetRaisesExceptionsTokenList);
                if (m_vecGetRaisesExceptionsTokenList.empty()) throw CCompileException(token, "Missing exception types.");
                break;
            default:
                break;
            }

            // Expect a right bracket
            token = GetToken();
            if (token != ")")
                throw CCompileException(token, "Expected right bracket ')'.");

            // Get the next token...
            token = GetToken();
        }
    }

    // Is there an assignment?
    if (token == "=")
    {
        if (!SupportAssignments())
            throw CCompileException(token, "Assignment operator detected, but type doesn't support assignments.");
        log << "Declaration assignment detected; storing expression for later processing..." << std::endl;

        // Read the tokens for the assignment expression. Read until ';' or ','; the latter not within an expression
        // sub-statement.
        size_t nDepth = 0;
        log << "Assignment expression:" << std::endl;
        while(true)
        {
            token = GetToken();
            if (!token)
                throw CCompileException("Unexpected end of file found; missing ';'.");
            if (token == ";" || ((token == "," || token == "}") && !nDepth))
                break;

            log << " " << static_cast<std::string>(token);

            if (token == "{") nDepth++;
            if (token == "}") nDepth--;
            m_lstAssignmentTokenList.push_back(token);
        }
        log << std::endl;
    } else
    {
        // Does the entity need an assignment?
        if (RequiresAssignment())
            throw CCompileException(token, "Expecting an assignment operator.");
    }

    // Assign any succeeding comments
    ProcessPostCommentTokenList(token.GetLine());

    // Another declaration?
    if (token == ",")
    {
        if (!SupportMultipleDeclarations())
            throw CCompileException(token, "Multiple declarations on a single line of code is not supported for this type.");
        log << "Another declaration of the same type..." << std::endl;

        // Peek for ending the definition
        if (DoNotEnfoceNextDeclarationAfterComma() && PeekToken() == "}") return;

        // Create another declaration
        CDeclarationEntity* pTypeEntity = nullptr;
        if (Get<CVariableEntity>()) pTypeEntity = CreateChild<CVariableEntity>(token.GetContext(), GetParentEntity().get(), IsReadOnly(), false)->Get<CDeclarationEntity>();
        else if (Get<CEnumEntry>()) pTypeEntity = CreateChild<CEnumEntry>(token.GetContext(), GetParentEntity().get())->Get<CDeclarationEntity>();
        else if (Get<CAttributeEntity>()) pTypeEntity = CreateChild<CAttributeEntity>(token.GetContext(), GetParentEntity().get(), IsReadOnly())->Get<CDeclarationEntity>();
        else
            throw CCompileException(token, "Unexpected token ','.");
        if (!pTypeEntity) throw CCompileException(token, "Internal error: failed to create another declaration entity.");

        // Set the new begin position of the declaration
        CToken tokenTemp = PeekToken();
        pTypeEntity->SetBeginPosition(tokenTemp.GetLine(), tokenTemp.GetCol());

        // Use the same type for the processing.
        pTypeEntity->ProcessDeclaration(m_typedecl);

        // Done.
        return;
    }

    // Set the end position of the declaration
    SetEndPosition(token.GetLine(), token.GetCol());

    // Reinsert the token
    PrependToken(token);
}

void CDeclarationEntity::PreprocessArrayDeclaration()
{
    CLog log("Checking for array...");

    // For each array dimension, add a tokenlist to the m_vecMultiArraySizeTokenList variable.
    bool bIsArray = false;
    while (true)
    {
        // Check for an array
        CToken token = GetToken();
        if (token != "[")
        {
            PrependToken(token);
            break;
        }

        if (!SupportArrays())
            throw CCompileException(token, "Unexpected token '['.");

        log << "Array detected; storing expression for later processing..." << std::endl;
        log << "Array expression: [";

        // Check for multidimensional arrays
        if (bIsArray && !GetParserRef().GetEnvironment().MultiDimArrayExtension())
            throw CCompileException(token, "Multi-dimentsional arrays are not allowed. Unexpected token '['.");
        bIsArray = true;

        // Read the tokens for the array size. Read until ']'.
        CTokenList lstArraySize;
        size_t nDepth = 1;
        while (true)
        {
            token = GetToken();
            log << " " << static_cast<std::string>(token);
            if (!token)
                throw CCompileException("Unexpected end of file found; missing ']'.");
            if (token == ";")
                throw CCompileException("Unexpected end of declaration; missing ']'.");
            if (token == "[")
                nDepth++;
            if (token == "]")
            {
                nDepth--;
                if (!nDepth)
                {
                    m_vecMultiArraySizeTokenList.push_back(std::move(lstArraySize));
                    break;
                }
            }
            lstArraySize.push_back(token);
        }
        log << std::endl;
    }
}

void CDeclarationEntity::PreprocessTokenListVector(std::vector<CTokenList>& rvecTokenList)
{
    CLog log("Checking for comma separated token lists...");

    CTokenList lstTokens;
    bool bInitial = true;
    while (true)
    {
        // Check for array index (allowed in certain situations)
        CToken token = GetToken();
        if (token == "[")
        {
            do
            {
                lstTokens.push_back(token);
                token = GetToken();
            } while (token && token != "]");
            lstTokens.push_back(token);
            token = GetToken();
        }

        // Check for template parameters
        if (token == "<")
        {
            size_t nDepth = 0;
            do
            {
                if (token == "<") nDepth++;
                if (token == ">>") // Special case when closing nested templates.
                {
                    token = CToken(">", ETokenType::token_operator);
                    PrependToken(token);
                }
                if (token == ">") nDepth--;
                lstTokens.push_back(token);
                token = GetToken();
            } while (token && static_cast<bool>(nDepth));
            lstTokens.push_back(token);
            token = GetToken();
        }

        // Check for end of processing
        if (!token || token == "]" || token == ")" || token == ";")
        {
            if (!bInitial)
                rvecTokenList.push_back(std::move(lstTokens));
            PrependToken(token);
            break;
        }

        if (bInitial)
            log << "Comma separated list detected: ";
        bInitial = false;

        log << " " << static_cast<std::string>(token);

        // Check for comma separator
        if (token == ",")
        {
            rvecTokenList.push_back(std::move(lstTokens));
            continue;
        }

        // Add the token to the token list
        lstTokens.push_back(token);
    }
    if (rvecTokenList.empty())
        log << "No comma separated list detected..." << std::endl;
    else
        log << std::endl;
}

void CDeclarationEntity::PostProcess()
{
    CLog log("Post process declaration...");

    // Check the assignment processing progression state...
    switch (m_eProcAssState)
    {
    case EProcessAssignmentProgression::unprocessed:
        // Not processed yet, start processing...
        m_eProcAssState = EProcessAssignmentProgression::currently_processing;
        break;
    case EProcessAssignmentProgression::currently_processing:
        log << "Post processing declaration assignment takes place already; cannot do this more than once at the same time..."
            << std::endl;
        // Alarm, circular references... cannot continue.
        throw CCompileException("Circular referencing entity.");
    case EProcessAssignmentProgression::processed:
    default:
        // Already done...
        log << "Post processing declaration was done before; no need to repeat..." << std::endl;
        return;
    }

    // The parent value is the value of the parent entity, if there is any value.
    CValueNodePtr ptrValueParent = GetParentEntity() ? GetParentEntity()->ValueRef() : nullptr;
    if (ptrValueParent)
        log << "The parent entity '" << GetParentEntity()->GetName() << "' has a value node..." << std::endl;
    else
        log << "No parent entity or no value node assigned to the parent entity..." << std::endl;

    // Create a value at each bottom leaf of a multi-dimensional array.
    std::function<void(CValueNodePtr&, const CValueNodePtr, std::function<CValueNodePtr(const CValueNodePtr)>)> fnCreateAtBottomLeaf =
        [&](CValueNodePtr& rptrValue, const CValueNodePtr& rptrValueParent, std::function<CValueNodePtr(const CValueNodePtr)> fnCreate)
        {
            // Check for an array value
            CArrayValueNode* psArrayValue = dynamic_cast<CArrayValueNode*>(rptrValue.get());

            // If there is an array value, call the function once more for each leaf.
            if (psArrayValue)
            {
                for (size_t nIndex = 0; nIndex < psArrayValue->GetSize(); nIndex++)
                    fnCreateAtBottomLeaf((*psArrayValue)[nIndex], rptrValue, fnCreate);
                return;
            }

            // Create a new element...
            rptrValue = fnCreate(rptrValueParent);
        };

    // Process parameters - since parameters might depend on other parameters, do the processing in two steps.
    size_t nIndex = 1;
    for (const CTokenList& rlstTokenList : m_vecParametersTokenList)
    {
        log << "Processing parameter #" << nIndex++ << std::endl;

        CEntityPtr ptrEntity = std::make_shared<CParameterEntity>(GetContext(), shared_from_this(), rlstTokenList);
        ptrEntity->Process();

        m_vecParameters.push_back(ptrEntity);
    }
    for (CEntityPtr& rptrParameter : m_vecParameters)
        rptrParameter->Get<CParameterEntity>()->PostProcess();

    // Build array tree
    if (m_vecMultiArraySizeTokenList.empty()) log << "No array processing needed..." << std::endl;
    else if (m_vecMultiArraySizeTokenList.size() == 1)  log << "Single-dimensional array processing needed..." << std::endl;
    else log << "Multi-dimensional array processing needed..." << std::endl;
    for (const CTokenList& rlstArrayExpression : m_vecMultiArraySizeTokenList)
    {
        std::pair<CConstVariant, bool> prArraySize = {0, false};
        log << "Start processing array dimension..." << std::endl;

        // Empty expression indicates retrieving the size from the assignment.
        if (!rlstArrayExpression.empty())
        {
            log << "Calculate the array size..." << std::endl;
            prArraySize = ProcessNumericExpression(rlstArrayExpression);

            // Is the array size dynamic?
            if (prArraySize.second)
                throw CCompileException(*rlstArrayExpression.begin(), "Cannot use non-const variable for the array size.");
            log << "The array has " << prArraySize.first.Get<uint32_t>() << " elements..." << std::endl;

            // Check whether the size is integral
            if (!prArraySize.first.IsIntegral())
                throw CCompileException(*rlstArrayExpression.begin(), "Only integral data types are supported for the array size.");
            if ((prArraySize.first < CConstVariant(0)).Get<bool>())
                throw CCompileException(*rlstArrayExpression.begin(), "Invalid array size.");
        } else
        {
            log << "Array is defined as unbound array retrieving the size from the variable assignment..." << std::endl;

            // Unbound arrays are not possible for writable variables. Exception are operations, attributes and parameters of local
            // interfaces.
            bool bError = true;
            switch (GetType())
            {
            case sdv::idl::EEntityType::type_typedef:
                bError = false;
                break;
            case sdv::idl::EEntityType::type_variable:
                // When not read-only, this is an error.
                if (IsReadOnly()) bError = false;
                break;
            default:
                break;
            }
            if (bError)
                throw CCompileException("Retrieving the size of the array through its assignment is"
                    " only possible with const declarations and typedefs.");
        }
        // The array creation function
        auto fnCreateArray = [&, this](const CValueNodePtr& rptrValueParent) -> CValueNodePtr
        {
            std::shared_ptr<CArrayValueNode> ptrArrayValue = std::make_shared<CArrayValueNode>(shared_from_this(), rptrValueParent);
            if (rlstArrayExpression.empty())
                ptrArrayValue->SetFixedSizeUnbound();
            else if (prArraySize.second)
                ptrArrayValue->SetDynamicSize(prArraySize.first.Get<size_t>(), rlstArrayExpression);
            else
                ptrArrayValue->SetFixedSize(prArraySize.first.Get<size_t>(), rlstArrayExpression);
            return ptrArrayValue;
        };

        // Create the array value at the bottom leaf.
        fnCreateAtBottomLeaf(ValueRef(), ptrValueParent, fnCreateArray);

        log << "Finalized processing array dimension..." << std::endl;
    }

    // Add the values of the type.
    if (m_vecMultiArraySizeTokenList.size() == 0)
        log << "Copy the existing type value tree or create a declaration value node for this assignment..." << std::endl;
    else
        log << "For each array element, copy the existing type value tree or create a"
            " declaration value node for this assignment..." << std::endl;
    auto fnCreateTypeValues = [&, this](const CValueNodePtr& rptrValueParent) -> CValueNodePtr
    {
        if (m_typedecl.GetTypeDefinitionEntityPtr())
        {
            // In case the original type was not processed yet, do so now.
            CDeclarationEntity* pOriginalType = m_typedecl.GetTypeDefinitionEntityPtr()->Get<CDeclarationEntity>();
            if (pOriginalType) pOriginalType->PostProcess();

            // Copy the existing entity of the type... this contains all the default assignments already...
            log << "Copy type value tree..." << std::endl;
            CValueNodePtr ptrValue = m_typedecl.GetTypeDefinitionEntityPtr()->ValueRef();
            if (ptrValue)
                return ptrValue->CreateCopy(shared_from_this(), rptrValueParent);
            else
            {
                if (pOriginalType)
                    throw CCompileException("Internal error: value tree must be available for '", GetName(), "'.");
                log << "No value tree present; nothing to copy..." << std::endl;
                return nullptr;
            }
        }
        else
        {
            log << "Create declaration value node..." << std::endl;
            return std::make_shared<CSimpleTypeValueNode>(shared_from_this(), ptrValueParent);
        }
    };
    fnCreateAtBottomLeaf(ValueRef(), ptrValueParent, fnCreateTypeValues);

    // If this is a variable declaration, add the value as part of the parent tree
    if (CanSupportComplexTypeAssignments() && ptrValueParent)
    {
        log << "The entity value tree is part of the value tree of the parent node..." << std::endl;
        ptrValueParent->AddChild(ValueRef());
    }

    // Add the assignment.
    if (!m_lstAssignmentTokenList.empty())
    {
        log << "Assignment was available, process the assignment..." << std::endl;
        if (!SupportAssignments())
            throw CCompileException(*m_lstAssignmentTokenList.begin(), "Type definitions cannot be assigned any values.");
        ValueRef()->ProcessValueAssignment(m_lstAssignmentTokenList);
    } else
    {
        // Does the entity need an assignment?
        if (RequiresAssignment())
            throw CCompileException("Expecting an assignment operator for '", GetName(), "'.");
    }

    // Build raises exception lists
    for (const CTokenList& rlstTokenList : m_vecRaisesExceptionsTokenList)
    {
        log << "Processing raising exception..." << std::endl;
        std::pair<std::string, CEntityPtr> prBase = ProcessScopedName(rlstTokenList);
        if (prBase.first.empty() || !prBase.second || !prBase.second->Get<CExceptionEntity>())
            throw CCompileException("Exception definition not found.");
        if (SupportSeparateSetGetRaiseExceptions())
        {
            m_vecGetRaisesExceptions.push_back(prBase.second);
            m_vecSetRaisesExceptions.push_back(prBase.second);
        } else
            m_vecRaisesExceptions.push_back(prBase.second);
        log << "Entity could raise exception on operation/attribute: " << prBase.first << std::endl;
    }
    for (const CTokenList& rlstTokenList : m_vecSetRaisesExceptionsTokenList)
    {
        log << "Processing set-raising exception..." << std::endl;
        std::pair<std::string, CEntityPtr> prBase = ProcessScopedName(rlstTokenList);
        if (prBase.first.empty() || !prBase.second || !prBase.second->Get<CExceptionEntity>())
            throw CCompileException("Exception definition not found.");
        m_vecSetRaisesExceptions.push_back(prBase.second);
        log << "Entity could raise exception on set attribute: " << prBase.first << std::endl;
    }
    for (const CTokenList& rlstTokenList : m_vecGetRaisesExceptionsTokenList)
    {
        log << "Processing get-raising exception..." << std::endl;
        std::pair<std::string, CEntityPtr> prBase = ProcessScopedName(rlstTokenList);
        if (prBase.first.empty() || !prBase.second || !prBase.second->Get<CExceptionEntity>())
            throw CCompileException("Exception definition not found.");
        m_vecGetRaisesExceptions.push_back(prBase.second);
        log << "Entity could raise exception on get attribute: " << prBase.first << std::endl;
    }

    // Processing is finalized...
    m_eProcAssState = EProcessAssignmentProgression::processed;
}

bool CDeclarationEntity::RequiresAssignment() const
{
    if (!ValueRef()) return false; // No value assigned yet...

    // If the type has an unbound array in its value, it requires assignment to determine the size of the type.
    CValueNodePtr ptrValue = ValueRef();
    while (ptrValue)
    {
        if (ptrValue->IsArray() && ptrValue->IsUnbound())
            return true;
        ptrValue = ptrValue->GetParentNode();
    }
    return false;
}

void CDeclarationEntity::CalcHash(CHashObject& rHash) const
{
    // Add the type
    if (m_typedecl.GetTypeDefinitionEntityPtr())
        m_typedecl.GetTypeDefinitionEntityPtr()->CalcHash(rHash);
    else
        rHash << m_typedecl.GetTypeString();

    // Add base entity
    CEntity::CalcHash(rHash);

    // Add array dimensions
    sdv::sequence<sdv::idl::SArrayDimension> seqArray = GetArrayDimensions();
    for (const sdv::idl::SArrayDimension& rsDimentation : seqArray)
    {
        rHash << "[";
        if (!rsDimentation.ssExpression.empty())
            rHash << rsDimentation.ssExpression;
        rHash << "]";
    }

    // Get the assignment
    std::string ssAssignment = GetAssignment();
    if (!ssAssignment.empty()) rHash << ssAssignment;

    // Add parameters
    for (const CEntityPtr& rptrEntity : m_vecParameters)
        rptrEntity->CalcHash(rHash);

    // Add exceptions
    for (const CEntityPtr& rptrEntity : m_vecRaisesExceptions)
    {
        rHash << "raises";
        rptrEntity->CalcHash(rHash);
    }

    // Add get-exceptions
    for (const CEntityPtr& rptrEntity : m_vecGetRaisesExceptions)
    {
        rHash << "get_raises";
        rptrEntity->CalcHash(rHash);
    }

    // Add set-exceptions
    for (const CEntityPtr& rptrEntity : m_vecSetRaisesExceptions)
    {
        rHash << "set_raises";
        rptrEntity->CalcHash(rHash);
    }

    // Add whether it is readonly
    if (IsReadOnly())
        rHash << "const";
}

