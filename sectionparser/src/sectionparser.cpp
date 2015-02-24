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


#include "sectionparser.h"

// C system includes
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

// C++ system includes

// Other libraries' includes

// Project's includes
#include <oswrap.h>

#define DVB_TABLE_DEBUG
#ifdef DVB_TABLE_DEBUG
#include "NitTable.h"
#include "SdtTable.h"
#include "EitTable.h"
#include "TotTable.h"
#include "BatTable.h"
#include "NetworkNameDescriptor.h"
#include "CableDeliverySystemDescriptor.h"
#include "ServiceDescriptor.h"
#include "ShortEventDescriptor.h"
#include "LocalTimeOffsetDescriptor.h"
#include "ServiceListDescriptor.h"
#include "ExtendedEventDescriptor.h"
#include "LogicalChannelDescriptor.h"
#include "ComponentDescriptor.h"
#include "MultilingualServiceNameDescriptor.h"
#include "MultilingualComponentDescriptor.h"
#include "MultilingualNetworkNameDescriptor.h"
#include "ParentalRatingDescriptor.h"
#include "ContentDescriptor.h"
#else
#include "SiTable.h"
#endif

using std::map;
using std::pair;

/**
 * Constructor
 *
 * @param eventMgr event manager
 */
SectionParser::SectionParser(void* context, SendEventCallback callback)
  : m_context(context),
    m_sendEventCb(callback)
{
}

/**
 * Destructor
 */
SectionParser::~SectionParser()
{
}

/**
 * Check if tables of certain type are supported or not.
 * Note that some optional DVB tables might not be supported.
 *
 * @param tableId table identifier
 * @return true if supported, false otherwise
 */
bool SectionParser::isTableSupported(uint8_t id)
{
    TableId tableId = static_cast<TableId>(id);

    // NIT, SDT, BAT, EIT, TDT, TOT
    if( ((tableId >= TableId::NIT) && (tableId <= TableId::SDT)) ||  
         (tableId == TableId::SDT_OTHER) || (tableId == TableId::BAT)  ||
        ((tableId >= TableId::EIT_PF) && (tableId <= TableId::EIT_SCHED_OTHER_END)) ||
         (tableId == TableId::TDT) || (tableId == TableId::TOT) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * Parse SI Section
 *
 * @param data section data
 * @param size data size
 */
void SectionParser::parse(uint8_t *data, uint32_t size)
{
    OS_LOG(DVB_TRACE3, "<%s> data: %p, size: 0x%x\n", __FUNCTION__, data, size);

    // Sanity check
    if(!data || size == 0)
    {
        OS_LOG(DVB_ERROR, "<%s> Invalid parameter passed(%p, 0x%x)\n", __FUNCTION__, data, size);
        return;
    }

    // Check if we support this table or not
    if(!isTableSupported(data[0]))
    {
        OS_LOG(DVB_WARN, "<%s> Table id 0x%x is not supported\n", __FUNCTION__, data[0]);
        return;
    }

    Section* section = new Section(data, size);

    OS_LOG(DVB_DEBUG, "<%s> Handling id = 0x%x, syntax = %d, extId = 0x%x, ver = %d %d/%d\n", __FUNCTION__,
            section->tableId, section->syntax, section->extensionId, section->version, section->number, section->lastNumber);

    // Find the list in the section map
    pair<uint8_t, uint16_t> key(section->tableId, section->extensionId);
    SectionList& secList = m_sectionMap[key];

    // Adding the section to the list
    if(secList.add(*section))
    {
        OS_LOG(DVB_TRACE3, "<%s> Add() returned true\n", __FUNCTION__);

        // Let's check if the table became complete
        if(secList.isComplete())
        {
            OS_LOG(DVB_DEBUG, "<%s> table is complete\n", __FUNCTION__);
            OS_LOG(DVB_DEBUG, "<%s> SectionList: %s\n", __FUNCTION__, secList.toString().c_str());

            // Time to build the table
            SiTable* tbl = secList.buildTable();
            if(tbl)
            {
// This block parses certain tables and logs the results. Used for debugging purposes only.
#ifdef DVB_TABLE_DEBUG
                if(tbl->getTableId() == TableId::NIT)
                {
                    OS_LOG(DVB_DEBUG, "<%s> NIT table received, id: 0x%x, extId: 0x%x\n", __FUNCTION__, tbl->getTableId(), tbl->getExtensionId());
                    NitTable* nit = static_cast<NitTable*>(tbl);
                    const std::vector<MpegDescriptor>& descriptors = nit->getNetworkDescriptors();
                    const MpegDescriptor* desc = MpegDescriptor::find(descriptors, DescriptorTag::NETWORK_NAME);
                    if(desc)
                    {
                        NetworkNameDescriptor netName(*desc);
                        OS_LOG(DVB_DEBUG, "<%s> NIT table, network name: %s\n", __FUNCTION__, netName.getName().c_str());
                    }
                    else
                    {
                        OS_LOG(DVB_DEBUG, "<%s> NIT table: network name descriptor not found\n", __FUNCTION__);
                    }

                    const std::vector<TransportStream>& tsList = nit->getTransportStreams();

                    // Let's iterate through the list of transport streams in order to extract certain ts descriptors
                    for(auto it = tsList.begin(), end = tsList.end(); it != end; ++it)
                    {
                        const std::vector<MpegDescriptor>& tsDescriptors = it->getTsDescriptors();
                        const MpegDescriptor* desc = MpegDescriptor::find(tsDescriptors, DescriptorTag::CABLE_DELIVERY);
                        if(desc)
                        {
                            CableDeliverySystemDescriptor cable(*desc);
                            OS_LOG(DVB_DEBUG, "<%s> NIT table: freq = 0x%x(%d), mod = 0x%x, symbol_rate = 0x%x(%d)\n",
                                    __FUNCTION__, cable.getFrequencyBcd(), cable.getFrequency(), cable.getModulation(), cable.getSymbolRateBcd(), cable.getSymbolRate());
                        }
                        else
                        {
                            OS_LOG(DVB_DEBUG, "<%s> NIT table: cable delivery descriptor not found\n", __FUNCTION__);
                        }
                    }

                    std::vector<MpegDescriptor> netList = MpegDescriptor::findAll(descriptors, DescriptorTag::MULTILINGUAL_NETWORK_NAME);

                    // Let's dump all the multilingual network name descriptors to the log
                    for(auto ext_it = netList.begin(), ext_end = netList.end(); ext_it != ext_end; ++ext_it)
                    {
                        MultilingualNetworkNameDescriptor netDesc(*ext_it);
                        for(uint8_t i = 0; i < netDesc.getCount(); i++)
                        {
                            OS_LOG(DVB_DEBUG, "<%s> NIT table: multilingual network name[%d] (%s): %s\n",
                                    __FUNCTION__, i, netDesc.getLanguageCode(i).c_str(), netDesc.getNetworkName(i).c_str());
                        }
                    }
                }
                else if(tbl->getTableId() == TableId::SDT || tbl->getTableId() == TableId::SDT_OTHER)
                {
                    OS_LOG(DVB_DEBUG, "<%s> SDT table received, id: 0x%x, extId: 0x%x\n", __FUNCTION__, tbl->getTableId(), tbl->getExtensionId());
                    SdtTable* sdt = static_cast<SdtTable*>(tbl);

                    const std::vector<DvbService>& serviceList = sdt->getServices();

                    // Let's iterate through the list of services in order to extract certain service descriptors
                    for(auto it = serviceList.begin(), end = serviceList.end(); it != end; ++it)
                    {
                        const std::vector<MpegDescriptor>& serviceDescriptors = it->getServiceDescriptors();
                        const MpegDescriptor* desc = MpegDescriptor::find(serviceDescriptors, DescriptorTag::SERVICE);
                        if(desc)
                        {
                            ServiceDescriptor servDesc(*desc);
                            OS_LOG(DVB_DEBUG, "<%s> SDT table: type = 0x%x, provider = %s, name = %s\n",
                                    __FUNCTION__, servDesc.getServiceType(), servDesc.getServiceProviderName().c_str(), servDesc.getServiceName().c_str());
                        }
                        else
                        {
                            OS_LOG(DVB_DEBUG, "<%s> SDT table: service descriptor not found\n", __FUNCTION__);
                        }

                        std::vector<MpegDescriptor> nameList = MpegDescriptor::findAll(serviceDescriptors, DescriptorTag::MULTILINGUAL_SERVICE_NAME);
                        for(auto ext_it = nameList.begin(), ext_end = nameList.end(); ext_it != ext_end; ++ext_it)
                        {
                            MultilingualServiceNameDescriptor nameDesc(*ext_it);

                            // Let's dump the service information in all available languages
                            for(uint8_t i = 0; i < nameDesc.getCount(); i++)
                            {
                                OS_LOG(DVB_DEBUG, "<%s> SDT table: multilingual service name[%d] (%s): provider = %s, name = %s\n",
                                        __FUNCTION__, i, nameDesc.getLanguageCode(i).c_str(), nameDesc.getServiceProviderName(i).c_str(),  nameDesc.getServiceName(i).c_str());
                            }
                        }
                    }
                }
                else if((tbl->getTableId() >= TableId::EIT_PF) && (tbl->getTableId() <= TableId::EIT_SCHED_OTHER_END))
                {
                    OS_LOG(DVB_DEBUG, "<%s> EIT table received, id: 0x%x, extId: 0x%x\n", __FUNCTION__, tbl->getTableId(), tbl->getExtensionId());
                    EitTable* eit = static_cast<EitTable*>(tbl);

                    const std::vector<DvbEvent>& eventList = eit->getEvents();
                    for(auto it = eventList.begin(), end = eventList.end(); it != end; ++it)
                    {
                        OS_LOG(DVB_DEBUG, "<%s> EIT table: event_id = 0x%x, duration = 0x%x(%d), status = %d\n",
                                __FUNCTION__, it->getEventId(), it->getDurationBcd(), it->getDuration(), it->getRunningStatus());
                        const std::vector<MpegDescriptor>& eventDescriptors = it->getEventDescriptors();
                        std::vector<MpegDescriptor> shortList = MpegDescriptor::findAll(eventDescriptors, DescriptorTag::SHORT_EVENT);

                        // Let's dump all short event descriptors to the log
                        for(auto ext_it = shortList.begin(), ext_end = shortList.end(); ext_it != ext_end; ++ext_it)
                        {
                            ShortEventDescriptor eventDesc(*ext_it);
                            OS_LOG(DVB_DEBUG, "<%s> EIT table: lang_code = %s, name = %s, text = %s\n",
                                    __FUNCTION__, eventDesc.getLanguageCode().c_str(), eventDesc.getEventName().c_str(), eventDesc.getText().c_str());
                        }

                        std::vector<MpegDescriptor> extList = MpegDescriptor::findAll(eventDescriptors, DescriptorTag::EXTENDED_EVENT);

                        // Let's dump all extended event descriptors to the log
                        for(auto ext_it = extList.begin(), ext_end = extList.end(); ext_it != ext_end; ++ext_it)
                        {
                            ExtendedEventDescriptor eventDesc(*ext_it);
                            OS_LOG(DVB_DEBUG, "<%s> EIT table: %d/%d, lang_code = %s, text = %s\n",
                                    __FUNCTION__, eventDesc.getNumber(), eventDesc.getLastNumber(), eventDesc.getLanguageCode().c_str(), eventDesc.getText().c_str());
                            for(uint8_t i = 0; i < eventDesc.getNumberOfItems(); i++)
                            {
                                OS_LOG(DVB_DEBUG, "<%s> EIT table: item[%d] %s: %s\n", __FUNCTION__, i, eventDesc.getItemDescription(i).c_str(), eventDesc.getItem(i).c_str());
                            }
                        }

                        const MpegDescriptor* desc = MpegDescriptor::find(eventDescriptors, DescriptorTag::PARENTAL_RATING);
                        if(desc)
                        {
                            ParentalRatingDescriptor prDesc(*desc);

                            // Let's dump the rating value for all available countries
                            for(uint8_t i = 0; i < prDesc.getCount(); i++)
                            {
                                OS_LOG(DVB_DEBUG, "<%s> EIT table: PR[%d] country_code = %s, rating = 0x%x\n",
                                        __FUNCTION__, i, prDesc.getCountryCode(i).c_str(), prDesc.getRating(i));
                            }
                        }
                        else
                        {
                            OS_LOG(DVB_DEBUG, "<%s> EIT table: parental rating descriptor not found\n", __FUNCTION__);
                        }

                        desc = MpegDescriptor::find(eventDescriptors, DescriptorTag::CONTENT_DESCRIPTOR);
                        if(desc)
                        {
                            ContentDescriptor contentDesc(*desc);

                            // Let's dump all the available content identifiers to the log
                            for(uint8_t i = 0; i < contentDesc.getCount(); i++)
                            {
                                OS_LOG(DVB_DEBUG, "<%s> EIT table: [%d] nibble_lvl_1 = 0x%x, nibble_lvl_2 = 0x%x, user_byte = 0x%x\n",
                                        __FUNCTION__, i, contentDesc.getNibbleLvl1(i), contentDesc.getNibbleLvl2(i), contentDesc.getUserByte(i));
                            }
                        }
                        else
                        {
                            OS_LOG(DVB_DEBUG, "<%s> EIT table: content descriptor not found\n", __FUNCTION__);
                        }

                        std::vector<MpegDescriptor> compList = MpegDescriptor::findAll(eventDescriptors, DescriptorTag::MULTILINGUAL_COMPONENT);
                        for(auto ext_it = compList.begin(), ext_end = compList.end(); ext_it != ext_end; ++ext_it)
                        {
                            MultilingualComponentDescriptor compDesc(*ext_it);

                            // Let's dump the component information in all available languages
                            for(uint8_t i = 0; i < compDesc.getCount(); i++)
                            {
                                OS_LOG(DVB_DEBUG, "<%s> EIT table: multilingual component descriptor[%d] tag = 0x%x, (%s): text = %s\n",
                                        __FUNCTION__, i, compDesc.getComponentTag(), compDesc.getLanguageCode(i).c_str(), compDesc.getText(i).c_str());
                            }
                        }
                    }
                }
                else if((tbl->getTableId() == TableId::TDT) || (tbl->getTableId() == TableId::TOT))
                {
                    TotTable* tot = static_cast<TotTable*>(tbl);
                    OS_LOG(DVB_DEBUG, "<%s> TDT/TOT table received, id: 0x%x, UTC: %"PRId64"\n", __FUNCTION__, tot->getTableId(), tot->getUtcTimeBcd());
                    if(tot->getTableId() == TableId::TOT)
                    {
                        std::vector<MpegDescriptor>& timeDescriptors = tot->getDescriptors();
                        const MpegDescriptor* desc = MpegDescriptor::find(timeDescriptors, DescriptorTag::LOCAL_TIME_OFFSET);
                        if(desc)
                        {
                            LocalTimeOffsetDescriptor offsetDesc(*desc);
                            for(uint8_t i = 0; i < offsetDesc.getCount(); i++)
                            {
                                OS_LOG(DVB_DEBUG, "<%s> TOT table: time_offset[%d] code = %s, reg_id = 0x%x, pol = %d, offset = 0x%x, ToC = %"PRId64", next offset = 0x%x\n",
                                        __FUNCTION__, i, offsetDesc.getCountryCode(i).c_str(), offsetDesc.getCountryRegionId(i), offsetDesc.getPolarity(i),
                                        offsetDesc.getLocalTimeOffset(i), offsetDesc.getTimeOfChange(i), offsetDesc.getNextTimeOffset(i));
                            }
                        }
                        else
                        {
                            OS_LOG(DVB_DEBUG, "<%s> TOT table: local time offset descriptor not found\n", __FUNCTION__);
                        }
                    }
                }
                else if(tbl->getTableId() == TableId::BAT)
                {
                    BatTable* bat = static_cast<BatTable*>(tbl);
                    OS_LOG(DVB_DEBUG, "<%s> BAT table received, id: 0x%x, bouquet_id: 0x%x\n", __FUNCTION__, bat->getTableId(), bat->getBouquetId());

                    const std::vector<MpegDescriptor>& bouquetDesc = bat->getBouquetDescriptors();
                    const MpegDescriptor* d = MpegDescriptor::find(bouquetDesc, DescriptorTag::LOGICAL_CHANNEL);
                    if(d)
                    {
                        LogicalChannelDescriptor lcnDesc(*d);
                        for(uint8_t i = 0; i < lcnDesc.getCount(); i++)
                        {
                            OS_LOG(DVB_DEBUG, "<%s> BAT table: [%d] service_id = 0x%x, visible = %d, lcn = %d\n",
                                    __FUNCTION__, i, lcnDesc.getServiceId(i), lcnDesc.isVisible(i), lcnDesc.getLcn(i));
                        }
                    }
                    else
                    {
                        OS_LOG(DVB_DEBUG, "<%s> BAT table: logical channel descriptor not found\n", __FUNCTION__);
                    }


                    const std::vector<TransportStream>& tsList = bat->getTransportStreams();

                    // Let's iterate through the list of transport streams in order to extract certain ts descriptors
                    for(auto it = tsList.begin(), end = tsList.end(); it != end; ++it)
                    {
                        const std::vector<MpegDescriptor>& tsDescriptors = it->getTsDescriptors();
                        const MpegDescriptor* desc = MpegDescriptor::find(tsDescriptors, DescriptorTag::SERVICE_LIST);
                        if(desc)
                        {
                            ServiceListDescriptor serviceList(*desc);

                            // Let's dump all the information about all services to the log
                            for(uint8_t i = 0; i < serviceList.getCount(); i++)
                            {
                                OS_LOG(DVB_DEBUG, "<%s> BAT table: service_id = 0x%x, service_type = 0x%x\n",
                                        __FUNCTION__, serviceList.getServiceId(i), serviceList.getServiceType(i));
                            }
                        }
                        else
                        {
                            OS_LOG(DVB_DEBUG, "<%s> BAT table: service list descriptor not found\n", __FUNCTION__);
                        }
                    }

                }
#endif // DVB_TABLE_DEBUG

                // Call platform ipc mechinism (callback)
                if(m_context && m_sendEventCb)
                {
                    // Let's publish the event
                    m_sendEventCb(m_context, (uint32_t)tbl->getTableId(), tbl, 0);
                }
            }
        }
        else // isComplete()
        {
            OS_LOG(DVB_DEBUG, "<%s> table is not complete yet\n", __FUNCTION__);
        }
    }
    else // Add()
    {
        OS_LOG(DVB_DEBUG, "<%s> Add() returned false\n", __FUNCTION__);
    }

    // clean up
    delete section;
}

