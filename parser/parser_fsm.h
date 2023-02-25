#pragma once

#include "functional"
#include "map"
#include "vector"
#include "iostream"
#include "memory"

#include "def/parser_def.h"
#include "def/sql_interface_def.h"
#include "common/lock_free_queue.h"

namespace sql::fsm
{

using StateTransitionTable_t = std::map<TransitionKey_t, TransitionProperty_t>;
using ParamMappingTable_t    = std::map<std::string, EnumParserParamType>;

template <typename CarrierType>
CarrierType* verifyCarrier(PacketCollection_t& carrier)
{
    if (std::get_if<std::monostate>(&carrier)) return nullptr;
    return std::get_if<CarrierType>(&carrier);
}

struct FsmContext_t
{
    EnumParserState                    cur_state = EnumParserState::IDLE;
    EnumParserErrorIndication          error_indication = EnumParserErrorIndication::IDLE;

    std::string                        cur_param = "";
    PacketCollection_t                 data_carrier = PacketCollection_t{std::monostate{}};
};

class FsmParser
{
public:
    bool init(std::shared_ptr<LockFreeQueue<PacketCollection_t>>& sp_flq, std::shared_ptr<bool>& sp_app_running);
    bool parseInput(std::vector<std::string>& params);

private:
    bool registerParam(std::string&& keyword, const EnumParserParamType param_type);
    bool registerTransition(TransitionKey_t&& condition, TransitionProperty_t&& action);

    bool parseCondition(std::string& str_condition, ConditionDescriptor_t& condition);
    bool transit(EnumParserParamType param_type);
    void errorIndicationHandler();
    bool sendToExecutor(PacketCollection_t&& command);

    EnumParserParamType getParamType(std::string copied_param);
    EnumValueType static getValueType(std::string copied_type);

    StateTransitionTable_t  state_transition_table_;
    ParamMappingTable_t     param_mapping_table_;
    FsmContext_t            context_;

    std::shared_ptr<LockFreeQueue<PacketCollection_t>>  sp_lfq_;
    std::shared_ptr<bool>                               sp_app_running_;
};

} // namespace fsm
