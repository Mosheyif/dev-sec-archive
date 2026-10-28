
# Automating EC2 Instance Shutdown

This guide provides two methods to automatically shut down AWS EC2 instances at the end of the working day to optimize cloud costs.

## Method 1: Bash Script with Cron Job

This method uses the AWS CLI to find all running instances in a specific region and shut them down. You can schedule this script to run automatically using a Linux cron job.

### The Script (`shutdown_ec2.sh`)

Create the script to shut down the instances.

### Scheduling the Script
1. Save the script to your machine (e.g., `/path/to/shutdown_ec2.sh`).
2. Make the script executable:
   ```bash
   chmod +x /path/to/shutdown_ec2.sh
   ```
3. Open your crontab editor:
   ```bash
   crontab -e
   ```
4. Add a cron job to run the script at your desired time (e.g., 6:00 PM every weekday):
   ```bash
   0 18 * * 1-5 /path/to/shutdown_ec2.sh
   ```

## Method 2: Amazon EventBridge (Serverless Approach)

A more cloud-native approach that does not require maintaining a running server or local machine for the cron job is using **Amazon EventBridge Scheduler**.

### Steps to Configure EventBridge
1. Navigate to the **Amazon EventBridge** console.
2. Click on **Schedules** and select **Create schedule**.
3. **Schedule Pattern:** Choose **Recurring schedule** and define a Cron-based expression (e.g., `cron(0 18 ? * MON-FRI *)` for 6:00 PM on weekdays).
4. **Target:** Search for the **EC2** API and select the `StopInstances` action.
5. **Input:** Provide the specific `InstanceIds` as a JSON payload.
6. **Permissions:** Allow EventBridge to create a new IAM role with the necessary permissions (`ec2:StopInstances`) to execute the action.
7. Review the settings and click **Create schedule**.

This method is highly recommended as it is fully managed, serverless, and highly resilient.
