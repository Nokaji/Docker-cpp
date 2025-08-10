
# Bibliothèque Docker

Cette bibliothèque C++ permet d'interagir avec Docker : gestion des conteneurs, images, réseaux et fichiers Docker Compose.

## Structure et documentation des fichiers

- `container_manager.*` : Interface principale pour créer, démarrer, arrêter et supprimer des conteneurs Docker.
- `container.*` : Définit la structure et les méthodes d'un conteneur individuel (ID, nom, état, etc.).
- `docker_compose_parser.*` : Parseur pour les fichiers Docker Compose (`docker-compose.yml`). Permet de charger et manipuler des stacks multi-conteneurs.
- `docker.*` : Fonctions utilitaires pour exécuter des commandes Docker, vérifier l'état du daemon, etc.
- `image_manager.*` : Permet de pull, build, supprimer et lister les images Docker.
- `network_manager.*` : Création, suppression et gestion des réseaux Docker.
- `types/` : Définitions des structures de données pour les conteneurs, images et réseaux (`container_types.h`, `image_types.h`, `network_types.h`).

## Exemples d'utilisation rapide (Client Docker)

### 1. Créer et démarrer un conteneur
```cpp
#include "docker/container_manager.h"

ContainerManager manager;
manager.createContainer("nginx", "nginx:latest");
manager.startContainer("nginx");
```

### 2. Lister les conteneurs
```cpp
#include "docker/container_manager.h"

ContainerManager manager;
auto containers = manager.listContainers();
for (const auto& c : containers) {
	std::cout << c.name << " (" << c.id << ")" << std::endl;
}
```

### 3. Puller une image Docker
```cpp
#include "docker/image_manager.h"

ImageManager imgManager;
imgManager.pullImage("ubuntu:latest");
```

### 4. Créer un réseau Docker
```cpp
#include "docker/network_manager.h"

NetworkManager netManager;
netManager.createNetwork("mon_reseau");
```

### 5. Charger un fichier Docker Compose
```cpp
#include "docker/docker_compose_parser.h"

DockerComposeParser parser;
parser.load("./docker-compose.yml");
auto services = parser.getServices();
```

## Dépendances

- Docker doit être installé et accessible via la ligne de commande.
- C++17 ou supérieur.

## Contribution

Les contributions sont les bienvenues ! Ouvrez une issue ou une pull request pour toute amélioration ou correction.

## Licence

Ce module suit la licence du projet Kernel-James.
