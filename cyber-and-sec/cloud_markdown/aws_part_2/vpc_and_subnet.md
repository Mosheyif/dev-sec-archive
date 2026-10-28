# AWS VPC & Networking Configuration Guide

This guide provides step-by-step instructions for setting up a custom Virtual
Private Cloud (VPC) with public and private subnets across multiple Availability
Zones, including internet routing and NAT configuration.

---

## Step 1: Create the VPC

1. Navigate to the **VPC Console** in AWS.
2. Click **Create VPC** (choose the "VPC only" option for manual setup).
3. **Name tag:** `MyCustomVPC`
4. **IPv4 CIDR block:** `10.0.0.0/16` (or your preferred CIDR).
5. Click **Create VPC**.

---

## Step 2: Create 4 Subnets (in 2 Different AZs)

Go to **Subnets** > **Create subnet**. Select your VPC and create the following:

- **Public Subnet A:** AZ `1a` | CIDR: `10.0.1.0/24`
- **Public Subnet B:** AZ `1b` | CIDR: `10.0.2.0/24`
- **Private Subnet A:** AZ `1a` | CIDR: `10.0.3.0/24`
- **Private Subnet B:** AZ `1b` | CIDR: `10.0.4.0/24`

---

## Step 3: Create and Attach the Internet Gateway (IGW)

1. Go to **Internet Gateways** > **Create internet gateway**.
2. **Name tag:** `MyIGW` > Click **Create**.
3. Select the created IGW, click **Actions** > **Attach to VPC**, and select
   `MyCustomVPC`.

---

## Step 4: Create a NAT Gateway

*Technical Note: The NAT Gateway must physically reside in a **Public Subnet**
so it can communicate with the Internet Gateway.*

1. Go to **NAT Gateways** > **Create NAT gateway**.
2. **Name:** `MyNATGateway`.
3. **Subnet:** Select one of your **Public Subnets** (e.g., Public Subnet A).
4. Click **Allocate Elastic IP**.
5. Click **Create NAT gateway**.

---

## Step 5: Add Route Tables and Assign to Subnets

Go to **Route Tables** > **Create route table**. Configure two separate tables:

### 1. Public Route Table

1. **Name:** `Public-RT` (select your VPC and create).
2. **Routes:** Select the table > **Edit routes** > Add route `0.0.0.0/0`
   targeting your **Internet Gateway (IGW)**.
3. **Subnet Associations:** Go to Subnet associations tab > **Edit explicit
   subnet associations** > Select both **Public Subnet A** and **Public Subnet
   B**.

### 2. Private Route Table

1. **Name:** `Private-RT` (select your VPC and create).
2. **Routes:** Select the table > **Edit routes** > Add route `0.0.0.0/0`
   targeting your **NAT Gateway**.
3. **Subnet Associations:** Go to Subnet associations tab > **Edit explicit
   subnet associations** > Select both **Private Subnet A** and **Private Subnet
   B**.

---

## Step 6: Architecture Diagram & Use Cases

### Flow Description & Use Cases

- **Public Subnets:** Have direct inbound and outbound access to the internet
  via the IGW.
  - **Use Cases:** Internet-facing ALBs, Bastion Hosts (Jump Servers), NAT
    Gateways, and public-facing static web servers.
- **Private Subnets:** Have NO direct inbound internet access. Outbound internet
  access (for software updates or external APIs) is securely routed through the
  NAT Gateway.
  - **Use Cases:** Backend application servers (like your Python EC2 instances),
    Databases (RDS), internal microservices, and secure storage elements.

### Conceptual Diagram

```text
   Internet
     │
     ▼
[ Internet Gateway (IGW) ]
     │
┌────┴──────────────────────────────────────────── VPC (10.0.0.0/16) ─┐
│                                                                      │
│  [ Public Route Table: 0.0.0.0/0 -> IGW ]                            │
│  ┌─────────────────────────┐      ┌─────────────────────────┐        │
│  │  Public Subnet 1 (AZ A) │      │  Public Subnet 2 (AZ B) │        │
│  │  [NAT Gateway]          │      │  [Load Balancer]        │        │
│  └─────────────────────────┘      └─────────────────────────┘        │
│         ▲                                                            │
│         │ (Outbound internet traffic routing)                        │
│                                                                      │
│  [ Private Route Table: 0.0.0.0/0 -> NAT Gateway ]                   │
│  ┌─────────────────────────┐      ┌─────────────────────────┐        │
│  │ Private Subnet 1 (AZ A) │      │ Private Subnet 2 (AZ B) │        │
│  │ [Python App Server]     │      │ [Python App Server]     │        │
│  └─────────────────────────┘      └─────────────────────────┘        │
└──────────────────────────────────────────────────────────────────────┘
```
