#ifndef DBCPARSER_H
#define DBCPARSER_H

#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <utility>
#include <set>
#include <map>
#include <memory>

/**
 * @brief DBC namespace
*/
namespace dbc
{
    /**
     * @brief DBC source management class
     */
    class CDbcSource
    {
    public:
        /**
         * @brief Default constructor
         */
        CDbcSource() = default;

        /**
        * @brief Open and read the DBC source file.
        * @param[in] rpathDbcfile Reference to the DBC file path.
        */
        CDbcSource(const std::filesystem::path& rpathDbcfile);

        /**
        * @brief Set DBC content.
        * @param[in] rssContent Reference to the DBC content.
        */
        CDbcSource(const std::string& rssContent);

        /**
         * @brief Is a valid source?
         * @return Returns whether the source is valid (has content).
         */
        bool IsValid() const;

        /**
         * @brief Get the currently assigned path.
         * @return The path to the source or an empty path when no path was assigned.
         */
        const std::filesystem::path& Path() const;

        /**
         * @brief Return a reference to the complete content.
         * @return Reference to the content string.
         */
        const std::string& Content() const;

        /**
         * @brief Return a pointer to the content at the current location.
         * @return Pointer to a character string starting at the current location.
        */
        const char* ContentPtr() const;

        /**
         * @brief Return the character at the current location.
         * @return Character or '\0' when there is no character at the current location.
         */
        char CurrentChar() const;

        /**
         * \{
         * @brief Return the current position.
         * @return A reference to the position.
        */
        const size_t& Pos() const;
        size_t& Pos();
        /**
         * \}
         */

        /**
         * @brief Reset the position to the beginning of the content.
         */
        void ResetPos();

        /**
         * @brief Does the current position point to the end of the content?
         * @return Returns whether the end of the content has been reached.
         */
        bool IsEOF() const;

        /**
         * @brief A position lock structure, allowing to rollback to a specific position.
         */
        struct SPosLock
        {
            /**
             * @brief Default constructor
             */
            SPosLock() = default;

            /**
             * @brief Constructor
             * @param[in] rThis Reference to the source class.
            */
            SPosLock(CDbcSource& rThis);

            /**
             * @brief Deleted copy constructor
             */
            SPosLock(const SPosLock&) = delete;

            /**
             * @brief Move constructor.
             * @param[in] rsPosLock Reference to the SPosLock structure to move from.
             */
            SPosLock(SPosLock&& rsPosLock) noexcept;

            /**
             * @brief Destructor
             */
            ~SPosLock();

            /**
             * @brief Deleted assignment operator.
             * @return Reference to this SPosLock structure.
            */
            SPosLock& operator=(const SPosLock&) = delete;

            /**
             * @brief Move operator.
             * @param[in] rsPosLock Reference to the SPosLock structure to move from.
             * @return Reference to this SPosLock structure.
             */
            SPosLock& operator=(SPosLock&& rsPosLock) noexcept;

            /**
             * @brief Accespt the current position; releases the lock.
             */
            void Promote();

            /**
             * @brief Rollback to the stored position and releases the lock.
             */
            void Rollback();

        private:
            CDbcSource*     pSource = nullptr;      ///< Pointer to the DBC source the lock is held on.
            size_t          nStoredPos = 0;         ///< The stored position that can be rolled back to.
        };

        /**
         * @brief Create a position lock structure.
         * @return The position lock structure that allows a rollback.
         */
        SPosLock CreatePosLock();

        /**
         * @brief Calculate the line the current position is on.
         * @return The line number starting with 1 for the first line.
         */
        size_t CalcLine() const;

        /**
         * @brief Calculate the column the current position is on.
         * @return The column number starting with 1 for the first column.
         */
        size_t CalcColumn() const;

    private:
        std::filesystem::path   m_pathFile;         ///< DBC file location (if available).
        std::string             m_ssContent;        ///< DBC content string.
        size_t                  m_nPos = 0;         ///< Current position.
    };

    /**
     * @brief Parser exception class
     */
    struct SDbcParserException : std::exception
    {
        /**
        * @brief Constructor supplying a reason for this exception.
        * @param[in] rSource Reference to the source causing the exception.
        * @param[in] rssReason Reference to the string containing the reason string.
        */
        SDbcParserException(const CDbcSource& rSource, const std::string& rssReason) :
            source(rSource), ssMessage(rssReason)
        {
            CreateWhatString();
        }

        /**
         * @brief Constructor supplying a reason for this exception.
         * @param[in] rssReason Reference to the string containing the reason string.
         */
        SDbcParserException(const std::string& rssReason) : SDbcParserException(CDbcSource(), rssReason)
        {}

        /**
         * @brief Constructor supplying a reason containing parameters for this exception.
         * @details The constructor can be supplied a reason as well as zero or more parameters. The reason string can contain
         * placeholders that will be replaced by the parameter. The placeholder is identified with the percent '%' character following
         * a parameter index (starting at 1). If another character follows the percent character, this character will be added to the
         * message (e.g. the struing "%%" will result in a message containing "%"). If an index is supplied which is out of range, the
         * messages will include the "<unknown>" at the location the parameter is supposed to be.
         * @tparam TParams Parameter types.
         * @param[in] rSource Reference to the source causing the exception.
         * @param[in] rssReason Reference to the string containing the reason string.
         * @param[in] rgtParams Parameter pack containing the parameters.
         */
        template <typename... TParams>
        SDbcParserException(const CDbcSource& rSource, const std::string& rssReason, TParams... rgtParams) : source(rSource)
        {
            std::vector<std::string> vecArgs;
            BuildParamList(vecArgs, rgtParams...);
            size_t nPos = 0;
            while (nPos < rssReason.size() && nPos != std::string::npos)
            {
                char c = rssReason[nPos++];
                switch (c)
                {
                case '%':   // Escaped
                {
                    std::string ssNumber;
                    while (c = rssReason[nPos++], std::isdigit(c))
                        ssNumber += c;
                    if (!ssNumber.empty())
                    {
                        size_t nIndex = static_cast<size_t>(std::atoll(ssNumber.c_str()));
                        if (nIndex && nIndex - 1 < vecArgs.size())
                            ssMessage += vecArgs[nIndex - 1];
                        else
                            ssMessage += "<unknown>";
                    }
                    if (c) ssMessage += c;
                    break;
                }
                default:
                    ssMessage += c;
                    break;
                }
            }
            CreateWhatString();
        }

        /**
        * @brief Constructor supplying a reason containing parameters for this exception.
        * @details The constructor can be supplied a reason as well as zero or more parameters. The reason string can contain
        * placeholders that will be replaced by the parameter. The placeholder is identified with the percent '%' character following
        * a parameter index (starting at 1). If another character follows the percent character, this character will be added to the
        * message (e.g. the struing "%%" will result in a message containing "%"). If an index is supplied which is out of range, the
        * messages will include the "<unknown>" at the location the parameter is supposed to be.
        * @tparam TParams Parameter types.
        * @param[in] rssReason Reference to the string containing the reason string.
        * @param[in] rgtParams Parameter pack containing the parameters.
        */
        template <typename... TParams>
        SDbcParserException(const std::string& rssReason, TParams... rgtParams) :
            SDbcParserException(CDbcSource(), rssReason, rgtParams...)
        {}

        /**
         * @brief Return the reason text of this exception.
         * @return The reason text.
         */
        virtual const char* what() const noexcept override
        {
            return ssWhat.c_str();
        }

        /**
         * @brief Get the source attached to the exception.
         * @return Reference to the source.
         */
        const CDbcSource& Source() const
        {
            return source;
        }

        /**
        * @brief Set the source attached to the exception.
        * @param[in] rSource Reference to the source to assign.
        */
        void Source(const CDbcSource& rSource)
        {
            source = rSource;
            CreateWhatString();
        }

    private:
        /**
         * @brief Build the parameter list - default function without any parameters. Does nothing.
         * \verbatim
         * @param[in] rvecParams Reference to the parameter list.
         * \endverbatim
         */
        void BuildParamList(std::vector<std::string>& /*rvecParams*/) {}

        /**
         * @brief Build the parameter list - add the parameters to the supplied vector.
         * @tparam TParam The first parameter type in the parameter list.
         * @tparam TParams The leftover parameter types in the parameter list.
         * @param[in] rvecParams Reference to the parameter list.
         * @param[in] tParam The first parameter.
         * @param[in] rgtParams Parameter pack containing the leftover parameter.
        */
        template <typename TParam, typename... TParams>
        void BuildParamList(std::vector<std::string>& rvecParams, TParam tParam, TParams... rgtParams)
        {
            std::stringstream sstream;
            sstream << tParam;
            rvecParams.push_back(std::move(sstream.str()));
            BuildParamList(rvecParams, rgtParams...);
        }

        /**
         * @brief Create what-string
         */
        void CreateWhatString()
        {
            std::stringstream sstream;
            if (source.IsValid())
                sstream << source.Path().generic_u8string() << "[" << source.CalcLine() << ", " << source.CalcColumn() << "]: ";
            sstream << ssMessage;
            ssWhat = std::move(sstream.str());
        }

        CDbcSource      source;         ///< The source with location that might have caused the exception.
        std::string     ssMessage;      ///< The message this exception represents (resolves parameters).
        std::string     ssWhat;         ///< String containing source and message information to be returned by what() function.
    };

    /**
     * @brief Value description type.
     */
    using TValDescMap = std::map<uint32_t, std::string>;

    /**
     * @brief Attribute definition.
     */
    struct SAttributeDef
    {
        /**
        * @brief Attribute type
        */
        enum class EType
        {
            integer,            ///< Integer attribute
            hex_integer,        ///< Hexadecimal integer attribute
            floating_point,     ///< Floating point attribute
            string,             ///< String attribute
            enumerator          ///< Enumerator attribute
        };

        /**
         * @brief Default constructor
         * @param[in] eTypeParam The attribute type (this cannot be changed any more).
         */
        SAttributeDef(EType eTypeParam);

        /**
         * @brief Destructor
         */
        ~SAttributeDef();

        /**
         * @brief Copy constructor
         * @param[in] rAttrDef Reference to the attribute definition to copy from.
         */
        SAttributeDef(const SAttributeDef& rAttrDef);

        /**
         * @brief Move constructor
         * @param[in] rAttrDef Reference to the attribute definition to move from.
         */
        SAttributeDef(SAttributeDef&& rAttrDef);

        /**
         * @brief Assignment operator
         * @param[in] rAttrDef Reference to the attribute definition to assign from.
         * @return Reference to this attribute definition.
         */
        SAttributeDef& operator=(const SAttributeDef& rAttrDef);

        /**
         * @brief Move operator
         * @param[in] rAttrDef Reference to the attribute definition to move from.
         * @return Reference to this attribute definition.
         */
        SAttributeDef& operator=(SAttributeDef&& rAttrDef);

        /**
         * @brief Object type this attribute is aiming for
         */
        enum class EObjectType
        {
            global,         ///< Global attribute definition (independent)
            node,           ///< Node specific attribute definition
            message,        ///< Message specific attribute definition
            signal,         ///< Signal specific attribute definition
            envvar,         ///< Environment variable specific attribute definition
        };

        EObjectType         eObjType = EObjectType::global;     ///< Object type
        std::string         ssName;                             ///< Attribute name
        const EType         eType;                              ///< Type of the attribute

        /// Unnamed union based on the attribute definition type
        union
        {
            struct
            {
                int32_t                     iMinimum;       ///< Minimum value for integer attribute
                int32_t                     iMaximum;       ///< Maximum value for integer attribute
                int32_t                     iDefault;       ///< Default value for integer attribute
            } sIntValues;                                   ///< Integer value structure
            struct
            {
                uint32_t                    uiMinimum;      ///< Minimum value for hexadecimal attribute
                uint32_t                    uiMaximum;      ///< Maximum value for hexadecimal attribute
                uint32_t                    uiDefault;      ///< Default value for hexadecimal attribute
            } sHexValues;                                   ///< Hexadecimal value structure
            struct
            {
                double                      dMinimum;       ///< Minimum value for floating point attribute
                double                      dMaximum;       ///< Maximum value for floating point attribute
                double                      dDefault;       ///< Default value for floating point attribute
            } sFltValues;                                   ///< Floating point value structure
            struct
            {
                std::string                 ssDefault;      ///< Default value for string attribute
            } sStringValues;                                ///< String value structure
            struct
            {
                std::vector<std::string>    vecEnumValues;  ///< Values for enumerator attribute
                std::string                 ssDefault;      ///< Default value for enumerator attribute
            } sEnumValues;                                  ///< Enumerator value structure
        };
    };

    /**
     * @brief Attribute definition shaed pointer.
    */
    using TAttributeDefPtr = std::shared_ptr<SAttributeDef>;

    /**
     * @brief Attribute value
    */
    struct SAttributeValue
    {
        /**
         * @brief Constructor
         * @param[in] ptrAttrDefParam Reference to the attribute definition.
         */
        SAttributeValue(TAttributeDefPtr& ptrAttrDefParam);

        /**
         * @brief Destructor
         */
        ~SAttributeValue();

        /**
         * @brief Copy constructor
         * @param[in] rAttrVal Reference to the attribute value to copy from.
         */
        SAttributeValue(const SAttributeValue& rAttrVal);

        /**
         * @brief Move constructor
         * @param[in] rAttrVal Reference to the attribute value to move from.
         */
        SAttributeValue(SAttributeValue&& rAttrVal);

        /**
         * @brief Assignment operator
         * @param[in] rAttrVal Reference to the attribute value to assign from.
         * @return Reference to this attribute value.
         */
        SAttributeValue& operator=(const SAttributeValue& rAttrVal);

        /**
         * @brief Move operator
         * @param[in] rAttrVal Reference to the attribute value to move from.
         * @return Reference to this attribute value.
         */
        SAttributeValue& operator=(SAttributeValue&& rAttrVal);

        TAttributeDefPtr    ptrAttrDef;     ///< Shared pointer to the attribute definition

        /// Unnamed union based on the attribute definition type.
        union
        {
            int32_t         iValue;         ///< Value for integer attribute
            uint32_t        uiValue;        ///< Value for hexadecimal attribute
            double          dValue;         ///< Value for floating point attribute
            std::string     ssValue;        ///< Value for string and enumerator attribute
        };
    };

    /**
     * @brief Node definition
     */
    struct SNodeDef
    {
        std::string                     ssName;         ///< Node name
        std::vector<std::string>        vecComments;    ///< Comments
        std::vector<SAttributeValue>    vecAttributes;  ///< Attribute values.
    };

    /**
     * @brief Signal type definition.
     */
    struct SSignalTypeBase
    {
        uint32_t        uiSize = 0;                     ///< Size of the signal

        /**
         * @brief Signal byte order
         */
        enum class EByteOrder
        {
            big_endian,     ///< Big endian byte order
            little_endian,  ///< Little endian byte order
        };

        EByteOrder      eByteOrder = EByteOrder::big_endian;    ///< Signal's byte order

        /**
         * @brief Value type
         */
        enum class EValueType
        {
            signed_integer,         ///< Signed integer
            unsigned_integer,       ///< Unsigned integer
            ieee_float,             ///< IEEE float
            ieee_double,            ///< IEEE double
        };

        EValueType                  eValType = EValueType::signed_integer;  ///< Value type
        double                      dFactor = 1.0;                          ///< Factor value: physical_value = raw_value * factor + offset
        double                      dOffset = 0.0;                          ///< Offset value
        double                      dMinimum = 0.0;                         ///< Minimum value
        double                      dMaximum = 0.0;                         ///< Maximum value
        std::string                 ssUnit;                                 ///< Unit
    };

    /**
     * @brief Signal type definition base
     */
    struct SSignalDef : SSignalTypeBase
    {
        uint32_t                        uiMsgId = 0u;                   ///< Message ID this signal belongs to
        std::string                     ssName;                         ///< Signal name

        /**
         * @brief Type of signal
         */
        enum class EMultiplexBitmask : uint32_t
        {
            normal = 0,             ///< Not part of multiplexing
            mltplx_switch = 1,      ///< Multiplexor signal
            mltplx_val = 2,         ///< Multiplexed signal
        };

        int32_t                         iMltplxCase = 0ll;              ///< For multiplexed signals: case value. Valid when
                                                                        ///< uiMultiplexBitmask has bit mltplx_val set.
        uint32_t                        uiMultiplexBitmask = 0u;        ///< Signal type
        uint32_t                        uiStartBit = 0;                 ///< Start bit
        std::vector<std::string>        vecReceivers;                   ///< Vector containing receivers
        TValDescMap                     mapValueDescriptions;           ///< Value descriptions
        std::string                     ssSignalTypeDef;                ///< When available, the type definition of the signal
                                                                        ///< (overriding the existing definition).
        std::vector<std::string>        vecComments;                    ///< Comments
        std::vector<SAttributeValue>    vecAttributes;                  ///< Attribute values.

        /**
         * @brief Extended multiplex information
         */
        struct SExtendedMultiplex
        {
            SSignalDef&                                 rsMultiplexor;  ///< Multiplexor signal
            std::vector<std::pair<uint32_t, uint32_t>>  vecRanges;      ///< Ranges of values that validates this signal
        };
        std::vector<SExtendedMultiplex> vecExtMultiplex;                ///< Extended multiplexing information
    };

    /**
     * @brief Signal type definition
     */
    struct SSignalTypeDef : SSignalTypeBase
    {
        std::string                 ssName;             ///< Signal type definition name
        double                      dDefaultValue;      ///< Default value
        std::string                 ssValueTable;       ///< Value table with signal value descriptions
    };

    /**
    * @brief Signal group definition; signals within a message that should be updated together.
    */
    struct SSignalGroupDef
    {
        std::string                 ssName;             ///< Signal group name
        uint32_t                    uiRepetitions = 0;  ///< Amount of repetitions.
        std::vector<std::string>    vecSignals;         ///< Signal definition names.
    };

    /**
     * @brief Message definition
     */
    struct SMessageDef
    {
        std::string                             ssName;             ///< Message name.
        uint32_t                                uiId = 0;           ///< Message ID (covers standard and extended ID).
        uint32_t                                uiSize = 0;         ///< Size of the message.
        std::vector<std::string>                vecTransmitters;    ///< Transmitter node names or "Vector__XXX".
        std::vector<SSignalDef>                 vecSignals;         ///< Vector with signals.
        std::map<std::string, SSignalGroupDef>  mapSigGroups;       ///< Map containing signal groups
        std::vector<std::string>                vecComments;        ///< Comments
        std::vector<SAttributeValue>            vecAttributes;      ///< Attribute values.
    };

    /**
     * @brief Message definition shared pointer.
    */
    using TMessageDefPtr = std::shared_ptr<SMessageDef>;

    /**
     * @brief Environment variable definition
     */
    struct SEnvVarDef
    {
        std::string                     ssName;                                 ///< Variable name

        /**
         * @brief Type of the variable
         */
        enum class EType
        {
            integer,            ///< The variable is an integer number
            floating_point,     ///< The variable is a floating point number
            string,             ///< The variable is a string
            data,               ///< The variable is a data type
        };

        EType                           eType = EType::integer;                 ///< Variable type
        double                          dMinimum = 0.0;                         ///< Minimum value
        double                          dMaximum = 0.0;                         ///< Maximum value
        std::string                     ssUnit;                                 ///< Unit
        double                          dInitVal = 0.0;                         ///< Maximum value
        uint32_t                        uiId = 0u;                              ///< Variable ID (obsolete)

        /**
         * @brief Access type if the variable
         */
        enum class EAccessType
        {
            unrestricted,       ///< Unrestricted access
            read,               ///< Read access
            write,              ///< Write access
            readwrite,          ///< Read+write access
        };

        EAccessType                     eAccess = EAccessType::unrestricted;    ///< Access type
        std::vector<std::string>        vecNodes;                               ///< Access nodes
        TValDescMap                     mapValueDescriptions;                   ///< Value descriptions
        uint32_t                        uiDataSize = 0;                         ///< Data size in bytes (when variable is data type)
        std::vector<std::string>        vecComments;                            ///< Comments
        std::vector<SAttributeValue>    vecAttributes;                          ///< Attribute values.
    };

    /**
     * @brief DBC parser class allowing access to the defined data structures.
     */
    class CDbcParser
    {
    public:
        /**
         * @brief Default constructor.
         * @param[in] bNoDefaultDef When set, no default definitions should be made.
         */
        CDbcParser(bool bNoDefaultDef = false);

        /**
         * @brief Parse the DBC content. The current content will be concatenated to the already existing content.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         */
        void Parse(CDbcSource& rSource);

        /**
         * @brief Clears the DBC information. Allowing new content to be added.
         */
        void Clear();

        /**
        * @brief Return a reference to the list of sources.
        * @return The reference to the vector containing the sources.
        */
        const std::vector<CDbcSource>& GetSources() const;

        /**
         * @brief Get the vector containing the version strings of all parsed DBC sources.
         * @return Reference to the vector containing the version strings.
         */
        const std::vector<std::string>& GetVersions() const;

        /**
         * @brief Has a node been defined?
         * @param[in] rssNodeDefName Reference to the string containing the name of the node.
         * @return Returns 'true' when a node has been defined; 'false' otherwise.
         */
        bool HasNodeDef(const std::string& rssNodeDefName) const;

        /**
         * @brief Get the vector containing the node definitions of all parsed DBC sources.
         * @return Vector containing the node definition strings.
         */
        const std::vector<std::string> GetNodeDefNames() const;

        /**
         * @brief Get the node definition with the supplied name.
         * @param[in] rssNodeDefName Reference to the string containing the name of the node.
         * @return Pair of the node and a boolean indicating whether the node exists.
         */
        std::pair<SNodeDef, bool> GetNodeDef(const std::string& rssNodeDefName) const;

        /**
         * @brief Is the value table with the supplied name defined?
         * @param[in] rssName Reference to the table name string.
         * @return Returns 'true' when the table with the supplied name has been defined; 'false' otherwise.
         */
        bool HasValueTable(const std::string& rssName) const;

        /**
         * @brief Is the value within the value table with the supplied table name defined?
         * @param[in] rssTableName Reference to the table name string.
         * @param[in] uiValue Value within the table.
         * @return Returns 'true' when the value within the table has been defined; 'false' otherwise.
         */
        bool HasValue(const std::string& rssTableName, uint32_t uiValue) const;

        /**
         * @brief Get the value within the value table with the supplied table name defined?
         * @param[in] rssTableName Reference to the table name string.
         * @param[in] uiValue Value within the table.
         * @return Returns the string when the value within the table has been defined; an empty string otherwise.
         */
        std::string GetValue(const std::string& rssTableName, uint32_t uiValue) const;

        /**
         * @brief Get a vector with all the value tables defined.
         * @return Vector with value table names.
         */
        std::vector<std::string> GetValueTableNames() const;

        /**
         * @brief Get a vector with all values for a value table.
         * @param[in] rssTableName Reference to the table name string.
         * @return Pair of a vector with values and a boolean indicating whether the table exists.
         */
        std::pair<std::vector<uint32_t>, bool> GetValues(const std::string& rssTableName) const;

        /**
         * @brief Has a message been defined?
         * @param[in] rssName Reference to the string containing the name of the message.
         * @return Returns 'true' when a message has been defined; 'false' otherwise.
         */
        bool HasMsgDef(const std::string& rssName) const;

        /**
         * @brief Has a message been defined?
         * @param[in] uiRawId ID of the message (covers both standard and extended ID).
         * @return Returns 'true' when a message has been defined; 'false' otherwise.
         */
        bool HasMsgDef(uint32_t uiRawId) const;

        /**
         * @brief Has a message been defined (lookup with standard ID)?
         * @param[in] uiStdId ID of the message (max 11 bits).
         * @return Returns 'true' when a message has been defined; 'false' otherwise.
         */
        bool HasMsgDefStdId(uint32_t uiStdId) const;

        /**
         * @brief Has a message been defined (lookup with extended ID)?
         * @param[in] uiExtId ID of the message (max 29 bits).
         * @return Returns 'true' when a message has been defined; 'false' otherwise.
         */
        bool HasMsgDefExtId(uint32_t uiExtId) const;

        /**
         * @brief Get the currently stored message IDs.
         * @return Vector containing the raw message IDs (covering both standard and extended IDs).
         */
        std::vector<uint32_t> GetMessageIDs() const;

        /**
         * @brief Get the message definition.
         * @param[in] rssName Reference to the string containing the name of the message.
         * @return Returns a pair containing the message and a boolean. If the message was found, the boolean is 'true'; otherwise the
         * boolean was 'false' and an empty message was returned.
         */
        std::pair<SMessageDef, bool> GetMsgDef(const std::string& rssName) const;

        /**
         * @brief Get the message ID from a raw ID.
         * @param[in] uiRawId ID of the message (covers both standard and extended ID).
         * @return Returns a pair with the message ID and a boolean indicating whether the ID is extended or not.
         */
        static std::pair<uint32_t, bool> ExtractMsgId(uint32_t uiRawId);

        /**
         * @brief Compose a raw ID from a message ID.
         * @param[in] uiMsgId The ID of the message.
         * @param[in] bExtended When set, the message ID is an extended ID (29 bits); otherwise it is a standard ID (11 bits).
         * @return Returns the raw ID.
         */
        static uint32_t ComposeRawId(uint32_t uiMsgId, bool bExtended);

        /**
         * @brief Get the message definition.
         * @param[in] uiRawId ID of the message (covers both standard and extended ID).
         * @return Returns a pair containing the message and a boolean. If the message was found, the boolean is 'true'; otherwise the
         * boolean was 'false' and an empty message was returned.
         */
        std::pair<SMessageDef, bool> GetMsgDef(uint32_t uiRawId) const;

        /**
         * @brief Get the message definition (lookup with standard ID)?
         * @param[in] uiStdId ID of the message (max 11 bits).
         * @return Returns a pair containing the message and a boolean. If the message was found, the boolean is 'true'; otherwise the
         * boolean was 'false' and an empty message was returned.
         */
        std::pair<SMessageDef, bool> GetMsgDefStdId(uint32_t uiStdId) const;

        /**
         * @brief Get the message definition (lookup with extended ID)?
         * @param[in] uiExtId ID of the message (max 29 bits).
         * @return Returns 'true' when a message has been defined; 'false' otherwise.
         */
        std::pair<SMessageDef, bool> GetMsgDefExtId(uint32_t uiExtId) const;

        /**
         * @brief Has a signal been defined?
         * @param[in] rssMsgName Reference to the string containing the name of the message.
         * @param[in] rssSignalName Reference to the string containing the name of the signal.
         * @return Returns 'true' when a signal has been defined; 'false' otherwise.
         *
         */
        bool HasSignalDef(const std::string & rssMsgName, const std::string & rssSignalName) const;

        /**
         * @brief Has a signal been defined?
         * @param[in] uiRawMsgId ID of the message (covers both standard and extended ID).
         * @param[in] rssSignalName Reference to the string containing the name of the signal.
         * @return Returns 'true' when a signal has been defined; 'false' otherwise.
         */
        bool HasSignalDef(uint32_t uiRawMsgId, const std::string & rssSignalName) const;

        /**
         * @brief Has a signal been defined?
         * @param[in] uiStdMsgId ID of the message (max 11 bits).
         * @param[in] rssSignalName Reference to the string containing the name of the signal.
         * @return Returns 'true' when a signal has been defined; 'false' otherwise.
         */
        bool HasSignalDefStdId(uint32_t uiStdMsgId, const std::string & rssSignalName) const;

        /**
         * @brief Has a signal been defined?
         * @param[in] uiExtMsgId ID of the message (max 29 bits).
         * @param[in] rssSignalName Reference to the string containing the name of the signal.
         * @return Returns 'true' when a signal has been defined; 'false' otherwise.
         */
        bool HasSignalDefExtId(uint32_t uiExtMsgId, const std::string & rssSignalName) const;

        /**
         * @brief Get the currently stored signal names for the supplied message.
         * @param[in] uiRawMsgId ID of the message (covers both standard and extended ID).
         * @return Vector containing the signal names.
         */
        std::vector<std::string> GetSignalNames(uint32_t uiRawMsgId) const;

        /**
         * @brief Get the message definition.
         * @param[in] rssMsgName Reference to the string containing the name of the message.
         * @param[in] rssSignalName Reference to the string containing the name of the signal.
         * @return Returns a pair containing the message and a boolean. If the message was found, the boolean is 'true'; otherwise the
         * boolean was 'false' and an empty message was returned.
         */
        std::pair<SSignalDef, bool> GetSignalDef(const std::string & rssMsgName, const std::string & rssSignalName) const;

        /**
         * @brief Get the message definition.
         * @param[in] uiRawMsgId ID of the message (covers both standard and extended ID).
         * @param[in] rssSignalName Reference to the string containing the name of the signal.
         * @return Returns a pair containing the message and a boolean. If the message was found, the boolean is 'true'; otherwise the
         * boolean was 'false' and an empty message was returned.
         */
        std::pair<SSignalDef, bool> GetSignalDef(uint32_t uiRawMsgId, const std::string & rssSignalName) const;

        /**
         * @brief Get the message definition.
         * @param[in] uiStdMsgId ID of the message (max 11 bits).
         * @param[in] rssSignalName Reference to the string containing the name of the signal.
         * @return Returns a pair containing the message and a boolean. If the message was found, the boolean is 'true'; otherwise the
         * boolean was 'false' and an empty message was returned.
         */
        std::pair<SSignalDef, bool> GetSignalDefStdId(uint32_t uiStdMsgId, const std::string & rssSignalName) const;

        /**
         * @brief Get the message definition.
         * @param[in] uiExtMsgId ID of the message (max 29 bits).
         * @param[in] rssSignalName Reference to the string containing the name of the signal.
         * @return Returns a pair containing the message and a boolean. If the message was found, the boolean is 'true'; otherwise the
         * boolean was 'false' and an empty message was returned.
         */
        std::pair<SSignalDef, bool> GetSignalDefExtId(uint32_t uiExtMsgId, const std::string & rssSignalName) const;

        /**
         * Get a vector with the environment variable names.
         * @return Vector with name strings.
         */
        std::vector<std::string> GetEnvVarNames() const;

        /**
         * @brief Get the environment variable definition.
         * @param[in] rssVarName Reference to the string containing the name of the environment variable.
         * @return Returns a pair containing the variable and a boolean. If the message was found, the boolean is 'true'; otherwise the
         * boolean is 'false' and an empty variable is returned.
         */
        std::pair<SEnvVarDef, bool> GetEnvVarDef(const std::string & rssVarName) const;

        /**
         * @brief Get a vector with the names of the signal type definitions.
         * @return Vector with name strings.
         */
        std::vector<std::string> GetSignalTypeDefNames() const;

        /**
         * @brief Get the signal type definition belonging to the provided definition name.
         * @param[in] rssSignalTypeDefName Reference to the string containing the name of the signal type definition to return.
         * @return Returns a pair containing the signal type definition and a boolean. If the type definition was found, the boolean is
         * 'true'; otherwise the boolean is 'false' and an empty type definition is returned.
         */
        std::pair<SSignalTypeDef, bool> GetSignalTypeDef(const std::string& rssSignalTypeDefName) const;

        /**
        * @brief Get a vector with the names of the signal groups definitions.
        * @param[in] uiRawMsgId ID of the message (covers both standard and extended ID).
        * @return Vector with name strings.
        */
        std::vector<std::string> GetSignalGroupDefNames(uint32_t uiRawMsgId) const;

        /**
        * @brief Get the signal group definition belonging to the provided name.
        * @param[in] uiRawMsgId ID of the message (covers both standard and extended ID).
        * @param[in] rssSignalGroupDefName Reference to the string containing the name of the signal group definition to return.
        * @return Returns a pair containing the signal group definition and a boolean. If the group definition was found, the boolean
        * is 'true'; otherwise the boolean is 'false' and an empty group definition is returned.
        */
        std::pair<SSignalGroupDef, bool> GetSignalGroupDef(uint32_t uiRawMsgId, const std::string& rssSignalGroupDefName) const;

        /**
         * @brief Get the global DBC file comments.
         * @return Reference to the vector with comment strings.
         */
        const std::vector<std::string>& GetComments() const;

        /**
        * @brief Get a vector with the names of the global attribute definitions.
        * @return Vector with name strings.
        */
        std::vector<std::string> GetAttributeDefNames() const;

        /**
        * @brief Get the attribute definition belonging to the provided name.
        * @param[in] rssAttributeDefName Reference to the string containing the name of the attribute definition to return.
        * @return Returns a pair containing the attribute definition and a boolean. If the attribute definition was found, the boolean
        * is 'true'; otherwise the boolean is 'false' and an empty attribute definition is returned.
        */
        std::pair<SAttributeDef, bool> GetAttributeDef(const std::string& rssAttributeDefName) const;

        /**
         * @brief Get global attributes.
         * @return Reference to the vector with attribute values.
         */
        const std::vector<SAttributeValue>& GetAttributes() const;

    private:
        /**
         * @brief Skip whitespace and comments by updating the current source position.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         */
        static void SkipWhitespace(CDbcSource& rSource);

        /**
        * @brief Get an unsigned integer and update the current source position.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        * @return Pair containing the integer, if available, and a boolean indicating the presence of an integer.
        */
        static std::pair<uint32_t, bool> GetUInt(CDbcSource& rSource);

        /**
        * @brief Get a signed integer and update the current source position.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        * @return Pair containing the integer, if available, and a boolean indicating the presence of an integer.
        */
        static std::pair<int32_t, bool> GetInt(CDbcSource& rSource);

        /**
        * @brief Get a double value and update the current source position.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        * @return Pair containing the double, if available, and a boolean indicating the presence of a double.
        */
        static std::pair<double, bool> GetDouble(CDbcSource & rSource);

        /**
        * @brief Expect a character and update the current source position.
        * @param[in] c The character to expect.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        * @return Returns whether the character was found.
        */
        static bool ExpectChar(char c, CDbcSource& rSource);

        /**
        * @brief Get the and update the current source position. A string starts with a quote and ends with a quote can contain any
        * printable character except quote '"' and back-slash '\'.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        * @return Pair containing the string, if available, and a boolean indicating the presence of a string.
        */
        static std::pair<std::string, bool> GetString(CDbcSource& rSource);

        /**
         * @brief Get an identifier and update the current source position. An identifier starts with an alphanumerical character or
         * underscore and is followed by zero or more alphanumerical character or underscores as well as digits.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         * @return The identifier that was read or empty when there was no identifier.
         */
        static std::string GetIdentifier(CDbcSource& rSource);

        /**
         * @brief A DBC identifier is an identifier that is not a keyword.
         * @param[in] rssIdentifier Reference to the identifier string.
         * @return Returns whether the supplied identifier is not a keyword.
         */
        static bool IsDbcIdentifier(const std::string& rssIdentifier);

        /**
         * @brief Read the DBC version string.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         */
        void ReadVersion(CDbcSource& rSource);

        /**
         * @brief Read the new symbols used in this DBC file.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         */
        void ReadNewSymbols(CDbcSource& rSource);

        /**
        * @brief Read the bit timing definition.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        */
        void ReadBitTiming(CDbcSource& rSource);

        /**
         * @brief Read the node definitions.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         */
        void ReadNodeDef(CDbcSource& rSource);

        /**
         * @brief Read the global value table.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         */
        void ReadValTable(CDbcSource& rSource);

        /**
         * @brief Read the message definitions.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         */
        void ReadMessageDef(CDbcSource& rSource);

        /**
         * @brief Read the signal type definition (base part).
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         * @param[in, out] rsSignalTypeDefBase Reference to the signal type def.
         */
        void ReadSignalTypeDefBase(CDbcSource& rSource, SSignalTypeBase& rsSignalTypeDefBase);

        /**
         * @brief Read the signal definitions.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         * @param[in, out] rsMsgDef Reference to the message definition for this signal.
         */
        void ReadSignalDef(CDbcSource& rSource, SMessageDef& rsMsgDef);

        /**
         * @brief Read the signal value type definition.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         */
        void ReadSignalValueTypeDef(CDbcSource& rSource);

        /**
         * @brief Read the message transmitters.
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         */
        void ReadMessageTransmitters(CDbcSource& rSource);

        /**
         * @brief Read the value descriptions (for signals and environment variables).
         * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
         */
        void ReadValueDescriptions(CDbcSource& rSource);

        /**
        * @brief Read the environment variable definition.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        */
        void ReadEnvVarDef(CDbcSource& rSource);

        /**
        * @brief Read the environment variable data.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        */
        void ReadEnvVarData(CDbcSource& rSource);

        /**
        * @brief Read the signal type definition.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        */
        void ReadSignalTypeDef(CDbcSource& rSource);

        /**
        * @brief Read the signal group definition.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        */
        void ReadSignalGroupDef(CDbcSource& rSource);

        /**
        * @brief Read the comment definitions.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        */
        void ReadCommentDef(CDbcSource& rSource);

        /**
        * @brief Read user attribute definitions.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        */
        void ReadAttrDef(CDbcSource& rSource);

        /**
        * @brief Read user attribute default definitions.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        */
        void ReadAttrDefaultDef(CDbcSource& rSource);

        /**
        * @brief Read user attributes.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        */
        void ReadAttributes(CDbcSource& rSource);

        /**
        * @brief Read the signal multiplex definition.
        * @param[in] rSource Reference to the DBC source. The position within the source will be updated.
        */
        void ReadSignalMultiplexDef(CDbcSource& rSource);

        std::vector<CDbcSource>                   m_vecSources;                 ///< Vector containing the DBC sources.
        std::vector<std::string>                  m_vecVersions;                ///< Vector containing the versions of the parsed
                                                                                ///< files.
        std::map<std::string, SNodeDef>           m_mapNodes;                   ///< Set containing all the nodes.
        std::map<std::string, TValDescMap>        m_mapValueTables;             ///< Value tables.
        std::map<std::string, TMessageDefPtr>     m_mapMsgDefByName;            ///< Message definitions sorted by name.
        std::map<uint32_t, TMessageDefPtr>        m_mapMsgDefById;              ///< Message definitions sorted by ID.
        std::map<std::string, SEnvVarDef>         m_mapEnvVars;                 ///< Map containing the environment variables.
        std::map<std::string, SSignalTypeDef>     m_mapSigTypeDefs;             ///< Map containing the signal type definitions.
        std::vector<std::string>                  m_vecComments;                ///< Global comments
        std::vector<SAttributeValue>              m_vecAttributes;              ///< Vector containing the global attribute values.
        uint32_t                                  m_uiIndepMsgId = 0xffffffff;  ///< Current ID for Vector independent message.
        std::map<std::string, TAttributeDefPtr>   m_mapAttrDefs;                ///< Map containing the attribute definitions.
    };

} // namespace dbc

#endif // !defined DBCPARSER_H