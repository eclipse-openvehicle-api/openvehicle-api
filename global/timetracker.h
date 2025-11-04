#ifndef TIME_TRACKER_H
#define TIME_TRACKER_H

#include "exec_dir_helper.h"
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <list>
#include <map>
#include <queue>
#include <fstream>

/**
 * @brief Track function to trigger one time entry.
 * @param tag The tag name to use to identify this entry.
 * @details Use the "track_time" macro within a function to activate. The tag is a name used to identify the code position that
 * should be tracked. For each track_time call, a time entry relative to the start of the application will be written (resolution
 * is micro-seconds). The tracking file will be written on shutdown of the application and is called the same as the application
 * with "_tracking.csv" attached to it.
 * Example:
 * @code
 * void MyFunc()
 * {
 *      track_time(enter);
 *
 *      // ...
 *
 *      track_timer(leave);
 * }
 * @endcode
 */
#define track_time(tag) GetTimeTracker().Trigger(__FUNCTION__, #tag)

/**
 * @brief Time tracker class used internally by the tracker.
 */
class CTimeTracker
{
public:
    /**
     * @brief Default constructor.
     */
    CTimeTracker();

    /**
     * @brief Destructor.
     */
    ~CTimeTracker();

    /**
     * @brief Trigger function to store a trigger timepoint.
     * @attention Both function and tag name must be a static string in a data segment, so the strings are valid even if the
     * function that caused the trigger is not within scope any more.
     * @param[in] szFunction Pointer to the function name.
     * @param[in] szTag Tag to identify the trigger with.
     */
    void Trigger(const char* szFunction, const char* szTag);

private:
    /**
     * @brief Trigger value structure
     */
    struct STriggerEntry
    {
        /**
         * @brief Constructor of a trigger value.
         * @param[in] szFuncParam String to function name
         * @param[in] szTagParam String to tag name
         */
        STriggerEntry(const char* szFuncParam, const char* szTagParam) :
            szFunction(szFuncParam), szTag(szTagParam), tpTrigger(std::chrono::high_resolution_clock::now())
        {}

        const char*                                     szFunction;         ///< Function name (is a static string in data segment).
        const char*                                     szTag;              ///< Function tag (is a static string in data segment).
        std::chrono::high_resolution_clock::time_point  tpTrigger;          ///< Trigger timepoint.
    };

    std::time_t                                     m_timeStartTime;            ///< Start time of the tracker (system clock time).
    std::chrono::high_resolution_clock::time_point  m_tpStartTime;              ///< Start time of tracker (high resolution clock).
    mutable std::mutex                              m_mtxTriggers;              ///< Queue protection.
    std::queue<STriggerEntry>                       m_queuTriggers;             ///< Queue with trigger values.
};

inline CTimeTracker& GetTimeTracker()
{
    static CTimeTracker tracker_global;
    return tracker_global;
}

inline CTimeTracker::CTimeTracker() :
    m_timeStartTime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())),
    m_tpStartTime(std::chrono::high_resolution_clock::now())
{}

inline CTimeTracker::~CTimeTracker()
{
    if (m_queuTriggers.empty()) return;

    // Trigger list translated to the system time in microsecs.
    using TTriggerList = std::list<uint64_t>;
    struct STagInfo
    {
        STagInfo(const char* szFunctionParam, const char* szTagParam) :
            szFunction(szFunctionParam), szTag(szTagParam)
        {}
        const char*             szFunction;
        const char*             szTag;
        TTriggerList            lstTriggers;
        TTriggerList::iterator  itPosition;
    };
    using TOrderedTagList = std::list<STagInfo>;
    TOrderedTagList         lstOrderedTags;
    TOrderedTagList         lstDeferredTags;

    // Process the trigger queue
    while (!m_queuTriggers.empty())
    {
        // Get the entry
        STriggerEntry sEntry = std::move(m_queuTriggers.front());
        m_queuTriggers.pop();

        // Calculate the time since start time (use system clock as reference)
        uint64_t uiTimeMicrosecs = std::chrono::duration_cast<std::chrono::microseconds>(sEntry.tpTrigger - m_tpStartTime).count();
        //uiTimeMicrosecs += static_cast<uint64_t>(m_timeStartTime) * 1000000ull;

        // Determine the order
        STagInfo* psTagInfo = nullptr;
        if (lstOrderedTags.empty())
        {
            // First entry... add to the list
            lstOrderedTags.push_back(STagInfo(sEntry.szFunction, sEntry.szTag));
            psTagInfo = &lstOrderedTags.back();
        }
        else
        {
            // Check whether the tag represents the first entry of the order list. If so, add all the deferred list entries.
            if (sEntry.szTag == lstOrderedTags.front().szTag && !lstDeferredTags.empty())
            {
                // Add the deferred tags to the end of the list
                for (STagInfo& rsTag : lstDeferredTags)
                    lstOrderedTags.push_back(std::move(rsTag));
                lstDeferredTags.clear();
            }

            // Search for the tag to exist in the ordered tags list.
            auto itTag = std::find_if(lstOrderedTags.begin(), lstOrderedTags.end(), [&](const STagInfo& rsTag) { return rsTag.szFunction == sEntry.szFunction && rsTag.szTag == sEntry.szTag; });
            if (itTag == lstOrderedTags.end())
            {
                // Tag is not in the ordered tags list; add to deferred tags list of not already in there
                itTag = std::find_if(lstDeferredTags.begin(), lstDeferredTags.end(), [&](const STagInfo& rsTag) { return rsTag.szFunction == sEntry.szFunction && rsTag.szTag == sEntry.szTag; });
                if (itTag == lstDeferredTags.end())
                {
                    lstDeferredTags.push_back(STagInfo(sEntry.szFunction, sEntry.szTag));
                    psTagInfo = &lstDeferredTags.back();
                }
                else
                    psTagInfo = &(*itTag);
            }
            else
            {
                // Tag is in the ordered tags list. Are there any tags on the deferred list? Add them before.
                for (STagInfo& rsTag : lstDeferredTags)
                    lstOrderedTags.insert(itTag, std::move(rsTag));
                lstDeferredTags.clear();

                psTagInfo = &(*itTag);
            }
        }

        // Add the trigger value
        if (psTagInfo)
            psTagInfo->lstTriggers.push_back(uiTimeMicrosecs);
        else
            std::cout << "INTERNAL ERROR: tag info pointer is invalid!";
    }

    // Run through deferred tags lists to add them still at the end of the ordered tags lists
    for (STagInfo& rsTag : lstDeferredTags)
        lstOrderedTags.push_back(std::move(rsTag));

    // Create CSV file
    std::filesystem::path path = (GetExecDirectory() / GetExecFilename().replace_extension("")).generic_u8string() + "_tracker.csv";
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        std::cout << "Cannot write tracker information to '" << path.generic_u8string() << "'..." << std::endl;
        return;
    }

    std::cout << "Writing tracker data to '" << path.generic_u8string() << "'..." << std::endl;

    // Iterate through the tags and add the header lines + assign the initial iterator position
    std::stringstream sstreamFirstLine;
    std::stringstream sstreamSecondLine;
    std::stringstream sstreamData;
    for (STagInfo& rsTagInfo : lstOrderedTags)
    {
        // First line is the function name
        if (!sstreamFirstLine.str().empty())
            sstreamFirstLine << "; ";
        sstreamFirstLine << rsTagInfo.szFunction;

        // Second line is the tag
        if (!sstreamSecondLine.str().empty())
            sstreamSecondLine << "; ";
        sstreamSecondLine << rsTagInfo.szTag;

        // Assign iterator
        rsTagInfo.itPosition = rsTagInfo.lstTriggers.begin();
    }
    stream << sstreamFirstLine.str() << std::endl <<
        sstreamSecondLine.str() << std::endl;

    // Add the data by running though the position iterators so long until all iterators are pointing to end...
    uint64_t uiMax = 0;
    do
    {
        // Determine the maximum time that includes at the most one value of each tag - check the time of the trigger value
        // following the current.
        uiMax = static_cast<uint64_t>(-1);
        for (const STagInfo& rsTagInfo : lstOrderedTags)
        {
            if (rsTagInfo.itPosition != rsTagInfo.lstTriggers.end())
            {
                // Get the current timestamp to determine the minimal amount to include this timestamp
                uint64_t uiPotentialMax = *rsTagInfo.itPosition + 1;
                auto itNextPosition = rsTagInfo.itPosition;
                itNextPosition++;

                // If the next timestamp is larger than the current timestamp, take the next timestamp
                if (itNextPosition != rsTagInfo.lstTriggers.end() && uiPotentialMax < *itNextPosition)
                    uiPotentialMax = *itNextPosition;

                // If the max timestamp should not exceed the next coming timestamp.
                if (uiMax > uiPotentialMax)
                    uiMax = uiPotentialMax;
            }
        }

        // Run through the values and stream those values that are smaller than the next position.
        bool bFirst = true;
        for (STagInfo& rsTagInfo : lstOrderedTags)
        {
            if (!bFirst)
                stream << "; ";
            bFirst = false;
            if (rsTagInfo.itPosition != rsTagInfo.lstTriggers.end() && *rsTagInfo.itPosition < uiMax)
            {
                stream << *rsTagInfo.itPosition;
                rsTagInfo.itPosition++;
            }
        }
        stream << std::endl;
    } while (uiMax != static_cast<uint64_t>(-1));

    // Finalize the stream
    stream.close();
}

inline void CTimeTracker::Trigger(const char* szFunction, const char* szTag)
{
    std::unique_lock<std::mutex> lock(m_mtxTriggers);
    m_queuTriggers.push(STriggerEntry(szFunction, szTag));
}

#endif // !defined TIME_TRACKER_H