/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#include "utils/property.h"
#include <iostream>
#include "container.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "docker.h"
#include <utils/curl.h>
#include <thread>
#include <fmt/format.h>
#include <sstream>

containerTypes::ContainerStatus stringToState(const std::string &status) {
	if (status == "created") return containerTypes::ContainerStatus::CREATED;
	if (status == "running") return containerTypes::ContainerStatus::RUNNING;
	if (status == "paused") return containerTypes::ContainerStatus::PAUSED;
	if (status == "restarting") return containerTypes::ContainerStatus::RESTARTING;
	if (status == "exited") return containerTypes::ContainerStatus::EXITED;
	if (status == "removing") return containerTypes::ContainerStatus::REMOVING;
	if (status == "dead") return containerTypes::ContainerStatus::DEAD;
	return containerTypes::ContainerStatus::UNKNOWN;
}

std::string stateToString(containerTypes::ContainerStatus status) {
	switch (status) {
		case containerTypes::ContainerStatus::CREATED:
			return "created";
		case containerTypes::ContainerStatus::RUNNING:
			return "running";
		case containerTypes::ContainerStatus::PAUSED:
			return "paused";
		case containerTypes::ContainerStatus::RESTARTING:
			return "restarting";
		case containerTypes::ContainerStatus::EXITED:
			return "exited";
		case containerTypes::ContainerStatus::REMOVING:
			return "removing";
		case containerTypes::ContainerStatus::DEAD:
			return "dead";
		default:
			return "unknown";
	}
}

Container::Container(std::shared_ptr<DockerClient> dockerClient, const nlohmann::json &containerJson) {
	try {
		this->dockerClient = dockerClient;
		// ID du container
		id = containerJson.value("Id", "");

		// Nom (retirer le '/' au début)
		if (containerJson.contains("Names") && containerJson["Names"].is_array() && !containerJson["Names"].empty()) {
			std::string fullName = containerJson["Names"][0].get<std::string>();
			name = fullName.substr(1); // Retire le '/' du début
		}


		// Basic properties
		appArmorProfile = containerJson.value("AppArmorProfile", "");
		name = containerJson.value("Name", "");
		created = containerJson.value("Created", "");
		driver = containerJson.value("Driver", "");
		image = containerJson.value("Image", "");
		logPath = containerJson.value("LogPath", "");
		mountLabel = containerJson.value("MountLabel", "");
		path = containerJson.value("Path", "");
		platform = containerJson.value("Platform", "");
		processLabel = containerJson.value("ProcessLabel", "");
		hostnamePath = containerJson.value("HostnamePath", "");
		hostsPath = containerJson.value("HostsPath", "");
		resolvConfPath = containerJson.value("ResolvConfPath", "");
		restartCount = containerJson.value("RestartCount", 0);


		// Args
		if (containerJson.contains("Args") && containerJson["Args"].is_array()) {
			for (const auto& arg : containerJson["Args"]) {
				if (arg.is_string()) {
					args.push_back(arg.get<std::string>());
				}
			}
		}

		// ExecIDs
		if (containerJson.contains("ExecIDs") && containerJson["ExecIDs"].is_array()) {
			for (const auto& execId : containerJson["ExecIDs"]) {
				if (execId.is_string()) {
					execIDs.push_back(execId.get<std::string>());
				}
			}
		}

		// State
		if (containerJson.contains("State") && containerJson["State"].is_object()) {
			const auto& stateJson = containerJson["State"];
			state.dead = stateJson.value("Dead", false);
			state.error = stateJson.value("Error", "");
			state.exitCode = stateJson.value("ExitCode", 0);
			state.finishedAt = stateJson.value("FinishedAt", "");
			state.oomKilled = stateJson.value("OOMKilled", false);
			state.paused = stateJson.value("Paused", false);
			state.pid = stateJson.value("Pid", 0);
			state.restarting = stateJson.value("Restarting", false);
			state.running = stateJson.value("Running", false);
			state.startedAt = stateJson.value("StartedAt", "");
			state.status = stringToState(stateJson.value("Status", "unknown"));
		}

		// Config
		if (containerJson.contains("Config") && containerJson["Config"].is_object()) {
			const auto& configJson = containerJson["Config"];
			config.attachStderr = configJson.value("AttachStderr", false);
			config.attachStdin = configJson.value("AttachStdin", false);
			config.attachStdout = configJson.value("AttachStdout", false);
			config.domainname = configJson.value("Domainname", "");
			config.hostname = configJson.value("Hostname", "");
			config.image = configJson.value("Image", "");
			config.openStdin = configJson.value("OpenStdin", false);
			config.stdinOnce = configJson.value("StdinOnce", false);
			config.tty = configJson.value("Tty", false);
			config.user = configJson.value("User", "");
			config.workingDir = configJson.value("WorkingDir", "");

			// Cmd array
			if (configJson.contains("Cmd") && configJson["Cmd"].is_array()) {
				for (const auto& cmd : configJson["Cmd"]) {
					if (cmd.is_string()) {
						config.cmd.push_back(cmd.get<std::string>());
					}
				}
			}

			// Entrypoint array
			if (configJson.contains("Entrypoint") && configJson["Entrypoint"].is_array()) {
				for (const auto& entry : configJson["Entrypoint"]) {
					if (entry.is_string()) {
						config.entrypoint.push_back(entry.get<std::string>());
					}
				}
			}

			// Environment variables
			if (configJson.contains("Env") && configJson["Env"].is_array()) {
				for (const auto& env : configJson["Env"]) {
					if (env.is_string()) {
						config.env.push_back(env.get<std::string>());
					}
				}
			}

			// Labels
			if (configJson.contains("Labels") && configJson["Labels"].is_object()) {
				for (const auto& [key, value] : configJson["Labels"].items()) {
					if (value.is_string()) {
						config.labels[key] = value.get<std::string>();
					}
				}
			}
		}

		// Mounts - gestion robuste des types
		if (containerJson.contains("Mounts") && containerJson["Mounts"].is_array()) {
			for (const auto& mountJson : containerJson["Mounts"]) {
				containerTypes::Mount mount;
				mount.destination = mountJson.contains("Destination") && !mountJson["Destination"].is_null() ? 
					mountJson["Destination"].get<std::string>() : "";
				mount.driver = mountJson.contains("Driver") && !mountJson["Driver"].is_null() ? 
					mountJson["Driver"].get<std::string>() : "";
				mount.mode = mountJson.contains("Mode") && !mountJson["Mode"].is_null() ? 
					mountJson["Mode"].get<std::string>() : "";
				mount.name = mountJson.contains("Name") && !mountJson["Name"].is_null() ? 
					mountJson["Name"].get<std::string>() : "";
				mount.propagation = mountJson.contains("Propagation") && !mountJson["Propagation"].is_null() ? 
					mountJson["Propagation"].get<std::string>() : "";
				mount.rw = mountJson.contains("RW") && !mountJson["RW"].is_null() ? 
					mountJson["RW"].get<bool>() : false;
				mount.source = mountJson.contains("Source") && !mountJson["Source"].is_null() ? 
					mountJson["Source"].get<std::string>() : "";
				mount.type = mountJson.contains("Type") && !mountJson["Type"].is_null() ? 
					mountJson["Type"].get<std::string>() : "";
				mounts.push_back(mount);
			}
		}

		if (containerJson.contains("HostConfig") && containerJson["HostConfig"].is_object()) {
			const auto& hostConfigJson = containerJson["HostConfig"];
			
			hostConfig.autoRemove = hostConfigJson.contains("AutoRemove") && !hostConfigJson["AutoRemove"].is_null() ? 
				hostConfigJson["AutoRemove"].get<bool>() : false;
			
			// Binds array
			if (hostConfigJson.contains("Binds") && hostConfigJson["Binds"].is_array()) {
				for (const auto& bind : hostConfigJson["Binds"]) {
					if (bind.is_string()) {
						hostConfig.binds.push_back(bind.get<std::string>());
					}
				}
			}
			
			hostConfig.blkioWeight = hostConfigJson.contains("BlkioWeight") && !hostConfigJson["BlkioWeight"].is_null() ? 
				hostConfigJson["BlkioWeight"].get<int>() : 0;
			
			// CapAdd array
			if (hostConfigJson.contains("CapAdd") && hostConfigJson["CapAdd"].is_array()) {
				for (const auto& cap : hostConfigJson["CapAdd"]) {
					if (cap.is_string()) {
						hostConfig.capAdd.push_back(cap.get<std::string>());
					}
				}
			}
			
			// CapDrop array
			if (hostConfigJson.contains("CapDrop") && hostConfigJson["CapDrop"].is_array()) {
				for (const auto& cap : hostConfigJson["CapDrop"]) {
					if (cap.is_string()) {
						hostConfig.capDrop.push_back(cap.get<std::string>());
					}
				}
			}
			
			hostConfig.cgroup = hostConfigJson.contains("Cgroup") && !hostConfigJson["Cgroup"].is_null() ? 
				hostConfigJson["Cgroup"].get<std::string>() : "";
			hostConfig.cgroupParent = hostConfigJson.contains("CgroupParent") && !hostConfigJson["CgroupParent"].is_null() ? 
				hostConfigJson["CgroupParent"].get<std::string>() : "";
			hostConfig.cgroupnsMode = hostConfigJson.contains("CgroupnsMode") && !hostConfigJson["CgroupnsMode"].is_null() ? 
				hostConfigJson["CgroupnsMode"].get<std::string>() : "";
			
			hostConfig.containerIDFile = hostConfigJson.contains("ContainerIDFile") && !hostConfigJson["ContainerIDFile"].is_null() ? 
				hostConfigJson["ContainerIDFile"].get<std::string>() : "";
			
			hostConfig.cpuCount = hostConfigJson.contains("CpuCount") && !hostConfigJson["CpuCount"].is_null() ? 
				hostConfigJson["CpuCount"].get<int>() : 0;
			hostConfig.cpuPercent = hostConfigJson.contains("CpuPercent") && !hostConfigJson["CpuPercent"].is_null() ? 
				hostConfigJson["CpuPercent"].get<int>() : 0;
			hostConfig.cpuPeriod = hostConfigJson.contains("CpuPeriod") && !hostConfigJson["CpuPeriod"].is_null() ? 
				hostConfigJson["CpuPeriod"].get<int>() : 0;
			hostConfig.cpuQuota = hostConfigJson.contains("CpuQuota") && !hostConfigJson["CpuQuota"].is_null() ? 
				hostConfigJson["CpuQuota"].get<int>() : 0;
			hostConfig.cpuRealtimePeriod = hostConfigJson.contains("CpuRealtimePeriod") && !hostConfigJson["CpuRealtimePeriod"].is_null() ? 
				hostConfigJson["CpuRealtimePeriod"].get<int>() : 0;
			hostConfig.cpuRealtimeRuntime = hostConfigJson.contains("CpuRealtimeRuntime") && !hostConfigJson["CpuRealtimeRuntime"].is_null() ? 
				hostConfigJson["CpuRealtimeRuntime"].get<int>() : 0;
			hostConfig.cpuShares = hostConfigJson.contains("CpuShares") && !hostConfigJson["CpuShares"].is_null() ? 
				hostConfigJson["CpuShares"].get<int>() : 0;
			
			hostConfig.cpusetCpus = hostConfigJson.contains("CpusetCpus") && !hostConfigJson["CpusetCpus"].is_null() ? 
				hostConfigJson["CpusetCpus"].get<std::string>() : "";
			hostConfig.cpusetMems = hostConfigJson.contains("CpusetMems") && !hostConfigJson["CpusetMems"].is_null() ? 
				hostConfigJson["CpusetMems"].get<std::string>() : "";
			
			// Devices array
			if (hostConfigJson.contains("Devices") && hostConfigJson["Devices"].is_array()) {
				for (const auto& deviceJson : hostConfigJson["Devices"]) {
					if (deviceJson.is_object()) {
						containerTypes::DeviceMapping device;
						device.pathOnHost = deviceJson.contains("PathOnHost") && !deviceJson["PathOnHost"].is_null() ? 
							deviceJson["PathOnHost"].get<std::string>() : "";
						device.pathInContainer = deviceJson.contains("PathInContainer") && !deviceJson["PathInContainer"].is_null() ? 
							deviceJson["PathInContainer"].get<std::string>() : "";
						device.cgroupPermissions = deviceJson.contains("CgroupPermissions") && !deviceJson["CgroupPermissions"].is_null() ? 
							deviceJson["CgroupPermissions"].get<std::string>() : "";
						hostConfig.devices.push_back(device);
					}
				}
			}
			
			// DNS array
			if (hostConfigJson.contains("Dns") && hostConfigJson["Dns"].is_array()) {
				for (const auto& dns : hostConfigJson["Dns"]) {
					if (dns.is_string()) {
						hostConfig.dns.push_back(dns.get<std::string>());
					}
				}
			}
			
			// DnsOptions array
			if (hostConfigJson.contains("DnsOptions") && hostConfigJson["DnsOptions"].is_array()) {
				for (const auto& option : hostConfigJson["DnsOptions"]) {
					if (option.is_string()) {
						hostConfig.dnsOptions.push_back(option.get<std::string>());
					}
				}
			}
			
			// DnsSearch array
			if (hostConfigJson.contains("DnsSearch") && hostConfigJson["DnsSearch"].is_array()) {
				for (const auto& search : hostConfigJson["DnsSearch"]) {
					if (search.is_string()) {
						hostConfig.dnsSearch.push_back(search.get<std::string>());
					}
				}
			}
			
			// ExtraHosts array
			if (hostConfigJson.contains("ExtraHosts") && hostConfigJson["ExtraHosts"].is_array()) {
				for (const auto& host : hostConfigJson["ExtraHosts"]) {
					if (host.is_string()) {
						hostConfig.extraHosts.push_back(host.get<std::string>());
					}
				}
			}
			
			// GroupAdd array
			if (hostConfigJson.contains("GroupAdd") && hostConfigJson["GroupAdd"].is_array()) {
				for (const auto& group : hostConfigJson["GroupAdd"]) {
					if (group.is_string()) {
						hostConfig.groupAdd.push_back(group.get<std::string>());
					}
				}
			}
			
			hostConfig.ioMaximumBandwidth = hostConfigJson.contains("IOMaximumBandwidth") && !hostConfigJson["IOMaximumBandwidth"].is_null() ? 
				hostConfigJson["IOMaximumBandwidth"].get<int>() : 0;
			hostConfig.ioMaximumIOps = hostConfigJson.contains("IOMaximumIOps") && !hostConfigJson["IOMaximumIOps"].is_null() ? 
				hostConfigJson["IOMaximumIOps"].get<int>() : 0;
			
			hostConfig.ipcMode = hostConfigJson.contains("IpcMode") && !hostConfigJson["IpcMode"].is_null() ? 
				hostConfigJson["IpcMode"].get<std::string>() : "";
			hostConfig.isolation = hostConfigJson.contains("Isolation") && !hostConfigJson["Isolation"].is_null() ? 
				hostConfigJson["Isolation"].get<std::string>() : "";
			
			// Links array
			if (hostConfigJson.contains("Links") && hostConfigJson["Links"].is_array()) {
				for (const auto& link : hostConfigJson["Links"]) {
					if (link.is_string()) {
						hostConfig.links.push_back(link.get<std::string>());
					}
				}
			}
			
			// LogConfig
			if (hostConfigJson.contains("LogConfig") && hostConfigJson["LogConfig"].is_object()) {
				const auto& logConfigJson = hostConfigJson["LogConfig"];
				hostConfig.logConfig.type = logConfigJson.contains("Type") && !logConfigJson["Type"].is_null() ? 
					logConfigJson["Type"].get<std::string>() : "";
				
				if (logConfigJson.contains("Config") && logConfigJson["Config"].is_object()) {
					for (const auto& [key, value] : logConfigJson["Config"].items()) {
						if (value.is_string()) {
							hostConfig.logConfig.config[key] = value.get<std::string>();
						}
					}
				}
			}
			
			// MaskedPaths array
			if (hostConfigJson.contains("MaskedPaths") && hostConfigJson["MaskedPaths"].is_array()) {
				for (const auto& path : hostConfigJson["MaskedPaths"]) {
					if (path.is_string()) {
						hostConfig.maskedPaths.push_back(path.get<std::string>());
					}
				}
			}
			
			hostConfig.memory = hostConfigJson.contains("Memory") && !hostConfigJson["Memory"].is_null() ? 
				hostConfigJson["Memory"].get<long long>() : 0;
			hostConfig.memoryReservation = hostConfigJson.contains("MemoryReservation") && !hostConfigJson["MemoryReservation"].is_null() ? 
				hostConfigJson["MemoryReservation"].get<long long>() : 0;
			hostConfig.memorySwap = hostConfigJson.contains("MemorySwap") && !hostConfigJson["MemorySwap"].is_null() ? 
				hostConfigJson["MemorySwap"].get<long long>() : 0;
			
			if (hostConfigJson.contains("MemorySwappiness") && !hostConfigJson["MemorySwappiness"].is_null()) {
				hostConfig.memorySwappiness = hostConfigJson["MemorySwappiness"].get<int>();
			}
			
			hostConfig.nanoCpus = hostConfigJson.contains("NanoCpus") && !hostConfigJson["NanoCpus"].is_null() ? 
				hostConfigJson["NanoCpus"].get<long long>() : 0;
			
			hostConfig.networkMode = hostConfigJson.contains("NetworkMode") && !hostConfigJson["NetworkMode"].is_null() ? 
				hostConfigJson["NetworkMode"].get<std::string>() : "";
			
			if (hostConfigJson.contains("OomKillDisable") && !hostConfigJson["OomKillDisable"].is_null()) {
				hostConfig.oomKillDisable = hostConfigJson["OomKillDisable"].get<bool>();
			}
			
			hostConfig.oomScoreAdj = hostConfigJson.contains("OomScoreAdj") && !hostConfigJson["OomScoreAdj"].is_null() ? 
				hostConfigJson["OomScoreAdj"].get<int>() : 0;
			
			hostConfig.pidMode = hostConfigJson.contains("PidMode") && !hostConfigJson["PidMode"].is_null() ? 
				hostConfigJson["PidMode"].get<std::string>() : "";
			
			if (hostConfigJson.contains("PidsLimit") && !hostConfigJson["PidsLimit"].is_null()) {
				hostConfig.pidsLimit = hostConfigJson["PidsLimit"].get<int>();
			}
			
			// PortBindings
			if (hostConfigJson.contains("PortBindings") && hostConfigJson["PortBindings"].is_object()) {
				for (const auto& [port, bindingsArray] : hostConfigJson["PortBindings"].items()) {
					if (bindingsArray.is_array()) {
						std::vector<containerTypes::PortBinding> bindings;
						for (const auto& bindingJson : bindingsArray) {
							if (bindingJson.is_object()) {
								containerTypes::PortBinding binding;
								binding.hostIp = bindingJson.contains("HostIp") && !bindingJson["HostIp"].is_null() ? 
									bindingJson["HostIp"].get<std::string>() : "";
								binding.hostPort = bindingJson.contains("HostPort") && !bindingJson["HostPort"].is_null() ? 
									bindingJson["HostPort"].get<std::string>() : "";
								bindings.push_back(binding);
							}
						}
						hostConfig.portBindings[port] = bindings;
					}
				}
			}
			
			hostConfig.privileged = hostConfigJson.contains("Privileged") && !hostConfigJson["Privileged"].is_null() ? 
				hostConfigJson["Privileged"].get<bool>() : false;
			hostConfig.publishAllPorts = hostConfigJson.contains("PublishAllPorts") && !hostConfigJson["PublishAllPorts"].is_null() ? 
				hostConfigJson["PublishAllPorts"].get<bool>() : false;
			
			// ReadonlyPaths array
			if (hostConfigJson.contains("ReadonlyPaths") && hostConfigJson["ReadonlyPaths"].is_array()) {
				for (const auto& path : hostConfigJson["ReadonlyPaths"]) {
					if (path.is_string()) {
						hostConfig.readonlyPaths.push_back(path.get<std::string>());
					}
				}
			}
			
			hostConfig.readonlyRootfs = hostConfigJson.contains("ReadonlyRootfs") && !hostConfigJson["ReadonlyRootfs"].is_null() ? 
				hostConfigJson["ReadonlyRootfs"].get<bool>() : false;
			
			// RestartPolicy
			if (hostConfigJson.contains("RestartPolicy") && hostConfigJson["RestartPolicy"].is_object()) {
				const auto& restartJson = hostConfigJson["RestartPolicy"];
				hostConfig.restartPolicy.name = restartJson.contains("Name") && !restartJson["Name"].is_null() ? 
					restartJson["Name"].get<std::string>() : "";
				hostConfig.restartPolicy.maximumRetryCount = restartJson.contains("MaximumRetryCount") && !restartJson["MaximumRetryCount"].is_null() ? 
					restartJson["MaximumRetryCount"].get<int>() : 0;
			}
			
			hostConfig.runtime = hostConfigJson.contains("Runtime") && !hostConfigJson["Runtime"].is_null() ? 
				hostConfigJson["Runtime"].get<std::string>() : "";
			
			// SecurityOpt array
			if (hostConfigJson.contains("SecurityOpt") && hostConfigJson["SecurityOpt"].is_array()) {
				for (const auto& opt : hostConfigJson["SecurityOpt"]) {
					if (opt.is_string()) {
						hostConfig.securityOpt.push_back(opt.get<std::string>());
					}
				}
			}
			
			hostConfig.shmSize = hostConfigJson.contains("ShmSize") && !hostConfigJson["ShmSize"].is_null() ? 
				hostConfigJson["ShmSize"].get<long long>() : 0;
			
			hostConfig.utsMode = hostConfigJson.contains("UTSMode") && !hostConfigJson["UTSMode"].is_null() ? 
				hostConfigJson["UTSMode"].get<std::string>() : "";
			
			// Ulimits array
			if (hostConfigJson.contains("Ulimits") && hostConfigJson["Ulimits"].is_array()) {
				for (const auto& ulimitJson : hostConfigJson["Ulimits"]) {
					if (ulimitJson.is_object()) {
						containerTypes::UlimitSetting ulimit;
						ulimit.name = ulimitJson.contains("Name") && !ulimitJson["Name"].is_null() ? 
							ulimitJson["Name"].get<std::string>() : "";
						ulimit.soft = ulimitJson.contains("Soft") && !ulimitJson["Soft"].is_null() ? 
							ulimitJson["Soft"].get<int>() : 0;
						ulimit.hard = ulimitJson.contains("Hard") && !ulimitJson["Hard"].is_null() ? 
							ulimitJson["Hard"].get<int>() : 0;
						hostConfig.ulimits.push_back(ulimit);
					}
				}
			}
			
			hostConfig.usernsMode = hostConfigJson.contains("UsernsMode") && !hostConfigJson["UsernsMode"].is_null() ? 
				hostConfigJson["UsernsMode"].get<std::string>() : "";
			hostConfig.volumeDriver = hostConfigJson.contains("VolumeDriver") && !hostConfigJson["VolumeDriver"].is_null() ? 
				hostConfigJson["VolumeDriver"].get<std::string>() : "";
			
			// VolumesFrom array
			if (hostConfigJson.contains("VolumesFrom") && hostConfigJson["VolumesFrom"].is_array()) {
				for (const auto& volume : hostConfigJson["VolumesFrom"]) {
					if (volume.is_string()) {
						hostConfig.volumesFrom.push_back(volume.get<std::string>());
					}
				}
			}
		}

		// Network Settings - gestion robuste des types
		if (containerJson.contains("NetworkSettings") && containerJson["NetworkSettings"].is_object()) {
			const auto& netJson = containerJson["NetworkSettings"];
			networkSettings.bridge = netJson.contains("Bridge") && !netJson["Bridge"].is_null() ? 
				netJson["Bridge"].get<std::string>() : "";
			networkSettings.endpointID = netJson.contains("EndpointID") && !netJson["EndpointID"].is_null() ? 
				netJson["EndpointID"].get<std::string>() : "";
			networkSettings.gateway = netJson.contains("Gateway") && !netJson["Gateway"].is_null() ? 
				netJson["Gateway"].get<std::string>() : "";
			networkSettings.globalIPv6Address = netJson.contains("GlobalIPv6Address") && !netJson["GlobalIPv6Address"].is_null() ? 
				netJson["GlobalIPv6Address"].get<std::string>() : "";
			networkSettings.globalIPv6PrefixLen = netJson.contains("GlobalIPv6PrefixLen") && !netJson["GlobalIPv6PrefixLen"].is_null() ? 
				netJson["GlobalIPv6PrefixLen"].get<int>() : 0;
			networkSettings.hairpinMode = netJson.contains("HairpinMode") && !netJson["HairpinMode"].is_null() ? 
				netJson["HairpinMode"].get<bool>() : false;
			networkSettings.ipAddress = netJson.contains("IPAddress") && !netJson["IPAddress"].is_null() ? 
				netJson["IPAddress"].get<std::string>() : "";
			networkSettings.ipPrefixLen = netJson.contains("IPPrefixLen") && !netJson["IPPrefixLen"].is_null() ? 
				netJson["IPPrefixLen"].get<int>() : 0;
			networkSettings.ipv6Gateway = netJson.contains("IPv6Gateway") && !netJson["IPv6Gateway"].is_null() ? 
				netJson["IPv6Gateway"].get<std::string>() : "";
			networkSettings.linkLocalIPv6Address = netJson.contains("LinkLocalIPv6Address") && !netJson["LinkLocalIPv6Address"].is_null() ? 
				netJson["LinkLocalIPv6Address"].get<std::string>() : "";
			networkSettings.linkLocalIPv6PrefixLen = netJson.contains("LinkLocalIPv6PrefixLen") && !netJson["LinkLocalIPv6PrefixLen"].is_null() ? 
				netJson["LinkLocalIPv6PrefixLen"].get<int>() : 0;
			networkSettings.macAddress = netJson.contains("MacAddress") && !netJson["MacAddress"].is_null() ? 
				netJson["MacAddress"].get<std::string>() : "";
			networkSettings.sandboxID = netJson.contains("SandboxID") && !netJson["SandboxID"].is_null() ? 
				netJson["SandboxID"].get<std::string>() : "";
			networkSettings.sandboxKey = netJson.contains("SandboxKey") && !netJson["SandboxKey"].is_null() ? 
				netJson["SandboxKey"].get<std::string>() : "";
			
			networkSettings.secondaryIPAddresses = netJson.contains("SecondaryIPAddresses") && netJson["SecondaryIPAddresses"].is_array() ?
				[&]() {
					std::vector<std::string> ips;
					for (const auto& ip : netJson["SecondaryIPAddresses"]) {
						if (ip.is_string()) {
							ips.push_back(ip.get<std::string>());
						}
					}
					return ips;
				}() : std::vector<std::string>();

			networkSettings.secondaryIPv6Addresses = netJson.contains("SecondaryIPv6Addresses") && netJson["SecondaryIPv6Addresses"].is_array() ?
				[&]() {
					std::vector<std::string> ipv6s;
					for (const auto& ipv6 : netJson["SecondaryIPv6Addresses"]) {
						if (ipv6.is_string()) {
							ipv6s.push_back(ipv6.get<std::string>());
						}
					}
					return ipv6s;
				}() : std::vector<std::string>();

			// Networks - gestion robuste des types
			if (netJson.contains("Networks") && netJson["Networks"].is_object()) {
				for (const auto& [networkName, networkData] : netJson["Networks"].items()) {
					containerTypes::NetworkEndpoint endpoint;
					endpoint.endpointID = networkData.contains("EndpointID") && !networkData["EndpointID"].is_null() ? 
						networkData["EndpointID"].get<std::string>() : "";
					endpoint.gateway = networkData.contains("Gateway") && !networkData["Gateway"].is_null() ? 
						networkData["Gateway"].get<std::string>() : "";
					endpoint.globalIPv6Address = networkData.contains("GlobalIPv6Address") && !networkData["GlobalIPv6Address"].is_null() ? 
						networkData["GlobalIPv6Address"].get<std::string>() : "";
					endpoint.globalIPv6PrefixLen = networkData.contains("GlobalIPv6PrefixLen") && !networkData["GlobalIPv6PrefixLen"].is_null() ? 
						networkData["GlobalIPv6PrefixLen"].get<int>() : 0;
					endpoint.ipAddress = networkData.contains("IPAddress") && !networkData["IPAddress"].is_null() ? 
						networkData["IPAddress"].get<std::string>() : "";
					endpoint.ipPrefixLen = networkData.contains("IPPrefixLen") && !networkData["IPPrefixLen"].is_null() ? 
						networkData["IPPrefixLen"].get<int>() : 0;
					endpoint.ipv6Gateway = networkData.contains("IPv6Gateway") && !networkData["IPv6Gateway"].is_null() ? 
						networkData["IPv6Gateway"].get<std::string>() : "";
					endpoint.macAddress = networkData.contains("MacAddress") && !networkData["MacAddress"].is_null() ? 
						networkData["MacAddress"].get<std::string>() : "";
					endpoint.networkID = networkData.contains("NetworkID") && !networkData["NetworkID"].is_null() ? 
						networkData["NetworkID"].get<std::string>() : "";
					networkSettings.networks[networkName] = endpoint;
				}
			}
		}

		// Graph Driver - gestion robuste des types
		if (containerJson.contains("GraphDriver") && containerJson["GraphDriver"].is_object()) {
			const auto& graphJson = containerJson["GraphDriver"];
			graphDriver.name = graphJson.contains("Name") && !graphJson["Name"].is_null() ? 
				graphJson["Name"].get<std::string>() : "";
			if (graphJson.contains("Data") && graphJson["Data"].is_object()) {
				for (const auto& [key, value] : graphJson["Data"].items()) {
					if (!value.is_null() && value.is_string()) {
						graphDriver.data[key] = value.get<std::string>();
					}
				}
			}
		}

	} catch (const nlohmann::json::exception &e) {
		std::cerr << "JSON parsing error for container: " << e.what() << std::endl;
		// Initialize with default values
		id = "";
		name = "";
		image = "";
		created = "";
		driver = "";
		restartCount = 0;
		state = containerTypes::ContainerState();
		config = containerTypes::ContainerConfig();
		networkSettings = containerTypes::NetworkSettings();	
		graphDriver = containerTypes::GraphDriver();
		mounts.clear();
		hostConfig = containerTypes::HostConfig();
		args.clear();
		execIDs.clear();
	}
}

// Methods

[[nodiscard]] nlohmann::json Container::inspect() {
	ReqUEST request = ReqUEST(dockerClient->getDockerApiUrl() + "/containers/" + id + "/json",
														{});

	auto response = request.execute();

	if (response->status_code != 200) {
		std::cerr << "Error: HTTP " << response->status_code << std::endl;
		throw std::runtime_error("Failed to inspect container: " + id);
	}
	
	try {
		return nlohmann::json::parse(response->body);
	} catch (const nlohmann::json::exception &e) {
		std::cerr << "JSON parsing error: " << e.what() << std::endl;
		throw std::runtime_error("Failed to parse inspect response for container: " + id);
	}
}

void Container::refresh(){
	if (!dockerClient) {
		throw std::runtime_error("dockerClient is null in Container::refresh()");
	}
	if (id.empty()) {
		throw std::runtime_error("Container id is empty in Container::refresh()");
	}

	try {
		nlohmann::json containerJson = inspect();

		std::cout << "Refreshing container: " << id << std::endl;

		*this = Container(dockerClient, containerJson);
	} catch (const nlohmann::json::exception &e) {
		std::cerr << "JSON parsing error: " << e.what() << std::endl;
		throw std::runtime_error("Failed to parse refresh response for container: " + id);
	}
}

void Container::run() {
	if (!dockerClient) {
		throw std::runtime_error("dockerClient is null in Container::run()");
	}
	if (id.empty()) {
		throw std::runtime_error("Container id is empty in Container::run()");
	}
	ReqUEST request = ReqUEST(dockerClient->getDockerApiUrl() + "/containers/" + id + "/start",
														{});

	request.setMethod(method::HttpMethod::_POST);

	std::shared_ptr<CurlResponse> response = request.execute();

	if (response->status_code != 204) {
		std::cerr << "Error: HTTP " << response->status_code << std::endl;
		state.running = false;
		state.status = containerTypes::ContainerStatus::EXITED;
		state.finishedAt = std::to_string(std::time(nullptr));
		throw std::runtime_error("Failed to start container: " + id);
	}

	state.running = true;
	state.status = containerTypes::ContainerStatus::RUNNING;
	state.startedAt = std::to_string(std::time(nullptr));
}

void Container::stop() {

	ReqUEST request = ReqUEST(dockerClient->getDockerApiUrl() + "/containers/" + id + "/stop",
														{});

	request.setMethod(method::HttpMethod::_POST);

	auto response = request.execute();

	if (response->status_code != 204) {
		std::cerr << "Error: HTTP " << response->status_code << std::endl;
		throw std::runtime_error("Failed to stop container: " + id);
	}
	
	state.running = false;
	state.status = containerTypes::ContainerStatus::EXITED;
	state.finishedAt = std::to_string(std::time(nullptr));
}

void Container::restart(){
	if (!dockerClient) {
		throw std::runtime_error("dockerClient is null in Container::restart()");
	}
	
	if (id.empty()) {
		throw std::runtime_error("Container id is empty in Container::restart()");
	}

	ReqUEST request = ReqUEST(dockerClient->getDockerApiUrl() + "/containers/" + id + "/restart", {});

	request.setMethod(method::HttpMethod::_POST);

	auto response = request.execute();

	if (response->status_code != 204) {
		std::cerr << "Error: HTTP " << response->status_code << std::endl;

		state.running = false;
		state.status = containerTypes::ContainerStatus::EXITED;
		state.finishedAt = std::to_string(std::time(nullptr));
		throw std::runtime_error("Failed to restart container: " + id);
	}

	state.running = true;
	state.status = containerTypes::ContainerStatus::RUNNING;
	state.startedAt = std::to_string(std::time(nullptr));
}

void Container::remove() {

}

std::string Container::to_string() {
	return fmt::format("Container(id: {}, name: {}, image: {}, status: {}, created: {}, driver: {}, restartCount: {})",
										 id, name, image, stateToString(state.status), created, driver, restartCount);
}

inline uint32_t networkToHost32(uint32_t netlong) {
#ifdef _WIN32
	// On Windows, we can use the built-in byte swapping
	return _byteswap_ulong(netlong);
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	// Little endian system - need to swap bytes
		return ((netlong & 0xFF000000) >> 24) |
					 ((netlong & 0x00FF0000) >> 8)  |
					 ((netlong & 0x0000FF00) << 8)  |
					 ((netlong & 0x000000FF) << 24);
#else
		// Big endian system or unknown - no conversion needed
		return netlong;
#endif
}

std::vector<std::string>
Container::logs(bool follow, bool capture_stdout, bool capture_stderr, long since, long until, bool timestamps,
								const std::string &tail) {


	std::vector<std::string> logs;

	ReqUEST request = ReqUEST(dockerClient->getDockerApiUrl() + fmt::format("/containers/{}/logs", id),
														{
														// 	CurlParam("follow", follow ? "true" : "false"),
														//  CurlParam("stdout", capture_stdout ? "true" : "false"),
														//  CurlParam("stderr", capture_stderr ? "true" : "false"),
														//  CurlParam("timestamps", timestamps ? "true" : "false"),
														//  CurlParam("since", std::to_string(since)),
														//  CurlParam("until", std::to_string(until)),
														//  CurlParam("tail", tail)
														});

	auto response = request.execute();

	if (response->status_code != 200) { // Les logs retournent 200, pas 204
		std::cerr << "Error: HTTP " << response->status_code << std::endl;
		throw std::runtime_error(
				"Failed to get logs for container: " + id + " (HTTP " + std::to_string(response->status_code) + ")");
	}

	// If not in follow mode, parse the response
	if (!follow && !response->body.empty()) {
		logs = parseLogsResponse(response->body);
	}

	return logs;
}

// Utility function to parse Docker logs response
std::vector<std::string> Container::parseLogsResponse(const std::string &response) {
	std::vector<std::string> logs;

	// Les logs Docker sont dans un format binaire avec header de 8 bytes
	size_t pos = 0;
	while (pos < response.size()) {
		if (pos + 8 <= response.size()) {
			// Lire l'header (8 bytes: stream_type + 3 bytes padding + size)
			// Bytes 0: Stream type (0=stdin, 1=stdout, 2=stderr)
			// Bytes 1-3: Padding (toujours 0)
			// Bytes 4-7: Size of following data (big-endian)

			uint8_t stream_type = static_cast<uint8_t>(response[pos]);
			uint32_t size_be = *reinterpret_cast<const uint32_t *>(response.data() + pos + 4);
			uint32_t log_size = ntohl(size_be); // Network to host byte order

			pos += 8; // Skip header

			if (pos + log_size <= response.size() && log_size > 0) {
				std::string log_line(response.data() + pos, log_size);

				// Optional: prefix with stream type
				std::string prefix;
				switch (stream_type) {
					case 0:
						prefix = "[STDIN] ";
						break;
					case 1:
						prefix = "[STDOUT] ";
						break;
					case 2:
						prefix = "[STDERR] ";
						break;
					default:
						prefix = "[UNKNOWN] ";
						break;
				}

				// Retirer le \n final s'il existe
				if (!log_line.empty() && log_line.back() == '\n') {
					log_line.pop_back();
				}

				logs.push_back(prefix + log_line);
				pos += log_size;
			} else {
				// Corrupted or incomplete data
				break;
			}
		} else {
			// Header incomplet
			break;
		}
	}

	// Fallback: if binary parsing fails, try simple text parsing
	if (logs.empty() && !response.empty()) {
		std::istringstream stream(response);
		std::string line;
		while (std::getline(stream, line)) {
			logs.push_back(line);
		}
	}

	return logs;
}

// Simplified version to get recent logs
std::vector<std::string> Container::getRecentLogs(int lines) {
	return logs(false, true, true, 0, 0, true, std::to_string(lines));
}

// Version pour suivre les logs en temps réel (non-bloquante)
void Container::followLogsAsync(std::function<void(const std::string &)> callback, int maxDuration) {
	std::thread logThread([this, callback, maxDuration]() {
			try {
				auto start = std::chrono::steady_clock::now();

				// Cette implémentation nécessiterait une approche différente
				// car curl_easy_perform est bloquant. Pour un vrai streaming,
				// il faudrait utiliser libcurl multi interface ou une autre approche

				auto logs = this->logs(true, true, true, 0, 0, true, "all");

				for (const auto &log: logs) {
					callback(log);

					// Vérifier le timeout
					if (maxDuration > 0) {
						auto now = std::chrono::steady_clock::now();
						auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start);
						if (duration.count() >= maxDuration) {
							break;
						}
					}
				}
			} catch (const std::exception &e) {
				callback("Error following logs: " + std::string(e.what()));
			}
	});

	logThread.detach();
}