#include "lcp.h"
#include "simulator.h"
#include "core.h"
#include "message_types.h"
#include "thread_manager.h"

#include "log.h"

// -- general LCP functionality

LCP::LCP()
   : m_proc_num(Config::getSingleton()->getCurrentProcessNum())
   , m_transport(Transport::getSingleton()->getGlobalNode())
   , m_finished(false)
{
}

LCP::~LCP()
{
}

void LCP::run()
{
   LOG_PRINT("LCP started.");

   while (!m_finished)
   {
      processPacket();
   }
}

void LCP::processPacket()
{
   Byte *pkt = m_transport->recv();

   SInt32 *msg_type = (SInt32*)pkt;

   LOG_PRINT("Received message type: %d", *msg_type);

   Byte *data = pkt + sizeof(SInt32);

   switch (*msg_type)
   {
   case LCP_MESSAGE_QUIT:
      LOG_PRINT("Received quit message.");
      m_finished = true;
      break;

   case LCP_MESSAGE_COMMID_UPDATE:
      updateCommId(data);
      break;

   case LCP_MESSAGE_SIMULATOR_FINISHED:
      Sim()->handleFinish();
      break;

   case LCP_MESSAGE_SIMULATOR_FINISHED_ACK:
      Sim()->deallocateProcess();
      break;

   case LCP_MESSAGE_THREAD_SPAWN_REQUEST_FROM_REQUESTER:
      Sim()->getThreadManager()->masterSpawnThread((ThreadSpawnRequest*)pkt);
      break;

   case LCP_MESSAGE_THREAD_SPAWN_REQUEST_FROM_MASTER:
      Sim()->getThreadManager()->slaveSpawnThread((ThreadSpawnRequest*)pkt);
      break;

   case LCP_MESSAGE_THREAD_SPAWN_REPLY_FROM_SLAVE:
      Sim()->getThreadManager()->masterSpawnThreadReply((ThreadSpawnRequest*)pkt);
      break;

   case LCP_MESSAGE_THREAD_EXIT:
      Sim()->getThreadManager()->masterOnThreadExit(*((SInt32*)data), *((UInt64*)data+4));
      break;

   case LCP_MESSAGE_THREAD_JOIN_REQUEST:
      Sim()->getThreadManager()->masterJoinThread((ThreadJoinRequest*)pkt);
      break;

   default:
      LOG_ASSERT_ERROR(false, "Unexpected message type: %d.", *msg_type);
      break;
   }

   delete [] pkt;
}

void LCP::finish()
{
   LOG_PRINT("Send LCP quit message");

   SInt32 msg_type = LCP_MESSAGE_QUIT;

   m_transport->globalSend(m_proc_num,
                           &msg_type,
                           sizeof(msg_type));

   while (!m_finished)
      sched_yield();

   LOG_PRINT("LCP finished.");
}

// -- functions for specific tasks

struct CommMapUpdate
{
   SInt32 comm_id;
   core_id_t core_id;
};

void LCP::updateCommId(void *vp)
{
   CommMapUpdate *update = (CommMapUpdate*)vp;

   LOG_PRINT("Initializing comm_id: %d to core_id: %d", update->comm_id, update->core_id);
   Config::getSingleton()->updateCommToCoreMap(update->comm_id, update->core_id);

   // FIXME: Do we need to send an ACK?
}
