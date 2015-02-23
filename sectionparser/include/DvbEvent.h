// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef DVBEVENT_H_
#define DVBEVENT_H_

// C system includes
#include <stdint.h>

// C++ system includes
#include <vector>

// Other libraries' includes

// Project's includes
#include "DvbUtils.h"

/**
 * EIT event class
 */
class DvbEvent
{
public:
    /**
     * Constructor
     *
     * @param id event identifier
     * @param startTime the event start time
     * @param dur duration of the event
     * @param runningStatus running status of the event
     * @param freeCa false if all the component streams of the event are not scrambled, true otherwise
     */
    DvbEvent(uint16_t id, uint64_t startTime, uint32_t dur, uint8_t runningStatus, bool freeCa)
        : m_eventId(id),
          m_startTime(startTime),
          m_duration(dur),
          m_runningStatus(runningStatus),
          m_freeCa(freeCa)
    {
        m_nvod = ((m_startTime & 0xFFFFF000) == 0xFFFFF000 && m_runningStatus == 0x0);
    }

    /**
     * Desctructor
     */
    virtual ~DvbEvent()
    {
    }

    /**
     * Get event descriptors
     *
     * @return vector of event descriptors
     */
    const std::vector<MpegDescriptor>& getEventDescriptors() const
    {
        return m_eventDescriptors;
    }

    /**
     * Add an event descriptor
     *
     * @param descriptor the event descriptor
     */
    void addDescriptor(const MpegDescriptor& descriptor)
    {
        m_eventDescriptors.push_back(descriptor);
    }

    /**
     * Add event descriptors
     *
     * @param descriptors event descriptors
     */
    void addDescriptors(const std::vector<MpegDescriptor>& descriptors)
    {
        m_eventDescriptors.insert(m_eventDescriptors.end(), descriptors.begin(), descriptors.end());
    }

    /**
     * Remove event descriptors
     */
    void removeDescriptors()
    {
        m_eventDescriptors.clear();
    }

    /**
     * Get the event duration as a binary coded decimal
     *
     * @return duration bcd
     */
    uint32_t getDurationBcd() const
    {
        return m_duration;
    }

    /**
     * Get the event duration
     *
     * @return duration
     */
    uint32_t getDuration() const
    {
        return BcdByteToDec((m_duration >> 16) & 0xFF)*3600 + BcdByteToDec((m_duration >> 8) & 0xFF)*60 + BcdByteToDec(m_duration & 0xFF);
    }

    /**
     * Get event identifier
     *
     * @return event identifier
     */
    uint16_t getEventId() const
    {
        return m_eventId;
    }

    /**
     * Check if event is scrambled
     *
     * @return true if one or more component streams is controlled by a CA system
     */
    bool isScrambled() const
    {
        return m_freeCa;
    }

    /**
     * Check if this is a Near Video on Demand (NVOD) reference event
     *
     * @return true if an NVOD reference event, false otherwise
     */
    bool isNvod() const
    {
        return m_nvod;
    }

    /**
     * Get the event running status
     *
     * @return running status
     *
     * @todo define possible running_status values (enum)
     * 0 undefined
     * 1 not running
     * 2 starts in a few seconds (e.g. for video recording)
     * 3 pausing
     * 4 running
     * 5 service off-air
     * 6 to 7 reserved for future use
     */
    uint8_t getRunningStatus() const
    {
        return m_runningStatus;
    }

    /**
     * Get the start time of the event
     *
     * @return time_t the event start time
     */
    time_t getStartTime() const
    {
        return MjdToDate(m_startTime);
    }

    /**
     * Get the start time of the event as a BCD
     *
     * @return start time (BCD)
     */
    uint64_t getStartTimeBcd() const
    {
        return m_startTime;
    }

private:
    /**
     * Event identifier
     */
    uint16_t m_eventId;

    /**
     * Start time
     */
    uint64_t m_startTime;

    /**
     * Duration
     */
    uint32_t m_duration;

    /**
     * Running status
     */
    uint8_t  m_runningStatus;

    /**
     * Free CA mode indicator
     */
    bool m_freeCa;

    /**
     * Near Video on Demand (NVOD) indicator
     */
    bool m_nvod;

    /**
     * Event descriptors
     */
    std::vector<MpegDescriptor> m_eventDescriptors;
};

#endif /* DVBEVENT_H_ */
