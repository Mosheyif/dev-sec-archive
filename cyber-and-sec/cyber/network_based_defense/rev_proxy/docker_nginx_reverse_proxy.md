# Reverse Proxy with Nginx and Docker Compose

**Objective:** Use an Nginx container as a reverse proxy to route incoming
requests between two web server containers based on the request URL. All
requests are channeled to `web1` by default; requests to `/api` are diverted
to `web2`.

## Step 1: Create the Nginx Configuration

Create a configuration file to define how Nginx should route traffic based on
the request path.

```bash
nano nginx.conf
```

Add the following configuration:

```nginx
events {}

http {
	resolver 127.0.0.11 valid=10s;

	server
	{
		listen 80;

		location /api
		{
			set $target http://web2:80/;
			proxy_pass $target;
		}

		location /
		{
			set $target http://web1:80;
			proxy_pass $target;
		}
	}
}
```

> **Note:** `location /api` must be defined before `location /`. Nginx matches
> the most specific path first, so placing the more specific rule first ensures
> `/api` requests are correctly diverted to `web2` before the catch-all `/`
> rule applies.
>
> `resolver 127.0.0.11` uses Docker's internal DNS. Using `set $target` with a
> variable defers hostname resolution to request time instead of startup,
> preventing Nginx from crashing when the upstream containers are not yet
> DNS-ready.
>
> The trailing slash in `http://web2:80/` strips the `/api` prefix before
> forwarding, so `web2` receives `/` instead of `/api`.

Save and exit (`Ctrl+O`, `Enter`, `Ctrl+X`).

## Step 2: Create the Docker Compose File

Create a `docker-compose.yml` file to orchestrate the two web servers and the
reverse proxy.

```bash
nano docker-compose.yml
```

Add the following content:

```yaml
services:
  web1:
    image: nginx:alpine

  web2:
    image: nginx:alpine

  nginx:
    image: nginx:latest
    restart: on-failure
    ports:
      - "8000:80"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
    depends_on:
      - web1
      - web2
```

Save and exit.

## Step 3: Deploy the Architecture

Start the orchestration from the `rev_proxy` directory:

```bash
sudo docker compose up -d
```

## Step 4: Verification

Verify that all three containers are running:

```bash
sudo docker ps
```

Test the routing by sending requests to each path:

```bash
# Should be served by web1
curl http://localhost:8000/

# Should be diverted to web2
curl http://localhost:8000/api
```
