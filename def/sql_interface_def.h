#pragma once

#include "vector"
#include "string"
#include "stdint.h"
#include "variant"
#include "optional"

namespace sql
{

using SqlValue_t = std::variant<std::monostate, int32_t, std::string>;

enum class EnumValueType
{
    VALUE_TYPE_IDLE      = 0,
    VALUE_TYPE_INT,
    VALUE_TYPE_STRING,
};

struct TableColumnProperty_t
{
    std::string    column_name;
    EnumValueType  value_type;
    bool           is_primary = false;
};

struct PacketCreateDatabase_t
{
    std::string  db_name;
};

struct PacketDropDatabase_t
{
    std::string  db_name;
};

struct PacketCreateTable_t
{
    std::string                         table_name;
    std::vector<TableColumnProperty_t>  vec_column_property;
};

struct PacketUseDatabase_t
{
    std::string  db_name;
};

struct PacketDropTable_t
{
    std::string  table_name;
};

enum class EnumConditionActionType
{
    IDLE      = 0,
    LT,
    LTEQ,
    EQ,
    GTEQ,
    GT,
};

struct ConditionDescriptor_t
{
    std::string                 column_name;
    EnumConditionActionType     action;
    SqlValue_t                  anchor_val;
};

struct PacketSelect_t
{
    std::string                 table_name;
    std::string                 column_name;
    ConditionDescriptor_t       condition;
};

struct PacketDelect_t
{
    std::string                 table_name;
    ConditionDescriptor_t       condition;
};

struct PacketInsert_t
{
    std::string                   table_name;
    std::vector<std::string>      vec_value;
};

using PacketCollection_t = std::variant<std::monostate,
                                        PacketCreateDatabase_t, 
                                        PacketDropDatabase_t, 
                                        PacketCreateTable_t, 
                                        PacketUseDatabase_t, 
                                        PacketDropTable_t, 
                                        PacketSelect_t, 
                                        PacketDelect_t, 
                                        PacketInsert_t>;


} // namespace sql
