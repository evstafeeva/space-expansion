#include "CommutatorTests.h"
#include <Autotests/Mocks/MockedBaseModule.h>
#include <Conveyor/Proceeders.h>

namespace autotests {

CommutatorTests::CommutatorTests()
  : m_Conveyor(1),
    m_fConveyorProceeder([this](){ proceedEnviroment(); })
{}

void CommutatorTests::SetUp()
{
  //   ?  ?  ?
  //   |  |  |
  // +----------+   +-----------+
  // | pTunnels |   |  pClient  |
  // +----------+   +-----------+
  //      |               |
  //      +-------+-------+                           ?   ?   ?
  //              |                                   |   |   |
  //      +----------------+                     ++===============++
  //      | pProtobufPipe  |                     ||  pCommutator  ||
  //      +----------------+                     ++===============++
  //              |                                       |
  //              |              +----------+             |
  //              +--------------| pChannel |-------------+
  //                             +----------+

  // Creating components
  m_pCommutatorManager = std::make_shared<modules::CommutatorManager>();
  m_pCommutatator      = std::make_shared<modules::Commutator>();
  m_pChannel           = std::make_shared<BidirectionalChannel>();

  m_pChannel           = std::make_shared<BidirectionalChannel>();
  m_pClient            = std::make_shared<client::ClientCommutator>();
  m_pProtobufPipe      = std::make_shared<client::SyncPipe>();

  // Setting up components
  m_Conveyor.addLogicToChain(m_pCommutatorManager);
  m_pProtobufPipe->setProceeder(m_fConveyorProceeder);

  // Linking components
  m_pCommutatator->attachToChannel(m_pChannel);
  m_pChannel->attachToTerminal(m_pCommutatator);
  m_pChannel->attachToClientSide(m_pProtobufPipe);
  m_pProtobufPipe->attachToDownlevel(m_pChannel);
  m_pClient->attachToChannel(m_pProtobufPipe);
}

void CommutatorTests::TearDown()
{
  m_pProtobufPipe->detachDownlevel();
  m_pChannel->detachFromTerminal();
  m_pCommutatator->detachFromChannel();
}

void CommutatorTests::proceedEnviroment()
{
  m_Conveyor.proceed(10000);
}

//========================================================================================
// Tests
//========================================================================================

TEST_F(CommutatorTests, GetSlotsCount)
{
  uint32_t nExpectedSlotsCount = 16;
  for (uint32_t i = 0; i < nExpectedSlotsCount; ++i) {
    m_pCommutatator->attachModule(std::make_shared<MockedBaseModule>());
  }
  ASSERT_TRUE(m_pClient->getTotalSlots(nExpectedSlotsCount));
}

TEST_F(CommutatorTests, OpenTunnelSuccessCase)
{
  // 1. Attaching nTotalSlots modules to commutator
  // 2. Opening 10 tunnels to each module

  uint32_t nTotalSlots = 16;

  // 1. Attaching 3 modules to commutator
  for (uint32_t nSlotId = 0; nSlotId < nTotalSlots; ++nSlotId) {
    m_pCommutatator->attachModule(std::make_shared<MockedBaseModule>());
  }

  // 2. Opening 10 tunnels to each module
  for (uint32_t nSlotId = 0; nSlotId < nTotalSlots; ++nSlotId) {
    for (uint32_t nCount = 1; nCount <= 10; ++nCount)
      ASSERT_TRUE(m_pClient->openTunnel(nSlotId))
          << "failed to open tunnel #" << nCount << " for slot #" << nSlotId;
  }
}

TEST_F(CommutatorTests, OpenTunnelToNonExistingSlot)
{
  // 1. Attaching nTotalSlots modules to commutator
  // 2. Trying to open tunnel to slot #nTotalSlots (should failed)

  uint32_t nTotalSlots = 4;
  for (uint32_t nSlotId = 0; nSlotId < nTotalSlots; ++nSlotId) {
    m_pCommutatator->attachModule(std::make_shared<MockedBaseModule>());
  }
  ASSERT_TRUE(m_pClient->sendOpenTunnel(nTotalSlots));
  ASSERT_TRUE(m_pClient->waitOpenTunnelFailed());
}

TEST_F(CommutatorTests, TunnelingMessage)
{
  // 1. Attaching commutator to MockedCommutator
  // 2. Doing 5 times:
  //   2.1. Opening new tunnel TO mocked commutator
  //   2.2. Opening new tunnel ON mocked commutator (via tunnel from 2.1)

  // 1. Attaching commutator to MockedCommutator
  MockedCommutatorPtr pMockedCommutator = std::make_shared<MockedCommutator>();
  pMockedCommutator->setEnviromentProceeder(m_fConveyorProceeder);
  pMockedCommutator->attachToChannel(m_pCommutatator);
  m_pCommutatator->attachModule(pMockedCommutator);

  // 2. Doing 5 times:
  for(uint32_t nSlotId = 5; nSlotId < 10; ++nSlotId) {
    //   2.1. Opening new tunnel TO mocked commutator
    client::TunnelPtr pTunnel = m_pClient->openTunnel(0);
    ASSERT_TRUE(pTunnel);

    //   2.2. Opening new tunnel ON mocked commutator (via tunnel from 2.1)
    client::ClientCommutatorPtr pAnotherClient =
        std::make_shared<client::ClientCommutator>();
    pAnotherClient->attachToChannel(pTunnel);
    ASSERT_TRUE(pAnotherClient->sendOpenTunnel(nSlotId));
    ASSERT_TRUE(pMockedCommutator->waitOpenTunnel(pTunnel->getTunnelId(), nSlotId));
    ASSERT_TRUE(pMockedCommutator->sendOpenTunnelFailed(
                  pTunnel->getTunnelId(), spex::ICommutator::INVALID_SLOT));
    ASSERT_TRUE(pAnotherClient->waitOpenTunnelFailed());
  }
}

TEST_F(CommutatorTests, TunnelingMessageToOfflineModule)
{
  // 1. Attaching commutator to MockedCommutator
  MockedCommutatorPtr pMockedCommutator = std::make_shared<MockedCommutator>();
  pMockedCommutator->attachToChannel(m_pCommutatator);
  m_pCommutatator->attachModule(pMockedCommutator);
  pMockedCommutator->setEnviromentProceeder(m_fConveyorProceeder);

  // 2. Opening tunnel to mocked commutator
  client::TunnelPtr pTunnel = m_pClient->openTunnel(0);
  ASSERT_TRUE(pTunnel);

  // 3. Put mocked commutator to offline and sending any command
  client::ClientCommutatorPtr pAnotherClient =
      std::make_shared<client::ClientCommutator>();
  pAnotherClient->attachToChannel(pTunnel);
  pMockedCommutator->putOffline();
  ASSERT_TRUE(pAnotherClient->sendOpenTunnel(1));
  ASSERT_FALSE(pMockedCommutator->waitAny(pTunnel->getTunnelId(), 50));
}

} // namespace autotests
