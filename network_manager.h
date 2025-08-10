/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#pragma once
#include <memory>
#include <vector>
#include <map>
#include <string>
#include "types/network_types.h"
#include "types/container_types.h"

// Forward declaration
class DockerClient;

class NetworkManager 
{
    private:
        std::shared_ptr<DockerClient> dockerClient;
    public:
        explicit NetworkManager(std::shared_ptr<DockerClient> client);
        std::vector<networkTypes::NetworkListResponse> list(std::map<std::string, std::string> filters = {});
        
        // Méthode utilitaire pour obtenir les réseaux sous forme JSON
        nlohmann::json listAsJson(std::map<std::string, std::string> filters = {});
        
        std::shared_ptr<networkTypes::NetworkListResponse> inspect(const std::string &id);
        void remove(const std::string &id);

        bool exists(const std::string &id);

        std::map<std::string, std::string> create(const networkTypes::NetworkConfig &config);
        bool createFromCompose(const std::string &composeFilePath);
        void connect(const std::string &networkId, const std::string &containerId, networkTypes::EndpointConfigNetwork &endpointConfig);
        void disconnect(const std::string &networkId, const std::string &containerId, bool force = false);

        void prune(const std::map<std::string, std::string> &filters = {});
};