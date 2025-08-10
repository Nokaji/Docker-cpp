/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#pragma once

#include <yaml-cpp/yaml.h>
#include "types/network_types.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Service {
    std::string image;
    std::string container_name;
    std::map<std::string, std::string> environment;
    std::vector<std::string> ports;
    std::string restart;
    std::vector<std::string> volumes;
    std::vector<std::string> networks;
    std::vector<std::string> depends_on;
};

struct ComposeConfig {
    std::map<std::string, Service> services;
    std::map<std::string, networkTypes::NetworkCompose> networks;
};
    
ComposeConfig parseComposeFile(const std::string& filePath);
std::map<std::string, networkTypes::NetworkCompose> parseComposeNetworks(const std::string& filePath);