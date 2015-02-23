// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef __SECTIONPARSER_H__
#define __SECTIONPARSER_H__

// C system includes
#include <stdint.h>

// C++ system includes
#include <map>
#include <list>
#include <utility>
#include <memory>

// Other libraries' includes

// Project's includes
#include "sectionlist.h"

typedef std::map<std::pair<uint8_t, uint16_t>, SectionList> SectionMap_t;
typedef void (*SendEventCallback) (void*, uint32_t, void*, size_t);

/**
 * SectionParser
 *
 * Takes sections as an input and produces parsed tables as an output
 */
class SectionParser
{
public:
    /**
     * Constructor
     *
     * @param eventMgr event manager
     */
    SectionParser(void* context, SendEventCallback callback);

    /**
     * Destructor
     */
    virtual ~SectionParser();

    /**
     * Parse SI Section
     *
     * @param data section data
     * @param size data size
     */
    void parse(uint8_t* data, uint32_t size);

private:
    /**
     * Check if tables of certain type are supported or not.
     * Note that some optional DVB tables might not be supported.
     *
     * @param tableId table identifier
     * @return true if supported, false otherwise
     */
    bool isTableSupported(uint8_t tableId);

    /**
     * Copy constructor
     */
    SectionParser(const SectionParser& other);

    /**
     * Assignment operator
     */
    SectionParser& operator=(const SectionParser&);

    /**
     * SendEvent's calling context 
     */
    void* m_context;

    /**
     *  SendEvent function pointer
     */
    SendEventCallback m_sendEventCb;

    /**
     * Map of section lists
     */
    SectionMap_t m_sectionMap;
};

#endif
