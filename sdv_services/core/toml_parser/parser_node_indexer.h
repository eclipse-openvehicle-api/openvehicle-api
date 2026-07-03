/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#ifndef PARSER_NODE_INDEXER_H
#define PARSER_NODE_INDEXER_H

#include <list>
#include <memory>
#include <cstdint>

namespace toml_parser
{
    class CNodeIndex;
    class CNodeIndexer;
    /// Placeholder index element
    struct SNodeIndexElement {};
    /// Index list
    using CIndexList = std::list<SNodeIndexElement>;
    /// Const iterator to an element in the index list
    using CIndexListIterator = CIndexList::const_iterator;

    /**
     * @brief Node index object used to manage the node order.
    */
    class CNodeIndex
    {
        friend CNodeIndexer;        ///< CNodeIndex uses friend access to create the node index and assign the iterator holding the
                                    ///< the position within the index list.
    private:
        /**
         * @brief Constructor of the node index object. Use the CNodeIndexer::CreateIndex function to create the object.
         * @param[in] rIndexList Reference to the index list.
         * @param[in] itPos Iterator to the position in the index list.
         */
        CNodeIndex(CIndexList& rIndexList, CIndexListIterator itPos);

    public:
        /**
         * @brief Destructor
         */
        virtual ~CNodeIndex();

        /**
         * @brief Copy constructor of the node index object.
         * @param[in] rIndex Reference to the index object to copy from.
         */
        CNodeIndex(const CNodeIndex& rIndex);

        /**
         * @brief Move constructor of the node index object.
         * @param[in] rIndex Reference to the index object to move from.
         */
        CNodeIndex(CNodeIndex&& rIndex);

        /**
         * @brief Copy assignment operator of the node index object.
         * @param[in] rIndex Reference to the index object to copy from.
         * @return Returns a reference to this object.
         */
        CNodeIndex& operator=(const CNodeIndex& rIndex);

        /**
         * @brief Move assignment operator of the node index object.
         * @param[in] rIndex Reference to the index object to move from.
         * @return Returns a reference to this object.
         */
        CNodeIndex& operator=(CNodeIndex&& rIndex);

        /**
         * @brief Compare this node index object to the supplied index object and return whether the position of this object is
         * identical to the position of the supplied object.
         * @param[in] rIndex Reference to the node index object to compare with.
         * @return Returns whether the provided index is identical to this index.
         */
        bool operator==(const CNodeIndex& rIndex) const;

        /**
         * @brief Compare this node index object to the supplied index object and return whether the position of this object is
         * not identical to the position of the supplied object.
         * @param[in] rIndex Reference to the node index object to compare with.
         * @return Returns whether the provided index is different than this index.
         */
        bool operator!=(const CNodeIndex& rIndex) const;

        /**
         * @brief Compare this node index object to the supplied index object and return whether the position of this object is
         * smaller than the position of the supplied object.
         * @param[in] rIndex Reference to the node index object to compare with.
         * @return Returns whether the provided index is smaller than this index.
         */
        bool operator<(const CNodeIndex& rIndex) const;

        /**
         * @brief Compare this node index object to the supplied index object and return whether the position of this object is
         * smaller than or equal to the position of the supplied object.
         * @param[in] rIndex Reference to the node index object to compare with.
         * @return Returns whether the provided index is smaller than or equal to this index.
         */
        bool operator<=(const CNodeIndex& rIndex) const;

        /**
         * @brief Compare this node index object to the supplied index object and return whether the position of this object is
         * larger than the position of the supplied object.
         * @param[in] rIndex Reference to the node index object to compare with.
         * @return Returns whether the provided index is larger than this index.
         */
        bool operator>(const CNodeIndex& rIndex) const;

        /**
         * @brief Compare this node index object to the supplied index object and return whether the position of this object is
         * larger than or equal to the position of the supplied object.
         * @param[in] rIndex Reference to the node index object to compare with.
         * @return Returns whether the provided index is larger than or equal to this index.
         */
        bool operator>=(const CNodeIndex& rIndex) const;

        /**
         * @brief Check for validity.
         * @return Returns whether the index object contains a valid index.
         */
        operator bool() const;

        /**
         * @brief Move the node index object before the index object provided as an argument.
         * @param[in] rIndex Reference to the node index object to move the object before.
         */
        void MoveBefore(const CNodeIndex& rIndex);

        /**
         * @brief Get the index in the list. Returns sdv::toml::npos if the index is not in the list any more.
         * @remarks This index os calculated dynamically and can change if indices are added, removed or swapped.
         * @return The current index in the list.
         */
        uint32_t Index() const;

    private:
        /**
         * @brief Iterator wrapper object with lifetime management.
         * @remarks The wrapper keeps track of the shared index list as well. It is possible that the parser is deleted before the
         * nodes are deleted, causing an issue when accessing the index list.
         */
        struct CIteratorWrapper
        {
            /**
             * @brief Constructor
             * @param[in] rIndexList Reference to the index list.
             * @param[in] itPos Position of this index in the index list.
            */
            CIteratorWrapper(CIndexList& rIndexList, CIndexListIterator itPos);
            
            /**
             * @brief Destructor removing the index object from the index list.
             */
            ~CIteratorWrapper();

            /**
             * @brief Get the index in the list.
             * @remarks This index os calculated dynamically and can change if indices are added, removed or swapped.
             * @return The current index in the list.
             */
            uint32_t Index() const;

            /**
             * @brief Move the index object before the supplied index object.
             * @param[in] ritTarget Reference to the index object to move the object before.
             */
            void MoveBeforeIndex(const CIteratorWrapper& ritTarget);

        private:
            CIndexList&         m_rIndexList;   ///< Reference to the index list.
            CIndexListIterator  m_itPos;        ///< Iterator in the list
        };

        std::shared_ptr<CIteratorWrapper>   m_ptrPos;   ///< Position in the index list
    };

    /**
     * @brief Node indexer object managing the node index objects allowing determining and changing the order of nodes.
     */
    class CNodeIndexer
    {
        friend CNodeIndex;      ///< CNodeIndex has friend relationship allowing access to specific indexing functions using the
                                ///< index iterator from the index list.
    public:

        /**
         * @brief Constructor
         */
        CNodeIndexer();

        /**
         * @brief Create a node index object.
         * @return The new node index object.
         */
        CNodeIndex CreateIndex();

        /**
         * @brief Create a new node index object and insert this object before the supplied object.
         * @attention There is no protection for supplying an invalid iterator or an iterator from a different indexer.
         * @param[in] rInsertBefore Insert the index object before the supplied iterator position.
         * @return The new node index object.
        */
        CNodeIndex CreateIndex(const CNodeIndex& rInsertBefore);

        /**
         * @brief Return the amount of indices allocated.
         * @return The amount of indices allocated in the index list.
         */
        static size_t Count();

    private:
        static CIndexList       m_lstIndexList;     ///< List of all node indices. This list is global to allow nodes from one
                                                    ///< parser to be inserted into the list of another parser.
    };
} // namespace toml_parser

#endif // !defined PARSER_NODE_INDEXER_H