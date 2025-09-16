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

#define PORT 1234
#define TIME_BETWEEN_BROADCAST 5

struct PlayerSettings {
    ENetPeer* peer;
    bool ready = false;

    // data received
    Vector2 position = Vector2(0, 0);
};

std::unordered_map<std::string, PlayerSettings> players;

void ParsePlayerInfo(std::string& username, std::string& player_data) {
    const size_t start = player_data.find('[');
    const size_t end = player_data.find(']');

    if (start == std::string::npos || end == std::string::npos || start > end) {
        std::cout << "Not correct position format" << std::endl;
        return;
    }

    std::string numbers = player_data.substr(start + 1, end - start - 1);

    // Split by comma
    std::stringstream ss(numbers);
    std::string token;
    std::vector<float> values;

    while (std::getline(ss, token, ',')) {
        values.push_back(std::stof(token));
    }

    if (values.size() != 2) {
        std::cout << "Expected exactly two numbers" << std::endl;
        return;
    }

    players[username].position.x = values[0];
    players[username].position.y = values[1];

    // std::cout << username << player_data << std::endl;
}

void SendPacket(ENetPeer* peer, std::string msg){
    ENetPacket* packet = enet_packet_create(msg.data(), msg.size(), ENET_PACKET_FLAG_RELIABLE);
    // Channel 0
    enet_peer_send(peer, 0, packet);
}

void ParseData(ENetEvent& event, ENetHost* server, int id, const char* data) {
    std::stringstream ss(data);
    std::string token;

    // Get data_type (first field)
    if (!std::getline(ss, token, '|')) return;
    int data_type = std::stoi(token);

    std::string username;
    std::string extra;

    switch (data_type) {
        case 1: {
            // 1|username
            if (std::getline(ss, username)) {
                if (players.find(username) != players.end()) {
                    std::cout <<  "Username: " << username << " is already present in the game" << std::endl;
                }
                else {
                    std::cout << username << " Registered." << std::endl;
                    players[username] = PlayerSettings{event.peer};
                }
            }
            break;
        }
        case 2: {
            // 2|username|ready
            if (std::getline(ss, username, '|') && std::getline(ss, extra)) {
                players[username].ready = std::stoi(extra);
                std::cout << username << " ready:" << players[username].ready << std::endl;
            }
            break;
        }
        case 3: {
            // 3|username|player_dat
            if (std::getline(ss, username, '|') && std::getline(ss, extra)) {
                std::string player_data = extra;
                if (players.find(username) == players.end()) {
                    std::cout << username << ": player not in game\n" << std::endl;
                    break;
                }

                ParsePlayerInfo(username, player_data);
            }
            break;
        }
        case 4: {
            // 4|username
            if (std::getline(ss, username)) {
                std::cout << username << " Disconnected." << std::endl;
                players.erase(username);
            }
            break;
        }
    }
}

bool EveryoneReady() {
    if (players.size() < 2) return false;
    for (auto& p : players) {
        if (!p.second.ready) return false;
    }
    return true;
}

void BroadcastPositions() {
    while (true) {
        std::string send_data;

        if (EveryoneReady()) {
            send_data = "AllReady";
            for (auto& [username, settings] : players) {
                SendPacket(settings.peer, send_data);
            }
            continue;
        }

        send_data = "3|" + std::to_string(players.size()) + "|";
        for (auto& [username, settings] : players) {
            send_data += "[" + username + ":" +
                         std::to_string(settings.position.x) + "," +
                         std::to_string(settings.position.y) + "]|";
        }
        send_data.pop_back(); // remove last '|'

        for (auto& [username, settings] : players) {
            SendPacket(settings.peer, send_data);
        }

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

    std::thread thread(BroadcastPositions);

    while(true){
        while(enet_host_service(server, &event, 1000) > 0){
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                printf("New Client Connected %x:%u\n",
                    event.peer->address.host,
                    event.peer->address.port
                 );
                SendPacket(event.peer, "Received - ACK");
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                // printf("%s\n", event.packet->data);

                ParseData(event, server, -1, reinterpret_cast<char *>(event.packet->data));
                enet_packet_destroy(event.packet);

                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%x:%u disconnected.\n",
                    event.peer->address.host,
                    event.peer->address.port
                 );

                // If sending data, set it to null
                event.peer->data = nullptr;
                break;
            
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