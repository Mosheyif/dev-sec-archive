Reviewer: Alon

# Exercise Guide: Linux User & Home Directory Management

## Step 1: Create a User and Password in Two Commands

```bash
sudo useradd <username>
sudo usermod <username> -p 'hashed_password'

```

## Step 2: Create a User and Password in One Command

```bash
sudo useradd <username> -p 'hashed_password'

```

The password must be hashed with the OS hash function, for example:

```bash
openssl passwd -6 password

```

To set a password using cleartext, you may use the following command:

```bash
sudo passwd <username>

```

It will ask you to enter the new password.

## Step 3: Create a Shared Directory and Set as Home

```bash
mkdir labusers_home
sudo usermod <username> -d <absolute_path>

```

Repeat the command for each user.
