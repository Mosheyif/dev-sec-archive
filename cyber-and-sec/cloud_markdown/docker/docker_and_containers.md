# Docker Practical Exercise Guide

This document serves as a step-by-step, focused instruction manual for basic
Docker operations.

## 1. Install Docker

```bash
sudo apt-get update && sudo apt-get install docker.io -y
sudo systemctl start docker
sudo systemctl enable docker
sudo usermod -aG docker $USER
```

Updates package lists, installs the Docker engine, starts the service, and
grants your user permission to run Docker commands without `sudo`.

## 2. Run Your First Container

```bash
docker run -it ubuntu:latest bash
```

Pulls the `ubuntu:latest` image and runs it interactively (`-it`), opening a
bash shell. When you type `exit`, this specific container will stop.

## 3. Keep the Container Running in the Background

```bash
docker ps
docker run -dit ubuntu:latest bash
```

`docker ps` checks active containers. Adding the `-d` (detached) flag ensures
the new container continues to run in the background.

## 4. Name Your Container

```bash
docker run -dit --name my-ubuntu-container ubuntu:latest bash
```

Docker assigns random names to containers by default. The `--name` flag allows
you to assign a specific, recognizable name.

## 5. Display Existing Local Images

```bash
docker images
```

Lists all the Docker images currently downloaded and stored on your host
machine's local drive.

## 6. Stop the Containers

```bash
docker stop <container_name>
```

Gracefully stops the running containers. You can pass multiple container names
or IDs separated by a space in a single command.

## 7. Display All Containers

```bash
docker ps -a
```

The `-a` (all) flag shows every container on your system, including the
inactive (exited) ones.

## 8. Delete the First Container Only

```bash
docker rm <container_name>
```

Permanently removes a stopped container from your system. Replace
`<container_name>` with the actual name or ID of your container.

## 9. Restart the Named Container

```bash
docker start my-ubuntu-container
```

Turns the stopped container back on. It will immediately start running in the
background.

## 10. Create a Custom Directory Inside the Container

```bash
docker exec my-ubuntu-container mkdir /<folder_name>
```

Executes the `mkdir` command directly inside the running container to create a
folder under the root (`/`) directory.

## 11. Enter the Container Shell

```bash
docker exec -it my-ubuntu-container bash
```

Opens a new, interactive bash terminal *inside* the running container without
attaching to its main startup process.

## 12. Exit the Container Without Stopping It

Type `exit` and press **Enter**.

Because you entered using `exec` (which creates a secondary process), typing
`exit` only closes that specific terminal window, leaving the main container
running in the background.

> **Note:** If you enter a container using `docker attach`, you must use the
> escape sequence `Ctrl+P` then `Ctrl+Q` to detach safely without killing the
> container.
