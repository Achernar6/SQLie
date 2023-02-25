#include "algorithm"

#include "parser/parser_fsm.h"

namespace sql::fsm
{

bool FsmParser::init(std::shared_ptr<LockFreeQueue<PacketCollection_t>>& sp_flq, std::shared_ptr<bool>& sp_app_running)
{
    sp_lfq_ = sp_flq;
    sp_app_running_ = sp_app_running;

    // register all keywords
    bool flag_register_param = 
           registerParam("CREATE",   EnumParserParamType::KW_CREATE)
        && registerParam("DATABASE", EnumParserParamType::KW_DATABASE)
        && registerParam("DROP",     EnumParserParamType::KW_DROP)
        && registerParam("USE",      EnumParserParamType::KW_USE)
        && registerParam("TABLE",    EnumParserParamType::KW_TABLE)
        && registerParam("PRIMARY",  EnumParserParamType::KW_PRIMARY)
        && registerParam("SELECT",   EnumParserParamType::KW_SELECT)
        && registerParam("FROM",     EnumParserParamType::KW_FROM)
        && registerParam("WHERE",    EnumParserParamType::KW_WHERE)
        && registerParam("DELETE",   EnumParserParamType::KW_DELETE)
        && registerParam("INSERT",   EnumParserParamType::KW_INSERT)
        && registerParam("VALUES",   EnumParserParamType::KW_VALUES)
        && registerParam("INT",      EnumParserParamType::KW_VALTYPE)
        && registerParam("STRING",   EnumParserParamType::KW_VALTYPE)
        && registerParam("EXIT",     EnumParserParamType::LOCAL_EXIT);

    if (!flag_register_param)
    {
        printf("Fail to register keywords\n");
        return false;
    }

    // register all transitions
    bool flag_register_transition = 
        // create
           registerTransition(
            TransitionKey_t{EnumParserState::IDLE, EnumParserParamType::KW_CREATE},
            TransitionProperty_t{EnumParserState::CREATE, PacketCollection_t{std::monostate{}}, [this](){ return true; }}
        )
        // create database
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE, EnumParserParamType::KW_DATABASE},
            TransitionProperty_t{EnumParserState::CREATE_DATABASE, PacketCollection_t{PacketCreateDatabase_t{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_DATABASE, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::CREATE_DATABASE_DBNAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateDatabase_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->db_name = this->context_.cur_param;

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_DATABASE_DBNAME, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::CREATE_DATABASE_DBNAME_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateDatabase_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        // create table
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE, EnumParserParamType::KW_TABLE},
            TransitionProperty_t{EnumParserState::CREATE_TABLE, PacketCollection_t{PacketCreateTable_t{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_TABLE, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::CREATE_TABLE_TBNAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->table_name = this->context_.cur_param;

                return true;
            }}
        )
        // create only a table
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_TABLE_TBNAME, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::CREATE_TABLE_TBNAME_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        // start of a column
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_TABLE_TBNAME, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                TableColumnProperty_t column;
                column.column_name = this->context_.cur_param;
                p_carrier->vec_column_property.emplace_back(column);

                return true; 
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME, EnumParserParamType::KW_VALTYPE},
            TransitionProperty_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                auto& column = p_carrier->vec_column_property.back();
                column.value_type = FsmParser::getValueType(this->context_.cur_param);
                if (column.value_type == EnumValueType::VALUE_TYPE_IDLE) return false;

                return true; 
            }}
        )
        // end of columns
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        // start another column
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                TableColumnProperty_t column;
                column.column_name = this->context_.cur_param;
                p_carrier->vec_column_property.emplace_back(column);

                return true; 
            }}
        )
        // a primary column
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME, EnumParserParamType::KW_PRIMARY},
            TransitionProperty_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME_PRIMARY, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                auto& column = p_carrier->vec_column_property.back();
                column.is_primary = true;

                return true; 
            }}
        )
        // start a new column just after a primary column
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME_PRIMARY, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                TableColumnProperty_t column;
                column.column_name = this->context_.cur_param;
                p_carrier->vec_column_property.emplace_back(column);

                return true; 
            }}
        )
        // end with a primary column
        && registerTransition(
            TransitionKey_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME_PRIMARY, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::CREATE_TABLE_TBNAME_COLUMNNAME_TYPENAME_PRIMARY_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketCreateTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        // drop
        && registerTransition(
            TransitionKey_t{EnumParserState::IDLE, EnumParserParamType::KW_DROP},
            TransitionProperty_t{EnumParserState::DROP, PacketCollection_t{std::monostate{}}, [this](){ return true; }}
        )
        // drop database
        && registerTransition(
            TransitionKey_t{EnumParserState::DROP, EnumParserParamType::KW_DATABASE},
            TransitionProperty_t{EnumParserState::DROP_DATABASE, PacketCollection_t{PacketDropDatabase_t{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::DROP_DATABASE, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::DROP_DATABASE_DBNAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketDropDatabase_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->db_name = this->context_.cur_param;

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::DROP_DATABASE_DBNAME, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::DROP_DATABASE_DBNAME_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketDropDatabase_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        // drop table
        && registerTransition(
            TransitionKey_t{EnumParserState::DROP, EnumParserParamType::KW_TABLE},
            TransitionProperty_t{EnumParserState::DROP_TABLE, PacketCollection_t{PacketDropTable_t{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::DROP_TABLE, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::DROP_TABLE_TBNAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketDropTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->table_name = this->context_.cur_param;

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::DROP_TABLE_TBNAME, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::DROP_TABLE_TBNAME_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketDropTable_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        // use database
        && registerTransition(
            TransitionKey_t{EnumParserState::IDLE, EnumParserParamType::KW_USE},
            TransitionProperty_t{EnumParserState::USE, PacketCollection_t{PacketUseDatabase_t{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::USE, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::USE_DBNAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketUseDatabase_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->db_name = this->context_.cur_param;

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::USE_DBNAME, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::USE_DBNAME_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketUseDatabase_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        // select
        && registerTransition(
            TransitionKey_t{EnumParserState::IDLE, EnumParserParamType::KW_SELECT},
            TransitionProperty_t{EnumParserState::SELECT, PacketCollection_t{PacketSelect_t{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::SELECT, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::SELECT_COLUMNNAME, PacketCollection_t{std::monostate{}}, [this](){ 
                auto p_carrier = verifyCarrier<PacketSelect_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->column_name = this->context_.cur_param;

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::SELECT_COLUMNNAME, EnumParserParamType::KW_FROM},
            TransitionProperty_t{EnumParserState::SELECT_COLUMNNAME_FROM, PacketCollection_t{std::monostate{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::SELECT_COLUMNNAME_FROM, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::SELECT_COLUMNNAME_FROM_TBNAME, PacketCollection_t{std::monostate{}}, [this](){ 
                auto p_carrier = verifyCarrier<PacketSelect_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->table_name = this->context_.cur_param;

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::SELECT_COLUMNNAME_FROM_TBNAME, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::SELECT_COLUMNNAME_FROM_TBNAME_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketSelect_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::SELECT_COLUMNNAME_FROM_TBNAME, EnumParserParamType::KW_WHERE},
            TransitionProperty_t{EnumParserState::SELECT_COLUMNNAME_FROM_TBNAME_WHERE, PacketCollection_t{std::monostate{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::SELECT_COLUMNNAME_FROM_TBNAME_WHERE, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::SELECT_COLUMNNAME_FROM_TBNAME_WHERE_COND, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketSelect_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                if (!parseCondition(this->context_.cur_param, p_carrier->condition)) return false;

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::SELECT_COLUMNNAME_FROM_TBNAME_WHERE, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::SELECT_COLUMNNAME_FROM_TBNAME_WHERE_COND_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketSelect_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        // delete
        && registerTransition(
            TransitionKey_t{EnumParserState::IDLE, EnumParserParamType::KW_DELETE},
            TransitionProperty_t{EnumParserState::DELETE, PacketCollection_t{PacketDelect_t{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::DELETE, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::DELETE_TBNAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketDelect_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->table_name = this->context_.cur_param;

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::DELETE_TBNAME, EnumParserParamType::KW_WHERE},
            TransitionProperty_t{EnumParserState::DELETE_TBNAME_WHERE, PacketCollection_t{std::monostate{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::DELETE_TBNAME_WHERE, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::DELETE_TBNAME_WHERE_COND, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketDelect_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                if (!parseCondition(this->context_.cur_param, p_carrier->condition)) return false;

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::DELETE_TBNAME_WHERE_COND, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::DELETE_TBNAME_WHERE_COND_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketDelect_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        // insert
        && registerTransition(
            TransitionKey_t{EnumParserState::IDLE, EnumParserParamType::KW_INSERT},
            TransitionProperty_t{EnumParserState::INSERT, PacketCollection_t{PacketInsert_t{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::INSERT, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::INSERT_TBNAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketInsert_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->table_name = this->context_.cur_param;

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::INSERT_TBNAME, EnumParserParamType::KW_VALUES},
            TransitionProperty_t{EnumParserState::INSERT_TBNAME_VALUES, PacketCollection_t{std::monostate{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::INSERT_TBNAME_VALUES, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::INSERT_TBNAME_VALUES_VALUENAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketInsert_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->vec_value.emplace_back(this->context_.cur_param);

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::INSERT_TBNAME_VALUES_VALUENAME, EnumParserParamType::VALUE_OR_NAME},
            TransitionProperty_t{EnumParserState::INSERT_TBNAME_VALUES_VALUENAME, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketInsert_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;
                p_carrier->vec_value.emplace_back(this->context_.cur_param);

                return true;
            }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::INSERT_TBNAME_VALUES_VALUENAME, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::INSERT_TBNAME_VALUES_VALUENAME_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                auto p_carrier = verifyCarrier<PacketInsert_t>(this->context_.data_carrier);
                if (p_carrier == nullptr) return false;

                sendToExecutor(PacketCollection_t{*p_carrier});
                return true; 
            }}
        )
        // local-exit
        && registerTransition(
            TransitionKey_t{EnumParserState::IDLE, EnumParserParamType::LOCAL_EXIT},
            TransitionProperty_t{EnumParserState::LOCAL_EXIT, PacketCollection_t{std::monostate{}}, [this](){ return true; }}
        )
        && registerTransition(
            TransitionKey_t{EnumParserState::LOCAL_EXIT, EnumParserParamType::END_MARKER},
            TransitionProperty_t{EnumParserState::LOCAL_EXIT_END, PacketCollection_t{std::monostate{}}, [this]()
            {
                *this->sp_app_running_ = false;
                return true;
            }}
        );

    if (!flag_register_transition)
    {
        printf("Fail to register transitions\n");
        return false;
    }

    return true;
}

bool FsmParser::parseInput(std::vector<std::string>& params)
{
    // reset context
    context_.cur_state        = EnumParserState::IDLE;
    context_.error_indication = EnumParserErrorIndication::IDLE;
    context_.cur_param        = "";
    context_.data_carrier     = std::monostate{};

    for (auto& param_string : params)
    {
        context_.cur_param = param_string;
        auto param = getParamType(param_string);
        if (!transit(param))
        {
            errorIndicationHandler();
            return false;
        }
    }

    if (!transit(EnumParserParamType::END_MARKER))
    {
        context_.error_indication = EnumParserErrorIndication::INCOMPLETE_COMMAND;
        errorIndicationHandler();
        return false;
    }

    return true;
}

bool FsmParser::registerParam(std::string&& keyword, const EnumParserParamType param_type)
{
    if (param_mapping_table_.find(keyword) != param_mapping_table_.end()) return false;
    param_mapping_table_.emplace(keyword, param_type);
    return true;
}

bool FsmParser::registerTransition(TransitionKey_t&& condition, TransitionProperty_t&& action)
{
    if (state_transition_table_.find(condition) != state_transition_table_.end()) return false;
    state_transition_table_.emplace(condition, action);
    return true;
}

bool FsmParser::parseCondition(std::string& str_condition, ConditionDescriptor_t& condition)
{
    std::string column_name = "";
    std::string op1         = "";
    std::string op2         = "";
    std::string value       = "";

    size_t index = 0;
    for (; index < str_condition.size(); index ++)
    {
        char& _char = str_condition[index];

        if (_char == '<' || _char == '>' || _char == '=')
        {
            if (op1.empty()) op1.append(1, _char);
            else if (op2.empty()) op2.append(1, _char);
            else
            {
                context_.error_indication = EnumParserErrorIndication::INVALID_CONDITION;
                return false;   
            }
        }
        else
        {
            if (op1.empty()) column_name.append(1, _char);
            else value.append(1, _char);
        }
    }

    if (column_name.empty() || op1.empty() || value.empty())
    {
        context_.error_indication = EnumParserErrorIndication::INVALID_CONDITION;
        return false;   
    }

    if (op1 == "<" && op2.empty()) condition.action = EnumConditionActionType::LT;
    else if (op1 == "<" && op2 == "=") condition.action = EnumConditionActionType::LTEQ;
    else if (op1 == "=" && op2 == "=") condition.action = EnumConditionActionType::EQ;
    else if (op1 == "=" && op2.empty()) condition.action = EnumConditionActionType::EQ;
    else if (op1 == ">" && op2 == "=") condition.action = EnumConditionActionType::GTEQ;
    else if (op1 == ">" && op2.empty()) condition.action = EnumConditionActionType::GT;
    else
    {
        context_.error_indication = EnumParserErrorIndication::INVALID_CONDITION;
        return false;   
    }

    condition.column_name = std::move(column_name);
    condition.anchor_val = std::move(value);

    return true;
}

bool FsmParser::transit(EnumParserParamType param_type)
{
    auto iter_state = state_transition_table_.find(TransitionKey_t{context_.cur_state, param_type}); 
    if (iter_state == state_transition_table_.end())
    {
        context_.error_indication = EnumParserErrorIndication::NO_TRANSITION;
        return false;
    }

    context_.cur_state = iter_state->second.next_state;
    if (std::get_if<std::monostate>(&iter_state->second.on_changing_data_carrier) == nullptr)
    {
        if (std::get_if<std::monostate>(&context_.data_carrier) == nullptr)
        {
            context_.error_indication = EnumParserErrorIndication::DUPLICATE_CARRIER;
            return false;
        }

        context_.data_carrier = iter_state->second.on_changing_data_carrier;
    }

    if (!iter_state->second.func_action())
    {
        return false;
    }

    return true;
}

void FsmParser::errorIndicationHandler()
{
    switch (context_.error_indication)
    {
        case EnumParserErrorIndication::NO_TRANSITION:
        {
            printf("Syntex error after \"%s\"\n", context_.cur_param.c_str());
            break;
        }
        case EnumParserErrorIndication::DUPLICATE_CARRIER:
        {
            printf("Duplicate carrier triggered after \"%s\"\n", context_.cur_param.c_str());
            break;
        }
        case EnumParserErrorIndication::INCOMPLETE_COMMAND:
        {
            printf("Incomplete command after \"%s\"\n", context_.cur_param.c_str());
            break;
        }
        case EnumParserErrorIndication::INVALID_CONDITION:
        {
            printf("Invalid condition \"%s\"", context_.cur_param.c_str());
            break;
        }
        default:
        {
            printf("False postive error after \"%s\"\n", context_.cur_param.c_str());
            break;
        }
    }
}

bool FsmParser::sendToExecutor(PacketCollection_t&& command)
{
    return sp_lfq_->push(command);
}

EnumParserParamType FsmParser::getParamType(std::string copied_param)
{
    std::transform(copied_param.begin(), copied_param.end(), copied_param.begin(), ::toupper);
    auto iter_param = param_mapping_table_.find(copied_param);
    if (iter_param == param_mapping_table_.end())
    {
        return EnumParserParamType::VALUE_OR_NAME;
    }
    else
    {
        return iter_param->second;
    }
}

EnumValueType FsmParser::getValueType(std::string copied_type)
{
    std::transform(copied_type.begin(), copied_type.end(), copied_type.begin(), ::toupper);
    if (copied_type == "INT") return EnumValueType::VALUE_TYPE_INT;
    else if (copied_type == "STRING") return EnumValueType::VALUE_TYPE_STRING;
    return EnumValueType::VALUE_TYPE_IDLE;
}


}