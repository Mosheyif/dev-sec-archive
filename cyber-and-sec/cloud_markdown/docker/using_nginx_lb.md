# Load Balancing with Nginx and Docker Compose

**Objective:** Use an Nginx container to load balance between two separate
container instances of the Flask application. Automate the deployment using
Docker Compose and ensure the application containers are isolated from direct
external access.

## Step 1: Clean Up Existing Containers

Remove the previous standalone container to free up port 8000 and prevent port
conflicts.

```bash
sudo docker rm -f running-weather-app
```

## Step 2: Create the Nginx Configuration

Create a configuration file to define how Nginx should route traffic to our
application instances.

```bash
nano nginx.conf
```

Add the following configuration:

```nginx
events {}

http {
    # Define the backend group containing our two Flask containers
    upstream weather_backend {
        server web1:8000;
        server web2:8000;
    }

    server {
        # Nginx listens on port 80 inside its own container
        listen 80;

        location / {
            # Forward all incoming requests to the upstream group
            proxy_pass http://weather_backend;

            # Pass original request headers to the application
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
        }
    }
}
```

Save and exit (`Ctrl+O`, `Enter`, `Ctrl+X`).

## Step 3: Create the Docker Compose File

Create a `docker-compose.yml` file to orchestrate the entire environment (the
application instances and the load balancer).

```bash
nano docker-compose.yml
```

Add the following content:

```yaml
version: '3.8'

services:
  web1:
    build: .
    restart: always
    # Security Rule: No ports mapped here. Accessible ONLY via internal network.

  web2:
    build: .
    restart: always
    # Security Rule: No ports mapped here. Accessible ONLY via internal network.

  nginx:
    image: nginx:latest
    ports:
      - "8000:80" # Only Nginx is exposed to the outside world
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
    depends_on:
      - web1
      - web2
```

Save and exit.

## Step 4: Deploy the Architecture

Start the orchestration. Docker will automatically build the images, create an
internal network, and start all services in the background.

```bash
sudo docker compose up -d
```

## Step 5: Verification and Testing

1. Verify that all three containers (`nginx`, `web1`, and `web2`) are running:

   ```bash
   sudo docker pschna
   ```

2. Open your web browser and navigate to `http://127.0.0.1:8000`. You are now
   accessing the application securely through the Nginx Load Balancer.

---

## Troubleshooting

**Missing Docker Compose Plugin:** If you receive a
`Command 'docker-compose' not found` or `unknown shorthand flag: 'd'` error,
the Compose plugin was not installed with the core Docker engine. Fix it:

```bash
sudo apt-get update && sudo apt-get install docker-compose-v2 -y
```
