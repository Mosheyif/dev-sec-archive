# AWS EC2 Fundamentals: A Step-by-Step Guide

This guide provides a concise, step-by-step walkthrough for deploying and
configuring a fundamental AWS EC2 environment, including storage management
and templating.

## 1. Create an AWS Account

1. Go to the [AWS Management Console](https://aws.amazon.com/) and select
   **Create an AWS Account**.
2. Follow the on-screen instructions to provide your email, contact details,
   and billing information.
3. Verify your identity and select the **Basic Support - Free** plan.

## 2. Launch an EC2 Instance

1. Navigate to the **EC2 Dashboard** and click **Launch instances**.
2. **Name:** Provide a name for your server (e.g., `my-web-server`).
3. **AMI:** Select **Amazon Linux 2023 AMI** (Architecture: 64-bit x86).
4. **Instance Type:** Select **t3.micro** (Free tier eligible).
5. **Key Pair:** Click **Create new key pair**, name it (e.g., `my-ssh-key`),
   select **RSA** and **.pem**, and download it securely to your machine.
6. **Network Settings:**
   - Ensure **Auto-assign public IP** is Enabled.
   - Check **Allow SSH traffic from** and set the source to **My IP** (this
     automatically detects your current IP address).
   - Check **Allow HTTP traffic from the internet** (Source: `0.0.0.0/0`).
7. Click **Launch instance**.

## 3. Connect to the Instance via SSH

1. Open your terminal (Linux/macOS) or PowerShell (Windows).
2. Navigate to the folder where your `.pem` key was downloaded.
3. Restrict the key permissions (Linux/macOS only):

   ```bash
   chmod 400 my-ssh-key.pem
   ```

4. Connect using the instance's Public IPv4 DNS or IP:

   ```bash
   ssh -i "my-ssh-key.pem" ec2-user@<instance-public-ip>
   ```

## 4. Attach an Additional EBS Volume

*Note: The EBS volume must be in the exact same Availability Zone (AZ) as your
EC2 instance.*

1. Check your instance's **Availability Zone** (e.g., `us-east-1a`) in the
   instance details.
2. In the left menu, under **Elastic Block Store**, click **Volumes** >
   **Create volume**.
3. **Type:** General Purpose SSD (gp2).
4. **Size:** 1 GiB.
5. **Availability Zone:** Select the exact AZ matching your instance.
6. Click **Create volume**.
7. Once created, select the new volume, click **Actions** > **Attach volume**.
8. Select your running instance from the dropdown, leave the default device
   name, and click **Attach volume**.

## 5. Take a Snapshot of the EBS Volume

1. Go to **Elastic Block Store** > **Volumes**.
2. Select the volume you want to back up.
3. Click **Actions** > **Create snapshot**.
4. Add a descriptive summary (e.g., "Pre-deployment backup").
5. Click **Create snapshot**. (The process will run in the background.)

## 6. Create a Launch Template from the Instance

1. Go to **Instances**, select your running instance.
2. Click **Actions** > **Image and templates** > **Create template from
   instance**.
3. Provide a **Launch template name** (e.g., `web-server-base-template`).
4. Scroll to the bottom and click **Create launch template**. This saves your
   instance's exact configuration (instance type, Security Group, SSH key) for
   future use.

## 7. Create an AMI from the Instance

1. Go to **Instances**, select your running instance.
2. Click **Actions** > **Image and templates** > **Create image**.
3. Provide an **Image name** (e.g., `web-server-base-image`).
4. (Optional) Provide a description. Leave "No reboot" unchecked for a clean,
   consistent backup.
5. Click **Create image**. This creates a golden image containing your OS
   configuration and data, ready for deployment or auto-scaling.
