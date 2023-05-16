#include "index.h"
#include "../common/re.h"// for Re, GenericError
#include "field.h"       // for FieldMeta
#include "record.h"
#include "table.h"// for TableMeta
#include <cstring>// for strlen
Re Index::Init(const IndexMeta &index_meta, const FieldMeta &field_meta) {
    index_meta_ = index_meta;
    field_meta_ = field_meta;
    return Re::Success;
}
