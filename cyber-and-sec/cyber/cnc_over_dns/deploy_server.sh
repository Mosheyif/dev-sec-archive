#!/usr/bin/env bash
# deploy_server.sh — Sync CNC server to EC2 and store identity key in SSM
#
# Usage:
#   ./deploy_server.sh <EC2_PUBLIC_IP>
#   EC2_HOST=1.2.3.4 ./deploy_server.sh
#
# Overridable env vars (all have defaults):
#   EC2_HOST            EC2 public IPv4 (or pass as $1)
#   AWS_DEFAULT_REGION  AWS region            (default: il-central-1)
#   EC2_USER            SSH username          (default: ubuntu)
#   KEY_FILE            Path to .pem key      (default: ./cnc-server-key.pem)
#   REMOTE_DIR          Remote directory      (default: /home/ubuntu/cnc)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ── Config ────────────────────────────────────────────────────────────────────
REGION="${AWS_DEFAULT_REGION:-il-central-1}"
EC2_USER="${EC2_USER:-ubuntu}"
KEY_FILE="${KEY_FILE:-${SCRIPT_DIR}/cnc-server-key.pem}"
REMOTE_DIR="${REMOTE_DIR:-/home/ubuntu/cnc}"
SSM_PARAM="/cnc/server_private_hex"
ENV_FILE="${SCRIPT_DIR}/server/.env.server"
EC2_HOST="${1:-${EC2_HOST:-}}"

# ── Helpers ───────────────────────────────────────────────────────────────────
log()  { printf '\033[32m[deploy]\033[0m %s\n' "$*"; }
warn() { printf '\033[33m[ warn ]\033[0m %s\n' "$*"; }
die()  { printf '\033[31m[error ]\033[0m %s\n' "$*" >&2; exit 1; }

ssh_run() {
    ssh -i "$KEY_FILE" \
        -o StrictHostKeyChecking=no \
        -o ConnectTimeout=10 \
        "${EC2_USER}@${EC2_HOST}" "$@"
}

# ── Preflight ─────────────────────────────────────────────────────────────────
[[ -n "$EC2_HOST" ]]  || die "EC2 host not set. Pass as \$1 or set EC2_HOST."
[[ -f "$KEY_FILE" ]]  || die "SSH key not found: $KEY_FILE"
[[ -f "$ENV_FILE" ]]  || die ".env.server not found: $ENV_FILE"
command -v aws   >/dev/null || die "AWS CLI not installed. Run: pip install awscli"
command -v rsync >/dev/null || die "rsync not installed."

log "Checking AWS credentials (region: $REGION)..."
aws sts get-caller-identity --region "$REGION" --no-cli-pager >/dev/null \
    || die "AWS credentials invalid or expired. Run: aws configure"

# ── 1. Read identity key from .env.server ─────────────────────────────────────
log "Reading SERVER_PRIVATE_HEX from $ENV_FILE"
PRIV_HEX=""
while IFS= read -r line; do
    [[ "$line" =~ ^[[:space:]]*#  ]] && continue
    [[ "$line" =~ ^SERVER_PRIVATE_HEX[[:space:]]*=[[:space:]]*(.+)$ ]] \
        && PRIV_HEX="${BASH_REMATCH[1]//[\'\"]}"
done < "$ENV_FILE"
[[ -n "$PRIV_HEX" ]] || die "SERVER_PRIVATE_HEX not found in $ENV_FILE"
log "Key read — ${#PRIV_HEX} hex chars (Ed25519 private key)"

# ── 2. Upload key to SSM Parameter Store ──────────────────────────────────────
log "Storing key in SSM: ${SSM_PARAM} (SecureString)"
aws ssm put-parameter \
    --region    "$REGION"    \
    --name      "$SSM_PARAM" \
    --value     "$PRIV_HEX"  \
    --type      SecureString \
    --overwrite              \
    --no-cli-pager
log "SSM parameter written."

# ── 3. Sync source code to EC2 ────────────────────────────────────────────────
log "Syncing code → ${EC2_USER}@${EC2_HOST}:${REMOTE_DIR}/"
rsync -az --delete \
    --exclude='.git/'             \
    --exclude='__pycache__/'      \
    --exclude='*.pyc'             \
    --exclude='.venv/'            \
    --exclude='client/'           \
    --exclude='tests/'            \
    --exclude='*.pem'             \
    --exclude='server/.env.server'\
    -e "ssh -i ${KEY_FILE} -o StrictHostKeyChecking=no -o ConnectTimeout=10" \
    "${SCRIPT_DIR}/" "${EC2_USER}@${EC2_HOST}:${REMOTE_DIR}/"
log "Code synced."

# ── 4. Install Python dependencies on EC2 ────────────────────────────────────
log "Installing dependencies on EC2 (this may take a moment)..."
ssh_run bash <<REMOTE
set -euo pipefail
cd "${REMOTE_DIR}"
python3 -m pip install --quiet --upgrade pip
python3 -m pip install --quiet scapy cryptography boto3
python3 -m pip install --quiet -e common/
echo "[remote] dependencies OK"
REMOTE
log "Dependencies ready."

# ── 5. Verify SSM is reachable from EC2 ──────────────────────────────────────
log "Verifying SSM access from the instance..."
ssh_run env AWS_DEFAULT_REGION="${REGION}" python3 -c "
import boto3, sys
from botocore.config import Config
try:
    r = boto3.client('ssm', region_name='${REGION}',
        config=Config(connect_timeout=5, read_timeout=5, retries={'max_attempts': 1})
    ).get_parameter(Name='${SSM_PARAM}', WithDecryption=True)
    v = r['Parameter']['Value']
    print(f'[remote] SSM OK — {len(v)} hex chars retrieved')
except Exception as e:
    print(f'[remote] SSM failed: {e}', file=sys.stderr)
    print('[remote] Ensure the EC2 IAM role has ssm:GetParameter on ${SSM_PARAM}', file=sys.stderr)
    sys.exit(1)
"
log "SSM access confirmed."

# ── Done ──────────────────────────────────────────────────────────────────────
printf '\n\033[32m✓ Deployment complete\033[0m\n'
cat <<EOF
  Host   : ${EC2_USER}@${EC2_HOST}
  Code   : ${REMOTE_DIR}/server/
  Region : ${REGION}
  SSM    : ${SSM_PARAM}

Start the server (replace CLIENT_NAT_IP and DOMAIN with your values):
  ssh -i ${KEY_FILE} ${EC2_USER}@${EC2_HOST}
  sudo AWS_DEFAULT_REGION=${REGION} \
    python3 ${REMOTE_DIR}/server/cnc_wrapper.py --port 53 -- \
    python3 ${REMOTE_DIR}/server/cnc_handler.py \
      -i eth0 -d <CLIENT_NAT_IP> --domain <DOMAIN> --port 53

Background with log streaming:
  sudo AWS_DEFAULT_REGION=${REGION} nohup \
    python3 ${REMOTE_DIR}/server/cnc_wrapper.py --port 53 -- \
    python3 ${REMOTE_DIR}/server/cnc_handler.py \
      -i eth0 -d <CLIENT_NAT_IP> --domain <DOMAIN> --port 53 \
    > /var/log/cnc-server.log 2>&1 &
  tail -f /var/log/cnc-server.log
EOF
