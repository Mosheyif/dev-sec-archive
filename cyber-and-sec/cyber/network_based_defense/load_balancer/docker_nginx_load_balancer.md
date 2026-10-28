# Load Balancing with Nginx and Docker Compose

**Objective:** Use an Nginx container to load balance between two separate
container instances running as HTTP web servers. Automate the deployment using
Docker Compose and ensure the application containers are isolated from direct
external access.

## Step 1: Create the Nginx Configuration

Create a configuration file to define how Nginx should route traffic to the
two application instances.

```bash
nano nginx.conf
```

Add the following configuration:

```nginx
events {}

http {
	upstream backend
	{
		server app1:80;
		server app2:80;
	}

	server
	{
		listen 80;

		location /
		{
			proxy_pass http://backend;
		}
	}
}
```

Save and exit (`Ctrl+O`, `Enter`, `Ctrl+X`).

## Step 2: Create the Docker Compose File

Create a `docker-compose.yml` file to orchestrate the entire environment.

```bash
nano docker-compose.yml
```

Add the following content:

```yaml
services:
  app1:
    image: nginx:alpine

  app2:
    image: nginx:alpine

  nginx:
    image: nginx:latest
    ports:
      - "8000:80" # Only Nginx is exposed to the outside world
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
    depends_on:
      - app1
      - app2
```

Save and exit.

## Step 3: Deploy the Architecture

Start the orchestration. Docker will create an internal network and start all
services in the background.

```bash
sudo docker compose up -d
```

## Step 4: Verification

Verify that all three containers (`nginx`, `app1`, and `app2`) are running:

```bash
sudo docker ps
```

**Expected output:**

```text
CONTAINER ID   IMAGE          COMMAND                  CREATED          STATUS          PORTS                                     NAMES
0049803bfd4b   nginx:latest   "/docker-entrypoint.…"   21 seconds ago   Up 21 seconds   0.0.0.0:8000->80/tcp, [::]:8000->80/tcp   network_based_defense-nginx-1
20038c6587c3   nginx:alpine   "/docker-entrypoint.…"   21 seconds ago   Up 21 seconds   80/tcp                                    network_based_defense-app1-1
4b3b83b58e0b   nginx:alpine   "/docker-entrypoint.…"   21 seconds ago   Up 21 seconds   80/tcp                                    network_based_defense-app2-1
```

* `nginx` is the only container exposed externally on port `8000`.
* `app1` and `app2` show `80/tcp` with no host mapping — reachable only
  through the internal Docker network by Nginx.
