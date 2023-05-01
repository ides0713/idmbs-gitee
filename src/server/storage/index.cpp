#include "index.h"
#include <cstring>                                  // for strlen
#include "../common/re.h"// for Re, GenericError
#include "field.h"                                  // for FieldMeta
#include "record.h"
#include "table.h"// for TableMeta
Re Index::Init(const IndexMeta &index_meta, const FieldMeta &field_meta) {
    index_meta_ = index_meta;
    field_meta_ = field_meta;
    return Re::Success;
}
