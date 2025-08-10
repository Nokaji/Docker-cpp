/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <memory>
#include <nlohmann/json.hpp>

namespace containerTypes {
    enum class ContainerStatus {
        CREATED,
        RUNNING,
        PAUSED,
        RESTARTING,
        EXITED,
        REMOVING,
        DEAD,
        UNKNOWN
    };

    struct Platform {
        std::string architecture;
        std::string os;
    };

    struct ImageManifestDescriptor {
        std::string digest;
        std::string mediaType;
        Platform platform;
        int size;
    };

    struct LogConfig {
        std::map<std::string, std::string> config;
        std::string type;
    };

    struct RestartPolicy {
        int maximumRetryCount;
        std::string name;
    };

    struct PortBinding {
        std::string hostIp;
        std::string hostPort;
    };

    struct ExposedPort {
        std::map<std::string, std::map<std::string, std::string>> ports;
    };

    struct Mount {
        std::string destination;
        std::string driver;
        std::string mode;
        std::string name;
        std::string propagation;
        bool rw;
        std::string source;
        std::string type;
    };

    struct NetworkEndpoint {
        std::vector<std::string> aliases;
        std::vector<std::string> dnsNames;
        std::map<std::string, std::string> driverOpts;
        std::string endpointID;
        std::string gateway;
        std::string globalIPv6Address;
        int globalIPv6PrefixLen;
        int gwPriority;
        std::map<std::string, std::string> ipamConfig;
        std::string ipAddress;
        int ipPrefixLen;
        std::string ipv6Gateway;
        std::vector<std::string> links;
        std::string macAddress;
        std::string networkID;
    };

    struct NetworkSettings {
        std::string bridge;
        std::string endpointID;
        std::string gateway;
        std::string globalIPv6Address;
        int globalIPv6PrefixLen;
        bool hairpinMode;
        std::string ipAddress;
        int ipPrefixLen;
        std::string ipv6Gateway;
        std::string linkLocalIPv6Address;
        int linkLocalIPv6PrefixLen;
        std::string macAddress;
        std::map<std::string, NetworkEndpoint> networks;
        std::map<std::string, std::vector<PortBinding>> ports;
        std::string sandboxID;
        std::string sandboxKey;
        std::vector<std::string> secondaryIPAddresses;
        std::vector<std::string> secondaryIPv6Addresses;
    };

    struct GraphDriver {
        std::map<std::string, std::string> data;
        std::string name;
    };

    struct ContainerState {
        bool dead;
        std::string error;
        int exitCode;
        std::string finishedAt;
        bool oomKilled;
        bool paused;
        int pid;
        bool restarting;
        bool running;
        std::string startedAt;
        ContainerStatus status;
    };

    struct DeviceMapping {
        std::string pathOnHost;
        std::string pathInContainer;
        std::string cgroupPermissions;
    };

    struct UlimitSetting {
        std::string name;
        int soft;
        int hard;
    };

    struct HostConfig {
        bool autoRemove;
        std::vector<std::string> binds;
        std::vector<std::map<std::string, std::string>> blkioDeviceReadBps;
        std::vector<std::map<std::string, std::string>> blkioDeviceReadIOps;
        std::vector<std::map<std::string, std::string>> blkioDeviceWriteBps;
        std::vector<std::map<std::string, std::string>> blkioDeviceWriteIOps;
        int blkioWeight;
        std::vector<std::map<std::string, std::string>> blkioWeightDevice;
        std::vector<std::string> capAdd;
        std::vector<std::string> capDrop;
        std::string cgroup;
        std::string cgroupParent;
        std::string cgroupnsMode;
        std::vector<int> consoleSize;
        std::string containerIDFile;
        int cpuCount;
        int cpuPercent;
        int cpuPeriod;
        int cpuQuota;
        int cpuRealtimePeriod;
        int cpuRealtimeRuntime;
        int cpuShares;
        std::string cpusetCpus;
        std::string cpusetMems;
        std::vector<std::string> deviceCgroupRules;
        std::vector<std::map<std::string, std::string>> deviceRequests;
        std::vector<DeviceMapping> devices;
        std::vector<std::string> dns;
        std::vector<std::string> dnsOptions;
        std::vector<std::string> dnsSearch;
        std::vector<std::string> extraHosts;
        std::vector<std::string> groupAdd;
        int ioMaximumBandwidth;
        int ioMaximumIOps;
        std::string ipcMode;
        std::string isolation;
        std::vector<std::string> links;
        LogConfig logConfig;
        std::vector<std::string> maskedPaths;
        long long memory;
        long long memoryReservation;
        long long memorySwap;
        std::optional<int> memorySwappiness;
        long long nanoCpus;
        std::string networkMode;
        std::optional<bool> oomKillDisable;
        int oomScoreAdj;
        std::string pidMode;
        std::optional<int> pidsLimit;
        std::map<std::string, std::vector<PortBinding>> portBindings;
        bool privileged;
        bool publishAllPorts;
        std::vector<std::string> readonlyPaths;
        bool readonlyRootfs;
        RestartPolicy restartPolicy;
        std::string runtime;
        std::vector<std::string> securityOpt;
        long long shmSize;
        std::string utsMode;
        std::vector<UlimitSetting> ulimits;
        std::string usernsMode;
        std::string volumeDriver;
        std::vector<std::string> volumesFrom;
    };

    struct ContainerConfig {
        bool attachStderr;
        bool attachStdin;
        bool attachStdout;
        std::vector<std::string> cmd;
        std::string domainname;
        std::vector<std::string> entrypoint;
        std::vector<std::string> env;
        ExposedPort exposedPorts;
        std::string hostname;
        std::string image;
        std::map<std::string, std::string> labels;
        std::vector<std::string> onBuild;
        bool openStdin;
        bool stdinOnce;
        bool tty;
        std::string user;
        std::map<std::string, std::map<std::string, std::string>> volumes;
        std::string workingDir;
    };

    // Structure pour les Device Requests
    struct DeviceRequest {
        std::string driver;
        int count;
        std::vector<std::string> deviceIDs;
        std::vector<std::vector<std::string>> capabilities;
        std::map<std::string, std::string> options;
    };

    // Structure pour la configuration IPAM
    struct IPAMConfig {
        std::string ipv4Address;
        std::string ipv6Address;
        std::vector<std::string> linkLocalIPs;
    };

    // Structure pour la configuration des endpoints
    struct EndpointConfig {
        std::optional<IPAMConfig> ipamConfig;
        std::vector<std::string> links;
        std::vector<std::string> aliases;
        
        // toJson() method
        nlohmann::json toJson() const {
            nlohmann::json j;
            if (ipamConfig.has_value()) {
                nlohmann::json ipamJson;
                ipamJson["IPv4Address"] = ipamConfig->ipv4Address;
                ipamJson["IPv6Address"] = ipamConfig->ipv6Address;
                ipamJson["LinkLocalIPs"] = ipamConfig->linkLocalIPs;
                j["IPAMConfig"] = ipamJson;
            }
            j["Links"] = links;
            j["Aliases"] = aliases;
            return j;
        }
    };

    // Structure pour la configuration réseau
    struct NetworkingConfig {
        std::map<std::string, EndpointConfig> endpointsConfig;
    };

    // Structure pour la configuration de l'hôte
    struct HostConfigCreate {
        std::vector<std::string> binds;
        std::vector<std::string> links;
        long long memory;
        long long memorySwap;
        long long memoryReservation;
        long long nanoCpus;
        int cpuPercent;
        int cpuShares;
        int cpuPeriod;
        int cpuRealtimePeriod;
        int cpuRealtimeRuntime;
        int cpuQuota;
        std::string cpusetCpus;
        std::string cpusetMems;
        int maximumIOps;
        int maximumIOBps;
        int blkioWeight;
        std::vector<std::map<std::string, std::string>> blkioWeightDevice;
        std::vector<std::map<std::string, std::string>> blkioDeviceReadBps;
        std::vector<std::map<std::string, std::string>> blkioDeviceReadIOps;
        std::vector<std::map<std::string, std::string>> blkioDeviceWriteBps;
        std::vector<std::map<std::string, std::string>> blkioDeviceWriteIOps;
        std::vector<DeviceRequest> deviceRequests;
        std::optional<int> memorySwappiness;
        bool oomKillDisable;
        int oomScoreAdj;
        std::string pidMode;
        int pidsLimit;
        std::map<std::string, std::vector<std::map<std::string, std::string>>> portBindings;
        bool publishAllPorts;
        bool privileged;
        bool readonlyRootfs;
        std::vector<std::string> dns;
        std::vector<std::string> dnsOptions;
        std::vector<std::string> dnsSearch;
        std::vector<std::string> volumesFrom;
        std::vector<std::string> capAdd;
        std::vector<std::string> capDrop;
        std::vector<std::string> groupAdd;
        RestartPolicy restartPolicy;
        bool autoRemove;
        std::string networkMode;
        std::vector<std::string> devices;
        std::vector<std::map<std::string, std::string>> ulimits;
        LogConfig logConfig;
        std::vector<std::string> securityOpt;
        std::map<std::string, std::string> storageOpt;
        std::string cgroupParent;
        std::string volumeDriver;
        int shmSize;

        // To JSON conversion
        nlohmann::json toJson() const {
            nlohmann::json j;
            j["Binds"] = binds;
            j["Links"] = links;
            j["Memory"] = memory;
            j["MemorySwap"] = memorySwap;
            j["MemoryReservation"] = memoryReservation;
            j["NanoCpus"] = nanoCpus;
            j["CpuPercent"] = cpuPercent;
            j["CpuShares"] = cpuShares;
            j["CpuPeriod"] = cpuPeriod;
            j["CpuRealtimePeriod"] = cpuRealtimePeriod;
            j["CpuRealtimeRuntime"] = cpuRealtimeRuntime;
            j["CpuQuota"] = cpuQuota;
            j["CpusetCpus"] = cpusetCpus;
            j["CpusetMems"] = cpusetMems;
            j["MaximumIOps"] = maximumIOps;
            j["MaximumIOBps"] = maximumIOBps;
            j["BlkioWeight"] = blkioWeight;

            // Convert blkioWeightDevice, blkioDeviceReadBps, etc. to JSON
            if (!blkioWeightDevice.empty()) {
                j["BlkioWeightDevice"] = blkioWeightDevice;
            }
            if (!blkioDeviceReadBps.empty()) {
                j["BlkioDeviceReadBps"] = blkioDeviceReadBps;
            }
            if (!blkioDeviceReadIOps.empty()) {
                j["BlkioDeviceReadIOps"] = blkioDeviceReadIOps;
            }
            if (!blkioDeviceWriteBps.empty()) {
                j["BlkioDeviceWriteBps"] = blkioDeviceWriteBps;
            }
            if (!blkioDeviceWriteIOps.empty()) {
                j["BlkioDeviceWriteIOps"] = blkioDeviceWriteIOps;
            }

            // Convert deviceRequests to JSON
            if (!deviceRequests.empty()) {
                nlohmann::json deviceRequestsJson = nlohmann::json::array();
                for (const auto& req : deviceRequests) {
                    nlohmann::json reqJson;
                    reqJson["Driver"] = req.driver;
                    reqJson["Count"] = req.count;
                    reqJson["DeviceIDs"] = req.deviceIDs;
                    reqJson["Capabilities"] = req.capabilities;
                    reqJson["Options"] = req.options;
                    deviceRequestsJson.push_back(reqJson);
                }
            }
            if (memorySwappiness.has_value()) {
                j["MemorySwappiness"] = memorySwappiness.value();
            }
            j["OomKillDisable"] = oomKillDisable;
            j["OomScoreAdj"] = oomScoreAdj;
            j["PidMode"] = pidMode;
            j["PidsLimit"] = pidsLimit;
            j["PortBindings"] = portBindings;
            j["PublishAllPorts"] = publishAllPorts;
            j["Privileged"] = privileged;
            j["ReadonlyRootfs"] = readonlyRootfs;
            j["Dns"] = dns;
            j["DnsOptions"] = dnsOptions;
            j["DnsSearch"] = dnsSearch;
            j["VolumesFrom"] = volumesFrom;
            j["CapAdd"] = capAdd;
            j["CapDrop"] = capDrop;
            j["GroupAdd"] = groupAdd;

            // Convert RestartPolicy to JSON
            nlohmann::json restartPolicyJson;
            restartPolicyJson["Name"] = restartPolicy.name;
            restartPolicyJson["MaximumRetryCount"] = restartPolicy.maximumRetryCount;
            j["RestartPolicy"] = restartPolicyJson;

            j["AutoRemove"] = autoRemove;
            j["NetworkMode"] = networkMode;
            j["Devices"] = devices;
            j["Ulimits"] = ulimits;

            // Convert LogConfig to JSON
            nlohmann::json logConfigJson;
            logConfigJson["Type"] = logConfig.type;
            logConfigJson["Config"] = logConfig.config;
            j["LogConfig"] = logConfigJson;

            j["SecurityOpt"] = securityOpt;
            j["StorageOpt"] = storageOpt;
            j["CgroupParent"] = cgroupParent;
            j["VolumeDriver"] = volumeDriver;
            j["ShmSize"] = shmSize;

            return j;
        };
    };

    // Main structure for container configuration
    struct ContainerCreateConfig {
        std::string hostname;
        std::string domainname;
        std::string user;
        bool attachStdin;
        bool attachStdout;
        bool attachStderr;
        bool tty;
        bool openStdin;
        bool stdinOnce;
        std::vector<std::string> env;
        std::vector<std::string> cmd;
        std::string entrypoint;
        std::string image;
        std::map<std::string, std::string> labels;
        std::map<std::string, std::map<std::string, std::string>> volumes;
        std::string workingDir;
        bool networkDisabled;
        std::string macAddress;
        std::map<std::string, std::map<std::string, std::string>> exposedPorts;
        std::string stopSignal;
        int stopTimeout;
        HostConfigCreate hostConfig;
        NetworkingConfig networkingConfig;

        // to JSON conversion

        nlohmann::json toJson() const {
            nlohmann::json j;
            j["Hostname"] = hostname;
            j["Domainname"] = domainname;
            j["User"] = user;
            j["AttachStdin"] = attachStdin;
            j["AttachStdout"] = attachStdout;
            j["AttachStderr"] = attachStderr;
            j["Tty"] = tty;
            j["OpenStdin"] = openStdin;
            j["StdinOnce"] = stdinOnce;
            j["Env"] = env;
            j["Cmd"] = cmd;
            j["Entrypoint"] = entrypoint;
            j["Image"] = image;
            j["Labels"] = labels;
            j["Volumes"] = volumes;
            j["WorkingDir"] = workingDir;
            j["NetworkDisabled"] = networkDisabled;
            j["MacAddress"] = macAddress;
            j["ExposedPorts"] = exposedPorts;
            j["StopSignal"] = stopSignal;
            j["StopTimeout"] = stopTimeout;

            // Convert HostConfigCreate to JSON
            nlohmann::json hostConfigJson = hostConfig.toJson();
            if (!hostConfigJson.empty()) {
                j["HostConfig"] = hostConfigJson;
            }

            // Convert NetworkingConfig to JSON
            if (!networkingConfig.endpointsConfig.empty()) {
                nlohmann::json endpointsJson;
                for (const auto& [name, config] : networkingConfig.endpointsConfig) {
                    endpointsJson[name] = config.toJson();
                }
                j["NetworkingConfig"]["EndpointsConfig"] = endpointsJson;
            }

            return j;
        }
    };
};