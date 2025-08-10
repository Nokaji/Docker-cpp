/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#include "network_manager.h"
#include "docker.h"
#include "docker_compose_parser.h"
#include "types/network_types.h"
#include "utils/curl.h"
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <functional>

NetworkManager::NetworkManager(std::shared_ptr<DockerClient> dockerClient) : dockerClient(dockerClient) {
    if (!this->dockerClient) {
        throw std::runtime_error("Docker client is not initialized.");
    }
}

std::vector<networkTypes::NetworkListResponse> NetworkManager::list(std::map<std::string, std::string> filters) {
    ReqUEST request(fmt::format("{}/networks", dockerClient->getDockerApiUrl()), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_GET);

    // if (!filters.empty()) {
    //     nlohmann::json filterJson = filters;
    //     request.setParameters({
    //         {"filters", filterJson.dump()} // Serialize filters to JSON string
    //     });
    // }

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 200) {
        throw std::runtime_error("Failed to list networks: " + response->error_message);
    }

    std::vector<networkTypes::NetworkListResponse> networks;
    try {
        auto jsonResponse = nlohmann::json::parse(response->body);
        networks.reserve(jsonResponse.size());

        for (const auto &item : jsonResponse) {
            networks.emplace_back(item.get<networkTypes::NetworkListResponse>());
        }

        std::cout << "Parsed " << networks.size() << " networks." << std::endl;

        return networks;

    } catch (const nlohmann::json::exception &e) {
        std::cerr << "Failed to parse network list response: " << e.what() << std::endl;
        return {};
    }
}

std::shared_ptr<networkTypes::NetworkListResponse> NetworkManager::inspect(const std::string &id) {
    ReqUEST request(fmt::format("{}/networks/{}", dockerClient->getDockerApiUrl(), id), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_GET);

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 200) {
        throw std::runtime_error("Failed to inspect network: " + response->error_message);
    }

    try {
        auto jsonResponse = nlohmann::json::parse(response->body);
        std::shared_ptr<networkTypes::NetworkListResponse> network = std::make_shared<networkTypes::NetworkListResponse>(jsonResponse.get<networkTypes::NetworkListResponse>());
        return network;
    } catch (const nlohmann::json::exception &e) {
        std::cerr << "Failed to parse network inspect response: " << e.what() << std::endl;
        return nullptr;
    }
}

void NetworkManager::remove(const std::string &id) {
    ReqUEST request(fmt::format("{}/networks/{}", dockerClient->getDockerApiUrl(), id), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_DELETE);

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 204) {
        throw std::runtime_error("Failed to remove network: " + response->error_message);
    }
}

std::map<std::string, std::string> NetworkManager::create(const networkTypes::NetworkConfig &config) {
    ReqUEST request(fmt::format("{}/networks/create", dockerClient->getDockerApiUrl()), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_POST);
    request.setHeader("Content-Type: application/json");

    nlohmann::json configJson = config; // Utilisation de la conversion JSON
    request.setBody(configJson.dump());

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 201) {
        throw std::runtime_error("Failed to create network: " + response->body);
    }

    try {
        auto jsonResponse = nlohmann::json::parse(response->body);
        return jsonResponse.get<std::map<std::string, std::string>>(); // Conversion dynamique
    } catch (const nlohmann::json::exception &e) {
        std::cerr << "Failed to parse network creation response: " << e.what() << std::endl;
        return {};
    }
}

bool NetworkManager::createFromCompose(const std::string &composeFilePath) 
{
    if (composeFilePath.empty()) {
        throw std::runtime_error("Compose file path cannot be empty.");
    }

    // Parse the compose file
    auto networks = parseComposeNetworks(composeFilePath);
    if (networks.empty()) {
        throw std::runtime_error("No networks found in the compose file.");
    }

    try {
        for (const auto &networkPair : networks) {
            if (exists(networkPair.first)) {
                std::cout << "Network already exists: " << networkPair.first << std::endl;
                continue; // Skip if the network already exists
            }
            const std::string &networkName = networkPair.first;
            const networkTypes::NetworkCompose &networkConfig = networkPair.second;
            
            // Create basic network configuration
            networkTypes::NetworkConfig config;
            config.name = networkName;
            config.driver = networkConfig.driver.empty() ? "bridge" : networkConfig.driver;
            config.internal = networkConfig.internal;
            config.attachable = networkConfig.attachable;
            config.enable_ipv6 = networkConfig.enable_ipv6;
            
            // Basic IPAM configuration
            config.ipam.driver = networkConfig.ipam.driver.empty() ? "default" : networkConfig.ipam.driver;
            
            // If there's IPAM configuration in the compose file, add it to the config array
            if (!networkConfig.ipam.config.empty()) {
                config.ipam.config = networkConfig.ipam.config;
            }
            
            std::map<std::string, std::string> response = create(config);

            if (response.empty()) {
                std::cerr << "Failed to create network: " << networkName << std::endl;
                continue; // Skip to next network if creation fails
            }

            std::cout << "Network created: " << networkName << " with ID: " << response["Id"] << std::endl;
        }

        std::cout << "Networks created successfully from compose file." << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Exception while creating networks from compose file: " << e.what() << std::endl;
        return false;
    }
}

bool NetworkManager::exists(const std::string &id) {
    try {
        auto network = inspect(id);
        return network != nullptr;
    } catch (const std::runtime_error &e) {
        // If the network does not exist, an exception will be thrown
        return false;
    }
}

nlohmann::json NetworkManager::listAsJson(std::map<std::string, std::string> filters) {
    std::vector<networkTypes::NetworkListResponse> networks = list(filters);
    nlohmann::json result = nlohmann::json::array();
    
    for (const auto& network : networks) {
        nlohmann::json networkJson;
        to_json(networkJson, network);
        result.push_back(networkJson);
    }
    
    return result;
}

void NetworkManager::connect(const std::string &networkId, const std::string &containerId, networkTypes::EndpointConfigNetwork &endpointConfig) {
    ReqUEST request(fmt::format("{}/networks/{}/connect", dockerClient->getDockerApiUrl(), networkId), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_POST);
    request.setHeader("Content-Type: application/json");

    nlohmann::json connectJson = {
        {"Container", containerId,
        "EndpointConfig", endpointConfig}
    };
    request.setBody(connectJson.dump());

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 200) {
        throw std::runtime_error("Failed to connect container to network: " + response->error_message);
    }
}

void NetworkManager::disconnect(const std::string &networkId, const std::string &containerId, bool force) {
    ReqUEST request(fmt::format("{}/networks/{}/disconnect", dockerClient->getDockerApiUrl(), networkId), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_POST);
    request.setHeader("Content-Type: application/json");

    nlohmann::json disconnectJson = {
        {"Container", containerId},
        {"Force", force}
    };
    request.setBody(disconnectJson.dump());

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 204) {
        throw std::runtime_error("Failed to disconnect container from network: " + response->error_message);
    }
}

void NetworkManager::prune(const std::map<std::string, std::string> &filters) {
    ReqUEST request(fmt::format("{}/networks/prune", dockerClient->getDockerApiUrl()), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_POST);
    
    if (!filters.empty()) {
        nlohmann::json filterJson = filters;
        request.setParameters({
            {"filters", filterJson.dump()} // Serialize filters to JSON string
        });
    }

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 200) {
        throw std::runtime_error("Failed to prune networks: " + response->error_message);
    }
}