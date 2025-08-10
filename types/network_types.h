/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#pragma once

#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>

namespace networkTypes
{
    struct IPAMConfigNetwork
    {
        std::string subnet;
        std::string ip_range;
        std::string gateway;
        std::map<std::string, std::string> aux_addresses;
    };

    // Fonction de désérialisation custom pour IPAMConfigNetwork
    inline void from_json(const nlohmann::json& j, IPAMConfigNetwork& config) {
        config.subnet = j.value("Subnet", std::string{});
        config.ip_range = j.value("IPRange", std::string{});
        config.gateway = j.value("Gateway", std::string{});
        config.aux_addresses = j.value("AuxAddresses", std::map<std::string, std::string>{});
    }

    // Fonction de sérialisation custom pour IPAMConfigNetwork
    inline void to_json(nlohmann::json& j, const IPAMConfigNetwork& config) {
        j["Subnet"] = config.subnet;
        j["IPRange"] = config.ip_range;
        j["Gateway"] = config.gateway;
        j["AuxAddresses"] = config.aux_addresses;
    }

    struct IPAM
    {
        std::string driver = "default";
        std::vector<IPAMConfigNetwork> config;
        std::map<std::string, std::string> options;
    };

    // Fonction de désérialisation custom pour IPAM
    inline void from_json(const nlohmann::json& j, IPAM& ipam) {
        ipam.driver = j.value("driver", std::string{"default"});
        
        if (j.contains("config")) {
            const auto& configJson = j["config"];
            ipam.config.clear();
            
            if (configJson.is_array()) {
                // Si c'est un tableau, traiter chaque élément
                for (const auto& configItem : configJson) {
                    IPAMConfigNetwork ipamConfig;
                    if (configItem.contains("subnet")) {
                        ipamConfig.subnet = configItem["subnet"];
                    }
                    if (configItem.contains("ip_range")) {
                        ipamConfig.ip_range = configItem["ip_range"];
                    }
                    if (configItem.contains("gateway")) {
                        ipamConfig.gateway = configItem["gateway"];
                    }
                    if (configItem.contains("aux_addresses")) {
                        ipamConfig.aux_addresses = configItem["aux_addresses"];
                    }
                    ipam.config.push_back(ipamConfig);
                }
            } else if (configJson.is_object()) {
                // Si c'est un objet unique, le traiter comme un seul élément
                IPAMConfigNetwork ipamConfig;
                if (configJson.contains("subnet")) {
                    ipamConfig.subnet = configJson["subnet"];
                }
                if (configJson.contains("ip_range")) {
                    ipamConfig.ip_range = configJson["ip_range"];
                }
                if (configJson.contains("gateway")) {
                    ipamConfig.gateway = configJson["gateway"];
                }
                if (configJson.contains("aux_addresses")) {
                    ipamConfig.aux_addresses = configJson["aux_addresses"];
                }
                ipam.config.push_back(ipamConfig);
            }
        }
        
        if (j.contains("options")) {
            ipam.options = j["options"];
        }
    }

    // Fonction de sérialisation custom pour IPAM
    inline void to_json(nlohmann::json& j, const IPAM& ipam) {
        j["driver"] = ipam.driver;
        j["config"] = ipam.config;
        j["options"] = ipam.options;
    }

    struct NetworkConfig
    {
        std::string name;
        std::string id;
        std::string driver;
        std::string scope;
        bool enable_ipv4 = true;
        bool enable_ipv6 = false;
        IPAM ipam;
        bool internal = false;
        bool attachable = false;
        bool ingress = false;
        std::map<std::string, std::string> config_from;
        bool config_only = false;
    };

    // Fonction de désérialisation custom pour NetworkConfig
    inline void from_json(const nlohmann::json& j, NetworkConfig& config) {
        config.name = j.value("Name", std::string{});
        config.id = j.value("Id", std::string{});
        config.driver = j.value("Driver", std::string{});
        config.scope = j.value("Scope", std::string{});
        config.enable_ipv4 = j.value("EnableIPv4", true);
        config.enable_ipv6 = j.value("EnableIPv6", false);
        if (j.contains("IPAM")) {
            j["IPAM"].get_to(config.ipam);
        }
        config.internal = j.value("Internal", false);
        config.attachable = j.value("Attachable", false);
        config.ingress = j.value("Ingress", false);
        config.config_from = j.value("ConfigFrom", std::map<std::string, std::string>{});
        config.config_only = j.value("ConfigOnly", false);
    }

    // Fonction de sérialisation custom pour NetworkConfig
    inline void to_json(nlohmann::json& j, const NetworkConfig& config) {
        j["Name"] = config.name;
        j["Id"] = config.id;
        j["Driver"] = config.driver;
        j["Scope"] = config.scope;
        j["EnableIPv4"] = config.enable_ipv4;
        j["EnableIPv6"] = config.enable_ipv6;
        j["IPAM"] = config.ipam;
        j["Internal"] = config.internal;
        j["Attachable"] = config.attachable;
        j["Ingress"] = config.ingress;
        j["ConfigFrom"] = config.config_from;
        j["ConfigOnly"] = config.config_only;
    }

    struct NetworkCompose
    {
        std::string name;
        std::string driver = "bridge";
        bool internal = false;
        bool attachable = false;
        bool enable_ipv4 = true;
        bool enable_ipv6 = false;
        bool external = false;
        IPAM ipam;
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> driver_opts;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(NetworkCompose, 
            name, driver, internal, attachable, enable_ipv6, ipam, 
            labels, driver_opts);
    };

    struct NetworkCreateRequest : public NetworkConfig
    {
        // Network creation specific fields
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> options;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(NetworkCreateRequest, 
            name, id, driver, scope, enable_ipv4, enable_ipv6, ipam, 
            internal, attachable, ingress, config_from, config_only,
            labels, options);
    };

    struct NetworkContainer
    {
        std::string name;
        std::string endpoint_id;
        std::string mac_address;
        std::string ipv4_address;
        std::string ipv6_address;
    };

    // Fonction de désérialisation custom pour NetworkContainer
    inline void from_json(const nlohmann::json& j, NetworkContainer& container) {
        container.name = j.value("Name", std::string{});
        container.endpoint_id = j.value("EndpointID", std::string{});
        container.mac_address = j.value("MacAddress", std::string{});
        container.ipv4_address = j.value("IPv4Address", std::string{});
        container.ipv6_address = j.value("IPv6Address", std::string{});
    }

    // Fonction de sérialisation custom pour NetworkContainer
    inline void to_json(nlohmann::json& j, const NetworkContainer& container) {
        j["Name"] = container.name;
        j["EndpointID"] = container.endpoint_id;
        j["MacAddress"] = container.mac_address;
        j["IPv4Address"] = container.ipv4_address;
        j["IPv6Address"] = container.ipv6_address;
    }

    struct Peers
    {
        std::string name;
        std::string id;
    };

    // Fonction de désérialisation custom pour Peers
    inline void from_json(const nlohmann::json& j, Peers& peers) {
        peers.name = j.value("Name", std::string{});
        peers.id = j.value("ID", std::string{});
    }

    // Fonction de sérialisation custom pour Peers
    inline void to_json(nlohmann::json& j, const Peers& peers) {
        j["Name"] = peers.name;
        j["ID"] = peers.id;
    }

    struct NetworkListResponse : public NetworkConfig
    {
        std::string created;
        std::map<std::string, NetworkContainer> containers = {};
        std::map<std::string, std::string> labels = {};
        std::vector<Peers> peers = {};

        // From JSON conversion
        void from_json(const nlohmann::json& j) {
            // Use the NetworkConfig from_json function
            networkTypes::from_json(j, static_cast<NetworkConfig&>(*this));
            
            // Additional fields specific to NetworkListResponse
            created = j.value("Created", std::string{});
            
            if (j.contains("Containers")) {
                containers.clear();
                for (const auto& [key, value] : j["Containers"].items()) {
                    NetworkContainer container;
                    networkTypes::from_json(value, container);
                    containers[key] = container;
                }
            }
            
            labels = j.value("Labels", std::map<std::string, std::string>{});
            
            if (j.contains("Peers")) {
                peers.clear();
                for (const auto& peer : j["Peers"]) {
                    Peers p;
                    networkTypes::from_json(peer, p);
                    peers.push_back(p);
                }
            }
        }
    };

    // Fonction de désérialisation globale pour NetworkListResponse
    inline void from_json(const nlohmann::json& j, NetworkListResponse& response) {
        response.from_json(j);
    }

    // Fonction de sérialisation globale pour NetworkListResponse
    inline void to_json(nlohmann::json& j, const NetworkListResponse& response) {
        // Use the NetworkConfig to_json function
        networkTypes::to_json(j, static_cast<const NetworkConfig&>(response));
        
        // Add additional fields specific to NetworkListResponse
        j["Created"] = response.created;
        j["Containers"] = response.containers;
        j["Labels"] = response.labels;
        j["Peers"] = response.peers;
    }

    struct EndpointIPAMConfig {
        std::string ipv4_address;
        std::string ipv6_address;
        std::map<std::string, std::string> link_local_ips;  
    };

    // Fonction de désérialisation custom pour EndpointIPAMConfig
    inline void from_json(const nlohmann::json& j, EndpointIPAMConfig& config) {
        config.ipv4_address = j.value("IPv4Address", std::string{});
        config.ipv6_address = j.value("IPv6Address", std::string{});
        config.link_local_ips = j.value("LinkLocalIPs", std::map<std::string, std::string>{});
    }

    // Fonction de sérialisation custom pour EndpointIPAMConfig
    inline void to_json(nlohmann::json& j, const EndpointIPAMConfig& config) {
        j["IPv4Address"] = config.ipv4_address;
        j["IPv6Address"] = config.ipv6_address;
        j["LinkLocalIPs"] = config.link_local_ips;
    }

    struct EndpointConfigNetwork
    {
        EndpointIPAMConfig ipam_config;
        std::vector<std::string> links;
        std::string mac_address;
        std::vector<std::string> aliases;
        std::map<std::string, std::string> driver_opts;
        int gw_priority = 0;
        std::string network_id;
        std::string endpoint_id;
        std::string gateway;
        std::string ip_address;
        int ip_prefix_len = 0;
        std::string ipv6_gateway;
        std::string global_ipv6_address;
        int64_t global_ipv6_prefix_len = 0;
        std::vector<std::string> dns_names;
    };

    // Fonction de désérialisation custom pour EndpointConfigNetwork
    inline void from_json(const nlohmann::json& j, EndpointConfigNetwork& config) {
        if (j.contains("IPAMConfig")) {
            j["IPAMConfig"].get_to(config.ipam_config);
        }
        config.links = j.value("Links", std::vector<std::string>{});
        config.mac_address = j.value("MacAddress", std::string{});
        config.aliases = j.value("Aliases", std::vector<std::string>{});
        config.driver_opts = j.value("DriverOpts", std::map<std::string, std::string>{});
        config.gw_priority = j.value("GwPriority", 0);
        config.network_id = j.value("NetworkID", std::string{});
        config.endpoint_id = j.value("EndpointID", std::string{});
        config.gateway = j.value("Gateway", std::string{});
        config.ip_address = j.value("IPAddress", std::string{});
        config.ip_prefix_len = j.value("IPPrefixLen", 0);
        config.ipv6_gateway = j.value("IPv6Gateway", std::string{});
        config.global_ipv6_address = j.value("GlobalIPv6Address", std::string{});
        config.global_ipv6_prefix_len = j.value("GlobalIPv6PrefixLen", 0);
        config.dns_names = j.value("DNSNames", std::vector<std::string>{});
    }

    // Fonction de sérialisation custom pour EndpointConfigNetwork
    inline void to_json(nlohmann::json& j, const EndpointConfigNetwork& config) {
        j["IPAMConfig"] = config.ipam_config;
        j["Links"] = config.links;
        j["MacAddress"] = config.mac_address;
        j["Aliases"] = config.aliases;
        j["DriverOpts"] = config.driver_opts;
        j["GwPriority"] = config.gw_priority;
        j["NetworkID"] = config.network_id;
        j["EndpointID"] = config.endpoint_id;
        j["Gateway"] = config.gateway;
        j["IPAddress"] = config.ip_address;
        j["IPPrefixLen"] = config.ip_prefix_len;
        j["IPv6Gateway"] = config.ipv6_gateway;
        j["GlobalIPv6Address"] = config.global_ipv6_address;
        j["GlobalIPv6PrefixLen"] = config.global_ipv6_prefix_len;
        j["DNSNames"] = config.dns_names;
    }
};