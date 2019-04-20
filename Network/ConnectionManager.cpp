#include "ConnectionManager.h"

namespace network {

ConnectionManager::ConnectionManager(boost::asio::io_service& ioContext)
  : m_IOContext(ioContext)
{}

void ConnectionManager::addConnection(IChannelPtr pChannel, BufferedTerminalPtr pTerminal)
{
  m_Connections.emplace_back(std::move(pChannel), std::move(pTerminal));
}

bool ConnectionManager::prephareStage(uint16_t)
{
  if (!m_IOContext.poll())
      return false;
  while(m_IOContext.poll());
  m_nNextConnectionId.store(0);
  return true;
}

void ConnectionManager::proceedStage(uint16_t, size_t)
{
  size_t nConnectionId = m_nNextConnectionId.fetch_add(1);
  m_Connections[nConnectionId].m_pTerminal->handleBufferedMessages();
}

} // namespace network