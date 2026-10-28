# Python Web App Deployment Guide (AWS EC2 & ALB)

This guide walks through setting up a Python web application on an AWS EC2
instance, duplicating it using an AMI, and configuring an Application Load
Balancer (ALB) for high availability and load balancing.

---

## Step 1: Launch and Configure the First EC2 Instance

1. Go to the AWS EC2 Console and click **Launch instance**.
2. Choose an Amazon Machine Image (e.g., **Amazon Linux 2023**).
3. Select an instance type (e.g., `t2.micro` or `t3.micro`).
4. Create or select a **Key Pair** for SSH access.
5. Configure **Network settings** (Security Group) to allow:
   - **SSH** (Port 22) from your IP (`My IP` or `0.0.0.0/0`).
   - **HTTP** (Port 80) from anywhere (`0.0.0.0/0`).
6. Click **Launch instance**.

---

## Step 2: Set Up the Application on the Server

1. Connect to your instance via SSH:

   ```bash
   ssh -i "your-key.pem" ec2-user@<YOUR-INSTANCE-IP>
   ```

2. Update system packages and install Python (e.g., Python 3.12):

   ```bash
   sudo dnf update -y
   sudo dnf install -y python3.12 python3.12-devel git
   ```

   Note: ensure the installed version matches or exceeds your project version.

3. Create your project directory and set up a virtual environment:

   ```bash
   mkdir project-app && cd project-app
   python3.12 -m venv venv
   source venv/bin/activate
   ```

4. Install your project requirements and Gunicorn:

   ```bash
   pip install --upgrade pip
   pip install -r requirements.txt
   pip install gunicorn
   ```

   If the installation fails, try removing version pins from `requirements.txt`.

---

## Step 3: Configure Systemd for Gunicorn

Create a systemd service to run Gunicorn automatically in the background:

1. Create the service file:

   ```bash
   sudo nano /etc/systemd/system/app.service
   ```

2. Add the following configuration (adjust paths as needed):

   ```ini
   [Unit]
   Description=Gunicorn instance to serve web app
   After=network.target

   [Service]
   User=ec2-user
   Group=ec2-user
   WorkingDirectory=/home/ec2-user/project-app
   Environment="PATH=/home/ec2-user/project-app/venv/bin"
   EnvironmentFile=/path/to/API-KEY/.env
   ExecStart=/home/ec2-user/project-app/venv/bin/gunicorn \
       --workers 3 --bind 127.0.0.1:8000 app:app

   [Install]
   WantedBy=multi-user.target
   ```

3. Enable and start the service:

   ```bash
   sudo systemctl daemon-reload
   sudo systemctl start app
   sudo systemctl enable app
   ```

---

## Step 4: Configure Nginx as a Reverse Proxy

1. Install Nginx:

   ```bash
   sudo dnf install -y nginx
   ```

2. Create an Nginx configuration file for your app:

   ```bash
   sudo nano /etc/nginx/conf.d/app.conf
   ```

3. Paste the following configuration:

   ```nginx
   server {
       listen 80;
       server_name _;

       location / {
           proxy_pass http://127.0.0.1:8000;
           proxy_set_header Host $host;
           proxy_set_header X-Real-IP $remote_addr;
           proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
       }
   }
   ```

4. Restart and enable Nginx:

   ```bash
   sudo systemctl restart nginx
   sudo systemctl enable nginx
   ```

---

## Step 5: Duplicate the Instance (AMI)

1. In the EC2 Console, select your running instance.
2. Click **Actions** > **Image and templates** > **Create image**.
3. Give the image a name and click **Create image**.
4. Wait in the **AMIs** section until the image status turns to **Available**.

---

## Step 6: Launch the Second Instance

1. Select your created AMI and click **Launch instance from AMI**.
2. Give the new instance a name.
3. Under **Network settings**, choose a **different Availability Zone (AZ)**
   than the first instance for high availability.
4. Attach the same Security Group and Key Pair, then click **Launch instance**.
5. *Recommendation:* Make a minor distinguishing change in this second instance
   to easily verify load balancing later.

---

## Step 7: Configure Target Group & Application Load Balancer (ALB)

1. Go to **Target Groups** in the EC2 sidebar and click **Create target group**.
   - Target type: **Instances**
   - Protocol: **HTTP**, Port: **80**
   - Select both instances and click **Include as pending below**, then
     **Create target group**.

2. Go to **Load Balancers** and click **Create load balancer**.
   - Choose **Application Load Balancer** (Internet-facing).
   - Select your VPC and check at least **two Availability Zones**.
   - Select your Security Group (allowing HTTP port 80).
   - Under **Listeners and routing**, route HTTP:80 to your new Target Group.

3. Click **Create load balancer**.

---

## Step 8: Testing

1. Copy the **DNS name** of your Load Balancer from the AWS console.
2. Open it in a web browser and refresh multiple times. You should see traffic
   alternate between your two application instances (Round Robin method).
