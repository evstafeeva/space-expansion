#pragma once

#include <queue>
#include <map>
#include <memory>
#include <functional>
#include <Protocol.pb.h>
#include <Network/Interfaces.h>

namespace autotests {

class ProtobufSyncPipe;
using ProtobufSyncPipePtr = std::shared_ptr<ProtobufSyncPipe>;

class ProtobufSyncPipe :
    public network::IProtobufChannel,
    public network::IProtobufTerminal
{
  using MessagesQueue = std::queue<spex::Message>;
  using SessionsQueue = std::map<uint32_t, MessagesQueue>;

public:
  static ProtobufSyncPipePtr MakeMockedChannelPipe();
  static ProtobufSyncPipePtr MakeMockedTeminalPipe();

  enum Mode {
    eMockedChannelMode,
    eMockedTerminalMode
  };

  ProtobufSyncPipe(Mode eMode) : m_eMode(eMode) {}

  virtual ~ProtobufSyncPipe() override = default;

  void setEnviromentProceeder(std::function<void()>&& fProceeder)
  { m_fEnviromentProceeder = std::move(fProceeder); }

  // All ICommutator::Message'es, that has been received in nSessionId, will be
  // deencapsulated in forwarded to uplevel tunnel
  void attachToUplevelTunnel(uint32_t nSessionId, ProtobufSyncPipePtr pUplevelTunnel);

  // Waiting message in already opened session nSessionId
  bool waitAny(uint32_t nSessionId, spex::Message &out, uint16_t nTimeoutMs = 500);
  // Waiting for any message, that creates new session
  bool waitAny(uint32_t* pNewSessionId, spex::Message &out, uint16_t nTimeoutMs = 500);

  bool wait(uint32_t nSessionId, spex::ICommutator &out, uint16_t nTimeoutMs = 500);
  bool wait(uint32_t* pSessionId, spex::ICommutator &out, uint16_t nTimeoutMs = 500);
  bool wait(uint32_t nSessionId, spex::INavigation &out, uint16_t nTimeoutMs = 500);
  bool wait(uint32_t* pSessionId, spex::INavigation &out, uint16_t nTimeoutMs = 500);

  // Expect, that no message will be received in session
  bool expectSilence(uint32_t nSessionId, uint16_t nTimeoutMs);

  // overrides from ITerminal interface
  bool openSession(uint32_t /*nSessionId*/) override;
  void onMessageReceived(uint32_t nSessionId, spex::Message &&frame) override;
  void onSessionClosed(uint32_t /*nSessionId*/) override;
  void attachToChannel(network::IProtobufChannelPtr pChannel) override
  { m_pAttachedChannel = pChannel; }
  void detachFromChannel() override { m_pAttachedChannel.reset(); }

  // overrides from IChannel interface
  bool send(uint32_t nSessionId, spex::Message &&frame) const override;
  void closeSession(uint32_t /*nSessionId*/) override;
  bool isValid() const override { return true; }
  void attachToTerminal(network::IProtobufTerminalPtr pTerminal) override
  { m_pAttachedTerminal = pTerminal; }
  void detachFromTerminal() override { m_pAttachedTerminal.reset(); }

protected:
  void proxyOrStoreMessage(uint32_t nSessionId, spex::Message&& message) const;

private:
  bool waitConcrete(uint32_t nSessionId, spex::Message::ChoiceCase eExpectedChoice,
                    spex::Message &out, uint16_t nTimeoutMs = 500);
  bool waitConcrete(uint32_t* pSessionId, spex::Message::ChoiceCase eExpectedChoice,
                    spex::Message &out, uint16_t nTimeoutMs = 500);

private:
  Mode m_eMode;
  // Used only in eMockedTerminalMode
  network::IProtobufChannelPtr m_pAttachedChannel;
  // Used only in eMockedChannelMode
  network::IProtobufTerminalPtr m_pAttachedTerminal;

  std::function<void()>      m_fEnviromentProceeder;
  mutable SessionsQueue      m_Sessions;
  mutable std::set<uint32_t> m_newSessionIds;
  mutable std::set<uint32_t> m_knownSessionsIds;
};

} // namespace autotest
