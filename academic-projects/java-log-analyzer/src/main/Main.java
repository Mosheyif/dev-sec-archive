/*********************************************************
*
* Description: Application entry point — loads configuration and starts log processing.
* Author: Moshe Yifrah
*
*********************************************************/

package main;

import java.io.IOException;
import java.util.Properties;

import config.ConfigurationManager;
import logprocessor.LogProcessor;

/**
 * Main class for the Distributed Log Analyzer application.
 * This class initializes and starts the log analysis process.
 */
public class Main {
    
    /**
     * Entry point of the application.
     * 
     * @param args Command line arguments.
     */
    public static void main(String[] args) {
        try {
            // Load configuration
            ConfigurationManager configManager = ConfigurationManager.getInstance();
            Properties config = configManager.loadConfig("properties.config");
            
            // Initialize the log processor with the loaded configuration
            LogProcessor processor = new LogProcessor(config);
            
            // Start processing the logs
            processor.processLogs();
            
        } catch (IOException e) {
            System.out.println("Error loading configuration: " + e.getMessage());
        } catch (Exception e) {
            System.out.println("Unexpected error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}