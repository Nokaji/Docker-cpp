## Complete Example: Connect and Use DockerClient

```cpp
#include "docker/docker.h"
#include "docker/container_manager.h"
#include "docker/image_manager.h"
#include "docker/network_manager.h"

// Docker connection configuration (default: localhost:2375)
DockerConfig config;
config.dockerHost = "localhost";
config.dockerPort = 2375; // Default Docker daemon port

// Create Docker client
DockerClient client(config);

// Check connection to Docker daemon
if (client.ping()) {
	std::cout << "Connected to Docker!" << std::endl;

	// Use container manager
	auto containerManager = client.containers();
	auto containers = containerManager->list();
	for (const auto& c : containers) {
		std::cout << "Container: " << c.names[0] << " (ID: " << c.id << ")" << std::endl;
	}

	// Use image manager
	auto imageManager = client.images();
	auto images = imageManager->list();
	for (const auto& img : images) {
		std::cout << "Image: " << img.repoTags[0] << std::endl;
	}

	// Use network manager
	auto networkManager = client.networks();
	auto networks = networkManager->list();
	for (const auto& net : networks) {
		std::cout << "Network: " << net.name << std::endl;
	}
} else {
	std::cerr << "Unable to connect to Docker." << std::endl;
}
```

# Docker C++ Library

This C++ library allows you to interact with Docker: manage containers, images, networks, and Docker Compose files.

## Structure and File Documentation

- `container_manager.*`: Main interface to create, start, stop, and remove Docker containers.
- `container.*`: Defines the structure and methods for an individual container (ID, name, state, etc.).
- `docker_compose_parser.*`: Parser for Docker Compose files (`docker-compose.yml`). Allows loading and manipulating multi-container stacks.
- `docker.*`: Utility functions to execute Docker commands, check daemon status, etc.
- `image_manager.*`: Pull, build, remove, and list Docker images.
- `network_manager.*`: Create, remove, and manage Docker networks.
- `types/`: Data structures for containers, images, and networks (`container_types.h`, `image_types.h`, `network_types.h`).

## Quick Usage Examples (Docker Client)

### 1. Create and Start a Container

```cpp
#include "docker/container_manager.h"

ContainerManager manager;
manager.createContainer("nginx", "nginx:latest");
manager.startContainer("nginx");
```

### 2. List Containers

```cpp
#include "docker/container_manager.h"

ContainerManager manager;
auto containers = manager.listContainers();
for (const auto& c : containers) {
	std::cout << c.name << " (" << c.id << ")" << std::endl;
}
```

### 3. Pull a Docker Image

```cpp
#include "docker/image_manager.h"

ImageManager imgManager;
imgManager.pullImage("ubuntu:latest");
```

### 4. Create a Docker Network

```cpp
#include "docker/network_manager.h"

NetworkManager netManager;
netManager.createNetwork("my_network");
```

### 5. Load a Docker Compose File

```cpp
#include "docker/docker_compose_parser.h"

DockerComposeParser parser;
parser.load("./docker-compose.yml");
auto services = parser.getServices();
```

## Complete Example: Connect and Use DockerClient

```cpp
#include "docker/docker.h"
#include "docker/container_manager.h"
#include "docker/image_manager.h"
#include "docker/network_manager.h"

// Docker connection configuration (default: localhost:2375)
DockerConfig config;
config.dockerHost = "localhost";
config.dockerPort = 2375; // Default Docker daemon port

// Create Docker client
DockerClient client(config);

// Check connection to Docker daemon
if (client.ping()) {
	std::cout << "Connected to Docker!" << std::endl;

	// Use container manager
	auto containerManager = client.containers();
	auto containers = containerManager->list();
	for (const auto& c : containers) {
		std::cout << "Container: " << c.names[0] << " (ID: " << c.id << ")" << std::endl;
	}

	// Use image manager
	auto imageManager = client.images();
	auto images = imageManager->list();
	for (const auto& img : images) {
		std::cout << "Image: " << img.repoTags[0] << std::endl;
	}

	// Use network manager
	auto networkManager = client.networks();
	auto networks = networkManager->list();
	for (const auto& net : networks) {
		std::cout << "Network: " << net.name << std::endl;
	}
} else {
	std::cerr << "Unable to connect to Docker." << std::endl;
}
```

## Dependencies

- Docker must be installed and accessible via the command line.
- C++17 or higher.

## Contribution

Contributions are welcome! Please open an issue or pull request for improvements or bug fixes.

## License

This module follows the Kernel-James project license.
