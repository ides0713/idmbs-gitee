#pragma once

#include "../parse/parse_defs.h"
#include <cstdint>
#include <sstream>

class RecordId {
public:
    int32_t page_id, slot_id;
public:
    RecordId() = default;

    RecordId(const int32_t &pid, const int32_t &sid) : page_id(pid), slot_id(sid) {}

    [[nodiscard]] std::string toString() const;

    bool operator==(const RecordId &other) const { return page_id == other.page_id && slot_id == other.slot_id; }

    bool operator!=(const RecordId &other) const { return !(*this == other); }

public:
    static int compare(const RecordId *rid_1, const RecordId *rid_2);

    static RecordId *min();

    static RecordId *max();
};

class Record {
public:
    Record() : data_(nullptr) {}

    void initialize(const RecordId &record_id, char *data);

    void setRecordId(const RecordId &record_id) { record_id_ = record_id; }

    void setData(char *data) { data_ = data; };

    RecordId &getRecordId() { return record_id_; }

    char *getData() { return data_; }

private:
    RecordId record_id_;
    char *data_;
};
