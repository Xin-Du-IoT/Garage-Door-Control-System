#include "persistent_storage.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

namespace garage {
namespace {

constexpr uint32_t kMagic   = 0x47445231u; // "GDR1"
constexpr uint16_t kVersion = 1;

constexpr uint32_t kFlashTargetOffset = PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;

struct StoredRecord {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    PersistentData data;
    uint32_t checksum;
};

static_assert(sizeof(StoredRecord) <= FLASH_PAGE_SIZE, "StoredRecord must fit in one flash page");

uint32_t fnv1a_32(const uint8_t* data, size_t len) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

uint32_t compute_checksum(const StoredRecord& record) {
    return fnv1a_32(reinterpret_cast<const uint8_t*>(&record),
                    offsetof(StoredRecord, checksum));
}

const StoredRecord* flash_record_ptr() {
    return reinterpret_cast<const StoredRecord*>(XIP_BASE + kFlashTargetOffset);
}

} // namespace

void PersistentStorage::init() {
}

bool PersistentStorage::save(const PersistentData& data) {
    StoredRecord record{};
    record.magic = kMagic;
    record.version = kVersion;
    record.reserved = 0;
    record.data = data;
    record.checksum = compute_checksum(record);

    alignas(FLASH_PAGE_SIZE) std::array<uint8_t, FLASH_PAGE_SIZE> page{};
    page.fill(0xFF);
    std::memcpy(page.data(), &record, sizeof(record));

    const uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(kFlashTargetOffset, FLASH_SECTOR_SIZE);
    flash_range_program(kFlashTargetOffset, page.data(), FLASH_PAGE_SIZE);
    restore_interrupts(ints);

    auto loaded = load();
    if (!loaded.has_value()) {
        return false;
    }

    return loaded->calibration_state == data.calibration_state
        && loaded->travel_steps == data.travel_steps
        && loaded->current_position_steps == data.current_position_steps
        && loaded->last_known_door_state == data.last_known_door_state;
}

std::optional<PersistentData> PersistentStorage::load() {
    const StoredRecord* record = flash_record_ptr();

    if (record->magic != kMagic) {
        return std::nullopt;
    }

    if (record->version != kVersion) {
        return std::nullopt;
    }

    if (record->checksum != compute_checksum(*record)) {
        return std::nullopt;
    }

    return record->data;
}

bool PersistentStorage::clear() {
    alignas(FLASH_PAGE_SIZE) std::array<uint8_t, FLASH_PAGE_SIZE> blank{};
    blank.fill(0xFF);

    const uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(kFlashTargetOffset, FLASH_SECTOR_SIZE);
    flash_range_program(kFlashTargetOffset, blank.data(), FLASH_PAGE_SIZE);
    restore_interrupts(ints);

    return !load().has_value();
}

} // namespace garage