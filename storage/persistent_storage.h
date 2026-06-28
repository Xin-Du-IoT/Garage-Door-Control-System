#pragma once

#include <optional>
#include "garage_types.h"

namespace garage {

    class IPersistentStorage {
    public:
        virtual ~IPersistentStorage() = default;

        virtual void init() = 0;
        virtual bool save(const PersistentData& data) = 0;
        virtual std::optional<PersistentData> load() = 0;
        virtual bool clear() = 0;
    };

    class PersistentStorage : public IPersistentStorage {
    public:
        void init() override;
        bool save(const PersistentData& data) override;
        std::optional<PersistentData> load() override;
        bool clear() override;
    };

} // namespace garage