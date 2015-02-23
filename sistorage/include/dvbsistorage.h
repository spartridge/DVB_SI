// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef __DVBSISTORAGE_H__
#define __DVBSISTORAGE_H__

// C system includes

// C++ system includes
#include <mutex>
#include <map>
#include <utility>
#include <vector>
#include <string>
#include <tuple>
#include <memory>
#include <thread>
#include <condition_variable>

// Other libraries' includes

// Project's includes
#include "dvbdb.h"
#include "dvbtuner.h"

/**
 * DvbStorage namespace
 */
namespace DvbStorage
{
    /**
     * TransportStream structure. Used to represent a Transport Stream from storage collections.
     */
    typedef struct TransportStream
    {
        /**
         * Constructor
         *
         * @param fr frequency
         * @param mod modulation
         * @param sym symbol rate
         * @param net network id
         * @param ts transport stream id
         */
        TransportStream(uint32_t fr,
                DvbModulationMode mod,
                uint32_t sym,
                uint16_t net,
                uint16_t ts)
          : frequency(fr),
            modulation(mod),
            symbolRate (sym),
            networkId(net),
            tsId(ts)
        {};
        uint32_t frequency;
        DvbModulationMode modulation;
        uint32_t symbolRate;
        uint16_t networkId;
        uint16_t tsId;
    } TransportStream_t;

    /**
     * Service structure. Used to represent a Service from the storage collections.
     */
    typedef struct Service
    {
        /**
         * Constructor
         *
         * @param net network id
         * @param ts transport stream id
         * @param id service id
         * @param name service name
         */
        Service(uint16_t net,
                uint16_t ts,
                uint16_t id,
                std::string name)
          : networkId(net),
            tsId(ts),
            serviceId(id),
            serviceName(std::move(name))
        {};
        uint16_t networkId;
        uint16_t tsId;
        uint16_t serviceId;
        std::string serviceName;
    } Service_t;

    /**
     * Event structure. Used to represent an Event from the storage collections.
     */
    typedef struct Event
    {
        uint16_t networkId;
        uint16_t tsId;
        uint16_t serviceId;
        uint16_t eventId;
        uint64_t startTime;
        uint32_t duration;
        std::string name;
        std::string text;
    } Event_t;
}

// Forward declarations
class SiTable;
class NitTable;
class SdtTable;
class EitTable;
class TotTable;
class BatTable;
class TransportStream;

/**
 * DvbSiStorage class. Main controller class for collecting and storing DVB SI data.
 */
class DvbSiStorage
{
public:
    /** 
     * Default Constructor
     */
    DvbSiStorage();

    /**
     * Destructor
     */
    ~DvbSiStorage() {};

    /**
     * Return a vector of Transport streams
     *
     * @param nId network id
     * @return vector of shared pointers of TransportStream structures
     */
    std::vector<std::shared_ptr<DvbStorage::TransportStream_t>> getTsListByNetId(uint16_t nId = 0);

    /**
     * Return a vector of Transport streams
     *
     * @param nId network id
     * @return vector of shared pointers of TransportStream structures
     */
    std::vector<std::shared_ptr<DvbStorage::TransportStream_t>> getTsListByNetIdCache(uint16_t nId = 0);

    /**
     * Return a vector of Events 
     *
     * @param nId network id
     * @param tsId transport stream id
     * @param sId service id
     * @return vector of shared pointers of Event_t structures
     */
    std::vector<std::shared_ptr<DvbStorage::Event_t>> getEventListByServiceId(uint16_t nId, uint16_t tsId, uint16_t sId);

    /**
     * Return a vector of Events 
     *
     * @param nId network id
     * @param tsId transport stream id
     * @param sId service id
     * @return vector of shared pointers of Event_t structures
     */
    std::vector<std::shared_ptr<DvbStorage::Event_t>> getEventListByServiceIdCache(uint16_t nId, uint16_t tsId, uint16_t sId);

    /**
     * Return a vector of Service_t structures
     *
     * @param nId network id
     * @param tsId transport stream id
     * @return vector of shared pointers of Service_t structures
     */
    std::vector<std::shared_ptr<DvbStorage::Service_t>> getServiceListByTsId(uint16_t nId, uint16_t tsId);

    /**
     * Return a vector of Service_t structures
     *
     * @param nId network id
     * @param tsId transport stream id
     * @return vector of shared pointers of Service_t structures
     */
    std::vector<std::shared_ptr<DvbStorage::Service_t>> getServiceListByTsIdCache(uint16_t nId, uint16_t tsId);

    /**
     * Dvb scanner states
     */ 
    enum class DvbScanState
    {
        SCAN_STOPPED,
        SCAN_STARTING,
        SCAN_IN_PROGRESS_FAST,
        SCAN_IN_PROGRESS_BKGD,
        SCAN_COMPLETED,
        SCAN_FAILED
    };

    /**
     * Transport Stream SI table status
     */ 
    struct DvbTsStatus
    {
        bool nitRcvd;
        bool batRcvd;
        bool sdtRcvd;
        bool eitPfRcvd;
        bool eitRcvd;
    };

    /**
     * Dvb scanner status
     */ 
    struct DvbScanStatus
    {
        DvbScanState state;
        std::vector<std::pair<uint32_t, DvbTsStatus>> tsList;
    };

    /**
     * Return scanner status
     */
    DvbScanStatus getScanStatus();

    /**
     * Get preferred network id value
     */
    uint16_t getPreferredNetworkId() const
    {
        return m_preferredNetworkId;
    }

    /**
     * Set preferred network id value
     */
    void setPreferredNetworkId(uint16_t id)
    {
        m_preferredNetworkId = id;
    }

    /**
     * Handle SI tables
     *
     * @param tbl si table
     */
    void handleTableEvent(const SiTable& tbl);

private:
    /** 
     * Copy Constructor
     */
    DvbSiStorage(const DvbSiStorage& other)  = delete;

    /**
     * Assignment operator
     */
    DvbSiStorage& operator=(const DvbSiStorage&)  = delete;

    /**
     * Handle Nit table
     *
     * @param nit Nit table
     */
    void handleNitEvent(const NitTable& nit);

    /**
     * Handle Sdt table
     *
     * @param sdt Sdt table
     */
    void handleSdtEvent(const SdtTable& sdt);

    /**
     * Handle Eit table
     *
     * @param eit Eit table
     */
    void handleEitEvent(const EitTable& eit);

    /**
     * Handle Tot table
     *
     * @param tot Tot table
     */
    void handleTotEvent(const TotTable& tot);

    /**
     * Handle Bat table
     *
     * @param bat Bat table
     */
    void handleBatEvent(const BatTable& tot);

    /**
     * Process Nit table for cache storage
     *
     * @param nit Nit table
     */
    void processNitEventCache(const NitTable& nit);

    /**
     * Process Sdt table for cache storage
     *
     * @param sdt Sdt table
     */
    void processSdtEventCache(const SdtTable& sdt);

    /**
     * Process Eit table for cache storage
     *
     * @param eit Eit table
     */
    void processEitEventCache(const EitTable& eit);

    /**
     * Process Bat table for cache storage
     *
     * @param bat Bat table
     */
    void processBatEventCache(const BatTable& bat);

    /**
     * Process Nit table for database storage
     *
     * @param nit Nit table
     */
    void processNitEventDb(const NitTable& nit);

    /**
     * Process Sdt table for database storage
     *
     * @param sdt Sdt table
     */
    void processSdtEventDb(const SdtTable& sdt);

    /**
     * Process Eit table for database storage
     *
     * @param eit Eit table
     */
    void processEitEventDb(const EitTable& eit);

    /**
     * Process Bat table for database storage
     *
     * @param bat Bat table
     */
    void processBatEventDb(const BatTable& bat);

    /**
     * Process Nit table for parsed database storage
     *
     * @param nit Nit table
     * @return foreign key
     */
    int64_t processNetwork(const NitTable& nit);

    /**
     * Process Bat table for parsed database storage
     *
     * @param bat Bat table
     * @return foreign key
     */
    int64_t processBouquet(const BatTable& bat);

    /**
     * Process Transport table for parsed database storage
     *
     * @param ts TransportStream reference
     * @param network_fk foreign key value of Network entry
     * @return foreign key
     */
    int64_t processTransport(const TransportStream& ts, int64_t network_fk);

    /**
     * Process Service table for parsed database storage
     *
     * @param sdt Sdt Table
     * @return foreign key
     */
    int64_t processService(const SdtTable& sdt);

    /**
     * Process Event table for parsed database storage
     *
     * @param eit Eit Table
     * @return foreign key
     */
    int64_t processEvent(const EitTable& eit);

    /**
     * Process Event Items for parsed database storage
     *
     * @param descList vector of eit descriptors
     * @param event_fk foreign key to event
     * @return foreign key
     */
    int64_t processEventItem(const std::vector<MpegDescriptor>& descList, int64_t event_fk);

    /**
     * Start function for monitor thread
     */
    void startMonitorThread();

    /**
     * Stop function for monitor thread
     */
    void stopMonitorThread();

    // DvbScan methods
    /**
     * Start scanning
     *
     * @param bFast boolean bFast true fast scan; false background scan
     */
    bool startScan(bool bFast);

    /**
     * Stop scanning
     */
    void stopScan();

    /**
     * Scan thread main loop
     *
     * @param bFast boolean true fast scan; false background scan
     */
    void scanThread(bool bFast);

    /**
     * Scan Home transport stream
     */
    bool scanHome();

    /**
     * Scan fast
     */
    bool scanFast();

    /**
     * Scan background
     */
    bool scanBackground();

    /**
     * Check cache collections
     */
    bool checkTables(std::vector<std::shared_ptr<SiTable>>& tables, int timeout);

    /**
     * Load environmental runtime settings
     *
     * @return bool true no change in settings; false settings have changed
     */
    bool loadSettings();

    /**
     * Clear out cach collections
     */
    void clearCachedTables();

    /**
     *  DvbDb database object
     */
    DvbDb m_db;

    /** 
     * Nit map collection
     *
     * key: nid
     */
    std::map<uint16_t, std::shared_ptr<NitTable>> m_nitMap;

    /** 
     * Sdt map collection
     *
     * key: onid, tsid
     */
    std::map<std::pair<uint16_t, uint16_t>, std::shared_ptr<SdtTable>> m_sdtMap;

    /** 
     * Sdt map collection
     *
     * key: onid, tsid, sid, pf
     */
    std::map<std::tuple<uint16_t, uint16_t, uint16_t, bool>, std::shared_ptr<EitTable>> m_eitMap;

    /** 
     * Sdt map collection
     *
     * key: bid
     */
    std::map<uint16_t, std::shared_ptr<BatTable>> m_batMap;

    /** 
     * Mutex to guard cache collections
     */
    std::mutex m_dataMutex;

    // DvbScan timeout values
    enum
    {
        NIT_TIMEOUT = 15,
        NIT_OTHER_TIMEOUT = 15,
        SDT_TIMEOUT = 5,
        SDT_OTHER_TIMEOUT = 15,
        BAT_TIMEOUT = 15,
        EIT_PF_TIMEOUT = 5,
        EIT_PF_OTHER_TIMEOUT = 15,
        EIT_8_DAY_SCHED_TIMEOUT = 15,
        EIT_PAST_8_DAY_SCHED_TIMEOUT = 60,
    };

    /** 
     * Preferred Network Id
     */
    uint16_t m_preferredNetworkId;

    /** 
     * Fast scan flag
     */
    bool m_isFastScanSmart;

    /** 
     * Background wait interval
     */
    uint32_t m_bkgdScanInterval;

    // Home TS data members
    /** 
     * Home frequency
     */
    uint32_t m_homeFrequency;

    /** 
     * Home Modulation enumeration
     */
    DvbModulationMode m_homeModulation;

    /** 
     * Home symbol rate
     */
    uint32_t m_homeSymbolRate;

    /** 
     * Home bouquets
     */
    std::vector<uint16_t> m_homeBouquets;

    // Barker TS data members
    /** 
     * Barker Frequency
     */
    uint32_t m_barkerFrequency;

    /** 
     * Barker modulation enumeration
     */
    DvbModulationMode m_barkerModulation;

    /** 
     * Barker symbol rate
     */
    uint32_t m_barkerSymbolRate;

    /** 
     * Barker Eit timeout value
     */
    uint32_t m_barkerEitTimeout;

    /** 
     * Thread object for scanning
     */
    std::thread m_scanThread;

    /** 
     * Scan status member varible
     */
    DvbScanStatus m_scanStatus;

    /** 
     * Conditional wait object
     */
    std::condition_variable m_scanCondition;

    /** 
     * Mutex to guard scanning operations
     */
    std::mutex m_scanMutex;
};

#endif
