#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <map>

#include "AGHUtils/ReadingClass.h"
#include "AGHUtils/WritingClass.h"

#include "AGHConfig/ConfigFrame.h"
#include "AGHConfig/ConfigSignal.h"
#include "AGHConfig/ConfigTrigger.h"

class Config : public WritableToBin, public ReadableFromBin {
    friend class ConfigFrame;
public:
    enum class EnCANBitrate {
        bitrate_50kbps  = 50,
        bitrate_125kbps = 125,
        bitrate_250kbps = 250,
        bitrate_500kbps = 500,
        bitrate_1Mbps   = 1000,
    };
    enum class EnGPSFrequency {
        freq_GPS_OFF = 0,
        freq_0_5_Hz  = 1,
        freq_1_Hz    = 2,
        freq_2_Hz    = 3,
        freq_5_Hz    = 4,
        freq_10_Hz   = 5
    };
    static constexpr unsigned int   ACTUAL_VERSION          = 0;
    static constexpr unsigned int   ACTUAL_SUB_VERSION      = 4;

    using FramesIterator        = std::vector<ConfigFrame*>::iterator;
    using ConstFramesIterator   = std::vector<ConfigFrame*>::const_iterator;

    using TriggersIterator      = std::vector<ConfigTrigger*>::iterator;
    using ConstTriggersIterator = std::vector<ConfigTrigger*>::const_iterator;
private:
    static constexpr EnCANBitrate   DEFAULT_CAN_BITRATE     = EnCANBitrate::bitrate_500kbps;
    static constexpr EnGPSFrequency DEFAULT_GPS_FREQUENCY   = EnGPSFrequency::freq_10_Hz;
    static constexpr unsigned int   DEFAULT_RTC_CONFIGURATION_FRAME_ID = 0x7FF;

    static constexpr unsigned int   START_CONFIG_TRIGGERS_MAX_NUMBER    = 5U;
    static constexpr unsigned int   STOP_CONFIG_TRIGGERS_MAX_NUMBER     = 5U;

    static constexpr unsigned int   MAX_FRAMES_NUMBER = (14U * 4U);
    static constexpr unsigned int   MAX_SIGNALS_NUMBER = 256U;

    static constexpr int            MIN_MAX_TIME_ZONE_SHIFT_30_MINS_UNIT = (14 * 2);

    static constexpr unsigned int   CONFIG_NAME_LENGTH = 20U;

    Config();

    void                                        addFrame(ConfigFrame *pFrame);
    void                                        addStartTrigger(ConfigTrigger* pTrigger);
    void                                        addStopTrigger(ConfigTrigger* pTrigger);
    std::vector<ConfigFrame*>::const_iterator   lowerBoundFrameConstIterator(unsigned int frameID) const;
    std::vector<ConfigFrame*>::iterator         lowerBoundFrameIterator(unsigned int frameID);
    void                                        sortFramesCallback();
    bool                                        isMaxSignalsNumber() const;

    unsigned int                version;
    unsigned int                subVersion;
    std::string                 logFileName;
    EnCANBitrate                canBitrate;
    EnGPSFrequency              gpsFrequency;
    unsigned int                rtcConfigurationFrameID;
    bool                        useDateAndTimeFromGPS;
    int                         timeZoneShift_30minsUnit;

    std::vector <ConfigFrame*>  framesVector;

    std::vector<ConfigTrigger*> startConfigTriggers;
    std::vector<ConfigTrigger*> stopConfigTriggers;

public:

    Config(
        std::string logFileName,
        EnCANBitrate canBitrate,
        EnGPSFrequency gpsFrequency,
        unsigned int rtcConfigurationFrameID,
        bool _useDateAndTimeFromGPS,
        double _timeZoneShift
    );
    Config(ReadingClass& reader);

    //<----- Access to preambule data ----->/
    void                    setVersion(unsigned int sVersion);
    void                    setSubVersion(unsigned int sSubVersion);
    void                    setLogFileName(std::string logFileName);
    void                    setCANBitrate(EnCANBitrate bitrate);
    void                    setGPSFrequency(EnGPSFrequency frequency);
    void                    setRTCConfigurationFrameID(unsigned int frameID);
    void                    setUseDateAndTimeFromGPS(bool _useDateAndTimeFromGPS);
    void                    setUseDateAndTimeFromGPS(unsigned int _useDateAndTimeFromGPS);
    void                    setTimeZoneShift_30mins(int _timeZoneShift_30minsUnit);
    void                    setTimeZoneShift(double _timeZoneShift);

    unsigned int            getVersion() const;
    unsigned int            getSubVersion() const;
    std::string             getLogFileName() const;
    EnCANBitrate            getCANBitrate() const;
    EnGPSFrequency          getGPSFrequency() const;
    unsigned int            getRTCConfigurationFrameID() const;
    bool                    getUseDateAndTimeFromGPS() const;
    int                     getTimeZoneShift_30mins() const;

    //<----- Access to frames definitions ----->/

    unsigned int            getFramesNumber() const;
    bool                    framesEmpty() const;

    bool                    hasFrameWithId(unsigned int frameID) const;
    ConfigFrame*            getFrameWithId(unsigned int frameID) const;
    ConfigFrame*            addFrame(unsigned int frameID, unsigned int expectedDLC, std::string frameName);
    void                    removeFrame(const FramesIterator frameIterator);
    void                    removeFrame(unsigned int frameID);

    bool                    hasSignal(unsigned int frameID, unsigned int signalID) const;
    ConfigSignal*           getSignal(unsigned int frameID, unsigned int signalID) const;

    FramesIterator          beginFrames();
    FramesIterator          endFrames();
    ConstFramesIterator     cbeginFrames() const;
    ConstFramesIterator     cendFrames() const;

    //<----- Access to triggers definitions ----->/
    unsigned int            getNumberOfStartTriggers();
    unsigned int            getNumberOfStopTriggers();

    ConfigTrigger*          addStartTrigger(std::string triggerName, ConfigTrigger::FrameSignalVariant frameSignalVariant, unsigned long compareConstValue, ConfigTrigger::TriggerCompareOperator compareOperator);
    void                    removeStartTrigger(TriggersIterator trigIt);
    void                    removeStartTrigger(ConfigTrigger* pTrigger);
    unsigned int            getStartTriggersNumber();

    ConfigTrigger*          addStopTrigger(std::string triggerName, ConfigTrigger::FrameSignalVariant frameSignalVariant, unsigned long compareConstValue, ConfigTrigger::TriggerCompareOperator compareOperator);
    void                    removeStopTrigger(TriggersIterator trigIt);
    void                    removeStopTrigger(ConfigTrigger* pTrigger);
    unsigned int            getStopTriggersNumber();

    TriggersIterator        beginStartTriggers();
    TriggersIterator        endStartTriggers();
    ConstTriggersIterator   cbeginStartTriggers() const;
    ConstTriggersIterator   cendStartTriggers() const;

    TriggersIterator        beginStopTriggers();
    TriggersIterator        endStopTriggers();
    ConstTriggersIterator   cbeginStopTriggers() const;
    ConstTriggersIterator   cendStopTriggers() const;

    //<----- General purpose methods definitions ----->/

    void                    removeTriggersWithSignal(const ConfigSignal* pSignal);
    void                    removeTriggersWithFrame(const ConfigFrame* pFrame);

    void                    reset();

    virtual void            writeToBin(WritingClass& writer) override;
    virtual void            readFromBin(ReadingClass& reader) override;

    virtual ~Config() override;
};


#endif // CONFIG_H
