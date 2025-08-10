/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "types/network_types.h"
#include "docker_compose_parser.h"

ComposeConfig parseComposeFile(const std::string& filePath) {
    YAML::Node config = YAML::LoadFile(filePath);
    ComposeConfig result;
    std::map<std::string, Service> services;
    
    if (config["services"]) {
        for (const auto& serviceNode : config["services"]) {
            std::string serviceName = serviceNode.first.as<std::string>();
            Service service;
            
            auto serviceConfig = serviceNode.second;
            
            if (serviceConfig["image"]) {
                service.image = serviceConfig["image"].as<std::string>();
            }
            
            if (serviceConfig["ports"]) {
                for (const auto& port : serviceConfig["ports"]) {
                    service.ports.push_back(port.as<std::string>());
                }
            }
            
            if (serviceConfig["volumes"]) {
                for (const auto& volume : serviceConfig["volumes"]) {
                    service.volumes.push_back(volume.as<std::string>());
                }
            }

            if (serviceConfig["networks"]) {
                auto networkNode = serviceConfig["networks"];
                
                if (networkNode.IsSequence()) {
                    // Format array: ["network1", "network2"]
                    for (const auto& network : networkNode) {
                        service.networks.push_back(network.as<std::string>());
                    }
                } else if (networkNode.IsMap()) {
                    // Format object: {network1: {}, network2: {}}
                    for (const auto& network : networkNode) {
                        service.networks.push_back(network.first.as<std::string>());
                    }
                }
            }

            if (serviceConfig["depends_on"]) {
                auto dependsNode = serviceConfig["depends_on"];
                
                if (dependsNode.IsSequence()) {
                    // Format array: ["service1", "service2"]
                    for (const auto& dependency : dependsNode) {
                        service.depends_on.push_back(dependency.as<std::string>());
                    }
                } else if (dependsNode.IsMap()) {
                    // Format object: {service1: {condition: "service_started"}}
                    for (const auto& dependency : dependsNode) {
                        service.depends_on.push_back(dependency.first.as<std::string>());
                    }
                }
            }
            
            if (serviceConfig["environment"]) {
                auto envNode = serviceConfig["environment"];
                if (envNode.IsMap()) {
                    // Format object: {KEY: VALUE, KEY2: VALUE2}
                    for (const auto& env : envNode) {
                        std::string key = env.first.as<std::string>();
                        std::string value = env.second.as<std::string>();
                        service.environment[key] = value;
                    }
                }
            }
            
            services[serviceName] = service;
        }
    }

    if (config["networks"]) {
        for (const auto& networkNode : config["networks"]) {
            std::string networkName = networkNode.first.as<std::string>();
            networkTypes::NetworkCompose network;
            network.name = networkName;
            
            auto networkConfig = networkNode.second;
            
            // Parse driver
            if (networkConfig["driver"]) {
                network.driver = networkConfig["driver"].as<std::string>();
            }
            
            // Parse external
            if (networkConfig["external"]) {
                network.external = networkConfig["external"].as<bool>();
            } else {
                network.external = false; // Default to false if not specified
            }
            
            // Parse IPAM
            if (networkConfig["ipam"]) {
                auto ipamNode = networkConfig["ipam"];
                
                if (ipamNode["driver"]) {
                    network.ipam.driver = ipamNode["driver"].as<std::string>();
                }

                if (ipamNode["config"]) {
                    for (const auto& config : ipamNode["config"]) {
                        networkTypes::IPAMConfigNetwork ipamConfig;
                        if (config["subnet"]) {
                            ipamConfig.subnet = config["subnet"].as<std::string>();
                        }
                        if (config["ip_range"]) {
                            ipamConfig.ip_range = config["ip_range"].as<std::string>();
                        }
                        if (config["gateway"]) {
                            ipamConfig.gateway = config["gateway"].as<std::string>();
                        }
                        if (config["aux_addresses"]) {
                            for (const auto& aux : config["aux_addresses"]) {
                                std::string key = aux.first.as<std::string>();
                                std::string value = aux.second.as<std::string>();
                                ipamConfig.aux_addresses[key] = value;
                            }
                        }
                        network.ipam.config.push_back(ipamConfig);
                    }
                }
                
                if (ipamNode["options"]) {
                    for (const auto& option : ipamNode["options"]) {
                        std::string key = option.first.as<std::string>();
                        std::string value = option.second.as<std::string>();
                        network.ipam.options[key] = value;
                    }
                }
            }
            
            // Parse labels
            if (networkConfig["labels"]) {
                for (const auto& label : networkConfig["labels"]) {
                    std::string key = label.first.as<std::string>();
                    std::string value = label.second.as<std::string>();
                    network.labels[key] = value;
                }
            }
            
            // Parse driver_opts
            if (networkConfig["driver_opts"]) {
                for (const auto& opt : networkConfig["driver_opts"]) {
                    std::string key = opt.first.as<std::string>();
                    std::string value = opt.second.as<std::string>();
                    network.driver_opts[key] = value;
                }
            }
            
            // Parse enable_ipv6
            if (networkConfig["enable_ipv6"]) {
                network.enable_ipv6 = networkConfig["enable_ipv6"].as<bool>();
            }
            
            if (networkConfig["enable_ipv4"]) {
                network.enable_ipv4 = networkConfig["enable_ipv4"].as<bool>();
            } else {
                network.enable_ipv4 = true; // Default to true if not specified
            }

            // Parse attachable
            if (networkConfig["attachable"]) {
                network.attachable = networkConfig["attachable"].as<bool>();
            }else {
                network.attachable = false; // Default to false if not specified
            }
            
            // Parse internal
            if (networkConfig["internal"]) {
                network.internal = networkConfig["internal"].as<bool>();
            } else {
                network.internal = false; // Default to false if not specified
            }
            
            result.networks[networkName] = network;
        }
    }

    if (config["volumes"]) {
        for (const auto& volumeNode : config["volumes"]) {
            std::string volumeName = volumeNode.first.as<std::string>();
            // You can add volume-specific parsing here if needed
            // Currently, we are not storing volumes in the Service struct
        }
    }
    
    result.services = services;
    return result;
}

std::map<std::string, networkTypes::NetworkCompose> parseComposeNetworks(const std::string& filePath) {
    return parseComposeFile(filePath).networks;
}