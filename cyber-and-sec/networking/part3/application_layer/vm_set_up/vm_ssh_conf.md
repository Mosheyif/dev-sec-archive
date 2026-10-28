

# SSH & VM Provisioning Lab Guide

## Prerequisites: Host Environment Setup

Ensure the host system has the necessary virtualization and provisioning tools installed. 

```bash
# Update repositories
sudo apt update

# Install cloud-init tools
sudo apt install cloud-image-utils -y

# Install KVM, libvirt, and provisioning utilities
sudo apt install qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils virtinst -y

```

* **Issue:** `Failed to connect socket to '/var/run/libvirt/libvirt-sock'` or `libvirtd.service not found`
* **Fix:** The full `libvirt-daemon-system` package must be installed. Start the service and add your user to the appropriate groups:
```bash
sudo systemctl enable --now libvirtd
sudo usermod -aG libvirt $USER
sudo usermod -aG kvm $USER

```

---

## 1. Set up a virtual machine

### 1.1 Generate SSH Key Pair (If not generated already)

Generate an Ed25519 key pair on the host machine (press Enter for default path and no passphrase for lab purposes):

```bash
ssh-keygen -t ed25519 -C "student@host"
cat ~/.ssh/id_ed25519.pub

```

*Copy the output string for the next step.*

### 1.2 Download OS Image

Download the official Ubuntu 22.04 Cloud Image.

* **Troubleshooting (404 Not Found):** The file extension is `.img`, not `.disk1.img`.

```bash
wget [https://cloud-images.ubuntu.com/jammy/current/jammy-server-cloudimg-amd64.img](https://cloud-images.ubuntu.com/jammy/current/jammy-server-cloudimg-amd64.img) -O ubuntu-cloud.qcow2

```

### 1.3 Create Cloud-Init Configuration

Create `user-data` to inject the SSH key and explicitly allow password authentication for the assignment.

```bash
cat <<EOF> user-data
#cloud-config
ssh_pwauth: true
chpasswd:
  list: |
    student:LabPassword123!
  expire: false

users:
  - name: student
    sudo: ALL=(ALL) NOPASSWD:ALL
    groups: sudo
    shell: /bin/bash
    ssh_authorized_keys:
      - <INSERT_YOUR_COPIED_PUBLIC_KEY_HERE>
EOF

```

Create `meta-data`:

```bash
cat <<EOF> meta-data
instance-id: lab-vm-01
local-hostname: lab-vm-01
EOF

```

### 1.4 Provision the VM

Package the configuration and execute the VM deployment:

```bash
cloud-localds seed.iso user-data meta-data

sudo virt-install \
  --name lab-vm-01 \
  --memory 2048 \
  --vcpus 2 \
  --disk ubuntu-cloud.qcow2,device=disk,bus=virtio \
  --disk seed.iso,device=cdrom \
  --os-variant ubuntu22.04 \
  --network network=default \
  --import \
  --noautoconsole

```

Retrieve the assigned IP address:

```bash
virsh net-dhcp-leases default

```

*Note the IP address as `<VM_IP>` for the following steps.*

---

## 2. Connect to it using ssh and a password

Force the SSH client to bypass the public key and prompt for the password (`LabPassword123!`):

```bash
ssh -o PreferredAuthentications=password student@<VM_IP>

```

*Exit the VM once successful (`exit`).*

---

## 3. Connect via ssh with a key & verify password fallback

Connect standardly; it will use your generated key automatically:

```bash
ssh student@<VM_IP>

```

*Exit the VM, and rerun the command from Step 2 to verify password authentication is still functional.*

---

## 4. Forbid password connections

Connect to the VM and modify the SSH daemon configuration.

### Troubleshooting: Cloud-Init Override

* **Issue:** Changing `/etc/ssh/sshd_config` to `PasswordAuthentication no` does not work.
* **Fix:** Cloud-init creates a drop-in file that overrides the main config. You must edit the specific override file.



```bash
# Edit the cloud-init override file directly inside the VM
sudo nano /etc/ssh/sshd_config.d/50-cloud-init.conf

```

Change the configuration to:

```text
PasswordAuthentication no

```

Restart the SSH service:

```bash
sudo systemctl restart ssh

```

### 4.1 Verification

Return to the host machine and test both methods:

```bash
# 1. Test Key (Should Succeed)
ssh student@<VM_IP>

# 2. Test Password (Should Fail with 'Permission denied')
ssh -o PreferredAuthentications=password student@<VM_IP>

```

---

## 5. File Transfer (SCP)

Create a test file inside the VM:

```bash
ssh student@<VM_IP>
echo "Lab transfer data" > test_file.txt
exit

```

Copy the file to the host machine using SCP:

```bash
scp student@<VM_IP>:/home/student/test_file.txt ./path/to/file
cat test_file.txt

```

---

## 6. What's the difference between sftp and scp?

* **SCP (Secure Copy Protocol):** A simple, non-interactive protocol designed solely for one-way file copying (push/pull). It executes a single transfer and terminates the session.
* **SFTP (SSH File Transfer Protocol):** An interactive, feature-rich subsystem of SSH. It maintains an open session, allowing bidirectional file transfers, directory navigation (`cd`, `ls`), permission modifications (`chmod`), and file deletion on the remote server.

```
