#pragma once
#include <vector>
#include <string>
#include <cstdint>

class SaveFile {
public:
    SaveFile() = default;
    
    bool load(const std::string& filepath);
    bool save(const std::string& filepath);
    bool isLoaded() const { return !data.empty(); }

    int32_t getStat(size_t internalOffset) const;
    void setStat(size_t internalOffset, int32_t amount);

    bool getItem(int id) const;
    void setItem(int id, bool unlocked);

    bool getAchievement(int id) const;
    void setAchievement(int id, bool unlocked);

    bool getChallenge(int id) const;
    void setChallenge(int id, bool unlocked);

    static constexpr size_t STAT_MOM_KILLS = 0x4;
    static constexpr size_t STAT_DEATHS = 0x24;
    static constexpr size_t STAT_DONATION = 0x4C;
    static constexpr size_t STAT_EDEN_TOKENS = 0x50;
    static constexpr size_t STAT_WIN_STREAK = 0x54;
    static constexpr size_t STAT_GREED_MACHINE = 0x1B0;

private:
    std::vector<uint8_t> data;
    std::vector<size_t> sectionOffsets;

    void parseSectionOffsets();
    int32_t getInt(size_t offset, size_t num_bytes = 4) const;
    void setInt(size_t offset, int32_t val, size_t num_bytes = 4);

    uint32_t calcChecksum(size_t ofs, size_t length) const;
    void updateChecksum();
    
    static const uint32_t CrcTable[256];
};