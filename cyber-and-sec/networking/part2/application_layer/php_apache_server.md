

# Apache2 & PHP HTTP Server Configuration Guide

## 1. Description
This guide demonstrates how to configure an HTTP web server using Apache2 and PHP on a Linux environment. It covers package installation, replacing the default static HTML index with a dynamic PHP script, and verifying the server's availability locally and across the network.

---

## 2. Configuration Steps

### Step 1: Package Installation
Install the Apache2 web server, the PHP engine, and the Apache-PHP integration module.

**Terminal Command:**
```bash
sudo apt update
sudo apt install apache2 php libapache2-mod-php -y

```

### Step 2: Configuring the Default Index

Apache serves `index.html` from its web root (`/var/www/html/`) by default. To serve a PHP page instead, remove the existing HTML file and create a new `index.php` containing the desired script.

**Terminal Command:**

```bash
sudo rm /var/www/html/index.html
echo "<?php echo 'Hello World'; ?>" | sudo tee /var/www/html/index.php

```

### Step 3: Service Management

Restart the Apache service to load the newly installed PHP module and apply the configuration changes. Check the status to ensure the service is active and running without errors.

**Terminal Command:**

```bash
sudo systemctl restart apache2
sudo systemctl status apache2

```

---

## 3. Testing & Verification

### Local Verification (Localhost)

To verify the server is functioning on the host machine, open a web browser and navigate to `http://localhost`, or use the command line utility `curl`:

**Terminal Command:**

```bash
curl http://localhost

```

*Expected Output:* `Hello World`

### Remote Verification (Network IP)

To test the server from another machine on the same network (or a host machine accessing a local VM/Docker container), retrieve the server's allocated IP address.

**Terminal Command:**

```bash
ip -4 addr show | grep inet

```

Once you identify the IP address (for example, `172.18.0.1`), open a web browser on the client machine and navigate to it.

**Browser Output:**

* **URL Bar:** `http://172.18.0.1`
* **Page Content:** `Hello World`

*(Note: Modern browsers will typically flag this connection as "Not Secure" in the address bar because the traffic is transmitted over standard, unencrypted HTTP rather than HTTPS).*

```

## 4. Service Management & Teardown (Optional)

While the server can safely remain running in the background, it is a best practice in system administration to stop unused services. This frees up system resources (RAM and CPU scheduling), prevents port 80 from being unnecessarily occupied, and minimizes the network attack surface. By default, Apache2 will also automatically start on every system boot.

### Stopping the Active Service
To immediately stop the running Apache2 process and release the network port:

**Terminal Command:**
```bash
sudo systemctl stop apache2

```

### Disabling Automatic Startup

To prevent the server daemon from automatically launching upon the next system reboot:

**Terminal Command:**

```bash
sudo systemctl disable apache2

```

*(Note: If you need to use the HTTP server again in the future, you can easily reverse these actions by substituting the commands with `start` and `enable` respectively).*

```

