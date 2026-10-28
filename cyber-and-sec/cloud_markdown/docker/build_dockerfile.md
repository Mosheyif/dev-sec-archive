# Build and Test the Weather Application Image

**Objective:** Write a `Dockerfile` to build an image for the Python weather
application and run a container to test the configuration.

## Prerequisites

Before creating the `Dockerfile`, you **must** navigate to the root directory
of your project where your source code (`app.py`) and `requirements.txt` are
located.

## Step 1: Create the Dockerfile

Create a new file named `Dockerfile` (no extension) in your project directory:

```bash
nano Dockerfile
```

Paste the following configuration:

```dockerfile
# 1. Base Image: Use a modern Python version (3.12+) to support modern syntax
FROM python:3.12-slim

# 2. Set the working directory inside the container
WORKDIR /app

# 3. Copy dependencies first to leverage Docker cache
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# 4. Copy the rest of the application code
COPY . .

# 5. Expose the port Gunicorn will use
EXPOSE 8000

# 6. Command to start the Gunicorn server
CMD ["gunicorn", "-b", "0.0.0.0:8000", "app:app"]
```

## Step 2: Build the Docker Image

Run the build command from within the project directory. The dot (`.`) at the
end tells Docker to use the current directory as the build context.

```bash
sudo docker build -t weather-app .
```

## Step 3: Run the Container

Run the new image in detached mode (`-d`), map port 8000 on your host to port
8000 in the container (`-p 8000:8000`), and name it.

```bash
sudo docker run -d -p 8000:8000 --name running-weather-app weather-app
```

## Step 4: Test the Configuration

1. Verify the container is actively running (status should be `Up`):

   ```bash
   sudo docker ps
   ```

2. Open your web browser and navigate to `http://127.0.0.1:8000` or
   `http://localhost:8000` to view the weather application.
