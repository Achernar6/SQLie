#pragma once

#include "functional"

#include "def/sql_interface_def.h"

#define PARSER_MAX_KEYWORD_NUM    256
#define PARSER_MAX_TRANSITION_NUM 65536

namespace sql::fsm
{

using OnTransitingAction_t = std::function<bool()>;

enum class EnumParserState
{
    IDLE          = 0,

    CREATE,
    CREATE_DATABASE,
    CREATE_DATABASE_DBNAME,
    CREATE_DATABASE_DBNAME_END,

    CREATE_TABLE,
    CREATE_TABLE_TBNAME,
    CREATE_TABLE_TBNAME_END,
    CREATE_TABLE_TBNAME_COLUMNNAME,
    CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME,
    CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME_END,
    CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME_PRIMARY,
    CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME_PRIMARY_END,

    DROP,
    DROP_DATABASE,
    DROP_DATABASE_DBNAME,
    DROP_DATABASE_DBNAME_END,
    
    DROP_TABLE,
    DROP_TABLE_TBNAME,
    DROP_TABLE_TBNAME_END,

    USE,
    USE_DBNAME,
    USE_DBNAME_END,

    SELECT,
    SELECT_COLUMNNAME,
    SELECT_COLUMNNAME_FROM,
    SELECT_COLUMNNAME_FROM_TBNAME,
    SELECT_COLUMNNAME_FROM_TBNAME_END,
    // SELECT_COLUMNNAME_FROM_TBNAME_WHERE,
    // SELECT_COLUMNNAME_FROM_TBNAME_WHERE_COND,
    // SELECT_COLUMNNAME_FROM_TBNAME_WHERE_COND_END,
    
    DELETE,
    DELETE_TBNAME,
    DELETE_TBNAME_END,
    // DELETE_TBNAME_WHERE,
    // DELETE_TBNAME_WHERE_COND,
    // DELETE_TBNAME_WHERE_COND_END,
    
    INSERT,
    INSERT_TBNAME,
    INSERT_TBNAME_VALUES,
    INSERT_TBNAME_VALUES_VALUENAME,
    INSERT_TBNAME_VALUES_VALUENAME_END,

    LOCAL_EXIT,
    LOCAL_EXIT_END,
};

enum class EnumParserParamType
{
    IDLE               = 0,
    VALUE_OR_NAME,

    KW_CREATE,
    KW_DATABASE,
    KW_DROP,
    KW_USE,
    KW_TABLE,
    KW_PRIMARY,
    KW_SELECT,
    KW_FROM,
    KW_WHERE,
    KW_DELETE,
    KW_INSERT,
    KW_VALUES,
    KW_VALTYPE,

    LOCAL_EXIT,

    END_MARKER,
};

enum class EnumParserErrorIndication
{
    IDLE              = 0,

    NO_TRANSITION,
    DUPLICATE_CARRIER,
    INCOMPLETE_COMMAND,
};

struct TransitionKey_t
{
    EnumParserState      src_state;
    EnumParserParamType  fsm_input;

    bool operator== (const TransitionKey_t& __o) const
    {
        return (this->src_state == __o.src_state) && (this->fsm_input == __o.fsm_input);
    }

    bool operator< (const TransitionKey_t& __o) const
    {
        if (this->src_state < __o.src_state) return true;
        else if (this->src_state == __o.src_state && this->fsm_input < __o.fsm_input) return true;

        return false;
    }

    bool operator< (const TransitionKey_t&& __o) const
    {
        if (this->src_state < __o.src_state) return true;
        else if (this->src_state == __o.src_state && this->fsm_input < __o.fsm_input) return true;

        return false;
    }
};

struct TransitionProperty_t
{
    EnumParserState                    next_state;
    PacketCollection_t                 on_changing_data_carrier;
    OnTransitingAction_t               func_action;
};

}