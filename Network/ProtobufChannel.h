#pragma once

#include <memory>
#include "Interfaces.h"
#include "BufferedTerminal.h"

namespace network {

class ProtobufChannel : public BufferedTerminal, public IProtobufChannel
{
public:
  // IProtobufChannel interface
  bool send(uint32_t nSessionId, spex::ICommutator&& message) override;
  void attachToTerminal(IProtobufTerminalPtr pTerminal) override;
  void detachFromTerminal() override;
  void closeSession(uint32_t nSessionId) override;
  bool isValid() const override;

protected:
  void handleMessage(uint32_t nSessionId, BinaryMessage const& message) override;

private:
  IProtobufTerminalPtr m_pTerminal;
};

using ProtobufChannelPtr = std::shared_ptr<ProtobufChannel>;

} // namespace network
