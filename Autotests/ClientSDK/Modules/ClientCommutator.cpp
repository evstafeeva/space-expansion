#include "ClientCommutator.h"

namespace autotests { namespace client {

bool ClientCommutator::getTotalSlots(uint32_t& nTotalSlots)
{
  spex::Message request;
  request.mutable_commutator()->mutable_gettotalslots();
  if (!m_pSyncPipe->send(request))
    return false;

  spex::ICommutator message;
  if (!m_pSyncPipe->wait(message))
    return false;
  if (message.choice_case() != spex::ICommutator::kTotalSlotsResponse)
    return false;
  nTotalSlots = message.totalslotsresponse().ntotalslots();
  return true;
}

bool ClientCommutator::getAttachedModulesList(
    uint32_t nTotal, ModulesList &attachedModules)
{
  spex::Message request;
  request.mutable_commutator()->mutable_getallmodulesinfo();
  if (!m_pSyncPipe->send(request))
    return false;

  for (size_t i = 0; i < nTotal; ++i) {
    spex::ICommutator response;
    if (!m_pSyncPipe->wait(response))
      return false;
    if (response.choice_case() != spex::ICommutator::kModuleInfo)
      return false;
    attachedModules.push_back(
          ModuleInfo({response.moduleinfo().nslotid(),
                      response.moduleinfo().smoduletype()}));
  }
  return attachedModules.size() == nTotal;
}

TunnelPtr ClientCommutator::openTunnel(uint32_t nSlotId)
{
  if (!sendOpenTunnel(nSlotId))
    return TunnelPtr();

  uint32_t nOpenedTunnelId = 0;
  if (!waitOpenTunnelSuccess(&nOpenedTunnelId))
    return TunnelPtr();

  TunnelPtr pTunnel = std::make_shared<Tunnel>(nOpenedTunnelId);
  m_pSyncPipe->attachTunnelHandler(nOpenedTunnelId, pTunnel);
  pTunnel->setProceeder(m_pSyncPipe->getProceeder());
  pTunnel->attachToDownlevel(m_pSyncPipe);
  return pTunnel;
}

bool ClientCommutator::sendOpenTunnel(uint32_t nSlotId)
{
  spex::Message request;
  request.mutable_commutator()->mutable_opentunnel()->set_nslotid(nSlotId);
  return m_pSyncPipe->send(request);
}

bool ClientCommutator::waitOpenTunnelSuccess(uint32_t *pOpenedTunnelId)
{
  spex::ICommutator message;
  if (!m_pSyncPipe->wait(message))
    return false;
  if (message.choice_case() != spex::ICommutator::kOpenTunnelSuccess)
    return false;
  if (pOpenedTunnelId) {
    *pOpenedTunnelId = message.opentunnelsuccess().ntunnelid();
  }
  return true;
}

bool ClientCommutator::waitOpenTunnelFailed()
{
  spex::ICommutator message;
  return m_pSyncPipe->wait(message)
      && message.choice_case() == spex::ICommutator::kOpenTunnelFailed;
}

}}  // namespace autotests::client
