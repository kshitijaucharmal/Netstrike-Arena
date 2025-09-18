//
// Created by kshitij on 10/9/25.
//

#include "NetworkClient.hpp"
#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include "Global.hpp"

NetworkClient::NetworkClient() {

}

void parseData(const std::string& data) {
    size_t firstSep = data.find('|');
    if (firstSep == std::string::npos) return;

    size_t secondSep = data.find('|', firstSep + 1);
    if (secondSep == std::string::npos) return;

    size_t start = secondSep + 1;

    while (start < data.size()) {
        if (data[start] != '[') {
            start++;
            continue;
        }

        size_t end = data.find(']', start);
        if (end == std::string::npos) break;

        std::string block = data.substr(start + 1, end - start - 1);
        // block looks like: "name:x,y"

        size_t colon = block.find(':');
        size_t comma = block.find(',');

        auto& players = Global::Get().players;
        if (colon != std::string::npos && comma != std::string::npos) {
            std::string name = block.substr(0, colon);
            float x = std::stof(block.substr(colon + 1, comma - colon - 1));
            float y = std::stof(block.substr(comma + 1));

            if (players.contains(name)) {
                players[name].position = Vector2(x, y);
            }
            else {
                players[name] = PlayerSettings(Vector2(x, y));
            }
        }

        start = end + 1; // Move to next
    }
}

void* MsgLoop(void* client){
    while(true){
        ENetEvent event;
        while(enet_host_service((ENetHost*)client, &event, 0) > 0){
            switch (event.type)
            {
                case ENET_EVENT_TYPE_RECEIVE:
                    std::string pdata = reinterpret_cast<char *>(event.packet->data);

                    if (pdata == "AllReady") {
                        Global::Get().allReady = true;
                        continue;
                    }

                    parseData(pdata);

                    enet_packet_destroy(event.packet);
                    break;
            }
        }
    }
}

void NetworkClient::SendPacket(std::string data){
    ENetPacket* packet = enet_packet_create(data.c_str(), data.size() + 1, ENET_PACKET_FLAG_RELIABLE);
    // Channel 0
    enet_peer_send(peer, 0, packet);
}

int NetworkClient::ConnectToServer(const std::string host, const int port) {
    if(enet_initialize() != 0){
        fprintf(stderr, "Error initialising Enet\n");
        return EXIT_FAILURE;
    }
    // At Exit, de init
    atexit(enet_deinitialize);

    client = enet_host_create(nullptr, 1, 1, 0, 0);

    if(client == nullptr){
        fprintf(stderr, "Error creating client\n");
        return EXIT_FAILURE;
    }

    // Connect to which server
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    // Actual connecting
    // data is 0 for now
    peer = enet_host_connect(client, &address, 1, 0);

    if(peer == nullptr){
        fprintf(stderr, "No Available peers\n");
        return EXIT_FAILURE;
    }

    // Wait for 5 seconds
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT){
        puts("Connection to server succeeded");
    }
    else{
        enet_peer_reset(peer);
        puts("Connection to server failed");
        // Not necessarily crash
        return EXIT_SUCCESS;
        // TODO: Go back to reconnect
    }

    pthread_t thread;
    pthread_create(&thread, nullptr, &MsgLoop, client);

    std::cout << "Connected to : " << host << " on " << port << std::endl;
    return 0;
}

int NetworkClient::Disconnect() {
    enet_peer_disconnect(peer, 0);
    while (enet_host_service(client, &event, 1000) > 0)
    {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                puts("Disconnection Succeeded");
                break;

            default:
                break;
        }
    }


    return EXIT_SUCCESS;
}

ENetPeer* NetworkClient::ConnectToServerUI(Player *player) {
    bool ConnectWindow = true;
    while (ConnectWindow && !WindowShouldClose()) {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Debug
        // -----------------------------------------------------------------------------
        DrawFPS(10, 10);
        // -----------------------------------------------------------------------------

        // start ImGui HUD
        rlImGuiBegin();
        ImGui::Begin("Connection Settings");

        static char username[80];
        ImGui::InputText("Username", username, sizeof(username));
        player->username = username;

        static int server_ip[4] = {127, 0, 0, 1};
        ImGui::InputInt4("Server IP", server_ip);

        static int port = 1234;
        ImGui::InputInt("Port", &port);

        if (ImGui::Button("START")) {
            const std::string host = std::to_string(server_ip[0]) + "." + std::to_string(server_ip[1]) + "." + std::to_string(server_ip[2]) + "." + std::to_string(server_ip[3]);

            const int error_code = ConnectToServer(host, port);
            // Send Player username to register
            SendPacket(json{"REGISTER", player->username}.dump(4));

            if (!error_code) ConnectWindow = false;
            else {
                std::cout << "Error: " << error_code << std::endl;
            }
        }
        ImGui::End();

        // end ImGui Content
        rlImGuiEnd();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    return nullptr;
}