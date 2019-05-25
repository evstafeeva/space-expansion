#pragma once

#include <Newton/PhysicalObject.h>
#include <Modules/BaseModule.h>
#include <Protocol.pb.h>

namespace modules {

class Engine : public BaseModule
{
public:
  Engine(uint32_t maxThrust);

  void installOn(newton::PhysicalObject* pPlatform);

protected:
  // override from BaseModule
  void handleEngineMessage(uint32_t, spex::IEngine const&) override;

  void getSpecification(uint32_t nSessionId) const;
  void setThrust(spex::IEngine::SetThrust const& req);
  void getThrust(uint32_t nSessionId) const;

private:
  newton::PhysicalObject* m_pPlatform = nullptr;
  size_t   m_nThrustVectorId = size_t(-1);
  uint32_t m_maxThrust       = 0;
};

} // namespace modules