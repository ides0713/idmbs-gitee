#include "record.h"
#include "table.h"

std::string RecordId::toString() const {
    std::stringstream ss;
    ss << "PageNum:" << page_id << ", SlotNum:" << slot_id;
    return ss.str();
}

int RecordId::compare(const RecordId *rid_1, const RecordId *rid_2) {
    int page_diff = rid_1->page_id - rid_2->page_id;
    if (page_diff != 0)
        return page_diff;
    else
        return rid_1->slot_id - rid_2->slot_id;
}

RecordId *RecordId::min() {
    static RecordId rid{0, 0};
    return &rid;
}

RecordId *RecordId::max() {
    static RecordId rid{std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()};
    return &rid;
}

void Record::initialize(const RecordId &record_id, char *data) {
    setRecordId(record_id);
    setData(data);
}
