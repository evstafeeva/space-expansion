#include "Ship.h"

#include <yaml-cpp/yaml.h>

DECLARE_GLOBAL_CONTAINER_CPP(ships::Ship);

namespace ships
{

Ship::Ship(std::string const& sShipType, double weight)
  : BaseModule(std::string("Ship/") + sShipType),
    newton::PhysicalObject(weight),
    m_pCommutator(std::make_shared<modules::Commutator>())
{
  GlobalContainer<Ship>::registerSelf(this);
}

Ship::~Ship()
{
  for (auto& kv : m_Modules)
    kv.second->onDoestroyed();
}

bool Ship::loadState(YAML::Node const& source)
{
  if (!PhysicalObject::loadState(source))
    return false;

  // Loading state of modules
  for (auto const& kv : source["modules"]) {
    std::string const& sModuleName = kv.first.as<std::string>();
    auto I = m_Modules.find(sModuleName);
    if (I == m_Modules.end())
      return false;
    modules::BaseModulePtr& pModule = I->second;
    if (!pModule->loadState(kv.second)) {
      return false;
    }
  }
  return true;
}

bool Ship::installModule(std::string sName, modules::BaseModulePtr pModule)
{
  if (m_Modules.find(sName) != m_Modules.end())
    return false;
  m_Modules.insert(std::make_pair(std::move(sName), pModule));
  pModule->installOn(this);
  return true;
}

void Ship::onMessageReceived(uint32_t nSessionId, spex::Message const& message)
{
  if (message.choice_case() == spex::Message::kCommutator) {
    // Forwarding message to commutator
    m_pCommutator->onMessageReceived(nSessionId, message);
  } else {
    BaseModule::onMessageReceived(nSessionId, message);
  }
}

void Ship::handleNavigationMessage(uint32_t nSessionId, spex::INavigation const& message)
{
  switch (message.choice_case()) {
    case spex::INavigation::kPositionRequest: {
      spex::INavigation navigation;
      spex::INavigation_GetPositionResponse* pBody =
          navigation.mutable_positionresponse();
      pBody->set_x(getPosition().x);
      pBody->set_y(getPosition().y);
      pBody->set_vx(getVelocity().getPosition().x);
      pBody->set_vy(getVelocity().getPosition().y);
      sendToClient(nSessionId, navigation);
      break;
    }
    default: {
      break;
    }
  }
}

} // namespace modules
