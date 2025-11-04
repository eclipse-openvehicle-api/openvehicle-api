#ifndef TOKENLIST_H
#define TOKENLIST_H

#include "token.h"
#include <list>

/**
 * @brief Token list class with extra functions to navigate through the list safely.
 */
class CTokenList : public std::list<CToken>
{
public:
    /**
     * @brief Default constructor.
     */
    CTokenList();

    /**
     * @brief Copy constructor
     * @param[in] rlstTokens Reference to the token list.
     */
    CTokenList(const CTokenList& rlstTokens);

    /**
     * @brief Move constructor
     * @param[in] rlstTokens Reference to the token list.
     */
    CTokenList(CTokenList&& rlstTokens) noexcept;

    /**
     * @brief Assignment operator
     * @param[in] rlstTokens Reference to the token list.
     * @return Reference to this tokenlist.
     */
    CTokenList& operator=(const CTokenList& rlstTokens);

    /**
     * @brief Move operator
     * @param[in] rlstTokens Reference to the token list.
     * @return Reference to this tokenlist.
     */
    CTokenList& operator=(CTokenList&& rlstTokens) noexcept;

    /**
     * @brief Get the token relative to the current position or if not valid an empty token.
     * @param[in] nIndex The amount of tokens to skip before returning the token.
     * @return Reference to the token.
     */
    const CToken& Current(size_t nIndex = 0) const;

    /**
     * @brief Get the current or last valid token. If there is no token, get an empty token.
     * @return Reference to the current token.
     */
    const CToken& LastValid() const;

    /**
     * @brief Increase the current token to the next position in the tokenlist.
     * @returns Copy of this tokenlist before incrementation.
     */
    CTokenList operator++(int) const;

    /**
     * @brief Increase the current token to the next position in the tokenlist.
     * @returns Reference to this tokenlist.
     */
    const CTokenList& operator++() const;

    /**
	 * @brief Returns whether the current token has reached the end of the list or whether the list is empty.
	 * @return Returns 'true' when the end of the token list has been reached; otherwise 'false'.
	 */
	bool End() const;

	/**
     * \{
     * @brief Add a token to the front of the list. Overload of std::list::push_front.
     * @param[in, out] rToken Reference to the token to insert.
     */
    void push_front(const CToken& rToken);
    void push_front(CToken&& rToken);
    /**
     * @}
     */

    /**
     * @brief Removes the first element of the container. Overload of std::list::pop_front.
     */
    void pop_front();

    /**
     * \{
     * @brief Add a token to the end of the list. Overload of std::list::push_back.
     * @param[in, out] rToken Reference to the token to insert.
     */
    void push_back(const CToken& rToken);
    void push_back(CToken&& rToken);
    /**
     * @}
     */

    /**
     * @brief Removes the last element of the container. Overload of std::list::pop_front.
     */
    void pop_back();

    /**
     * \{
     * @brief Insert a token at the current position in the list, moving the current position to the newly inserted token. Loose
     * overload of std::list::insert.
     * @param[in, out] rToken Reference to the token to insert.
     */
    void insert(const CToken& rToken);
    void insert(CToken&& rToken);
    /**
     * @}
     */


    /**
     * @brief Create a token with the supplied arguments and place it at the end of the list.
     * @tparam Args The argument types
     * @param[in] args The arguments
     * @return Reference to the token added.
     */
    template<class... Args>
    CToken& emplace_back(Args&&... args);

private:
    mutable std::list<CToken>::iterator     m_itCurrent;    ///< Current iterator
};

template<class... Args>
CToken& CTokenList::emplace_back(Args&&... args)
{
    bool bEmpty = empty();
    bool bEnd = bEmpty || m_itCurrent == end();
    CToken& rToken = std::list<CToken>::emplace_back(args...);
    if (bEmpty) m_itCurrent = begin();
    else if (bEnd)
    {
        m_itCurrent = end();
        --m_itCurrent;
    }
    return rToken;
}

#endif // !defined(TOKENLIST_H)