// MagicPodsCore: https://github.com/steam3d/MagicPodsCore
// Copyright: 2020-2025 Aleksandr Maslov <https://magicpods.app> & Andrei Litvintsev <a.a.litvintsev@gmail.com>
// License: GPL-3.0

#pragma once

#include "IRequest.h"
#include "ClientConnectionType.h"
#include "Event.h"
#include "BlockingQueue.h"

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#ifndef _WIN32
#include <sys/socket.h>
#endif
#ifndef _WIN32
#include <bluetooth/bluetooth.h>
#endif
#ifndef _WIN32
#include <bluetooth/l2cap.h>
#endif
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <thread>
#include <array>
#include <memory>
#include <optional>
#include <mutex>
#include <functional>

namespace MagicPodsCore {

    class Client {
    private:
        static const int CONNECTION_TO_SOCKET_ATTEMPTS_NUMBER = 1;

        std::string _address{};
        unsigned short _port{};
        std::string _serviceUuid{};

        ClientConnectionType _connectionType{};

        int _socket{};
        bool _isStarted{false};

        std::mutex _startStopMutex{};

        BlockingQueue<std::vector<unsigned char>> _outcomeMessagesQueue{};

        Event<std::vector<unsigned char>> _onReceivedDataEvent{};

    public:
        void Start(const std::function<void(Client&)>& justAfterStartLogic = {});
        void Stop();

        bool IsStarted() const {
            return _isStarted;
        }

        Event<std::vector<unsigned char>>& GetOnReceivedDataEvent() {
            return _onReceivedDataEvent;
        }

        void SendData(const std::vector<unsigned char>& data);

    private:
        inline bool ConnectToSocketL2CAP();
        inline bool ConnectToSocketRFCOMM();
        inline bool ConnectToSocket(int attemptsNumber);
        inline static std::optional<uint8_t> RetrieveServicePortRFCOMM(uint8_t* uuid, const char* deviceAddress);

    private:
        explicit Client(const std::string& address, unsigned short port, ClientConnectionType connectionType);
        explicit Client(const std::string& address, const std::string& serviceUuid, ClientConnectionType connectionType);

    public:
        static std::unique_ptr<Client> CreateL2CAP(const std::string& address, unsigned short port);
        static std::unique_ptr<Client> CreateRFCOMM(const std::string& address, const std::string& serviceUuid);
        ~Client();
    };
}
