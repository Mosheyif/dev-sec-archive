# AWS Infrastructure Migration: Secure Public & Private Architecture

This guide outlines the steps to migrate your web application infrastructure
into a custom Virtual Private Cloud (VPC), implementing a secure architecture
with public and private subnets, a Bastion host, and stringent access controls.

## Step 1 & 2: Infrastructure Relocation & Subnet Separation

*Goal: Move your resources to the custom VPC. Expose only what is necessary
(ALB) and hide the application servers.*

1. **Launch Private Application Servers:**
   - Go to **EC2 > AMIs** and launch new instances using the image you created
     in previous exercises.
   - **Network Settings:** Select your custom VPC.
   - **Subnet:** Choose a **Private Subnet** for each instance (e.g., Private
     Subnet A and Private Subnet B).
   - **Auto-assign Public IP:** **Disable**. These servers should not be
     directly reachable from the internet.

2. **Reconfigure the Load Balancer (ALB):**
   - Create a new **Target Group** inside your custom VPC and register your new
     private instances.
   - Create a new **Application Load Balancer (ALB)**.
   - **Network Settings:** Select your custom VPC and choose the **Public
     Subnets**. (The ALB must be in public subnets to receive internet traffic.)
   - Route the ALB's listener (Port 80) to your new Target Group.

## Step 3: Configure Security Groups (Access Control)

*Goal: Restrict traffic flow. The ALB talks to the world, the Bastion talks to
the devs, and the App servers only talk to the ALB and Bastion.*

Delete or ignore default open groups and create three specific Security Groups:

1. **ALB-SG (Load Balancer):**
   - **Inbound:** HTTP (Port 80) from `0.0.0.0/0`.
   - **Outbound:** All traffic to `0.0.0.0/0`.

2. **Bastion-SG (Jump Host):**
   - **Inbound:** SSH (Port 22) from `MY-IP` only.
   - **Outbound:** All traffic to `0.0.0.0/0`.

3. **Private-App-SG (Application Servers):**
   - **Inbound Rule 1 (Web):** HTTP (Port 80). Source: **Select the ID of the
     `ALB-SG`**.
   - **Inbound Rule 2 (Admin):** SSH (Port 22). Source: **Select the ID of the
     `Bastion-SG`**.
   - **Outbound:** All traffic to `0.0.0.0/0`.

## Step 4: Implement a Bastion Host & NAT Gateway

*Goal: Establish secure administrative access (SSH) and allow private servers
to fetch external updates.*

1. **Create the Bastion Host:**
   - Launch a small EC2 instance (e.g., `t3.micro`) into a **Public Subnet** of
     your custom VPC.
   - Ensure it is assigned a **Public IP**.
   - Attach the `Bastion-SG`.

2. **Create a NAT Gateway:**
   - Go to **VPC > NAT Gateways** and create a new NAT.
   - **Crucial:** Deploy this NAT Gateway into a **Public Subnet** of your
     custom VPC. Allocate an Elastic IP (can be automatic allocation).

3. **Update Private Route Tables:**
   - Go to **Route Tables**, select the table associated with your Private
     Subnets.
   - Add a route: Destination `0.0.0.0/0` -> Target: `Your NAT Gateway`.

### Secure SSH Connection (ProxyJump)

Do not copy your private `.pem` key to the Bastion host. Instead, use your
local SSH agent to jump through it.

```bash
# 1. Start the SSH agent locally
eval "$(ssh-agent -s)"

# 2. Add your private key to the agent
ssh-add /path/to/your-key.pem

# 3. Connect using the ProxyJump (-J) flag
ssh -J ec2-user@<BASTION_PUBLIC_IP> ec2-user@<PRIVATE_APP_INTERNAL_IP>
```

## Step 5: Network Diagram (Post-Reorganization)

*Goal: Visualize the data flow and isolation layers.*

```text
   INTERNET
      │
[ Internet Gateway (IGW) ]
      │
┌─────┴────────────────────────────── CUSTOM VPC ────────────────────────┐
│                                                                        │
│  PUBLIC SUBNETS (Route 0.0.0.0/0 -> IGW)                               │
│  ┌─────────────────────────┐        ┌─────────────────────────┐        │
│  │    Availability Zone A  │        │    Availability Zone B  │        │
│  │                         │        │                         │        │
│  │  [ ALB ] ◄──HTTP──►     │        │                         │        │
│  │  [ Bastion Host ]       │        │                         │        │
│  │  [ NAT Gateway ]        │        │                         │        │
│  └───────┬─────────┬───────┘        └─────────────────────────┘        │
│          │         │                                                   │
├──────────┼─────────┼────────────────────────────────────────────────── ┤
│          │         │                                                   │
│  PRIVATE SUBNETS (Route 0.0.0.0/0 -> NAT Gateway)                      │
│  ┌───────▼─────────▼───────┐        ┌─────────────────────────┐        │
│  │    Availability Zone A  │        │    Availability Zone B  │        │
│  │                         │        │                         │        │
│  │  [ App Server 1 (EC2) ] │        │  [ App Server 2 (EC2) ] │        │
│  │                         │        │                         │        │
│  └─────────────────────────┘        └─────────────────────────┘        │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘
```
