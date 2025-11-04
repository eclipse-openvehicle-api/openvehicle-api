#ifndef ASC_FILE_WRITER_H
#define ASC_FILE_WRITER_H

#include "ascreader.h"
#include <list>
#include <chrono>

namespace asc
{
    /**
    * @brief This class allows writing the Vector ASC file format.
    * @attention This class assumes no concurrency between writing the file and adding samples. No thread synchronization is
    * implemented.
    */
    class CAscWriter
    {
    public:
        /**
         * @brief Constructor
         */
        CAscWriter();

        /**
         * @brief Start the timer for automatic timestamp creation.
         * @remarks Resets the start time only if there are no samples are available.
         */
        void StartTimer();

        /**
         * @brief Add a sample.
         * @attention The sample will not be added if the timestamp is smaller than the last sample.
         * @param[in] rsSample Reference to the CAN sample structure. If the timestamp is empty, the timestamp is created automatically
         * using the integrated timer.
         */
        void AddSample(const SCanMessage& rsSample);

        /**
         * @brief Returns whether at least one sample is stored.
         * @return Returns 'true' when samples are available; 'false' when not.
        */
        bool HasSamples() const;

        /**
         * @brief Clear the content.
         */
        void Clear();

        /**
         * @brief Write to a file (the file gets overwritten if existing).
         * @param[in] rpathFile Reference to the file path.
         * @return Returns 'true' on success or 'false' when not.
         */
        bool Write(const std::filesystem::path& rpathFile);

    private:
        std::list<SCanMessage>                              m_lstMessages;        ///< Queue with messages
        std::chrono::high_resolution_clock::time_point      m_timepointStart;     ///< Starting timepoint for automatic timestamp
                                                                                  ///< generation.
        std::chrono::system_clock::time_point               m_timepointSystem;    ///< Starting timepoint for time/date generation.
    };
}

#endif // !defined ASC_FILE_WRITER_H