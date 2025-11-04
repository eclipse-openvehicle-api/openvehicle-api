#ifndef ASC_FILE_READER_H
#define ASC_FILE_READER_H

#include <list>
#include <functional>
#include <filesystem>
#include <thread>

namespace asc
{
    /**
     * @brief CAN message structure
     */
    struct SCanMessage
    {
        double      dTimestamp;                         ///< Timestamp in seconds
        uint32_t    uiChannel;                          ///< CAN channel (first channel starts with 0).
        uint32_t    uiId;                               ///< CAN ID
        bool        bExtended;                          ///< Set when the CAN ID is extended (29 bits) or not when standard
                                                        ///< (11 bits).
        bool        bCanFd;                             ///< Set when the message contains CAN-FD data (more than 8 bytes).
        /// Direction enumeration
        enum class EDirection { rx, tx } eDirection;    ///< Direction RX or TX.
        uint32_t    uiLength;                           ///< Length of the CAN data.
        uint8_t     rguiData[64];                       ///< Array with the CAN data.
    };

    /**
     * @brief This class allows reading the Vector ASC file format.
     * @attention This class assumes no concurrency between reading a file, navigation and playback. No thread synchronization is
     * implemented.
     */
    class CAscReader
    {
    public:
        /**
         * @brief Constructor
         */
        CAscReader();

        /**
         * @brief Destructor
         */
        ~CAscReader();

        /**
         * @brief Read a file (this will replace the current samples with the samples of the file).
         * @param[in] rpathFile Reference to the file path.
         * @return Returns 'true' on success or 'false' when not.
        */
        bool Read(const std::filesystem::path& rpathFile);

        /**
         * @brief Get the current sample. If the current position is EOF, no sample is returned.
         * @return A pair consisting of the current sample and a boolean indicating that the sample is valid.
         */
        std::pair<SCanMessage, bool> Get() const;

        /**
         * @brief Get the number of loops the data set was sent
         * @return Number of loops the data set was sent including the current loop
         */        
        uint32_t GetLoopCount() const;
        
        /**
         * @brief Jump to the first sample.
         */
        void JumpBegin();

        /**
         * @brief Jump beyond the last sample.
         */
        void JumpEnd();

        /**
         * @{
         * @brief Increase the current position to the next sample.
         * @return Reference to this class.
         */
        CAscReader& operator++();
        CAscReader& operator++(int);
        /**
         * @}
         */

         /**
         * @{
         * @brief Decrease the current position to the previous sample.
         * @return Reference to this class.
         */
        CAscReader& operator--();
        CAscReader& operator--(int);
        /**
         * @}
         */

        /**
         * @brief Does the current position point to the first sample?
         * @return Returns whether the current position points to the first sample.
         */
        bool IsBOF() const;

        /**
         * @brief Does the current position point beyond the last sample?
         * @return Returns whether the current position points beyond the last sample.
        */
        bool IsEOF() const;

        /**
         * @brief Start playback using the timestamp of the samples to come as close to the original recording time.
         * @param[in] fnCallback The function being called with the current sample.
         * @param[in] bRepeat When set, the playback continuous at the beginning when reaching the end of the data set.
        */
        void StartPlayback(std::function<void(const SCanMessage&)> fnCallback, bool bRepeat = true);

        /**
         * @brief Stop the current playback.
         * @remarks This does not change the current position (pointing to the next sample following the one just sent.
         */
        void StopPlayback();

        /**
         * @brief Reset the playback to the beginning of the data set. This is equal to calling StopPlayback and JumpBegin.
         */
        void ResetPlayback();

        /**
         * @brief Returns whether playback is running at the moment.
         * @return Returns whether playback is running.
         */
        bool PlaybackRunning() const;

    private:
        /**
         * @brief Process a measurement value sample (one line within the ASC trigger block section).
         * @details The ASC measurement uses the following format for one CAN measurement value:
         * 
         * \verbatim 
         * For CAN: Timestamp Busnumber CanId ["x"] "Rx|Tx" "d" Number-of-bytes Byte1, Byte2, ..., Byte8 [...]
         * For CAN-FD: Timestamp CANFD <channel> <dir> <id> <brs> <esi> <dlc> <data_len> <data> [ ... ] 
         * \endverbatim
         * 
         * @param[in] rssSample Reference to the measurement value sample.
         */
        void ProcessSample(const std::string& rssSample);

        /**
         * @brief Playback thread function. If the current position is BOF, the playback starts from time = 0.0000, otherwise the
         * playback starts from the current position.
         */
        void PlaybackThreadFunc(std::function<void(const SCanMessage&)> fnCallback, std::atomic<uint32_t>& loopCount, bool bRepeat);

        std::list<SCanMessage>              m_lstMessages;                  ///< Vector with messages
        std::list<SCanMessage>::iterator    m_itCurrent;                    ///< Current iterator position
        std::thread                         m_threadPlayback;               ///< Playback thread.
        bool                                m_bPlaybackThread = false;      ///< Set when running playback thread
        bool                                m_bPlayback = false;            ///< Set when running playback
        std::atomic<uint32_t>               m_uiLoopCount{ 0 };             ///< Counter how often the data set was sent
    };
}

#endif // !defined ASC_FILE_READER_H
