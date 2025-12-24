#include <iostream>
#include <raylib.h>
#include <enet/enet.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <regex>
#include <chrono>
#include <thread>

#include "../common/common_headers.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace nlohmann::literals;

#define PORT 1234
#define TIME_BETWEEN_BROADCAST 5

struct PlayerSettings {
    ENetPeer* peer = nullptr;
    bool ready = false;

    // data received
    Vector2 position = Vector2(0, 0);
    float angle = 90 * DEG2RAD;
};

std::unordered_map<std::string, PlayerSettings> players;

void ParsePlayerInfo(json p, ENetPeer* peer) {
    const auto username = p["name"].get<std::string>();
    players[username].position.x = p["px"].get<float>();
    players[username].position.y = p["py"].get<float>();
    players[username].angle = p["angle"].get<float>();

    players[username].peer = peer;
}


void ParseData(ENetEvent& event, ENetHost* server, int id, const char* data_raw) {
    json data;
    try {
        data = json::parse(data_raw);
    } catch (const std::exception& e) {
        std::cout << e.what() << "Invalid format" << std::endl;
        return;
    }

    const auto command = data[0].get<CommandType>();

    switch (command)
    {
    case REGISTER:
        {
            const auto username = data[1].get<std::string>();
            if (players.contains(username)) {
                std::cout <<  "Username: " << username << " is already present in the game" << std::endl;
            }
            else {
                std::cout << username << " Registered." << std::endl;
                players[username] = PlayerSettings{event.peer};
            }
            break;
        }
    case READY:
        {
            // {
            json info = data[1];
            const auto username = info["username"].get<std::string>();
            const auto ready = info["ready"].get<bool>();
            players[username].ready = ready;
            std::cout << username << " ready:" << players[username].ready << std::endl;

            break;
        }
    case PLAYER_INFO:
        {
            auto username = data[1]["name"].get<std::string>();
            ParsePlayerInfo(data[1], event.peer);
            break;
        }
    case DISCONNECT:
        {
            const auto username = data[1].get<std::string>();
            players.erase(username);
            std::cout << username << " Disconnected." << std::endl;
        }
        break;
    }
}

// Check if all players are READY
bool EveryoneReady() {
    // TODO: 2 -> Lobby size
    if (players.size() < 2) return false;

    // Return if not all ready
    bool all_ready = true;
    for (const auto& p : players) {
        if (!p.second.ready)
        {
            all_ready = false;
            std::cout << "Player " + p.first + " is not ready\n";
        }
    }

    return all_ready;
}

void SendPacket(ENetPeer* peer, std::string msg){
    ENetPacket* packet = enet_packet_create(msg.data(), msg.size(), ENET_PACKET_FLAG_RELIABLE);
    // Channel 0
    enet_peer_send(peer, 0, packet);
}

[[noreturn]] void BroadcastPositions() {
    while (true) {
        // TODO: Call this when START is pressed
        // All ready
        // if (EveryoneReady()) {
        //     // All ready, with optional data
        //     json send_data = {ALL_READY, ""};
        //     // Send to all
        //     for (const auto &settings: players | std::views::values) {
        //         SendPacket(settings.peer, send_data.dump());
        //     }
        //     continue;
        // }

        // Placeholder
        json all_players_info = {
            {"size", std::to_string(players.size())},
            {"players", {}}
        };

        for (auto& [username, settings] : players)
        {
            json player;
            player["name"] = username;
            player["px"] = settings.position.x;
            player["py"] = settings.position.y;
            all_players_info["players"].push_back(player);
        }

        // std::string send_data = "3|" + std::to_string(players.size()) + "|";
        // for (auto& [username, settings] : players) {
        //     send_data += "[" + username + ":" +
        //                  std::to_string(settings.position.x) + "," +
        //                  std::to_string(settings.position.y) + "]|";
        // }
        // send_data.pop_back(); // remove last '|'

        json final_command = {ALL_PLAYERS_INFO, all_players_info};
        std::cout << final_command << std::endl;

        // Send to all connected players
        for (auto& [username, settings] : players) {
            SendPacket(settings.peer, final_command.dump());
        }

        // Wait for a while to simulate lag, and also keep it stable
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_BETWEEN_BROADCAST));
    }
}

int main(int argc, char ** argv){
    if(enet_initialize() != 0){
        fprintf(stderr, "Error initialising Enet\n");
        return EXIT_FAILURE;
    }
    // At Exit, de init
    atexit(enet_deinitialize);

    // Representation of address of host
    ENetAddress address;
    // Receiving data for example
    ENetEvent event;
    // Server object
    ENetHost* server;

    // Can connect from anywhere, MACRO for 0
    address.host = ENET_HOST_ANY;
    address.port = PORT;

    // Player limit, channels, bw limit (0 -> unlimited)
    server = enet_host_create(&address, 32, 1, 0, 0);

    if(server == nullptr){
        fprintf(stderr, "Error creating server\n");
        return EXIT_FAILURE;
    }

    // Game loop START
    printf("Server Started on localhost:%d\n", PORT);

    std::thread thread(BroadcastPositions);

    while(true){
        while(enet_host_service(server, &event, 1000) > 0){
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                printf("New Client Connected %x:%u\n",
                    event.peer->address.host,
                    event.peer->address.port
                 );

                // Send Acknowledge with optional message
                json msg = {ACK, ""};
                SendPacket(event.peer, msg.dump());
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                ParseData(event, server, -1, reinterpret_cast<char *>(event.packet->data));
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                printf("%x:%u disconnected.\n",
                    event.peer->address.host,
                    event.peer->address.port
                 );

                // If sending data, set it to null
                event.peer->data = nullptr;
                break;
            }

            default:
                break;
            }
        }
    }
    thread.join();
    // Game loop END

    enet_host_destroy(server);

    return EXIT_SUCCESS;

}
