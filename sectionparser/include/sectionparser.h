//
// DVB_SI for Reference Design Kit (RDK)
//
// Copyright (C) 2015  Dmitry Barablin & Stan Partridge (stan.partridge@arris.com)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


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
