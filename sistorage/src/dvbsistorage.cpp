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


// C system includes
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>


// C++ system includes
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>

// Other libraries' includes

// Project's includes

#include "oswrap.h"
#include "dvbsistorage.h"
#include "dvbdb.h"
#include "NitTable.h"
#include "SdtTable.h"
#include "EitTable.h"
#include "TotTable.h"
#include "BatTable.h"
#include "CableDeliverySystemDescriptor.h"
#include "ShortEventDescriptor.h"
#include "ServiceDescriptor.h"
#include "MultilingualNetworkNameDescriptor.h"
#include "NetworkNameDescriptor.h"
#include "LogicalChannelDescriptor.h"
#include "ParentalRatingDescriptor.h"
#include "ContentDescriptor.h"


using std::map;
using std::pair;
using std::vector;
using std::tuple;
using std::shared_ptr;
using std::string;

/** 
 * Default Constructor
 */
DvbSiStorage::DvbSiStorage()
  : m_preferredNetworkId(0),
    m_homeFrequency(0),
    m_homeModulation(DVB_MODULATION_UNKNOWN),
    m_homeSymbolRate(0),
    m_barkerFrequency(0),
    m_barkerModulation(DVB_MODULATION_UNKNOWN),
    m_barkerSymbolRate(0),
    m_isFastScanSmart(false),
    m_bkgdScanInterval(21600),
    m_barkerEitTimeout(EIT_PAST_8_DAY_SCHED_TIMEOUT)
{
    m_scanStatus.state  = DvbScanState::SCAN_STOPPED;

    DvbDb::FileStatus status = m_db.open(string(OS_GETENV("FEATURE.DVB.DB_FILENAME")));
    OS_LOG(DVB_INFO, "<%s> - DB status = 0x%x\n", __FUNCTION__, status);

    if(status != DvbDb::OPENED && status != DvbDb::CREATED)
    {
        OS_LOG(DVB_ERROR, "<%s> - Unable to open db\n", __FUNCTION__);
        // TODO: Consider throwing an exception here. We can go on without the DB. 
    }

    bool changed = loadSettings();
    if(changed)
    {
        OS_LOG(DVB_INFO, "<%s> - DVB scan settings have changed\n", __FUNCTION__);

        // Need to flush the db if it's populated and force the scan
        if(status == DvbDb::OPENED)
        {
            OS_LOG(DVB_INFO, "<%s> - Flushing db\n", __FUNCTION__);
            m_db.dropTables();
            m_db.createTables();

            // Forcing a fast scan
            status = DvbDb::CREATED;
        }
    }
    else
    {
        OS_LOG(DVB_DEBUG, "<%s> - DVB scan settings have not changed\n", __FUNCTION__);
    }

    // Let's scan only if we have home TS parameters set
    if(m_homeFrequency && m_homeModulation && m_homeSymbolRate)
    {
        // DvbDb::open() calls sanityCheck() that in turn calls contentCheck(),
        // so we don't have to make the sanity and content checks here.
        // Let's skip the fast scan only if status == OPENED
        if(status == DvbDb::OPENED)
        {
            // Kick off a periodic background scan only
            startScan(false);
        }
        else
        {
            // Let's do both fast and bkgd scans
            startScan(true);
        }
    }
    else
    {
        OS_LOG(DVB_ERROR, "<%s> - home ts parameters not set correctly. not scanning.\n", __FUNCTION__);
    }
}

/**
 * Load environmental runtime settings
 *
 * @return bool true no change in settings; false settings have changed
 */
bool DvbSiStorage::loadSettings()
{
    // TODO: Refactor. This function does much more that it is supposed to.
    bool changed = false;

    // Let's read all the settings from the DB first
    size_t numDbSettings = m_db.loadSettings(); 

    // Preferred nentwork id
    const char* value = OS_GETENV("FEATURE.DVB.PREFERRED_NETWORK_ID");
    const char* dbValue = m_db.getSetting("FEATURE.DVB.PREFERRED_NETWORK_ID");
    if(value && dbValue && strcmp(value, dbValue) != 0)
    {
        changed = true;
        OS_LOG(DVB_DEBUG, "<%s> Preferred network id changed. Env: nid = %s Db: nid = %s\n", __FUNCTION__, value, dbValue);
    }

    if(value)
    {
        std::stringstream(string(value)) >> m_preferredNetworkId;
        OS_LOG(DVB_DEBUG, "<%s> preferred network id = 0x%x\n", __FUNCTION__, m_preferredNetworkId);
    }
    m_db.setSetting("FEATURE.DVB.PREFERRED_NETWORK_ID", value);

    // Bouquets
    value = OS_GETENV("FEATURE.DVB.BOUQUET_ID_LIST");
    if(value)
    {
        string str(value);
        std::stringstream ss(str);
        uint32_t id = 0;

        while(ss >> id)
        {
            OS_LOG(DVB_DEBUG, "<%s> adding bouquet_id 0x%x to the list\n", __FUNCTION__, id);
            m_homeBouquets.push_back(id);

            if(ss.peek() == ',')
            {
                ss.ignore();
            }
        }
    }
    m_db.setSetting("FEATURE.DVB.BOUQUET_ID_LIST", value);

    // Home TS frequency
    value = OS_GETENV("FEATURE.DVB.HOME_TS_FREQUENCY");
    dbValue = m_db.getSetting("FEATURE.DVB.HOME_TS_FREQUENCY");
    if(value && dbValue && strcmp(value, dbValue) != 0)
    {
        changed = true;
        OS_LOG(DVB_DEBUG, "<%s> Home TS frequency changed. Env: freq = %s Db: freq = %s\n", __FUNCTION__, value, dbValue);
    }

    if(value)
    {
        std::stringstream(string(value)) >> m_homeFrequency;
    }
    m_db.setSetting("FEATURE.DVB.HOME_TS_FREQUENCY", value);

    // Home TS modulation
    value = OS_GETENV("FEATURE.DVB.HOME_TS_MODULATION");
    if(value)
    {
        uint32_t mod = 0;
        std::stringstream(string(value)) >> mod;
        m_homeModulation = static_cast<DvbModulationMode>(mod);
    }
    m_db.setSetting("FEATURE.DVB.HOME_TS_MODULATION", value);

    // Home TS symbol rate
    value = OS_GETENV("FEATURE.DVB.HOME_TS_SYMBOL_RATE");
    if(value)
    {
        std::stringstream(string(value)) >> m_homeSymbolRate;
    }
    m_db.setSetting("FEATURE.DVB.HOME_TS_SYMBOL_RATE", value);

    OS_LOG(DVB_DEBUG, "<%s> Home TS: freq = %d, mod = %d, sym_rate = %d\n",
            __FUNCTION__, m_homeFrequency, m_homeModulation, m_homeSymbolRate);

    // Barker TS frequency
    value = OS_GETENV("FEATURE.DVB.BARKER_TS_FREQUENCY");
    if(value)
    {
        std::stringstream(string(value)) >> m_barkerFrequency;
    }
    m_db.setSetting("FEATURE.DVB.BARKER_TS_FREQUENCY", value);

    // Barker TS modulation
    value = OS_GETENV("FEATURE.DVB.BARKER_TS_MODULATION");
    if(value)
    {
        uint32_t mod = 0;
        std::stringstream(string(value)) >> mod;
        m_barkerModulation = static_cast<DvbModulationMode>(mod);
    }
    m_db.setSetting("FEATURE.DVB.BARKER_TS_MODULATION", value);

    // Berker TS symbol rate
    value = OS_GETENV("FEATURE.DVB.BARKER_TS_SYMBOL_RATE");
    if(value)
    {
        std::stringstream(string(value)) >> m_barkerSymbolRate;
    }
    m_db.setSetting("FEATURE.DVB.BARKER_TS_SYMBOL_RATE", value);

    // Barker EIT timeout
    value = OS_GETENV("FEATURE.DVB.BARKER_EIT_TIMEOUT");
    if(value)
    {
        std::stringstream(string(value)) >> m_barkerEitTimeout;
    }
    m_db.setSetting("FEATURE.DVB.BARKER_EIT_TIMEOUT", value);

    OS_LOG(DVB_DEBUG, "<%s> Barker TS: freq = %d, mod = %d, sym_rate = %d, eit_timeout = %d\n",
            __FUNCTION__, m_barkerFrequency, m_barkerModulation, m_barkerSymbolRate, m_barkerEitTimeout);

    // Smart scan flag
    value = OS_GETENV("FEATURE.DVB.FAST_SCAN_SMART");
    if(value && (strcmp(value, "TRUE") == 0))
    {
        m_isFastScanSmart = true;
    }
    m_db.setSetting("FEATURE.DVB.FAST_SCAN_SMART", value);

    // Interval between background scans
    value = OS_GETENV("FEATURE.DVB.BACKGROUND_SCAN_INTERVAL");
    if(value)
    {
        std::stringstream(string(value)) >> m_bkgdScanInterval;
    }
    m_db.setSetting("FEATURE.DVB.BACKGROUND_SCAN_INTERVAL", value);

    OS_LOG(DVB_DEBUG, "<%s> Home TS: smart = %d, bkgd scan interval = %d sec\n",
            __FUNCTION__, m_isFastScanSmart, m_bkgdScanInterval);

    m_db.clearSettings();

    return changed;
}

// Constellation
typedef enum _DVBConstellation
{
    DVB_CONSTELLATION_UNDEFINED,
    DVB_CONSTELLATION_QAM16,
    DVB_CONSTELLATION_QAM32,
    DVB_CONSTELLATION_QAM64,
    DVB_CONSTELLATION_QAM128,
    DVB_CONSTELLATION_QAM256
} DVBConstellation;

/**
 * Return DvbModulationMode enumeration based on DVBConstellation enumeration
 *
 * @param in DVBConstellation enum
 */
static DvbModulationMode mapModulationMode(DVBConstellation in)
{
    DvbModulationMode out;

    switch(in)
    {
        case DVB_CONSTELLATION_QAM16:
            out = DVB_MODULATION_QAM16;
            break;
        case DVB_CONSTELLATION_QAM32:
            out = DVB_MODULATION_QAM32;
            break;
        case DVB_CONSTELLATION_QAM64:
            out = DVB_MODULATION_QAM64;
            break;
        case DVB_CONSTELLATION_QAM128:
            out = DVB_MODULATION_QAM128;
            break;
        case DVB_CONSTELLATION_QAM256:
            out = DVB_MODULATION_QAM256;
            break;
        case DVB_CONSTELLATION_UNDEFINED:
        default:
            out = DVB_MODULATION_UNKNOWN;
            break;
    }

    return out;
}

/**
 * Return a vector of Transport streams
 *
 * @param nId network id
 * @return vector of shared pointers of TransportStream structures
 */
vector<shared_ptr<DvbStorage::TransportStream_t>> DvbSiStorage::getTsListByNetIdCache(uint16_t nId)
{
    vector<shared_ptr<DvbStorage::TransportStream_t>> ret;

    std::lock_guard<std::mutex> lock(m_dataMutex);

    auto it = m_nitMap.begin();
    if(nId != 0)
    {
        it = m_nitMap.find(nId);
    }
    else if(m_preferredNetworkId != 0)
    {
        it = m_nitMap.find(m_preferredNetworkId);
    }

    if(it == m_nitMap.end())
    {
        return ret;
    }

    const vector<TransportStream>& tsList = it->second->getTransportStreams();
    for(auto it = tsList.begin(), end = tsList.end(); it != end; ++it)
    {
        const std::vector<MpegDescriptor>& tsDescriptors = it->getTsDescriptors();
        const MpegDescriptor* desc = MpegDescriptor::find(tsDescriptors, DescriptorTag::CABLE_DELIVERY);
        if(desc)
        {
            CableDeliverySystemDescriptor cable(*desc);
            OS_LOG(DVB_DEBUG, "<%s> NIT table: freq = 0x%x(%d), mod = 0x%x, symbol_rate = 0x%x(%d)\n",
                    __FUNCTION__, cable.getFrequencyBcd(), cable.getFrequency(), cable.getModulation(), cable.getSymbolRateBcd(), cable.getSymbolRate());
            std::shared_ptr<DvbStorage::TransportStream_t> ts(new DvbStorage::TransportStream_t(cable.getFrequency(),
                                                  mapModulationMode((DVBConstellation)cable.getModulation()),
                                                  cable.getSymbolRate(), it->getOriginalNetworkId(), it->getTsId()));

            OS_LOG(DVB_DEBUG, "<%s> ts_id =0x%x, frequency: %d, modulation: %d, symbol_rate: 0x%x\n",
                    __FUNCTION__, ts->tsId, ts->frequency,  ts->modulation, ts->symbolRate);
            ret.push_back(ts);
        }
        else
        {
            OS_LOG(DVB_ERROR, "<%s> NIT table: cable delivery descriptor not found\n", __FUNCTION__);
        }
    }

    return ret;
}

/**
 * Return a vector of Transport streams
 *
 * @param nId network id
 * @return vector of shared pointers of TransportStream structures
 */
vector<shared_ptr<DvbStorage::TransportStream_t>> DvbSiStorage::getTsListByNetId(uint16_t nId)
{
    vector<shared_ptr<DvbStorage::TransportStream_t>> ret;

    uint16_t networkId;

    if(nId != 0)
    {
        networkId = nId;
    }
    else if(m_preferredNetworkId != 0)
    {
        networkId = m_preferredNetworkId;
    }
    else
    {
        OS_LOG(DVB_ERROR, "<%s> Invalid network id: %d, 0x%x\n", __FUNCTION__, nId, nId);
        return ret;
    }

    string cmdStr("SELECT t.original_network_id, t.transport_id, t.frequency, t.modulation, t.symbol_rate " \
                  "FROM Transport t INNER JOIN Network n "                                                  \
                  "ON t.network_fk = n.network_pk "                                                         \
                  "WHERE n.network_id = ");

    std::stringstream ss;
    ss << networkId;
    cmdStr += ss.str();
    cmdStr += " ORDER BY t.transport_id ASC;";

    vector<vector<string>> results = m_db.query(cmdStr);

    OS_LOG(DVB_DEBUG, "<%s> nid = %d num rows: %u\n", __FUNCTION__, networkId, results.size());

    for(vector<vector<string>>::iterator it = results.begin(); it != results.end(); ++it)
    {
        vector<string> row = *it;

        if(row.size() == 5) // 5 columns specified in select statement
        {
            int onId;
            int tsId;
            int frequency;
            int symbolRate;
            int mod;

            std::stringstream(row.at(0)) >> onId;
            std::stringstream(row.at(1)) >> tsId;
            std::stringstream(row.at(2)) >> frequency;
            std::stringstream(row.at(3)) >> symbolRate;
            std::stringstream(row.at(4)) >> mod;

            OS_LOG(DVB_DEBUG, "<%s> onId: %d tsId: %d frequency: %d mod: %d symbolRate: %d\n", __FUNCTION__, onId, tsId, frequency, mod, symbolRate);
            std::shared_ptr<DvbStorage::TransportStream_t> ts(new DvbStorage::TransportStream_t(static_cast<uint32_t>(frequency),
                                                              mapModulationMode((DVBConstellation)mod),
                                                              static_cast<uint32_t>(symbolRate), static_cast<uint16_t>(onId),
                                                              static_cast<uint16_t>(tsId)));
            ret.push_back(ts);
        }
    }

    return ret;
}

/**
 * Return a vector of Service_t structures
 *
 * @param nId network id
 * @param tsId transport stream id
 * @return vector of shared pointers of Service_t structures
 */
vector<shared_ptr<DvbStorage::Service_t>> DvbSiStorage::getServiceListByTsIdCache(uint16_t nId, uint16_t tsId)
{
    vector<shared_ptr<DvbStorage::Service_t>> ret;

    std::lock_guard<std::mutex> lock(m_dataMutex);

    OS_LOG(DVB_DEBUG, "<%s> called: nid.tsid = 0x%x.0x%x\n", __FUNCTION__, nId, tsId);

    pair<uint16_t, uint16_t> key(nId, tsId);
    auto it = m_sdtMap.find(key);
    if(it == m_sdtMap.end())
    {
        OS_LOG(DVB_ERROR, "<%s> No SDT found for nid.tsid = 0x%x.0x%x\n", __FUNCTION__, nId, tsId);
        return ret;
    }

    const std::vector<DvbService>& serviceList = it->second->getServices();
    for(auto srv = serviceList.begin(), end = serviceList.end(); srv != end; ++srv)
    {
        const std::vector<MpegDescriptor>& serviceDescriptors = srv->getServiceDescriptors();
        const MpegDescriptor* desc = MpegDescriptor::find(serviceDescriptors, DescriptorTag::SERVICE);
        if(desc)
        {
            ServiceDescriptor servDesc(*desc);
            OS_LOG(DVB_DEBUG, "<%s> SDT table: type = 0x%x, provider = %s, name = %s\n",
                    __FUNCTION__, servDesc.getServiceType(), servDesc.getServiceProviderName().c_str(), servDesc.getServiceName().c_str());

            std::shared_ptr<DvbStorage::Service> service(new DvbStorage::Service(it->second->getOriginalNetworkId(), it->second->getExtensionId(),
                                                         srv->getServiceId(), servDesc.getServiceName()));

            ret.push_back(service);
        }
        else
        {
            OS_LOG(DVB_ERROR, "<%s> SDT table: service descriptor not found\n", __FUNCTION__);
        }
    }

    return ret;
}

/**
 * Return a vector of Service_t structures
 *
 * @param nId network id
 * @param tsId transport stream id
 * @return vector of shared pointers of Service_t structures
 */
vector<shared_ptr<DvbStorage::Service_t>> DvbSiStorage::getServiceListByTsId(uint16_t nId, uint16_t tsId)
{
    vector<shared_ptr<DvbStorage::Service_t>> ret;

    string cmdStr("SELECT t.original_network_id, t.transport_id, s.service_id, s.service_name FROM Service s " \
                  " INNER JOIN Transport t "                                                                   \
                  " ON s.transport_fk = t.transport_pk "                                                       \
                  "WHERE t.original_network_id = ");

    std::stringstream ss;
    ss << nId;
    cmdStr += ss.str();
    cmdStr += " AND t.transport_id = ";

    ss.str("");
    ss << tsId;
    cmdStr += ss.str();
    cmdStr += " ORDER BY s.service_id ASC;";

    vector<vector<string>> results = m_db.query(cmdStr);

    OS_LOG(DVB_DEBUG, "<%s> nid.tsid = %d.%d num rows: %u\n", __FUNCTION__, nId, tsId, results.size());

    for(vector<vector<string>>::iterator it = results.begin(); it != results.end(); ++it)
    {
        vector<string> row = *it;

        if(row.size() == 4)  // number of columns in select statement
        {
            int onId;
            int tsId;
            int serviceId;

            std::stringstream(row.at(0)) >> onId;
            std::stringstream(row.at(1)) >> tsId;
            std::stringstream(row.at(2)) >> serviceId;
            string serviceName = row.at(3);

            OS_LOG(DVB_DEBUG, "<%s> onId: %d tsId: %d serviceId: %d %s\n", __FUNCTION__, onId, tsId, serviceId, serviceName.c_str());

            std::shared_ptr<DvbStorage::Service> service(new DvbStorage::Service(static_cast<uint16_t>(onId),
                                                         static_cast<uint16_t>(tsId), static_cast<uint16_t>(serviceId), serviceName));

            ret.push_back(service);
        }
    }

    return ret;
}

/**
 * Return a vector of Events 
 *
 * @param nId network id
 * @param tsId transport stream id
 * @param sId service id
 * @return vector of shared pointers of Event_t structures
 */
vector<shared_ptr<DvbStorage::Event_t>> DvbSiStorage::getEventListByServiceIdCache(uint16_t nId, uint16_t tsId, uint16_t sId)
{
    vector<shared_ptr<DvbStorage::Event_t>> ret;

    std::lock_guard<std::mutex> lock(m_dataMutex);

    OS_LOG(DVB_DEBUG, "<%s> called: nid.tsid.sid = 0x%x.0x%x.0x%x\n", __FUNCTION__, nId, tsId, sId);

    tuple<uint16_t, uint16_t, uint16_t, bool> key(nId, tsId, sId, false);
    auto it = m_eitMap.find(key);
    if(it == m_eitMap.end())
    {
        OS_LOG(DVB_ERROR, "<%s> No EIT found for nid.tsid.sid = 0x%x.0x%x.0x%x\n", __FUNCTION__, nId, tsId, sId);
        return ret;
    }

    const std::vector<DvbEvent>& eventList = it->second->getEvents();
    for(auto e = eventList.begin(), end = eventList.end(); e != end; ++e)
    {
        OS_LOG(DVB_DEBUG, "<%s> EIT table: event_id = 0x%x, duration = %d, status = %d\n",
                __FUNCTION__, e->getEventId(), e->getDuration(), e->getRunningStatus());

        const std::vector<MpegDescriptor>& eventDescriptors = e->getEventDescriptors();
        std::vector<MpegDescriptor> shortList = MpegDescriptor::findAll(eventDescriptors, DescriptorTag::SHORT_EVENT);

        for(auto ext_it = shortList.begin(), ext_end = shortList.end(); ext_it != ext_end; ++ext_it)
        {
            ShortEventDescriptor eventDesc(*ext_it);
            OS_LOG(DVB_DEBUG, "<%s> EIT table: lang_code = %s, name = %s, text = %s\n",
                    __FUNCTION__, eventDesc.getLanguageCode().c_str(), eventDesc.getEventName().c_str(), eventDesc.getText().c_str());

            std::shared_ptr<DvbStorage::Event> event(new DvbStorage::Event);
            event->networkId = it->second->getNetworkId();
            event->tsId = it->second->getTsId();
            event->serviceId = it->second->getExtensionId();
            event->eventId = e->getEventId();
            event->startTime = e->getStartTime();
            event->duration = e->getDuration();
            event->name = eventDesc.getEventName();
            event->text = eventDesc.getText();

            OS_LOG(DVB_TRACE1, "<%s> nid.tsid.sid = 0x%x.0x%x.0x%x, event id: %d, start time: %"PRId64", duration: %d\n",
                    __FUNCTION__, nId, tsId, sId, event->eventId, event->startTime, event->duration);
            OS_LOG(DVB_TRACE1, "<%s> nid.tsid.sid = 0x%x.0x%x.0x%x, event id: %d, event name: %s, text: %s\n",
                    __FUNCTION__, nId, tsId, sId, event->eventId, event->name.c_str(), event->text.c_str());
            ret.push_back(event);
        }
    }

    return ret;
}

/**
 * Return a vector of Events 
 *
 * @param nId network id
 * @param tsId transport stream id
 * @param sId service id
 * @return vector of shared pointers of Event_t structures
 */
vector<shared_ptr<DvbStorage::Event_t>> DvbSiStorage::getEventListByServiceId(uint16_t nId, uint16_t tsId, uint16_t sId)
{
    vector<shared_ptr<DvbStorage::Event_t>> ret;

    string cmdStr("SELECT e.network_id, e.transport_id, e.service_id, e.event_id, e.start_time, e.duration, " \
                  " ei.title, ei.description FROM EventItem ei "                                              \
                  " INNER JOIN Event e "                                                                      \
                  " ON ei.event_fk = e.event_pk "                                                             \
                  "WHERE e.network_id = ");

    std::stringstream ss;
    ss << nId;
    cmdStr += ss.str();
    cmdStr += " AND e.transport_id = ";

    ss.str("");
    ss << tsId;
    cmdStr += ss.str();
    cmdStr += " AND e.service_id = ";

    ss.str("");
    ss << sId;
    cmdStr += ss.str();
    cmdStr += " ORDER BY e.event_id ASC;";

    vector<vector<string>> results = m_db.query(cmdStr);

    OS_LOG(DVB_DEBUG, "<%s> nid.tsid.sid = %d.%d.%d num rows: %u\n", __FUNCTION__, nId, tsId, sId, results.size());

    for(vector<vector<string>>::iterator it = results.begin(); it != results.end(); ++it)
    {
        vector<string> row = *it;

        if(row.size() == 8)  // number of columns in select statement
        {
            int onId;
            int tsId;
            int serviceId;
            int eventId;
            long long int startTime;
            int duration;

            std::stringstream(row.at(0)) >> onId;
            std::stringstream(row.at(1)) >> tsId;
            std::stringstream(row.at(2)) >> serviceId;
            std::stringstream(row.at(3)) >> eventId;
            std::stringstream(row.at(4)) >> startTime;
            std::stringstream(row.at(5)) >> duration;
            string title = row.at(6);
            string description = row.at(7);

            OS_LOG(DVB_DEBUG, "<%s> nId: %d tsId: %d serviceId: %d eventId: %d startTime: %lld " \
                                   "duration: %d title length: %u description length: %u\n", __FUNCTION__, nId, tsId, serviceId, 
                                   eventId, startTime, duration, title.size(), description.size());

            std::shared_ptr<DvbStorage::Event> event(new DvbStorage::Event);
            event->networkId = nId;
            event->tsId = tsId;
            event->serviceId = serviceId;
            event->eventId = eventId;
            event->startTime = startTime;
            event->duration = duration;
            event->name = title;
            event->text = description;

            ret.push_back(event);
        }
    }

    return ret;
}

/**
 * Handle Nit table
 *
 * @param nit Nit table
 */
void DvbSiStorage::handleNitEvent(const NitTable& nit)
{
    // TODO: Consider removing one level of handle methods.
    processNitEventCache(nit);

    processNitEventDb(nit);
}

/**
 * Process Nit table for cache storage
 *
 * @param nit Nit table
 */
void DvbSiStorage::processNitEventCache(const NitTable& nit)
{
    std::lock_guard<std::mutex> lock(m_dataMutex);

    auto it = m_nitMap.find(nit.getNetworkId());
    if(it == m_nitMap.end())
    {
        OS_LOG(DVB_DEBUG, "<%s> Adding NIT table to the map. Network id: 0x%x\n", __FUNCTION__, nit.getNetworkId());
        m_nitMap.insert(std::make_pair(nit.getNetworkId(), std::make_shared<NitTable>(nit)));
    }
    else
    {
        OS_LOG(DVB_DEBUG, "<%s> NIT already in cache. Network id: %d version: %d\n", __FUNCTION__,nit.getNetworkId(),nit.getVersion());
        if(nit.getVersion() == it->second->getVersion())
        {
            OS_LOG(DVB_DEBUG, "<%s> NIT version matches (%d). Skipping\n", __FUNCTION__, nit.getVersion());
        }
        else
        {
            OS_LOG(DVB_DEBUG, "<%s> Current version: 0x%x, new version: 0x%x\n", __FUNCTION__, it->second->getVersion(), nit.getVersion());
            it->second = std::make_shared<NitTable>(nit);
        }
    }

}

/**
 * Process Nit table for database storage
 *
 * @param nit Nit table
 */
void DvbSiStorage::processNitEventDb(const NitTable& nit)
{
    int64_t network_fk = -1;
    int8_t nitVersion = -1;

    string cmdStr("SELECT nit_pk, version FROM Nit WHERE network_id = ");

    std::stringstream ss;
    ss << nit.getNetworkId();
    cmdStr += ss.str();
    cmdStr += ";";

    int64_t nit_fk = m_db.findKey(cmdStr, nitVersion);
    if(nit_fk > 0)  // FOUND
    {
        if(nit.getVersion() == nitVersion)
        {
            OS_LOG(DVB_DEBUG, "<%s> NIT version matches (%d). Skipping\n", __FUNCTION__, nit.getVersion());
        }
        else 
        {
            OS_LOG(DVB_DEBUG, "<%s> Current version: %d, new version: %d\n", __FUNCTION__, nitVersion, nit.getVersion());

            m_db.dropTables();
            m_db.createTables();

            return;
        }
    }

    if(nit_fk < 1)  // NOT FOUND
    {
        OS_LOG(DVB_DEBUG, "<%s> Adding NIT table to database. Network id: %d version: %d\n", __FUNCTION__, nit.getNetworkId(), nit.getVersion());

        DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO Nit (network_id, version) VALUES (?, ?);"));
        cmd.bind(1, static_cast<int>(nit.getNetworkId()));
        cmd.bind(2, static_cast<int>(nit.getVersion()));
        cmd.execute(nit_fk);

        OS_LOG(DVB_DEBUG, "<%s> nit_fk %lld\n", __FUNCTION__, nit_fk);

        network_fk = processNetwork(nit);
        OS_LOG(DVB_DEBUG, "<%s> network_fk %lld\n", __FUNCTION__, network_fk);
    }

    if(nit_fk > 0)
    {
        m_db.insertDescriptor(static_cast<const char*>("NitDescriptor"), nit_fk, nit.getNetworkDescriptors());
    }

    const std::vector<TransportStream>& tsList = nit.getTransportStreams();
    for(auto it = tsList.begin(); it != tsList.end(); ++it)
    {
        string cmdStr("SELECT nit_transport_pk FROM NitTransport WHERE original_network_id = ");
        std::stringstream ss;
        ss << it->getOriginalNetworkId();
        cmdStr += ss.str();
        cmdStr += " AND transport_id = ";

        ss.str("");
        ss << it->getTsId();
        cmdStr += ss.str();
        cmdStr += ";";

        int64_t nit_transport_fk = m_db.findKey(cmdStr);
        if(nit_transport_fk < 1)   // NOT FOUND
        {
            DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO NitTransport (original_network_id, transport_id, nit_fk) VALUES (?, ?, ?);"));
            cmd.bind(1, static_cast<int>(it->getOriginalNetworkId()));
            cmd.bind(2, static_cast<int>(it->getTsId()));
            cmd.bind(3, static_cast<int>(nit_fk));
            cmd.execute(nit_transport_fk);

            OS_LOG(DVB_DEBUG, "<%s> Insert nit_transport_fk %lld\n", __FUNCTION__, nit_transport_fk);

            if(network_fk > 0)
            {
                processTransport(*it, network_fk);
            }
        }

        if(nit_transport_fk > 0)
        {
            m_db.insertDescriptor(static_cast<const char*>("NitTransportDescriptor"), nit_transport_fk, 
                                           it->getTsDescriptors());
        }
    }

    m_db.performUpdate();
}

/**
 * Handle Bat table
 *
 * @param bat Bat table
 */
void DvbSiStorage::handleBatEvent(const BatTable& bat)
{
    processBatEventCache(bat);

    processBatEventDb(bat);
}

/**
 * Process Bat table for cache storage
 *
 * @param bat Bat table
 */
void DvbSiStorage::processBatEventCache(const BatTable& bat)
{
    std::lock_guard<std::mutex> lock(m_dataMutex);

    auto it = m_batMap.find(bat.getBouquetId());
    if(it == m_batMap.end())
    {
        OS_LOG(DVB_DEBUG, "<%s> Adding BAT table to the map. Bouquet id: 0x%x\n", __FUNCTION__, bat.getBouquetId());
        m_batMap.insert(std::make_pair(bat.getBouquetId(), std::make_shared<BatTable>(bat)));
    }
    else
    {
        OS_LOG(DVB_DEBUG, "<%s> BAT already in cache. Bouquet id: %d version: %d\n", __FUNCTION__, bat.getBouquetId(),bat.getVersion());
        if(bat.getVersion() == it->second->getVersion())
        {
            OS_LOG(DVB_DEBUG, "<%s> BAT version matches (%d). Skipping\n", __FUNCTION__, bat.getVersion());
        }
        else
        {
            OS_LOG(DVB_DEBUG, "<%s> Current version: 0x%x, new version: 0x%x\n", __FUNCTION__, it->second->getVersion(), bat.getVersion());
            it->second = std::make_shared<BatTable>(bat);
        }
    }
}

/**
 * Process Bat table for database storage
 *
 * @param bat Bat table
 */
void DvbSiStorage::processBatEventDb(const BatTable& bat)
{
    int64_t network_fk = -1;

    if(m_preferredNetworkId != 0)
    {    
        string cmdStr("SELECT network_pk FROM Network WHERE network_id = ");

        std::stringstream ss;
        ss << m_preferredNetworkId;
        cmdStr += ss.str();
        cmdStr += ";";

        network_fk = m_db.findKey(cmdStr);
        if(network_fk < 1)
        {
            OS_LOG(DVB_DEBUG, "<%s> network_fk %lld\n", __FUNCTION__, network_fk);
            return;
        }  
    }

    string cmdStr("SELECT bat_pk, version FROM Bat WHERE bouquet_id = ");

    std::stringstream ss;
    ss << bat.getBouquetId();
    cmdStr += ss.str();
    cmdStr += ";";

    int8_t batVersion = -1;
    int64_t bat_fk = m_db.findKey(cmdStr, batVersion);
    if(bat_fk > 0)    // FOUND
    {
        if(bat.getVersion() == batVersion)
        {
            OS_LOG(DVB_DEBUG, "<%s> BAT version matches (%d). Skipping\n", __FUNCTION__, bat.getVersion());
        }
        else
        {
            OS_LOG(DVB_DEBUG, "<%s> Current version: %d, new version: %d\n", __FUNCTION__, batVersion, bat.getVersion());

            // Bat version change
            DvbDb::Command cmd(m_db, string("DELETE FROM Bat WHERE bouquet_id = ? AND version != ?;"));
            cmd.bind(1, static_cast<int>(bat.getBouquetId()));
            cmd.bind(2, static_cast<int>(bat.getVersion()));
            cmd.execute();

            m_db.sqlCommand(string("DELETE FROM BatDescriptor WHERE fkey NOT IN (SELECT DISTINCT bat_pk FROM Bat);"));

            m_db.sqlCommand(string("DELETE FROM BatTransport WHERE bat_fk NOT IN (SELECT DISTINCT bat_pk FROM Bat);"));

            m_db.sqlCommand(string("DELETE FROM BatTransportDescriptor WHERE fkey NOT IN " \
                                   " (SELECT DISTINCT bat_transport_pk FROM BatTransport);"));

            processBouquet(bat);

            bat_fk = m_db.findKey(cmdStr, batVersion);
        }
    }

    if(bat_fk < 1)    // NOT FOUND
    {
        OS_LOG(DVB_DEBUG, "<%s> Adding BAT table to the database. Bouquet id: %d version: %d\n", __FUNCTION__, bat.getBouquetId(), bat.getVersion());

        DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO Bat (bouquet_id, version) VALUES (?, ?);"));
        cmd.bind(1, static_cast<int>(bat.getBouquetId()));
        cmd.bind(2, static_cast<int>(bat.getVersion()));
        cmd.execute(bat_fk);

        OS_LOG(DVB_DEBUG, "<%s> Insert bat_fk %lld\n", __FUNCTION__, bat_fk);

        processBouquet(bat);
    }

    if(bat_fk > 0)
    {
        m_db.insertDescriptor(static_cast<const char*>("BatDescriptor"), bat_fk, bat.getBouquetDescriptors());
    }

    const std::vector<TransportStream>& tsList = bat.getTransportStreams();
    for(auto it = tsList.begin(); it != tsList.end(); ++it)
    {
        // NOTE: Order dependency of NitTransport entries existing before BatTransport entries.
        string cmdStr("SELECT nit_transport_pk FROM NitTransport WHERE original_network_id = ");
        std::stringstream ss;
        ss << it->getOriginalNetworkId();
        cmdStr += ss.str();
        cmdStr += " AND transport_id = ";

        ss.str("");
        ss << it->getTsId();
        cmdStr += ss.str();
        cmdStr += ";";

        int64_t nit_transport_fk = m_db.findKey(cmdStr);
        if(nit_transport_fk > 0)
        {
            string cmdStr("SELECT bat_transport_pk FROM BatTransport WHERE original_network_id = ");

            std::stringstream ss;
            ss << it->getOriginalNetworkId();
            cmdStr += ss.str();
            cmdStr += " AND transport_id = ";

            ss.str("");
            ss << it->getTsId();
            cmdStr += ss.str();
            cmdStr += ";";

            int64_t bat_transport_fk = m_db.findKey(cmdStr);
            if(bat_transport_fk < 1)  // NOT FOUND
            { 
                DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO BatTransport (original_network_id, transport_id, bat_fk, " \
                                      "nit_transport_fk) VALUES (?, ?, ?, ?);"));
                cmd.bind(1, static_cast<int>(it->getOriginalNetworkId()));
                cmd.bind(2, static_cast<int>(it->getTsId()));
                cmd.bind(3, static_cast<long long int>(bat_fk));
                cmd.bind(4, static_cast<long long int>(nit_transport_fk));
                cmd.execute(bat_transport_fk);

                OS_LOG(DVB_DEBUG, "<%s> Insert bat_transport_fk %lld\n", __FUNCTION__, bat_transport_fk);
            }

            if(bat_transport_fk > 0)
            {
                m_db.insertDescriptor(static_cast<const char*>("BatTransportDescriptor"), bat_transport_fk, it->getTsDescriptors());
            }
        }
    }

    m_db.performUpdate();
}

/**
 * Handle Sdt table
 *
 * @param sdt Sdt table
 */
void DvbSiStorage::handleSdtEvent(const SdtTable& sdt)
{
    processSdtEventCache(sdt);

    processSdtEventDb(sdt);
}

/**
 * Process Sdt table for cache storage
 *
 * @param sdt Sdt table
 */
void DvbSiStorage::processSdtEventCache(const SdtTable& sdt)
{
    std::lock_guard<std::mutex> lock(m_dataMutex);

    pair<uint16_t, uint16_t> key(sdt.getOriginalNetworkId(), sdt.getExtensionId());
    auto it = m_sdtMap.find(key);
    if(it == m_sdtMap.end())
    {
        OS_LOG(DVB_DEBUG, "<%s> Adding SDT table to the cache. nid.tsid: 0x%x.0x%x\n", __FUNCTION__, sdt.getOriginalNetworkId(), sdt.getExtensionId());
        m_sdtMap.insert(std::make_pair(key, std::make_shared<SdtTable>(sdt)));
    }
    else
    {
        OS_LOG(DVB_DEBUG, "<%s> SDT already in cache. nid.tsid: %d.%d\n", __FUNCTION__, sdt.getOriginalNetworkId(), sdt.getExtensionId());
        if(sdt.getVersion() == it->second->getVersion())
        {
            OS_LOG(DVB_DEBUG, "<%s> SDT version matches (0x%x). Skipping\n", __FUNCTION__, sdt.getVersion());
        }
        else
        {
            OS_LOG(DVB_DEBUG, "<%s> Current version: 0x%x, new version: 0x%x\n", __FUNCTION__, it->second->getVersion(), sdt.getVersion());
            it->second = std::make_shared<SdtTable>(sdt);
        }
    }
}

/**
 * Process Sdt table for database storage
 *
 * @param sdt Sdt table
 */
void DvbSiStorage::processSdtEventDb(const SdtTable& sdt)
{

    string cmdStr("SELECT nit_transport_pk FROM NitTransport WHERE original_network_id = ");

    std::stringstream ss;
    ss << sdt.getOriginalNetworkId();
    cmdStr += ss.str();
    cmdStr += " AND transport_id = ";

    ss.str("");
    ss << sdt.getExtensionId();
    cmdStr += ss.str();
    cmdStr += ";";

    int64_t nit_transport_fk = m_db.findKey(cmdStr);
    if(nit_transport_fk < 1)
    {
        OS_LOG(DVB_DEBUG, "<%s> nit_transport_fk %lld\n", __FUNCTION__, nit_transport_fk);
        return;
    }

    const std::vector<DvbService>&  serviceList = sdt.getServices();
    for(auto it = serviceList.begin(); it != serviceList.end(); ++it)
    {
        const DvbService& service = (*it);

        string cmdStr("SELECT s.sdt_pk, version FROM Sdt s INNER JOIN NitTransport nt " \
                      " ON s.nit_transport_fk = nt.nit_transport_pk "                   \
                      "WHERE nt.original_network_id = ");

        std::stringstream ss;
        ss << sdt.getOriginalNetworkId();
        cmdStr += ss.str();
        cmdStr += " AND nt.transport_id = ";

        ss.str("");
        ss << sdt.getExtensionId();
        cmdStr += ss.str();
        cmdStr += " AND s.service_id = ";

        ss.str("");
        ss << service.getServiceId();
        cmdStr += ss.str();
        cmdStr += ";";

        int8_t sdtVersion = -1; 
        int64_t sdt_fk = m_db.findKey(cmdStr, sdtVersion);
        if(sdt_fk > 0)   // FOUND 
        {
            if(sdt.getVersion() == sdtVersion)
            {
                OS_LOG(DVB_DEBUG, "<%s> Sdt version matches (%d). Skipping\n", __FUNCTION__, sdt.getVersion());
            }
            else
            {
                OS_LOG(DVB_DEBUG, "<%s> Current version: %d, new version: %d\n", __FUNCTION__, sdtVersion, sdt.getVersion());

                // Sdt version change
                {
                    DvbDb::Command cmd(m_db, string("DELETE FROM Sdt WHERE service_id = ? AND version != ? AND nit_transport_fk IN " \
                                                 " (SELECT nt.nit_transport_pk FROM Sdt s INNER JOIN NitTransport nt "            \
                                                 "  ON s.nit_transport_fk = nt.nit_transport_pk "                                 \
                                                 "  WHERE nt.original_network_id = ? AND nt.transport_id = ? AND s.service_id = ?);"));
                    cmd.bind(1, static_cast<int>(service.getServiceId()));
                    cmd.bind(2, static_cast<int>(sdt.getVersion()));
                    cmd.bind(3, static_cast<int>(sdt.getOriginalNetworkId()));
                    cmd.bind(4, static_cast<int>(sdt.getExtensionId()));
                    cmd.bind(5, static_cast<int>(service.getServiceId()));
                    cmd.execute();
                }

                m_db.sqlCommand(string("DELETE FROM SdtDescriptor WHERE fkey NOT IN (SELECT DISTINCT sdt_pk FROM Sdt);"));

                {
                    DvbDb::Command cmd(m_db, string("DELETE FROM Eit WHERE network_id = ? AND transport_id = ? AND service_id = ?;"));
                    cmd.bind(1, static_cast<int>(sdt.getOriginalNetworkId()));
                    cmd.bind(2, static_cast<int>(sdt.getExtensionId()));
                    cmd.bind(3, static_cast<int>(service.getServiceId()));
                    cmd.execute();
                }

                m_db.sqlCommand(string("DELETE FROM EitDescriptor WHERE fkey NOT IN (SELECT DISTINCT eit_pk FROM Eit);"));

                sdt_fk = m_db.findKey(cmdStr, sdtVersion);

                processService(sdt);
             }
        }

        if(sdt_fk < 1)  // NOT FOUND
        {
            OS_LOG(DVB_DEBUG, "<%s> Adding SDT table to database. nid.tsid: %d.%d\n", __FUNCTION__, sdt.getOriginalNetworkId(), sdt.getExtensionId());

            DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO Sdt (service_id, nit_transport_fk, version, schedule, " \
                                         " present_following, scrambled, running) VALUES (?, ?, ?, ?, ?, ?, ?);"));
            cmd.bind(1, static_cast<int>(service.getServiceId()));
            cmd.bind(2, static_cast<int>(nit_transport_fk));
            cmd.bind(3, static_cast<int>(sdt.getVersion()));
            cmd.bind(4, static_cast<int>(service.isEitSchedFlagSet()));
            cmd.bind(5, static_cast<int>(service.isEitPfFlagSet()));
            cmd.bind(6, static_cast<int>(service.isScrambled()));
            cmd.bind(7, static_cast<int>(service.getRunningStatus()));
            cmd.execute(sdt_fk);
            
            OS_LOG(DVB_DEBUG, "<%s> Insert sdt_fk = %lld\n", __FUNCTION__, sdt_fk);

            processService(sdt);
        }

        if(sdt_fk > 0)
        {
            m_db.insertDescriptor(static_cast<const char*>("SdtDescriptor"), sdt_fk, service.getServiceDescriptors());
        }

    } // for

    m_db.performUpdate();
}

/**
 * Handle Eit table
 *
 * @param eit Eit table
 */
void DvbSiStorage::handleEitEvent(const EitTable& eit)
{
    processEitEventCache(eit);

    processEitEventDb(eit);
}

/**
 * Process Eit table for cache storage
 *
 * @param eit Eit table
 */
void DvbSiStorage::processEitEventCache(const EitTable& eit)
{
    std::lock_guard<std::mutex> lock(m_dataMutex);

    bool isPf = false;
    TableId tableId = eit.getTableId();
    if(tableId == TableId::EIT_PF || tableId == TableId::EIT_PF_OTHER)
    {
        isPf = true;
    }

    tuple<uint16_t, uint16_t, uint16_t, bool> key(eit.getNetworkId(), eit.getTsId(), eit.getExtensionId(), isPf);
    auto it = m_eitMap.find(key);
    if(it == m_eitMap.end())
    {
        OS_LOG(DVB_DEBUG, "<%s> Adding EIT table to the cache. nid.tsid.sid: 0x%x.0x%x.0x%x\n",
                __FUNCTION__, eit.getNetworkId(), eit.getTsId(), eit.getExtensionId());

        m_eitMap.insert(std::make_pair(key, std::make_shared<EitTable>(eit)));
    }
    else
    {
        OS_LOG(DVB_DEBUG, "<%s> EIT already in cache. nid.tsid.sid: 0x%x.0x%x.0x%x\n", __FUNCTION__, eit.getNetworkId(), eit.getTsId(), eit.getExtensionId());
        if(eit.getVersion() == it->second->getVersion())
        {
            OS_LOG(DVB_DEBUG, "<%s> EIT version matches (0x%x). Skipping\n", __FUNCTION__, eit.getVersion());
        }
        else
        {
            OS_LOG(DVB_DEBUG, "<%s> Current version: 0x%x, new version: 0x%x\n", __FUNCTION__, it->second->getVersion(), eit.getVersion());
            it->second = std::make_shared<EitTable>(eit);
        }
    }
}

/**
 * Process Eit table for database storage
 *
 * @param eit Eit table
 */
void DvbSiStorage::processEitEventDb(const EitTable& eit)
{

    const std::vector<DvbEvent>&  eventList = eit.getEvents();

    for(auto it = eventList.begin(); it != eventList.end(); ++it)
    {
        const DvbEvent& event = (*it);

        string cmdStr("SELECT eit_pk, version FROM Eit WHERE network_id = ");
        std::stringstream ss;
        ss << eit.getNetworkId();
        cmdStr += ss.str();
        cmdStr += " AND transport_id = ";

        ss.str("");
        ss << eit.getTsId();
        cmdStr += ss.str();
        cmdStr += " AND service_id = ";

        ss.str("");
        ss << eit.getExtensionId();
        cmdStr += ss.str();
        cmdStr += " AND event_id = ";

        ss.str("");
        ss << event.getEventId();
        cmdStr += ss.str();
        cmdStr += ";";

        int8_t eitVersion = -1;
        int64_t eit_fk = m_db.findKey(cmdStr, eitVersion);
        if(eit_fk > 0)  // FOUND
        {
            if(eit.getVersion() == eitVersion)
            {
                OS_LOG(DVB_DEBUG, "<%s> EIT version matches (%d). Skipping\n", __FUNCTION__, eit.getVersion());
            }
            else
            {
                OS_LOG(DVB_DEBUG, "<%s> Current version: %d, new version: %d\n", __FUNCTION__, eitVersion, eit.getVersion());
                // Eit version change
                DvbDb::Command cmd(m_db, string("DELETE FROM Eit WHERE network_id = ? AND transport_id = ? AND event_id = ? AND " \
                                             " version != ?;"));
                cmd.bind(1, static_cast<int>(eit.getNetworkId()));
                cmd.bind(2, static_cast<int>(eit.getTsId()));
                cmd.bind(3, static_cast<int>(eit.getExtensionId()));
                cmd.bind(4, static_cast<int>(eit.getVersion()));
                cmd.execute();

                m_db.sqlCommand(string("DELETE FROM EitDescriptor WHERE fkey NOT IN (SELECT DISTINCT eit_pk FROM Eit);"));

                eit_fk = m_db.findKey(cmdStr, eitVersion);

                processEvent(eit);
            }
        }

        if(eit_fk < 1)  // NOT FOUND
        {
            OS_LOG(DVB_DEBUG, "<%s> Adding EIT table to the database. nid.tsid.sid: 0x%x.0x%x.0x%x\n", __FUNCTION__, eit.getNetworkId(), eit.getTsId(), eit.getExtensionId());

            DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO Eit (network_id, transport_id, service_id, event_id, version, " \
                                         " start_time, duration, scrambled, running) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);"));
            cmd.bind(1, static_cast<int>(eit.getNetworkId()));
            cmd.bind(2, static_cast<int>(eit.getTsId()));
            cmd.bind(3, static_cast<int>(eit.getExtensionId()));
            cmd.bind(4, static_cast<int>(event.getEventId()));
            cmd.bind(5, static_cast<int>(eit.getVersion()));
            cmd.bind(6, static_cast<long long int>(event.getStartTime()));
            cmd.bind(7, static_cast<int>(event.getDuration()));
            cmd.bind(8, static_cast<int>(event.isScrambled()));
            cmd.bind(9, static_cast<int>(event.getRunningStatus()));
            cmd.execute(eit_fk);

            OS_LOG(DVB_DEBUG, "<%s> Insert eit_fk %lld\n", __FUNCTION__, eit_fk);

            processEvent(eit);
        }

        if(eit_fk > 0)
        {
            m_db.insertDescriptor(static_cast<const char*>("EitDescriptor"), eit_fk, it->getEventDescriptors());
        }
    } 
}

/**
 * Process Nit table for parsed database storage
 *
 * @param nit Nit table
 * @return foreign key
 */
int64_t  DvbSiStorage::processNetwork(const NitTable& nit)
{
    int64_t network_fk = -1;
    string networkName;
    string iso639languageCode;

    const vector<MpegDescriptor>& descList = nit.getNetworkDescriptors();

    for(auto it = descList.cbegin(); it != descList.cend(); ++it)
    {
        const MpegDescriptor& md = *it;
        if(md.getTag() == DescriptorTag::NETWORK_NAME)
        {
            NetworkNameDescriptor nnd(md);
            networkName = nnd.getName();
            break;
        }

        if(md.getTag() == DescriptorTag::MULTILINGUAL_NETWORK_NAME)
        {
            MultilingualNetworkNameDescriptor  mnnd(md);
            for(int32_t i=0; i < mnnd.getCount(); i++)
            {
                iso639languageCode += mnnd.getLanguageCode(i);
                iso639languageCode += " ";
                networkName += mnnd.getNetworkName(i);
                networkName += " ";
            }
        }
    }

    DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO Network (network_id, version, iso_639_language_code, name) " \
                                 " VALUES (?, ?, ?, ?);"));
    cmd.bind(1, static_cast<int>(nit.getNetworkId()));
    cmd.bind(2, static_cast<int>(nit.getVersion()));

    if(iso639languageCode.empty())
    {  
        cmd.bind(3); // Db NULL type
    }
    else
    {
        cmd.bind(3, iso639languageCode);
    }

    if(networkName.empty())
    {
        cmd.bind(4);
    }
    else
    {
        cmd.bind(4, networkName);
    }

    cmd.execute(network_fk);

    OS_LOG(DVB_DEBUG, "<%s> Insert network_fk %lld\n", __FUNCTION__, network_fk);

    return network_fk;
}

/**
 * Process Bat table for parsed database storage
 *
 * @param bat Bat table
 * @return foreign key
 */
int64_t  DvbSiStorage::processBouquet(const BatTable& bat)
{

    string cmdStr("SELECT bouquet_pk, version FROM Bouquet WHERE bouquet_id = ");

    std::stringstream ss;
    ss << bat.getBouquetId();
    cmdStr += ss.str();
    cmdStr += ";";

    int8_t bouquetVersion = -1;
    int64_t bouquet_fk = m_db.findKey(cmdStr, bouquetVersion);
    if(bouquet_fk > 0)  // FOUND
    {
        if(bat.getVersion() == bouquetVersion)
        {
            OS_LOG(DVB_DEBUG, "<%s> BAT version matches (%d). Skipping\n", __FUNCTION__, bat.getVersion());
        }
        else
        {
            OS_LOG(DVB_DEBUG, "<%s> Current version: %d, new version: %d\n", __FUNCTION__, bouquetVersion, bat.getVersion());

            // Bat version change
            DvbDb::Command cmd(m_db, string("DELETE FROM Bouquet WHERE bouquet_id = ? AND version != ?;"));
            cmd.bind(1, static_cast<int>(bat.getVersion()));
            cmd.bind(2, static_cast<int>(bat.getVersion())); 
            cmd.execute();

            bouquet_fk = m_db.findKey(cmdStr, bouquetVersion);
        }
    }

    if(bouquet_fk < 1)  // NOT FOUND
    {
        string networkName;
        string iso639languageCode;

        // Either a Network Name descriptor or a Multilingual Network descriptor exists. 
        const vector<MpegDescriptor>& descList = bat.getBouquetDescriptors();

        for(auto it = descList.cbegin(); it != descList.cend(); ++it)
        {
            const MpegDescriptor& md = *it;
            if(md.getTag() == DescriptorTag::NETWORK_NAME)
            {
                NetworkNameDescriptor nnd(md);
                networkName += nnd.getName();
            }

            if(md.getTag() == DescriptorTag::MULTILINGUAL_NETWORK_NAME)
            {
                MultilingualNetworkNameDescriptor  mnnd(md);
                for(int32_t i=0; i < mnnd.getCount(); i++)
                {
                    iso639languageCode += mnnd.getLanguageCode(i);
                    iso639languageCode += " ";
                    networkName += mnnd.getNetworkName(i);
                    networkName += " ";
                }
            }
        }

        OS_LOG(DVB_DEBUG, "<%s> bouquet_id: %d version: %d networkName: %s iso: %s\n", __FUNCTION__, bat.getBouquetId(), bat.getVersion(), networkName.c_str(), iso639languageCode.c_str());

        DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO Bouquet (bouquet_id, version, iso_639_language_code, name) " \
                                     " VALUES (?, ?, ?, ?);"));
        cmd.bind(1, static_cast<int>(bat.getBouquetId()));
        cmd.bind(2, static_cast<int>(bat.getVersion()));

        if(iso639languageCode.empty())
        {  
            cmd.bind(3);
        }
        else
        {
            cmd.bind(3, iso639languageCode);
        }

        if(networkName.empty())
        {
            cmd.bind(4);
        }
        else
        {
            cmd.bind(4, networkName);
        }

        cmd.execute(bouquet_fk);

        OS_LOG(DVB_DEBUG, "<%s> Insert bouquet_fk %lld\n", __FUNCTION__, bouquet_fk);

        // Set bouquet_fk foreign key in Transport.
        if(bouquet_fk > 0)
        {
            const std::vector<TransportStream>& tsList = bat.getTransportStreams();
            for(auto it = tsList.begin(); it != tsList.end(); ++it)
            {
                string cmdStr("SELECT transport_pk FROM Transport WHERE original_network_id = ");

                std::stringstream ss;
                ss << it->getOriginalNetworkId();
                cmdStr += ss.str();
                cmdStr += " AND transport_id = ";

                ss.str("");
                ss << it->getTsId();
                cmdStr += ss.str();
                cmdStr += ";";

                int64_t transport_fk = m_db.findKey(cmdStr);
                if(transport_fk > 0)
                {
                    DvbDb::Command cmd(m_db, string("UPDATE Transport SET bouquet_fk = ? WHERE transport_pk = ?;"));
                    cmd.bind(1, static_cast<long long int>(bouquet_fk));
                    cmd.bind(2, static_cast<long long int>(transport_fk));
                    cmd.execute();

                    if(m_db.modifications() == 0) // no change schedule an update 
                    {
                        string cmdStr("UPDATE Transport SET bouquet_fk = ");

                        std::stringstream ss;
                        ss << bouquet_fk;
                        cmdStr += ss.str();
                        cmdStr += " WHERE transport_pk = ";

                        ss.str("");
                        ss << transport_fk;
                        cmdStr += ss.str();
                        cmdStr += ";";

                        m_db.addUpdate(cmdStr.c_str());
                    }
                }
            }
        }
    }

    return bouquet_fk;
}

/**
 * Process Transport table for parsed database storage
 *
 * @param ts TransportStream reference
 * @param network_fk foreign key value of Network entry
 * @return foreign key
 */
int64_t  DvbSiStorage::processTransport(const TransportStream& ts, int64_t network_fk)
{
    int64_t transport_fk = -1;
    uint8_t modulation = 0;
    uint32_t frequency = 0;
    uint32_t symbolRate = 0;
    uint8_t  fecInner = 0;
    uint8_t  fecOuter = 0;

    const vector<MpegDescriptor>& tsDesc = ts.getTsDescriptors();

    for(auto it = tsDesc.cbegin(); it != tsDesc.cend(); ++it)
    {
        const MpegDescriptor& md = *it;
        if(md.getTag() == DescriptorTag::CABLE_DELIVERY)
        {
            CableDeliverySystemDescriptor cable(md);
            frequency = cable.getFrequency();
            modulation = static_cast<uint8_t>(mapModulationMode((DVBConstellation)cable.getModulation()));
            symbolRate = cable.getSymbolRate();
            fecInner = cable.getFecInner();
            fecOuter = cable.getFecOuter();
        }
    }

    DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO Transport (original_network_id, transport_id, network_fk, frequency, " \
                                 " modulation, symbol_rate, fec_outer, fec_inner) VALUES (?, ?, ?, ?, ?, ?, ?, ?);"));
    cmd.bind(1, static_cast<int>(ts.getOriginalNetworkId()));
    cmd.bind(2, static_cast<int>(ts.getTsId()));
    cmd.bind(3, static_cast<long long int>(network_fk));
    cmd.bind(4, static_cast<int>(frequency));
    cmd.bind(5, static_cast<int>(modulation));
    cmd.bind(6, static_cast<int>(symbolRate));
    cmd.bind(7, static_cast<int>(fecOuter));
    cmd.bind(8, static_cast<int>(fecInner));
    cmd.execute(transport_fk);

    OS_LOG(DVB_DEBUG, "<%s> Insert transport_fk %lld\n", __FUNCTION__, transport_fk);

    return  transport_fk;
}

/**
 * Process Service table for parsed database storage
 *
 * @param sdt Sdt Table
 * @return foreign key
 */
int64_t  DvbSiStorage::processService(const SdtTable& sdt)
{
    int64_t service_fk = -1;

    string cmdStr("SELECT transport_pk FROM Transport WHERE original_network_id = ");

    std::stringstream ss;
    ss << sdt.getOriginalNetworkId();
    cmdStr += ss.str();
    cmdStr += " AND transport_id = ";

    ss.str("");
    ss << sdt.getExtensionId();
    cmdStr += ss.str();
    cmdStr += ";";

    int64_t transport_fk = m_db.findKey(cmdStr);
    if(transport_fk < 1)
    {
        OS_LOG(DVB_DEBUG, "<%s> transport_fk = %lld\n", __FUNCTION__, transport_fk);
        return service_fk;
    }

    const std::vector<DvbService>&  serviceList = sdt.getServices();
    for(auto it = serviceList.begin(); it != serviceList.end(); ++it)
    {
        const DvbService& service = (*it);
        string cmdStr("SELECT s.service_pk, s.version FROM Service s INNER JOIN Transport t " \
                      " ON s.transport_fk = t.transport_pk "                                  \
                      "WHERE t.original_network_id = ");

        std::stringstream ss;
        ss << sdt.getOriginalNetworkId();
        cmdStr += ss.str();
        cmdStr += " AND t.transport_id = ";

        ss.str("");
        ss << sdt.getExtensionId();
        cmdStr += ss.str();
        cmdStr += " AND s.service_id = ";

        ss.str("");
        ss << service.getServiceId();
        cmdStr += ss.str();
        cmdStr += ";";

        int8_t serviceVersion = -1; 
        int64_t service_fk = m_db.findKey(cmdStr, serviceVersion);
        if(service_fk > 0)  // FOUND
        {
            if(sdt.getVersion() == serviceVersion) 
            {
                OS_LOG(DVB_DEBUG, "<%s> Sdt version matches (%d). Skipping\n", __FUNCTION__, sdt.getVersion());
            }
            else
            {
                OS_LOG(DVB_DEBUG, "<%s> Current version: %d, new version: %d\n", __FUNCTION__, serviceVersion, sdt.getVersion());

                // Sdt version change
                {
                    DvbDb::Command cmd(m_db, string("DELETE FROM Service WHERE service_id = ? AND version != ? AND transport_fk IN " \
                                                 "(SELECT t.transport_pk FROM Service s INNER JOIN Transport t "                  \
                                                 " ON s.transport_fk = t.transport_pk "                                           \
                                                 "WHERE t.original_network_id = ? AND t.transport_id = ? AND s.service_id = ?);"));
                    cmd.bind(1, static_cast<int>(service.getServiceId()));
                    cmd.bind(2, static_cast<int>(sdt.getVersion()));
                    cmd.bind(3, static_cast<int>(sdt.getOriginalNetworkId()));
                    cmd.bind(4, static_cast<int>(sdt.getExtensionId()));
                    cmd.bind(5, static_cast<int>(service.getServiceId()));
                    cmd.execute();
                }

                m_db.sqlCommand(string("DELETE FROM ServiceComponent WHERE fkey NOT IN (SELECT DISTINCT service_pk FROM Service);"));

                {
                    DvbDb::Command cmd(m_db, string("DELETE FROM Event WHERE network_id = ? AND transport_id = ? AND service_id = ?;"));
                    cmd.bind(1, static_cast<int>(sdt.getOriginalNetworkId()));
                    cmd.bind(2, static_cast<int>(sdt.getExtensionId()));
                    cmd.bind(3, static_cast<int>(service.getServiceId()));
                    cmd.execute();
                }

                m_db.sqlCommand(string("DELETE FROM EventItem WHERE event_fk NOT IN (SELECT DISTINCT event_pk FROM Event);"));

                m_db.sqlCommand(string("DELETE FROM EventComponent WHERE fkey NOT IN (SELECT DISTINCT event_pk FROM Event);"));

                service_fk = m_db.findKey(cmdStr, serviceVersion);
            }
        }

        if(service_fk < 1)  // NOT found
        {
            uint8_t serviceType = 0;
            uint16_t lcn = 0;
            string serviceName;
            string providerName;

            const vector<MpegDescriptor>& serveDesc = service.getServiceDescriptors();

            for(auto it = serveDesc.cbegin(); it != serveDesc.cend(); ++it)
            {
                const MpegDescriptor& md = *it;
                if(md.getTag() == DescriptorTag::LOGICAL_CHANNEL)
                {
                    LogicalChannelDescriptor lcd(md);
                    lcn = lcd.getLcn(0);
                }
                if(md.getTag() == DescriptorTag::SERVICE)
                {
                    ServiceDescriptor sd(md);
                    serviceType = sd.getServiceType();
                    serviceName = sd.getServiceName();
                    providerName = sd.getServiceProviderName();
                }
            }

            DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO Service (service_id, transport_fk, version, service_type, " \
                                         "logical_channel_number, running, scrambled, schedule, present_following, "        \
                                         "iso_639_language_code, service_name, provider_name )  VALUES "                    \
                                         "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"));
            cmd.bind(1, static_cast<int>(service.getServiceId()));

            if(transport_fk > 0)
            {
                cmd.bind(2, static_cast<long long int>(transport_fk));
            }
            else
            {
                cmd.bind(2);
            }
            
            cmd.bind(3, static_cast<int>(sdt.getVersion())); 
            cmd.bind(4, static_cast<int>(serviceType));

            if(lcn > 0)
            { 
                cmd.bind(5, static_cast<int>(lcn));
            }
            else
            {
                cmd.bind(5);
            }

            cmd.bind(6, static_cast<int>(service.getRunningStatus())); 
            cmd.bind(7, static_cast<int>(service.isScrambled()));
            cmd.bind(8, static_cast<int>(service.isEitSchedFlagSet()));
            cmd.bind(9, static_cast<int>(service.isEitPfFlagSet()));
            cmd.bind(10);

            if(serviceName.empty())
            {
                cmd.bind(11);
            }
            else
            {
                cmd.bind(11, serviceName);
            }
            
            if(providerName.empty())
            {
                cmd.bind(12);
            }
            else
            {
                cmd.bind(12, providerName);
            }

            cmd.execute(service_fk);

            OS_LOG(DVB_DEBUG, "<%s> Insert service_fk = %lld\n", __FUNCTION__, service_fk);
        }

        if(service_fk > 0)
        {
            m_db.insertComponent("Service", service_fk, service.getServiceDescriptors());
        }
    } // for

    return  service_fk;
}

/**
 * Process Event table for parsed database storage
 *
 * @param eit Eit Table
 * @return foreign key
 */
int64_t  DvbSiStorage::processEvent(const EitTable& eit)
{
    int64_t event_fk = -1;
    const std::vector<DvbEvent>&  eventList = eit.getEvents();

    for(auto it = eventList.begin(); it != eventList.end(); ++it)
    {
        const DvbEvent& event = (*it);
        string cmdStr("SELECT event_pk, version FROM Event WHERE network_id = ");

        std::stringstream ss;
        ss << eit.getNetworkId();
        cmdStr += ss.str();
        cmdStr += " AND transport_id = ";

        ss.str("");
        ss << eit.getTsId();
        cmdStr += ss.str();
        cmdStr += " AND service_id = ";

        ss.str("");
        ss << eit.getExtensionId();
        cmdStr += ss.str();
        cmdStr += " AND event_id = ";

        ss.str("");
        ss << event.getEventId();
        cmdStr += ss.str();
        cmdStr += ";";

        int8_t eventVersion = -1;
        event_fk = m_db.findKey(cmdStr, eventVersion);
        if(event_fk > 0)  // FOUND
        {
            if(eit.getVersion() == eventVersion)
            {
                OS_LOG(DVB_DEBUG, "<%s> EIT version matches (%d). Skipping\n", __FUNCTION__, eit.getVersion());
            }
            else
            {
                OS_LOG(DVB_DEBUG, "<%s> Current version: %d, new version: %d\n", __FUNCTION__, eventVersion, eit.getVersion());

                // Event version change
                DvbDb::Command cmd(m_db, string("DELETE FROM Event WHERE network_id = ? AND transport_id = ? AND event_id = ? AND version != ?;"));
                cmd.bind(1, static_cast<int>(eit.getNetworkId()));
                cmd.bind(2, static_cast<int>(eit.getTsId()));
                cmd.bind(3, static_cast<int>(eit.getExtensionId()));
                cmd.bind(4, static_cast<int>(eit.getVersion()));
                cmd.execute();

                m_db.sqlCommand(string("DELETE FROM EventItem WHERE event_fk NOT IN (SELECT DISTINCT event_pk FROM Event);"));

                m_db.sqlCommand(string("DELETE FROM EventComponent WHERE fkey NOT IN (SELECT DISTINCT event_pk FROM Event);"));

                event_fk = m_db.findKey(cmdStr, eventVersion);
            }
        }

        if(event_fk < 1)  // NOT FOUND
        {
            string parentalRating;
            string content;

            const vector<MpegDescriptor>& eventDesc = event.getEventDescriptors();

            for(auto it = eventDesc.cbegin(); it != eventDesc.cend(); ++it)
            {
                const MpegDescriptor& md = *it;
                if(md.getTag() == DescriptorTag::CONTENT_DESCRIPTOR)
                {
                    ContentDescriptor cd(md);
                    for(int i=0; i < cd.getCount(); i++)
                    {
                        string conStr("(");
                        std::stringstream ss;
                        ss << static_cast<int>(cd.getNibbleLvl1(i));
                        content += ss.str(); 

                        ss << static_cast<int>(cd.getNibbleLvl2(i));
                        content += ss.str(); 

                        ss << static_cast<int>(cd.getUserByte(i));
                        content += ss.str(); 
                        content += ")";
                    }
                }
                if(md.getTag() == DescriptorTag::PARENTAL_RATING)
                {
                    ParentalRatingDescriptor prd(md);
                    for(int i=0; i < prd.getCount(); i++)
                    {
                        parentalRating += "(";
                        parentalRating += prd.getCountryCode(i);
                        parentalRating += " ";

                        std::stringstream ss;
                        ss << static_cast<int>(prd.getRating(i));
                        parentalRating += ss.str();
                        parentalRating += ")";
                    }
                }
            }

            DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO Event (network_id, transport_id, service_id, event_id, " \
                                         "version, start_time, duration, scrambled, running, parental_rating, content) " \
                                         " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"));
            cmd.bind(1, static_cast<int>(eit.getNetworkId()));
            cmd.bind(2, static_cast<int>(eit.getTsId()));
            cmd.bind(3, static_cast<int>(eit.getExtensionId()));
            cmd.bind(4, static_cast<int>(event.getEventId()));
            cmd.bind(5, static_cast<int>(eit.getVersion()));
            cmd.bind(6, static_cast<long long int>(event.getStartTime()));
            cmd.bind(7, static_cast<int>(event.getDuration()));
            cmd.bind(8, static_cast<int>(event.isScrambled()));
            cmd.bind(9, static_cast<int>(event.getRunningStatus()));

            if(parentalRating.empty())
            {
                cmd.bind(10);
            }
            else
            {
                cmd.bind(10, parentalRating);
            }

            if(content.empty())
            {
                cmd.bind(11);
            }
            else
            {
                cmd.bind(11, content);
            }

            cmd.execute(event_fk);

            OS_LOG(DVB_DEBUG, "<%s> Insert event_fk = %lld\n", __FUNCTION__, event_fk);

            if(event_fk > 0)
            {
                processEventItem(event.getEventDescriptors(), event_fk);

                m_db.insertComponent("Event", event_fk, event.getEventDescriptors());
            }
        }
    } 

    return  event_fk;
}

/**
 * Process Event Items for parsed database storage
 *
 * @param descList vector of eit descriptors
 * @param event_fk foreign key to event
 * @return foreign key
 */
int64_t  DvbSiStorage::processEventItem(const vector<MpegDescriptor>& descList, int64_t event_fk)
{
    int64_t eventItem_fk = -1; 
    string iso_639_language_code;
    string title;
    string description;

    for(auto it = descList.cbegin(); it != descList.cend(); ++it)
    {
        const MpegDescriptor& md = *it;
        if(md.getTag() == DescriptorTag::SHORT_EVENT)
        {
            ShortEventDescriptor sed(md);

            iso_639_language_code = sed.getLanguageCode();
            title = sed.getEventName();
            description = sed.getText();

            DvbDb::Command cmd(m_db, string("INSERT OR IGNORE INTO EventItem (event_fk, iso_639_language_code, title, description) " \
                                         "VALUES (?, ?, ?, ?);"));

            cmd.bind(1, (long long int)event_fk);
            if(iso_639_language_code.empty())
            {
                cmd.bind(2);
            }
            else
            {
                cmd.bind(2, iso_639_language_code);
            }
               
            if(title.empty())
            {
                cmd.bind(3);
            }
            else
            {
                cmd.bind(3, title);
            }
               
            if(description.empty())
            {
                cmd.bind(4);
            }
            else
            {
                cmd.bind(4, description);
            }

            cmd.execute(eventItem_fk);

            OS_LOG(DVB_DEBUG, "<%s> Insert eventItem_fk: %lld\n", __FUNCTION__, eventItem_fk);
        }
    }

    return  eventItem_fk; 
}

/**
 * Handle Tot table
 *
 * @param tot Tot table
 */
void DvbSiStorage::handleTotEvent(const TotTable& tot)
{
    if(tot.getTableId() == TableId::TDT)
    {
        OS_LOG(DVB_DEBUG, "<%s> TDT: Time and Date Table\n", __FUNCTION__);

        time_t newTime = tot.getUtcTime();
        if(newTime > 0)
        {
            OS_LOG(DVB_DEBUG, "<%s> newTime = 0x%x\n", __FUNCTION__, newTime);

            struct timeval timeVal = {};
            timeVal.tv_sec = newTime;
            int ret = settimeofday(&timeVal, NULL);
            if(ret < 0)
            {
                OS_LOG(DVB_ERROR, "<%s> settimeofday(0x%x) failed, %s\n", __FUNCTION__, newTime, strerror(errno));
            }
            m_db.setTotReceived(true);
        }
        else
        {
            OS_LOG(DVB_ERROR, "<%s> MjdToDate() failed: %s\n", __FUNCTION__, strerror(errno));
        }
    }
    else if(tot.getTableId() == TableId::TDT)
    {
        OS_LOG(DVB_DEBUG, "<%s> TOT: Time Offset Table\n", __FUNCTION__);
    }

    OS_LOG(DVB_DEBUG, "\tUTC time       : %"PRId64"\n", tot.getUtcTimeBcd());
}

/**
 * Handle SI tables
 *
 * @param tbl si table
 */
void DvbSiStorage::handleTableEvent(const SiTable& tbl)
{
    TableId tableId = tbl.getTableId();

    if((tableId == TableId::NIT) || (tableId == TableId::NIT_OTHER))
    {
        // Only handle NIT tables from preferred Network if specified.
        if(m_preferredNetworkId == 0 || m_preferredNetworkId == tbl.getExtensionId())
        {
            handleNitEvent(static_cast<const NitTable&>(tbl));
        }
    }
    else if((tableId == TableId::SDT) || (tableId == TableId::SDT_OTHER))
    {
        handleSdtEvent(static_cast<const SdtTable&>(tbl));
    }
    else if((tableId >= TableId::EIT_PF) && (tableId <= TableId::EIT_SCHED_OTHER_END))
    {
        handleEitEvent(static_cast<const EitTable&>(tbl));
    }
    else if((tableId == TableId::TDT) || (tableId == TableId::TOT))
    {
        handleTotEvent(static_cast<const TotTable&>(tbl));
    }
    else if(tableId == TableId::BAT)
    {
        handleBatEvent(static_cast<const BatTable&>(tbl));
    }
    else 
    {
        OS_LOG(DVB_ERROR, "<%s> Unknown table id = 0x%x\n", __FUNCTION__, tableId);
    }

}

/**
 * Start scanning
 *
 * @param bFast boolean bFast true fast scan; false background scan
 */
bool DvbSiStorage::startScan(bool bFast)
{
    OS_LOG(DVB_INFO, "%s(%d): called\n", __FUNCTION__, bFast);

    std::lock_guard<std::mutex> lock(m_scanMutex);

    // Check if the scan is already in progress
    if(m_scanStatus.state != DvbScanState::SCAN_STOPPED)
    {
        OS_LOG(DVB_ERROR, "%s(): scan is already in progress\n", __FUNCTION__);
        return false;
    }

    m_scanStatus.state = DvbScanState::SCAN_STARTING;
    m_scanThread = std::thread(&DvbSiStorage::scanThread, this, bFast);

    return true;
}

/**
 * Stop scanning
 */
void DvbSiStorage::stopScan()
{
    OS_LOG(DVB_INFO, "%s(): called\n", __FUNCTION__);

    std::unique_lock<std::mutex> lk(m_scanMutex);

    if(m_scanStatus.state == DvbScanState::SCAN_STOPPED)
    {
        OS_LOG(DVB_INFO, "%s(): scan is already stopped\n", __FUNCTION__);
        return;
    }

    while(m_scanStatus.state != DvbScanState::SCAN_STOPPED)
    {
        OS_LOG(DVB_INFO, "%s(): sending stop request\n", __FUNCTION__);
        lk.unlock();
        m_scanCondition.notify_one();
        sleep(3);
        lk.lock();
    }

    if(m_scanThread.joinable())
    {
        m_scanThread.join();
    }

    OS_LOG(DVB_INFO, "%s(): done\n", __FUNCTION__);
}

/**
 * Scan thread main loop
 *
 * @param bFast boolean true fast scan; false background scan
 */
void DvbSiStorage::scanThread(bool bFast)
{
    OS_LOG(DVB_INFO, "%s(): created(%d)\n", __FUNCTION__, bFast);

    while(true)
    {
        if(bFast)
        {
            m_scanStatus.state = DvbScanState::SCAN_IN_PROGRESS_FAST;

            if(!scanFast())
            {
                OS_LOG(DVB_ERROR, "%s(): scanFast() failed\n", __FUNCTION__);
                m_scanStatus.state = DvbScanState::SCAN_FAILED;
            }
            else
            {
                // It's enough to run the fast scan only once
                bFast = false;
                continue;
            }
        }
        else
        {
            m_scanStatus.state = DvbScanState::SCAN_IN_PROGRESS_BKGD;
            if(!scanBackground())
            {
                OS_LOG(DVB_ERROR, "%s(): scanBackground() failed\n", __FUNCTION__);
                m_scanStatus.state = DvbScanState::SCAN_FAILED;
            }
            else
            {
                OS_LOG(DVB_INFO, "%s(): scan completed successfully\n", __FUNCTION__);
                m_scanStatus.state = DvbScanState::SCAN_COMPLETED;
            }
        }

        // TODO: Enable audits
        //m_db.audits();

        // Let's check if we need to stop the scan
        {
            std::unique_lock<std::mutex> lk(m_scanMutex);
            // TODO: Handle spurious wake-ups
            if(m_scanCondition.wait_for(lk, std::chrono::seconds(bFast ? 30 : m_bkgdScanInterval)) == std::cv_status::no_timeout)
            {
                OS_LOG(DVB_INFO, "%s(): stopping\n", __FUNCTION__);
                m_scanStatus.state = DvbScanState::SCAN_STOPPED;
                return;
            }
        }
    }

}

/**
 * Scan Home transport stream
 */
bool DvbSiStorage::scanHome()
{
    DvbTsStatus status = {};

    // Let's start over clean slate
    clearCachedTables();

    shared_ptr<DvbTuner> tuner = DvbTuner::createTuner();
    if(!tuner)
    {
        OS_LOG(DVB_ERROR, "%s(): Unable to create tuner\n", __FUNCTION__);
        return false;
    }

    OS_LOG(DVB_INFO, "%s:%d: tuning to home ts(%d)\n", __FUNCTION__, __LINE__, m_homeFrequency);
    int32_t ret = tuner->tune(m_homeFrequency, m_homeModulation, m_homeSymbolRate);
    if(ret != 0)
    {
        OS_LOG(DVB_ERROR, "%s(): tune(%d) failed with 0x%x\n", __FUNCTION__, m_homeFrequency, ret);
        // scan failed
        return false;
    }

    vector<shared_ptr<SiTable>> tables;

    // NIT
    tables.emplace_back(new NitTable((uint8_t)TableId::NIT, m_preferredNetworkId, 0, true));

    // BAT(s)
    for(auto it = m_homeBouquets.begin(), end = m_homeBouquets.end(); it != end; ++it)
    {
        OS_LOG(DVB_DEBUG, "%s:%d: Adding BAT(0x%x) to the list\n", __FUNCTION__, __LINE__, *it);
        tables.emplace_back(new BatTable((uint8_t)TableId::BAT, *it, 0, true));
    }

    OS_LOG(DVB_INFO, "%s:%d: Collecting NIT & BAT(s)\n", __FUNCTION__, __LINE__);
    if(checkTables(tables, NIT_TIMEOUT > BAT_TIMEOUT ? NIT_TIMEOUT : BAT_TIMEOUT))
    {
        OS_LOG(DVB_INFO, "%s:%d: NIT & BAT(s) found\n", __FUNCTION__, __LINE__);
        status.nitRcvd = true;
        status.batRcvd = true;
    }
    else
    {
        OS_LOG(DVB_ERROR, "%s:%d: NIT & BAT(s) not found\n", __FUNCTION__, __LINE__);
        return false;
    }

    tables.clear();

    // Collect SDT & EIT pf(optional)
    vector<shared_ptr<DvbStorage::TransportStream_t>> tsList = getTsListByNetIdCache(m_preferredNetworkId);
    for(auto it = tsList.begin(), end = tsList.end(); it != end; ++it)
    {
        if(!m_isFastScanSmart)
        {
            if(m_homeFrequency == (*it)->frequency)
            {
                OS_LOG(DVB_DEBUG, "%s:%d: Adding SDT(0x%x.0x%x) actual to the list\n",
                        __FUNCTION__, __LINE__, (*it)->networkId, (*it)->tsId);
                SdtTable* sdt = new SdtTable((uint8_t)TableId::SDT, (*it)->tsId, 0, true);
                sdt->setOriginalNetworkId((*it)->networkId);
                tables.emplace_back(sdt);
                break;
            }
        }
        else
        {
            OS_LOG(DVB_DEBUG, "%s:%d: Adding SDT(0x%x.0x%x) to the list\n",
                    __FUNCTION__, __LINE__, (*it)->networkId, (*it)->tsId);
            SdtTable* sdt = new SdtTable((uint8_t)TableId::SDT, (*it)->tsId, 0, true);
            sdt->setOriginalNetworkId((*it)->networkId);
            tables.emplace_back(sdt);
        }
    }

    if(checkTables(tables, m_isFastScanSmart ? SDT_OTHER_TIMEOUT : SDT_TIMEOUT))
    {
        OS_LOG(DVB_INFO, "%s:%d: All SDTs received\n", __FUNCTION__, __LINE__);
        status.sdtRcvd = true;
    }
    else
    {
        OS_LOG(DVB_ERROR, "%s:%d: All SDTs not received\n", __FUNCTION__, __LINE__);
    }
   
    m_scanStatus.tsList.emplace_back(m_homeFrequency, status);

    return true;
}

/**
 * Scan fast
 */
bool DvbSiStorage::scanFast()
{
    OS_LOG(DVB_INFO, "%s: Started\n", __FUNCTION__);

    m_scanStatus.tsList.clear();

    if(!scanHome())
    {
        OS_LOG(DVB_ERROR, "%s: scanHome() failed\n", __FUNCTION__);
        return false;
    }

    shared_ptr<DvbTuner> tuner = DvbTuner::createTuner();
    if(!tuner)
    {
        OS_LOG(DVB_ERROR, "%s(): Unable to create tuner\n", __FUNCTION__);
        return false;
    }

    vector<shared_ptr<DvbStorage::TransportStream_t>> tsList = getTsListByNetIdCache(m_preferredNetworkId);
    for(auto it = tsList.begin(), end = tsList.end(); it != end; ++it)
    {
        DvbTsStatus status = {};

        // collect SDTa & EITa pf
        SdtTable* sdt = new SdtTable((uint8_t)TableId::SDT, (*it)->tsId, 0, true);
        sdt->setOriginalNetworkId((*it)->networkId);

        vector<shared_ptr<SiTable>> tables;
        tables.emplace_back(sdt);

        vector<shared_ptr<DvbStorage::Service_t>> serviceList = getServiceListByTsIdCache((*it)->networkId, (*it)->tsId);
        for(auto srv = serviceList.begin(), end = serviceList.end(); srv != end; ++srv)
        {
            OS_LOG(DVB_DEBUG, "%s:%d: Adding EITpf(0x%x.0x%x.0x%x) to the list\n",
                    __FUNCTION__, __LINE__, (*it)->networkId, (*it)->tsId, (*srv)->serviceId);

            EitTable* eit = new EitTable((uint8_t)TableId::EIT_PF, (*srv)->serviceId, 0, true);
            eit->setNetworkId((*it)->networkId);
            eit->setTsId((*it)->tsId);
            tables.emplace_back(eit);
        }

        if(m_isFastScanSmart && checkTables(tables, 0))
        {
            OS_LOG(DVB_INFO, "%s:%d: SDTa(0x%x.0x%x) & EITa pf already received. Skipping.\n",
                    __FUNCTION__, __LINE__, (*it)->networkId, sdt->getExtensionId());
            status.sdtRcvd = true;
            status.eitPfRcvd = true;
        }
        else
        {
            OS_LOG(DVB_INFO, "%s:%d: tune(%d)\n", __FUNCTION__, __LINE__, (*it)->frequency);
            int32_t ret = tuner->tune((*it)->frequency, (*it)->modulation, (*it)->symbolRate);
            if(ret != 0)
            {
                OS_LOG(DVB_ERROR, "%s(): tune(%d) failed with 0x%x\n", __FUNCTION__, (*it)->frequency, ret);
            }

            OS_LOG(DVB_INFO, "%s:%d: Collecting SDTa & EITs pf\n", __FUNCTION__, __LINE__);
            if(checkTables(tables, SDT_TIMEOUT > EIT_PF_TIMEOUT ? SDT_TIMEOUT : EIT_PF_TIMEOUT))
            {
                OS_LOG(DVB_INFO, "%s:%d: SDT(0x%x.0x%x) & EITs pf received\n",
                        __FUNCTION__, __LINE__, (*it)->networkId, sdt->getExtensionId());
                status.sdtRcvd = true;
                status.eitPfRcvd = true;
            }
            else
            {
                OS_LOG(DVB_ERROR, "%s:%d: SDT(0x%x.0x%x) and/or EITs pf not received\n",
                        __FUNCTION__, __LINE__, (*it)->networkId, sdt->getExtensionId());
            }

            OS_LOG(DVB_INFO, "%s:%d: untune(%d)\n", __FUNCTION__, __LINE__, (*it)->frequency);
            tuner->untune();
        }

        m_scanStatus.tsList.emplace_back((*it)->frequency, status);
    }

    OS_LOG(DVB_ERROR, "%s:%d: Done\n", __FUNCTION__, __LINE__);

    return true;
}

/**
 * Scan background
 */
bool DvbSiStorage::scanBackground()
{
    OS_LOG(DVB_INFO, "%s: Started\n", __FUNCTION__);

    m_scanStatus.tsList.clear();

    if(!scanHome())
    {
        OS_LOG(DVB_ERROR, "%s: scanHome() failed\n", __FUNCTION__);
        return false;
    }

    shared_ptr<DvbTuner> tuner = DvbTuner::createTuner();
    if(!tuner)
    {
        OS_LOG(DVB_ERROR, "%s(): Unable to create tuner\n", __FUNCTION__);
        return false;
    }

    OS_LOG(DVB_INFO, "%s:%d: tuning to home ts(%d)\n", __FUNCTION__, __LINE__, m_homeFrequency);
    vector<shared_ptr<SiTable>> fullEitSchedule;

    vector<shared_ptr<DvbStorage::TransportStream_t>> tsList = getTsListByNetIdCache(getPreferredNetworkId());
    for(auto it = tsList.begin(), end = tsList.end(); it != end; ++it)
    {
        DvbTsStatus status = {};

        OS_LOG(DVB_INFO, "%s:%d: tune(%d)\n", __FUNCTION__, __LINE__, (*it)->frequency);
        int32_t ret = tuner->tune((*it)->frequency, (*it)->modulation, (*it)->symbolRate);
        if(ret != 0)
        {
            OS_LOG(DVB_ERROR, "%s(): tune(%d) failed with 0x%x\n", __FUNCTION__, (*it)->frequency, ret);
        }

        // collect SDTa & EITa pf
        // SDTa
        SdtTable* sdt = new SdtTable((uint8_t)TableId::SDT, (*it)->tsId, 0, true);
        sdt->setOriginalNetworkId((*it)->networkId);

        vector<shared_ptr<SiTable>> tables;
        vector<shared_ptr<SiTable>> eitSchedule;
        tables.emplace_back(sdt);

        OS_LOG(DVB_INFO, "%s:%d: Collecting SDTa(0x%x.0x%x)\n",
                __FUNCTION__, __LINE__, (*it)->networkId, sdt->getExtensionId());
        if(checkTables(tables, SDT_TIMEOUT))
        {
            OS_LOG(DVB_INFO, "%s:%d: SDTa(0x%x.0x%x) received\n",
                    __FUNCTION__, __LINE__, (*it)->networkId, sdt->getExtensionId());
            status.sdtRcvd = true;
        }
        else
        {
            OS_LOG(DVB_ERROR, "%s:%d: SDTa(0x%x.0x%x) not received\n",
                    __FUNCTION__, __LINE__, (*it)->networkId, sdt->getExtensionId());
        }

        tables.clear();

        //EITa shed & EITa pf
        vector<shared_ptr<DvbStorage::Service_t>> serviceList = getServiceListByTsIdCache((*it)->networkId, (*it)->tsId);
        for(auto srv = serviceList.begin(), end = serviceList.end(); srv != end; ++srv)
        {
            OS_LOG(DVB_DEBUG, "%s:%d: Adding EITsched(0x%x.0x%x.0x%x) to the list\n",
                                    __FUNCTION__, __LINE__, (*it)->networkId, (*it)->tsId, (*srv)->serviceId);

            EitTable* eitSched = new EitTable((uint8_t)TableId::EIT_SCHED_START, (*srv)->serviceId, 0, true);
            eitSched->setNetworkId((*it)->networkId);
            eitSched->setTsId((*it)->tsId);
            eitSchedule.emplace_back(eitSched);

            EitTable* eit = new EitTable((uint8_t)TableId::EIT_PF, (*srv)->serviceId, 0, true);
            eit->setNetworkId((*it)->networkId);
            eit->setTsId((*it)->tsId);
            tables.emplace_back(eit);
        }

        fullEitSchedule.insert(fullEitSchedule.end(), eitSchedule.begin(), eitSchedule.end());

        OS_LOG(DVB_INFO, "%s:%d: Collecting EITa pf\n", __FUNCTION__, __LINE__);
        if(checkTables(tables, EIT_PF_TIMEOUT))
        {
            OS_LOG(DVB_INFO, "%s:%d: EITa pf received\n",
                    __FUNCTION__, __LINE__);
            status.eitPfRcvd = true;
        }
        else
        {
            OS_LOG(DVB_ERROR, "%s:%d: EITa pf not received\n",
                    __FUNCTION__, __LINE__);
        }

        if((*it)->frequency != m_barkerFrequency)
        {
            OS_LOG(DVB_INFO, "%s:%d: Collecting EITa sched\n", __FUNCTION__, __LINE__);
            if(checkTables(eitSchedule, EIT_8_DAY_SCHED_TIMEOUT))
            {
                OS_LOG(DVB_INFO, "%s:%d: EITsched received\n", __FUNCTION__, __LINE__);
                status.eitRcvd = true;
            }
            else
            {
                OS_LOG(DVB_ERROR, "%s:%d: EITsched not received\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            OS_LOG(DVB_INFO, "%s:%d: Not collecting EITa sched on barker(%d)\n",
                    __FUNCTION__, __LINE__, (*it)->frequency);
        }

        OS_LOG(DVB_INFO, "%s:%d: untune(%d)\n", __FUNCTION__, __LINE__, (*it)->frequency);
        tuner->untune();

        m_scanStatus.tsList.emplace_back((*it)->frequency, status);
    }

    // Sitting on barker ts
    if(m_barkerFrequency && m_barkerModulation && m_barkerSymbolRate)
    {
        DvbTsStatus status = {};
        // Clear cached EIT tables
        {
            OS_LOG(DVB_DEBUG, "%s:%d: Clearing cached EIT tables\n", __FUNCTION__, __LINE__);
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_eitMap.clear();
        }

        OS_LOG(DVB_INFO, "%s:%d: tune(%d) barker\n", __FUNCTION__, __LINE__, m_barkerFrequency);
        int32_t ret = tuner->tune(m_barkerFrequency, m_barkerModulation, m_barkerSymbolRate);
        if(ret != 0)
        {
            OS_LOG(DVB_ERROR, "%s(): tune(%d) failed with 0x%x\n", __FUNCTION__, m_barkerFrequency, ret);
        }


        OS_LOG(DVB_INFO, "%s:%d: Collecting EIT sched (barker)\n", __FUNCTION__, __LINE__);
        if(checkTables(fullEitSchedule, m_barkerEitTimeout))
        {
            OS_LOG(DVB_INFO, "%s:%d: EITsched received (barker)\n", __FUNCTION__, __LINE__);
            status.eitRcvd = true;
        }
        else
        {
            OS_LOG(DVB_ERROR, "%s:%d: EITsched not received (barker)\n", __FUNCTION__, __LINE__);
        }

        OS_LOG(DVB_INFO, "%s:%d: untune(%d)\n", __FUNCTION__, __LINE__, m_barkerFrequency);
        tuner->untune();

        m_scanStatus.tsList.emplace_back(m_barkerFrequency, status);
    }

    OS_LOG(DVB_INFO, "%s:%d: Done\n", __FUNCTION__, __LINE__);

    return true;
}

/**
 * Check cache collections
 */
bool DvbSiStorage::checkTables(vector<shared_ptr<SiTable>>& tables, int timeout)
{
    do
    {
        bool found = true;
        for(auto tbl = tables.begin(), end = tables.end(); tbl != end; ++tbl)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            TableId tableId = (*tbl)->getTableId();
            if((tableId == TableId::NIT) || (tableId == TableId::NIT_OTHER))
            {
                OS_LOG(DVB_DEBUG, "%s:%d: Looking for NIT(0x%x)\n",
                        __FUNCTION__, __LINE__, (*tbl)->getExtensionId());
                auto it = m_nitMap.find((*tbl)->getExtensionId());
                if(it == m_nitMap.end())
                {
                    found = false;
                    break;
                }
                else
                {
                    OS_LOG(DVB_DEBUG, "%s:%d: NIT(0x%x) found\n",
                            __FUNCTION__, __LINE__, (*tbl)->getExtensionId());
                }
            }
            else if(tableId == TableId::BAT)
            {
                OS_LOG(DVB_DEBUG, "%s:%d: Looking for BAT(0x%x)\n",
                        __FUNCTION__, __LINE__, (*tbl)->getExtensionId());
                auto it = m_batMap.find((*tbl)->getExtensionId());
                if(it == m_batMap.end())
                {
                    found = false;
                    break;
                }
                else
                {
                    OS_LOG(DVB_DEBUG, "%s:%d: BAT(0x%x) found\n",
                            __FUNCTION__, __LINE__, (*tbl)->getExtensionId());
                }
            }
            else if((tableId == TableId::SDT) || (tableId == TableId::SDT_OTHER))
            {
                shared_ptr<SdtTable> sdt = std::static_pointer_cast<SdtTable>((*tbl));

                OS_LOG(DVB_DEBUG, "%s:%d: Looking for SDT(0x%x.0x%x)\n",
                        __FUNCTION__, __LINE__, sdt->getOriginalNetworkId(), sdt->getExtensionId());

                pair<uint16_t, uint16_t> key(sdt->getOriginalNetworkId(), sdt->getExtensionId());
                auto it = m_sdtMap.find(key);
                if(it == m_sdtMap.end())
                {
                    found = false;
                    break;
                }
                else
                {
                    OS_LOG(DVB_DEBUG, "%s:%d: SDT(0x%x.0x%x) found\n",
                            __FUNCTION__, __LINE__, sdt->getOriginalNetworkId(), sdt->getExtensionId());
                }
            }
            else if((tableId >= TableId::EIT_PF) && (tableId <= TableId::EIT_SCHED_OTHER_END))
            {
                bool isPf = false;
                if(tableId == TableId::EIT_PF || tableId == TableId::EIT_PF_OTHER)
                {
                    isPf = true;
                }

                shared_ptr<EitTable> eit = std::static_pointer_cast<EitTable>((*tbl));

                OS_LOG(DVB_DEBUG, "%s:%d: Looking for EIT(0x%x.0x%x.0x%x) isPF: %d\n",
                        __FUNCTION__, __LINE__, eit->getNetworkId(), eit->getTsId(), eit->getExtensionId(), isPf);

                tuple<uint16_t, uint16_t, uint16_t, bool> key(eit->getNetworkId(), eit->getTsId(), eit->getExtensionId(), isPf);
                auto it = m_eitMap.find(key);
                if(it == m_eitMap.end())
                {
                    found = false;
                    break;
                }
                else
                {
                    OS_LOG(DVB_DEBUG, "%s:%d: Looking for EIT(0x%x.0x%x.0x%x) isPF: %d found\n",
                            __FUNCTION__, __LINE__, eit->getNetworkId(), eit->getTsId(), eit->getExtensionId(), isPf);
                }
            }
        }

        if(found)
        {
            return true;
        }
        
        // TODO: Consider using m_scanCondition.wait_for() here
        sleep(1);
    }
    while(timeout-- > 0);

    return false;
}

/**
 * Clear out cach collections
 */
void DvbSiStorage::clearCachedTables()
{
    std::lock_guard<std::mutex> lock(m_dataMutex);

    OS_LOG(DVB_INFO, "%s:%d: Clearing cached tables\n", __FUNCTION__, __LINE__);
    m_nitMap.clear();
    m_sdtMap.clear();
    m_eitMap.clear();
    m_batMap.clear();
}

/**
 * Return scanner status
 */
DvbSiStorage::DvbScanStatus DvbSiStorage::getScanStatus()
{
    OS_LOG(DVB_INFO, "%s: m_scanStatus.tsList.size() = %d\n", __FUNCTION__, __LINE__, m_scanStatus.tsList.size());
    return m_scanStatus;
}

