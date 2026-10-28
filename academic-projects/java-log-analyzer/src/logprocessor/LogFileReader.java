/*********************************************************
*
* Description: Reads and parses log files into LogEntry collections.
* Author: Moshe Yifrah
*
*********************************************************/

package logprocessor;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import model.LogEntry;
import parser.LogParser;

/**
 * Handles reading log files and parsing them into LogEntry objects.
 */
public class LogFileReader {
    
    /**
     * Reads a log file and parses its contents into LogEntry objects.
     * 
     * @param file The log file to read.
     * @return A list of LogEntry objects.
     * @throws IOException If an error occurs while reading the file.
     */
    public List<LogEntry> readLogFile(File file) throws IOException {
        System.out.println("Reading file: " + file.getName() + " (size: " + file.length() + " bytes)");
        List<LogEntry> entries = new ArrayList<>();
        
        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            String line;
            int lineCount = 0;
            int validEntries = 0;
            
            while ((line = reader.readLine()) != null) {
                lineCount++;
                // Skip empty lines
                if (line.trim().isEmpty()) {
                    continue;
                }
                
                // Parse the log entry and include the file name
                LogEntry entry = LogParser.parse(line, file.getName());
                if (entry != null) {
                    entries.add(entry);
                    validEntries++;
                }
            }
            
            System.out.println("File " + file.getName() + " - Total lines: " + lineCount + ", Valid entries: " + validEntries);
        } catch (IOException e) {
            System.out.println("error processing file " + file.getName() + ": " + e.getMessage());
            throw e;
        }
        
        return entries;
    }
    
    /**
     * Gets a list of log files from a directory.
     * 
     * @param directoryPath The path to the directory containing log files.
     * @return A list of log files.
     * @throws IllegalArgumentException If the directory is invalid or cannot be accessed.
     */
    public List<File> getLogFiles(String directoryPath) {
        System.out.println("Searching for log files in directory: " + directoryPath);
        System.out.println("Current working directory: " + System.getProperty("user.dir"));
        
        File directory = new File(directoryPath);
        
        if (!directory.exists() || !directory.isDirectory()) {
            System.out.println("invalid log path: " + directory.getAbsolutePath());
            throw new IllegalArgumentException("Invalid log directory: " + directoryPath);
        }
        
        List<File> logFiles = new ArrayList<>();
        File[] files = directory.listFiles();
        
        if (files != null) {
            System.out.println("Found " + files.length + " files in directory");
            for (File file : files) {
                System.out.println("Checking file: " + file.getName() + " - isFile: " + file.isFile());
                if (file.isFile() && file.getName().toLowerCase().endsWith(".log")) {
                    logFiles.add(file);
                    System.out.println("Added " + file.getName() + " to processing list");
                }
            }
        } else {
            System.out.println("No files found in directory or directory cannot be read");
        }
        
        System.out.println("Found " + logFiles.size() + " log files to process");
        return logFiles;
    }
}