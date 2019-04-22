#pragma once

#include <stdint.h>
#include <memory>

#include <Protocol.pb.h>

namespace network
{

using MessagePtr = uint8_t const*;

class ITerminal;
class IChannel;
using ITerminalPtr = std::shared_ptr<ITerminal>;
using IChannelPtr  = std::shared_ptr<IChannel>;

class IProtobufTerminal;
class IProtobufChannel;
using IProtobufTerminalPtr     = std::shared_ptr<IProtobufTerminal>;
using IProtobufTerminalWeakPtr = std::weak_ptr<IProtobufTerminal>;
using IProtobufChannelPtr      = std::shared_ptr<IProtobufChannel>;
using IProtobufChannelWeakPtr  = std::weak_ptr<IProtobufChannel>;

class ITerminal
{
public:
  virtual ~ITerminal() = default;

  virtual void onMessageReceived(MessagePtr pMessage, size_t nLength) = 0;

  virtual void attachToChannel(IChannelPtr pChannel) = 0;
  virtual void detachFromChannel() = 0;
};


class IChannel
{
public:
  virtual ~IChannel() = default;

  virtual bool sendMessage(MessagePtr pMessage, size_t nLength) = 0;

  virtual bool isValid() const = 0;

  virtual void attachToTerminal(ITerminalPtr pTerminal) = 0;
  virtual void detachFromTerminal() = 0;
};


class IProtobufTerminal
{
public:
  virtual ~IProtobufTerminal() = default;

  virtual void onMessageReceived(spex::CommandCenterMessage&& message) = 0;

  virtual void attachToChannel(IProtobufChannelPtr pChannel) = 0;
  virtual void detachFromChannel() = 0;
};


class IProtobufChannel
{
public:
  virtual ~IProtobufChannel() = default;

  virtual bool sendMessage(spex::CommandCenterMessage const& message) = 0;

  virtual void attachToTerminal(IProtobufTerminalPtr pTerminal) = 0;
  virtual void detachFromTerminal() = 0;
};

} // namespace network
