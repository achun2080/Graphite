#include "tile.h"
#include "log.h"
#include "magic_pep_performance_model.h"
#include "branch_predictor.h"

using std::endl;

MagicPepPerformanceModel::MagicPepPerformanceModel(Tile *tile, float frequency)
    : CorePerfModel(tile, frequency)
    , m_instruction_count(0)
{
}

MagicPepPerformanceModel::~MagicPepPerformanceModel()
{
}

void MagicPepPerformanceModel::outputSummary(std::ostream &os)
{
   os << "  Instructions: " << getInstructionCount() << endl;
   frequencySummary(os);

   if (getBranchPredictor())
      getBranchPredictor()->outputSummary(os);
}

void MagicPepPerformanceModel::handleInstruction(Instruction *instruction)
{
   // compute cost
   UInt64 cost = 0;

   const OperandList &ops = instruction->getOperands();
   for (unsigned int i = 0; i < ops.size(); i++)
   {
      const Operand &o = ops[i];

      if (o.m_type == Operand::MEMORY)
      {
         DynamicInstructionInfo &info = getDynamicInstructionInfo();

         if (o.m_direction == Operand::READ)
         {
            LOG_ASSERT_ERROR(info.type == DynamicInstructionInfo::MEMORY_READ,
                             "Expected memory read info, got: %d.", info.type);

            cost += info.memory_info.latency;
            // ignore address
         }
         else
         {
            LOG_ASSERT_ERROR(info.type == DynamicInstructionInfo::MEMORY_WRITE,
                             "Expected memory write info, got: %d.", info.type);

            cost += info.memory_info.latency;
            // ignore address
         }

         popDynamicInstructionInfo();
      }
   }

   UInt64 instruction_cost = instruction->getCost();
   if (isModeled(instruction->getType()))
      cost += instruction_cost;
   else
      cost += 1;

   // update counters
   m_instruction_count++;
   m_cycle_count += cost;
}

bool MagicPepPerformanceModel::isModeled(InstructionType instruction_type)
{
   switch(instruction_type)
   {
      case INST_RECV:
      case INST_SYNC:
      case INST_SPAWN:
         return true;

      default:
         return false;
   }
}