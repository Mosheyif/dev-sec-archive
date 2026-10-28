/*********************************************************
*
* Description: Singleton configuration manager with property file loading.
* Author: Moshe Yifrah
*
*********************************************************/

package config;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

/**
 * Singleton class responsible for managing configuration settings.
 * Uses the Singleton design pattern to ensure only one instance exists.
 */
public class ConfigurationManager {
    
    private static ConfigurationManager instance;
    private Properties defaultProperties;
    
    /**
     * Private constructor to prevent direct instantiation.
     * Initializes default configuration values.
     */
    private ConfigurationManager() {
        defaultProperties = new Properties();
        
        // Set default configuration values
        defaultProperties.setProperty("log.directory", "logs");
        defaultProperties.setProperty("thread.pool.size", "5");
        defaultProperties.setProperty("output.file", "output.json");
        defaultProperties.setProperty("log.analysis", "COUNT_LEVELS");
        defaultProperties.setProperty("log.analysis.anomalies.levels", "ERROR");
        defaultProperties.setProperty("log.analysis.anomalies.window", "30");
        defaultProperties.setProperty("log.analysis.anomalies.threshold", "2");
    }
    
    /**
     * Gets the singleton instance of ConfigurationManager.
     * 
     * @return The ConfigurationManager instance.
     */
    public static synchronized ConfigurationManager getInstance() {
        if (instance == null) {
            instance = new ConfigurationManager();
        }
        return instance;
    }
    
    /**
     * Loads configuration from a properties file, applying default values for missing properties.
     * 
     * @param configFile Path to the configuration file.
     * @return Properties object containing the configuration.
     * @throws IOException If the configuration file cannot be read.
     */
    public Properties loadConfig(String configFile) throws IOException {
        Properties properties = new Properties(defaultProperties);
        
        try (InputStream input = new FileInputStream(configFile)) {
            properties.load(input);
            return properties;
        } catch (IOException e) {
            System.out.println("Warning: Could not load configuration file. Using default values.");
            return defaultProperties;
        }
    }
}